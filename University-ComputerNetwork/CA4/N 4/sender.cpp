#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include "packet.hpp"
#include "logger.hpp"

using namespace std;

static double elapsed_ms(const chrono::steady_clock::time_point& start_time) {
    return chrono::duration<double, milli>(chrono::steady_clock::now() - start_time).count();
}

static void log_event(ofstream& events, const chrono::steady_clock::time_point& start_time, const string& event) {
    events << fixed << setprecision(3) << elapsed_ms(start_time) << " ms | " << event << '\n';
    events.flush();
}

static void record_metrics(ofstream& cwnd_file, ofstream& throughput_file,
                           const chrono::steady_clock::time_point& start_time,
                           double cwnd, double ssthresh, uint64_t acknowledged_bytes,
                           const string& event = "") {
    double time_ms = elapsed_ms(start_time);
    double throughput_kbps = time_ms > 0.0 ? (acknowledged_bytes * 8.0) / time_ms : 0.0;
    cwnd_file << fixed << setprecision(3) << time_ms << ',' << cwnd << ',' << ssthresh
              << ',' << event << '\n';
    throughput_file << fixed << setprecision(3) << time_ms << ',' << throughput_kbps << '\n';
    cwnd_file.flush();
    throughput_file.flush();
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: ./sender <receiver_ip> <receiver_port> <input_file>"
                " [--packet-size <bytes>] [--timeout <ms>] [--no-fast-retransmit]\n";
        return -1;
    }

    string receiver_ip = argv[1];
    int receiver_port = stoi(argv[2]);
    string input_filename = argv[3];
    size_t packet_size = DEFAULT_PAYLOAD_SIZE;
    int timeout_ms = 1000;
    bool fast_retransmit_enabled = true;

    for (int index = 4; index < argc; index++) {
        string argument = argv[index];
        if (argument == "--packet-size" && index + 1 < argc) {
            packet_size = stoul(argv[++index]);
        } else if (argument == "--timeout" && index + 1 < argc) {
            timeout_ms = stoi(argv[++index]);
        } else if (argument == "--no-fast-retransmit") {
            fast_retransmit_enabled = false;
        } else {
            cerr << "Unknown or incomplete argument: " << argument << '\n';
            return -1;
        }
    }

    if (packet_size == 0 || packet_size > MAX_PAYLOAD_SIZE) {
        cerr << "Packet size must be between 1 and " << MAX_PAYLOAD_SIZE << " bytes\n";
        return -1;
    }
    if (timeout_ms <= 0) {
        cerr << "Timeout must be greater than zero milliseconds\n";
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        Logger::error("Socket creation failed");
        return -1;
    }

    struct sockaddr_in receiver_addr {};
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(receiver_port);
    receiver_addr.sin_addr.s_addr = inet_addr(receiver_ip.c_str());

    ifstream infile(input_filename, ios::binary);
    if (!infile) {
        Logger::error("Cannot open " + input_filename);
        close(sockfd);
        return -1;
    }

    vector<Packet> packets;
    uint32_t sequence_number = 1;
    while (infile.peek() != EOF) {
        vector<char> buffer(packet_size);
        infile.read(buffer.data(), packet_size);
        size_t bytes_read = infile.gcount();
        buffer.resize(bytes_read);

        Packet packet;
        packet.header.seq_num = sequence_number++;
        packet.header.ack_num = 0;
        packet.header.length = bytes_read;
        packet.header.flags = FLAG_DATA;
        packet.payload = buffer;
        packet.header.checksum = Packet::calculate_checksum(packet.header, packet.payload);
        packets.push_back(packet);
    }

    ofstream cwnd_file("cwnd.csv", ios::trunc);
    ofstream throughput_file("throughput.csv", ios::trunc);
    ofstream events("events.log", ios::trunc);
    if (!cwnd_file || !throughput_file || !events) {
        Logger::error("Cannot create metric output files");
        close(sockfd);
        return -1;
    }

    cwnd_file << "time_ms,cwnd_packets,ssthresh_packets,event\n";
    throughput_file << "time_ms,throughput_kbps\n";

    const uint32_t receiver_window = 64;
    const uint32_t total_packets = packets.size();
    uint32_t base = 1;
    uint32_t next_seq_num = 1;
    double cwnd = 1.0;
    double ssthresh = 16.0;
    uint64_t acknowledged_bytes = 0;
    int total_packets_sent = 0;
    int retransmitted_packets = 0;
    int retransmission_events = 0;
    int timeout_count = 0;
    int fast_retransmit_count = 0;
    uint32_t duplicate_ack_number = 0;
    int duplicate_ack_count = 0;
    bool in_fast_recovery = false;
    uint32_t recovery_point = 0;
    vector<bool> packet_sent(total_packets, false);

    auto start_time = chrono::steady_clock::now();
    record_metrics(cwnd_file, throughput_file, start_time, cwnd, ssthresh, acknowledged_bytes);
    log_event(events, start_time,
              "START transfer, cwnd=1, ssthresh=16, fast_retransmit=" +
              string(fast_retransmit_enabled ? "enabled" : "disabled"));

    auto send_packet = [&](uint32_t packet_number, const string& reason,
                           uint32_t effective_window) {
        vector<char> serialized_data = packets[packet_number - 1].serialize();
        ssize_t sent_bytes = sendto(sockfd, serialized_data.data(), serialized_data.size(), 0,
                                    reinterpret_cast<const struct sockaddr*>(&receiver_addr),
                                    sizeof(receiver_addr));
        if (sent_bytes < 0) {
            return false;
        }
        total_packets_sent++;
        if (packet_sent[packet_number - 1]) {
            retransmitted_packets++;
        }
        packet_sent[packet_number - 1] = true;
        string event = reason + " packet seq=" + to_string(packet_number) +
                       ", cwnd=" + to_string(cwnd) +
                       ", effective_window=" + to_string(effective_window);
        Logger::info(event);
        log_event(events, start_time, event);
        return true;
    };

    while (base <= total_packets) {
        uint32_t congestion_window = max(1U, static_cast<uint32_t>(floor(cwnd)));
        uint32_t effective_window = min(receiver_window, congestion_window);

        while (next_seq_num < base + effective_window && next_seq_num <= total_packets) {
            if (!send_packet(next_seq_num, "SEND", effective_window)) {
                Logger::error("Failed to send packet");
                close(sockfd);
                return -1;
            }
            next_seq_num++;
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        struct timeval timeout {};
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;

        int select_result = select(sockfd + 1, &readfds, nullptr, nullptr, &timeout);
        if (select_result > 0 && FD_ISSET(sockfd, &readfds)) {
            vector<char> ack_buffer(sizeof(PacketHeader) + MAX_PAYLOAD_SIZE);
            struct sockaddr_in sender_addr {};
            socklen_t address_length = sizeof(sender_addr);
            int received_length = recvfrom(sockfd, ack_buffer.data(), ack_buffer.size(), 0,
                                           reinterpret_cast<struct sockaddr*>(&sender_addr),
                                           &address_length);
            if (received_length <= 0) {
                continue;
            }

            ack_buffer.resize(received_length);
            Packet ack_packet = Packet::deserialize(ack_buffer);
            uint16_t expected_checksum = ack_packet.header.checksum;
            ack_packet.header.checksum = 0;
            uint16_t calculated_checksum = Packet::calculate_checksum(ack_packet.header, ack_packet.payload);
            if (calculated_checksum != expected_checksum || ack_packet.header.flags != FLAG_ACK) {
                log_event(events, start_time, "INVALID ACK ignored");
                continue;
            }

            uint32_t acknowledged_until = min(ack_packet.header.ack_num, total_packets + 1);
            if (acknowledged_until <= base) {
                if (acknowledged_until == base) {
                    if (duplicate_ack_number == acknowledged_until) {
                        duplicate_ack_count++;
                    } else {
                        duplicate_ack_number = acknowledged_until;
                        duplicate_ack_count = 1;
                    }
                }
                string event = "DUPLICATE ACK ack=" + to_string(ack_packet.header.ack_num) +
                               ", count=" + to_string(duplicate_ack_count);
                Logger::info(event);
                log_event(events, start_time, event);

                if (fast_retransmit_enabled && acknowledged_until == base &&
                    duplicate_ack_count == 3 && !in_fast_recovery) {
                    double previous_cwnd = cwnd;
                    ssthresh = max(2.0, floor(cwnd / 2.0));
                    cwnd = ssthresh + 3.0;
                    recovery_point = next_seq_num - 1;
                    in_fast_recovery = true;
                    retransmission_events++;
                    fast_retransmit_count++;
                    string fast_event = "FAST RETRANSMIT seq=" + to_string(base) +
                                        ", cwnd " + to_string(previous_cwnd) +
                                        " -> " + to_string(cwnd) +
                                        ", ssthresh=" + to_string(ssthresh) +
                                        ", recovery_point=" + to_string(recovery_point);
                    Logger::error(fast_event);
                    log_event(events, start_time, fast_event);
                    record_metrics(cwnd_file, throughput_file, start_time, cwnd, ssthresh,
                                   acknowledged_bytes, "FAST_RETRANSMIT");
                    if (!send_packet(base, "FAST RETRANSMIT SEND", effective_window)) {
                        Logger::error("Failed to fast retransmit packet");
                        close(sockfd);
                        return -1;
                    }
                } else if (in_fast_recovery && acknowledged_until == base &&
                           duplicate_ack_count > 3) {
                    cwnd += 1.0;
                    record_metrics(cwnd_file, throughput_file, start_time, cwnd, ssthresh,
                                   acknowledged_bytes, "FAST_RECOVERY");
                }
                continue;
            }

            uint32_t newly_acknowledged = acknowledged_until - base;
            for (uint32_t packet_number = base; packet_number < acknowledged_until; packet_number++) {
                acknowledged_bytes += packets[packet_number - 1].payload.size();
            }
            base = acknowledged_until;
            duplicate_ack_number = 0;
            duplicate_ack_count = 0;

            string phase;
            if (in_fast_recovery) {
                if (base > recovery_point) {
                    cwnd = ssthresh;
                    in_fast_recovery = false;
                    phase = "Fast Recovery Exit";
                } else {
                    cwnd = ssthresh + 3.0;
                    phase = "Fast Recovery Partial ACK";
                    retransmission_events++;
                    fast_retransmit_count++;
                    string fast_event = "FAST RETRANSMIT partial ACK, seq=" + to_string(base) +
                                        ", cwnd=" + to_string(cwnd) +
                                        ", ssthresh=" + to_string(ssthresh);
                    Logger::error(fast_event);
                    log_event(events, start_time, fast_event);
                    record_metrics(cwnd_file, throughput_file, start_time, cwnd, ssthresh,
                                   acknowledged_bytes, "FAST_RETRANSMIT");
                    if (!send_packet(base, "FAST RETRANSMIT SEND", effective_window)) {
                        Logger::error("Failed to fast retransmit packet");
                        close(sockfd);
                        return -1;
                    }
                }
            } else {
                for (uint32_t index = 0; index < newly_acknowledged; index++) {
                    if (cwnd < ssthresh) {
                        cwnd += 1.0;
                        phase = "Slow Start";
                    } else {
                        cwnd += 1.0 / cwnd;
                        phase = "Congestion Avoidance";
                    }
                }
            }

            string event = "ACK ack=" + to_string(ack_packet.header.ack_num) +
                           ", newly_acked=" + to_string(newly_acknowledged) +
                           ", phase=" + phase + ", cwnd=" + to_string(cwnd);
            Logger::success(event);
            log_event(events, start_time, event);
            record_metrics(cwnd_file, throughput_file, start_time, cwnd, ssthresh, acknowledged_bytes);
        } else if (select_result == 0) {
            double previous_cwnd = cwnd;
            ssthresh = max(1.0, floor(cwnd / 2.0));
            cwnd = 1.0;
            next_seq_num = base;
            timeout_count++;
            retransmission_events++;
            duplicate_ack_number = 0;
            duplicate_ack_count = 0;
            in_fast_recovery = false;

            string event = "TIMEOUT at base=" + to_string(base) +
                           ", cwnd " + to_string(previous_cwnd) +
                           " -> 1, ssthresh=" + to_string(ssthresh) +
                           ", RETRANSMISSION starts";
            Logger::error(event);
            log_event(events, start_time, event);
            record_metrics(cwnd_file, throughput_file, start_time, cwnd, ssthresh,
                           acknowledged_bytes, "TIMEOUT");
        } else {
            Logger::error("select() failed");
            log_event(events, start_time, "ERROR select() failed");
            close(sockfd);
            return -1;
        }
    }

    double total_time_ms = elapsed_ms(start_time);
    double average_throughput_kbps = total_time_ms > 0.0
        ? (acknowledged_bytes * 8.0) / total_time_ms : 0.0;

    log_event(events, start_time, "COMPLETE transfer");
    cout << "\n--- Transfer Complete ---\n";
    cout << "Total Packets Sent: " << total_packets_sent << '\n';
    cout << "Retransmitted Packets: " << retransmitted_packets << '\n';
    cout << "Total Retransmission Events: " << retransmission_events << '\n';
    cout << "Total Timeouts: " << timeout_count << '\n';
    cout << "Fast Retransmits: " << fast_retransmit_count << '\n';
    cout << "Total Transfer Time: " << total_time_ms << " ms\n";
    cout << "Average Throughput: " << average_throughput_kbps << " Kbps\n";
    cout << "Final cwnd: " << cwnd << " packets\n";
    cout << "Final ssthresh: " << ssthresh << " packets\n";
    cout << "Metrics: cwnd.csv, throughput.csv, events.log\n";

    close(sockfd);
    return 0;
}

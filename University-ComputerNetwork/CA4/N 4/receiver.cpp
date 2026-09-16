#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
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

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: ./receiver <port> <output_file> [--loss <prob>] [--delay <ms>]"
                " [--packet-size <bytes>] [--seed <value>] [--drop-seq-once <seq>]\n";
        return -1;
    }

    int port = stoi(argv[1]);
    string output_filename = argv[2];
    double loss_prob = 0.0;
    int delay_ms = 0;
    size_t packet_size = DEFAULT_PAYLOAD_SIZE;
    unsigned int random_seed = static_cast<unsigned int>(time(nullptr));
    uint32_t drop_seq_once = 0;
    bool deterministic_drop_done = false;

    for (int index = 3; index < argc; index++) {
        string argument = argv[index];
        if (argument == "--loss" && index + 1 < argc) {
            loss_prob = stod(argv[++index]);
        } else if (argument == "--delay" && index + 1 < argc) {
            delay_ms = stoi(argv[++index]);
        } else if (argument == "--packet-size" && index + 1 < argc) {
            packet_size = stoul(argv[++index]);
        } else if (argument == "--seed" && index + 1 < argc) {
            random_seed = static_cast<unsigned int>(stoul(argv[++index]));
        } else if (argument == "--drop-seq-once" && index + 1 < argc) {
            drop_seq_once = static_cast<uint32_t>(stoul(argv[++index]));
        } else {
            cerr << "Unknown or incomplete argument: " << argument << '\n';
            return -1;
        }
    }

    if (loss_prob < 0.0 || loss_prob > 1.0) {
        cerr << "Loss probability must be between 0 and 1\n";
        return -1;
    }
    if (delay_ms < 0) {
        cerr << "Delay must not be negative\n";
        return -1;
    }
    if (packet_size == 0 || packet_size > MAX_PAYLOAD_SIZE) {
        cerr << "Packet size must be between 1 and " << MAX_PAYLOAD_SIZE << " bytes\n";
        return -1;
    }

    srand(random_seed);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        Logger::error("Socket creation failed");
        return -1;
    }

    struct sockaddr_in server_addr {};
    struct sockaddr_in client_addr {};
    socklen_t client_length = sizeof(client_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, reinterpret_cast<const struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        Logger::error("Bind failed");
        close(sockfd);
        return -1;
    }

    Logger::info("Receiver is waiting on port " + to_string(port) + "...");
    Logger::info("Settings -> Loss: " + to_string(loss_prob) + ", Delay: " + to_string(delay_ms) + " ms");
    Logger::info("Random seed: " + to_string(random_seed));
    Logger::info("Packet size: " + to_string(packet_size) + " bytes");

    ofstream outfile(output_filename, ios::binary | ios::trunc);
    ofstream events("receiver_events.log", ios::trunc);
    if (!outfile || !events) {
        Logger::error("Failed to create receiver output files");
        close(sockfd);
        return -1;
    }

    auto start_time = chrono::steady_clock::now();
    log_event(events, start_time, "START receiver on port=" + to_string(port));
    uint32_t expected_seq_num = 1;
    map<uint32_t, vector<char>> out_of_order_packets;

    while (true) {
        vector<char> buffer(sizeof(PacketHeader) + packet_size);
        int received_length = recvfrom(sockfd, buffer.data(), buffer.size(), 0,
                                       reinterpret_cast<struct sockaddr*>(&client_addr),
                                       &client_length);
        if (received_length <= 0) {
            continue;
        }

        buffer.resize(received_length);
        Packet packet = Packet::deserialize(buffer);

        if (!deterministic_drop_done && drop_seq_once != 0 &&
            packet.header.seq_num == drop_seq_once) {
            deterministic_drop_done = true;
            string event = "DROP ONCE packet seq=" + to_string(packet.header.seq_num);
            Logger::error(event);
            log_event(events, start_time, event);
            continue;
        }

        if (loss_prob > 0.0) {
            double random_value = static_cast<double>(rand()) / RAND_MAX;
            if (random_value < loss_prob) {
                string event = "DROP packet seq=" + to_string(packet.header.seq_num);
                Logger::error("Simulated LOSS for packet Seq: " + to_string(packet.header.seq_num));
                log_event(events, start_time, event);
                continue;
            }
        }

        if (delay_ms > 0) {
            this_thread::sleep_for(chrono::milliseconds(delay_ms));
        }

        uint16_t expected_checksum = packet.header.checksum;
        packet.header.checksum = 0;
        uint16_t calculated_checksum = Packet::calculate_checksum(packet.header, packet.payload);
        if (calculated_checksum != expected_checksum) {
            Logger::error("Checksum validation failed!");
            log_event(events, start_time, "CHECKSUM failure");
            continue;
        }

        if (packet.header.seq_num == expected_seq_num) {
            outfile.write(packet.payload.data(), packet.payload.size());
            Logger::success("Packet " + to_string(packet.header.seq_num) + " received & written.");
            log_event(events, start_time, "RECEIVE packet seq=" + to_string(packet.header.seq_num));
            expected_seq_num++;
            while (out_of_order_packets.count(expected_seq_num) != 0) {
                const vector<char>& payload = out_of_order_packets[expected_seq_num];
                outfile.write(payload.data(), payload.size());
                log_event(events, start_time,
                          "FLUSH BUFFERED packet seq=" + to_string(expected_seq_num));
                out_of_order_packets.erase(expected_seq_num);
                expected_seq_num++;
            }
            outfile.flush();
        } else if (packet.header.seq_num > expected_seq_num) {
            if (out_of_order_packets.count(packet.header.seq_num) == 0) {
                out_of_order_packets[packet.header.seq_num] = packet.payload;
                log_event(events, start_time,
                          "BUFFER OUT_OF_ORDER packet seq=" + to_string(packet.header.seq_num) +
                          ", expected=" + to_string(expected_seq_num));
            }
            Logger::info("Out of order packet buffered (Seq: " +
                         to_string(packet.header.seq_num) + "). Expected: " +
                         to_string(expected_seq_num));
        } else {
            Logger::info("Duplicate packet (Seq: " +
                         to_string(packet.header.seq_num) + "). Expected: " +
                         to_string(expected_seq_num));
            log_event(events, start_time,
                      "DUPLICATE packet seq=" + to_string(packet.header.seq_num) +
                      ", expected=" + to_string(expected_seq_num));
        }

        Packet ack_packet;
        ack_packet.header.seq_num = 0;
        ack_packet.header.ack_num = expected_seq_num;
        ack_packet.header.length = 0;
        ack_packet.header.flags = FLAG_ACK;
        ack_packet.header.checksum = Packet::calculate_checksum(ack_packet.header, ack_packet.payload);

        vector<char> ack_data = ack_packet.serialize();
        sendto(sockfd, ack_data.data(), ack_data.size(), 0,
               reinterpret_cast<struct sockaddr*>(&client_addr), client_length);
        log_event(events, start_time, "SEND ACK ack=" + to_string(expected_seq_num));
    }
}

#ifndef PACKET_HPP
#define PACKET_HPP

#include <cstdint>
#include <vector>
#include <cstring>
#include <arpa/inet.h>

using namespace std;

const uint8_t FLAG_DATA = 0x01;
const uint8_t FLAG_ACK  = 0x02;
const uint8_t FLAG_FIN  = 0x04;

const size_t DEFAULT_PAYLOAD_SIZE = 1024;
const size_t MAX_PAYLOAD_SIZE = 60000;

#pragma pack(push, 1)
struct PacketHeader {
    uint32_t seq_num = 0;
    uint32_t ack_num = 0;
    uint16_t length = 0;
    uint16_t checksum = 0;
    uint8_t flags = 0;
};
#pragma pack(pop)


struct Packet {
    PacketHeader header;
    vector<char> payload;

    static uint16_t calculate_checksum(PacketHeader hdr, const vector<char>& payload_data) {
        hdr.checksum = 0;
        uint32_t sum = 0;
        
        uint16_t* ptr = reinterpret_cast<uint16_t*>(&hdr);
        for (size_t i = 0; i < sizeof(PacketHeader) / 2; ++i) {
            sum += ptr[i];
        }
        if (sizeof(PacketHeader) % 2 != 0) {
            sum += reinterpret_cast<uint8_t*>(&hdr)[sizeof(PacketHeader) - 1];
        }

        const uint16_t* payload_ptr = reinterpret_cast<const uint16_t*>(payload_data.data());
        for (size_t i = 0; i < payload_data.size() / 2; ++i) {
            sum += payload_ptr[i];
        }
        if (payload_data.size() % 2 != 0) {
            sum += reinterpret_cast<const uint8_t*>(payload_data.data())[payload_data.size() - 1];
        }

        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
        return ~sum;
    }

    vector<char> serialize() const {
        vector<char> buffer(sizeof(PacketHeader) + payload.size());
        
        uint32_t net_seq = htonl(header.seq_num);
        uint32_t net_ack = htonl(header.ack_num);
        uint16_t net_len = htons(header.length);
        uint16_t net_chk = htons(header.checksum);

        size_t offset = 0;
        memcpy(buffer.data() + offset, &net_seq, sizeof(net_seq)); offset += sizeof(net_seq);
        memcpy(buffer.data() + offset, &net_ack, sizeof(net_ack)); offset += sizeof(net_ack);
        memcpy(buffer.data() + offset, &net_len, sizeof(net_len)); offset += sizeof(net_len);
        memcpy(buffer.data() + offset, &net_chk, sizeof(net_chk)); offset += sizeof(net_chk);
        memcpy(buffer.data() + offset, &header.flags, sizeof(header.flags)); offset += sizeof(header.flags);

        if (!payload.empty()) {
            memcpy(buffer.data() + offset, payload.data(), payload.size());
        }
        return buffer;
    }

    static Packet deserialize(const std::vector<char>& data) {
        Packet pkt;
        if (data.size() < sizeof(PacketHeader)) return pkt;

        size_t offset = 0;
        uint32_t net_seq, net_ack;
        uint16_t net_len, net_chk;

        memcpy(&net_seq, data.data() + offset, sizeof(net_seq)); offset += sizeof(net_seq);
        memcpy(&net_ack, data.data() + offset, sizeof(net_ack)); offset += sizeof(net_ack);
        memcpy(&net_len, data.data() + offset, sizeof(net_len)); offset += sizeof(net_len);
        memcpy(&net_chk, data.data() + offset, sizeof(net_chk)); offset += sizeof(net_chk);
        memcpy(&pkt.header.flags, data.data() + offset, sizeof(pkt.header.flags)); offset += sizeof(pkt.header.flags);

        pkt.header.seq_num = ntohl(net_seq);
        pkt.header.ack_num = ntohl(net_ack);
        pkt.header.length = ntohs(net_len);
        pkt.header.checksum = ntohs(net_chk);

        size_t payload_size = data.size() - sizeof(PacketHeader);
        if (payload_size > 0) {
            pkt.payload.assign(data.begin() + sizeof(PacketHeader), data.end());
        }
        return pkt;
    }
};

#endif

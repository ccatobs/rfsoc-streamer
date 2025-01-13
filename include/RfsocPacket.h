#ifndef RFSOCPACKET_H
#define RFSOCPACKET_H

#define NUMBER_CHANNELS 2048 // Treating I and Q as separate channels
// 8212 total bytes in packet - 1024*2*8 + 20

struct RfsocPacket {
    //Trying immediately casting packets to ints for easier access in RfsocBuilder
    int32_t data[NUMBER_CHANNELS]; 
    uint8_t packet_info[2]; // two bytes of flags for current drone
    uint16_t channel_count; // number of tones currently written
    uint32_t packet_count;  // packet count since streaming began
    unsigned char raw_ptp_timestamp[12];
} __attribute__((packed));

typedef std::shared_ptr<RfsocPacket> RfsocPacketPtr;
typedef std::shared_ptr<const RfsocPacket> RfsocPacketConstPtr;

#endif

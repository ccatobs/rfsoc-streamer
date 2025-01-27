#ifndef RFSOCPACKET_H
#define RFSOCPACKET_H

#define NUMBER_CHANNELS 2048 // Treating I and Q as separate channels
// 8212 total bytes in packet - 1024*2*8 + 20

struct RfsocPacket {
    //Trying immediately casting packets to ints for easier access in RfsocBuilder
    int32_t data[NUMBER_CHANNELS]; 
    uint8_t packet_info[2]; // two bytes of flags for current drone
    // variables below being read as little-endian, but need to be converted to big-endian
    // conversion takes place in RfsocSample.h
    uint16_t channel_count; // number of tones currently written
    uint32_t packet_count;  // packet count since streaming began
    uint32_t ptp_int_array[3];  // 3-int array of 12-byte raw_ptp_timestamp
} __attribute__((packed));

typedef std::shared_ptr<RfsocPacket> RfsocPacketPtr;
typedef std::shared_ptr<const RfsocPacket> RfsocPacketConstPtr;

#endif

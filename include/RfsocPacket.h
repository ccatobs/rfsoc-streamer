#ifndef RFSOCPACKET_H
#define RFSOCPACKET_H

#define BUFFER_SIZE     8176 // 1022 total possible resonators, 8 bytes each
#define NUMBER_CHANNELS 2044 // Treating I and Q as separate channels

struct RfsocPacket {
    //Trying immediately casting packets to ints for easier access in RfsocBuilder
    int32_t data[NUMBER_CHANNELS]; // is this unsigned or signed?
    unsigned char unused_buffer[6];
    uint32_t packet_info;
    uint32_t packet_count;
    unsigned char raw_ptp_timestamp[12];
} __attribute__((packed));

#endif

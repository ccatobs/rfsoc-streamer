#ifndef RFSOCPACKET_H
#define RFSOCPACKET_H

#define BUFFER_SIZE     9000

struct RfsocPacket {
    unsigned char buffer[BUFFER_SIZE];
} __attribute__((packed));

#endif

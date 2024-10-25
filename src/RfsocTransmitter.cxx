/*
 * RfsocTransmitter.cpp
 *
 * Code to take in packets and place them into G3Pipeline queue
 *
 * Modeled extensively off of the SO smurf-streamer's SmurfTransmitter.cpp
 * but with added elements from spt3g-software's DfMuxCollector.cxx
 */

#include "RfsocTransmitter.h"
#include "RfsocSample.h"
//#include "RfsocBuilder.h"
#include <iostream>
#include <algorithm>
#include <iterator>
#include <boost/algorithm/string.hpp>
#include <inttypes.h>

#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BROADCAST_IP	"192.168.3.40"	// The real transmitter
//#define BROADCAST_IP	"127.255.255.255"	// The local computer for testing
#define BROADCAST_PORT	4096

RfsocTransmitter::RfsocTransmitter(G3EventBuilderPtr builder) :
    builder_(builder), success_(false), stop_listening_(false)
{
    success_ = (SetupUDPSocket() != 0);
}

RfsocTransmitter::~RfsocTransmitter()
{
    Stop();
    close(sockfd);
}

void RfsocTransmitter::dataTransmit(struct RfsocPacket *rp){

    G3Time ts = G3Time::Now();
    RfsocSamplePtr rfsoc_sample(new RfsocSample(ts, rp));
    builder_->AsyncDatum(ts.time, rfsoc_sample);
}

// borrowing from Ben's code - just listening for anything on network that arrives at BROADCAST_IP
int RfsocTransmitter::SetupUDPSocket()
{
    struct sockaddr_in rx_addr;

    // Create the socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Use the socket in "broadcast" mode
    int enabled = 1;
    if ((setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled))) < 0 ) {
        perror("socket broadcast option failed");
        exit(EXIT_FAILURE);
    }

    // Specify the receiver address
    memset(&rx_addr, 0, sizeof(rx_addr));
    rx_addr.sin_family      = AF_INET;
    rx_addr.sin_port        = htons(BROADCAST_PORT);
    rx_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind to the receiver socket
    if (bind(sockfd, (const sockaddr*)&rx_addr, sizeof(rx_addr)) < 0) {
	perror("bind failed");
	exit(EXIT_FAILURE);
    }

    return 0;
}

int RfsocTransmitter::Start()
{
    stop_listening_ = false;
    listen_thread_ = std::thread(Listen, this);

    return (0);
}

int RfsocTransmitter::Stop()
{
    stop_listening_ = true;
    listen_thread_.join();

    return (0);
}

//Adapted to use Ben's code, but could go back to DfMuxCollectors's recvfrom
void RfsocTransmitter::Listen(RfsocTransmitter *transmitter)
{
    struct RfsocPacket buf;
    ssize_t len;

    while (!transmitter->stop_listening_) {
        len = recv(transmitter->sockfd, &buf, sizeof(buf), 0);
        // Because the current packet is in an incomplete format, we need to roll
        // it by -1 to make sure it is byte-aligned and then make groups of
        // four bytes into ints. In Python, the code we are currently using to do
        // this is:
        //     data = sock.recv(9000)
        //     data = bytearray(data)
        //     data = np.roll(data, -1)
        //     return np.frombuffer(data, dtype="<i").astype("float")
        // We need to implement an equivalent here until we get the updated firmware
        // in which these issues are fixed. I believe the char array is already an equivalent of bytearray.
        std::rotate(std::begin(buf.buffer), std::begin(buf.buffer) + 1, std::end(buf.buffer)); //rotating one to the left

        // The frombuffer equivalent is implemented in RfsocBuilder::FrameFromSamples() when it calls
        // so3g's SetDataFromBuffer, I believe

        buf.buffer[len] = '\0';
        transmitter->dataTransmit(&buf);
    }
}


/*
 * RfsocTransmitter.cpp
 *
 * Code to take in packets and place them into G3Pipeline queue
 *
 * Modeled extensively off of the SO Rfsoc-streamer's RfsocTransmitter.cpp
 * but with added elements from spt3g-software's DfMuxCollector.cxx
 */

#include "RfsocTransmitter.h"
#include "RfsocSample.h"
//#include "RfsocBuilder.h"
#include <iostream>
#include <algorithm>
#include <iterator>
#include <inttypes.h>
#include <boost/python.hpp>

#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define RECEIVE_IP	"192.168.3.40"	// The receiving computer IP
//#define CONNECT_IP  "192.168.3.58"  // The drone IP address from which to receive packets
#define RECEIVE_PORT	4096
#define CONNECT_PORT    4096

namespace bp = boost::python;

RfsocTransmitter::RfsocTransmitter(G3EventBuilderPtr builder, std::string source_ip) :
    builder_(builder), success_(false), stop_listening_(false)
{
    connect_ip_ = source_ip;
    success_ = (SetupUDPSocket() != 0);
}

RfsocTransmitter::~RfsocTransmitter()
{
    Stop();
    close(sockfd);
}

void RfsocTransmitter::dataTransmit(RfsocPacketPtr rp){

    G3Time ts = G3Time::Now();
    RfsocSamplePtr rfsoc_sample(new RfsocSample(ts, rp));
    builder_->AsyncDatum(ts.time, rfsoc_sample);
}

// borrowing from Ben's code - just listening for anything on network that arrives at BROADCAST_IP
int RfsocTransmitter::SetupUDPSocket()
{
    struct sockaddr_in rx_addr;
    struct sockaddr_in serv_addr;

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

    // Allow multiple listeners on same receive port
    int enable_multi = 1;
    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable_multi, sizeof(enable_multi))) < 0) {
        perror("socket reuseaddr option failed");
        exit(EXIT_FAILURE);
    }

    // Specify the receiver address
    memset(&rx_addr, 0, sizeof(rx_addr));
    rx_addr.sin_family      = AF_INET;
    rx_addr.sin_port        = htons(RECEIVE_PORT);
    rx_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind to the receiver socket
    if (bind(sockfd, (const sockaddr*)&rx_addr, sizeof(rx_addr)) < 0) {
	perror("bind failed");
	exit(EXIT_FAILURE);
    }

    // Specify the drone/server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family    = AF_INET;
    serv_addr.sin_port      = htons(CONNECT_PORT);
    // Convert server IP to binary
    if (inet_pton(AF_INET, connect_ip_.c_str(), &serv_addr.sin_addr) < 0) {
        perror("Invalid connect IP address");
        exit(EXIT_FAILURE);
    }
    // Connect to drone/server address to get only packets from here
    if (connect(sockfd, (const sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connect failed");
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
    ssize_t len;

    while (!transmitter->stop_listening_) {
        RfsocPacketPtr buf = std::make_shared<RfsocPacket>();
        len = recv(transmitter->sockfd, buf.get(), sizeof(RfsocPacket), 0);
        transmitter->dataTransmit(buf);
    }
}

void RfsocTransmitter::setup_python(){
    bp::class_< RfsocTransmitter,
                std::shared_ptr<RfsocTransmitter>,
                boost::noncopyable >
                ("RfsocTransmitter", bp::init<G3EventBuilderPtr, std::string>())
    .def("Start", &RfsocTransmitter::Start)
    .def("GetConnectIP", &RfsocTransmitter::GetConnectIP)
    ;
}

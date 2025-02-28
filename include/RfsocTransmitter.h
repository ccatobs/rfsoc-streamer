#ifndef _RFSOCTRANSMITTER_H
#define _RFSOCTRANSMITTER_H

#include <G3Frame.h>
#include <G3Writer.h>
#include <G3Data.h>
#include <G3Vector.h>
#include <G3Timestream.h>
#include <G3TimeStamp.h>
#include <G3Units.h>
#include <G3NetworkSender.h>
#include <G3EventBuilder.h>
#include <G3Logging.h>
#include <RfsocPacket.h>

class RfsocTransmitter{
public:

    RfsocTransmitter(G3EventBuilderPtr builder, std::string source_ip);

    ~RfsocTransmitter();

    static void setup_python();

    int Start(); //Start listening thread 
    int Stop(); // Stop listening thread

    std::string GetConnectIP() { return connect_ip_; }
 
private:

    G3EventBuilderPtr builder_;

    int SetupUDPSocket();                               //Build socket and bind to it in broadcast mode
    // original code with RfsocPacket *rp
    //void dataTransmit(struct RfsocPacket *rp);          //Wrap data packet into RfsocSample (since the builder expects a G3FrameObject instance) and pass to builder
    // new code with RfsocPacketPtr
    void dataTransmit(RfsocPacketPtr rp);               //Wrap data packet into RfsocSample
    static void Listen(RfsocTransmitter *transmitter);  //Capture packet, adjust for incomplete packet format, and pass to dataTransmit
	std::thread listen_thread_;

    bool success_;
    volatile bool stop_listening_;
    int sockfd;

    std::string connect_ip_ = "";

    SET_LOGGER("RfsocTransmitter")
};

typedef std::shared_ptr<RfsocTransmitter> RfsocTransmitterPtr;
typedef std::shared_ptr<const RfsocTransmitter> RfsocTransmitterConstPtr;

#endif

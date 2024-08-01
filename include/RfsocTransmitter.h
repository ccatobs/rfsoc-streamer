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

class RfsocTransmitter{
public:

    RfsocTransmitter(G3EventBuilderPtr builder);

    ~RfsocTransmitter();

    int Start(); //Start listening thread 
    int Stop(); // Stop listening thread


private:

    G3EventBuilderPtr builder_;

    int SetupUDPSocket();                               //Build socket and bind to it in broadcast mode
    void dataTransmit(struct RfsocPacket *rp);          //Wrap data packet into RfsocSample (since the builder expects a G3FrameObject instance) and pass to builder
    static void Listen(RfsocTransmitter *transmitter);  //Capture packet, adjust for incomplete packet format, and pass to dataTransmit
	std::thread listen_thread_;

    bool success_;
	volatile bool stop_listening_;
    int sockfd;

    SET_LOGGER("RfsocTransmitter")
};

typedef std::shared_ptr<RfsocTransmitter> RfsocTransmitterPtr;
typedef std::shared_ptr<const RfsocTransmitter> RfsocTransmitterConstPtr;

#endif

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

TO-DO: See if there are things from the smurf BaseTransmitter.h that we need

class RfsocSampleFrameObject : public G3FrameObject{
public:
    uint32_t seq;
};

void printRfsocPacket(RfsocPacketROPtr rp);

class SmurfTransmitter : public sct::BaseTransmitter{
public:

    SmurfTransmitter(G3EventBuilderPtr builder);

    ~SmurfTransmitter();


private:

    G3EventBuilderPtr builder_;

    void dataTransmit(RfsocPacketROPtr packet);

    SET_LOGGER("RfsocTransmitter")
};

typedef std::shared_ptr<RfsocTransmitter> RfsocTransmitterPtr;
typedef std::shared_ptr<const RfsocTransmitter> RfsocTransmitterConstPtr;

#endif

#ifndef RFSOCSAMPLE_H
#define RFSOCSAMPLE_H

#include <G3Frame.h>
#include <G3TimeStamp.h>
#include <vector>

#include <pybindings.h>
#include <serialization.h>

//#include <RfsocPacket.h> - placed all of the packet stuff as a struct in RfsocTransmitter.cpp for now

class RfsocSample : public G3FrameObject{
public:
    RfoscSample(G3Time time, struct RfsocPacket rp) :
        G3FrameObject(), time_(time), rp(rp) {}

    const struct RfsocPacket rp;

    // Returns G3Time for packet. If time can be determined from timing system
    // this will use that and the GetTimingParadigm will return HighPrecision.
    // If not, this will use the timestamp generated in software and
    // GetTimingParadigm will return LowPrecision. -- not implemented for RFSoC!
    const G3Time GetTime() const {
        return time_;
    }

private:
    const G3Time time_;
};

G3_POINTERS(RfsocSample);

#endif

#ifndef RFSOCSAMPLE_H
#define RFSOCSAMPLE_H

#include <G3Frame.h>
#include <G3TimeStamp.h>
#include <vector>

#include <RfsocPacket.h>

class RfsocSample : public G3FrameObject{
public:
    RfoscSample(G3Time time, RfsocPacketROPtr rp) :
        G3FrameObject(), time_(time), rp(rp) {}

    const RfsocPacketROPtr rp;

    // Returns G3Time for packet. If time can be determined from timing system
    // this will use that and the GetTimingParadigm will return HighPrecision.
    // If not, this will use the timestamp generated in software and
    // GetTimingParadigm will return LowPrecision.
    const G3Time GetTime() const {
        return time_;
    }

private:
    const G3Time time_;
};

G3_POINTERS(RfsocSample);

#endif




"""
TO-DO: need to define packet fully in here instead of in separate SmurfPacket class (or just make an RfsocPacket class) - from SmurfPacket.h,
it seems to have a lot of overhead for constructors and destructors but the main things are just buffers for the header and data (probably want the CopyCreator versions,
not the ZeroCopyCreator versions - we don't have separate frame objects, so I'm just going to stuff each incoming packet into buffers using Ben's code).

"""

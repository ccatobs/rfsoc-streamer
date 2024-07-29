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

#ifndef RFSOCSAMPLE_H
#define RFSOCSAMPLE_H

#include <G3Frame.h>
#include <G3TimeStamp.h>
#include <vector>

#include <pybindings.h>
#include <serialization.h>

#include <RfsocPacket.h>

class RfsocSample : public G3FrameObject {
public:
    RfsocSample(G3Time time, RfsocPacketPtr rp) :
        G3FrameObject(), time_(time), rp(rp) {}

    RfsocPacketPtr rp;

    // Returns G3Time for packet. If time can be determined from timing system
    // this will use that and the GetTimingParadigm will return HighPrecision.
    // If not, this will use the timestamp generated in software and
    // GetTimingParadigm will return LowPrecision. -- not implemented for RFSoC!
    const G3Time GetTime() const {
        return time_;
    }
    
    const uint16_t GetChannelCount() const {
        return __builtin_bswap16(rp->channel_count);
    }
    const uint32_t GetPacketCount() const {
        return __builtin_bswap32(rp->packet_count);
    }
    /* for further development to get array creation correct
    const std::vector<uint32_t> GetPtpIntArray() const { 
        std::vector<uint32_t> arr(3);
        arr[0] = __builtin_bswap32(rp->ptp_int_array[0]);
        arr[1] = __builtin_bswap32(rp->ptp_int_array[1]); 
        arr[2] = __builtin_bswap32(rp->ptp_int_array[2]); 
        //return { __builtin_bswap32(rp->ptp_int_array[0]), __builtin_bswap32(rp->ptp_int_array[1]), __builtin_bswap32(rp->ptp_int_array[2]) };
        return arr;
    }
    */


private:
    const G3Time time_;
};

G3_POINTERS(RfsocSample);

#endif

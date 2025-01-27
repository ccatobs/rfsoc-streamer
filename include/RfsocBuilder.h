#ifndef RFSOC_BUILDER_H
#define RFSOC_BUILDER_H
#include <unordered_map>
#include <deque>
#include <thread>

#include <G3EventBuilder.h>
#include <G3Logging.h>

#include "RfsocSample.h"

#define MAX_DATASOURCE_QUEUE_SIZE 3000

// Maximum total size of the write queue (channels * samples)
// Memory usage should be capped to around a few GB
#define MAX_BUILDER_QUEUE_SIZE 150000000 //Check if this make sense for us

class RfsocBuilder : public G3EventBuilder{
public:
    RfsocBuilder();
    ~RfsocBuilder();

    G3FramePtr FrameFromSamples(
            std::deque<RfsocSampleConstPtr>::iterator start,
            std::deque<RfsocSampleConstPtr>::iterator stop);

    float agg_duration_; // Aggregation duration in seconds
    void SetAggDuration(float dur){ agg_duration_ = dur;};
    const float GetAggDuration(){ return agg_duration_; };


protected:
    void ProcessNewData();

private:
    // Deques containing data sample pointers.
    std::deque<RfsocSampleConstPtr> write_stash_, read_stash_; 
    std::mutex write_stash_lock_, read_stash_lock_;

    uint16_t num_channels_;

    // Puts all stashed data in G3Frame and sends it out.
    void FlushStash();

    // Calls FlushStash every agg_duration_ seconds
    static void ProcessStashThread(RfsocBuilder *);

    // Converts raw_ptp_timestamp to utc 
    G3Time get_utc_from_ptp_int_array(uint32_t ptp_ints[3], int offset);
    double double_get_utc_from_ptp_int_array(uint32_t ptp_ints[3], int offset);

    // Sets SuperTimestream encoding algorithms:
    //   - 0: No compression
    //   - 1: FLAC only
    //   - 2: bzip only
    //   - 3: FLAC + bzip
    // See So3G docs for more info.
    int data_encode_algo_;
    int time_encode_algo_;
    int enable_compression_;
    int bz2_work_factor_;
    int flac_level_;
    size_t queue_size_;
    size_t dropped_frames_;

    bool running_;
    bool debug_;
    bool encode_timestreams_;

    std::thread process_stash_thread_;

    // Stores current output frame number
    uint32_t out_num_;
};

G3_POINTERS(RfsocBuilder);

#endif

/*
 * RfsocBuilder.cpp
 *
 * Implementation of G3EventBuilder abstract class
 * Takes packets from G3Pipeline queue, aggregates agg_duration
 * worth of data, and writes it into a frame that is passed
 * to the rest of the G3Pipeline with FrameOut.
 *
 * Modeled extensively off of the SO smurf-streamer's SmurfBuilder.cpp
 */

#define NO_IMPORT_ARRAY
#include "RfsocBuilder.h"

#include <G3Frame.h>
#include <G3Data.h>
#include <G3Timestream.h>
#include <G3Map.h>
#include <G3Timesample.h>
#include <G3Units.h>

#include <pybindings.h>                //from spt3g
//#include <boost/python.hpp>
//#include <boost/pointer_cast.hpp>
//#include <boost/shared_ptr.hpp>
//#include <boost/make_shared.hpp>
#include <container_pybindings.h>      //from spt3g

#include <chrono>
#include <string>
#include <thread>
#include <inttypes.h>
#include <G3SuperTimestream.h>         //from so3g

RfsocBuilder::RfsocBuilder() :
    G3EventBuilder(MAX_DATASOURCE_QUEUE_SIZE),
    out_num_(0), num_channels_(0),
    agg_duration_(3), debug_(false), encode_timestreams_(true),
    dropped_frames_(0), enable_compression_(1), data_encode_algo_(3),
    time_encode_algo_(2), bz2_work_factor_(1), flac_level_(3)
{
    process_stash_thread_ = std::thread(ProcessStashThread, this);
}

RfsocBuilder::~RfsocBuilder(){
    running_ = false;
    process_stash_thread_.join();
}

// Takes data from G3Pipeline queue and puts it into the write_stash
// Automatically called by the G3Pipeline as new data is available
void RfsocBuilder::ProcessNewData(){
    G3FrameObjectConstPtr pkt;
    G3TimeStamp ts;

    {
        std::lock_guard<std::mutex> lock(queue_lock_);
        ts = queue_.front().first;
        pkt = queue_.front().second;
        queue_.pop_front();
    }

    RfsocSampleConstPtr data_pkt;

    if (data_pkt = std::dynamic_pointer_cast<const RfsocSample>(pkt)){
        std::lock_guard<std::mutex> lock(write_stash_lock_);
        if (queue_size_ < MAX_BUILDER_QUEUE_SIZE){
            write_stash_.push_back(data_pkt);
            queue_size_ += 2048;
            //queue_size_ += data_pkt->sp->getHeader()->getNumberChannels(); //currently not working without full packet format
        }
        else{
            dropped_frames_++;
        }
    }
}

// Calls FlushStash every agg_duration_ seconds
void RfsocBuilder::ProcessStashThread(RfsocBuilder *builder){
    builder->running_ = true;

    while (builder->running_) {

        auto start = std::chrono::system_clock::now();
        builder->FlushStash();
        auto end = std::chrono::system_clock::now();

        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::chrono::milliseconds iter_time((int)(1000 * builder->agg_duration_));
        std::chrono::milliseconds sleep_time = iter_time - diff;

        if (sleep_time.count() > 0)
            std::this_thread::sleep_for(sleep_time);
    }
}

// Processes write_stash samples into a single Frame 
void RfsocBuilder::FlushStash(){
    // Swaps stashes
    std::lock_guard<std::mutex> read_lock(read_stash_lock_);
    {
        std::lock_guard<std::mutex> write_lock(write_stash_lock_);
        write_stash_.swap(read_stash_);
        queue_size_ = 0;
    }

    if (read_stash_.empty()){
        G3FramePtr frame = std::make_shared<G3Frame>();
        frame->Put("time", std::make_shared<G3Time>(G3Time::Now()));
        //There was a flow control bit here in SmurfBuilder
        FrameOut(frame);
        return;
    }

    auto start = read_stash_.begin();
    auto stop = read_stash_.begin();
    int nchans = __builtin_bswap16((*start)->rp->channel_count) * 2;
    while(true){
        stop += 1;
        if (stop == read_stash_.end()){
            FrameOut(FrameFromSamples(start, stop));
            break;
        }
        // There was more stuff here in SmurfBuilder to catch if the number of channels changed
        int stop_nchans = __builtin_bswap16((*stop)->rp->channel_count) * 2;        
        if (stop_nchans != nchans) {
            printf("NumChannels has changed from %d to %d!!", nchans, stop_nchans);
            // Do we just want to issue the warning, or carry out the remaining SmurfBuilder steps?
            // FrameOut(FrameFromSamples(start, stop));
            // start = stop;
            // nchans = stop_nchans;
        }
    }
    read_stash_.clear();
}

double RfsocBuilder::double_get_utc_from_ptp_int_array(uint32_t ptp_ints[3], int offset=-37)
{
  // ptp_int_array elements are read as little-endian, but need to be converted to big-endian
  // ptp_timestamp is in TAI -- need to convert to UTC
  // offset from TAI to UTC is currently -37 seconds
  
  uint32_t d0 = __builtin_bswap32(ptp_ints[0]);
  uint32_t d1 = __builtin_bswap32(ptp_ints[1]);
  uint32_t d2 = __builtin_bswap32(ptp_ints[2]);
  return int((d0 << 18) | (d1 >> 14)) + 
    int((((d1 & 0x00003FFF) << 16) | (d2 >> 16))) * 1e-9 + offset;
  //return int((d[-3] << 18) | (d[-2] >> 14)) + int((((d[-2] & 0x00003FFF) << 16) | (d[-1] >> 16))) * 1e-9 + offset;
}

G3Time RfsocBuilder::get_utc_from_ptp_int_array(uint32_t ptp_ints[3], int offset=-37)
{
  // ptp_int_array elements are read as little-endian, but need to be converted to big-endian
  // ptp_timestamp is in TAI -- need to convert to UTC
  // offset from TAI to UTC is currently -37 seconds
  
  uint32_t d0 = __builtin_bswap32(ptp_ints[0]);
  uint32_t d1 = __builtin_bswap32(ptp_ints[1]);
  uint32_t d2 = __builtin_bswap32(ptp_ints[2]);

  int64_t seconds = int((d0 << 18) | (d1 >> 14)) + offset;
  int64_t nanoseconds =  int((((d1 & 0x00003FFF) << 16) | (d2 >> 16)));
  int64_t microseconds = int(nanoseconds/1000);
  //return G3Time(seconds*G3TimeStamp(G3Units::s) + nanoseconds*G3TimeStamp(G3Units::ns));
  return G3Time(seconds*G3TimeStamp(G3Units::s) + microseconds*G3TimeStamp(G3Units::us));


  //return int((d[-3] << 18) | (d[-2] >> 14)) + int((((d[-2] & 0x00003FFF) << 16) | (d[-1] >> 16))) * 1e-9 + offset;
}


// Builds a frame from whatever number of samples were in the queue
G3FramePtr RfsocBuilder::FrameFromSamples(
        std::deque<RfsocSampleConstPtr>::iterator start,
        std::deque<RfsocSampleConstPtr>::iterator stop){

    uint16_t channel_count = __builtin_bswap16((*start)->rp->channel_count);
    uint32_t start_packet_count = __builtin_bswap32((*start)->rp->packet_count);

    int nsamps = stop - start;
    int nchans = channel_count * 2; // each channel contains I,Q -- so multiply by 2
    int nres = channel_count; // Total number of resonators for generating names

    // Initialize detector timestreams
    int32_t* data_buffer= (int32_t*) calloc(nchans * nsamps, sizeof(int32_t));

    int data_shape[2] = {nchans, nsamps};
    auto data_ts = G3SuperTimestreamPtr(new G3SuperTimestream());
    data_ts->names = G3VectorString();
    char name[16];
    for (int i = 0; i < nres; i++){
        snprintf(name, sizeof(name), "r%04d_I", i);
        data_ts->names.push_back(name);
        snprintf(name, sizeof(name), "r%04d_Q", i);
        data_ts->names.push_back(name);
    }

    G3VectorTime sample_times = G3VectorTime(nsamps);

    // Read data in to G3 Objects
    int sample = 0;
    uint32_t current_packet_count;
    for (auto it = start; it != stop; it++, sample++){
        sample_times[sample] = (*it)->GetTime();

        for (int i = 0; i < nchans; i++){
            // Data should already by stored as int32 in data vector in the packet
            //const int32_t *per_data = (*it)->rp_->data;
            //data_buffer[sample + i * nsamps] = per_data[i];
            data_buffer[sample + i * nsamps] = (*it)->rp->data[i];
        }
        current_packet_count = (*it)->rp->packet_count;
    }

    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    data_ts->times = sample_times;
    // SetDataFromBuffer is from so3g/src/G3SuperTimestream.cxx
    data_ts->SetDataFromBuffer((void*)data_buffer, 2, data_shape, NPY_INT32,
        std::pair<int,int>(0, nsamps));

    if (enable_compression_){
        data_ts->Options(
            enable_compression_, flac_level_, bz2_work_factor_, data_encode_algo_,
            time_encode_algo_
        );
    }
    else{
        data_ts->Options(enable_compression_, -1, -1, -1, -1);
    }

    if (encode_timestreams_){
        data_ts->Encode();
    }

    PyGILState_Release(gstate);
    free(data_buffer);

    //uint16_t channel_count = __builtin_bswap16((*start)->rp->channel_count);
    //uint32_t start_packet_count = __builtin_bswap32((*start)->rp->packet_count);
    // moved code to get channel_count and start_packet count to the top of this method
    uint32_t last_packet_count = __builtin_bswap32(current_packet_count);
    //uint32_t ptp_ints[3];
    //ptp_ints = (*start)->rp->ptp_int_array;
    uint32_t ptp_ints[3] = { ((*start)->rp->ptp_int_array)[0], ((*start)->rp->ptp_int_array)[1], ((*start)->rp->ptp_int_array)[2] };
    G3Time utc_from_ptp = RfsocBuilder::get_utc_from_ptp_int_array(ptp_ints);
    double double_utc_from_ptp = RfsocBuilder::double_get_utc_from_ptp_int_array(ptp_ints);

    //std::cout << (*start)->rp->channel_count << " " << __builtin_bswap16((*start)->rp->channel_count) << std::endl;
    //std::cout << (*start)->rp->packet_count << " " << __builtin_bswap32((*start)->rp->packet_count) << std::endl;

    uint8_t packet_flag_1 = ((*start)->rp->packet_info)[0];
    uint8_t packet_flag_2 = ((*start)->rp->packet_info)[1];
    //uint8_t packet_flag_1 = { ((*start)->rp->packet_info)[0] };
    //uint8_t packet_flag_2 = { ((*start)->rp->packet_info)[1] };

    // Create and return G3Frame
    G3FramePtr frame = std::make_shared<G3Frame>(G3Frame::Scan);
    frame->Put("time", std::make_shared<G3Time>(G3Time::Now()));
    frame->Put("channel_count", std::make_shared<G3Int>(channel_count));
    frame->Put("start_packet_count", std::make_shared<G3Int>(start_packet_count));
    frame->Put("last_packet_count", std::make_shared<G3Int>(last_packet_count));
    frame->Put("num_samples", std::make_shared<G3Int>(nsamps));
    frame->Put("utc_from_ptp", std::make_shared<G3Time>(utc_from_ptp));
    //frame->Put("double_utc_from_ptp", std::make_shared<G3Double>(double_utc_from_ptp));
    frame->Put("packet_flag_1", std::make_shared<G3Int>(packet_flag_1));
    frame->Put("packet_flag_2", std::make_shared<G3Int>(packet_flag_2));
    frame->Put("data", data_ts);

    return frame;
}



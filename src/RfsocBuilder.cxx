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

#include <pybindings.h>                //from spt3g
//#include <boost/python.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <container_pybindings.h>      //from spt3g

#include <chrono>
#include <string>
#include <thread>
#include <inttypes.h>
#include <G3SuperTimestream.h>         //from so3g

RfsocBuilder::RfsocBuilder() :
    G3EventBuilder(MAX_DATASOURCE_QUEUE_SIZE),
    out_num_(0), num_channels_(0),
    agg_duration_(3), debug_(false), encode_timestreams_(false),
    dropped_frames_(0)
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

    if (data_pkt = boost::dynamic_pointer_cast<const RfsocSample>(pkt)){
        std::lock_guard<std::mutex> lock(write_stash_lock_);
        if (queue_size_ < MAX_BUILDER_QUEUE_SIZE){
            write_stash_.push_back(data_pkt);
            queue_size_ += 1;
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
        G3FramePtr frame = boost::make_shared<G3Frame>();
        frame->Put("time", boost::make_shared<G3Time>(G3Time::Now()));
        //There was a flow control bit here in SmurfBuilder
        FrameOut(frame);
        return;
    }

    auto start = read_stash_.begin();
    auto stop = read_stash_.begin();
    while(true){
        stop += 1;
        if (stop == read_stash_.end()){
            FrameOut(FrameFromSamples(start, stop));
            break;
        }
        // There was more stuff here in SmurfBuilder to catch if the number of channels changed
    }
    read_stash_.clear();
}

// Builds a frame from whatever number of samples were in the queue
G3FramePtr RfsocBuilder::FrameFromSamples(
        std::deque<RfsocSampleConstPtr>::iterator start,
        std::deque<RfsocSampleConstPtr>::iterator stop){

    int nsamps = stop - start;
    int nchans = 1; // Eventually will get this from packet; for now, just take one channel with no channel name

    // Initialize detector timestreams
    int32_t* data_buffer= (int32_t*) calloc(nchans * nsamps, sizeof(int32_t));
 
    int data_shape[2] = {nchans, nsamps};
    auto data_ts = G3SuperTimestreamPtr(new G3SuperTimestream());
    data_ts->names = G3VectorString();
    for (int i = 0; i < nchans; i++){
        data_ts->names.push_back("test_channel");
    }

    G3VectorTime sample_times = G3VectorTime(nsamps);

    // Read data in to G3 Objects
    int sample = 0;
    for (auto it = start; it != stop; it++, sample++){
        sample_times[sample] = (*it)->GetTime();

        for (int i = 0; i < nchans; i++){
            // Just taking the I data point from a single resonator for now - will implement this for all
            // channels when packet is properly defined so that we can skip the header
            data_buffer[sample + i * nsamps] = (*it)->rp->buffer[16];
        }
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

    // Create and return G3Frame
    G3FramePtr frame = boost::make_shared<G3Frame>(G3Frame::Scan);
    frame->Put("time", boost::make_shared<G3Time>(G3Time::Now()));
    frame->Put("data", data_ts);  
    frame->Put("num_samples", boost::make_shared<G3Int>(nsamps));
    
    return frame;
}



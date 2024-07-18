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
        frame->Put("sostream_flowcontrol", boost::make_shared<G3Int>(FC_ALIVE));
        frame->Put("time", boost::make_shared<G3Time>(G3Time::Now()));
        FrameOut(frame);
        return;
    }

    auto start = read_stash_.begin();
    auto stop = read_stash_.begin();
    //int nchans = (*start)->sp->getHeader()->getNumberChannels(); //Not implemented for RFSoC yet
    while(true){
        stop += 1;
        if (stop == read_stash_.end()){
            FrameOut(FrameFromSamples(start, stop));
            break;
        }
       // Not implmented yet for RFSoC - handles if number of channels ever changes and writes a partial frame if so
       /* int stop_nchans = (*stop)->sp->getHeader()->getNumberChannels();
        *if (stop_nchans != nchans){
        *    printf("NumChannels has changed from %d to %d!!", nchans, stop_nchans);
        *    FrameOut(FrameFromSamples(start, stop));
        *    start = stop;
        *    nchans = stop_nchans;
        *}
        */
    }
    read_stash_.clear();
}

//Needs changed!!
//Takes data from G3Pipeline queue and puts it into the write_stash
void SmurfBuilder::ProcessNewData(){
    G3FrameObjectConstPtr pkt;
    G3TimeStamp ts;

    {
        std::lock_guard<std::mutex> lock(queue_lock_);
        ts = queue_.front().first;
        pkt = queue_.front().second;
        queue_.pop_front();
    }

    SmurfSampleConstPtr data_pkt; //Needs changed!!
    //StatusSampleConstPtr status_pkt; // Don't have for RFSoC now

    if (status_pkt = boost::dynamic_pointer_cast<const StatusSample>(pkt)){

        G3FramePtr frame(boost::make_shared<G3Frame>(G3Frame::Wiring));
        frame->Put("time", boost::make_shared<G3Time>(G3Time::Now()));
        frame->Put("status", boost::make_shared<G3String>(status_pkt->status_));

        FrameOut(frame);
    }

    else if (data_pkt = boost::dynamic_pointer_cast<const SmurfSample>(pkt)){
        std::lock_guard<std::mutex> lock(write_stash_lock_);
        if (queue_size_ < MAX_BUILDER_QUEUE_SIZE){
            write_stash_.push_back(data_pkt);
            queue_size_ += data_pkt->sp->getHeader()->getNumberChannels();
        }
        else{
            dropped_frames_++;
        }
    }
}

G3FramePtr SmurfBuilder::FrameFromSamples(
        std::deque<SmurfSampleConstPtr>::iterator start,
        std::deque<SmurfSampleConstPtr>::iterator stop){

    // Need to put packet info into arrays
    // Need to fix boost stuff//

    // Create and return G3Frame
    G3FramePtr frame = boost::make_shared<G3Frame>(G3Frame::Scan);
    frame->Put("time", boost::make_shared<G3Time>(G3Time::Now()));
    frame->Put("timing_paradigm",
               boost::make_shared<G3String>(TimestampTypeStrings[timing_type])
    );
    frame->Put("data", data_ts);
    frame->Put("tes_biases", tes_ts);
    frame->Put("num_samples", boost::make_shared<G3Int>(nsamps));

    frame->Put("primary", primary_ts);    
    
    return frame;
}



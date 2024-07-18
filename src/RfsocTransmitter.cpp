/*
 * RfsocTransmitter.cpp
 *
 * Code to take in packets and place them into G3Pipeline queue
 *
 * Modeled extensively off of the SO smurf-streamer's SmurfTransmitter.cpp
 */
 
#include "RfsocTransmitter.h"
#include "RfsocSample.h"
#include "RfsocBuilder.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <inttypes.h>


// Need to check what comes from BaseTransmitter() in smurf::core::transmitters
RfsocTransmitter::RfsocTransmitter(G3EventBuilderPtr builder) :
    sct::BaseTransmitter(), builder_(builder){}

RfsocTransmitter::~RfsocTransmitter(){}

void RfsocTransmitter::dataTransmit(RfsocPacketROPtr rp){

    G3Time ts = G3Time::Now();
    RfsocSamplePtr rfsoc_sample(new RfsocSample(ts, rp)); 
    builder_->AsyncDatum(ts.time, rfsoc_sample);
}

void printSmurfPacket(RfsocPacketROPtr rp){
    std::size_t numCh {rp->getHeader()->getNumberChannels()};

    std::cout << "=====================================" << std::endl;
    std::cout << "Packet received" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << std::endl;

    std::cout << "-----------------------" << std::endl;
    std::cout << " HEADER:" << std::endl;
    std::cout << "-----------------------" << std::endl;
    std::cout << "Version            = " << unsigned(rp->getHeader()->getVersion()) << std::endl;
    std::cout << "Crate ID           = " << unsigned(rp->getHeader()->getCrateID()) << std::endl;
    std::cout << "Slot number        = " << unsigned(rp->getHeader()->getSlotNumber()) << std::endl;
    std::cout << "Number of channels = " << unsigned(numCh) << std::endl;
    std::cout << "Unix time          = " << unsigned(rp->getHeader()->getUnixTime()) << std::endl;
    std::cout << "Frame counter      = " << unsigned(rp->getHeader()->getFrameCounter()) << std::endl;
    std::cout << std::endl;

    std::cout << std::endl;

    std::cout << "-----------------------" << std::endl;
    std::cout << " DATA (up to the first 5 points):" << std::endl;
    std::cout << "-----------------------" << std::endl;

    std::size_t n{5};
    if (numCh < n)
        n = numCh;

    for (std::size_t i(0); i < n; ++i)
            std::cout << "Data[" << i << "] = " << rp->getData(i) << std::endl;

        std::cout << "-----------------------" << std::endl;
        std::cout << std::endl;

        std::cout << "=====================================" << std::endl;
}

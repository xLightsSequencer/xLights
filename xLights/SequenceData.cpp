//  SequenceData.cpp
//  xLights
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cstddef>

#include "SequenceData.h"
#include <wx/wx.h>
#include <log4cpp/Category.hh>
#include "UtilFunctions.h"

const unsigned char FrameData::constzero = 0;

SequenceData::SequenceData() {
    data = nullptr;
    invalidData = nullptr;
    numFrames = 0;
    numChannels = 0;
    bytesPerFrame = 0;
    frameTime = 50;
}

SequenceData::~SequenceData() {
    if (data != nullptr) {
        free(data);
    }
    if (invalidData != nullptr) {
        free(invalidData);
    }
}

void SequenceData::init(unsigned int numChannels, unsigned int numFrames, unsigned int frameTime) {
    if (data != nullptr) {
        free(data);
        data = nullptr;
    }
    if (invalidData != nullptr) {
        free(invalidData);
        invalidData = nullptr;
    }
    this->numChannels = numChannels;
    this->numFrames = numFrames;
    this->frameTime = frameTime;
    this->bytesPerFrame = rountTo4(numChannels);

    if (numFrames > 0 && numChannels > 0) {
        //unsigned long tmp = bytesPerFrame;
        //tmp *= numFrames;
        //size_t sz = tmp;
        //wxASSERT((unsigned long)sz == tmp); // if this fails then we are asking for more memory than the system can address
        size_t sz = (size_t)bytesPerFrame * (size_t)numFrames;
        data = (unsigned char *)calloc(1, sz);
        wxASSERT(data != nullptr); // if this fails then we have a memory allocation error
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        if (data == nullptr)
        {
            logger_base.crit("Error allocating memory for frame data. Frames=%d, Channels=%d, Memory=%ld.", numFrames, numChannels, sz);
            logger_base.crit("***** THIS IS GOING TO CRASH *****");
        }
        else
        {
            logger_base.debug("Memory allocated for frame data. Frames=%d, Channels=%d, Memory=%ld.", numFrames, numChannels, sz);
        }
    }
    invalidData = (unsigned char *)calloc(1, bytesPerFrame);
}

FrameData SequenceData::operator[](unsigned int frame) {
    if (frame >= numFrames) {
        return FrameData(numChannels, invalidData);
    }
    std::ptrdiff_t offset = frame;
    offset *= bytesPerFrame;
    return FrameData(numChannels, &data[offset]);
}
const FrameData SequenceData::operator[](unsigned int frame) const {
    if (frame >= numFrames) {
        return FrameData(numChannels, invalidData);
    }
    std::ptrdiff_t offset = frame;
    offset *= bytesPerFrame;
    return FrameData(numChannels, &data[offset]);
}

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

const unsigned char FrameData::constzero = 0;

SequenceData::SequenceData() {
    data = NULL;
    invalidData = NULL;
    numFrames = 0;
    numChannels = 0;
    frameTime = 50;
}

SequenceData::~SequenceData() {
    if (data != NULL) {
        free(data);
    }
    if (invalidData != NULL) {
        free(invalidData);
    }
}

void SequenceData::init(unsigned int numChannels, unsigned int numFrames, unsigned int frameTime) {
    if (data != NULL) {
        free(data);
        data = NULL;
    }
    if (invalidData != NULL) {
        free(invalidData);
        invalidData = NULL;
    }
    this->numChannels = numChannels;
    this->numFrames = numFrames;
    this->frameTime = frameTime;

    if (numFrames > 0 && numChannels > 0) {
        unsigned long tmp = numChannels;
        tmp *= numFrames;
        size_t sz = tmp;
        wxASSERT((unsigned long)sz == tmp); // if this fails then we are asking for more memory than the system can address
        data = (unsigned char *)calloc(1, sz);
        wxASSERT(data != NULL); // if this fails then we have a memory allocation error
        log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        if (data == NULL)
        {
            logger_base.crit("Error allocating memory for frame data. Frames=%d, Channels=%d, Memory=%ld.", numFrames, numChannels, tmp);
        }
        else
        {
            logger_base.debug("Memory allocated for frame data. Frames=%d, Channels=%d, Memory=%ld.", numFrames, numChannels, tmp);
        }
    }
    invalidData = (unsigned char *)calloc(1, numChannels);
}

FrameData SequenceData::operator[](unsigned int frame) {
    if (frame >= numFrames) {
        return FrameData(numChannels, invalidData);
    }
    std::ptrdiff_t offset = frame;
    offset *= numChannels;
    return FrameData(numChannels, &data[offset]);
}
const FrameData SequenceData::operator[](unsigned int frame) const {
    if (frame >= numFrames) {
        return FrameData(numChannels, invalidData);
    }
    std::ptrdiff_t offset = frame;
    offset *= numChannels;
    return FrameData(numChannels, &data[offset]);
}
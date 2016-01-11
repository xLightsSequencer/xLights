//  SequenceData.cpp
//  xLights
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "SequenceData.h"

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
        data = (unsigned char *)calloc(1, sz);
    }
    invalidData = (unsigned char *)calloc(1, numChannels);
}

FrameData SequenceData::operator[](unsigned int frame) {
    if (frame >= numFrames) {
        return FrameData(numChannels, invalidData);
    }
    return FrameData(numChannels, &data[frame * numChannels]);
}
const FrameData SequenceData::operator[](unsigned int frame) const {
    if (frame >= numFrames) {
        return FrameData(numChannels, invalidData);
    }
    return FrameData(numChannels, &data[frame * numChannels]);
}
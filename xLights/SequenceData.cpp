//  SequenceData.cpp
//  xLights
//

#include <stdlib.h>
#include <string.h>

#include "SequenceData.h"

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

void SequenceData::init(int numChannels, int numFrames, int frameTime) {
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
        data = (unsigned char *)malloc(numChannels * numFrames);
        memset(data, 0, numChannels * numFrames);
    }
    invalidData = (unsigned char *)malloc(numChannels);
    memset(invalidData, 0, numChannels);
}

FrameData SequenceData::operator[](int frame) {
    if (frame >= numFrames) {
        return FrameData(numChannels, invalidData);
    }
    return FrameData(numChannels, &data[frame * numChannels]);
}

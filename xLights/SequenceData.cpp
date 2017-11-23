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

const unsigned char FrameData::_constzero = 0;

SequenceData::SequenceData() {
    _data = nullptr;
    _invalidData = nullptr;
    _numFrames = 0;
    _numChannels = 0;
    _bytesPerFrame = 0;
    _frameTime = 50;
}

SequenceData::~SequenceData() {
    if (_data != nullptr) {
        free(_data);
    }
    if (_invalidData != nullptr) {
        free(_invalidData);
    }
}

void SequenceData::init(unsigned int numChannels, unsigned int numFrames, unsigned int frameTime) {
    if (_data != nullptr) {
        free(_data);
        _data = nullptr;
    }
    if (_invalidData != nullptr) {
        free(_invalidData);
        _invalidData = nullptr;
    }
    _numChannels = roundTo4(numChannels);
    _numFrames = numFrames;
    _frameTime = frameTime;
    _bytesPerFrame = roundTo4(numChannels);

    if (numFrames > 0 && numChannels > 0) {
        //unsigned long tmp = bytesPerFrame;
        //tmp *= numFrames;
        //size_t sz = tmp;
        //wxASSERT((unsigned long)sz == tmp); // if this fails then we are asking for more memory than the system can address
        size_t sz = (size_t)_bytesPerFrame * (size_t)_numFrames;
        _data = (unsigned char *)calloc(1, sz);
        wxASSERT(_data != nullptr); // if this fails then we have a memory allocation error
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        if (_data == nullptr)
        {
            logger_base.crit("Error allocating memory for frame data. Frames=%d, Channels=%d, Memory=%ld.", _numFrames, _numChannels, sz);
            logger_base.crit("***** THIS IS GOING TO CRASH *****");
            wxString settings = wxString::Format("Frames=%d, Channels=%d, Memory=%ld.", _numFrames, _numChannels, sz);
            wxMessageBox("Bad news ... xLights is about to crash because it could not get memory it needed. If you are running 32 bit xLights then moving to 64 bit will probably fix this. Alternatively look to reduce memory usage by shortening sequences and/or reducing channels.\n" + settings, "xLights is about to crash");
        }
        else
        {
            logger_base.debug("Memory allocated for frame data. Frames=%d, Channels=%d, Memory=%ld.", _numFrames, _numChannels, sz);
        }
    }
    _invalidData = (unsigned char *)calloc(1, _bytesPerFrame);
}

FrameData SequenceData::operator[](unsigned int frame) {
    if (frame >= _numFrames) {
        return FrameData(_numChannels, _invalidData);
    }
    std::ptrdiff_t offset = frame;
    offset *= _bytesPerFrame;
    return FrameData(_numChannels, &_data[offset]);
}

const FrameData SequenceData::operator[](unsigned int frame) const {
    if (frame >= _numFrames) {
        return FrameData(_numChannels, _invalidData);
    }
    std::ptrdiff_t offset = frame;
    offset *= _bytesPerFrame;
    return FrameData(_numChannels, &_data[offset]);
}

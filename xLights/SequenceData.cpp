//  SequenceData.cpp
//  xLights

//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>
//#include <cstddef>

#include <wx/wx.h>

#include "SequenceData.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>

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

void SequenceData::init(unsigned int numChannels, unsigned int numFrames, unsigned int frameTime, bool roundto4) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_data != nullptr) {
        free(_data);
        _data = nullptr;
    }
    if (_invalidData != nullptr) {
        free(_invalidData);
        _invalidData = nullptr;
    }
    if (roundto4)
    {
        _numChannels = roundTo4(numChannels);
    }
    else
    {
        _numChannels = numChannels;
    }
    _numFrames = numFrames;
    _frameTime = frameTime;
    _bytesPerFrame = roundTo4(numChannels);

    if (numFrames > 0 && numChannels > 0) {
        size_t sz = (size_t)_bytesPerFrame * (size_t)_numFrames;
        _data = (unsigned char *)calloc(1, sz);
        wxASSERT(_data != nullptr); // if this fails then we have a memory allocation error
        if (_data == nullptr)
        {
            logger_base.crit("Error allocating memory for frame data. Frames=%d, Channels=%d, Memory=%ld.", _numFrames, _numChannels, sz);
            logger_base.crit("***** THIS IS GOING TO CRASH *****");
            wxString settings = wxString::Format("Frames=%d, Channels=%d, Memory=%ld.", _numFrames, _numChannels, sz);
            DisplayError("Bad news ... xLights is about to crash because it could not get memory it needed. If you are running 32 bit xLights then moving to 64 bit will probably fix this. Alternatively look to reduce memory usage by shortening sequences and/or reducing channels.\n" + settings);
        }
        else
        {
            logger_base.debug("Memory allocated for frame data. Frames=%d, Channels=%d, Memory=%ld.", _numFrames, _numChannels, sz);
        }
    }
    else
    {
        logger_base.debug("Sequence memory released.");
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

// This encodes the sequence data grouped by channel
wxString SequenceData::base64_encode()
{
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    wxString ret;
    int i = 0;

    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (size_t channel = 0; channel < NumChannels(); channel++) {
        for (size_t frame = 0; frame < NumFrames(); frame++) {
            char_array_3[i++] = (*this)[frame][channel];
            if (i == 3)
            {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; (i <4); i++)
                {
                    ret += base64_chars[char_array_4[i]];
                }
                i = 0;
            }
        }
    }

    if (i)
    {
        for (int j = i; j < 3; j++)
        {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int j = 0; (j < i + 1); j++)
        {
            ret += base64_chars[char_array_4[j]];
        }

        while ((i++ < 3))
        {
            ret += '=';
        }

    }
    return ret;
}

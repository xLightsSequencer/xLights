//
//  SequenceData.h
//  xLights
//
//  Created by Daniel Kulp on 1/12/15.
//  Copyright (c) 2015 Daniel Kulp. All rights reserved.
//

#ifndef SEQUENCEDATA_H
#define SEQUENCEDATA_H

#include <wx/wx.h>

class FrameData {
    static const unsigned char _constzero;
    unsigned char _zero;
    unsigned int _numChannels;
    unsigned char* _data;

public:
    void Zero()
    {
        memset(_data, 0x00, _numChannels);
    }
    FrameData(unsigned int nc, unsigned char *d) {
        _numChannels = nc;
        _data = d;
        _zero = 0;
    }
    
    unsigned char &operator[](unsigned int channel) {
        wxASSERT(_zero == 0);
        return channel < _numChannels ? _data[channel] : _zero;
    }
    
    const unsigned char *operator[](unsigned int channel) const {
        const unsigned char* cdata = _data;
        return channel < _numChannels ? &cdata[channel] : &_constzero;
    }
};

class SequenceData {
    unsigned char *_invalidData;
    unsigned char *_data;
    unsigned int _bytesPerFrame;
    unsigned int _numChannels;
    unsigned int _numFrames;
    unsigned int _frameTime;

    SequenceData(const SequenceData&);  //make sure we cannot "copy" these
    SequenceData &operator=(const SequenceData& rgb);

public:
    SequenceData();
    virtual ~SequenceData();
    
    void init(unsigned int numChannels, unsigned int numFrames, unsigned int frameTime, bool roundto4 = true);
    unsigned int TotalTime() const { return _numFrames * _frameTime; }
    bool OK(unsigned int frame, unsigned int channel) const { return frame < _numFrames && channel < _numChannels; }
    
    FrameData operator[](unsigned int frame);
    const FrameData operator[](unsigned int frame) const;
    
    unsigned int NumChannels() const { return _numChannels;}
    unsigned int NumFrames() const { return _numFrames;}
    unsigned int FrameTime() const { return _frameTime;}
    bool IsValidData() const { return _data != nullptr; }

    // encodes contents of SeqData in channel order
    wxString base64_encode();
};
#endif

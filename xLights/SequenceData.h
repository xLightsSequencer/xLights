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
#include <memory>

class FrameData {
    FrameData(const FrameData&) = delete;
    FrameData &operator=(const FrameData& d) = delete;
    
    static const unsigned char _constzero;
    unsigned char _zero;
    unsigned int _numChannels;
    unsigned char* _data;
    friend class SequenceData;
    
    FrameData() : _zero(0), _numChannels(0), _data(nullptr) {}
    FrameData(unsigned int nc, unsigned char *d) : _zero(0), _numChannels(nc), _data(d) {}

public:
    FrameData(const FrameData && d) : _zero(d._zero), _numChannels(d._numChannels), _data(d._data) {}
    
    void Zero() {
        memset(_data, 0x00, _numChannels);
    }
    void Zero(unsigned int start, unsigned int count) {
        if (start < 0) return;
        if (count < 1) return;
        if (start + count > _numChannels) return;
        memset(&_data[start], 0x00, count);
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
    enum class BlockType {
        NORMAL,
        HUGE_PAGE
    };
    class DataBlock {
        DataBlock(const DataBlock&d) = delete;
        DataBlock &operator=(const DataBlock& d) = delete;
    public:
        DataBlock(size_t s, unsigned char *d, BlockType bt) : size(s), data(d), type(bt) {}
        DataBlock(DataBlock&&d) : size(d.size), data(d.data), type(d.type) {d.data = nullptr; };
        ~DataBlock();
        
        unsigned char * data;
        size_t size;
        BlockType type;
    };
    static std::list<std::unique_ptr<DataBlock>> HUGE_BLOCK_CACHE;
    
    FrameData _invalidFrame;
    std::vector<FrameData> _frames;
    std::list<std::unique_ptr<DataBlock>> _dataBlocks;
    bool _hugePagesFailed;
    
    unsigned int _bytesPerFrame;
    unsigned int _numChannels;
    unsigned int _numFrames;
    unsigned int _frameTime;

    SequenceData(const SequenceData&) = delete;  //make sure we cannot "copy" these
    SequenceData &operator=(const SequenceData& rgb) = delete;

    void Cleanup();
    unsigned char *AllocBlock(size_t requested, size_t &szAllocated);
public:
    SequenceData();
    virtual ~SequenceData();
    
    void init(unsigned int numChannels, unsigned int numFrames, unsigned int frameTime, bool roundto4 = true);
    unsigned int TotalTime() const { return _numFrames * _frameTime; }
    bool OK(unsigned int frame, unsigned int channel) const { return frame < _numFrames && channel < _numChannels; }
    
    FrameData &operator[](unsigned int frame) {
        if (frame >= _numFrames) {
            return _invalidFrame;
        }
        return _frames[frame];
    }
    const FrameData &operator[](unsigned int frame) const {
        if (frame >= _numFrames) {
            return _invalidFrame;
        }
        return _frames[frame];
    }
    
    unsigned int NumChannels() const { return _numChannels;}
    unsigned int NumFrames() const { return _numFrames;}
    unsigned int FrameTime() const { return _frameTime;}
    bool IsValidData() const { return !_dataBlocks.empty(); }

    // encodes contents of SeqData in channel order
    wxString base64_encode();
};
#endif

#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <memory>
#include <mutex>

#ifdef __WXOSX__
#include <sys/mman.h>
#include <mach/vm_statistics.h>
#define USE_MMAP_BLOCKS
#elif defined(LINUX)
#include <sys/mman.h>
#define USE_MMAP_BLOCKS
#else
//Windows
#endif

class SequenceData {
public:
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
        FrameData(const FrameData && d) noexcept : _zero(d._zero), _numChannels(d._numChannels), _data(d._data) {}
        
        void Zero() {
            memset(_data, 0x00, _numChannels);
        }

        void Zero(unsigned int start, unsigned int count) {
            if (_data == nullptr) return;
            if (start < 0) return;
            if (count < 1) return;
            if (start + count > _numChannels) return;
            memset(&_data[start], 0x00, count);
        }
        
        [[nodiscard]] unsigned char& operator[](unsigned int channel)
        {
            wxASSERT(_zero == 0);
            return channel < _numChannels ? _data[channel] : _zero;
        }
        
        [[nodiscard]] const unsigned char* operator[](unsigned int channel) const
        {
            const unsigned char* cdata = _data;
            return channel < _numChannels ? &cdata[channel] : &_constzero;
        }
    };

    enum class BlockType {
        NORMAL,
        HUGE_PAGE
    };
    class DataBlock {
        DataBlock(const DataBlock&d) = delete;
        DataBlock &operator=(const DataBlock& d) = delete;
    public:
        DataBlock(size_t s, unsigned char *d, BlockType bt) : size(s), data(d), type(bt) {}
        DataBlock(DataBlock&&d) noexcept : size(d.size), data(d.data), type(d.type) {d.data = nullptr; d.size = 0; d.type = BlockType::NORMAL;};
        ~DataBlock();
        
        unsigned char * data;
        size_t size;
        BlockType type;
    };
#ifdef USE_MMAP_BLOCKS
    static std::list<std::unique_ptr<DataBlock>> HUGE_BLOCK_CACHE;
#endif    
    FrameData _invalidFrame;
    std::vector<FrameData> _frames;
    std::list<std::unique_ptr<DataBlock>> _dataBlocks;
    
    unsigned int _bytesPerFrame;
    unsigned int _numChannels;
    unsigned int _numFrames;
    unsigned int _frameTime;

    SequenceData(const SequenceData&) = delete;  //make sure we cannot "copy" these
    SequenceData &operator=(const SequenceData& rgb) = delete;

    void Cleanup();
    unsigned char *checkBlockPtr(unsigned char *block, size_t sizeRemaining);
    static unsigned char *AllocBlock(size_t requested, size_t &szAllocated, BlockType &bt);
public:
    SequenceData();
    virtual ~SequenceData();
    
    void init(unsigned int numChannels, unsigned int numFrames, unsigned int frameTime, bool roundto4 = true);
    unsigned int TotalTime() const { return _numFrames * _frameTime; }
    bool OK(unsigned int frame, unsigned int channel) const { return frame < _numFrames && channel < _numChannels; }
    
    [[nodiscard]] FrameData& operator[](unsigned int frame)
    {
        if (frame >= _numFrames) {
            return _invalidFrame;
        }
        return _frames[frame];
    }
    [[nodiscard]] const FrameData& operator[](unsigned int frame) const
    {
        if (frame >= _numFrames) {
            return _invalidFrame;
        }
        return _frames[frame];
    }
    
    [[nodiscard]] unsigned int NumChannels() const
    {
        return _numChannels;
    }
    [[nodiscard]] unsigned int NumFrames() const
    {
        return _numFrames;
    }
    [[nodiscard]] unsigned int FrameTime() const
    {
        return _frameTime;
    }
    [[nodiscard]] bool IsValidData() const
    {
        return !_dataBlocks.empty();
    }

    // encodes contents of SeqData in channel order
    [[nodiscard]] wxString base64_encode();
};

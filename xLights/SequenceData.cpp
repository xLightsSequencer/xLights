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

#include "SequenceData.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>

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

const unsigned char FrameData::_constzero = 0;

SequenceData::SequenceData() : _invalidFrame()
{
    _numFrames = 0;
    _numChannels = 0;
    _bytesPerFrame = 0;
    _frameTime = 50;
}

SequenceData::~SequenceData()
{
    Cleanup();
}

std::list<std::unique_ptr<SequenceData::DataBlock>> SequenceData::HUGE_BLOCK_CACHE;

SequenceData::DataBlock::~DataBlock()
{
    if (data) {
#ifdef USE_MMAP_BLOCKS
        munmap(data, size);
#else
        free(data);
#endif
    }
}

void SequenceData::Cleanup()
{
    _frames.clear();
    for (auto& p : _dataBlocks) {
        if (p.get() && p.get()->type == BlockType::HUGE_PAGE) {
            //save these for later, HUGE_PAGE blocks are limitted and hard to come by
            //so if we get any, we'll hold onto them
            HUGE_BLOCK_CACHE.emplace_back(std::move(p));
        }
    }
    _dataBlocks.clear();
    _invalidFrame._numChannels = 0;
    free(_invalidFrame._data);
    _invalidFrame._data = nullptr;
}

unsigned char* SequenceData::AllocBlock(size_t requested, size_t& szAllocated)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    unsigned char* data = nullptr;
    size_t sz = requested;
    BlockType type = BlockType::NORMAL;
#ifdef USE_MMAP_BLOCKS
    // OSX/Linux allows 2MB large pages (or Superpages as they call them on OSX)
    static const size_t LARGE_PAGE_SIZE = 2 * 1024 * 1024;
    // max on OSX is 2GB block for the superpage sizes
    static const size_t MAX_SP_BLOCK_SIZE = LARGE_PAGE_SIZE * 1024;
    static const size_t MAX_BLOCK_SIZE = 1024 * 1024 * 1024;
    if (sz > MAX_SP_BLOCK_SIZE) {
        sz = MAX_SP_BLOCK_SIZE;
    }
    else {
        sz = sz - (sz % LARGE_PAGE_SIZE) + LARGE_PAGE_SIZE;
    }
    if (!HUGE_BLOCK_CACHE.empty()) {
        std::unique_ptr<DataBlock> d = std::move(HUGE_BLOCK_CACHE.front());
        HUGE_BLOCK_CACHE.pop_front();
        data = d.get()->data;
        sz = d.get()->size;
        memset(data, 0, sz);
        szAllocated = sz;
        _dataBlocks.push_back(std::move(d));
        return data;
    }
    else if (!_hugePagesFailed) {
#ifdef __WXOSX__
        type = BlockType::HUGE_PAGE;
        data = (unsigned char*)mmap(nullptr, sz,
            PROT_READ | PROT_WRITE,
            MAP_ANON | MAP_PRIVATE,
            VM_FLAGS_SUPERPAGE_SIZE_2MB, 0);
        if (data == MAP_FAILED && (sz > 256 * 1024 * 1024)) {
            data = (unsigned char*)mmap(nullptr, 256 * 1024 * 1024,
                PROT_READ | PROT_WRITE,
                MAP_ANON | MAP_PRIVATE,
                VM_FLAGS_SUPERPAGE_SIZE_2MB, 0);
            if (data != MAP_FAILED) {
                sz = 256 * 1024 * 1024;
            }
            else {
                _hugePagesFailed = true;
            }
        }
#else
        data = (unsigned char*)mmap(nullptr, sz,
            PROT_READ | PROT_WRITE,
            MAP_ANON | MAP_PRIVATE | MAP_HUGETLB,
            -1, 0);
#endif
    }
    if (data == nullptr || data == MAP_FAILED) {
        _hugePagesFailed = true;
        type = BlockType::NORMAL;
        if (sz > MAX_BLOCK_SIZE) {
            sz = MAX_BLOCK_SIZE;
        }
        //could not get a superpage, we'll use the regular 4K pages
        data = (unsigned char*)mmap(nullptr, sz,
            PROT_READ | PROT_WRITE,
            MAP_ANON | MAP_PRIVATE,
            -1, 0);
        if (data == nullptr || data == MAP_FAILED) {
            //could not allocate the block, we'll try a 128MB block
            if (sz > 1024 * 1024 * 128) {
                sz = 1024 * 1024 * 128;
                data = (unsigned char*)mmap(nullptr, sz,
                    PROT_READ | PROT_WRITE,
                    MAP_ANON | MAP_PRIVATE,
                    -1, 0);
            }
        }
        if (data == nullptr || data == MAP_FAILED) {
            data = nullptr;
        }
#ifdef LINUX
        if (data) {
            // let the transparent hugepage daemon know it can/should promote to
            // huge page if at all possible
            madvise(data, sz, MADV_HUGEPAGE);
        }
#endif
    }
#else
    // we'll keep the callocs below 1GB in size.  Should keep pressure off
    // the VM to find a huge block of space, but still not waste much
    // memory.  Most users sequences will likely fit in this anyway
    static const size_t MAX_BLOCK_SIZE = 1024 * 1024 * 1024;
    if (sz > MAX_BLOCK_SIZE) {
        sz = MAX_BLOCK_SIZE;
    }
    data = (unsigned char*)calloc(1, sz);
    if (data == nullptr) {
        //could not allocate the block, we'll try a 128MB block
        if (sz > 1024 * 1024 * 128) {
            sz = 1024 * 1024 * 128;
            data = (unsigned char*)calloc(1, sz);
        }
        if (data == nullptr) {
            //still could not allocate the block, we'll try a 32MB block
            if (sz > 1024 * 1024 * 32) {
                sz = 1024 * 1024 * 32;
                data = (unsigned char*)calloc(1, sz);
            }
        }
    }
#endif
    wxASSERT(data != nullptr); // if this fails then we have a memory allocation error
    if (data == nullptr) {
        logger_base.crit("Error allocating memory for frame data. Frames=%d, Channels=%d, Memory=%ld.", _numFrames, _numChannels, sz);
        logger_base.crit("***** THIS IS GOING TO CRASH *****");
        wxString settings = wxString::Format("Frames=%d, Channels=%d, Memory=%ld.", _numFrames, _numChannels, sz);
        DisplayError("Bad news ... xLights is about to crash because it could not get memory it needed. If you are running 32 bit xLights then moving to 64 bit will probably fix this. Alternatively look to reduce memory usage by shortening sequences and/or reducing channels.\n" + settings);
    }
    else {
        logger_base.debug("Memory allocated for frame data. Block=%d, Frames=%d, Channels=%d, Memory=%ld.", _dataBlocks.size(), _numFrames, _numChannels, sz);
    }

    szAllocated = sz;
    _dataBlocks.push_back(std::make_unique<DataBlock>(sz, data, type));
    return data;
}

void SequenceData::init(unsigned int numChannels, unsigned int numFrames, unsigned int frameTime, bool roundto4)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    Cleanup();
    _hugePagesFailed = false;
    if (roundto4) {
        _numChannels = roundTo4(numChannels);
    }
    else {
        _numChannels = numChannels;
    }
    _numFrames = numFrames;
    _frameTime = frameTime;
    _bytesPerFrame = roundTo4(numChannels);

    if (numFrames > 0 && numChannels > 0) {
        _frames.reserve(numFrames);
        size_t sizeRemaining = (size_t)_bytesPerFrame * (size_t)_numFrames;
        size_t blockSize = 0;
        unsigned char* block = AllocBlock(sizeRemaining, blockSize);

        for (unsigned int frame = 0; frame < numFrames; ++frame) {
            if (blockSize < _bytesPerFrame) {
                block = AllocBlock(sizeRemaining, blockSize);
            }
            _frames.push_back(FrameData(_numChannels, block));
            block += _bytesPerFrame;
            sizeRemaining -= _bytesPerFrame;
            blockSize -= _bytesPerFrame;
        }
    }
    else {
        logger_base.debug("Sequence memory released.");
    }
    _invalidFrame._data = (unsigned char*)calloc(1, _bytesPerFrame);
    _invalidFrame._numChannels = _numChannels;
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
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; (i < 4); i++) {
                    ret += base64_chars[char_array_4[i]];
                }
                i = 0;
            }
        }
    }

    if (i) {
        for (int j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int j = 0; (j < i + 1); j++) {
            ret += base64_chars[char_array_4[j]];
        }

        while ((i++ < 3)) {
            ret += '=';
        }

    }
    return ret;
}

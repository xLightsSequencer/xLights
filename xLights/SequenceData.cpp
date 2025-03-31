/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>


#include <log4cpp/Category.hh>


#include "../common/xlBaseApp.h"
#include "SequenceData.h"
#include "UtilFunctions.h"

const unsigned char SequenceData::FrameData::_constzero = 0;


// we'll keep the callocs below 1GB in size.  Should keep pressure off
// the VM to find a huge block of space, but still not waste much
// memory.  Most users sequences will likely fit in this anyway
static const size_t MAX_BLOCK_SIZE = 1024 * 1024 * 1024;

#ifdef USE_MMAP_BLOCKS
std::list<std::unique_ptr<SequenceData::DataBlock>> SequenceData::HUGE_BLOCK_CACHE;
#include <thread>
// OSX/Linux allows 2MB huge pages (or Superpages as they call them on OSX)
static const size_t LARGE_PAGE_SIZE = 2 * 1024 * 1024;
static bool firstSeq = true;
static std::mutex HUGE_BLOCK_LOCK;
#ifdef __WXOSX__
static const size_t DEFAULT_HUGE_ALLOC_SIZE = 64 * 1024 * 1024;
#else
static const size_t DEFAULT_HUGE_ALLOC_SIZE = MAX_BLOCK_SIZE;
#endif
static size_t _hugePageAllocSize = DEFAULT_HUGE_ALLOC_SIZE;
static bool _hugePagesFailed;
#endif

SequenceData::SequenceData() : _invalidFrame()
{
#ifdef USE_MMAP_BLOCKS
    if (firstSeq) {
        firstSeq = false;
        std::thread([]{
            try
            {
                xlCrashHandler::SetupCrashHandlerForNonWxThread();

                _hugePageAllocSize = 32 * 1024 * 1024;
                size_t blockSize = 0;
                BlockType type = BlockType::NORMAL;
                unsigned char* block = AllocBlock(32 * 1024 * 1024, blockSize, type);
                if (type == BlockType::HUGE_PAGE) {
                    _hugePageAllocSize = DEFAULT_HUGE_ALLOC_SIZE;
                    std::unique_lock<std::mutex> lock(HUGE_BLOCK_LOCK);
                    HUGE_BLOCK_CACHE.push_back(std::make_unique<DataBlock>(blockSize, block, type));
                    lock.unlock();
                } else {
                    // couldn't even get a small block, don't waste time trying larger blocks
                    munmap(block, blockSize);
                    return;
                }
                
                size_t sizeRemaining = MAX_BLOCK_SIZE;
                block = AllocBlock(sizeRemaining, blockSize, type);
                while (block) {
                    if (type == BlockType::HUGE_PAGE) {
                        std::unique_lock<std::mutex> lock(HUGE_BLOCK_LOCK);
                        HUGE_BLOCK_CACHE.push_back(std::make_unique<DataBlock>(blockSize, block, type));
                        lock.unlock();
                        if (blockSize > sizeRemaining) {
                            sizeRemaining -= blockSize;
                            block = AllocBlock(sizeRemaining, blockSize, type);
                        } else {
                            block = nullptr;
                        }
                    } else {
                        munmap(block, blockSize);
                        block = nullptr;
                    }
                }
            }
            catch (...)
            {
                wxTheApp->OnUnhandledException();
            }
        }).detach();
    }
#endif
    _numFrames = 0;
    _numChannels = 0;
    _bytesPerFrame = 0;
    _frameTime = 50;
}

SequenceData::~SequenceData()
{
    Cleanup();
}

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
#ifdef USE_MMAP_BLOCKS
    for (auto& p : _dataBlocks) {
        if (p.get() && p.get()->type == BlockType::HUGE_PAGE) {
            //save these for later, HUGE_PAGE blocks are limited and hard to come by
            //so if we get any, we'll hold onto them
            std::unique_lock<std::mutex> lock(HUGE_BLOCK_LOCK);
            HUGE_BLOCK_CACHE.emplace_back(std::move(p));
        }
    }
#endif

    _dataBlocks.clear();
    _invalidFrame._numChannels = 0;
    free(_invalidFrame._data);
    _invalidFrame._data = nullptr;
}

unsigned char* SequenceData::AllocBlock(size_t requested, size_t& szAllocated, BlockType &blockType)
{
    unsigned char* data = nullptr;
    size_t sz = requested;
    blockType = BlockType::NORMAL;
#ifdef USE_MMAP_BLOCKS
    if (sz > MAX_BLOCK_SIZE) {
        sz = MAX_BLOCK_SIZE;
    } else {
        sz = sz - (sz % LARGE_PAGE_SIZE) + LARGE_PAGE_SIZE;
    }
    std::unique_lock<std::mutex> lock(HUGE_BLOCK_LOCK);
    if (!HUGE_BLOCK_CACHE.empty()) {
        std::unique_ptr<DataBlock> d = std::move(HUGE_BLOCK_CACHE.front());
        HUGE_BLOCK_CACHE.pop_front();
        data = d.get()->data;
        sz = d.get()->size;
        blockType = d.get()->type;
        szAllocated = sz;
        memset(data, 0, sz);
        d.get()->data = nullptr;
        return data;
    }
    lock.unlock();
    if (!_hugePagesFailed) {
#ifdef __WXOSX__
        wxStopWatch sw;
        //size_t origSize = sz;
        blockType = BlockType::HUGE_PAGE;
        size_t szToAlloc = sz;
        if (szToAlloc > _hugePageAllocSize) {
            szToAlloc = _hugePageAllocSize;
        }
        data = (unsigned char*)mmap(nullptr, szToAlloc,
            PROT_READ | PROT_WRITE,
            MAP_ANON | MAP_PRIVATE,
            VM_FLAGS_SUPERPAGE_SIZE_2MB, 0);
        if (data != MAP_FAILED) {
            sz = szToAlloc;
        }
        while (data == MAP_FAILED && (_hugePageAllocSize >= 16 * 1024 * 1024) && (sw.Time() < 1500)) {
            _hugePageAllocSize /= 2;
            data = (unsigned char*)mmap(nullptr, _hugePageAllocSize,
                PROT_READ | PROT_WRITE,
                MAP_ANON | MAP_PRIVATE,
                VM_FLAGS_SUPERPAGE_SIZE_2MB, 0);
            if (data != MAP_FAILED) {
                sz = _hugePageAllocSize;
            }
        }
        if (data == MAP_FAILED) {
            data = nullptr;
            _hugePagesFailed = true;
        }
        if (sw.Time() > 2000) {
            //if its taking more than 2 seconds to get a huge block, we'll stop as we're obviously having memory constraints
            _hugePagesFailed = true;
        }
        //printf("OK:  %d    Fail:  %d   Size %zu     Requested:  %zu     Time:  %d\n", data != nullptr, _hugePagesFailed, sz, origSize, sw.Time() );
#else
        data = (unsigned char*)mmap(nullptr, sz,
            PROT_READ | PROT_WRITE,

            MAP_ANON | MAP_PRIVATE | MAP_HUGETLB,
            -1, 0);
#endif
    }
    if (data == nullptr || data == MAP_FAILED) {
        _hugePagesFailed = true;
        blockType = BlockType::NORMAL;
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

    szAllocated = sz;
    return data;
}

unsigned char *SequenceData::checkBlockPtr(unsigned char *block, size_t sizeRemaining) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxASSERT(block != nullptr); // if this fails then we have a memory allocation error
    if (block == nullptr) {
        logger_base.crit("Error allocating memory for frame data. Frames=%d, Channels=%d, Memory=%zu.", _numFrames, _numChannels, sizeRemaining);
        logger_base.crit("***** THIS IS GOING TO CRASH *****");
        wxString settings = wxString::Format("Frames=%d, Channels=%d, Memory=%ld.", _numFrames, _numChannels, sizeRemaining);
        DisplayError("Bad news ... xLights is about to crash because it could not get memory it needed. If you are running 32 bit xLights then moving to 64 bit will probably fix this. Alternatively look to reduce memory usage by shortening sequences and/or reducing channels.\n" + settings);
    } else {
        logger_base.debug("Memory allocated for frame data. Block=%d, Frames=%d, Channels=%d, Memory=%zu.", _dataBlocks.size(), _numFrames, _numChannels, sizeRemaining);
    }
    return block;
}

void SequenceData::init(unsigned int numChannels, unsigned int numFrames, unsigned int frameTime, bool roundto4)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    Cleanup();
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
        
        BlockType type = BlockType::NORMAL;
        unsigned char* block = checkBlockPtr(AllocBlock(sizeRemaining, blockSize, type), sizeRemaining);
        _dataBlocks.push_back(std::make_unique<DataBlock>(blockSize, block, type));
        
        for (unsigned int frame = 0; frame < numFrames; ++frame) {
            if (blockSize < _bytesPerFrame) {
                block = checkBlockPtr(AllocBlock(sizeRemaining, blockSize, type), sizeRemaining);
                _dataBlocks.push_back(std::make_unique<DataBlock>(blockSize, block, type));
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

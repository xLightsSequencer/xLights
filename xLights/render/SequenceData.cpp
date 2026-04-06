/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cassert>
#include <chrono>

#include <log.h>

#include "utils/Base64.h"
#include "render/SequenceData.h"
#include "UtilFunctions.h"
#include "utils/AppCallbacks.h"

#ifdef USE_MMAP_BLOCKS
#include <unistd.h>
#include <sys/mman.h>
#endif
#ifdef __APPLE__
#include <mach/vm_statistics.h>
#include <sys/sysctl.h>
#include <TargetConditionals.h>
#endif


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
#ifdef __APPLE__
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
            try {
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
                // Huge page pre-allocation failed — release any blocks we
                // managed to map and mark huge pages as failed so AllocBlock
                // falls back to normal calloc allocations.
                std::unique_lock<std::mutex> lock(HUGE_BLOCK_LOCK);
                HUGE_BLOCK_CACHE.clear(); // ~DataBlock calls munmap
                lock.unlock();
                _hugePagesFailed = true;
            }
        }).detach();
    }
#endif
    _numFrames = 0;
    _numChannels = 0;
    _bytesPerFrame = 0;
    _frameTime = 50;
}

bool SequenceData::ShouldUseFileBacked(size_t totalSize) {
#if TARGET_OS_IOS
    return true; // always on iPad — jetsam is the constraint
#elif defined(__APPLE__)
    // Use file-backed if sequence would consume >50% of physical memory
    int mib[2] = { CTL_HW, HW_MEMSIZE };
    uint64_t physMem = 0;
    size_t len = sizeof(physMem);
    sysctl(mib, 2, &physMem, &len, nullptr, 0);
    return physMem > 0 && totalSize > (physMem / 2);
#else
    return false; // Linux/Windows: keep current behavior
#endif
}

SequenceData::~SequenceData()
{
    Cleanup();
}

SequenceData::DataBlock::~DataBlock()
{
    if (data) {
#ifdef USE_MMAP_BLOCKS
        munmap(data, size); // works for both anonymous, huge page, and file-backed mappings
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
        // FILE_BACKED blocks are just munmap'd normally (via ~DataBlock) —
        // the unlinked temp file disappears automatically
    }
#endif

    _dataBlocks.clear();
    _invalidFrame._numChannels = 0;
    free(_invalidFrame._data);
    _invalidFrame._data = nullptr;
}

unsigned char* SequenceData::AllocBlock(size_t requested, size_t& szAllocated, BlockType &blockType, bool fileBacked)
{
    unsigned char* data = nullptr;
    size_t sz = requested;
    blockType = BlockType::NORMAL;
#ifdef USE_MMAP_BLOCKS
    if (fileBacked) {
        if (sz > MAX_BLOCK_SIZE) {
            sz = MAX_BLOCK_SIZE;
        }
        // Create a temporary file, mmap it, then unlink so it auto-cleans on munmap
        std::string tmpPath = std::string(std::getenv("TMPDIR") ? std::getenv("TMPDIR") : "/tmp") + "/xlseqXXXXXX";
        int fd = mkstemp(tmpPath.data());
        if (fd >= 0) {
            unlink(tmpPath.c_str()); // unlink immediately — file disappears when mapping is destroyed
            if (ftruncate(fd, sz) == 0) {
                data = (unsigned char*)mmap(nullptr, sz,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, 0);
                if (data == MAP_FAILED) {
                    data = nullptr;
                } else {
                    blockType = BlockType::FILE_BACKED;
                    madvise(data, sz, MADV_SEQUENTIAL);
                }
            }
            close(fd);
        }
        if (data) {
            szAllocated = sz;
            return data;
        }
        // file-backed failed — fall through to anonymous mmap
        spdlog::warn("File-backed mmap failed for {} bytes, falling back to anonymous mmap", sz);
    }
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
#ifdef __APPLE__
        auto sw_start = std::chrono::steady_clock::now();
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
        while (data == MAP_FAILED && (_hugePageAllocSize >= 16 * 1024 * 1024) && (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - sw_start).count() < 1500)) {
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
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - sw_start).count() > 2000) {
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
    
    assert(block != nullptr); // if this fails then we have a memory allocation error
    if (block == nullptr) {
        spdlog::critical("Error allocating memory for frame data. Frames={}, Channels={}, Memory={}.", _numFrames, _numChannels, sizeRemaining);
        spdlog::critical("***** THIS IS GOING TO CRASH *****");
        std::string settings = "Frames=" + std::to_string(_numFrames) + ", Channels=" + std::to_string(_numChannels) + ", Memory=" + std::to_string(sizeRemaining) + ".";
        DisplayError("Bad news ... xLights is about to crash because it could not get memory it needed. If you are running 32 bit xLights then moving to 64 bit will probably fix this. Alternatively look to reduce memory usage by shortening sequences and/or reducing channels.\n" + settings);
    } else {
        spdlog::debug("Memory allocated for frame data. Block={}, Frames={}, Channels={}, Memory={}.", _dataBlocks.size(), _numFrames, _numChannels, sizeRemaining);
    }
    return block;
}

void SequenceData::init(unsigned int numChannels, unsigned int numFrames, unsigned int frameTime, bool roundto4)
{
    
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
        bool fileBacked = ShouldUseFileBacked(sizeRemaining);
        if (fileBacked) {
            spdlog::debug("SequenceData using file-backed mmap for {} bytes ({} frames, {} channels)",
                         sizeRemaining, _numFrames, _numChannels);
        }

        BlockType type = BlockType::NORMAL;
        unsigned char* block = checkBlockPtr(AllocBlock(sizeRemaining, blockSize, type, fileBacked), sizeRemaining);
        _dataBlocks.push_back(std::make_unique<DataBlock>(blockSize, block, type));

        for (unsigned int frame = 0; frame < numFrames; ++frame) {
            if (blockSize < _bytesPerFrame) {
                block = checkBlockPtr(AllocBlock(sizeRemaining, blockSize, type, fileBacked), sizeRemaining);
                _dataBlocks.push_back(std::make_unique<DataBlock>(blockSize, block, type));
            }
            _frames.push_back(FrameData(_numChannels, block));
            block += _bytesPerFrame;
            sizeRemaining -= _bytesPerFrame;
            blockSize -= _bytesPerFrame;
        }
    }
    else {
        spdlog::debug("Sequence memory released.");
    }
    _invalidFrame._data = (unsigned char*)calloc(1, _bytesPerFrame);
    _invalidFrame._numChannels = _numChannels;
}



// This encodes the sequence data grouped by channel
std::string SequenceData::base64_encode()
{
    std::vector<uint8_t> data;
    data.reserve(NumChannels() * NumFrames());
    for (size_t channel = 0; channel < NumChannels(); channel++) {
        for (size_t frame = 0; frame < NumFrames(); frame++) {
            data.push_back((*this)[frame][channel]);
            }
        }
    return Base64::Encode(data.data(), data.size());
}

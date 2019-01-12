// This #define must be before any #include's
#define _FILE_OFFSET_BITS 64
#define __STDC_FORMAT_MACROS


#include <vector>
#include <cstring>
#include <memory>

#include <stdio.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#include <wx/wx.h>
int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970
    static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((uint64_t)file_time.dwLowDateTime);
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}
#define ftello _ftelli64
#define fseeko _fseeki64
#define NO_ZLIB

#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "FSEQFile.h"

#if defined(PLATFORM_PI) || defined(PLATFORM_BBB) || defined(PLATFORM_ODROID) || defined(PLATFORM_ORANGEPI) || defined(PLATFORM_UNKNOWN)
//for FPP, use FPP logging
#include "log.h"
#else
//compiling within xLights, use log4cpp
#define PLATFORM_UNKNOWN
#include <log4cpp/Category.hh>
template<typename... Args> static void LogErr(int i, const char *fmt, Args... args) {
    static log4cpp::Category &fseq_logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    fseq_logger_base.error(fmt, args...);
}
template<typename... Args> static void LogInfo(int i, const char *fmt, Args... args) {
    static log4cpp::Category &fseq_logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    fseq_logger_base.info(fmt, args...);
}
template<typename... Args> static void LogDebug(int i, const char *fmt, Args... args) {
    static log4cpp::Category &fseq_logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    fseq_logger_base.debug(fmt, args...);
}
#define VB_SEQUENCE 1
#define VB_ALL 0
#endif


#ifndef NO_ZSTD
#include <zstd.h>
#endif
#ifndef NO_ZLIB
#include <zlib.h>
#endif

using FrameData = FSEQFile::FrameData;

inline void DumpHeader(const char *title, unsigned char data[], int len) {
    int x = 0;
    char tmpStr[128];

    sprintf( tmpStr, "%s: (%d bytes)\n", title, len);
    LogInfo(VB_ALL, tmpStr);

    for (int y = 0; y < len; y++) {
        if ( x == 0 ) {
            sprintf( tmpStr, "%06x: ", y);
        }
        sprintf( tmpStr + strlen(tmpStr), "%02x ", (int)(data[y] & 0xFF) );
        x++;
        if (x == 16) {
            x = 0;
            sprintf( tmpStr + strlen(tmpStr), "\n" );
            LogInfo(VB_ALL, tmpStr);
        }
    }
    if (x != 0) {
        sprintf( tmpStr + strlen(tmpStr), "\n" );
        LogInfo(VB_ALL, tmpStr);
    }
}

inline uint64_t GetTime(void) {
    struct timeval now_tv;
    gettimeofday(&now_tv, NULL);
    return now_tv.tv_sec * 1000000LL + now_tv.tv_usec;
}

inline long roundTo4(long i) {
    long remainder = i % 4;
    if (remainder == 0) {
        return i;
    }
    return i + 4 - remainder;
}

inline uint32_t read4ByteUInt(const uint8_t* data) {
    uint32_t r = (data[0])
        + (data[1] << 8)
        + (data[2] << 16)
        + (data[3] << 24);
    return r;
}
inline uint32_t read3ByteUInt(const uint8_t* data) {
    uint32_t r = (data[0])
        + (data[1] << 8)
        + (data[2] << 16);
    return r;
}
inline uint32_t read2ByteUInt(const uint8_t* data) {
    uint32_t r = (data[0])
        + (data[1] << 8);
    return r;
}
inline void write2ByteUInt(uint8_t* data, uint32_t v) {
    data[0] = (uint8_t)(v & 0xFF);
    data[1] = (uint8_t)((v >> 8) & 0xFF);
}
inline void write3ByteUInt(uint8_t* data, uint32_t v) {
    data[0] = (uint8_t)(v & 0xFF);
    data[1] = (uint8_t)((v >> 8) & 0xFF);
    data[2] = (uint8_t)((v >> 16) & 0xFF);
}
inline void write4ByteUInt(uint8_t* data, uint32_t v) {
    data[0] = (uint8_t)(v & 0xFF);
    data[1] = (uint8_t)((v >> 8) & 0xFF);
    data[2] = (uint8_t)((v >> 16) & 0xFF);
    data[3] = (uint8_t)((v >> 24) & 0xFF);
}

FSEQFile* FSEQFile::openFSEQFile(const std::string &fn) {

    FILE *seqFile = fopen((const char *)fn.c_str(), "rb");
    if (seqFile == NULL) {
        LogErr(VB_SEQUENCE, "Error opening sequence file: %s. fopen returned NULL\n",
               fn.c_str());
        return nullptr;
    }

    fseeko(seqFile, 0L, SEEK_SET);
    unsigned char tmpData[48];
    int bytesRead = fread(tmpData, 1, 48, seqFile);
#ifndef PLATFORM_UNKNOWN
    posix_fadvise(fileno(seqFile), 0, 0, POSIX_FADV_RANDOM);
    posix_fadvise(fileno(seqFile), 0, 1024*1024, POSIX_FADV_WILLNEED);
#endif

    if ((bytesRead < 4)
        || (tmpData[0] != 'P' && tmpData[0] != 'F' && tmpData[0] != 'E')
        || tmpData[1] != 'S'
        || tmpData[2] != 'E'
        || tmpData[3] != 'Q') {
        LogErr(VB_SEQUENCE, "Error opening sequence file: %s. Incorrect File Format header: '%s', bytesRead: %d\n",
               fn.c_str(), tmpData, bytesRead);
        DumpHeader("Sequence File head:", tmpData, bytesRead);
        fclose(seqFile);
        return nullptr;
    }
    if (bytesRead < 8) {
        LogErr(VB_SEQUENCE, "Sequence file %s too short, unable to read FSEQ version fields\n", fn.c_str());
        DumpHeader("Sequence File head:", tmpData, bytesRead);
        fclose(seqFile);
        return nullptr;
    }
    int seqVersionMinor = tmpData[6];
    int seqVersionMajor = tmpData[7];

    ///////////////////////////////////////////////////////////////////////
    // Get Channel Data Offset
    uint64_t seqChanDataOffset = read2ByteUInt(&tmpData[4]);
    
    if (tmpData[0] == 'E') {
        //v1 eseq file.  This is basically an uncompressed v2 file with a custom header
        seqChanDataOffset = 20;
        seqVersionMajor = 2;
        seqVersionMinor = 0;
    }
    std::vector<uint8_t> header(seqChanDataOffset);
    fseeko(seqFile, 0L, SEEK_SET);
    bytesRead = fread(&header[0], 1, seqChanDataOffset, seqFile);
    if (bytesRead != seqChanDataOffset) {
        LogErr(VB_SEQUENCE, "Error opening sequence file: %s. Could not read header.\n", fn.c_str());
        DumpHeader("Sequence File head:", &header[0], bytesRead);
        fclose(seqFile);
        return nullptr;
    }

    FSEQFile *file = nullptr;
    if (seqVersionMajor == 1) {
        file = new V1FSEQFile(fn, seqFile, header);
    } else if (seqVersionMajor == 2) {
        file = new V2FSEQFile(fn, seqFile, header);
    } else {
        LogErr(VB_SEQUENCE, "Error opening sequence file: %s. Unknown FSEQ version %d-%d\n",
               fn.c_str(), seqVersionMajor, seqVersionMinor);
        DumpHeader("Sequence File head:", tmpData, bytesRead);
        fclose(seqFile);
        return nullptr;
    }
    file->dumpInfo();
    return file;
}
FSEQFile* FSEQFile::createFSEQFile(const std::string &fn,
                                   int version,
                                   CompressionType ct,
                                   int level) {
    if (version == 1) {
        return new V1FSEQFile(fn);
    }
    return new V2FSEQFile(fn, ct, level);
}
std::string FSEQFile::getMediaFilename(const std::string &fn) {
    std::unique_ptr<FSEQFile> file(FSEQFile::openFSEQFile(fn));
    if (file) {
        return file->getMediaFilename();
    }
    return "";
}
std::string FSEQFile::getMediaFilename() const {
    for (auto &a : m_variableHeaders) {
        if (a.code[0] == 'm' && a.code[1] == 'f') {
            const char *d = (const char *)&a.data[0];
            return d;
        }
    }
    return "";
}

FSEQFile::FSEQFile(const std::string &fn)
    : m_filename(fn),
    m_seqNumFrames(0),
    m_seqChannelCount(0),
    m_seqStepTime(50),
    m_variableHeaders(),
    m_uniqueId(0),
    m_seqFileSize(0),
    m_seqVersionMajor(1),
    m_seqVersionMinor(0),
    m_memoryBuffer(),
    m_seqChanDataOffset(0),
    m_memoryBufferPos(0)
{
    if (fn == "-memory-") {
        m_seqFile = nullptr;
        m_memoryBuffer.reserve(1024*1024);
    } else {
        m_seqFile = fopen((const char *)fn.c_str(), "wb");
    }
}

void FSEQFile::dumpInfo(bool indent) {
    char ind[5] = "    ";
    if (!indent) {
        ind[0] = 0;
    }
    LogDebug(VB_SEQUENCE, "%sSequence File Information\n", ind);
    LogDebug(VB_SEQUENCE, "%sseqFilename           : %s\n", ind, m_filename.c_str());
    LogDebug(VB_SEQUENCE, "%sseqVersion            : %d.%d\n", ind, m_seqVersionMajor, m_seqVersionMinor);
    LogDebug(VB_SEQUENCE, "%sseqChanDataOffset     : %d\n", ind, m_seqChanDataOffset);
    LogDebug(VB_SEQUENCE, "%sseqChannelCount       : %d\n", ind, m_seqChannelCount);
    LogDebug(VB_SEQUENCE, "%sseqNumPeriods         : %d\n", ind, m_seqNumFrames);
    LogDebug(VB_SEQUENCE, "%sseqStepTime           : %dms\n", ind, m_seqStepTime);
}


void FSEQFile::initializeFromFSEQ(const FSEQFile& fseq) {
    m_seqNumFrames = fseq.m_seqNumFrames;
    m_seqChannelCount = fseq.m_seqChannelCount;
    m_seqStepTime = fseq.m_seqStepTime;
    m_variableHeaders = fseq.m_variableHeaders;
    m_uniqueId = fseq.m_uniqueId;
}


FSEQFile::FSEQFile(const std::string &fn, FILE *file, const std::vector<uint8_t> &header)
    : m_filename(fn),
    m_seqFile(file),
    m_uniqueId(0),
    m_memoryBuffer(),
    m_memoryBufferPos(0)
{
    fseeko(m_seqFile, 0L, SEEK_END);
    m_seqFileSize = ftello(m_seqFile);
    fseeko(m_seqFile, 0L, SEEK_SET);

    if (header[0] == 'E') {
        m_seqChanDataOffset = 20;
        m_seqVersionMinor = 0;
        m_seqVersionMajor = 2;
        m_seqChannelCount = read4ByteUInt(&header[8]);
        m_seqStepTime = 50;
        m_seqNumFrames = (m_seqFileSize - 20) / m_seqChannelCount;
    } else {
        m_seqChanDataOffset = read2ByteUInt(&header[4]);
        m_seqVersionMinor = header[6];
        m_seqVersionMajor = header[7];
        m_seqChannelCount = read4ByteUInt(&header[10]);
        m_seqNumFrames = read4ByteUInt(&header[14]);
        m_seqStepTime = header[18];
    }
}
FSEQFile::~FSEQFile() {
    if (m_seqFile) {
        fclose(m_seqFile);
    }
}

int FSEQFile::seek(uint64_t location, int origin) {
    if (m_seqFile) {
        return fseeko(m_seqFile, location, origin);
    } else if (origin == SEEK_SET) {
        m_memoryBufferPos = location;
    } else if (origin == SEEK_CUR) {
        m_memoryBufferPos += location;
    } else if (origin == SEEK_END) {
        m_memoryBufferPos = m_memoryBuffer.size();
    }
    return 0;
}

uint64_t FSEQFile::tell() {
    if (m_seqFile) {
       return ftello(m_seqFile);
    }
    return m_memoryBufferPos;
}

uint64_t FSEQFile::write(const void * ptr, uint64_t size) {
    if (m_seqFile) {
        return fwrite(ptr, 1, size, m_seqFile);
    }
    if ((m_memoryBufferPos + size) > m_memoryBuffer.size()) {
        m_memoryBuffer.resize(m_memoryBufferPos + size);
    }
    memcpy(&m_memoryBuffer[m_memoryBufferPos], ptr, size);
    m_memoryBufferPos += size;
    return size;
}

uint64_t FSEQFile::read(void *ptr, uint64_t size) {
    return fread(ptr, 1, size, m_seqFile);
}

void FSEQFile::preload(uint64_t pos, uint64_t size) {
#ifndef PLATFORM_UNKNOWN
    posix_fadvise(fileno(m_seqFile), pos, size, POSIX_FADV_WILLNEED);
#endif
}

void FSEQFile::parseVariableHeaders(const std::vector<uint8_t> &header, int start) {
    while (start < header.size() - 5) {
        int len = read2ByteUInt(&header[start]);
        if (len) {
            VariableHeader vheader;
            vheader.code[0] = header[start + 2];
            vheader.code[1] = header[start + 3];
            vheader.data.resize(len - 4);
            memcpy(&vheader.data[0], &header[start + 4], len - 4);
            m_variableHeaders.push_back(vheader);
        } else {
            len += 4;
        }
        start += len;
    }
}
void FSEQFile::finalize() {
    fflush(m_seqFile);
}


V1FSEQFile::V1FSEQFile(const std::string &fn)
  : FSEQFile(fn), m_dataBlockSize(0)
{
}

void V1FSEQFile::writeHeader() {
    static int fixedHeaderLength = 28;
    uint8_t header[28];
    memset(header, 0, 28);
    header[0] = 'P';
    header[1] = 'S';
    header[2] = 'E';
    header[3] = 'Q';

    // data offset
    uint32_t dataOffset = fixedHeaderLength;
    for (auto &a : m_variableHeaders) {
        dataOffset += a.data.size() + 4;
    }
    dataOffset = roundTo4(dataOffset);
    write2ByteUInt(&header[4], dataOffset);

    header[6] = 0; //minor
    header[7] = 1; //major
    m_seqVersionMinor = header[6];
    m_seqVersionMajor = header[7];
    // Fixed header length
    write2ByteUInt(&header[8], fixedHeaderLength);
    // Step Size
    write4ByteUInt(&header[10], m_seqChannelCount);
    // Number of Steps
    write4ByteUInt(&header[14], m_seqNumFrames);
    // Step time in ms
    header[18] = m_seqStepTime;
    //flags
    header[19] = 0;
    // universe count
    write2ByteUInt(&header[20], 0);
    // universe Size
    write2ByteUInt(&header[22], 0);
    // universe Size
    header[24] = 1;
    // color order
    header[25] = 2;
    header[26] = 0;
    header[27] = 0;
    write(header, 28);
    for (auto &a : m_variableHeaders) {
        uint8_t buf[4];
        uint32_t len = a.data.size() + 4;
        write2ByteUInt(buf, len);
        buf[2] = a.code[0];
        buf[3] = a.code[1];
        write(buf, 4);
        write(&a.data[0], a.data.size());
    }
    uint64_t pos = tell();
#ifdef _MSC_VER
    wxASSERT(pos <= dataOffset); // i dont see how this could be wrong but seeing some crashes
#endif
    if (pos != dataOffset) {
        char buf[4] = {0,0,0,0};
#ifdef _MSC_VER
        wxASSERT(dataOffset - pos <= 4); // i dont see how this could be wrong but seeing some crashes
#endif
        write(buf, dataOffset - pos);
    }
    m_seqChanDataOffset = dataOffset;
    dumpInfo(false);
}

V1FSEQFile::V1FSEQFile(const std::string &fn, FILE *file, const std::vector<uint8_t> &header)
: FSEQFile(fn, file, header) {

    // m_seqNumUniverses = (header[20])       + (header[21] << 8);
    // m_seqUniverseSize = (header[22])       + (header[23] << 8);
    // m_seqGamma         = header[24];
    // m_seqColorEncoding = header[25];

    // 0 = header[26]
    // 0 = header[27]
    parseVariableHeaders(header, 28);

    //use the last modified time for the uniqueId
    struct stat stats;
    fstat(fileno(file), &stats);
    m_uniqueId = stats.st_mtime;
}

V1FSEQFile::~V1FSEQFile() {

}

class UncompressedFrameData : public FSEQFile::FrameData {
public:
    UncompressedFrameData(uint32_t frame,
                          uint32_t sz,
                          const std::vector<std::pair<uint32_t, uint32_t>> &ranges)
    : FrameData(frame), m_ranges(ranges) {
        m_data = (uint8_t*)malloc(sz);
    }
    virtual ~UncompressedFrameData() {
        free(m_data);
    }

    virtual void readFrame(uint8_t *data) {
        uint32_t offset = 0;
        for (auto &rng : m_ranges) {
            uint32_t toRead = rng.second;
            memcpy(&data[rng.first], &m_data[offset], toRead);
            offset += toRead;
        }
    }

    uint8_t *m_data;
    std::vector<std::pair<uint32_t, uint32_t>> m_ranges;
};
void V1FSEQFile::prepareRead(const std::vector<std::pair<uint32_t, uint32_t>> &ranges) {
    m_rangesToRead = ranges;
    m_dataBlockSize = 0;
    for (auto &rng : m_rangesToRead) {
        //make sure we don't read beyond the end of the sequence data
        int toRead = rng.second;
        if ((rng.first + toRead) > m_seqChannelCount) {
            toRead = m_seqChannelCount - rng.first;
            rng.second = toRead;
        }
        m_dataBlockSize += toRead;
    }
    FrameData *f = getFrame(0);
    if (f) {
        delete f;
    }
}

FrameData *V1FSEQFile::getFrame(uint32_t frame) {
    if (m_rangesToRead.empty()) {
        std::vector<std::pair<uint32_t, uint32_t>> range;
        range.push_back(std::pair<uint32_t, uint32_t>(0, m_seqChannelCount));
        prepareRead(range);
    }
    uint64_t offset = m_seqChannelCount;
    offset *= frame;
    offset += m_seqChanDataOffset;

    UncompressedFrameData *data = new UncompressedFrameData(frame, m_dataBlockSize, m_rangesToRead);
    if (seek(offset, SEEK_SET)) {
        LogErr(VB_SEQUENCE, "Failed to seek to proper offset for channel data for frame %d! %" PRIu64 "\n", frame, offset);
        return data;
    }
    uint32_t sz = 0;
    //read the ranges into the buffer
    for (auto &rng : data->m_ranges) {
        if (rng.first < m_seqChannelCount) {
            int toRead = rng.second;
            uint64_t doffset = offset;
            doffset += rng.first;
            seek(doffset, SEEK_SET);
            size_t bread = read(&data->m_data[sz], toRead);
            if (bread != toRead) {
                LogErr(VB_SEQUENCE, "Failed to read channel data for frame %d!   Needed to read %d but read %d\n",
                       frame, toRead, (int)bread);
            }
            sz += toRead;
        }
    }
    return data;
}

void V1FSEQFile::addFrame(uint32_t frame,
                          const uint8_t *data) {
    write(data, m_seqChannelCount);
}

void V1FSEQFile::finalize() {
    FSEQFile::finalize();
}

uint32_t V1FSEQFile::getMaxChannel() const {
    return m_seqChannelCount;
}

static const int V2FSEQ_HEADER_SIZE = 32;
#if !defined(NO_ZLIB) || !defined(NO_ZSTD)
static const int V2FSEQ_OUT_BUFFER_SIZE = 1024*1024; //1M output buffer
static const int V2FSEQ_OUT_BUFFER_FLUSH_SIZE = 900 * 1024; //90% full, flush it
#endif

class V2Handler {
public:
    V2Handler(V2FSEQFile *f)
        : m_file(f)
    {
        m_seqChanDataOffset = f->m_seqChanDataOffset;
    }
    virtual ~V2Handler() {}


    virtual uint8_t getCompressionType() = 0;
    virtual FrameData *getFrame(uint32_t frame) = 0;

    virtual uint32_t computeMaxBlocks() = 0;
    virtual void addFrame(uint32_t frame, const uint8_t *data) = 0;
    virtual void finalize() = 0;

    int seek(uint64_t location, int origin) {
        return m_file->seek(location, origin);
    }
    uint64_t tell() {
        return m_file->tell();
    }
    uint64_t write(const void * ptr, uint64_t size) {
        return m_file->write(ptr, size);
    }
    uint64_t read(void *ptr, uint64_t size) {
        return m_file->read(ptr, size);
    }
    void preload(uint64_t pos, uint64_t size) {
        m_file->preload(pos, size);
    }

    V2FSEQFile *m_file;
    uint64_t   m_seqChanDataOffset;
};

class V2NoneCompressionHandler : public V2Handler {
public:
    V2NoneCompressionHandler(V2FSEQFile *f) : V2Handler(f) {}
    virtual ~V2NoneCompressionHandler() {}

    virtual uint8_t getCompressionType() override { return 0;}
    virtual uint32_t computeMaxBlocks() override {return 0;}

    virtual FrameData *getFrame(uint32_t frame) override {
        UncompressedFrameData *data = new UncompressedFrameData(frame, m_file->m_dataBlockSize, m_file->m_rangesToRead);
        uint64_t offset = m_file->getChannelCount();
        offset *= frame;
        offset += m_seqChanDataOffset;
        if (seek(offset, SEEK_SET)) {
            LogErr(VB_SEQUENCE, "Failed to seek to proper offset for channel data! %" PRIu64 "\n", offset);
            return data;
        }
        if (m_file->m_sparseRanges.empty()) {
            uint32_t sz = 0;
            //read the ranges into the buffer
            for (auto &rng : data->m_ranges) {
                if (rng.first < m_file->getChannelCount()) {
                    int toRead = rng.second;
                    uint64_t doffset = offset;
                    doffset += rng.first;
                    seek(doffset, SEEK_SET);
                    size_t bread = read(&data->m_data[sz], toRead);
                    if (bread != toRead) {
                        LogErr(VB_SEQUENCE, "Failed to read channel data!   Needed to read %d but read %d\n", toRead, (int)bread);
                    }
                    sz += toRead;
                }
            }
        } else {
            size_t bread = read(data->m_data, m_file->m_dataBlockSize);
            if (bread != m_file->m_dataBlockSize) {
                LogErr(VB_SEQUENCE, "Failed to read channel data!   Needed to read %d but read %d\n", m_file->m_dataBlockSize, (int)bread);
            }
        }
        return data;
    }
    virtual void addFrame(uint32_t frame, const uint8_t *data) override {
        if (m_file->m_sparseRanges.empty()) {
            write(data, m_file->getChannelCount());
        } else {
            for (auto &a : m_file->m_sparseRanges) {
                write(&data[a.first], a.second);
            }
        }
    }

    virtual void finalize() override {}

};
class V2CompressedHandler : public V2Handler {
public:
    V2CompressedHandler(V2FSEQFile *f) : V2Handler(f), m_maxBlocks(0), m_curBlock(99999), m_framesPerBlock(0), m_curFrameInBlock(0) {
        if (!m_file->m_frameOffsets.empty()) {
            m_maxBlocks = m_file->m_frameOffsets.size() - 1;
        }
    }
    virtual ~V2CompressedHandler() {}

    virtual uint32_t computeMaxBlocks() override {
        if (m_maxBlocks > 0) {
            return m_maxBlocks;
        }
        //determine a good number of compression blocks
        uint64_t datasize = m_file->getChannelCount() * m_file->getNumFrames();
        uint64_t numBlocks = datasize;
        numBlocks /= (64*2014); //at least 64K per block
        if (numBlocks > 255) {
            //need a lot of blocks, use as many as we can
            numBlocks = 255;
        } else if (numBlocks < 1) {
            numBlocks = 1;
        }
        m_framesPerBlock = m_file->getNumFrames() / numBlocks;
        if (m_framesPerBlock < 10) m_framesPerBlock = 10;
        m_curFrameInBlock = 0;
        m_curBlock = 0;

        numBlocks = m_file->getNumFrames() / m_framesPerBlock + 1;
        while (numBlocks > 255) {
            m_framesPerBlock++;
            numBlocks = m_file->getNumFrames() / m_framesPerBlock + 1;
        }
        // first block is going to be smaller, so add some blocks
        if (numBlocks < 254) {
            numBlocks += 2;
        } else if (numBlocks < 255) {
            numBlocks++;
        }
        m_maxBlocks = numBlocks;
        m_curBlock = 0;
        return m_maxBlocks;
    }

    virtual void finalize() override {
        uint64_t curr = tell();
        uint64_t off = V2FSEQ_HEADER_SIZE;
        seek(off, SEEK_SET);
        int count = m_file->m_frameOffsets.size();
        m_file->m_frameOffsets.push_back(std::pair<uint32_t, uint64_t>(99999999, curr));
        for (int x = 0 ; x < count; x++) {
            uint8_t buf[8];
            uint32_t frame = m_file->m_frameOffsets[x].first;
            write4ByteUInt(buf,frame);

            uint64_t len64 = m_file->m_frameOffsets[x + 1].second;
            len64 -= m_file->m_frameOffsets[x].second;
            uint32_t len = len64;
            write4ByteUInt(&buf[4], len);
            write(buf, 8);
            //printf("%d    %d: %d\n", x, frame, len);
        }
        m_file->m_frameOffsets.pop_back();
        seek(curr, SEEK_SET);
    }


    // for compressed files, this is the compression data
    uint32_t m_framesPerBlock;
    uint32_t m_curFrameInBlock;
    uint32_t m_curBlock;
    uint32_t m_maxBlocks;
};

#ifndef NO_ZSTD
class V2ZSTDCompressionHandler : public V2CompressedHandler {
public:
    V2ZSTDCompressionHandler(V2FSEQFile *f) : V2CompressedHandler(f),
    m_cctx(nullptr),
    m_dctx(nullptr)
    {
        m_outBuffer.pos = 0;
        m_outBuffer.size = V2FSEQ_OUT_BUFFER_SIZE;
        m_outBuffer.dst = malloc(m_outBuffer.size);
        m_inBuffer.src = nullptr;
        m_inBuffer.size = 0;
        m_inBuffer.pos = 0;
    }
    virtual ~V2ZSTDCompressionHandler() {
        free(m_outBuffer.dst);
        if (m_inBuffer.src != nullptr) {
            free((void*)m_inBuffer.src);
        }
        if (m_dctx) {
            ZSTD_freeCStream(m_cctx);
        }
        if (m_dctx) {
            ZSTD_freeDStream(m_dctx);
        }
    }
    virtual uint8_t getCompressionType() override { return 1;}

    virtual FrameData *getFrame(uint32_t frame) override {
        if (m_curBlock > 256 || (frame < m_file->m_frameOffsets[m_curBlock].first) || (frame >= m_file->m_frameOffsets[m_curBlock + 1].first)) {
            //frame is not in the current block
            m_curBlock = 0;
            while (frame >= m_file->m_frameOffsets[m_curBlock + 1].first) {
                m_curBlock++;
            }
            if (m_dctx == nullptr) {
                m_dctx = ZSTD_createDStream();
            }
            ZSTD_initDStream(m_dctx);
            seek(m_file->m_frameOffsets[m_curBlock].second, SEEK_SET);

            uint64_t len = m_file->m_frameOffsets[m_curBlock + 1].second;
            len -= m_file->m_frameOffsets[m_curBlock].second;
            if (m_inBuffer.src) {
                free((void*)m_inBuffer.src);
            }
            m_inBuffer.src = malloc(len);
            m_inBuffer.pos = 0;
            m_inBuffer.size = len;
            int bread = read((void*)m_inBuffer.src, len);
            if (bread != len) {
                LogErr(VB_SEQUENCE, "Failed to read channel data for frame %d!   Needed to read %" PRIu64 " but read %d\n", frame, len, (int)bread);
            }

            if (m_curBlock < m_file->m_frameOffsets.size() - 2) {
                //let the kernel know that we'll likely need the next block in the near future
                uint64_t len2 = m_file->m_frameOffsets[m_curBlock + 2].second;
                len2 -= m_file->m_frameOffsets[m_curBlock+1].second;
                preload(tell(), len2);
            }

            free(m_outBuffer.dst);
            int numFrames = (m_file->m_frameOffsets[m_curBlock + 1].first > m_file->getNumFrames() ? m_file->getNumFrames() :  m_file->m_frameOffsets[m_curBlock + 1].first) - m_file->m_frameOffsets[m_curBlock].first;
            m_outBuffer.size = numFrames * m_file->getChannelCount();
            m_outBuffer.dst = malloc(m_outBuffer.size);
            m_outBuffer.pos = 0;
            ZSTD_decompressStream(m_dctx, &m_outBuffer, &m_inBuffer);
        }
        int fidx = frame - m_file->m_frameOffsets[m_curBlock].first;
        fidx *= m_file->getChannelCount();
        uint8_t *fdata = (uint8_t*)m_outBuffer.dst;
        UncompressedFrameData *data = new UncompressedFrameData(frame, m_file->m_dataBlockSize, m_file->m_rangesToRead);
        if (!m_file->m_sparseRanges.empty()) {
            memcpy(data->m_data, &fdata[fidx], m_file->getChannelCount());
        } else {
            uint32_t sz = 0;
            //read the ranges into the buffer
            for (auto &rng : data->m_ranges) {
                if (rng.first < m_file->getChannelCount()) {
                    memcpy(&data->m_data[sz], &fdata[fidx + rng.first], rng.second);
                    sz += rng.second;
                }
            }
        }
        return data;
    }
    void compressData(ZSTD_CStream* m_cctx, ZSTD_inBuffer_s &input, ZSTD_outBuffer_s &output) {
        ZSTD_compressStream(m_cctx, &output, &input);
        int count = input.pos;
        int total = input.size;
        uint8_t *curData = (uint8_t*)input.src;
        while (count < total) {
            count += input.pos;
            curData += input.pos;
            input.src = curData;
            input.size -= input.pos;
            input.pos = 0;
            if (output.pos) {
                write(output.dst, output.pos);
                output.pos = 0;
            }
            ZSTD_compressStream(m_cctx, &output, &input);
            count += input.pos;
        }
    }
    virtual void addFrame(uint32_t frame, const uint8_t *data) override {

        if (m_cctx == nullptr) {
            m_cctx = ZSTD_createCStream();
        }
        if (m_curFrameInBlock == 0) {
            uint64_t offset = tell();
            m_file->m_frameOffsets.push_back(std::pair<uint32_t, uint64_t>(frame, offset));
            int clevel = m_file->m_compressionLevel == -1 ? 10 : m_file->m_compressionLevel;
            if (clevel < 0 || clevel > 25) {
                clevel = 10;
            }
            ZSTD_initCStream(m_cctx, clevel);
        }

        uint8_t *curData = (uint8_t *)data;
        if (m_file->m_sparseRanges.empty()) {
            ZSTD_inBuffer_s input = {
                curData,
                m_file->getChannelCount(),
                0
            };
            compressData(m_cctx, input, m_outBuffer);
        } else {
            for (auto &a : m_file->m_sparseRanges) {
                ZSTD_inBuffer_s input = {
                    &curData[a.first],
                    a.second,
                    0
                };
                compressData(m_cctx, input, m_outBuffer);
            }
        }

        if (m_outBuffer.pos > V2FSEQ_OUT_BUFFER_FLUSH_SIZE) {
            //buffer is getting full, better flush it
            write(m_outBuffer.dst, m_outBuffer.pos);
            m_outBuffer.pos = 0;
        }

        m_curFrameInBlock++;
        //if we hit the max per block OR we're in the first block and hit frame #10
        //we'll start a new block.  We want the first block to be small so startup is
        //quicker and we can get the first few frames as fast as possible.
        if ((m_curBlock == 0 && m_curFrameInBlock == 10)
            || (m_curFrameInBlock == m_framesPerBlock && m_file->m_frameOffsets.size() < m_maxBlocks)) {
            while(ZSTD_endStream(m_cctx, &m_outBuffer) > 0) {
                write(m_outBuffer.dst, m_outBuffer.pos);
                m_outBuffer.pos = 0;
            }
            write(m_outBuffer.dst, m_outBuffer.pos);
            m_outBuffer.pos = 0;
            m_curFrameInBlock = 0;
            m_curBlock++;
        }
    }
    virtual void finalize() override {
        if (m_curFrameInBlock) {
            while(ZSTD_endStream(m_cctx, &m_outBuffer) > 0) {
                write(m_outBuffer.dst, m_outBuffer.pos);
                m_outBuffer.pos = 0;
            }
            write(m_outBuffer.dst, m_outBuffer.pos);
            m_outBuffer.pos = 0;
            m_curFrameInBlock = 0;
            m_curBlock++;
        }
        V2CompressedHandler::finalize();
    }

    ZSTD_CStream* m_cctx;
    ZSTD_DStream* m_dctx;
    ZSTD_outBuffer_s m_outBuffer;
    ZSTD_inBuffer_s m_inBuffer;
};
#endif

#ifndef NO_ZLIB
class V2ZLIBCompressionHandler : public V2CompressedHandler {
public:
    V2ZLIBCompressionHandler(V2FSEQFile *f) : V2CompressedHandler(f), m_stream(nullptr), m_outBuffer(nullptr), m_inBuffer(nullptr) {
    }
    virtual ~V2ZLIBCompressionHandler() {
        if (m_outBuffer) {
            free(m_outBuffer);
        }
        if (m_inBuffer) {
            free(m_inBuffer);
        }
    }
    virtual uint8_t getCompressionType() override { return 2; }


    virtual FrameData *getFrame(uint32_t frame) override {
        if (m_curBlock > 256 || (frame < m_file->m_frameOffsets[m_curBlock].first) || (frame >= m_file->m_frameOffsets[m_curBlock + 1].first)) {
            //frame is not in the current block
            m_curBlock = 0;
            while (frame >= m_file->m_frameOffsets[m_curBlock + 1].first) {
                m_curBlock++;
            }
            seek(m_file->m_frameOffsets[m_curBlock].second, SEEK_SET);
            uint64_t len = m_file->m_frameOffsets[m_curBlock + 1].second;
            len -= m_file->m_frameOffsets[m_curBlock].second;
            if (m_inBuffer) {
                free(m_inBuffer);
            }
            m_inBuffer = (uint8_t*)malloc(len);

            int bread = read((void*)m_inBuffer, len);
            if (bread != len) {
                LogErr(VB_SEQUENCE, "Failed to read channel data for frame %d!   Needed to read %" PRIu64 " but read %d\n", frame, len, (int)bread);
            }

            if (m_curBlock < m_file->m_frameOffsets.size() - 2) {
                //let the kernel know that we'll likely need the next block in the near future
                uint64_t len = m_file->m_frameOffsets[m_curBlock + 2].second;
                len -= m_file->m_frameOffsets[m_curBlock+1].second;
                preload(tell(), len);
            }

            if (m_stream == nullptr) {
                m_stream = (z_stream*)calloc(1, sizeof(z_stream));
                m_stream->next_in = m_inBuffer;
                m_stream->avail_in = len;
                inflateInit(m_stream);
            }
            if (m_outBuffer != nullptr) {
                free(m_outBuffer);
            }
            int numFrames = (m_file->m_frameOffsets[m_curBlock + 1].first > m_file->getNumFrames() ? m_file->getNumFrames() :  m_file->m_frameOffsets[m_curBlock + 1].first) - m_file->m_frameOffsets[m_curBlock].first;
            int outsize = numFrames * m_file->getChannelCount();
            m_outBuffer = (uint8_t*)malloc(outsize);
            m_stream->next_out = m_outBuffer;
            m_stream->avail_out = outsize;

            inflate(m_stream, Z_SYNC_FLUSH);
            inflateEnd(m_stream);
            free(m_stream);
            m_stream = nullptr;
        }
        int fidx = frame - m_file->m_frameOffsets[m_curBlock].first;
        fidx *= m_file->getChannelCount();
        uint8_t *fdata = (uint8_t*)m_outBuffer;
        UncompressedFrameData *data = new UncompressedFrameData(frame, m_file->m_dataBlockSize, m_file->m_rangesToRead);
        if (!m_file->m_sparseRanges.empty()) {
            memcpy(data->m_data, &fdata[fidx], m_file->getChannelCount());
        } else {
            uint32_t sz = 0;
            //read the ranges into the buffer
            for (auto &rng : data->m_ranges) {
                if (rng.first < m_file->getChannelCount()) {
                    memcpy(&data->m_data[sz], &fdata[fidx + rng.first], rng.second);
                    sz += rng.second;
                }
            }
        }
        return data;
    }
    virtual void addFrame(uint32_t frame, const uint8_t *data) override {
        if (m_outBuffer == nullptr) {
            m_outBuffer = (uint8_t*)malloc(V2FSEQ_OUT_BUFFER_SIZE);
        }
        if (m_stream == nullptr) {
            m_stream = (z_stream*)calloc(1, sizeof(z_stream));
        }
        if (m_curFrameInBlock == 0) {
            uint64_t offset = tell();
            m_file->m_frameOffsets.push_back(std::pair<uint32_t, uint64_t>(frame, offset));
            deflateEnd(m_stream);
            memset(m_stream, 0, sizeof(z_stream));
        }
        if (m_curFrameInBlock == 0) {
            int clevel = m_file->m_compressionLevel == -1 ? 3 : m_file->m_compressionLevel;
            if (clevel < 0 || clevel > 9) {
                clevel = 3;
            }
            deflateInit(m_stream, clevel);
            m_stream->next_out = m_outBuffer;
            m_stream->avail_out = V2FSEQ_OUT_BUFFER_SIZE;
        }

        uint8_t *curData = (uint8_t *)data;
        if (m_file->m_sparseRanges.empty()) {
            m_stream->next_in = curData;
            m_stream->avail_in = m_file->getChannelCount();
            deflate(m_stream, 0);
        } else {
            for (auto &a : m_file->m_sparseRanges) {
                m_stream->next_in = &curData[a.first];
                m_stream->avail_in = a.second;
                deflate(m_stream, 0);
            }
        }
        if (m_stream->avail_out < (V2FSEQ_OUT_BUFFER_SIZE - V2FSEQ_OUT_BUFFER_FLUSH_SIZE)) {
            //buffer is getting full, better flush it
            uint64_t sz = V2FSEQ_OUT_BUFFER_SIZE;
            sz -= m_stream->avail_out;
            write(m_outBuffer, sz);
            m_stream->next_out = m_outBuffer;
            m_stream->avail_out = V2FSEQ_OUT_BUFFER_SIZE;
        }
        m_curFrameInBlock++;
        //if we hit the max per block OR we're in the first block and hit frame #10
        //we'll start a new block.  We want the first block to be small so startup is
        //quicker and we can get the first few frames as fast as possible.
        if ((m_curBlock == 0 && m_curFrameInBlock == 10)
            || (m_curFrameInBlock == m_framesPerBlock && m_file->m_frameOffsets.size() < m_maxBlocks)) {

            while (deflate(m_stream, Z_FINISH) != Z_STREAM_END) {
                uint64_t sz = V2FSEQ_OUT_BUFFER_SIZE;
                sz -= m_stream->avail_out;
                write(m_outBuffer, sz);
                m_stream->next_out = m_outBuffer;
                m_stream->avail_out = V2FSEQ_OUT_BUFFER_SIZE;
            }
            uint64_t sz = V2FSEQ_OUT_BUFFER_SIZE;
            sz -= m_stream->avail_out;
            write(m_outBuffer, sz);
            m_stream->next_out = m_outBuffer;
            m_stream->avail_out = V2FSEQ_OUT_BUFFER_SIZE;

            m_curFrameInBlock = 0;
            m_curBlock++;
        }
    }
    virtual void finalize() override {
        if (m_curFrameInBlock) {
            while (deflate(m_stream, Z_FINISH) != Z_STREAM_END) {
                uint64_t sz = V2FSEQ_OUT_BUFFER_SIZE;
                sz -= m_stream->avail_out;
                write(m_outBuffer, sz);
                m_stream->next_out = m_outBuffer;
                m_stream->avail_out = V2FSEQ_OUT_BUFFER_SIZE;
            }
            uint64_t sz = V2FSEQ_OUT_BUFFER_SIZE;
            sz -= m_stream->avail_out;
            write(m_outBuffer, sz);
            m_stream->next_out = m_outBuffer;
            m_stream->avail_out = V2FSEQ_OUT_BUFFER_SIZE;

            m_curFrameInBlock = 0;
            m_curBlock++;
        }
        V2CompressedHandler::finalize();
    }

    z_stream *m_stream;
    uint8_t *m_outBuffer;
    uint8_t *m_inBuffer;
};
#endif


void V2FSEQFile::createHandler() {
    switch (m_compressionType) {
    case CompressionType::none:
        m_handler = new V2NoneCompressionHandler(this);
        break;
    case CompressionType::zstd:
#ifdef NO_ZSTD
        LogErr(VB_ALL, "No support for zstd compression");
#else
        m_handler = new V2ZSTDCompressionHandler(this);
#endif
        break;
    case CompressionType::zlib:
#ifdef NO_ZLIB
        LogErr(VB_ALL, "No support for zlib compression");
#else
        m_handler = new V2ZLIBCompressionHandler(this);
#endif
        break;
    }
    if (m_handler == nullptr) {
        m_handler = new V2NoneCompressionHandler(this);
    }
}

V2FSEQFile::V2FSEQFile(const std::string &fn, CompressionType ct, int cl)
    : FSEQFile(fn),
    m_compressionType(ct),
    m_compressionLevel(cl),
    m_handler(nullptr)
{
    m_seqVersionMajor = 2;
    m_seqVersionMinor = 0;
    createHandler();
}
void V2FSEQFile::writeHeader() {
    if (!m_sparseRanges.empty()) {
        //make sure the sparse ranges fit, and then
        //recalculate the channel count for in the fseq
        for (auto &a : m_sparseRanges) {
            if (a.first + a.second > m_seqChannelCount) {
                a.second = m_seqChannelCount - a.first;
            }
        }
        m_seqChannelCount = 0;
        for (auto &a : m_sparseRanges) {
            m_seqChannelCount += a.second;
        }
    }

    uint8_t header[V2FSEQ_HEADER_SIZE];
    memset(header, 0, V2FSEQ_HEADER_SIZE);
    header[0] = 'P';
    header[1] = 'S';
    header[2] = 'E';
    header[3] = 'Q';

    header[6] = 0; //minor
    header[7] = 2; //major

    // Step Size
    write4ByteUInt(&header[10], m_seqChannelCount);
    // Number of Steps
    write4ByteUInt(&header[14], m_seqNumFrames);
    // Step time in ms
    header[18] = m_seqStepTime;
    //flags
    header[19] = 0;

    // compression type
    header[20] = m_handler->getCompressionType();
    //num blocks in compression index, (ignored if not compressed)
    header[21] = 0;
    //num ranges in sparse range index
    header[22] = m_sparseRanges.size();
    //reserved for future use
    header[23] = 0;


    //24-31 - timestamp/uuid/identifier
    if (m_uniqueId == 0) {
        m_uniqueId = GetTime();
    }
    memcpy(&header[24], &m_uniqueId, sizeof(m_uniqueId));

    // index size
    uint32_t maxBlocks = m_handler->computeMaxBlocks() & 0xFF;
    header[21] = maxBlocks;

    int headerSize = V2FSEQ_HEADER_SIZE + maxBlocks * 8 + m_sparseRanges.size() * 6;

    // Fixed header length
    write2ByteUInt(&header[8], headerSize);

    int dataOffset = headerSize;
    for (auto &a : m_variableHeaders) {
        dataOffset += a.data.size() + 4;
    }
    dataOffset = roundTo4(dataOffset);
    write2ByteUInt(&header[4], dataOffset);
    m_seqChanDataOffset = dataOffset;

    write(header, V2FSEQ_HEADER_SIZE);
    for (int x = 0; x < maxBlocks; x++) {
        uint8_t buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        //frame number and len
        write(buf, 8);
    }
    for (auto &a : m_sparseRanges) {
        uint8_t buf[6] = {0, 0, 0, 0, 0, 0};
        write3ByteUInt(buf, a.first);
        write3ByteUInt(&buf[3], a.second);
        write(buf, 6);
    }
    for (auto &a : m_variableHeaders) {
        uint8_t buf[4];
        uint32_t len = a.data.size() + 4;
        write2ByteUInt(buf, len);
        buf[2] = a.code[0];
        buf[3] = a.code[1];
        write(buf, 4);
        write(&a.data[0], a.data.size());
    }
    uint64_t pos = tell();
    if (pos != dataOffset) {
        char buf[4] = {0,0,0,0};
        write(buf, dataOffset - pos);
    }
    LogDebug(VB_SEQUENCE, "Setup for writing v2 FSEQ\n");
    dumpInfo(true);
}


V2FSEQFile::V2FSEQFile(const std::string &fn, FILE *file, const std::vector<uint8_t> &header)
: FSEQFile(fn, file, header),
m_compressionType(none),
m_handler(nullptr)
{
    if (header[0] == 'E') {
        uint32_t modelLen = read4ByteUInt(&header[16]);
        uint32_t modelStart = read4ByteUInt(&header[12]);

        m_compressionType = CompressionType::none;
        //ESEQ files use 1 based start channels, we need 0 based
        m_sparseRanges.push_back(std::pair<uint32_t, uint32_t>(modelStart - 1, modelLen));
    } else {
        //24-31 - timestamp/uuid/identifier
        uint64_t *a = (uint64_t*)&header[24];
        m_uniqueId = *a;
        
        switch (header[20]) {
            case 0:
            m_compressionType = CompressionType::none;
            break;
            case 1:
            m_compressionType = CompressionType::zstd;
            break;
            case 2:
            m_compressionType = CompressionType::zlib;
            break;
            default:
            LogErr(VB_SEQUENCE, "Unknown compression type: %d", (int)header[32]);
        }
        
        uint32_t maxBlocks = header[21];
        
        uint64_t offset = m_seqChanDataOffset;
        int hoffset = V2FSEQ_HEADER_SIZE;
        for (int x = 0; x < maxBlocks; x++) {
            int frame = read4ByteUInt(&header[hoffset]);
            hoffset += 4;
            uint64_t dlen = read4ByteUInt(&header[hoffset]);
            hoffset += 4;
            if (dlen > 0) {
                m_frameOffsets.push_back(std::pair<uint32_t, uint64_t>(frame, offset));
                offset += dlen;
            }
            if (x == 0) {
                uint64_t doff = m_seqChanDataOffset;
                preload(doff, dlen);
            }
        }
        
        m_frameOffsets.push_back(std::pair<uint32_t, uint64_t>(99999999, offset));
        //sparse ranges
        for (int x = 0; x < header[22]; x++) {
            uint32_t st = read3ByteUInt(&header[hoffset]);
            uint32_t len = read3ByteUInt(&header[hoffset + 3]);
            hoffset += 6;
            m_sparseRanges.push_back(std::pair<uint32_t, uint32_t>(st, len));
        }
        parseVariableHeaders(header, hoffset);
    }

    createHandler();
}
V2FSEQFile::~V2FSEQFile() {
    if (m_handler) {
        delete m_handler;
    }
}
void V2FSEQFile::dumpInfo(bool indent) {
    FSEQFile::dumpInfo(indent);
    char ind[5] = "    ";
    if (!indent) {
        ind[0] = 0;
    }

    LogDebug(VB_SEQUENCE, "%sSequence File Information\n", ind);
    LogDebug(VB_SEQUENCE, "%scompressionType       : %d\n", ind, m_compressionType);
    LogDebug(VB_SEQUENCE, "%snumBlocks             : %d\n", ind, m_handler->computeMaxBlocks());
    for (auto &a : m_frameOffsets) {
        LogDebug(VB_SEQUENCE, "%s      %d              : %" PRIu64 "\n", ind, a.first, a.second);
    }
    LogDebug(VB_SEQUENCE, "%snumRanges             : %d\n", ind, m_sparseRanges.size());
    for (auto &a : m_sparseRanges) {
        LogDebug(VB_SEQUENCE, "%s      Start: %d    Len: %d\n", ind, a.first, a.second);
    }
}


void V2FSEQFile::prepareRead(const std::vector<std::pair<uint32_t, uint32_t>> &ranges) {
    if (m_sparseRanges.empty()) {
        m_rangesToRead = ranges;
        m_dataBlockSize = 0;
        for (auto &rng : m_rangesToRead) {
            //make sure we don't read beyond the end of the sequence data
            int toRead = rng.second;
            if ((rng.first + toRead) > m_seqChannelCount) {
                toRead = m_seqChannelCount - rng.first;
                rng.second = toRead;
            }
            m_dataBlockSize += toRead;
        }
    } else if (m_compressionType != CompressionType::none) {
        //with compression, there is no way to NOT read the entire frame of data, we'll just
        //use the sparse data range since we'll have everything anyway so the ranges
        //needed is relatively irrelevant
        m_dataBlockSize = m_seqChannelCount;
        m_rangesToRead = m_sparseRanges;
    } else {
        //no compression with sparse ranges
        //FIXME - an intersection between the two would be useful, but hard
        //for now, just assume that if it's sparse, it has all the data that is needed
        //and read everything
        m_dataBlockSize = m_seqChannelCount;
        m_rangesToRead = m_sparseRanges;
    }
    FrameData *f = getFrame(0);
    if (f) {
        delete f;
    }
}
FrameData *V2FSEQFile::getFrame(uint32_t frame) {
    if (m_rangesToRead.empty()) {
        std::vector<std::pair<uint32_t, uint32_t>> range;
        range.push_back(std::pair<uint32_t, uint32_t>(0, getMaxChannel() + 1));
        prepareRead(range);
    }
    if (frame >= m_seqNumFrames) {
        return nullptr;
    }
    if (m_handler != nullptr) {
        return m_handler->getFrame(frame);
    }
    return nullptr;
}
void V2FSEQFile::addFrame(uint32_t frame,
                          const uint8_t *data) {
    if (m_handler != nullptr) {
        m_handler->addFrame(frame, data);
    }
}

void V2FSEQFile::finalize() {
    if (m_handler != nullptr) {
        m_handler->finalize();
    }
    FSEQFile::finalize();
}

uint32_t V2FSEQFile::getMaxChannel() const {
    uint32_t ret = m_seqChannelCount;
    for (auto &a : m_sparseRanges) {
        uint32_t m = a.first + a.second - 1;
        if (m > ret) {
            ret = m;
        }
    }
    return ret;
}


// This #define must be before any #include's
#define _FILE_OFFSET_BITS 64
#define __STDC_FORMAT_MACROS

#include <cstring>
#include <memory>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <wx/wx.h>
int gettimeofday(struct timeval* tp, struct timezone* tzp) {
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970
    static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    SYSTEMTIME system_time;
    FILETIME file_time;
    uint64_t time;

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

#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "FSEQFile.h"

#if defined(PLATFORM_OSX)
#define PLATFORM_UNKNOWN
#endif

#if defined(PLATFORM_PI) || defined(PLATFORM_BBB) || defined(PLATFORM_UNKNOWN) || defined(PLATFORM_DEBIAN) || defined(PLATFORM_FEDORA) || defined(PLATFORM_UBUNTU) || defined(PLATFORM_MINT)
//for FPP, use FPP logging
#include "Warnings.h"
#include "log.h"
inline void AddSlowStorageWarning() {
    WarningHolder::AddWarningTimeout("FSEQ Data Block not available - Likely slow storage", 90);
}
#else
//compiling within xLights, use log4cpp
#define PLATFORM_UNKNOWN
#include <log4cpp/Category.hh>
template<typename... Args>
static void LogErr(int i, const char* fmt, Args... args) {
    static log4cpp::Category& fseq_logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    char buf[256];
    const char* nfmt = fmt;
    if (fmt[strlen(fmt) - 1] == '\n') {
        strcpy(buf, fmt);
        buf[strlen(fmt) - 1] = 0;
        nfmt = buf;
    }
    fseq_logger_base.error(nfmt, args...);
}
template<typename... Args>
static void LogInfo(int i, const char* fmt, Args... args) {
    static log4cpp::Category& fseq_logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    char buf[256];
    const char* nfmt = fmt;
    if (fmt[strlen(fmt) - 1] == '\n') {
        strcpy(buf, fmt);
        buf[strlen(fmt) - 1] = 0;
        nfmt = buf;
    }
    fseq_logger_base.info(nfmt, args...);
}
template<typename... Args>
static void LogDebug(int i, const char* fmt, Args... args) {
    static log4cpp::Category& fseq_logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    char buf[256];
    const char* nfmt = fmt;
    if (fmt[strlen(fmt) - 1] == '\n') {
        strcpy(buf, fmt);
        buf[strlen(fmt) - 1] = 0;
        nfmt = buf;
    }
    fseq_logger_base.debug(nfmt, args...);
}
inline void AddSlowStorageWarning() {
}

#define VB_SEQUENCE 1
#define VB_ALL 0
#endif

#ifndef NO_ZSTD
#ifndef LINUX
//zstd on Debian 11 doesn't have the thread pool stuff
#define ZSTD_STATIC_LINKING_ONLY
#endif
#include <zstd.h>
#include <thread>

#ifdef ZSTD_STATIC_LINKING_ONLY
class ZSTDThreadPoolHolder {
    struct POOL_ctx_s* pool = nullptr;
public:
    ZSTDThreadPoolHolder() {}
    ~ZSTDThreadPoolHolder() {
        if (pool) {
            ZSTD_freeThreadPool(pool);
        }
    }
    ZSTD_threadPool* getPool() {
        if (pool == nullptr) {
            pool = ZSTD_createThreadPool(std::thread::hardware_concurrency());
        }
        return pool;
    }
    
    static ZSTDThreadPoolHolder INSTANCE;
};
ZSTDThreadPoolHolder ZSTDThreadPoolHolder::INSTANCE;
#endif

#endif
#ifndef NO_ZLIB
#include <zlib.h>
#endif

using FrameData = FSEQFile::FrameData;

inline void DumpHeader(const char* title, unsigned char data[], int len) {
    int x = 0;
    char tmpStr[128];

    snprintf(tmpStr, 128, "%s: (%d bytes)\n", title, len);
    LogInfo(VB_ALL, tmpStr);

    for (int y = 0; y < len; y++) {
        if (x == 0) {
            snprintf(tmpStr, 128, "%06x: ", y);
        }
        snprintf(tmpStr + strlen(tmpStr), 128 - strlen(tmpStr), "%02x ", (int)(data[y] & 0xFF));
        x++;
        if (x == 16) {
            x = 0;
            snprintf(tmpStr + strlen(tmpStr), 128 - strlen(tmpStr), "\n");
            LogInfo(VB_ALL, tmpStr);
        }
    }
    if (x != 0) {
        snprintf(tmpStr + strlen(tmpStr), 128 - strlen(tmpStr), "\n");
        LogInfo(VB_ALL, tmpStr);
    }
}

inline uint64_t GetTime(void) {
    struct timeval now_tv;
    gettimeofday(&now_tv, NULL);
    return now_tv.tv_sec * 1000000LL + now_tv.tv_usec;
}

inline long roundTo4Internal(long i) {
    long remainder = i % 4;
    if (remainder == 0) {
        return i;
    }
    return i + 4 - remainder;
}

inline uint32_t read4ByteUInt(const uint8_t* data) {
    uint32_t r = (data[0]) + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
    return r;
}
inline uint32_t read3ByteUInt(const uint8_t* data) {
    uint32_t r = (data[0]) + (data[1] << 8) + (data[2] << 16);
    return r;
}
inline uint32_t read2ByteUInt(const uint8_t* data) {
    uint32_t r = (data[0]) + (data[1] << 8);
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

static const int V1FSEQ_MINOR_VERSION = 0;
static const int V1FSEQ_MAJOR_VERSION = 1;

static const int V2FSEQ_MINOR_VERSION = 0;
static const int V2FSEQ_MAJOR_VERSION = 2;

static const int V1ESEQ_MINOR_VERSION = 0;
static const int V1ESEQ_MAJOR_VERSION = 2;
static const int V1ESEQ_HEADER_IDENTIFIER = 'E';
static const int V1ESEQ_CHANNEL_DATA_OFFSET = 20;
static const int V1ESEQ_STEP_TIME = 50;

FSEQFile* FSEQFile::openFSEQFile(const std::string& fn) {
    FILE* seqFile = fopen((const char*)fn.c_str(), "rb");
    if (seqFile == NULL) {
        LogErr(VB_SEQUENCE, "Error pre-reading FSEQ file (%s), fopen returned NULL\n", fn.c_str());
        return nullptr;
    }

    fseeko(seqFile, 0L, SEEK_SET);

    // An initial read request of 8 bytes covers the file identifier, version fields and channel data offset
    // This is the minimum needed to validate the file and prepare the proper sized buffer for a larger read
    static const int initialReadLen = 8;

    unsigned char headerPeek[initialReadLen];
    int bytesRead = fread(headerPeek, 1, initialReadLen, seqFile);
#ifndef PLATFORM_UNKNOWN
    posix_fadvise(fileno(seqFile), 0, 0, POSIX_FADV_SEQUENTIAL);
    posix_fadvise(fileno(seqFile), 0, 1024 * 1024, POSIX_FADV_WILLNEED);
#endif

    // Validate bytesRead covers at least the initial read length
    if (bytesRead < initialReadLen) {
        LogErr(VB_SEQUENCE, "Error pre-reading FSEQ file (%s) header, required %d bytes but read %d\n", fn.c_str(), initialReadLen, bytesRead);
        DumpHeader("File hader peek:", headerPeek, bytesRead);
        fclose(seqFile);
        return nullptr;
    }

    // Validate the 4 byte file format identifier is supported
    if ((headerPeek[0] != 'P' && headerPeek[0] != 'F' && headerPeek[0] != V1ESEQ_HEADER_IDENTIFIER) || headerPeek[1] != 'S' || headerPeek[2] != 'E' || headerPeek[3] != 'Q') {
        LogErr(VB_SEQUENCE, "Error pre-reading FSEQ file (%s) header, invalid identifier\n", fn.c_str());
        DumpHeader("File header peek:", headerPeek, bytesRead);
        fclose(seqFile);
        return nullptr;
    }

    uint64_t seqChanDataOffset = read2ByteUInt(&headerPeek[4]);
    int seqVersionMinor = headerPeek[6];
    int seqVersionMajor = headerPeek[7];

    // Test for a ESEQ file (identifier[0] == 'E')
    // ESEQ files are uncompressed V2 FSEQ files with a custom header
    if (headerPeek[0] == V1ESEQ_HEADER_IDENTIFIER) {
        seqChanDataOffset = V1ESEQ_CHANNEL_DATA_OFFSET;
        seqVersionMajor = V1ESEQ_MAJOR_VERSION;
        seqVersionMinor = V1ESEQ_MINOR_VERSION;
    }

    // Read the full header size (beginning at 0 and ending at seqChanDataOffset)
    std::vector<uint8_t> header(seqChanDataOffset);
    fseeko(seqFile, 0L, SEEK_SET);
    bytesRead = fread(&header[0], 1, seqChanDataOffset, seqFile);

    if (bytesRead != seqChanDataOffset) {
        LogErr(VB_SEQUENCE, "Error reading FSEQ file (%s) header, length is %d bytes but read %d\n", fn.c_str(), seqChanDataOffset, bytesRead);
        DumpHeader("File header:", &header[0], bytesRead);
        fclose(seqFile);
        return nullptr;
    }

    // Validate the major version is supported
    // Return a file wrapper to handle version specific metadata
    FSEQFile* file = nullptr;
    if (seqVersionMajor == V1FSEQ_MAJOR_VERSION) {
        file = new V1FSEQFile(fn, seqFile, header);
    } else if (seqVersionMajor == V2FSEQ_MAJOR_VERSION) {
        file = new V2FSEQFile(fn, seqFile, header);
    } else {
        LogErr(VB_SEQUENCE, "Error opening FSEQ file (%s), unknown version %d.%d\n", fn.c_str(), seqVersionMajor, seqVersionMinor);
        DumpHeader("File header:", &header[0], bytesRead);
        fclose(seqFile);
        return nullptr;
    }

    file->dumpInfo();
    return file;
}
FSEQFile* FSEQFile::createFSEQFile(const std::string& fn,
                                   int version,
                                   CompressionType ct,
                                   int level) {
    if (version == V1FSEQ_MAJOR_VERSION) {
        V1FSEQFile* f = new V1FSEQFile(fn);
        if (!f->m_seqFile) {
            delete f;
            f = nullptr;
        }
        return f;
    } else if (version == V2FSEQ_MAJOR_VERSION) {
        V2FSEQFile* f = new V2FSEQFile(fn, ct, level);
        if (!f->m_seqFile) {
            delete f;
            f = nullptr;
        }
        return f;
    }
    LogErr(VB_SEQUENCE, "Error creating FSEQ file (%s), unknown version %d\n", fn.c_str(), version);
    return nullptr;
}
std::string FSEQFile::getMediaFilename(const std::string& fn) {
    std::unique_ptr<FSEQFile> file(FSEQFile::openFSEQFile(fn));
    if (file) {
        return file->getMediaFilename();
    }
    return "";
}
std::string FSEQFile::getMediaFilename() const {
    for (auto& a : m_variableHeaders) {
        if (a.code[0] == 'm' && a.code[1] == 'f') {
            const char* d = (const char*)&a.data[0];
            return d;
        }
    }
    return "";
}

static const int FSEQ_DEFAULT_STEP_TIME = 50;
static const int FSEQ_VARIABLE_HEADER_SIZE = 4;

FSEQFile::FSEQFile(const std::string& fn) :
    m_filename(fn),
    m_seqNumFrames(0),
    m_seqChannelCount(0),
    m_seqStepTime(FSEQ_DEFAULT_STEP_TIME),
    m_variableHeaders(),
    m_uniqueId(0),
    m_seqFileSize(0),
    m_memoryBuffer(),
    m_seqChanDataOffset(0),
    m_memoryBufferPos(0) {
    if (fn == "-memory-") {
        m_seqFile = nullptr;
        m_memoryBuffer.reserve(1024 * 1024);
    } else {
        m_seqFile = fopen((const char*)fn.c_str(), "wb");
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

    if (fseq.getVersionMajor() >= 2) {
        const V2FSEQFile *v2 = dynamic_cast<const V2FSEQFile*>(&fseq);
        if (!v2->m_sparseRanges.empty()) {
            for (auto &a : v2->m_sparseRanges) {
                m_seqChannelCount = std::max(m_seqChannelCount, (a.first + a.second));
            }
        }
    }
}

FSEQFile::FSEQFile(const std::string& fn, FILE* file, const std::vector<uint8_t>& header) :
    m_filename(fn),
    m_seqFile(file),
    m_uniqueId(0),
    m_memoryBuffer(),
    m_memoryBufferPos(0) {
    fseeko(m_seqFile, 0L, SEEK_END);
    m_seqFileSize = ftello(m_seqFile);
    fseeko(m_seqFile, 0L, SEEK_SET);

    if (header[0] == V1ESEQ_HEADER_IDENTIFIER) {
        m_seqChanDataOffset = V1ESEQ_CHANNEL_DATA_OFFSET;
        m_seqVersionMinor = V1ESEQ_MINOR_VERSION;
        m_seqVersionMajor = V1ESEQ_MAJOR_VERSION;
        m_seqChannelCount = read4ByteUInt(&header[8]);
        m_seqStepTime = V1ESEQ_STEP_TIME;
        m_seqNumFrames = (m_seqFileSize - V1ESEQ_CHANNEL_DATA_OFFSET) / m_seqChannelCount;
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

uint64_t FSEQFile::write(const void* ptr, uint64_t size) {
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

uint64_t FSEQFile::read(void* ptr, uint64_t size) {
    return fread(ptr, 1, size, m_seqFile);
}

void FSEQFile::preload(uint64_t pos, uint64_t size) {
#ifndef PLATFORM_UNKNOWN
    if (posix_fadvise(fileno(m_seqFile), pos, size, POSIX_FADV_WILLNEED) != 0) {
        LogErr(VB_SEQUENCE, "Could not advise kernel %d  size: %d\n", (int)pos, (int)size);
    }
#endif
}

inline bool isRecognizedStringVariableHeader(uint8_t a, uint8_t b) {
    // mf - media filename
    // sp - sequence producer
    // see https://github.com/FalconChristmas/fpp/blob/master/docs/FSEQ_Sequence_File_Format.txt#L48 for more information
    return (a == 'm' && b == 'f') || (a == 's' && b == 'p');
}
inline bool isRecognizedBinaryVariableHeader(uint8_t a, uint8_t b) {
    // FC - FPP Commands
    // FE - FPP Effects
    // ED - Extended data
    return (a == 'F' && b == 'C') || (a == 'F' && b == 'E') || (a == 'E' && b == 'D');
}

void FSEQFile::parseVariableHeaders(const std::vector<uint8_t>& header, int readIndex) {
    const int VariableCodeSize = 2;
    const int VariableLengthSize = 2;

    // when encoding, the header size is rounded to the nearest multiple of 4
    // this comparison ensures that there is enough bytes left to at least constitute a 2 byte length + 2 byte code
    while (readIndex + FSEQ_VARIABLE_HEADER_SIZE < header.size()) {
        int dataLength = read2ByteUInt(&header[readIndex]);
        readIndex += VariableLengthSize;

        uint8_t code0 = header[readIndex];
        uint8_t code1 = header[readIndex + 1];
        readIndex += VariableCodeSize;

        VariableHeader vheader;
        vheader.code[0] = code0;
        vheader.code[1] = code1;
        if (dataLength <= FSEQ_VARIABLE_HEADER_SIZE) {
            // empty data, advance only the length of the 2 byte code
            LogInfo(VB_SEQUENCE, "VariableHeader has 0 length data: %c%c", code0, code1);
        } else if (code0 == 'E' && code1 == 'D') {
            // The actual data is elsewhere in the file
            code0 = header[readIndex];
            code1 = header[readIndex + 1];
            readIndex += VariableCodeSize;
            vheader.code[0] = code0;
            vheader.code[1] = code1;
            vheader.extendedData = true;

            uint64_t offset;
            memcpy(&offset, &header[readIndex], 8);
            uint32_t len;
            memcpy(&len, &header[readIndex + 8], 4);
            vheader.data.resize(len);

            uint64_t t = tell();
            seek(offset, SEEK_SET);
            read(&vheader.data[0], len);
            seek(t, SEEK_SET);
            readIndex += 12;
        } else if (readIndex + (dataLength - FSEQ_VARIABLE_HEADER_SIZE) > header.size()) {
            // ensure the data length is contained within the header
            // this is primarily protection against hand modified, or corrupted, sequence files
            LogErr(VB_SEQUENCE, "VariableHeader '%c%c' has out of bounds data length: %d bytes, max length: %d bytes\n",
                   code0, code1, readIndex + dataLength, header.size());

            // there is no reasonable way to recover from this error - the reported dataLength is longer than possible
            // return from parsing variable headers and let the program attempt to read the rest of the file
            return;
        } else {
            // log when reading unrecongized variable headers
            dataLength -= FSEQ_VARIABLE_HEADER_SIZE;
            if (!isRecognizedStringVariableHeader(code0, code1)) {
                if (!isRecognizedBinaryVariableHeader(code0, code1)) {
                    LogDebug(VB_SEQUENCE, "Unrecognized VariableHeader code: %c%c, length: %d bytes\n", code0, code1, dataLength);
                }
            } else {
                // print a warning if the data is not null terminated
                // this is to assist debugging potential string related issues
                // the data is not forcibly null terminated to avoid mutating unknown data
                if (header.size() < readIndex + dataLength) {
                    LogErr(VB_SEQUENCE, "VariableHeader %c%c data exceeds header buffer size!  %d > %d\n",
                           code0, code1, (readIndex + dataLength), header.size());
                } else if (header[readIndex + dataLength - 1] != '\0') {
                    LogErr(VB_SEQUENCE, "VariableHeader %c%c data is not NULL terminated!\n", code0, code1);
                }
            }

            vheader.data.resize(dataLength);
            memcpy(&vheader.data[0], &header[readIndex], dataLength);


            LogDebug(VB_SEQUENCE, "Read VariableHeader: %c%c, length: %d bytes\n", vheader.code[0], vheader.code[1], dataLength);

            // advance the length of the data
            // readIndex now points at the next VariableHeader's length (if any)
            readIndex += dataLength;
        }
        m_variableHeaders.push_back(vheader);
    }
}
void FSEQFile::finalize() {
    fflush(m_seqFile);
}

static const int V1FSEQ_HEADER_SIZE = 28;

V1FSEQFile::V1FSEQFile(const std::string& fn) :
    FSEQFile(fn),
    m_dataBlockSize(0) {
    m_seqVersionMinor = V1FSEQ_MINOR_VERSION;
    m_seqVersionMajor = V1FSEQ_MAJOR_VERSION;
}

void V1FSEQFile::writeHeader() {
    // Additional file format documentation available at:
    // https://github.com/FalconChristmas/fpp/blob/master/docs/FSEQ_Sequence_File_Format.txt#L1

    // Compute headerSize to include the header and variable headers
    int headerSize = V1FSEQ_HEADER_SIZE;
    headerSize += m_variableHeaders.size() * FSEQ_VARIABLE_HEADER_SIZE;
    for (auto& a : m_variableHeaders) {
        headerSize += a.data.size();
    }

    // Round to a product of 4 for better memory alignment
    m_seqChanDataOffset = roundTo4Internal(headerSize);

    // Use m_seqChanDataOffset for buffer size to avoid additional writes or buffer allocations
    // It also comes pre-memory aligned to avoid additional padding
    uint8_t* header = (uint8_t*)malloc(m_seqChanDataOffset);
    memset(header, 0, m_seqChanDataOffset);

    // File identifier (PSEQ) - 4 bytes
    header[0] = 'P';
    header[1] = 'S';
    header[2] = 'E';
    header[3] = 'Q';

    // Channel data start offset - 2 bytes
    write2ByteUInt(&header[4], m_seqChanDataOffset);

    // File format version - 2 bytes
    header[6] = m_seqVersionMinor;
    header[7] = m_seqVersionMajor;

    // Fixed header length - 2 bytes
    // Unlike m_seqChanDataOffset this is a static value and does not include m_variableHeaders
    write2ByteUInt(&header[8], V1FSEQ_HEADER_SIZE);
    // Channel count - 4 bytes
    write4ByteUInt(&header[10], m_seqChannelCount);
    // Number of frames - 4 bytes
    write4ByteUInt(&header[14], m_seqNumFrames);

    // Step time in milliseconds - 1 byte
    header[18] = m_seqStepTime;
    // Flags (unused & reserved, should be 0) - 1 byte
    header[19] = 0;

    // Universe count (unused by fpp, should be 0) - 2 bytes
    write2ByteUInt(&header[20], 0);
    // Universe size (unused by fpp, should be 0) - 2 bytes
    write2ByteUInt(&header[22], 0);

    // Gamma (unused by fpp, should be 1) - 1 byte
    header[24] = 1;
    // Color order (unused by fpp, should be 2) - 1 byte
    header[25] = 2;
    // Unused and reserved field(s), should be 0 - 2 bytes
    header[26] = 0;
    header[27] = 0;

    int writePos = V1FSEQ_HEADER_SIZE;

    // Variable headers
    // 4 byte size minimum (2 byte length + 2 byte code)
    for (auto& a : m_variableHeaders) {
        uint32_t len = FSEQ_VARIABLE_HEADER_SIZE + a.data.size();
        write2ByteUInt(&header[writePos], len);
        header[writePos + 2] = a.code[0];
        header[writePos + 3] = a.code[1];
        memcpy(&header[writePos + 4], &a.data[0], a.data.size());
        writePos += len;
    }

    // Validate final write position matches expected channel data offset
    if (roundTo4Internal(writePos) != m_seqChanDataOffset) {
        LogErr(VB_SEQUENCE, "Final write position (%d, roundTo4 = %d) does not match channel data offset (%d)! This means the header size failed to compute an accurate buffer size.\n", writePos, roundTo4Internal(writePos), m_seqChanDataOffset);
    }

    // Write full header at once
    // header buffer is sized to the value of m_seqChanDataOffset, which comes padded for memory alignment
    // If writePos extends past m_seqChanDataOffset (in error), writing m_seqChanDataOffset prevents data overflow
    write(header, m_seqChanDataOffset);
    free(header);
    LogDebug(VB_SEQUENCE, "Setup for writing v1 FSEQ\n");
    dumpInfo(true);
}

V1FSEQFile::V1FSEQFile(const std::string& fn, FILE* file, const std::vector<uint8_t>& header) :
    FSEQFile(fn, file, header) {
    parseVariableHeaders(header, V1FSEQ_HEADER_SIZE);

    //Use the last modified time for the uniqueId
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
                          const std::vector<std::pair<uint32_t, uint32_t>>& ranges) :
        FrameData(frame),
        m_ranges(ranges) {
        m_size = sz;
        m_data = (uint8_t*)malloc(sz);
    }
    virtual ~UncompressedFrameData() {
        if (m_data != nullptr) {
            free(m_data);
        }
    }

    virtual bool readFrame(uint8_t* data, uint32_t maxChannels) override {
        if (m_data == nullptr)
            return false;
        uint32_t offset = 0;
        for (auto& rng : m_ranges) {
            uint32_t toRead = rng.second;
            if (offset + toRead <= m_size) {
                uint32_t toCopy = std::min(toRead, maxChannels - rng.first);
                memcpy(&data[rng.first], &m_data[offset], toCopy);
                offset += toRead;
            } else {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] virtual size_t GetSize() const override {
        return m_size;
    }

    [[nodiscard]] virtual uint8_t* GetData() const override {
        return m_data;
    }

    uint32_t m_size;
    uint8_t* m_data;
    std::vector<std::pair<uint32_t, uint32_t>> m_ranges;
};

void V1FSEQFile::prepareRead(const std::vector<std::pair<uint32_t, uint32_t>>& ranges, uint32_t startFrame) {
    m_rangesToRead = ranges;
    m_dataBlockSize = 0;
    for (auto& rng : m_rangesToRead) {
        //make sure we don't read beyond the end of the sequence data
        int toRead = rng.second;
        if ((rng.first + toRead) > m_seqChannelCount) {
            toRead = m_seqChannelCount - rng.first;
            rng.second = toRead;
        }
        m_dataBlockSize += toRead;
    }
    FrameData* f = getFrame(startFrame);
    if (f) {
        delete f;
    }
}

FrameData* V1FSEQFile::getFrame(uint32_t frame) {
    if (m_rangesToRead.empty()) {
        std::vector<std::pair<uint32_t, uint32_t>> range;
        range.push_back(std::pair<uint32_t, uint32_t>(0, m_seqChannelCount));
        prepareRead(range, frame);
    }
    uint64_t offset = m_seqChannelCount;
    offset *= frame;
    offset += m_seqChanDataOffset;

    UncompressedFrameData* data = new UncompressedFrameData(frame, m_dataBlockSize, m_rangesToRead);
    if (seek(offset, SEEK_SET)) {
        LogErr(VB_SEQUENCE, "Failed to seek to proper offset for channel data for frame %d! %" PRIu64 "\n", frame, offset);
        return data;
    }
    uint32_t sz = 0;
    //read the ranges into the buffer
    for (auto& rng : data->m_ranges) {
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
                          const uint8_t* data) {
    write(data, m_seqChannelCount);
}

void V1FSEQFile::finalize() {
    FSEQFile::finalize();
}

uint32_t V1FSEQFile::getMaxChannel() const {
    return m_seqChannelCount;
}

static const int V2FSEQ_HEADER_SIZE = 32;
static const int V2FSEQ_SPARSE_RANGE_SIZE = 6;
static const int V2FSEQ_COMPRESSION_BLOCK_SIZE = 8;
#if !defined(NO_ZLIB) || !defined(NO_ZSTD)
static const int V2FSEQ_OUT_BUFFER_SIZE = 32 * 1024 * 1024;        // 32MB output buffer
static const int V2FSEQ_OUT_BUFFER_FLUSH_SIZE = 16 * 1024 * 1024;  // 50% full, flush it
static const int V2FSEQ_OUT_COMPRESSION_BLOCK_SIZE = 64 * 1024; // 64KB blocks
#endif

class V2Handler {
public:
    V2Handler(V2FSEQFile* f) :
        m_file(f) {
        m_seqChanDataOffset = f->m_seqChanDataOffset;
    }
    virtual ~V2Handler() {}

    virtual uint8_t getCompressionType() = 0;
    virtual FrameData* getFrame(uint32_t frame) = 0;

    virtual uint32_t computeMaxBlocks(int max = 255) { return 0; }
    virtual void addFrame(uint32_t frame, const uint8_t* data) = 0;
    virtual std::string GetType() const = 0;

    int seek(uint64_t location, int origin) {
        return m_file->seek(location, origin);
    }
    uint64_t tell() {
        return m_file->tell();
    }
    uint64_t write(const void* ptr, uint64_t size) {
        return m_file->write(ptr, size);
    }
    uint64_t read(void* ptr, uint64_t size) {
        return m_file->read(ptr, size);
    }
    void preload(uint64_t pos, uint64_t size) {
        m_file->preload(pos, size);
    }

    virtual void prepareRead(uint32_t frame) {}

    virtual void finalize() {
        if (!m_file->getVariableHeaders().empty()) {
            for (int x = 0; x < m_variableHeaderOffsets.size(); x++) {
                if (m_variableHeaderOffsets[x] != 0) {
                    uint64_t curEnd = tell();
                    auto &h = m_file->getVariableHeaders()[x];
                    write(&h.data[0], h.data.size());
                    size_t cur = tell();
                    uint64_t off = m_variableHeaderOffsets[x];
                    seek(off, SEEK_SET);
                    write(&curEnd, 8);
                    seek(cur, SEEK_SET);
                }
            }
        }
    }

    V2FSEQFile* m_file = nullptr;
    uint64_t m_seqChanDataOffset = 0;
    
    std::vector<uint64_t> m_variableHeaderOffsets;
};

class V2NoneCompressionHandler : public V2Handler {
public:
    V2NoneCompressionHandler(V2FSEQFile* f) :
        V2Handler(f) {}
    virtual ~V2NoneCompressionHandler() {}

    virtual uint8_t getCompressionType() override { return 0; }
    virtual std::string GetType() const override { return "No Compression"; }
    virtual void prepareRead(uint32_t frame) override {
        FrameData* f = getFrame(frame);
        if (f) {
            delete f;
        }
    }
    virtual FrameData* getFrame(uint32_t frame) override {
        UncompressedFrameData* data = new UncompressedFrameData(frame, m_file->m_dataBlockSize, m_file->m_rangesToRead);
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
            for (auto& rng : data->m_ranges) {
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
    virtual void addFrame(uint32_t frame, const uint8_t* data) override {
        if (m_file->m_sparseRanges.empty()) {
            write(data, m_file->getChannelCount());
        } else {
            for (auto& a : m_file->m_sparseRanges) {
                write(&data[a.first], a.second);
            }
        }
    }
};
class V2CompressedHandler : public V2Handler {
public:
    V2CompressedHandler(V2FSEQFile* f) :
        V2Handler(f),
        m_maxBlocks(0),
        m_curBlock(99999),
        m_framesPerBlock(0),
        m_curFrameInBlock(0) {
        if (!m_file->m_frameOffsets.empty()) {
            m_maxBlocks = m_file->m_frameOffsets.size() - 1;
        }
    }
    virtual ~V2CompressedHandler() {}

    virtual uint32_t computeMaxBlocks(int maxNumBlocks) override {
        if (m_maxBlocks > 0) {
            return m_maxBlocks;
        }
        //determine a good number of compression blocks
        uint64_t datasize = m_file->getChannelCount();
        uint64_t numFrames = m_file->getNumFrames();
        datasize *= numFrames;
        uint64_t numBlocks = datasize / V2FSEQ_OUT_COMPRESSION_BLOCK_SIZE;
        if (numBlocks > maxNumBlocks) {
            //need a lot of blocks, use as many as we can
            numBlocks = maxNumBlocks;
        } else if (numBlocks < 1) {
            numBlocks = 1;
        }
        m_framesPerBlock = numFrames / numBlocks;
        if (m_framesPerBlock < 10)
            m_framesPerBlock = 10;
        m_curFrameInBlock = 0;
        m_curBlock = 0;

        numBlocks = m_file->getNumFrames() / m_framesPerBlock + 1;
        while (numBlocks > maxNumBlocks) {
            m_framesPerBlock++;
            numBlocks = m_file->getNumFrames() / m_framesPerBlock + 1;
        }
        // first block is going to be smaller, so add some blocks
        if (numBlocks < (maxNumBlocks - 1)) {
            numBlocks += 2;
        } else if (numBlocks < maxNumBlocks) {
            numBlocks++;
        }
        m_maxBlocks = numBlocks;
        m_curBlock = 0;
        return m_maxBlocks;
    }

    virtual void finalize() override {
        uint64_t lastFrame = tell();
        uint64_t off = V2FSEQ_HEADER_SIZE;
        seek(off, SEEK_SET);
        int count = m_file->m_frameOffsets.size();
        if (count == 0) {
            //not good, count should never be 0 unless no frames were written,
            //this really should never happen, but I've seen fseq files without the
            //blocks filled in so I know it DOES happen, just haven't figured out
            //how it's possible yet.
            LogErr(VB_SEQUENCE, "Error writing fseq file.  No compressed blocks created.\n");

            //we'll use the offset to the data for the 0 frame
            seek(0, SEEK_SET);
            uint8_t header[10];
            read(header, 10);
            int seqChanDataOffset = read2ByteUInt(&header[4]);
            seek(off, SEEK_SET);
            m_file->m_frameOffsets.push_back(std::pair<uint32_t, uint64_t>(0, seqChanDataOffset));
            count++;
        }
        m_file->m_frameOffsets.push_back(std::pair<uint32_t, uint64_t>(99999999, lastFrame));
        for (int x = 0; x < count; x++) {
            uint8_t buf[8];
            uint32_t frame = m_file->m_frameOffsets[x].first;
            write4ByteUInt(buf, frame);

            uint64_t len64 = m_file->m_frameOffsets[x + 1].second;
            len64 -= m_file->m_frameOffsets[x].second;
            uint32_t len = len64;
            write4ByteUInt(&buf[4], len);
            write(buf, 8);
            //printf("%d    %d: %d\n", x, frame, len);
        }
        m_file->m_frameOffsets.pop_back();

        seek(lastFrame, SEEK_SET);
        V2Handler::finalize();
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
    
    V2ZSTDCompressionHandler(V2FSEQFile* f) :
        V2CompressedHandler(f),
        m_cctx(nullptr),
        m_dctx(nullptr) {
        m_outBuffer.pos = 0;
        m_outBuffer.size = V2FSEQ_OUT_BUFFER_SIZE;
        m_outBuffer.dst = malloc(m_outBuffer.size);
        m_inBuffer.src = nullptr;
        m_inBuffer.size = 0;
        m_inBuffer.pos = 0;
        LogDebug(VB_SEQUENCE, "  Prepared to read/write a ZSTD compress fseq file.\n");
    }
    virtual ~V2ZSTDCompressionHandler() {
        free(m_outBuffer.dst);
        if (m_inBuffer.src != nullptr) {
            free((void*)m_inBuffer.src);
        }
        if (m_cctx) {
            ZSTD_freeCStream(m_cctx);
        }
        if (m_dctx) {
            ZSTD_freeDStream(m_dctx);
        }
    }
    virtual uint8_t getCompressionType() override { return 1; }
    virtual std::string GetType() const override { return "Compressed ZSTD"; }

    virtual FrameData *getFrame(uint32_t frame) override {

        if (m_file == nullptr) LogDebug(VB_SEQUENCE, " getFrame m_file unexpectantly null.\n");

        if (m_curBlock >= m_file->m_frameOffsets.size() || (frame < m_file->m_frameOffsets[m_curBlock].first) || (frame >= m_file->m_frameOffsets[m_curBlock + 1].first)) {
            //frame is not in the current block
            m_curBlock = 0;
            if (m_file->m_frameOffsets.size() <= 1) LogDebug(VB_SEQUENCE, " getFrame m_frameOffsets size <= 1.\n");
            while (frame >= m_file->m_frameOffsets[m_curBlock + 1].first) {
                m_curBlock++;
                if (m_curBlock + 1 >= m_file->m_frameOffsets.size()) LogDebug(VB_SEQUENCE, " getFrame m_curBlock + 1 > m_frameOffsets size.\n");
            }
            if (m_dctx == nullptr) {
                m_dctx = ZSTD_createDStream();
                if (m_dctx == nullptr) LogDebug(VB_SEQUENCE, " getFrame ZSTD_createDStream failed.\n");
            }
            ZSTD_initDStream(m_dctx);
            seek(m_file->m_frameOffsets[m_curBlock].second, SEEK_SET);

            uint64_t len = m_file->m_frameOffsets[m_curBlock + 1].second;
            len -= m_file->m_frameOffsets[m_curBlock].second;
            uint64_t max = m_file->getNumFrames();
            max *= (uint64_t)m_file->getChannelCount();
            if (len > max) {
                len = max;
            }
            if (m_inBuffer.src) {
                free((void*)m_inBuffer.src);
            }
            m_inBuffer.src = malloc(len);
            if (m_inBuffer.src == nullptr) LogDebug(VB_SEQUENCE, " getFrame m_inBuffer.src malloc failed.\n");
            m_inBuffer.pos = 0;
            m_inBuffer.size = len;
            int bread = read((void*)m_inBuffer.src, len);
            if (bread != len) {
                LogErr(VB_SEQUENCE, "Failed to read channel data for frame %d!   Needed to read %" PRIu64 " but read %d\n", frame, len, (int)bread);
            }

            if (m_curBlock < m_file->m_frameOffsets.size() - 2) {
                //let the kernel know that we'll likely need the next block in the near future
                uint64_t len2 = m_file->m_frameOffsets[m_curBlock + 2].second;
                len2 -= m_file->m_frameOffsets[m_curBlock + 1].second;
                preload(tell(), len2);
            }

            free(m_outBuffer.dst);
            m_framesPerBlock = (m_file->m_frameOffsets[m_curBlock + 1].first > m_file->getNumFrames() ? m_file->getNumFrames() : m_file->m_frameOffsets[m_curBlock + 1].first) - m_file->m_frameOffsets[m_curBlock].first;
            m_outBuffer.size = m_framesPerBlock * m_file->getChannelCount();
            m_outBuffer.dst = malloc(m_outBuffer.size);
            if (m_outBuffer.dst == nullptr) LogDebug(VB_SEQUENCE, " getFrame m_outBuffer.dst malloc failed.\n");
            m_outBuffer.pos = 0;
            m_curFrameInBlock = 0;
        }
        uint32_t fidx = frame - m_file->m_frameOffsets[m_curBlock].first;

        if (fidx >= m_curFrameInBlock) {
            if (fidx >= m_framesPerBlock) {
                // should not happen
                // m_outBuffer.dst is pre-sized to "m_framesPerBlock * m_file->getChannelCount()"
                //  ( see malloc above )
                // so as long as the fidx < m_framesPerBlock, we don't need to resize.   We just
                // need to update the m_outBuffer.size to let the decompressor know there is space there
                // for the next frame.
                //
                // Note: we COULD just set the m_outBuffer.size to the full size of the decompressed block
                // up front instead of per frame.  That would be very slightly faster to load the
                // fseq in xLights (which loads all the frames up front), but would cause extra
                // latencies in FPP and xSchedule which request one frame at a time.  Requesting
                // the first frame in the block would trigger decompressing all the frames in the
                // block immediately which, if there are a lot of frames, could take much longer
                // than we'd have available in a latency critical step.
                if ((fidx + 1) * m_file->getChannelCount() > m_outBuffer.size) {
                    LogDebug(VB_SEQUENCE,
                             " getFrame m_outBuffer.size increased but memory not reallocated.  Block: %u Frame: %u FramesInBlock: %u\n",
                             m_curBlock, fidx, m_framesPerBlock);
                }
            }
            m_outBuffer.size = (fidx + 1) * m_file->getChannelCount();
            ZSTD_decompressStream(m_dctx, &m_outBuffer, &m_inBuffer);
            m_curFrameInBlock = fidx + 1;
        }

        fidx *= m_file->getChannelCount();
        uint8_t* fdata = (uint8_t*)m_outBuffer.dst;
        UncompressedFrameData* data = new UncompressedFrameData(frame, m_file->m_dataBlockSize, m_file->m_rangesToRead);

        // This stops the crash on load ... but it is not the root cause.
        // But better to not load completely than crashing
        if (fidx < 0) {
            // this is not going to end well ... best to give up here
            LogErr(VB_SEQUENCE, "Frame index calculated as a negative number. Aborting frame %d load.\n", (int)frame);
            return data;
        }

        if (!m_file->m_sparseRanges.empty()) {
            memcpy(data->m_data, &fdata[fidx], m_file->getChannelCount());
        } else {
            uint32_t sz = 0;
            //read the ranges into the buffer
            for (auto& rng : data->m_ranges) {
                if (rng.first < m_file->getChannelCount()) {
                    uint64_t start = fidx + rng.first;
                    memcpy(&data->m_data[sz], &fdata[start], rng.second);
                    sz += rng.second;
                }
            }
        }
        return data;
    }
    void compressData(ZSTD_CStream* m_cctx, ZSTD_inBuffer_s& input, ZSTD_outBuffer_s& output) {
        ZSTD_compressStream2(m_cctx, &output, &input, ZSTD_e_continue);
        int count = input.pos;
        int total = input.size;
        uint8_t* curData = (uint8_t*)input.src;
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
            ZSTD_compressStream2(m_cctx, &output, &input, ZSTD_e_continue);
            count += input.pos;
        }
    }
    virtual void addFrame(uint32_t frame, const uint8_t* data) override {
        if (m_cctx == nullptr) {
            m_cctx = ZSTD_createCStream();
        }
        if (m_curFrameInBlock == 0) {
            uint64_t offset = tell();
            //LogDebug(VB_SEQUENCE, "  Preparing to create a compressed block of data starting at frame %d, offset  %" PRIu64 ".\n", frame, offset);
            m_file->m_frameOffsets.push_back(std::pair<uint32_t, uint64_t>(frame, offset));
            int clevel = m_file->m_compressionLevel == -99 ? 2 : m_file->m_compressionLevel;
            if (clevel < -25 || clevel > 25) {
                clevel = 2;
            }
            if (frame == 0 && (ZSTD_versionNumber() > 10305)) {
                // first frame needs to be grabbed as fast as possible
                // or remotes may be off by a few frames at start.  Thus,
                // if using recent zstd, we'll use the negative levels
                // for the first block so the decompression can
                // be as fast as possible
                clevel = -10;
            }
            if (ZSTD_versionNumber() <= 10305 && clevel < 0) {
                clevel = 0;
            }
            ZSTD_initCStream(m_cctx, clevel);
            //ZSTD_CCtx_reset(m_cctx, ZSTD_reset_session_only);
            //ZSTD_CCtx_refCDict(m_cctx, NULL);
            //ZSTD_CCtx_setParameter(m_cctx, ZSTD_c_compressionLevel, clevel);

#ifdef ZSTD_STATIC_LINKING_ONLY
            ZSTD_CCtx_setParameter(m_cctx, ZSTD_c_nbWorkers, std::thread::hardware_concurrency());
            ZSTD_CCtx_refThreadPool(m_cctx, ZSTDThreadPoolHolder::INSTANCE.getPool());
#endif
        }

        uint8_t* curData = (uint8_t*)data;
        if (m_file->m_sparseRanges.empty()) {
            ZSTD_inBuffer_s input = {
                curData,
                m_file->getChannelCount(),
                0
            };
            compressData(m_cctx, input, m_outBuffer);
        } else {
            for (auto& a : m_file->m_sparseRanges) {
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
        if ((m_curBlock == 0 && m_curFrameInBlock == 10) || (m_curFrameInBlock >= m_framesPerBlock && m_file->m_frameOffsets.size() < m_maxBlocks)) {
            ZSTD_inBuffer_s input = {
                0, 0, 0
            };
            while(ZSTD_compressStream2(m_cctx, &m_outBuffer, &input, ZSTD_e_end) > 0) {
                write(m_outBuffer.dst, m_outBuffer.pos);
                m_outBuffer.pos = 0;
            }
            write(m_outBuffer.dst, m_outBuffer.pos);
            //LogDebug(VB_SEQUENCE, "  Finalized block of data ending at frame %d.  Frames in block: %d.\n", frame, m_curFrameInBlock);
            m_outBuffer.pos = 0;
            m_curFrameInBlock = 0;
            m_curBlock++;
        }
    }
    virtual void finalize() override {
        if (m_curFrameInBlock) {
            ZSTD_inBuffer_s input = {
                0, 0, 0
            };
            while(ZSTD_compressStream2(m_cctx, &m_outBuffer, &input, ZSTD_e_end) > 0) {
                write(m_outBuffer.dst, m_outBuffer.pos);
                m_outBuffer.pos = 0;
            }
            write(m_outBuffer.dst, m_outBuffer.pos);
            LogDebug(VB_SEQUENCE, "  Finalized last block of data.  Frames in block: %d.\n", m_curFrameInBlock);
            m_outBuffer.pos = 0;
            m_curFrameInBlock = 0;
            m_curBlock++;
        }
        V2CompressedHandler::finalize();
    }

    ZSTD_CCtx* m_cctx = nullptr;
    ZSTD_DStream* m_dctx = nullptr;
    ZSTD_outBuffer_s m_outBuffer;
    ZSTD_inBuffer_s m_inBuffer;
};
#endif

#ifndef NO_ZLIB
class V2ZLIBCompressionHandler : public V2CompressedHandler {
public:
    V2ZLIBCompressionHandler(V2FSEQFile* f) :
        V2CompressedHandler(f),
        m_stream(nullptr),
        m_outBuffer(nullptr),
        m_inBuffer(nullptr) {
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
    virtual std::string GetType() const override { return "Compressed ZLIB"; }

    virtual FrameData* getFrame(uint32_t frame) override {
        if (m_curBlock >= m_file->m_frameOffsets.size() || (frame < m_file->m_frameOffsets[m_curBlock].first) || (frame >= m_file->m_frameOffsets[m_curBlock + 1].first)) {
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
            int numFrames = (m_file->m_frameOffsets[m_curBlock + 1].first > m_file->getNumFrames() ? m_file->getNumFrames() : m_file->m_frameOffsets[m_curBlock + 1].first) - m_file->m_frameOffsets[m_curBlock].first;
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
        uint8_t* fdata = (uint8_t*)m_outBuffer;
        UncompressedFrameData* data = new UncompressedFrameData(frame, m_file->m_dataBlockSize, m_file->m_rangesToRead);
        if (!m_file->m_sparseRanges.empty()) {
            memcpy(data->m_data, &fdata[fidx], m_file->getChannelCount());
        } else {
            uint32_t sz = 0;
            //read the ranges into the buffer
            for (auto& rng : data->m_ranges) {
                if (rng.first < m_file->getChannelCount()) {
                    memcpy(&data->m_data[sz], &fdata[fidx + rng.first], rng.second);
                    sz += rng.second;
                }
            }
        }
        return data;
    }
    virtual void addFrame(uint32_t frame, const uint8_t* data) override {
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
            int clevel = m_file->m_compressionLevel == -99 ? 3 : m_file->m_compressionLevel;
            if (clevel < 0 || clevel > 9) {
                clevel = 3;
            }
            deflateInit(m_stream, clevel);
            m_stream->next_out = m_outBuffer;
            m_stream->avail_out = V2FSEQ_OUT_BUFFER_SIZE;
        }

        uint8_t* curData = (uint8_t*)data;
        if (m_file->m_sparseRanges.empty()) {
            m_stream->next_in = curData;
            m_stream->avail_in = m_file->getChannelCount();
            deflate(m_stream, 0);
        } else {
            for (auto& a : m_file->m_sparseRanges) {
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
        if ((m_curBlock == 0 && m_curFrameInBlock == 10) || (m_curFrameInBlock == m_framesPerBlock && m_file->m_frameOffsets.size() < m_maxBlocks)) {
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

    z_stream* m_stream;
    uint8_t* m_outBuffer;
    uint8_t* m_inBuffer;
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
        LogDebug(VB_SEQUENCE, "Creating a default none compression handler. %d", (int)m_compressionType);
        m_handler = new V2NoneCompressionHandler(this);
    }
}

V2FSEQFile::V2FSEQFile(const std::string& fn, CompressionType ct, int cl) :
    FSEQFile(fn),
    m_compressionType(ct),
    m_compressionLevel(cl),
    m_handler(nullptr),
    m_allowExtendedBlocks(false) {
    m_seqVersionMajor = V2FSEQ_MAJOR_VERSION;
    m_seqVersionMinor = V2FSEQ_MINOR_VERSION;

    createHandler();
}
void V2FSEQFile::writeHeader() {
    if (!m_sparseRanges.empty()) {
        //make sure the sparse ranges fit, and then
        //recalculate the channel count for in the fseq
        std::vector<std::pair<uint32_t, uint32_t>> newRanges;
        for (auto& a : m_sparseRanges) {
            if (a.first < m_seqChannelCount) {
                if (a.first + a.second > m_seqChannelCount) {
                    a.second = m_seqChannelCount - a.first;
                }
                newRanges.push_back(a);
            }
        }
        m_sparseRanges = newRanges;
        if (!m_sparseRanges.empty()) {
            m_seqChannelCount = 0;
            for (auto& a : m_sparseRanges) {
                m_seqChannelCount += a.second;
            }
        }
    }

    // Additional file format documentation available at:
    // https://github.com/FalconChristmas/fpp/blob/master/docs/FSEQ_Sequence_File_Format.txt#L17

    uint16_t maxBlocks = m_handler->computeMaxBlocks(m_allowExtendedBlocks ? 4095 : 255) & 0xFFF;

    // Compute headerSize to include the header, compression blocks and sparse ranges
    int headerSize = V2FSEQ_HEADER_SIZE;
    headerSize += maxBlocks * V2FSEQ_COMPRESSION_BLOCK_SIZE;
    headerSize += m_sparseRanges.size() * V2FSEQ_SPARSE_RANGE_SIZE;

    // Channel data offset is the headerSize plus size of variable headers
    // Round to a product of 4 for better memory alignment
    m_seqChanDataOffset = headerSize;
    uint64_t seqChanDataOffset2 = headerSize;
    for (auto& a : m_variableHeaders) {
        uint32_t sze = a.data.size() + FSEQ_VARIABLE_HEADER_SIZE;
        if (a.extendedData) {
            seqChanDataOffset2 += FSEQ_VARIABLE_HEADER_SIZE + 14;
            m_seqChanDataOffset += FSEQ_VARIABLE_HEADER_SIZE + 14;
        } else if (sze <= (14 + FSEQ_VARIABLE_HEADER_SIZE)) {
            // smaller than the extended data type so would never be output
            // as extended data
            m_seqChanDataOffset += sze;
            seqChanDataOffset2 += sze; // smaller than the shifted size
        } else {
            // could potentially be placed in the extended area if the header is too large
            // record both sizes to compare later
            m_seqChanDataOffset += sze;
            seqChanDataOffset2 += FSEQ_VARIABLE_HEADER_SIZE + 14; //64bit offset into file, 32bit length, new 2byte code
        }
    }
    bool forceExtended = false;
    if (m_seqVersionMinor >= 2 && (m_seqChanDataOffset >= 0xFFFF)) {
        // beyond the byte header size.  The variable header data will need to be stored elsewhere
        m_seqChanDataOffset = seqChanDataOffset2;
        forceExtended = true;
    }
    m_seqChanDataOffset = roundTo4Internal(m_seqChanDataOffset);

    // Use m_seqChanDataOffset for buffer size to avoid additional writes or buffer allocations
    // It also comes pre-memory aligned to avoid adding padding
    //uint8_t header[m_seqChanDataOffset]; // MSW does not support non compile time constant array sizes
    uint8_t* header = (uint8_t*)malloc(m_seqChanDataOffset);
    memset(header, 0, m_seqChanDataOffset);

    // File identifier (PSEQ) - 4 bytes
    header[0] = 'P';
    header[1] = 'S';
    header[2] = 'E';
    header[3] = 'Q';

    // Channel data start offset - 2 bytes
    write2ByteUInt(&header[4], m_seqChanDataOffset);

    // File format version - 2 bytes
    header[6] = m_seqVersionMinor;
    header[7] = m_seqVersionMajor;

    // Computed header length - 2 bytes
    write2ByteUInt(&header[8], headerSize);
    // Channel count - 4 bytes
    write4ByteUInt(&header[10], m_seqChannelCount);
    // Number of frames - 4 bytes
    write4ByteUInt(&header[14], m_seqNumFrames);

    // Step time in milliseconds - 1 byte
    header[18] = m_seqStepTime;
    // Flags (unused & reserved, should be 0) - 1 byte
    header[19] = 0;
    // Compression type and upper 4 bits of max blocks- 1 byte
    header[20] = ((maxBlocks >> 4) & 0xF0) | m_handler->getCompressionType();
    // Number of blocks in compressed channel data (should be 0 if not compressed) - 1 byte, lower 8 bits
    header[21] = maxBlocks & 0xFF;
    // Number of ranges in sparse range index - 1 byte
    header[22] = m_sparseRanges.size();
    // Flags (unused & reserved, should be 0) - 1 byte
    header[23] = 0;

    // Timestamp based UUID - 8 bytes
    if (m_uniqueId == 0) {
        m_uniqueId = GetTime();
    }
    memcpy(&header[24], &m_uniqueId, sizeof(m_uniqueId));

    int writePos = V2FSEQ_HEADER_SIZE;

    // Empty compression blocks are automatically added when calculating headerSize (see maxBlocks)
    // Their data is initialized to 0 by memset and computed later
    writePos += maxBlocks * V2FSEQ_COMPRESSION_BLOCK_SIZE;

    // Sparse ranges
    // 6 byte size (3 byte value + 3 byte value)
    for (auto& a : m_sparseRanges) {
        write3ByteUInt(&header[writePos], a.first);
        write3ByteUInt(&header[writePos + 3], a.second);
        writePos += V2FSEQ_SPARSE_RANGE_SIZE;
    }

    // Variable headers
    // 4 byte size minimum (2 byte length + 2 byte code)
    int idx = 0;
    m_handler->m_variableHeaderOffsets.resize(m_variableHeaders.size());
    for (auto& a : m_variableHeaders) {
        uint32_t len = FSEQ_VARIABLE_HEADER_SIZE + a.data.size();
        bool doExtended = a.extendedData;
        if (!doExtended && forceExtended && len > 18) {
            // longer than the extended header and we need to save space
            doExtended = true;
        }
        if (doExtended) {
            len = 14 + FSEQ_VARIABLE_HEADER_SIZE;
        }
        write2ByteUInt(&header[writePos], len);
        writePos += 2;
        if (doExtended) {
            header[writePos] = 'E';
            header[writePos + 1] = 'D';
            writePos += 2;
        }
        header[writePos] = a.code[0];
        header[writePos + 1] = a.code[1];
        writePos += 2;
        if (doExtended) {
            memset(&header[writePos], 0, 8);
            m_handler->m_variableHeaderOffsets[idx] = writePos;
            writePos += 8; //file position
            write4ByteUInt(&header[writePos], a.data.size());
            writePos += 4;
        } else {
            m_handler->m_variableHeaderOffsets[idx] = 0;
            memcpy(&header[writePos], &a.data[0], a.data.size());
            writePos += a.data.size();
        }
        ++idx;
    }

    // Validate final write position matches expected channel data offset
    if (roundTo4Internal(writePos) != m_seqChanDataOffset) {
        LogErr(VB_SEQUENCE, "Final write position (%d, roundTo4 = %d) does not match channel data offset (%d)! This means the header size failed to compute an accurate buffer size.\n", writePos, roundTo4Internal(writePos), m_seqChanDataOffset);
    }

    // Write full header at once
    // header buffer is sized to the value of m_seqChanDataOffset, which comes padded for memory alignment
    // If writePos extends past m_seqChanDataOffset (in error), writing m_seqChanDataOffset prevents data overflow
    write(header, m_seqChanDataOffset);

    free(header);

    LogDebug(VB_SEQUENCE, "Setup for writing v2 FSEQ\n");
    dumpInfo(true);
}

V2FSEQFile::V2FSEQFile(const std::string& fn, FILE* file, const std::vector<uint8_t>& header) :
    FSEQFile(fn, file, header),
    m_compressionType(none),
    m_handler(nullptr) {
    if (m_seqVersionMajor == 2 && m_seqVersionMinor > 2) {
        LogErr(VB_SEQUENCE, "Unknown minor version: %d.  FSEQ may not load properly.\n", m_seqVersionMinor);
    }

    if (header[0] == V1ESEQ_HEADER_IDENTIFIER) {
        m_compressionType = CompressionType::none;

        uint32_t modelStart = read4ByteUInt(&header[12]);
        uint32_t modelLen = read4ByteUInt(&header[16]);

        // ESEQ files use 1 based start channels, offset to start at 0
        m_sparseRanges.push_back(std::pair<uint32_t, uint32_t>(modelStart ? modelStart - 1 : modelStart, modelLen));
    } else {
        switch (header[20] & 0xF) {
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
            LogErr(VB_SEQUENCE, "Unknown compression type: %d\n", (int)header[20]);
        }

        // readPos tracks the reader index for variable length data past the fixed header size
        // This is used to check for reader index overflows
        int readPos = V2FSEQ_HEADER_SIZE;

        // Read compression blocks
        // 8 byte size each (4 byte firstFrame + 4 byte length)
        // header[21] is the "max blocks count" field, lower 8 bits
        // header[20] & 0xF0 is the upper 4 bits, total 12 bits
        // for maximum of 4095 blocks
        uint64_t lastBlockOffset = m_seqChanDataOffset;
        uint32_t numBlocks = header[20];
        numBlocks &= 0xF0;
        numBlocks <<= 4;
        numBlocks |= header[21];

        for (uint32_t i = 0; i < numBlocks; i++) {
            uint32_t firstFrame = read4ByteUInt(&header[readPos]);
            uint64_t length = read4ByteUInt(&header[readPos + 4]);

            if (length > 0) {
                m_frameOffsets.push_back(std::pair<uint32_t, uint64_t>(firstFrame, lastBlockOffset));
                lastBlockOffset += length;
            }

            readPos += V2FSEQ_COMPRESSION_BLOCK_SIZE;

            // Duplicated legacy behavior
            // Preloads up to [length] bytes starting at m_seqChanDataOffset
            // This pre-buffers the first compression block
            if (i == 0) {
                preload(m_seqChanDataOffset, length);
            }
        }

        if (m_compressionType == CompressionType::none) {
            // Push frame offsets that cover the entire file length given the channel data is effectively a single block
            // For uncompressed blocks, maxBlocks should always be 0 and m_frameOffsets initially empty
            m_frameOffsets.push_back(std::pair<uint32_t, uint64_t>(0, m_seqChanDataOffset));
        } else if (m_frameOffsets.size() == 0) {
            LogErr(VB_SEQUENCE, "FSEQ file corrupt: did not load any block references from header.\n");

            // File is flagged as compressed but no compression blocks were read
            // The file is likely corrupted, read the full channel data as a single block as a recovery attempt
            m_frameOffsets.push_back(std::pair<uint32_t, uint64_t>(0, m_seqChanDataOffset));
        }

        // Always push a final frame offset that ensures coverage of the full channel data length
        m_frameOffsets.push_back(std::pair<uint32_t, uint64_t>(getNumFrames() + 2, this->m_seqFileSize));

        // Read sparse ranges
        // 6 byte size each (3 byte firstChannel + 3 byte length)
        // header[22] is the "sparse range count" field
        for (int i = 0; i < header[22]; i++) {
            uint32_t startChan = read3ByteUInt(&header[readPos]);
            uint32_t length = read3ByteUInt(&header[readPos + 3]);

            m_sparseRanges.push_back(std::pair<uint32_t, uint32_t>(startChan, length));

            readPos += V2FSEQ_SPARSE_RANGE_SIZE;
        }

        // Validate read position matches expected header size
        // This does not include the variable headers length
        uint16_t headerSize = read2ByteUInt(&header[8]);

        if (readPos != headerSize) {
            LogErr(VB_SEQUENCE, "Read position (%d) does not match expected header size %d!\n", readPos, headerSize);
        }

        // Read timestamp based UUID - 8 bytes
        // This does not advance readPos since it is a fixed index
        m_uniqueId = *((uint64_t*)&header[24]);

        // The remainder of the buffer (m_seqChanDataOffset - headerSize) contains an unknown count of variable headers
        // This will loop and continue reading until it hits padding or m_seqChanDataOffset
        // As long as readPos == headerSize prior to this call, the read is a success
        parseVariableHeaders(header, readPos);
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
    // Commented out to declutter the logs ... we can add it back in if we start seeing issues
    //for (auto &a : m_frameOffsets) {
    //    LogDebug(VB_SEQUENCE, "%s      %d              : %" PRIu64 "\n", ind, a.first, a.second);
    //}
    LogDebug(VB_SEQUENCE, "%snumRanges             : %d\n", ind, m_sparseRanges.size());
    // Commented out to declutter the logs ... we can add it back in if we start seeing issues
    //for (auto &a : m_sparseRanges) {
    //    LogDebug(VB_SEQUENCE, "%s      Start: %d    Len: %d\n", ind, a.first, a.second);
    //}
}

void V2FSEQFile::prepareRead(const std::vector<std::pair<uint32_t, uint32_t>>& ranges, uint32_t startFrame) {
    if (m_sparseRanges.empty()) {
        m_rangesToRead.clear();
        m_dataBlockSize = 0;
        for (auto rng : ranges) {
            //make sure we don't read beyond the end of the sequence data
            int toRead = rng.second;

            if (rng.first < m_seqChannelCount) {
                if ((rng.first + toRead) > m_seqChannelCount) {
                    toRead = m_seqChannelCount - rng.first;
                    rng.second = toRead;
                }
                m_dataBlockSize += toRead;
                m_rangesToRead.push_back(rng);
            }
        }
        if (m_dataBlockSize == 0) {
            m_rangesToRead.push_back(std::pair<uint32_t, uint32_t>(0, getMaxChannel()));
            m_dataBlockSize = getMaxChannel();
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
    m_handler->prepareRead(startFrame);
}
FrameData* V2FSEQFile::getFrame(uint32_t frame) {
    if (m_rangesToRead.empty()) {
        std::vector<std::pair<uint32_t, uint32_t>> range;
        range.push_back(std::pair<uint32_t, uint32_t>(0, getMaxChannel()));
        prepareRead(range, frame);
    }
    if (frame >= m_seqNumFrames) {
        return nullptr;
    }
    if (m_handler != nullptr) {
        FrameData* fd = nullptr;
        try {
            fd = m_handler->getFrame(frame);
        } catch (...) {
            LogErr(VB_SEQUENCE, "Error getting frame from handler %s.\n", m_handler->GetType().c_str());
        }
        return fd;
    }
    return nullptr;
}
void V2FSEQFile::addFrame(uint32_t frame,
                          const uint8_t* data) {
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
    for (auto& a : m_sparseRanges) {
        uint32_t m = a.first + a.second;
        if (m > ret) {
            ret = m;
        }
    }
    return ret;
}

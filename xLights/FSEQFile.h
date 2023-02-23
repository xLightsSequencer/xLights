#pragma once

#include <stdio.h>
#include <string>
#include <vector>

class FSEQFile {
public:
    class VariableHeader {
        public:
        VariableHeader() { code[0] = code[1] = 0; }
        VariableHeader(const VariableHeader& cp) : data(cp.data) {
            code[0] = cp.code[0];
            code[1] = cp.code[1];
            extendedData = cp.extendedData;
        }
        ~VariableHeader() {}

        uint8_t code[2];
        std::vector<uint8_t> data;
        bool extendedData = false;
    };


    class FrameData {
        public:
        FrameData(uint32_t f) : frame(f) {};
        virtual ~FrameData() {};

        virtual bool readFrame(uint8_t *data, uint32_t maxChannels) = 0;

        uint32_t frame;
    };

    enum CompressionType {
        none,
        zstd,
        zlib
    };

protected:
    //open file for reading
    FSEQFile(const std::string &fn, FILE *file, const std::vector<uint8_t> &header);
    //open file for writing
    FSEQFile(const std::string &fn);

public:

    virtual ~FSEQFile();

    static FSEQFile* openFSEQFile(const std::string &fn);

    static FSEQFile* createFSEQFile(const std::string &fn,
                                    int version,
                                    CompressionType ct = CompressionType::zstd,
                                    int level = -99);
    //utility methods
    static std::string getMediaFilename(const std::string &fn);
    std::string getMediaFilename() const;
    uint32_t getTotalTimeMS() const { return m_seqNumFrames * m_seqStepTime; }

    void parseVariableHeaders(const std::vector<uint8_t> &header, int start);


    //prepare to start reading. The ranges will be the list of channel ranges that
    //are actually needed for each frame.   The reader can optimize to only
    //read those frames.
    virtual void prepareRead(const std::vector<std::pair<uint32_t, uint32_t>> &ranges, uint32_t startFrame = 0) {}

    //For reading data from the fseq file, returns an object can
    //provide the necessary data in a timely fashion for the given frame
    //It may not be used right away and will be deleted at some point in the future
    virtual FrameData *getFrame(uint32_t frame) = 0;

    //For writing to the fseq file
    virtual void enableMinorVersionFeatures(uint8_t ver) {}
    virtual void initializeFromFSEQ(const FSEQFile& fseq);
    virtual void writeHeader() = 0;
    virtual void addFrame(uint32_t frame,
                          const uint8_t *data) = 0;
    virtual void finalize();

    virtual void dumpInfo(bool indent = false);


    uint32_t      getNumFrames() const { return m_seqNumFrames; }
    int           getStepTime() const { return m_seqStepTime; }
    uint32_t      getChannelCount() const  { return m_seqChannelCount; }
    int           getVersionMajor() const { return m_seqVersionMajor; }
    int           getVersionMinor() const { return m_seqVersionMinor; }
    uint64_t      getUniqueId() const { return m_uniqueId; }
    const std::string& getFilename() const { return m_filename; }


    virtual uint32_t getMaxChannel() const = 0;
    const std::vector<VariableHeader> &getVariableHeaders() const { return m_variableHeaders;}

    void setNumFrames(uint32_t f) { m_seqNumFrames = f; }
    void setStepTime(int st) { m_seqStepTime = st; }
    void setChannelCount(int cc) { m_seqChannelCount = cc; }
    void addVariableHeader(const VariableHeader &header) { m_variableHeaders.push_back(header);}


    const std::vector<uint8_t> &getMemoryBuffer() const { return m_memoryBuffer;}
    uint64_t getMemoryBufferPos() const { return m_memoryBufferPos; }
protected:
    std::string   m_filename;
    uint64_t      m_uniqueId;
    uint32_t      m_seqNumFrames;
    uint32_t      m_seqChannelCount;
    int           m_seqStepTime;
    int           m_seqVersionMajor;
    int           m_seqVersionMinor;

    std::vector<VariableHeader> m_variableHeaders;

protected:
    uint64_t      m_seqFileSize;
    uint64_t      m_seqChanDataOffset;

    int seek(uint64_t location, int origin);
    uint64_t tell();
    uint64_t write(const void * ptr, uint64_t size);
    uint64_t read(void *ptr, uint64_t size);
    void preload(uint64_t pos, uint64_t size);

private:
    FILE* volatile  m_seqFile;
    std::vector<uint8_t> m_memoryBuffer;
    uint64_t      m_memoryBufferPos;
};


class V1FSEQFile : public FSEQFile {
public:
    V1FSEQFile(const std::string &fn, FILE *file, const std::vector<uint8_t> &header);
    V1FSEQFile(const std::string &fn);

    virtual ~V1FSEQFile();

    virtual void prepareRead(const std::vector<std::pair<uint32_t, uint32_t>> &ranges, uint32_t startFrame = 0) override;
    virtual FrameData *getFrame(uint32_t frame) override;

    virtual void writeHeader() override;
    virtual void addFrame(uint32_t frame,
                          const uint8_t *data) override;
    virtual void finalize() override;

    virtual uint32_t getMaxChannel() const override;


    //The ranges to read and the data size needed to read the ranges
    std::vector<std::pair<uint32_t, uint32_t>> m_rangesToRead;
    uint32_t m_dataBlockSize;
};


class V2Handler;

class V2FSEQFile : public FSEQFile {

public:
    V2FSEQFile(const std::string &fn, FILE *file, const std::vector<uint8_t> &header);
    V2FSEQFile(const std::string &fn, CompressionType ct, int cl);

    virtual ~V2FSEQFile();

    virtual void prepareRead(const std::vector<std::pair<uint32_t, uint32_t>> &ranges, uint32_t startFrame = 0) override;
    virtual FrameData *getFrame(uint32_t frame) override;

    virtual void writeHeader() override;
    virtual void addFrame(uint32_t frame,
                          const uint8_t *data) override;
    virtual void finalize() override;

    virtual void dumpInfo(bool indent = false) override;

    virtual uint32_t getMaxChannel() const override;

    virtual void enableMinorVersionFeatures(uint8_t ver) override {
        m_seqVersionMinor = ver;
        if (ver == 0) {
            m_allowExtendedBlocks = false;
        }
        if (ver >= 1) {
            m_allowExtendedBlocks = true;
        }
    }

    CompressionType m_compressionType;
    int             m_compressionLevel;
    std::vector<std::pair<uint32_t, uint32_t>> m_sparseRanges;
    std::vector<std::pair<uint32_t, uint32_t>> m_rangesToRead;
    std::vector<std::pair<uint32_t, uint64_t>> m_frameOffsets;
    uint32_t m_dataBlockSize;
    bool m_allowExtendedBlocks;
private:

    void createHandler();

    V2Handler *m_handler;
    friend class V2Handler;
};

#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>

#include "spdlog/spdlog.h"
#include <argparse/argparse.hpp>
#include <cassert>

#include "git_version.h"

#include "../xlights/FSEQFile.h"

static int fseqMajVersion = 2;
static int fseqMinVersion = 0;
static int compressionLevel = -99;
static std::vector<std::pair<uint32_t, uint32_t>> ranges;
static bool sparse = true;
static FSEQFile::CompressionType compressionType = FSEQFile::CompressionType::zstd;

// Utility function to split a string by a delimiter
std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream stream(str);
    std::string token;
    while (std::getline(stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::debug);
    argparse::ArgumentParser program("fseq_convert", GIT_COMMIT_HASH);
    program.set_prefix_chars("-+/");

    program.add_argument("-i", "--input").required().help("Input fseq File");
    program.add_argument("-o", "--output").help("Ouput fseq File");
    program.add_argument("-c", "--compression").help("Compression type (none|zstd|zlib)");
    program.add_argument("-r", "--ranges").append().help("Ranges, Start Channel-End Channel or Start Channel+Length(1-300|1+100)");
    program.add_argument("-f", "--freq").help("Set fseq version (1|2.0|2.2)");
    program.add_argument("-n", "--nosparse").flag().help("No Sparse Format");
    program.add_argument("-l", "--level").help("Compression level").scan<'i', int>();

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        spdlog::critical( err.what());
        exit(EXIT_FAILURE);
    }

    std::unique_ptr<FSEQFile> src(FSEQFile::openFSEQFile(program.get("-i")));
    if (nullptr == src) {
        spdlog::critical( "Error opening input file: {}", program.get("-i") );
        exit(EXIT_FAILURE);
    }

    if (program.is_used("-f")) {
        std::string const sversion = program.get("-f");
        if (sversion.contains('.')) {
            std::vector<std::string> const versions = split(sversion, '.');
            if (versions.size() != 2) {
                spdlog::critical("Invalid version format: {}", sversion);
                exit(EXIT_FAILURE);
            }
            fseqMajVersion = std::stoi(versions[0]);
            fseqMinVersion = std::stoi(versions[1]);
        } else {
            fseqMajVersion = std::stoi(sversion);
        }
    }

    if (program.is_used("-n")) {
        sparse = false;
    }

    if (program.is_used("-l")) {
        compressionLevel = program.get<int>("-l");
    }

    if (program.is_used("-c")) {
        std::string const compression = program.get("-c");
        if (compression.contains( "none")) {
            compressionType = V2FSEQFile::CompressionType::none;
        } else if (compression.contains("zlib")) {
            compressionType = V2FSEQFile::CompressionType::zlib;
        } else if (compression.contains("zstd")) {
            compressionType = V2FSEQFile::CompressionType::zstd;
        } else {
            spdlog::critical("Unknown compression type: {}", compression);
            exit(EXIT_FAILURE);
        }
    }
    uint32_t channelCount { 0 };
    if (program.is_used("-r")) {
        auto const inranges = program.get<std::vector<std::string>>("-r");
        for (auto const& r : inranges) {
            if (r.empty()) {
                spdlog::critical("Invalid range format: {}", r);
                exit(EXIT_FAILURE);
            }
            if (r.contains('-')) {
                std::vector<std::string> const startEnd = split(r, '-');
                if (startEnd.size() != 2) {
                    spdlog::critical("Invalid range format: {}", r);
                    exit(EXIT_FAILURE);
                }
                uint32_t const start = std::stoul(startEnd[0]);
                uint32_t const end = std::stoul(startEnd[1]);
                ranges.emplace_back(start - 1, end - start + 1);
                channelCount += (end - start + 1);
            } else if (r.contains('+')) {
                std::vector<std::string> const startEnd = split(r, '+');
                if (startEnd.size() != 2) {
                    spdlog::critical("Invalid range format: {}", r);
                    exit(EXIT_FAILURE);
                }
                uint32_t const start = std::stoul(startEnd[0]);
                uint32_t const end = start + std::stoul(startEnd[1]);
                ranges.emplace_back(start - 1, end);
                channelCount += end;
            } else {
                if (r.empty()) {
                    spdlog::critical("Invalid range format: {}", r);
                    exit(EXIT_FAILURE);
                }
            }
            if (ranges.back().first >= src->getChannelCount()) {
                spdlog::critical("Range start {} is out of bounds for the source file with {} channels", ranges.back().first, src->getChannelCount());
                exit(EXIT_FAILURE);
            }
        }
    }
    uint32_t const ogNumber_of_Frames = src->getNumFrames();
    uint32_t const ogNum_Channels = src->getChannelCount();
    int const ogFrame_Rate = src->getStepTime();
    if (ranges.empty()) {
        ranges.push_back(std::pair<uint32_t, uint32_t>(0, ogNum_Channels));
        channelCount = ogNum_Channels;
    }

    if (program.is_used("-o")) {
        std::string const outputFile = program.get("-o");
        std::unique_ptr<FSEQFile> dest(FSEQFile::createFSEQFile(outputFile,
                                                                fseqMajVersion,
                                                                compressionType,
                                                                compressionLevel));
        if (nullptr == dest) {
            spdlog::critical("Failed to create FSEQ file (returned nullptr)!");
            exit(EXIT_FAILURE);
        }
        dest->enableMinorVersionFeatures(fseqMinVersion);

        if (fseqMajVersion == 2 && sparse) {
            V2FSEQFile* f = (V2FSEQFile*)dest.get();
            f->m_sparseRanges = ranges;
        }
        src->prepareRead(ranges);

        dest->initializeFromFSEQ(*src);
        dest->setChannelCount(channelCount);
        dest->writeHeader();

        uint8_t* WriteBuf = new uint8_t[channelCount];

        // read buff
        uint8_t* tmpBuf = new uint8_t[ogNum_Channels];

        uint32_t frame {0};

        while (frame < ogNumber_of_Frames) {
            FSEQFile::FrameData* data = src->getFrame(frame);

            data->readFrame(tmpBuf, ogNum_Channels); // we have a read frame

            uint8_t* destBuf = WriteBuf;

            // Loop through ranges
            for (auto const& [start, count] : ranges) {
                uint8_t* tempSrc = tmpBuf + start;
                memmove(destBuf, tempSrc, count);
                destBuf += count;
            }
            dest->addFrame(frame, WriteBuf);

            delete data;
            frame++;
        }

        dest->finalize();
        delete[] tmpBuf;
        delete[] WriteBuf;
    }
    return 0;
}
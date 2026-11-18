#include <catch2/catch_test_macros.hpp>

#include "render/FSEQFile.h"
#include "support/TempDir.h"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {
using Ranges = std::vector<std::pair<uint32_t, uint32_t>>;

uint8_t Val(uint32_t frame, uint32_t ch) {
    return (uint8_t)((frame * 17 + ch * 5 + (ch >> 7) * 3 + 1) & 0xFF);
}

struct Spec {
    int version = 2;
    FSEQFile::CompressionType ct = FSEQFile::CompressionType::zstd;
    int level = -99;
    uint32_t channels = 64;
    uint32_t frames = 10;
    Ranges sparse;
};

bool Write(const std::string& path, const Spec& s) {
    std::unique_ptr<FSEQFile> f(FSEQFile::createFSEQFile(path, s.version, s.ct, s.level));
    if (!f) return false;
    f->setChannelCount((int)s.channels);
    f->setStepTime(50);
    f->setNumFrames(s.frames);
    if (!s.sparse.empty()) {
        auto* v2 = dynamic_cast<V2FSEQFile*>(f.get());
        if (v2 == nullptr) return false;
        v2->m_sparseRanges = s.sparse;
    }
    f->writeHeader();
    std::vector<uint8_t> d(s.channels);
    for (uint32_t i = 0; i < s.frames; ++i) {
        for (uint32_t c = 0; c < s.channels; ++c) d[c] = Val(i, c);
        f->addFrame(i, d.data());
    }
    f->finalize();
    return true;
}

std::vector<uint8_t> Slurp(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

void Spit(const std::string& path, const std::vector<uint8_t>& d) {
    std::ofstream out(path, std::ios::binary);
    out.write((const char*)d.data(), (std::streamsize)d.size());
}

// Reads one frame and checks every channel in `want` against the pattern (others must be zero).
bool FrameMatches(FSEQFile& f, uint32_t frame, const Ranges& want) {
    std::unique_ptr<FSEQFile::FrameData> fd(f.getFrame(frame));
    if (!fd) return false;
    std::vector<uint8_t> buf(f.getMaxChannel(), 0);
    if (!fd->readFrame(buf.data(), f.getMaxChannel())) return false;
    std::vector<uint8_t> exp(f.getMaxChannel(), 0);
    for (auto& r : want)
        for (uint32_t c = r.first; c < r.first + r.second && c < exp.size(); ++c) exp[c] = Val(frame, c);
    return buf == exp;
}

} // namespace

TEST_CASE("FSEQ bulk reads of a many block zstd file match streaming reads", "[seq2][fseq]") {
    TempDir td;
    Spec s;
    s.channels = 4096;
    s.frames = 220;
    s.level = 1;
    auto p = td.File("bulk.fseq");
    REQUIRE(Write(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f);
    f->setReadPattern(FSEQFile::ReadPattern::Bulk);
    CHECK(f->getReadPattern() == FSEQFile::ReadPattern::Bulk);
    const Ranges all{ { 0, s.channels } };
    f->prepareRead(all, 0);
    bool ok = true;
    for (uint32_t i = 0; i < s.frames && ok; ++i) {
        ok = FrameMatches(*f, i, all);
        if (!ok) FAIL_CHECK("frame " << i);
    }
    // going backwards and jumping far ahead restart the pipeline
    CHECK(FrameMatches(*f, 3, all));
    CHECK(FrameMatches(*f, 200, all));
    CHECK(FrameMatches(*f, 40, all));
    CHECK(FrameMatches(*f, 41, all));
    CHECK(f->getFrame(s.frames) == nullptr);
}

TEST_CASE("FSEQ bulk reads honour a channel sub range", "[seq2][fseq]") {
    TempDir td;
    Spec s;
    s.channels = 4096;
    s.frames = 120;
    auto p = td.File("bulksub.fseq");
    REQUIRE(Write(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f);
    f->setReadPattern(FSEQFile::ReadPattern::Bulk);
    const Ranges sub{ { 100, 50 }, { 3000, 20 } };
    f->prepareRead(sub, 0);
    for (uint32_t i : { 0u, 1u, 30u, 31u, 119u }) {
        INFO("frame " << i);
        std::unique_ptr<FSEQFile::FrameData> fd(f->getFrame(i));
        REQUIRE(fd);
        CHECK(fd->GetSize() == 70);
        std::vector<uint8_t> buf(f->getMaxChannel(), 0);
        REQUIRE(fd->readFrame(buf.data(), f->getMaxChannel()));
        CHECK(buf[100] == Val(i, 100));
        CHECK(buf[149] == Val(i, 149));
        CHECK(buf[3019] == Val(i, 3019));
        CHECK(buf[99] == 0);
        CHECK(buf[150] == 0);
    }
}

TEST_CASE("FSEQ bulk reads of a sparse zstd file", "[seq2][fseq]") {
    TempDir td;
    Spec s;
    s.channels = 6000;
    s.frames = 150;
    s.sparse = { { 10, 2000 }, { 3000, 1500 } };
    auto p = td.File("bulksparse.fseq");
    REQUIRE(Write(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f);
    f->setReadPattern(FSEQFile::ReadPattern::Bulk);
    for (uint32_t i = 0; i < s.frames; i += 7) {
        INFO("frame " << i);
        CHECK(FrameMatches(*f, i, s.sparse));
    }
}

TEST_CASE("FSEQ bulk read pattern on small or non zstd files falls back to streaming", "[seq2][fseq]") {
    TempDir td;
    for (auto ct : { FSEQFile::CompressionType::zstd, FSEQFile::CompressionType::zlib, FSEQFile::CompressionType::none }) {
        INFO("compression " << (int)ct);
        Spec s;
        s.ct = ct;
        s.frames = 5;
        auto p = td.File("small" + std::to_string((int)ct) + ".fseq");
        REQUIRE(Write(p, s));
        std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
        REQUIRE(f);
        f->setReadPattern(FSEQFile::ReadPattern::Bulk);
        for (uint32_t i = 0; i < s.frames; ++i) {
            CHECK(FrameMatches(*f, i, { { 0, s.channels } }));
        }
    }
}

TEST_CASE("FSEQ out of range compression levels fall back to the default", "[seq2][fseq]") {
    TempDir td;
    for (auto ct : { FSEQFile::CompressionType::zstd, FSEQFile::CompressionType::zlib }) {
        for (int level : { 99, -99 + 1, 30 }) {
            INFO("compression " << (int)ct << " level " << level);
            Spec s;
            s.ct = ct;
            s.level = level;
            s.frames = 25;
            s.channels = 300;
            auto p = td.File("lvl.fseq");
            REQUIRE(Write(p, s));
            std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
            REQUIRE(f);
            for (uint32_t i = 0; i < s.frames; ++i) CHECK(FrameMatches(*f, i, { { 0, s.channels } }));
        }
    }
}

TEST_CASE("FSEQ zlib file read out of order and on block edges", "[seq2][fseq]") {
    TempDir td;
    Spec s;
    s.ct = FSEQFile::CompressionType::zlib;
    s.channels = 9000;
    s.frames = 60;
    auto p = td.File("z.fseq");
    REQUIRE(Write(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f);
    for (uint32_t i : { 59u, 0u, 9u, 10u, 11u, 30u, 29u, 58u }) {
        INFO("frame " << i);
        CHECK(FrameMatches(*f, i, { { 0, s.channels } }));
    }
}

TEST_CASE("FSEQ dumpInfo describes every compression type", "[seq2][fseq]") {
    TempDir td;
    for (auto ct : { FSEQFile::CompressionType::none, FSEQFile::CompressionType::zstd, FSEQFile::CompressionType::zlib }) {
        Spec s;
        s.ct = ct;
        auto p = td.File("d.fseq");
        REQUIRE(Write(p, s));
        std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
        REQUIRE(f);
        f->dumpInfo(true);
        f->dumpInfo(false);
        auto* v2 = dynamic_cast<V2FSEQFile*>(f.get());
        REQUIRE(v2);
        CHECK(v2->CompressionTypeString() == FSEQFile::CompressionTypeStrings[(int)ct]);
    }
    Spec v1;
    v1.version = 1;
    auto p = td.File("v1.fseq");
    REQUIRE(Write(p, v1));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f);
    f->dumpInfo();
    CHECK(f->getVersionMajor() == 1);
}

TEST_CASE("FSEQ with an unknown compression nibble or minor version still opens", "[seq2][fseq]") {
    TempDir td;
    Spec s;
    s.ct = FSEQFile::CompressionType::none;
    auto p = td.File("u.fseq");
    REQUIRE(Write(p, s));
    auto bytes = Slurp(p);
    REQUIRE(bytes.size() > 32);
    bytes[20] = (uint8_t)((bytes[20] & 0xF0) | 0x7);
    bytes[6] = 9;
    auto q = td.File("u2.fseq");
    Spit(q, bytes);
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(q));
    REQUIRE(f);
    CHECK(f->getVersionMinor() == 9);
    auto* v2 = dynamic_cast<V2FSEQFile*>(f.get());
    REQUIRE(v2);
    CHECK(v2->m_compressionType == FSEQFile::CompressionType::none);
    for (uint32_t i = 0; i < s.frames; ++i) CHECK(FrameMatches(*f, i, { { 0, s.channels } }));
}

TEST_CASE("FSEQ v1 prepareRead with a sub range reads only those channels", "[seq2][fseq]") {
    TempDir td;
    Spec s;
    s.version = 1;
    s.channels = 128;
    s.frames = 6;
    auto p = td.File("v1sub.fseq");
    REQUIRE(Write(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f);
    const Ranges sub{ { 8, 4 }, { 100, 28 } };
    f->prepareRead(sub, 2);
    for (uint32_t i = 0; i < s.frames; ++i) {
        INFO("frame " << i);
        std::unique_ptr<FSEQFile::FrameData> fd(f->getFrame(i));
        REQUIRE(fd);
        CHECK(fd->GetSize() == 32);
        CHECK(FrameMatches(*f, i, sub));
    }
}

TEST_CASE("FSEQ v1 getFrame beyond the last frame returns null", "[seq2]") {
    TempDir td;
    Spec s;
    s.version = 1;
    auto p = td.File("v1end.fseq");
    REQUIRE(Write(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f);
    std::unique_ptr<FSEQFile::FrameData> fd(f->getFrame(s.frames + 5));
    CHECK(fd == nullptr);
}

namespace {
struct MemFile : V2FSEQFile {
    using V2FSEQFile::V2FSEQFile;
    using FSEQFile::seek;
    using FSEQFile::tell;
    using FSEQFile::write;
};
}

TEST_CASE("FSEQ in memory writer supports relative and end seeks", "[seq2][fseq]") {
    MemFile f("-memory-", FSEQFile::CompressionType::none, 0);
    const char a[] = "abcd";
    f.write(a, 4);
    CHECK(f.tell() == 4);
    f.seek(2, SEEK_CUR);
    CHECK(f.tell() == 6);
    f.write(a, 2);
    CHECK(f.getMemoryBuffer().size() == 8);
    f.seek(0, SEEK_END);
    CHECK(f.tell() == 8);
    f.seek(1, SEEK_SET);
    CHECK(f.getMemoryBufferPos() == 1);
}

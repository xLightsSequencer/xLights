#include <catch2/catch_test_macros.hpp>

#include "render/FSEQFile.h"
#include "support/TempDir.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {
using Ranges = std::vector<std::pair<uint32_t, uint32_t>>;

uint8_t Pattern(uint32_t frame, uint32_t ch) {
    return (uint8_t)((frame * 31 + ch * 7 + (ch >> 8) * 13 + 3) & 0xFF);
}

struct WriteSpec {
    int version = 2;
    FSEQFile::CompressionType ct = FSEQFile::CompressionType::zstd;
    int level = -99;
    uint32_t channels = 64;
    uint32_t frames = 10;
    int step = 25;
    Ranges sparse;
    int minor = -1;
    std::vector<FSEQFile::VariableHeader> headers;
};

FSEQFile::VariableHeader StringHeader(char a, char b, const std::string& s) {
    FSEQFile::VariableHeader h;
    h.code[0] = a;
    h.code[1] = b;
    h.data.assign(s.begin(), s.end());
    h.data.push_back(0);
    return h;
}

std::vector<uint8_t> MakeFrame(const WriteSpec& s, uint32_t frame, uint32_t width) {
    std::vector<uint8_t> f(width, 0);
    for (uint32_t c = 0; c < width; ++c) {
        f[c] = Pattern(frame, c);
    }
    return f;
}

bool WriteFile(const std::string& path, const WriteSpec& s) {
    std::unique_ptr<FSEQFile> f(FSEQFile::createFSEQFile(path, s.version, s.ct, s.level));
    if (!f) {
        return false;
    }
    if (s.minor >= 0) {
        f->enableMinorVersionFeatures((uint8_t)s.minor);
    }
    f->setChannelCount((int)s.channels);
    f->setStepTime(s.step);
    f->setNumFrames(s.frames);
    for (const auto& h : s.headers) {
        f->addVariableHeader(h);
    }
    if (!s.sparse.empty()) {
        auto* v2 = dynamic_cast<V2FSEQFile*>(f.get());
        if (v2 == nullptr) {
            return false;
        }
        v2->m_sparseRanges = s.sparse;
    }
    f->writeHeader();
    for (uint32_t i = 0; i < s.frames; ++i) {
        auto data = MakeFrame(s, i, s.channels);
        f->addFrame(i, data.data());
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

bool ReadFrameInto(FSEQFile& f, uint32_t frame, std::vector<uint8_t>& out) {
    std::unique_ptr<FSEQFile::FrameData> fd(f.getFrame(frame));
    if (!fd) {
        return false;
    }
    out.assign(f.getMaxChannel(), 0);
    return fd->readFrame(out.data(), f.getMaxChannel());
}

void CheckAllFrames(const std::string& path, const WriteSpec& s, const Ranges& expectedRanges = {}) {
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(path));
    REQUIRE(f != nullptr);
    CHECK(f->getNumFrames() == s.frames);
    CHECK(f->getStepTime() == s.step);
    std::vector<uint8_t> got;
    for (uint32_t i = 0; i < s.frames; ++i) {
        INFO("frame " << i);
        REQUIRE(ReadFrameInto(*f, i, got));
        std::vector<uint8_t> want(f->getMaxChannel(), 0);
        if (expectedRanges.empty()) {
            for (uint32_t c = 0; c < s.channels; ++c) {
                want[c] = Pattern(i, c);
            }
        } else {
            for (auto& r : expectedRanges) {
                for (uint32_t c = r.first; c < r.first + r.second; ++c) {
                    want[c] = Pattern(i, c);
                }
            }
        }
        REQUIRE(got.size() == want.size());
        CHECK(got == want);
    }
}
}

TEST_CASE("FSEQ v2 uncompressed round trip", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.ct = FSEQFile::CompressionType::none;
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    CheckAllFrames(p, s);
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    CHECK(f->getVersionMajor() == 2);
    CHECK(f->getChannelCount() == 64);
    auto* v2 = dynamic_cast<V2FSEQFile*>(f.get());
    REQUIRE(v2 != nullptr);
    CHECK(v2->m_compressionType == FSEQFile::CompressionType::none);
    CHECK(v2->CompressionTypeString() == "none");
}

TEST_CASE("FSEQ v2 zstd round trip", "[fseq]") {
    TempDir td;
    WriteSpec s;
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    CheckAllFrames(p, s);
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    auto* v2 = dynamic_cast<V2FSEQFile*>(f.get());
    REQUIRE(v2 != nullptr);
    CHECK(v2->m_compressionType == FSEQFile::CompressionType::zstd);
}

TEST_CASE("FSEQ v2 zlib round trip", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.ct = FSEQFile::CompressionType::zlib;
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    CheckAllFrames(p, s);
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    auto* v2 = dynamic_cast<V2FSEQFile*>(f.get());
    REQUIRE(v2 != nullptr);
    CHECK(v2->m_compressionType == FSEQFile::CompressionType::zlib);
}

TEST_CASE("FSEQ v1 round trip", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.version = 1;
    auto p = td.File("v1.fseq");
    REQUIRE(WriteFile(p, s));
    CheckAllFrames(p, s);
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    CHECK(f->getVersionMajor() == 1);
    CHECK(f->getChannelCount() == 64);
    CHECK(f->getMaxChannel() == 64);
}

TEST_CASE("FSEQ unknown version cannot be created", "[fseq]") {
    TempDir td;
    CHECK(FSEQFile::createFSEQFile(td.File("x.fseq"), 3) == nullptr);
    CHECK(FSEQFile::createFSEQFile(td.File("x.fseq"), 0) == nullptr);
}

TEST_CASE("FSEQ create fails when the file cannot be opened", "[fseq]") {
    TempDir td;
    CHECK(FSEQFile::createFSEQFile(td.File("nodir/x.fseq"), 2) == nullptr);
    CHECK(FSEQFile::createFSEQFile(td.File("nodir/x.fseq"), 1) == nullptr);
}

TEST_CASE("FSEQ header fields round trip", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.channels = 100;
    s.frames = 37;
    s.step = 33;
    auto p = td.File("h.fseq");
    REQUIRE(WriteFile(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f != nullptr);
    CHECK(f->getNumFrames() == 37);
    CHECK(f->getStepTime() == 33);
    CHECK(f->getChannelCount() == 100);
    CHECK(f->getTotalTimeMS() == 37u * 33u);
    CHECK(f->getUniqueId() != 0);
}

TEST_CASE("FSEQ frame time math for common frame intervals", "[fseq]") {
    TempDir td;
    for (int step : { 10, 20, 25, 33, 50, 100 }) {
        WriteSpec s;
        s.frames = 40;
        s.step = step;
        s.channels = 8;
        auto p = td.File("t" + std::to_string(step) + ".fseq");
        REQUIRE(WriteFile(p, s));
        std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
        REQUIRE(f != nullptr);
        CHECK(f->getTotalTimeMS() == 40u * (uint32_t)step);
    }
}

TEST_CASE("FSEQ v2 zero frames still produces a readable file", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.frames = 0;
    auto p = td.File("z.fseq");
    REQUIRE(WriteFile(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f != nullptr);
    CHECK(f->getNumFrames() == 0);
    CHECK(f->getTotalTimeMS() == 0);
    CHECK(f->getFrame(0) == nullptr);
}

TEST_CASE("FSEQ v2 single frame", "[fseq]") {
    TempDir td;
    for (auto ct : { FSEQFile::CompressionType::none, FSEQFile::CompressionType::zstd, FSEQFile::CompressionType::zlib }) {
        WriteSpec s;
        s.ct = ct;
        s.frames = 1;
        auto p = td.File("one" + std::to_string((int)ct) + ".fseq");
        REQUIRE(WriteFile(p, s));
        CheckAllFrames(p, s);
    }
}

TEST_CASE("FSEQ getFrame beyond the last frame returns null", "[fseq]") {
    TempDir td;
    WriteSpec s;
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f != nullptr);
    CHECK(f->getFrame(s.frames) == nullptr);
    CHECK(f->getFrame(s.frames + 1000) == nullptr);
    CHECK(f->getFrame(0xFFFFFFFFu) == nullptr);
}

TEST_CASE("FSEQ v2 compressed file with many blocks reads every frame", "[fseq]") {
    TempDir td;
    for (auto ct : { FSEQFile::CompressionType::zstd, FSEQFile::CompressionType::zlib }) {
        WriteSpec s;
        s.ct = ct;
        s.channels = 20000;
        s.frames = 80;
        auto p = td.File("big" + std::to_string((int)ct) + ".fseq");
        REQUIRE(WriteFile(p, s));
        CheckAllFrames(p, s);
        std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
        auto* v2 = dynamic_cast<V2FSEQFile*>(f.get());
        REQUIRE(v2 != nullptr);
        CHECK(v2->m_frameOffsets.size() > 3);
    }
}

TEST_CASE("FSEQ v2 compressed frames can be read out of order", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.channels = 20000;
    s.frames = 60;
    auto p = td.File("ooo.fseq");
    REQUIRE(WriteFile(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f != nullptr);
    std::vector<uint8_t> got;
    for (uint32_t frame : { 59u, 0u, 30u, 31u, 1u, 58u, 30u }) {
        INFO("frame " << frame);
        REQUIRE(ReadFrameInto(*f, frame, got));
        CHECK(got == MakeFrame(s, frame, 20000));
    }
}

TEST_CASE("FSEQ v2 compression levels", "[fseq]") {
    TempDir td;
    for (int level : { -99, 1, 3, 10, 22 }) {
        WriteSpec s;
        s.level = level;
        s.channels = 500;
        s.frames = 20;
        auto p = td.File("lvl" + std::to_string(level + 100) + ".fseq");
        REQUIRE(WriteFile(p, s));
        CheckAllFrames(p, s);
    }
}

TEST_CASE("FSEQ v2 sparse ranges round trip", "[fseq]") {
    TempDir td;
    Ranges ranges = { { 4, 8 }, { 20, 5 }, { 40, 24 } };
    for (auto ct : { FSEQFile::CompressionType::none, FSEQFile::CompressionType::zstd, FSEQFile::CompressionType::zlib }) {
        WriteSpec s;
        s.ct = ct;
        s.sparse = ranges;
        auto p = td.File("sp" + std::to_string((int)ct) + ".fseq");
        REQUIRE(WriteFile(p, s));
        std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
        REQUIRE(f != nullptr);
        auto* v2 = dynamic_cast<V2FSEQFile*>(f.get());
        REQUIRE(v2 != nullptr);
        CHECK(v2->m_sparseRanges == ranges);
        CHECK(f->getChannelCount() == 8u + 5u + 24u);
        CHECK(f->getMaxChannel() == 64);
        CheckAllFrames(p, s, ranges);
    }
}

TEST_CASE("FSEQ v2 sparse ranges beyond the channel count are trimmed", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.channels = 50;
    s.sparse = { { 10, 10 }, { 45, 20 }, { 80, 5 } };
    auto p = td.File("trim.fseq");
    REQUIRE(WriteFile(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f != nullptr);
    auto* v2 = dynamic_cast<V2FSEQFile*>(f.get());
    REQUIRE(v2 != nullptr);
    REQUIRE(v2->m_sparseRanges.size() == 2);
    CHECK(v2->m_sparseRanges[0] == std::make_pair(10u, 10u));
    CHECK(v2->m_sparseRanges[1] == std::make_pair(45u, 5u));
    CHECK(f->getChannelCount() == 15);
}

TEST_CASE("FSEQ v2 sparse single range at channel zero", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.sparse = { { 0, 16 } };
    auto p = td.File("one.fseq");
    REQUIRE(WriteFile(p, s));
    CheckAllFrames(p, s, s.sparse);
}

TEST_CASE("FSEQ v2 sparse read leaves channels outside the ranges untouched", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.sparse = { { 8, 8 } };
    auto p = td.File("untouched.fseq");
    REQUIRE(WriteFile(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f != nullptr);
    std::unique_ptr<FSEQFile::FrameData> fd(f->getFrame(2));
    REQUIRE(fd != nullptr);
    std::vector<uint8_t> buf(f->getMaxChannel(), 0xEE);
    REQUIRE(fd->readFrame(buf.data(), f->getMaxChannel()));
    for (uint32_t c = 0; c < 8; ++c) {
        CHECK(buf[c] == 0xEE);
    }
    for (uint32_t c = 8; c < 16; ++c) {
        CHECK(buf[c] == Pattern(2, c));
    }
    CHECK(fd->GetSize() == 8);
}

TEST_CASE("FSEQ v2 sparse file is smaller than the dense equivalent", "[fseq]") {
    TempDir td;
    WriteSpec dense;
    dense.ct = FSEQFile::CompressionType::none;
    dense.channels = 400;
    dense.frames = 20;
    WriteSpec sparse = dense;
    sparse.sparse = { { 100, 50 } };
    REQUIRE(WriteFile(td.File("dense.fseq"), dense));
    REQUIRE(WriteFile(td.File("sparse.fseq"), sparse));
    CHECK(Slurp(td.File("sparse.fseq")).size() < Slurp(td.File("dense.fseq")).size());
}

TEST_CASE("FSEQ v2 prepareRead with a sub range on an uncompressed dense file", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.ct = FSEQFile::CompressionType::none;
    s.channels = 200;
    auto p = td.File("sub.fseq");
    REQUIRE(WriteFile(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f != nullptr);
    f->prepareRead({ { 50, 20 }, { 100, 10 } }, 0);
    std::unique_ptr<FSEQFile::FrameData> fd(f->getFrame(3));
    REQUIRE(fd != nullptr);
    CHECK(fd->GetSize() == 30);
    std::vector<uint8_t> buf(200, 0);
    REQUIRE(fd->readFrame(buf.data(), 200));
    for (uint32_t c = 50; c < 70; ++c) {
        CHECK(buf[c] == Pattern(3, c));
    }
    for (uint32_t c = 100; c < 110; ++c) {
        CHECK(buf[c] == Pattern(3, c));
    }
    CHECK(buf[0] == 0);
    CHECK(buf[80] == 0);
}

TEST_CASE("FSEQ v1 prepareRead clamps ranges to the channel count", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.version = 1;
    s.channels = 40;
    auto p = td.File("v1c.fseq");
    REQUIRE(WriteFile(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f != nullptr);
    f->prepareRead({ { 30, 100 } }, 0);
    std::unique_ptr<FSEQFile::FrameData> fd(f->getFrame(1));
    REQUIRE(fd != nullptr);
    CHECK(fd->GetSize() == 10);
    std::vector<uint8_t> buf(40, 0);
    REQUIRE(fd->readFrame(buf.data(), 40));
    for (uint32_t c = 30; c < 40; ++c) {
        CHECK(buf[c] == Pattern(1, c));
    }
}

TEST_CASE("FSEQ variable headers round trip for v1 and v2", "[fseq]") {
    TempDir td;
    for (int version : { 1, 2 }) {
        WriteSpec s;
        s.version = version;
        s.headers.push_back(StringHeader('m', 'f', "C:/music/song.mp3"));
        s.headers.push_back(StringHeader('s', 'p', "xLights 2025.01"));
        FSEQFile::VariableHeader bin;
        bin.code[0] = 'F';
        bin.code[1] = 'C';
        bin.data = { 1, 2, 3, 4, 5, 0, 255 };
        s.headers.push_back(bin);
        auto p = td.File("vh" + std::to_string(version) + ".fseq");
        REQUIRE(WriteFile(p, s));

        std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
        REQUIRE(f != nullptr);
        auto& hs = f->getVariableHeaders();
        REQUIRE(hs.size() == 3);
        CHECK(hs[0].code[0] == 'm');
        CHECK(hs[0].code[1] == 'f');
        CHECK(std::string((const char*)hs[0].data.data()) == "C:/music/song.mp3");
        CHECK(std::string((const char*)hs[1].data.data()) == "xLights 2025.01");
        CHECK(hs[2].data == bin.data);
        CHECK(f->getMediaFilename() == "C:/music/song.mp3");
        CHECK(FSEQFile::getMediaFilename(p) == "C:/music/song.mp3");
        CheckAllFrames(p, s);
    }
}

TEST_CASE("FSEQ getMediaFilename is empty when absent, blank or the file is missing", "[fseq]") {
    TempDir td;
    WriteSpec s;
    auto p = td.File("nomf.fseq");
    REQUIRE(WriteFile(p, s));
    CHECK(FSEQFile::getMediaFilename(p).empty());
    CHECK(FSEQFile::getMediaFilename(td.File("missing.fseq")).empty());

    WriteSpec blank;
    blank.headers.push_back(StringHeader('m', 'f', ""));
    auto p2 = td.File("blank.fseq");
    REQUIRE(WriteFile(p2, blank));
    CHECK(FSEQFile::getMediaFilename(p2).empty());
}

TEST_CASE("FSEQ extended variable headers are stored outside the fixed header", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.minor = 2;
    FSEQFile::VariableHeader big;
    big.code[0] = 'X';
    big.code[1] = 'R';
    big.extendedData = true;
    big.data.resize(5000);
    for (size_t i = 0; i < big.data.size(); ++i) {
        big.data[i] = (uint8_t)(i * 7);
    }
    s.headers.push_back(StringHeader('s', 'p', "x"));
    s.headers.push_back(big);
    auto p = td.File("ext.fseq");
    REQUIRE(WriteFile(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f != nullptr);
    auto& hs = f->getVariableHeaders();
    REQUIRE(hs.size() == 2);
    CHECK(hs[1].code[0] == 'X');
    CHECK(hs[1].extendedData);
    CHECK(hs[1].data == big.data);
    CheckAllFrames(p, s);
}

TEST_CASE("FSEQ very large headers are forced out of the fixed header", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.minor = 2;
    FSEQFile::VariableHeader big;
    big.code[0] = 'X';
    big.code[1] = 'N';
    big.data.assign(70000, 0x5A);
    s.headers.push_back(big);
    auto p = td.File("force.fseq");
    REQUIRE(WriteFile(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f != nullptr);
    REQUIRE(f->getVariableHeaders().size() == 1);
    CHECK(f->getVariableHeaders()[0].data == big.data);
    CheckAllFrames(p, s);
}

TEST_CASE("FSEQ removeVariableHeader removes only the first match", "[fseq]") {
    TempDir td;
    std::unique_ptr<FSEQFile> f(FSEQFile::createFSEQFile(td.File("r.fseq"), 2));
    REQUIRE(f != nullptr);
    f->addVariableHeader(StringHeader('m', 'f', "a"));
    f->addVariableHeader(StringHeader('s', 'p', "b"));
    f->addVariableHeader(StringHeader('m', 'f', "c"));
    f->removeVariableHeader('m', 'f');
    REQUIRE(f->getVariableHeaders().size() == 2);
    CHECK(f->getVariableHeaders()[0].code[0] == 's');
    f->removeVariableHeader('z', 'z');
    CHECK(f->getVariableHeaders().size() == 2);
}

TEST_CASE("FSEQ v2 more than 255 sparse ranges with minor version 3 use an SR block", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.minor = 3;
    s.channels = 3000;
    s.frames = 4;
    for (uint32_t i = 0; i < 300; ++i) {
        s.sparse.push_back({ i * 10, 5 });
    }
    auto p = td.File("sr.fseq");
    REQUIRE(WriteFile(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f != nullptr);
    auto* v2 = dynamic_cast<V2FSEQFile*>(f.get());
    REQUIRE(v2 != nullptr);
    CHECK(v2->m_sparseRanges == s.sparse);
    CHECK(f->getChannelCount() == 1500);
    for (auto& h : f->getVariableHeaders()) {
        CHECK_FALSE((h.code[0] == 'S' && h.code[1] == 'R'));
    }
    CheckAllFrames(p, s, s.sparse);
}

TEST_CASE("FSEQ v2 more than 255 sparse ranges below minor 3 are merged to fit", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.channels = 3000;
    s.frames = 3;
    for (uint32_t i = 0; i < 300; ++i) {
        s.sparse.push_back({ i * 10, 5 });
    }
    auto p = td.File("merge.fseq");
    REQUIRE(WriteFile(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f != nullptr);
    auto* v2 = dynamic_cast<V2FSEQFile*>(f.get());
    REQUIRE(v2 != nullptr);
    CHECK(v2->m_sparseRanges.size() == 255);
    for (auto& r : s.sparse) {
        bool covered = false;
        for (auto& m : v2->m_sparseRanges) {
            if (r.first >= m.first && r.first + r.second <= m.first + m.second) {
                covered = true;
                break;
            }
        }
        CHECK(covered);
    }
}

TEST_CASE("FSEQ fitSparseRangesToHeader leaves short tables alone", "[fseq]") {
    Ranges r = { { 10, 5 }, { 0, 5 } };
    Ranges copy = r;
    V2FSEQFile::fitSparseRangesToHeader(r);
    CHECK(r == copy);
}

TEST_CASE("FSEQ fitSparseRangesToHeader merges across the smallest gaps", "[fseq]") {
    Ranges r;
    for (uint32_t i = 0; i < 256; ++i) {
        r.push_back({ i * 100, 10 });
    }
    r[100].first = r[99].first + 10 + 3;
    V2FSEQFile::fitSparseRangesToHeader(r);
    REQUIRE(r.size() == 255);
    bool merged = false;
    for (auto& a : r) {
        if (a.first == 9900 && a.second == 13) {
            merged = true;
        }
    }
    CHECK_FALSE(merged);
    uint32_t cover = 0;
    for (auto& a : r) {
        cover += a.second;
    }
    CHECK(cover >= 256u * 10u - 10u);
    CHECK(std::is_sorted(r.begin(), r.end()));
}

TEST_CASE("FSEQ fitSparseRangesToHeader sorts unsorted input and handles overlap", "[fseq]") {
    Ranges r;
    for (uint32_t i = 0; i < 300; ++i) {
        r.push_back({ (299 - i) * 20, 25 });
    }
    V2FSEQFile::fitSparseRangesToHeader(r);
    CHECK(r.size() == 255);
    CHECK(std::is_sorted(r.begin(), r.end()));
    CHECK(r.front().first == 0);
    CHECK(r.back().first + r.back().second == 299u * 20u + 25u);
}

TEST_CASE("FSEQ unique id is preserved on rewrite when set", "[fseq]") {
    TempDir td;
    std::unique_ptr<FSEQFile> a;
    {
        WriteSpec s;
        auto p = td.File("a.fseq");
        REQUIRE(WriteFile(p, s));
        a.reset(FSEQFile::openFSEQFile(p));
        REQUIRE(a != nullptr);
    }
    std::unique_ptr<FSEQFile> b(FSEQFile::createFSEQFile(td.File("b.fseq"), 2, FSEQFile::CompressionType::none));
    REQUIRE(b != nullptr);
    b->initializeFromFSEQ(*a);
    CHECK(b->getUniqueId() == a->getUniqueId());
    CHECK(b->getNumFrames() == a->getNumFrames());
    CHECK(b->getStepTime() == a->getStepTime());
    CHECK(b->getChannelCount() == a->getChannelCount());
}

TEST_CASE("FSEQ initializeFromFSEQ widens the channel count to cover sparse ranges", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.sparse = { { 40, 10 } };
    auto p = td.File("sparse.fseq");
    REQUIRE(WriteFile(p, s));
    std::unique_ptr<FSEQFile> a(FSEQFile::openFSEQFile(p));
    REQUIRE(a != nullptr);
    CHECK(a->getChannelCount() == 10);
    std::unique_ptr<FSEQFile> b(FSEQFile::createFSEQFile(td.File("b.fseq"), 2));
    REQUIRE(b != nullptr);
    b->initializeFromFSEQ(*a);
    CHECK(b->getChannelCount() == 50);
}

TEST_CASE("FSEQ in-memory writer produces a valid file", "[fseq]") {
    TempDir td;
    std::unique_ptr<FSEQFile> f(new V2FSEQFile("-memory-", FSEQFile::CompressionType::zstd, -99));
    f->setChannelCount(32);
    f->setNumFrames(5);
    f->setStepTime(50);
    f->writeHeader();
    WriteSpec s;
    for (uint32_t i = 0; i < 5; ++i) {
        auto d = MakeFrame(s, i, 32);
        f->addFrame(i, d.data());
    }
    f->finalize();
    auto buf = f->getMemoryBuffer();
    REQUIRE(buf.size() > 32);
    CHECK(buf[0] == 'P');
    CHECK(buf[1] == 'S');
    CHECK(buf[2] == 'E');
    CHECK(buf[3] == 'Q');
    Spit(td.File("mem.fseq"), buf);
    s.channels = 32;
    s.frames = 5;
    s.step = 50;
    CheckAllFrames(td.File("mem.fseq"), s);
}

TEST_CASE("FSEQ written header has the documented magic version and offset", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.ct = FSEQFile::CompressionType::none;
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    auto b = Slurp(p);
    REQUIRE(b.size() > 32);
    CHECK(std::string((const char*)b.data(), 4) == "PSEQ");
    uint32_t offset = b[4] | (b[5] << 8);
    CHECK(offset % 4 == 0);
    CHECK(offset >= 32);
    CHECK(b[7] == 2);
    CHECK(b[6] == 0);
    uint32_t chans = b[10] | (b[11] << 8) | (b[12] << 16) | ((uint32_t)b[13] << 24);
    uint32_t frames = b[14] | (b[15] << 8) | (b[16] << 16) | ((uint32_t)b[17] << 24);
    CHECK(chans == 64);
    CHECK(frames == 10);
    CHECK(b[18] == 25);
    CHECK(b[20] == 0);
    CHECK(b.size() == offset + 64u * 10u);
}

TEST_CASE("FSEQ v1 written file layout", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.version = 1;
    s.channels = 12;
    s.frames = 3;
    auto p = td.File("v1.fseq");
    REQUIRE(WriteFile(p, s));
    auto b = Slurp(p);
    CHECK(std::string((const char*)b.data(), 4) == "PSEQ");
    CHECK(b[7] == 1);
    uint32_t offset = b[4] | (b[5] << 8);
    CHECK(b.size() == offset + 12u * 3u);
    CHECK(b[offset] == Pattern(0, 0));
    CHECK(b[offset + 12] == Pattern(1, 0));
}

TEST_CASE("FSEQ open fails for a missing file", "[fseq][corrupt]") {
    TempDir td;
    CHECK(FSEQFile::openFSEQFile(td.File("nope.fseq")) == nullptr);
}

TEST_CASE("FSEQ open fails for an empty file", "[fseq][corrupt]") {
    TempDir td;
    Spit(td.File("e.fseq"), {});
    CHECK(FSEQFile::openFSEQFile(td.File("e.fseq")) == nullptr);
}

TEST_CASE("FSEQ open fails for files shorter than the initial header read", "[fseq][corrupt]") {
    TempDir td;
    for (size_t n = 1; n < 8; ++n) {
        Spit(td.File("s.fseq"), std::vector<uint8_t>(n, 'P'));
        CHECK(FSEQFile::openFSEQFile(td.File("s.fseq")) == nullptr);
    }
}

TEST_CASE("FSEQ open fails for a wrong magic", "[fseq][corrupt]") {
    TempDir td;
    std::vector<uint8_t> junk(64, 0);
    std::memcpy(junk.data(), "XXXX", 4);
    Spit(td.File("m.fseq"), junk);
    CHECK(FSEQFile::openFSEQFile(td.File("m.fseq")) == nullptr);
    std::string text = "this is a text file that is definitely not an fseq file at all";
    Spit(td.File("t.fseq"), std::vector<uint8_t>(text.begin(), text.end()));
    CHECK(FSEQFile::openFSEQFile(td.File("t.fseq")) == nullptr);
}

TEST_CASE("FSEQ open fails for an unsupported major version", "[fseq][corrupt]") {
    TempDir td;
    WriteSpec s;
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    auto b = Slurp(p);
    for (uint8_t ver : { (uint8_t)0, (uint8_t)3, (uint8_t)9, (uint8_t)255 }) {
        b[7] = ver;
        Spit(td.File("v.fseq"), b);
        CHECK(FSEQFile::openFSEQFile(td.File("v.fseq")) == nullptr);
    }
}

TEST_CASE("FSEQ open fails for a channel data offset that is too small", "[fseq][corrupt]") {
    TempDir td;
    WriteSpec s;
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    auto b = Slurp(p);
    for (uint16_t off : { (uint16_t)0, (uint16_t)7, (uint16_t)8, (uint16_t)20, (uint16_t)31 }) {
        b[4] = off & 0xFF;
        b[5] = off >> 8;
        Spit(td.File("o.fseq"), b);
        CHECK(FSEQFile::openFSEQFile(td.File("o.fseq")) == nullptr);
    }
}

TEST_CASE("FSEQ open fails when the header is cut short", "[fseq][corrupt]") {
    TempDir td;
    WriteSpec s;
    s.headers.push_back(StringHeader('m', 'f', "media.mp3"));
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    auto b = Slurp(p);
    uint32_t offset = b[4] | (b[5] << 8);
    REQUIRE(offset > 32);
    for (size_t cut : { (size_t)8, (size_t)16, (size_t)31, (size_t)offset - 1 }) {
        std::vector<uint8_t> t(b.begin(), b.begin() + cut);
        Spit(td.File("c.fseq"), t);
        CHECK(FSEQFile::openFSEQFile(td.File("c.fseq")) == nullptr);
    }
}

TEST_CASE("FSEQ truncated channel data does not crash any compression type", "[fseq][corrupt]") {
    TempDir td;
    for (auto ct : { FSEQFile::CompressionType::none, FSEQFile::CompressionType::zstd, FSEQFile::CompressionType::zlib }) {
        WriteSpec s;
        s.ct = ct;
        s.channels = 4000;
        s.frames = 30;
        auto p = td.File("a" + std::to_string((int)ct) + ".fseq");
        REQUIRE(WriteFile(p, s));
        auto b = Slurp(p);
        for (size_t divisor : { (size_t)2, (size_t)3, (size_t)10 }) {
            std::vector<uint8_t> t(b.begin(), b.begin() + b.size() / divisor);
            auto cp = td.File("cut.fseq");
            Spit(cp, t);
            std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(cp));
            if (f) {
                for (uint32_t i = 0; i < f->getNumFrames(); ++i) {
                    std::unique_ptr<FSEQFile::FrameData> fd(f->getFrame(i));
                    if (fd) {
                        std::vector<uint8_t> buf(f->getMaxChannel(), 0);
                        fd->readFrame(buf.data(), f->getMaxChannel());
                    }
                }
            }
        }
    }
    SUCCEED();
}

TEST_CASE("FSEQ corrupt compressed data fails the frame read gracefully", "[fseq][corrupt]") {
    TempDir td;
    for (auto ct : { FSEQFile::CompressionType::zstd, FSEQFile::CompressionType::zlib }) {
        WriteSpec s;
        s.ct = ct;
        s.channels = 2000;
        s.frames = 20;
        auto p = td.File("a.fseq");
        REQUIRE(WriteFile(p, s));
        auto b = Slurp(p);
        uint32_t offset = b[4] | (b[5] << 8);
        for (size_t i = offset; i < b.size(); ++i) {
            b[i] = (uint8_t)(0xA5 ^ (i * 13));
        }
        auto cp = td.File("garbage.fseq");
        Spit(cp, b);
        std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(cp));
        REQUIRE(f != nullptr);
        for (uint32_t i = 0; i < f->getNumFrames(); ++i) {
            std::unique_ptr<FSEQFile::FrameData> fd(f->getFrame(i));
            if (fd) {
                std::vector<uint8_t> buf(f->getMaxChannel(), 0);
                fd->readFrame(buf.data(), f->getMaxChannel());
                CHECK(buf != MakeFrame(s, i, 2000));
            }
        }
    }
}

TEST_CASE("FSEQ corrupt block table does not crash", "[fseq][corrupt]") {
    TempDir td;
    WriteSpec s;
    s.channels = 20000;
    s.frames = 60;
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    auto orig = Slurp(p);
    uint32_t maxBlocks = ((orig[20] & 0xF0) << 4) | orig[21];
    REQUIRE(maxBlocks > 1);

    SECTION("out of order start frames") {
        auto b = orig;
        b[32 + 8] = 200;
        b[32 + 16] = 5;
        Spit(td.File("c.fseq"), b);
    }
    SECTION("huge block lengths") {
        auto b = orig;
        for (uint32_t i = 0; i < maxBlocks; ++i) {
            b[32 + i * 8 + 4] = 0xFF;
            b[32 + i * 8 + 5] = 0xFF;
            b[32 + i * 8 + 6] = 0xFF;
            b[32 + i * 8 + 7] = 0x7F;
        }
        Spit(td.File("c.fseq"), b);
    }
    SECTION("all zero block table") {
        auto b = orig;
        std::fill(b.begin() + 32, b.begin() + 32 + maxBlocks * 8, 0);
        Spit(td.File("c.fseq"), b);
    }
    SECTION("block count larger than the header") {
        auto b = orig;
        b[20] |= 0xF0;
        b[21] = 0xFF;
        Spit(td.File("c.fseq"), b);
    }
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(td.File("c.fseq")));
    if (f) {
        for (uint32_t i = 0; i < f->getNumFrames(); ++i) {
            std::unique_ptr<FSEQFile::FrameData> fd(f->getFrame(i));
            if (fd) {
                std::vector<uint8_t> buf(f->getMaxChannel(), 0);
                fd->readFrame(buf.data(), f->getMaxChannel());
            }
        }
    }
    SUCCEED();
}

TEST_CASE("FSEQ corrupt sparse range count does not crash", "[fseq][corrupt]") {
    TempDir td;
    WriteSpec s;
    s.sparse = { { 0, 8 }, { 16, 8 } };
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    auto b = Slurp(p);
    b[22] = 255;
    Spit(td.File("c.fseq"), b);
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(td.File("c.fseq")));
    if (f) {
        std::unique_ptr<FSEQFile::FrameData> fd(f->getFrame(0));
        if (fd) {
            std::vector<uint8_t> buf(f->getMaxChannel() + 1, 0);
            fd->readFrame(buf.data(), f->getMaxChannel());
        }
    }
    SUCCEED();
}

TEST_CASE("FSEQ a zero step time is replaced by the 50ms default", "[fseq][corrupt]") {
    TempDir td;
    WriteSpec s;
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    auto b = Slurp(p);
    b[18] = 0;
    Spit(td.File("z.fseq"), b);
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(td.File("z.fseq")));
    REQUIRE(f != nullptr);
    CHECK(f->getStepTime() == 50);
    CHECK(f->getTotalTimeMS() == 10u * 50u);
}

TEST_CASE("FSEQ variable header with an out of bounds length is ignored", "[fseq][corrupt]") {
    TempDir td;
    WriteSpec s;
    s.ct = FSEQFile::CompressionType::none;
    s.headers.push_back(StringHeader('m', 'f', "media.mp3"));
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    auto b = Slurp(p);
    uint32_t headerEnd = b[8] | (b[9] << 8);
    b[headerEnd] = 0xFF;
    b[headerEnd + 1] = 0x7F;
    Spit(td.File("v.fseq"), b);
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(td.File("v.fseq")));
    REQUIRE(f != nullptr);
    CHECK(f->getVariableHeaders().empty());
    CHECK(f->getMediaFilename().empty());
}

TEST_CASE("FSEQ extended data header pointing past the end of the file does not crash", "[fseq][corrupt]") {
    TempDir td;
    WriteSpec s;
    s.minor = 2;
    FSEQFile::VariableHeader big;
    big.code[0] = 'X';
    big.code[1] = 'R';
    big.extendedData = true;
    big.data.assign(200, 1);
    s.headers.push_back(big);
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    auto b = Slurp(p);
    uint32_t headerEnd = b[8] | (b[9] << 8);
    for (int i = 0; i < 8; ++i) {
        b[headerEnd + 2 + 2 + 2 + i] = 0xFF;
    }
    Spit(td.File("e.fseq"), b);
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(td.File("e.fseq")));
    REQUIRE(f != nullptr);
    REQUIRE(f->getVariableHeaders().size() == 1);
    CHECK(f->getVariableHeaders()[0].data.empty());
}

TEST_CASE("FSEQ uncompressed file with missing frame data fails the read", "[fseq][corrupt]") {
    TempDir td;
    WriteSpec s;
    s.ct = FSEQFile::CompressionType::none;
    s.channels = 100;
    s.frames = 10;
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    auto b = Slurp(p);
    b.resize(b.size() - 500);
    Spit(td.File("c.fseq"), b);
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(td.File("c.fseq")));
    REQUIRE(f != nullptr);
    std::vector<uint8_t> got;
    CHECK(ReadFrameInto(*f, 0, got));
    CHECK(got == MakeFrame(s, 0, 100));
    std::unique_ptr<FSEQFile::FrameData> last(f->getFrame(9));
    if (last) {
        std::vector<uint8_t> buf(100, 0);
        last->readFrame(buf.data(), 100);
        CHECK(buf != MakeFrame(s, 9, 100));
    }
}

TEST_CASE("ESEQ effect file is read as a sparse uncompressed sequence", "[fseq]") {
    TempDir td;
    std::vector<uint8_t> b(20, 0);
    std::memcpy(b.data(), "ESEQ", 4);
    b[8] = 6;
    b[12] = 11;
    b[16] = 6;
    for (int f = 0; f < 3; ++f) {
        for (int c = 0; c < 6; ++c) {
            b.push_back((uint8_t)(f * 10 + c + 1));
        }
    }
    auto p = td.File("a.eseq");
    Spit(p, b);
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f != nullptr);
    CHECK(f->getNumFrames() == 3);
    CHECK(f->getStepTime() == 50);
    CHECK(f->getChannelCount() == 6);
    CHECK(f->getVersionMajor() == 2);
    auto* v2 = dynamic_cast<V2FSEQFile*>(f.get());
    REQUIRE(v2 != nullptr);
    REQUIRE(v2->m_sparseRanges.size() == 1);
    CHECK(v2->m_sparseRanges[0] == std::make_pair(10u, 6u));
    CHECK(f->getMaxChannel() == 16);
    std::vector<uint8_t> got;
    REQUIRE(ReadFrameInto(*f, 2, got));
    for (int c = 0; c < 6; ++c) {
        CHECK(got[10 + c] == 20 + c + 1);
    }
    CHECK(got[0] == 0);
}

TEST_CASE("ESEQ file with a zero channel count or truncated header reads as empty", "[fseq][corrupt]") {
    TempDir td;
    std::vector<uint8_t> b(24, 0);
    std::memcpy(b.data(), "ESEQ", 4);
    Spit(td.File("zero.eseq"), b);
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(td.File("zero.eseq")));
    if (f) {
        CHECK(f->getNumFrames() == 0);
    }
    std::vector<uint8_t> shortb(12, 0);
    std::memcpy(shortb.data(), "ESEQ", 4);
    shortb[8] = 4;
    Spit(td.File("short.eseq"), shortb);
    std::unique_ptr<FSEQFile> g(FSEQFile::openFSEQFile(td.File("short.eseq")));
    if (g) {
        CHECK(g->getNumFrames() == 0);
    }
}

TEST_CASE("FSEQ FrameData reports size and raw data for dense reads", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.ct = FSEQFile::CompressionType::none;
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    REQUIRE(f != nullptr);
    std::unique_ptr<FSEQFile::FrameData> fd(f->getFrame(4));
    REQUIRE(fd != nullptr);
    CHECK(fd->frame == 4);
    CHECK(fd->GetSize() == 64);
    REQUIRE(fd->GetData() != nullptr);
    CHECK(std::memcmp(fd->GetData(), MakeFrame(s, 4, 64).data(), 64) == 0);
}

TEST_CASE("FSEQ stress many frames of random like data round trips", "[fseq]") {
    TempDir td;
    WriteSpec s;
    s.channels = 3000;
    s.frames = 200;
    s.step = 20;
    s.ct = FSEQFile::CompressionType::zstd;
    auto p = td.File("a.fseq");
    REQUIRE(WriteFile(p, s));
    CheckAllFrames(p, s);
}

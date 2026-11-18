#include <catch2/catch_test_macros.hpp>

#include "render/FSEQFile.h"
#include "render/FSEQFileIO.h"
#include "render/SequenceData.h"
#include "support/TempDir.h"
#include "utils/Base64.h"

#include <memory>
#include <string>
#include <vector>

namespace {
uint8_t Pat(unsigned f, unsigned c) {
    return (uint8_t)((f * 17 + c * 5 + 1) & 0xFF);
}

void Fill(SequenceData& sd) {
    for (unsigned f = 0; f < sd.NumFrames(); ++f) {
        for (unsigned c = 0; c < sd.NumChannels(); ++c) {
            sd[f][c] = Pat(f, c);
        }
    }
}
}

TEST_CASE("SequenceData default state", "[sequence][seqdata]") {
    SequenceData sd;
    CHECK(sd.NumChannels() == 0);
    CHECK(sd.NumFrames() == 0);
    CHECK(sd.FrameTime() == 50);
    CHECK(sd.TotalTime() == 0);
    CHECK_FALSE(sd.IsValidData());
    CHECK_FALSE(sd.OK(0, 0));
}

TEST_CASE("SequenceData init rounds channels up to a multiple of four", "[sequence][seqdata]") {
    SequenceData sd;
    sd.init(10, 5, 25);
    CHECK(sd.NumChannels() == 12);
    CHECK(sd.NumFrames() == 5);
    CHECK(sd.FrameTime() == 25);
    CHECK(sd.TotalTime() == 125);
    CHECK(sd.IsValidData());
}

TEST_CASE("SequenceData init can keep the exact channel count", "[sequence][seqdata]") {
    SequenceData sd;
    sd.init(10, 5, 25, false);
    CHECK(sd.NumChannels() == 10);
    sd.init(12, 5, 25);
    CHECK(sd.NumChannels() == 12);
    sd.init(1, 1, 50);
    CHECK(sd.NumChannels() == 4);
}

TEST_CASE("SequenceData frames start zeroed and hold independent data", "[sequence][seqdata]") {
    SequenceData sd;
    sd.init(16, 6, 50);
    for (unsigned f = 0; f < 6; ++f) {
        for (unsigned c = 0; c < 16; ++c) {
            CHECK(sd[f][c] == 0);
        }
    }
    Fill(sd);
    for (unsigned f = 0; f < 6; ++f) {
        for (unsigned c = 0; c < 16; ++c) {
            REQUIRE(sd[f][c] == Pat(f, c));
        }
    }
}

TEST_CASE("SequenceData const access mirrors mutable access", "[sequence][seqdata]") {
    SequenceData sd;
    sd.init(8, 3, 50);
    sd[1][5] = 99;
    const SequenceData& csd = sd;
    CHECK(*csd[1][5] == 99);
    CHECK(*csd[1][4] == 0);
}

TEST_CASE("SequenceData OK checks frame and channel bounds", "[sequence][seqdata]") {
    SequenceData sd;
    sd.init(8, 3, 50);
    CHECK(sd.OK(0, 0));
    CHECK(sd.OK(2, 7));
    CHECK_FALSE(sd.OK(3, 0));
    CHECK_FALSE(sd.OK(0, 8));
}

TEST_CASE("SequenceData out of range reads are safe and zero", "[sequence][seqdata]") {
    SequenceData sd;
    sd.init(8, 3, 50);
    Fill(sd);
    const SequenceData& csd = sd;
    CHECK(*csd[3][0] == 0);
    CHECK(*csd[1000][3] == 0);
    CHECK(*csd[1][8] == 0);
    CHECK(*csd[1][100000] == 0);
    CHECK(*csd[0xFFFFFFFFu][0] == 0);
}

TEST_CASE("SequenceData Zero clears a whole frame", "[sequence][seqdata]") {
    SequenceData sd;
    sd.init(8, 3, 50);
    Fill(sd);
    sd[1].Zero();
    for (unsigned c = 0; c < 8; ++c) {
        CHECK(sd[1][c] == 0);
        CHECK(sd[0][c] == Pat(0, c));
        CHECK(sd[2][c] == Pat(2, c));
    }
}

TEST_CASE("SequenceData Zero range clears only that range and ignores bad ranges", "[sequence][seqdata]") {
    SequenceData sd;
    sd.init(16, 2, 50);
    Fill(sd);
    sd[0].Zero(4, 4);
    for (unsigned c = 0; c < 16; ++c) {
        if (c >= 4 && c < 8) {
            CHECK(sd[0][c] == 0);
        } else {
            CHECK(sd[0][c] == Pat(0, c));
        }
    }
    sd[1].Zero(10, 0);
    sd[1].Zero(16, 1);
    sd[1].Zero(14, 5);
    sd[1].Zero(100, 100);
    for (unsigned c = 0; c < 16; ++c) {
        CHECK(sd[1][c] == Pat(1, c));
    }
    sd[1].Zero(12, 4);
    CHECK(sd[1][12] == 0);
    CHECK(sd[1][15] == 0);
}

TEST_CASE("SequenceData re-init discards previous contents", "[sequence][seqdata]") {
    SequenceData sd;
    sd.init(8, 4, 50);
    Fill(sd);
    sd.init(8, 4, 50);
    for (unsigned f = 0; f < 4; ++f) {
        for (unsigned c = 0; c < 8; ++c) {
            CHECK(sd[f][c] == 0);
        }
    }
}

TEST_CASE("SequenceData init with no frames or no channels releases the data", "[sequence][seqdata]") {
    SequenceData sd;
    sd.init(8, 4, 50);
    REQUIRE(sd.IsValidData());
    sd.init(8, 0, 50);
    CHECK_FALSE(sd.IsValidData());
    CHECK(sd.NumFrames() == 0);
    sd.init(8, 4, 50);
    REQUIRE(sd.IsValidData());
    sd.init(0, 4, 50);
    CHECK_FALSE(sd.IsValidData());
    CHECK(sd.TotalTime() == 200);
}

TEST_CASE("SequenceData Cleanup invalidates the data", "[sequence][seqdata]") {
    SequenceData sd;
    sd.init(8, 4, 50);
    sd.Cleanup();
    CHECK_FALSE(sd.IsValidData());
}

TEST_CASE("SequenceData holds a multi megabyte sequence", "[sequence][seqdata]") {
    SequenceData sd;
    sd.init(30000, 200, 25);
    REQUIRE(sd.IsValidData());
    sd[0][0] = 1;
    sd[199][29999] = 2;
    sd[100][15000] = 3;
    CHECK(sd[0][0] == 1);
    CHECK(sd[199][29999] == 2);
    CHECK(sd[100][15000] == 3);
    CHECK(sd[100][14999] == 0);
    CHECK(sd.TotalTime() == 5000);
}

TEST_CASE("SequenceData time arithmetic scales with frame time", "[sequence][seqdata]") {
    SequenceData sd;
    sd.init(4, 1200, 20);
    CHECK(sd.TotalTime() == 24000);
    sd.init(4, 1200, 33);
    CHECK(sd.TotalTime() == 39600);
}

TEST_CASE("SequenceData base64 encoding groups bytes by channel", "[sequence][seqdata]") {
    SequenceData sd;
    sd.init(2, 3, 50, false);
    REQUIRE(sd.NumChannels() == 2);
    for (unsigned f = 0; f < 3; ++f) {
        sd[f][0] = (uint8_t)(10 + f);
        sd[f][1] = (uint8_t)(20 + f);
    }
    auto bytes = Base64::Decode(sd.base64_encode());
    REQUIRE(bytes.size() == 6);
    CHECK(bytes == std::vector<uint8_t>{ 10, 11, 12, 20, 21, 22 });
}

TEST_CASE("SequenceData base64 encoding of empty data is empty", "[sequence][seqdata]") {
    SequenceData sd;
    CHECK(sd.base64_encode().empty());
}

TEST_CASE("FSEQFileIO Write rejects bad input", "[sequence][seqdata][fseqio]") {
    TempDir td;
    SequenceData empty;
    CHECK_FALSE(FSEQFileIO::Write(td.File("a.fseq"), empty, nullptr, nullptr));

    SequenceData sd;
    sd.init(8, 4, 50);
    CHECK_FALSE(FSEQFileIO::Write("", sd, nullptr, nullptr));
    CHECK_FALSE(FSEQFileIO::Write(td.File("nodir/a.fseq"), sd, nullptr, nullptr));

    SequenceData noFrames;
    noFrames.init(8, 0, 50);
    CHECK_FALSE(FSEQFileIO::Write(td.File("b.fseq"), noFrames, nullptr, nullptr));
}

TEST_CASE("FSEQFileIO Write round trips sequence data", "[sequence][seqdata][fseqio]") {
    TempDir td;
    struct Case {
        int version;
        FSEQFile::CompressionType ct;
    };
    for (Case c : { Case{ 2, FSEQFile::CompressionType::zstd }, Case{ 2, FSEQFile::CompressionType::none }, Case{ 2, FSEQFile::CompressionType::zlib }, Case{ 1, FSEQFile::CompressionType::none } }) {
        SequenceData sd;
        sd.init(200, 50, 25);
        Fill(sd);
        FSEQFileIO::WriteOptions opt;
        opt.version = c.version;
        opt.compression = c.ct;
        auto p = td.File("w" + std::to_string(c.version) + std::to_string((int)c.ct) + ".fseq");
        REQUIRE(FSEQFileIO::Write(p, sd, nullptr, nullptr, opt));

        std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
        REQUIRE(f != nullptr);
        CHECK(f->getVersionMajor() == c.version);
        CHECK(f->getNumFrames() == 50);
        CHECK(f->getStepTime() == 25);
        CHECK(f->getChannelCount() == 200);
        for (unsigned fr = 0; fr < 50; ++fr) {
            std::unique_ptr<FSEQFile::FrameData> fd(f->getFrame(fr));
            REQUIRE(fd != nullptr);
            std::vector<uint8_t> buf(f->getMaxChannel(), 0);
            REQUIRE(fd->readFrame(buf.data(), f->getMaxChannel()));
            for (unsigned ch = 0; ch < 200; ++ch) {
                REQUIRE(buf[ch] == Pat(fr, ch));
            }
        }
    }
}

TEST_CASE("FSEQFileIO Write rounds the channel count to four unless an effect sequence", "[sequence][seqdata][fseqio]") {
    TempDir td;
    SequenceData sd;
    sd.init(10, 4, 50, false);
    REQUIRE(sd.NumChannels() == 10);
    FSEQFileIO::WriteOptions opt;
    opt.compression = FSEQFile::CompressionType::none;
    REQUIRE(FSEQFileIO::Write(td.File("a.fseq"), sd, nullptr, nullptr, opt));
    std::unique_ptr<FSEQFile> a(FSEQFile::openFSEQFile(td.File("a.fseq")));
    REQUIRE(a != nullptr);
    CHECK(a->getChannelCount() == 12);

    opt.isEffectSeq = true;
    REQUIRE(FSEQFileIO::Write(td.File("b.fseq"), sd, nullptr, nullptr, opt));
    std::unique_ptr<FSEQFile> b(FSEQFile::openFSEQFile(td.File("b.fseq")));
    REQUIRE(b != nullptr);
    CHECK(b->getChannelCount() == 10);
    bool hasES = false;
    for (auto& h : b->getVariableHeaders()) {
        if (h.code[0] == 'e' && h.code[1] == 'S') {
            hasES = true;
        }
    }
    CHECK(hasES);
}

TEST_CASE("FSEQFileIO Write emits media and source headers", "[sequence][seqdata][fseqio]") {
    TempDir td;
    SequenceData sd;
    sd.init(8, 4, 50);
    FSEQFileIO::WriteOptions opt;
    opt.mediaFile = "song.mp3";
    opt.source = "unit test";
    REQUIRE(FSEQFileIO::Write(td.File("a.fseq"), sd, nullptr, nullptr, opt));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(td.File("a.fseq")));
    REQUIRE(f != nullptr);
    CHECK(f->getMediaFilename() == "song.mp3");
    std::string sp;
    for (auto& h : f->getVariableHeaders()) {
        if (h.code[0] == 's' && h.code[1] == 'p') {
            sp = (const char*)h.data.data();
        }
    }
    CHECK(sp == "unit test");
}

TEST_CASE("FSEQFileIO Write omits the media header when no media is given", "[sequence][seqdata][fseqio]") {
    TempDir td;
    SequenceData sd;
    sd.init(8, 4, 50);
    REQUIRE(FSEQFileIO::Write(td.File("a.fseq"), sd, nullptr, nullptr));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(td.File("a.fseq")));
    REQUIRE(f != nullptr);
    CHECK(f->getMediaFilename().empty());
    bool hasSP = false;
    for (auto& h : f->getVariableHeaders()) {
        if (h.code[0] == 's' && h.code[1] == 'p') {
            hasSP = std::string((const char*)h.data.data()) == "xLights";
        }
    }
    CHECK(hasSP);
}

TEST_CASE("FSEQFileIO Write embeds compressed blobs from memory or disk", "[sequence][seqdata][fseqio]") {
    TempDir td;
    SequenceData sd;
    sd.init(8, 4, 50);
    FSEQFileIO::WriteOptions opt;
    FSEQFileIO::EmbeddedBlob mem;
    mem.code[0] = 'X';
    mem.code[1] = 'S';
    mem.xml = "<xsequence>" + std::string(500, 'a') + "</xsequence>";
    opt.embedded.push_back(mem);
    FSEQFileIO::EmbeddedBlob disk;
    disk.code[0] = 'X';
    disk.code[1] = 'R';
    disk.filePath = td.Write("rgb.xml", "<xrgb>" + std::string(500, 'b') + "</xrgb>");
    disk.xml = "<ignored/>";
    opt.embedded.push_back(disk);
    FSEQFileIO::EmbeddedBlob empty;
    empty.code[0] = 'X';
    empty.code[1] = 'N';
    opt.embedded.push_back(empty);
    REQUIRE(FSEQFileIO::Write(td.File("a.fseq"), sd, nullptr, nullptr, opt));

    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(td.File("a.fseq")));
    REQUIRE(f != nullptr);
    const FSEQFile::VariableHeader* xs = nullptr;
    const FSEQFile::VariableHeader* xr = nullptr;
    const FSEQFile::VariableHeader* xn = nullptr;
    for (auto& h : f->getVariableHeaders()) {
        if (h.code[0] == 'X' && h.code[1] == 'S') xs = &h;
        if (h.code[0] == 'X' && h.code[1] == 'R') xr = &h;
        if (h.code[0] == 'X' && h.code[1] == 'N') xn = &h;
    }
    REQUIRE(xs != nullptr);
    REQUIRE(xr != nullptr);
    CHECK(xn == nullptr);
    CHECK(xs->extendedData);
    REQUIRE(xs->data.size() >= 4);
    CHECK(xs->data[0] == 0x28);
    CHECK(xs->data[1] == 0xB5);
    CHECK(xs->data[2] == 0x2F);
    CHECK(xs->data[3] == 0xFD);
    CHECK(xs->data.size() < mem.xml.size());
    CHECK(xr->data.size() < 200);
}

TEST_CASE("FSEQFileIO Write appends caller supplied headers", "[sequence][seqdata][fseqio]") {
    TempDir td;
    SequenceData sd;
    sd.init(8, 4, 50);
    FSEQFileIO::WriteOptions opt;
    FSEQFile::VariableHeader h;
    h.code[0] = 'Z';
    h.code[1] = 'Z';
    h.data = { 9, 8, 7 };
    opt.extraHeaders.push_back(h);
    REQUIRE(FSEQFileIO::Write(td.File("a.fseq"), sd, nullptr, nullptr, opt));
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(td.File("a.fseq")));
    REQUIRE(f != nullptr);
    bool found = false;
    for (auto& v : f->getVariableHeaders()) {
        if (v.code[0] == 'Z' && v.code[1] == 'Z') {
            found = v.data == h.data;
        }
    }
    CHECK(found);
}

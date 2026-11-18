#include <catch2/catch_test_macros.hpp>

#include "support/TempDir.h"
#include "utils/XsqFileScanner.h"

TEST_CASE("ScanXsqFile finds the sequence marker and media file", "[xsq]") {
    TempDir dir;
    auto f = dir.Write("a.xsq",
                       "<?xml version=\"1.0\"?>\n<xsequence BaseChannel=\"0\">\n<head>\n"
                       "<mediaFile>C:\\Music\\a &amp; b &lt;1&gt;.mp3</mediaFile>\n</head>\n");
    auto info = ScanXsqFile(f);
    CHECK(info.isSequence);
    CHECK(info.mediaFile == "C:\\Music\\a & b <1>.mp3");
}

TEST_CASE("ScanXsqFile decodes all five entities", "[xsq]") {
    TempDir dir;
    auto f = dir.Write("a.xsq", "<xsequence><mediaFile>&lt;&gt;&amp;&apos;&quot;&unknown;</mediaFile>");
    CHECK(ScanXsqFile(f).mediaFile == "<>&'\"&unknown;");
}

TEST_CASE("ScanXsqFile handles sequences without media", "[xsq]") {
    TempDir dir;
    auto info = ScanXsqFile(dir.Write("a.xsq", "<xsequence></xsequence>"));
    CHECK(info.isSequence);
    CHECK(info.mediaFile.empty());
}

TEST_CASE("ScanXsqFile ignores non-sequences, empty and missing files", "[xsq]") {
    TempDir dir;
    CHECK_FALSE(ScanXsqFile(dir.Write("other.xml", "<xrgb><models/></xrgb>")).isSequence);
    CHECK_FALSE(ScanXsqFile(dir.Write("empty.xsq", "")).isSequence);
    CHECK_FALSE(ScanXsqFile(dir.File("missing.xsq")).isSequence);
}

TEST_CASE("ScanXsqFile only looks at the head of the file", "[xsq]") {
    TempDir dir;
    std::string body = "<xsequence>" + std::string(60 * 1024, ' ') + "<mediaFile>late.mp3</mediaFile>";
    auto info = ScanXsqFile(dir.Write("big.xsq", body));
    CHECK(info.isSequence);
    CHECK(info.mediaFile.empty());

    CHECK_FALSE(ScanXsqFile(dir.Write("late_marker.xsq", std::string(60 * 1024, ' ') + "<xsequence>")).isSequence);
}

TEST_CASE("ScanXsqFile needs the closing media tag", "[xsq]") {
    TempDir dir;
    CHECK(ScanXsqFile(dir.Write("a.xsq", "<xsequence><mediaFile>unterminated.mp3")).mediaFile.empty());
}

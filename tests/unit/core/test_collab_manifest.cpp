#include <catch2/catch_test_macros.hpp>

#include "test_collab_join_common.h"

#include "collab/CollabManifest.h"
#include "render/SequenceMediaCommands.h"

#include <set>

using namespace Collab;
using namespace CollabTest;
namespace fs = std::filesystem;

namespace {

const FileEntry* Find(const std::vector<FileEntry>& m, const std::string& path) {
    for (const auto& e : m) {
        if (e.path == path) {
            return &e;
        }
    }
    return nullptr;
}

const FileCheck* FindCheck(const std::vector<FileCheck>& c, const std::string& path) {
    for (const auto& e : c) {
        if (e.entry.path == path) {
            return &e;
        }
    }
    return nullptr;
}

std::vector<FileEntry> ManifestOf(MediaShow& ms) {
    Doc d(ms.show.path, ms.show.dir.File("manifest"));
    d.Open(ms.xsq);
    d.InstallDirs();
    return BuildManifest(CollectSessionFiles(ms.show.path, d.Sequence(), d.AudioFiles()));
}

} // namespace

TEST_CASE("The content hash is XXH64 and streams in any chunk size", "[collab][collab_manifest]") {
    // Published XXH64 (seed 0) values: the hash is the same on every platform.
    CHECK(HashBytes("") == 0xEF46DB3751D8E999ull);
    CHECK(HashBytes("abc") == 0x44BC2CF5AD770999ull);
    std::string big;
    for (int i = 0; i < 1000; ++i) {
        big += (char)(i * 37 + 11);
    }
    uint64_t whole = HashBytes(big);
    for (size_t chunk : { 1u, 3u, 7u, 31u, 32u, 33u, 64u, 999u }) {
        ContentHash h;
        for (size_t at = 0; at < big.size(); at += chunk) {
            h.Update(big.data() + at, std::min(chunk, big.size() - at));
        }
        CHECK(h.Final() == whole);
    }
    std::string other = big;
    other[500] ^= 1;
    CHECK(HashBytes(other) != whole);

    TempDir t;
    std::string f = t.Write("big.bin", big);
    uint64_t size = 0;
    uint64_t hash = 0;
    REQUIRE(HashFile(f, size, hash));
    CHECK(size == big.size());
    CHECK(hash == whole);
    CHECK_FALSE(HashFile(t.File("missing"), size, hash));
}

TEST_CASE("The hash cache re-reads a file only when it changed", "[collab][collab_manifest]") {
    TempDir t;
    std::string f = t.Write("a.bin", "one");
    HashCache cache;
    uint64_t size = 0;
    uint64_t hash = 0;
    REQUIRE(cache.Hash(f, size, hash));
    REQUIRE(cache.Hash(f, size, hash));
    CHECK(cache.FilesRead() == 1);
    CHECK(hash == HashBytes("one"));
    t.Write("a.bin", "three");
    REQUIRE(cache.Hash(f, size, hash));
    CHECK(cache.FilesRead() == 2);
    CHECK(hash == HashBytes("three"));
    CHECK_FALSE(cache.Hash(t.File("gone"), size, hash));
}

TEST_CASE("Session-copy paths never leave the copy", "[collab][collab_manifest]") {
    CHECK(CachePathFor("Images/a.png") == "Images/a.png");
    CHECK(CachePathFor("Images\\Sub\\a.png") == "Images/Sub/a.png");
    CHECK(CachePathFor("./Images//a.png") == "Images/a.png");
    CHECK(CachePathFor("xlights_rgbeffects.xml") == "xlights_rgbeffects.xml");
    // Absolute or climbing paths keep their last folder and name, where
    // FixFile looks for a file that moved.
    CHECK(CachePathFor("/Users/someone/Pictures/a.png") == "Pictures/a.png");
    CHECK(CachePathFor("C:\\Show\\Videos\\v.mp4") == "Videos/v.mp4");
    CHECK(CachePathFor("../Media/v.mp4") == "Media/v.mp4");
    CHECK(CachePathFor("../../../outside.png") == "outside.png");
    CHECK(CachePathFor("Images/../../x.png") == "x.png");
    CHECK(CachePathFor("C:a.png") == "");
    CHECK(CachePathFor("/") == "");
    CHECK(CachePathFor("..") == "");
    CHECK(CachePathFor("") == "");
}

TEST_CASE("Manifest paths are show-relative when the file is in the show", "[collab][collab_manifest]") {
    TempDir show;
    TempDir other;
    std::string in = WriteFile(show.path / "Images" / "a.png", "a");
    std::string out = other.Write("b.png", "b");
    CHECK(ManifestPathFor(show.path.string(), in, in) == "Images/a.png");
    CHECK(ManifestPathFor(show.path.string(), "Images\\a.png", in) == "Images/a.png");
    CHECK(ManifestPathFor(show.path.string(), out, out) == out);
    // Found through a media folder: the reference as stored.
    CHECK(ManifestPathFor(show.path.string(), "Media\\b.png", out) == "Media/b.png");
}

TEST_CASE("The manifest lists layout, audio and every referenced file, not embedded media", "[collab][collab_manifest]") {
    MediaShow ms;
    auto m = ManifestOf(ms);
    INFO(m.size());
    const FileEntry* rgb = Find(m, kLayoutFile);
    const FileEntry* net = Find(m, kNetworksFile);
    const FileEntry* song = Find(m, "song.wav");
    const FileEntry* a = Find(m, "Images/a.png");
    const FileEntry* b = Find(m, ms.outsideFile);
    REQUIRE(rgb != nullptr);
    REQUIRE(net != nullptr);
    REQUIRE(song != nullptr);
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    CHECK(rgb->role == FileRole::Layout);
    CHECK(net->role == FileRole::Layout);
    CHECK(song->role == FileRole::Audio);
    CHECK(a->role == FileRole::Media);
    CHECK(b->role == FileRole::Media);
    CHECK(rgb->hash == HashBytes(ReadFile(ms.show.dir.File(kLayoutFile))));
    CHECK(a->hash == HashBytes("picture-a"));
    CHECK(a->size == 9);
    CHECK(b->hash == HashBytes("picture-b"));
    // Embedded media travels in the .xsq; a file nobody has isn't listed.
    CHECK(Find(m, "Images/emb.png") == nullptr);
    CHECK(Find(m, "Images/gone.png") == nullptr);
    CHECK(m.size() == 5);
}

TEST_CASE("The manifest finds effect file references and media entries each on their own", "[collab][collab_manifest]") {
    MediaShow ms;
    Doc d(ms.show.path, ms.show.dir.File("manifest"));
    d.Open(ms.xsq);
    d.InstallDirs();
    WriteFile(ms.show.dir.path / "Images" / "entry.png", "entry only");
    auto paths = [&] {
        std::set<std::string> out;
        for (const auto& f : CollectSessionFiles(ms.show.path, d.Sequence(), d.AudioFiles())) {
            out.insert(f.path);
        }
        return out;
    };
    SequenceMedia& media = d.Sequence().GetSequenceMedia();
    {
        SeqMutationGuard::Scope test;
        media.Clear();
        media.AddEmbeddedImage("Images/emb.png", xlImage(2, 2));
    }
    // Effects alone name the pictures.
    auto byEffects = paths();
    CHECK(byEffects.count("Images/a.png") == 1);
    CHECK(byEffects.count(ms.outsideFile) == 1);
    CHECK(byEffects.count("Images/emb.png") == 0);
    // A media entry no effect uses yet.
    {
        SeqMutationGuard::Scope test;
        REQUIRE(PutMediaCommand(MediaEntryState::FileReference("Images/entry.png", MediaType::Image)).Apply(d.Sequence(), ApplyMode::Strict) != nullptr);
    }
    CHECK(paths().count("Images/entry.png") == 1);
}

TEST_CASE("Comparing a show folder with the manifest finds missing and different files", "[collab][collab_manifest]") {
    MediaShow ms;
    auto m = ManifestOf(ms);
    TempDir copy;
    CopyShowFiles(ms.show.path, copy.path, { kLayoutFile, kNetworksFile, "song.wav", "Images/a.png" });
    {
        FixFileReset reset;
        auto checks = CompareManifest(copy.path.string(), m);
        REQUIRE(checks.size() == m.size());
        for (const auto& c : checks) {
            INFO(c.entry.path);
            CHECK(c.state == FileState::Match);
        }
        CHECK(FindCheck(checks, "Images/a.png")->local == (copy.path / "Images" / "a.png").string());
    }
    WriteFile(copy.path / "Images" / "a.png", "picture-A");
    fs::remove(copy.path / "song.wav");
    // A layout file anywhere but the top of the show folder doesn't count.
    fs::create_directories(copy.path / "sub");
    fs::rename(copy.path / kNetworksFile, copy.path / "sub" / kNetworksFile);
    FixFileReset reset;
    FileUtils::SetFixFileShowDir(copy.path.string());
    auto checks = CompareManifest(copy.path.string(), m);
    CHECK(FindCheck(checks, "Images/a.png")->state == FileState::Different);
    CHECK(FindCheck(checks, "song.wav")->state == FileState::Missing);
    CHECK(FindCheck(checks, kNetworksFile)->state == FileState::Missing);
    CHECK(FindCheck(checks, kLayoutFile)->state == FileState::Match);
}

TEST_CASE("Layout differences are described record by record", "[collab][collab_manifest]") {
    Show show;
    std::string host = ReadFile(show.dir.File(kLayoutFile));
    std::string mine = ChangedLayout(host);
    LayoutDifference d = DescribeLayoutDifference(kLayoutFile, mine, host);
    INFO(d.text);
    CHECK(d.readable);
    CHECK(d.changes == 3);
    CHECK(d.text.find("1 model changed (Arch)") != std::string::npos);
    CHECK(d.text.find("1 model only on this device (Extra)") != std::string::npos);
    CHECK(d.text.find("1 model only on the host (Pole)") != std::string::npos);

    // Layout ids, attribute order and whitespace aren't differences.
    std::string same = ReformattedLayout(host);
    REQUIRE(same != host);
    LayoutDifference none = DescribeLayoutDifference(kLayoutFile, same, host);
    INFO(none.text);
    CHECK(none.changes == 0);
    CHECK(none.text.empty());

    LayoutDifference bad = DescribeLayoutDifference(kLayoutFile, "<xrgb><models>", host);
    CHECK_FALSE(bad.readable);
    CHECK(bad.changes != 0);

    std::string netA = "<Networks><Controller Name=\"A\" IP=\"10.0.0.1\"/><Controller Name=\"B\" IP=\"10.0.0.2\"/></Networks>";
    std::string netB = "<Networks><Controller Name=\"A\" IP=\"10.0.0.9\"/></Networks>";
    LayoutDifference n = DescribeLayoutDifference(kNetworksFile, netA, netB);
    INFO(n.text);
    CHECK(n.text.find("1 Controller changed (A)") != std::string::npos);
    CHECK(n.text.find("1 Controller only on this device (B)") != std::string::npos);
}

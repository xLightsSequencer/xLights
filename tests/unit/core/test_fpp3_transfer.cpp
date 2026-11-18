// FPP Connect file transfers: the FPP 7+ chunked PATCH upload, the older single POST
// upload (and the ESPixelStick zip form), media checks, and the sequence upload
// decisions (send the original, build a sparse copy, or skip an unchanged one).
#ifndef _WIN32

#include "test_fpp3_common.h"

#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstdlib>
#include <fstream>

#include "render/FSEQFile.h"
#include "support/TempDir.h"

using namespace fpp3;
using Catch::Matchers::ContainsSubstring;

namespace {

// Points the process temp directory (where FPP builds its upload copies) into a TempDir
// for the life of the test.
struct TmpDirOverride {
    std::string prev;
    bool had = false;
    explicit TmpDirOverride(const std::filesystem::path& p) {
        const char* t = std::getenv("TMPDIR");
        had = t != nullptr;
        if (had) prev = t;
        setenv("TMPDIR", p.string().c_str(), 1);
    }
    ~TmpDirOverride() {
        if (had) {
            setenv("TMPDIR", prev.c_str(), 1);
        } else {
            unsetenv("TMPDIR");
        }
    }
};

std::string WriteSeq(const std::filesystem::path& path, int version = 2, FSEQFile::CompressionType ct = FSEQFile::CompressionType::zstd, uint32_t channels = 60,
                     uint32_t frames = 4, int step = 50, bool effect = false) {
    std::unique_ptr<FSEQFile> f(FSEQFile::createFSEQFile(path.string(), version, ct, -99));
    REQUIRE(f);
    f->setChannelCount((int)channels);
    f->setStepTime(step);
    f->setNumFrames(frames);
    if (effect) {
        FSEQFile::VariableHeader h;
        h.code[0] = 'e';
        h.code[1] = 'S';
        h.data = { 'x', 0 };
        f->addVariableHeader(h);
    }
    f->writeHeader();
    std::vector<uint8_t> frame(channels);
    for (uint32_t fr = 0; fr < frames; fr++) {
        for (uint32_t c = 0; c < channels; c++) {
            frame[c] = (uint8_t)(fr * 10 + c);
        }
        f->addFrame(fr, frame.data());
    }
    f->finalize();
    return path.string();
}

nlohmann::json MetaFor(FSEQFile* f, int compression = 1) {
    nlohmann::json m;
    m["Version"] = std::to_string(f->getVersionMajor()) + ".0";
    m["ID"] = std::to_string(f->getUniqueId());
    m["NumFrames"] = f->getNumFrames();
    m["StepTime"] = f->getStepTime();
    m["MaxChannel"] = f->getMaxChannel();
    m["ChannelCount"] = f->getChannelCount();
    m["CompressionType"] = compression;
    return m;
}

std::string ReadAll(const std::filesystem::path& p) {
    std::ifstream in(p, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), {});
}

} // namespace

TEST_CASE("FPP 7 upload sends the file in PATCH chunks", "[fpp3][transfer]") {
    TempDir dir;
    dir.Write("song.mp3", "0123456789");
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    int progressCalls = 0;
    f->setProgress({ [&](int) { ++progressCalls; }, [] { return false; }, [] {} });
    CHECK_FALSE(f->uploadOrCopyFile("song.mp3", (dir.path / "song.mp3").string(), "music"));
    DrainCurls();
    auto up = srv.Sent("PATCH", "/api/file/music");
    REQUIRE(up.size() == 1);
    CHECK(up[0].body == "0123456789");
    CHECK(f->messages.empty());
    CHECK(f->faileduploads.empty());
    CHECK(progressCalls > 0);
}

TEST_CASE("FPP 7 upload retries a failed chunk three times then gives up", "[fpp3][transfer]") {
    TempDir dir;
    dir.Write("a.fseq", "abc");
    FppServer srv;
    srv.Post("/api/file/sequences", 500);
    auto f = MakeFpp(srv, 9);
    f->uploadFileV7("a.fseq", (dir.path / "a.fseq").string(), "sequences");
    DrainCurls();
    CHECK(srv.Count("PATCH", "/api/file/sequences") == 4);
    REQUIRE(f->faileduploads.size() == 1);
    CHECK(f->faileduploads.front() == "a.fseq");
    CHECK_THAT(f->messages.back(), ContainsSubstring("ERROR Uploading file: a.fseq. Response code: 500"));
}

// The chunks are pumped by whoever runs CurlManager next (the app's idle handler), so
// an FPP destroyed mid-upload must finish the transfer itself rather than leave the
// callbacks holding a dangling pointer to it.
TEST_CASE("FPP 7 upload in flight when the FPP is destroyed completes first", "[fpp3][transfer]") {
    TempDir dir;
    dir.Write("bg.png", "0123456789");
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    bool ownerAlive = true;
    int lateProgress = 0;
    f->setProgress({ [&](int) { if (!ownerAlive) ++lateProgress; }, [] { return false; }, [] {} });
    f->uploadFileV7("bg.png", (dir.path / "bg.png").string(), "virtualdisplay_assets");
    ownerAlive = false;
    f.reset();
    CHECK(srv.Count("PATCH", "/api/file/virtualdisplay_assets") == 1);
    CHECK(lateProgress == 0);
    CHECK_FALSE(CurlManager::INSTANCE.processCurls());
}

TEST_CASE("FPP 7 upload of a missing file reports it", "[fpp3][transfer]") {
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    f->uploadFileV7("x.fseq", "/nonexistent/dir/x.fseq", "sequences");
    CHECK(srv.Requests().empty());
    CHECK(f->faileduploads.front() == "x.fseq");
    CHECK_THAT(f->messages.back(), ContainsSubstring("Could not open source file"));
}

TEST_CASE("FPP 7 upload goes through the proxy", "[fpp3][transfer]") {
    TempDir dir;
    dir.Write("a.fseq", "abc");
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    f->ipAddress = "198.51.100.2";
    f->_fppProxy = srv.Address();
    f->uploadFileV7("a.fseq", (dir.path / "a.fseq").string(), "sequences");
    DrainCurls();
    CHECK(srv.Count("PATCH", "/proxy/198.51.100.2/api/file/sequences") == 1);
}

// uploadFile only queues the transfer, so everything curl writes to (including its error
// buffer) must outlive the call.
TEST_CASE("Older upload POSTs the file then moves it into place", "[fpp3]") {
    TempDir dir;
    dir.Write("my seq.fseq", "SEQDATA");
    FppServer srv;
    srv.Get("/api/file/move/my%20seq.fseq", "ok");
    auto f = MakeFpp(srv, 9);
    f->fppType = FPP_TYPE::GENIUS;
    f->username = "admin";
    f->password = "pw";
    f->uploadOrCopyFile("my seq.fseq", (dir.path / "my seq.fseq").string(), "sequences");
    DrainCurls();
    auto up = srv.Sent("POST", "/api/file/uploads/my%20seq.fseq");
    REQUIRE(up.size() == 1);
    CHECK(up[0].body == "SEQDATA");
    CHECK(srv.Has("GET", "/api/file/move/my%20seq.fseq"));
    CHECK(f->faileduploads.empty());
}

TEST_CASE("Older upload failure is reported", "[fpp3]") {
    TempDir dir;
    dir.Write("a.fseq", "x");
    FppServer srv;
    srv.Post("/api/file/uploads/a.fseq", 403);
    auto f = MakeFpp(srv, 9);
    f->fppType = FPP_TYPE::FALCONV4V5;
    f->uploadFile("a.fseq", (dir.path / "a.fseq").string());
    DrainCurls();
    CHECK_FALSE(srv.Has("GET", "/api/file/move/a.fseq"));
    REQUIRE(f->faileduploads.size() == 1);
    CHECK_THAT(f->messages.back(), ContainsSubstring("Response Code: 403"));
}

TEST_CASE("ESPixelStick upload posts without a move", "[fpp3]") {
    TempDir dir;
    dir.Write("a.fseq", "DATA");
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    f->fppType = FPP_TYPE::ESPIXELSTICK;
    f->uploadFile("a.fseq", (dir.path / "a.fseq").string());
    DrainCurls();
    auto up = srv.Sent("POST", "/fpp?path=uploadFile&filename=a.fseq");
    REQUIRE(up.size() == 1);
    CHECK(up[0].body == "DATA");
    CHECK(srv.Count("GET", "/api/file/move/a.fseq") == 0);
}

TEST_CASE("ESPixelStick zip upload retries uncompressed after a failure", "[fpp3]") {
    TempDir dir;
    std::string payload(4000, 'z');
    dir.Write("a.fseq", payload);
    FppServer srv;
    srv.Post("/fpp?path=uploadFile&filename=a.xlz", 500);
    auto f = MakeFpp(srv, 9);
    f->fppType = FPP_TYPE::ESPIXELSTICK;
    f->canZipUpload = true;
    f->uploadFile("a.fseq", (dir.path / "a.fseq").string());
    DrainCurls();
    INFO(srv.All());
    auto up = srv.Sent("POST", "/fpp?path=uploadFile&filename=a.xlz");
    REQUIRE(up.size() == 2);
    CHECK(up[0].body.rfind("PK", 0) == 0);
    CHECK(up[1].body.rfind("PK", 0) == 0);
    // deflate shrinks a run of one byte; the retry is stored, so carries it whole
    CHECK(up[0].body.size() < payload.size());
    CHECK(up[1].body.size() > payload.size());
    CHECK_THAT(up[1].body, ContainsSubstring(payload));
    CHECK(f->faileduploads.size() == 1);
    CHECK_FALSE(std::filesystem::exists(dir.path / "a.fseq.raw"));
}

TEST_CASE("Media already on the FPP at the same size is not uploaded again", "[fpp3][transfer][media]") {
    TempDir dir;
    dir.Write("song.mp3", "0123456789");
    FppServer srv;
    srv.Get("/api/media/song.mp3/meta", R"({"format":{"size":"10"}})");
    auto f = MakeFpp(srv, 9);
    std::string base;
    CHECK_FALSE(f->CheckUploadMedia((dir.path / "song.mp3").string(), base));
    CHECK(base == "song.mp3");
    DrainCurls();
    CHECK_FALSE(srv.Has("PATCH", "/api/file/music"));

    srv.Get("/api/media/song.mp3/meta", R"({"format":{"size":"11"}})");
    f->CheckUploadMedia((dir.path / "song.mp3").string(), base);
    DrainCurls();
    CHECK(srv.Count("PATCH", "/api/file/music") == 1);
}

TEST_CASE("Video media goes to the videos folder and a missing file is reported", "[fpp3][transfer][media]") {
    TempDir dir;
    dir.Write("clip.mp4", "vid");
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    std::string base;
    f->CheckUploadMedia((dir.path / "clip.mp4").string(), base);
    f->CheckUploadMedia((dir.path / "missing.mp3").string(), base);
    DrainCurls();
    CHECK(srv.Count("PATCH", "/api/file/videos") == 1);
    REQUIRE(f->faileduploads.size() == 1);
    CHECK(f->faileduploads.front() == "missing.mp3");
    CHECK_THAT(f->messages.back(), ContainsSubstring("Source file not found"));
}

TEST_CASE("Host specific media is preferred on FPP 6+", "[fpp3][transfer][media]") {
    TempDir dir;
    dir.Write("song.mp3", "generic");
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    f->hostName = "porch";
    std::string base;

    SECTION("same extension") {
        dir.Write("song-porch.mp3", "porch");
        f->CheckUploadMedia((dir.path / "song.mp3").string(), base);
        CHECK(base == "song-porch.mp3");
    }
    SECTION("another media extension") {
        dir.Write("song-porch.ogg", "porch");
        f->CheckUploadMedia((dir.path / "song.mp3").string(), base);
        CHECK(::Lower(base) == "song-porch.ogg"); // either case: the extension list has both and the file system may not care
    }
    SECTION("a folder named for the host") {
        std::filesystem::create_directories(dir.path / "porch");
        dir.Write("porch/song.mp3", "porch");
        f->CheckUploadMedia((dir.path / "song.mp3").string(), base);
        CHECK(base == "song.mp3");
    }
    SECTION("a host suffixed file in the host folder") {
        std::filesystem::create_directories(dir.path / "porch");
        dir.Write("porch/song-porch.wav", "porch");
        f->CheckUploadMedia((dir.path / "song.mp3").string(), base);
        CHECK(::Lower(base) == "song-porch.wav");
    }
    DrainCurls();
    auto up = srv.Sent("PATCH", "/api/file/music");
    REQUIRE(up.size() == 1);
    CHECK(up[0].body == "porch");
}

TEST_CASE("Sequence upload of a full zstd v2 file sends the original", "[fpp3][transfer][sequence]") {
    TempDir dir;
    TmpDirOverride tmp(dir.path);
    auto path = WriteSeq(dir.path / "show.fseq");
    std::unique_ptr<FSEQFile> file(FSEQFile::openFSEQFile(path));
    REQUIRE(file);
    FppServer srv;
    auto f = MakeFpp(srv, 10);
    CHECK_FALSE(f->PrepareUploadSequence(file.get(), path, "", 2, FSEQFile::CompressionType::zstd, false));
    CHECK(f->WillUploadSequence());
    CHECK_FALSE(f->NeedCustomSequence());
    CHECK(f->GetTempFile() == path);
    CHECK(f->sequences["show.fseq"].duration == 0.2f);
    CHECK_FALSE(f->FinalizeUploadSequence());
    DrainCurls();
    auto up = srv.Sent("PATCH", "/api/file/sequences");
    REQUIRE(up.size() == 1);
    CHECK(up[0].body == ReadAll(path));
    CHECK(std::filesystem::exists(path)); // the original is never deleted
    CHECK_FALSE(f->WillUploadSequence());
}

TEST_CASE("Sequence upload is skipped when the FPP already has it", "[fpp3][transfer][sequence]") {
    TempDir dir;
    TmpDirOverride tmp(dir.path);
    auto path = WriteSeq(dir.path / "show.fseq");
    std::unique_ptr<FSEQFile> file(FSEQFile::openFSEQFile(path));
    FppServer srv;
    srv.GetJson("/api/sequence/show.fseq/meta", MetaFor(file.get()));
    auto f = MakeFpp(srv, 10);
    CHECK_FALSE(f->PrepareUploadSequence(file.get(), path, "", 2, FSEQFile::CompressionType::zstd, false));
    CHECK_FALSE(f->WillUploadSequence());

    SECTION("a different ID means a new render") {
        auto m = MetaFor(file.get());
        m["ID"] = "1";
        srv.GetJson("/api/sequence/show.fseq/meta", m);
        f->PrepareUploadSequence(file.get(), path, "", 2, FSEQFile::CompressionType::zstd, false);
        CHECK(f->WillUploadSequence());
    }
    SECTION("a different compression") {
        auto m = MetaFor(file.get(), 0);
        srv.GetJson("/api/sequence/show.fseq/meta", m);
        f->PrepareUploadSequence(file.get(), path, "", 2, FSEQFile::CompressionType::zstd, false);
        CHECK(f->WillUploadSequence());
    }
    SECTION("different frames or step time") {
        auto m = MetaFor(file.get());
        m["StepTime"] = 25;
        srv.GetJson("/api/sequence/show.fseq/meta", m);
        f->PrepareUploadSequence(file.get(), path, "", 2, FSEQFile::CompressionType::zstd, false);
        CHECK(f->WillUploadSequence());
    }
    SECTION("the FPP has a v1 file") {
        auto m = MetaFor(file.get());
        m["Version"] = "1.0";
        m.erase("CompressionType");
        srv.GetJson("/api/sequence/show.fseq/meta", m);
        f->PrepareUploadSequence(file.get(), path, "", 2, FSEQFile::CompressionType::zstd, false);
        CHECK(f->WillUploadSequence());
    }
    SECTION("the FPP copy is sparse") {
        auto m = MetaFor(file.get());
        m["Ranges"] = R"([{"Start":0,"Length":30}])"_json;
        srv.GetJson("/api/sequence/show.fseq/meta", m);
        f->PrepareUploadSequence(file.get(), path, "", 2, FSEQFile::CompressionType::zstd, false);
        CHECK(f->WillUploadSequence());
    }
}

TEST_CASE("Sequence upload uses a prefetched meta answer", "[fpp3][transfer][sequence]") {
    TempDir dir;
    TmpDirOverride tmp(dir.path);
    auto path = WriteSeq(dir.path / "show.fseq");
    std::unique_ptr<FSEQFile> file(FSEQFile::openFSEQFile(path));
    FppServer srv;
    srv.GetJson("/api/sequence/show.fseq/meta", MetaFor(file.get()));
    auto f = MakeFpp(srv, 10);
    f->PrefetchSequenceMeta(path);
    DrainCurls();
    CHECK(f->_prefetchedMetaRC == 200);
    int before = srv.Count("GET", "/api/sequence/show.fseq/meta");
    f->PrepareUploadSequence(file.get(), path, "", 2, FSEQFile::CompressionType::zstd, false);
    CHECK(srv.Count("GET", "/api/sequence/show.fseq/meta") == before);
    CHECK_FALSE(f->WillUploadSequence());
    CHECK(f->_prefetchedMetaName.empty());

    // other device types have no meta to fetch
    auto g = MakeFpp(srv, 10);
    g->fppType = FPP_TYPE::GENIUS;
    g->PrefetchSequenceMeta(path);
    CHECK(g->_prefetchedMetaName.empty());
}

TEST_CASE("Sparse sequence upload builds a copy with this controller's channels", "[fpp3][transfer][sequence]") {
    TempDir dir;
    TmpDirOverride tmp(dir.path);
    auto path = WriteSeq(dir.path / "show.fseq", 2, FSEQFile::CompressionType::zstd, 60, 3);
    std::unique_ptr<FSEQFile> file(FSEQFile::openFSEQFile(path));
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    f->model = "BeagleBone Black";
    f->ranges = "10-19,40";
    CHECK_FALSE(f->PrepareUploadSequence(file.get(), path, "", 2, FSEQFile::CompressionType::zstd, true));
    REQUIRE(f->NeedCustomSequence());
    std::string temp = f->GetTempFile();
    CHECK(std::filesystem::path(temp).parent_path() == std::filesystem::path(dir.path));
    std::vector<uint8_t> frame(60);
    for (uint32_t fr = 0; fr < 3; fr++) {
        for (uint32_t c = 0; c < 60; c++) frame[c] = (uint8_t)(fr * 10 + c);
        f->AddFrameToUpload(fr, frame.data());
    }
    f->FinalizeUploadSequence();
    DrainCurls();
    CHECK_FALSE(std::filesystem::exists(temp)); // the copy is removed once sent
    auto up = srv.Sent("PATCH", "/api/file/sequences");
    REQUIRE(up.size() == 1);
    dir.Write("got.fseq", up[0].body);
    std::unique_ptr<FSEQFile> got(FSEQFile::openFSEQFile((dir.path / "got.fseq").string()));
    REQUIRE(got);
    auto* v2 = dynamic_cast<V2FSEQFile*>(got.get());
    REQUIRE(v2 != nullptr);
    CHECK(v2->m_sparseRanges == std::vector<std::pair<uint32_t, uint32_t>>{ { 10, 10 }, { 40, 4 } });
    CHECK(got->getNumFrames() == 3);

    SECTION("the same ranges already on the FPP are not sent again") {
        auto m = MetaFor(file.get());
        m["Ranges"] = R"([{"Start":10,"Length":10},{"Start":40,"Length":4}])"_json;
        srv.GetJson("/api/sequence/show.fseq/meta", m);
        auto g = MakeFpp(srv, 9);
        g->ranges = "10-19,40";
        g->PrepareUploadSequence(file.get(), path, "", 2, FSEQFile::CompressionType::zstd, true);
        CHECK_FALSE(g->WillUploadSequence());
    }
}

TEST_CASE("Sequence upload of a v1 file and an effect sequence", "[fpp3][transfer][sequence]") {
    TempDir dir;
    TmpDirOverride tmp(dir.path);
    FppServer srv;
    SECTION("v1 to a v1 upload is sent as is") {
        auto path = WriteSeq(dir.path / "old.fseq", 1, FSEQFile::CompressionType::none);
        std::unique_ptr<FSEQFile> file(FSEQFile::openFSEQFile(path));
        auto f = MakeFpp(srv, 9);
        f->PrepareUploadSequence(file.get(), path, "", 1, FSEQFile::CompressionType::none, false);
        CHECK(f->WillUploadSequence());
        CHECK_FALSE(f->NeedCustomSequence());
        f->FinalizeUploadSequence();
        DrainCurls();
        CHECK(srv.Count("PATCH", "/api/file/sequences") == 1);
    }
    SECTION("an effect sequence goes to the effects folder") {
        auto path = WriteSeq(dir.path / "fx.eseq", 2, FSEQFile::CompressionType::zstd, 60, 2, 50, true);
        std::unique_ptr<FSEQFile> file(FSEQFile::openFSEQFile(path));
        auto f = MakeFpp(srv, 9);
        f->PrepareUploadSequence(file.get(), path, "", 2, FSEQFile::CompressionType::zstd, false);
        f->FinalizeUploadSequence();
        DrainCurls();
        CHECK(srv.Count("PATCH", "/api/file/effects") == 1);
    }
    SECTION("an effect sequence that touches none of the controller's channels needs no file") {
        auto path = WriteSeq(dir.path / "fx.fseq", 2, FSEQFile::CompressionType::zstd, 60, 2, 50, true);
        {
            // give the source sparse ranges so the effect's channels are known
            std::unique_ptr<FSEQFile> src(FSEQFile::createFSEQFile((dir.path / "fx2.fseq").string(), 2, FSEQFile::CompressionType::zstd, -99));
            src->setChannelCount(60);
            src->setStepTime(50);
            src->setNumFrames(1);
            FSEQFile::VariableHeader h;
            h.code[0] = 'e';
            h.code[1] = 'S';
            h.data = { 'x', 0 };
            src->addVariableHeader(h);
            dynamic_cast<V2FSEQFile*>(src.get())->m_sparseRanges = { { 0, 6 } };
            src->writeHeader();
            std::vector<uint8_t> fr(6, 1);
            src->addFrame(0, fr.data());
            src->finalize();
        }
        std::unique_ptr<FSEQFile> file(FSEQFile::openFSEQFile((dir.path / "fx2.fseq").string()));
        auto f = MakeFpp(srv, 9);
        f->ranges = "30-39";
        CHECK_FALSE(f->PrepareUploadSequence(file.get(), (dir.path / "fx2.fseq").string(), "", 2, FSEQFile::CompressionType::zstd, true));
        CHECK_FALSE(f->WillUploadSequence());
        CHECK(f->GetTempFile().empty());
        (void)path;
    }
}

// For a non-FPP the copy is left for the controller's own handler (Falcon/Experience) to send.
TEST_CASE("Sequence upload to a non FPP builds an uncompressed copy for its handler", "[fpp3]") {
    TempDir dir;
    TmpDirOverride tmp(dir.path);
    auto path = WriteSeq(dir.path / "show.fseq");
    std::unique_ptr<FSEQFile> file(FSEQFile::openFSEQFile(path));
    FppServer srv;
    srv.Get("/api/file/move/show.fseq", "ok");
    auto f = MakeFpp(srv, 9);
    f->fppType = FPP_TYPE::GENIUS;
    f->PrepareUploadSequence(file.get(), path, "media.mp3", 2, FSEQFile::CompressionType::none, false);
    REQUIRE(f->NeedCustomSequence());
    CHECK(f->sequences["show.fseq"].media.empty()); // only an FPP is sent media
    std::vector<uint8_t> frame(60, 7);
    for (uint32_t fr = 0; fr < 4; fr++) f->AddFrameToUpload(fr, frame.data());
    CHECK_FALSE(f->FinalizeUploadSequence());
    DrainCurls();
    INFO(srv.All());
    CHECK(srv.Count("POST", "/api/file/uploads/show.fseq") == 0);
    REQUIRE_FALSE(f->GetTempFile().empty());
    std::unique_ptr<FSEQFile> copy(FSEQFile::openFSEQFile(f->GetTempFile()));
    REQUIRE(copy);
    CHECK(copy->getNumFrames() == 4);
    f->ClearTempFile();
}

TEST_CASE("Sequence upload with media checks the media too", "[fpp3][transfer][sequence]") {
    TempDir dir;
    TmpDirOverride tmp(dir.path);
    auto path = WriteSeq(dir.path / "show.fseq");
    dir.Write("show.mp3", "audio");
    std::unique_ptr<FSEQFile> file(FSEQFile::openFSEQFile(path));
    FppServer srv;
    auto f = MakeFpp(srv, 9);
    f->PrepareUploadSequence(file.get(), path, (dir.path / "show.mp3").string(), 2, FSEQFile::CompressionType::zstd, false);
    CHECK(f->sequences["show.fseq"].media == "show.mp3");
    f->FinalizeUploadSequence();
    DrainCurls();
    CHECK(srv.Has("GET", "/api/media/show.mp3/meta"));
    CHECK(srv.Count("PATCH", "/api/file/music") == 1);
    CHECK(srv.Count("PATCH", "/api/file/sequences") == 1);
}

#endif

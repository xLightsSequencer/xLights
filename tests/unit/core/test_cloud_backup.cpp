#include <catch2/catch_test_macros.hpp>

#include "support/TempDir.h"
#include "utils/S3Client.h"
#include "utils/Sha256.h"
#include "utils/ShowArchiver.h"

#include <minizip/zip.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {

void WriteFile(const fs::path& p, const std::string& body) {
    fs::create_directories(p.parent_path());
    std::ofstream(p, std::ios::binary) << body;
}

std::string ReadFile(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    return { std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
}

void SetAge(const fs::path& p, std::chrono::hours ago) {
    fs::last_write_time(p, fs::file_time_type::clock::now() - ago);
}

std::string U8(const fs::path& p) {
    auto u = p.u8string();
    return { u.begin(), u.end() };
}

// Hand-made archive, entries stored uncompressed so their bytes can be found in the file.
void MakeZip(const fs::path& zip, const std::vector<std::pair<std::string, std::string>>& entries) {
    zipFile zf = zipOpen64(U8(zip).c_str(), APPEND_STATUS_CREATE);
    REQUIRE(zf != nullptr);
    for (const auto& [name, data] : entries) {
        zip_fileinfo zi{};
        REQUIRE(zipOpenNewFileInZip64(zf, name.c_str(), &zi, nullptr, 0, nullptr, 0, nullptr, 0, 0, 0) == ZIP_OK);
        zipWriteInFileInZip(zf, data.data(), (unsigned)data.size());
        zipCloseFileInZip(zf);
    }
    zipClose(zf, nullptr);
}

const char* XSQ = "<?xml version=\"1.0\"?>\n<xsequence><head><mediaFile>song.mp3</mediaFile></head></xsequence>\n";

// A show folder with one sequence in a subfolder, its audio, a rendered .fseq
// newer than the sequence and a Backup folder the archive must skip.
struct Show {
    TempDir dir;
    fs::path show;
    ShowArchiveOptions options;

    Show() {
        show = dir.path / "My Show";
        WriteFile(show / "xlights_rgbeffects.xml", "<xrgb/>");
        WriteFile(show / "Sequences" / "song.xsq", XSQ);
        WriteFile(show / "Sequences" / "song.fseq", "PSEQ rendered");
        WriteFile(show / "song.mp3", "audio");
        WriteFile(show / "Backup" / "old.zip", "previous backup");
        SetAge(show / "Sequences" / "song.xsq", std::chrono::hours(2));
        SetAge(show / "Sequences" / "song.fseq", std::chrono::hours(1));
        options.showDir = U8(show);
    }
};

bool HasEntry(const ShowArchivePlan& plan, const std::string& zipPath) {
    for (const auto& e : plan.entries) {
        if (e.zipPath == zipPath) {
            return true;
        }
    }
    return false;
}

} // namespace

TEST_CASE("Sha256 matches the reference digests", "[cloud]") {
    CHECK(Sha256::HexOf("", 0) == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    CHECK(Sha256::HexOf("abc", 3) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    TempDir dir;
    std::string big(200000, 'x');
    auto f = dir.Write("big.bin", big);
    auto h = Sha256::HexOfFile(fs::path(f));
    REQUIRE(h.has_value());
    CHECK(*h == Sha256::HexOf(big.data(), big.size()));
}

TEST_CASE("S3Client::UriEncode follows the SigV4 rules", "[cloud]") {
    const std::string key = "a b/c+\xC3\xA9~_.-";
    CHECK(S3Client::UriEncode(key, true) == "a%20b/c%2B%C3%A9~_.-");
    CHECK(S3Client::UriEncode(key, false) == "a%20b%2Fc%2B%C3%A9~_.-");
}

TEST_CASE("ShowArchiver::Scan picks up the show, its .fseq and audio", "[cloud]") {
    Show s;
    auto plan = ShowArchiver::Scan(s.options);
    REQUIRE(plan.sequences.size() == 1);
    const auto& seq = plan.sequences[0];
    CHECK(seq.name == "Sequences/song");
    CHECK(seq.fseqEntry >= 0);
    CHECK(seq.mediaEntry >= 0);
    CHECK(seq.fseqUpToDate);
    CHECK(plan.StaleOrMissingFseq().empty());
    CHECK(HasEntry(plan, "show/xlights_rgbeffects.xml"));
    CHECK_FALSE(HasEntry(plan, "show/Backup/old.zip"));

    SECTION("a .fseq older than its sequence is reported") {
        SetAge(s.show / "Sequences" / "song.fseq", std::chrono::hours(3));
        auto stale = ShowArchiver::Scan(s.options).StaleOrMissingFseq();
        REQUIRE(stale.size() == 1);
        CHECK(stale[0] == "Sequences/song");
    }

    SECTION(".fseq files can be left out") {
        s.options.includeFseq = false;
        auto p = ShowArchiver::Scan(s.options);
        CHECK_FALSE(HasEntry(p, "show/Sequences/song.fseq"));
        REQUIRE(p.sequences.size() == 1);
        CHECK(p.sequences[0].fseqEntry < 0);
        CHECK(p.StaleOrMissingFseq().empty());
    }
}

TEST_CASE("A show survives an archive and restore round trip", "[cloud]") {
    Show s;
    auto plan = ShowArchiver::Scan(s.options);
    plan.extraFiles.push_back({ "thumbnail.png", "not really a png" });
    fs::path zip = s.dir.path / "backup.zip";
    auto made = ShowArchiver::Create(plan, zip);
    REQUIRE(made.ok);
    CHECK(made.archiveSize > 0);

    fs::path dest = s.dir.path / "Restored";
    auto res = ShowArchiver::Extract(zip, dest);
    REQUIRE(res.ok);
    CHECK(res.filesWritten == 4);
    CHECK(res.warnings.empty());
    for (const char* rel : { "xlights_rgbeffects.xml", "Sequences/song.xsq", "Sequences/song.fseq", "song.mp3" }) {
        CHECK(ReadFile(dest / rel) == ReadFile(s.show / rel));
    }
    // Archive-only entries stay out of the show.
    CHECK_FALSE(fs::exists(dest / "manifest.json"));
    CHECK_FALSE(fs::exists(dest / "thumbnail.png"));
    CHECK_FALSE(fs::exists(dest / "Backup"));
    // Dates are kept, so the restored .fseq still reads as up to date.
    CHECK(fs::last_write_time(dest / "Sequences" / "song.fseq") >= fs::last_write_time(dest / "Sequences" / "song.xsq"));
}

TEST_CASE("Restore puts a separate-folder .fseq next to its sequence", "[cloud]") {
    Show s;
    fs::remove(s.show / "Sequences" / "song.fseq");
    fs::remove(s.show / "song.mp3");
    WriteFile(s.dir.path / "FSEQ" / "song.fseq", "PSEQ elsewhere");
    WriteFile(s.dir.path / "Music" / "song.mp3", "external audio");
    s.options.fseqDir = U8(s.dir.path / "FSEQ");
    s.options.mediaDirs.push_back(U8(s.dir.path / "Music"));

    auto plan = ShowArchiver::Scan(s.options);
    CHECK(HasEntry(plan, "fseq/song.fseq"));
    CHECK(HasEntry(plan, "media/song.mp3"));
    fs::path zip = s.dir.path / "backup.zip";
    REQUIRE(ShowArchiver::Create(plan, zip).ok);

    fs::path dest = s.dir.path / "Restored";
    auto res = ShowArchiver::Extract(zip, dest);
    REQUIRE(res.ok);
    CHECK(ReadFile(dest / "Sequences" / "song.fseq") == "PSEQ elsewhere");
    CHECK(ReadFile(dest / "External Media" / "song.mp3") == "external audio");
    CHECK(res.warnings.size() == 2);
}

TEST_CASE("Restore refuses entries that would leave the destination", "[cloud]") {
    TempDir dir;
    fs::path zip = dir.path / "evil.zip";
    MakeZip(zip, { { "show/ok.txt", "fine" },
                   { "show/../../escaped1.txt", "bad" },
                   { "show/sub/../../escaped2.txt", "bad" },
                   { "show/C:/escaped3.txt", "bad" },
                   { "show//escaped4.txt", "bad" },
                   { "fseq/../escaped5.fseq", "bad" } });
    fs::path dest = dir.path / "out" / "inner";
    auto res = ShowArchiver::Extract(zip, dest);
    REQUIRE(res.ok);
    CHECK(res.filesWritten == 1);
    CHECK(ReadFile(dest / "ok.txt") == "fine");
    CHECK(res.warnings.size() == 5);
    for (const auto& e : fs::recursive_directory_iterator(dir.path)) {
        CHECK(e.path().filename().string().rfind("escaped", 0) != 0);
    }
}

TEST_CASE("Restore keeps files that are already there", "[cloud]") {
    TempDir dir;
    fs::path zip = dir.path / "a.zip";
    MakeZip(zip, { { "show/a.txt", "from backup" }, { "show/b.txt", "new" } });
    fs::path dest = dir.path / "out";
    WriteFile(dest / "a.txt", "mine");
    auto res = ShowArchiver::Extract(zip, dest);
    REQUIRE(res.ok);
    CHECK(ReadFile(dest / "a.txt") == "mine");
    CHECK(ReadFile(dest / "b.txt") == "new");
    CHECK(res.warnings.size() == 1);
}

TEST_CASE("A damaged archive fails and leaves nothing behind", "[cloud]") {
    TempDir dir;
    fs::path zip = dir.path / "bad.zip";
    const std::string payload(4096, 'Q');
    MakeZip(zip, { { "show/first.txt", "ok" }, { "show/data.bin", payload } });
    std::string bytes = ReadFile(zip);
    auto pos = bytes.find(payload);
    REQUIRE(pos != std::string::npos);
    bytes[pos + payload.size() / 2] = 'X';
    WriteFile(zip, bytes);

    fs::path dest = dir.path / "out";
    auto res = ShowArchiver::Extract(zip, dest);
    CHECK_FALSE(res.ok);
    CHECK(res.error.find("damaged") != std::string::npos);
    CHECK_FALSE(fs::exists(dest));
}

TEST_CASE("Cancelling stops and cleans up", "[cloud]") {
    Show s;
    auto plan = ShowArchiver::Scan(s.options);
    fs::path zip = s.dir.path / "backup.zip";

    auto cancelledCreate = ShowArchiver::Create(plan, zip, [](uint64_t, uint64_t, const std::string&) { return false; });
    CHECK(cancelledCreate.cancelled);
    CHECK_FALSE(fs::exists(zip));

    REQUIRE(ShowArchiver::Create(plan, zip).ok);
    fs::path dest = s.dir.path / "Restored";
    auto cancelledExtract = ShowArchiver::Extract(zip, dest, [](uint64_t, uint64_t, const std::string&) { return false; });
    CHECK(cancelledExtract.cancelled);
    CHECK_FALSE(fs::exists(dest));
}

TEST_CASE("Uploaded manifests can be read back", "[cloud]") {
    Show s;
    auto plan = ShowArchiver::Scan(s.options);
    plan.extraFiles.push_back({ "thumbnail.png", "png" });
    REQUIRE(ShowArchiver::Create(plan, s.dir.path / "backup.zip").ok);

    ShowArchiver::ManifestInfo info;
    info.showName = "My Show";
    info.createdUtc = "2026-11-06T14:30:00Z";
    info.archiveKey = "xlights/My Show/2026-11-06-143000.zip";
    info.archiveSize = 1234;
    info.archiveSha256 = "abc123";
    info.thumbnailKey = "xlights/My Show/2026-11-06-143000.thumbnail.png";
    auto b = ShowArchiver::ParseManifest(ShowArchiver::BuildManifest(plan, info));
    REQUIRE(b.ok);
    CHECK(b.showName == "My Show");
    CHECK(b.createdUtc == info.createdUtc);
    CHECK(b.archiveKey == info.archiveKey);
    CHECK(b.archiveSize == 1234);
    CHECK(b.archiveSha256 == "abc123");
    CHECK(b.thumbnailKey == info.thumbnailKey);
    CHECK(b.sequenceCount == 1);
    CHECK(b.fileCount == 4);
    CHECK(b.fseqIncluded);

    CHECK_FALSE(ShowArchiver::ParseManifest("").ok);
    CHECK_FALSE(ShowArchiver::ParseManifest("[1, 2]").ok);
    CHECK_FALSE(ShowArchiver::ParseManifest("{\"format\": \"something-else\"}").ok);
    CHECK(ShowArchiver::ParseManifest("{\"format\": \"xlights-show-backup\"}").ok);
}

TEST_CASE("Backup keys use UTC timestamps", "[cloud]") {
    CHECK(ShowArchiver::FormatUtcIso(0) == "1970-01-01T00:00:00Z");
    CHECK(ShowArchiver::FormatUtcForKey(1793284200) == "2026-10-29-143000");
}

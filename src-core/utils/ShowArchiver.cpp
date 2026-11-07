/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowArchiver.h"

#include "ExternalHooks.h"
#include "FileUtils.h"
#include "Sha256.h"
#include "XsqFileScanner.h"
#include "string_utils.h"
#include "xLightsVersion.h"

#include <minizip/unzip.h>
#include <minizip/zip.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>

namespace fs = std::filesystem;

namespace {

std::string ToUtf8(const fs::path& p) {
    auto u = p.u8string();
    return std::string(u.begin(), u.end());
}

std::string ToUtf8Generic(const fs::path& p) {
    auto u = p.generic_u8string();
    return std::string(u.begin(), u.end());
}

fs::path FromUtf8(const std::string& s) {
    return fs::path(std::u8string(s.begin(), s.end()));
}

// Key used to tell whether two paths name the same file.
std::string PathKey(const fs::path& p) {
    std::string k = ToUtf8Generic(p.lexically_normal());
#ifdef _WIN32
    k = Lower(k);
#endif
    while (k.size() > 1 && k.back() == '/') {
        k.pop_back();
    }
    return k;
}

bool IsInside(const fs::path& child, const fs::path& parent) {
    std::string c = PathKey(child);
    std::string p = PathKey(parent);
    return c.size() > p.size() && c.compare(0, p.size(), p) == 0 && c[p.size()] == '/';
}

uint64_t FileSize(const fs::path& p) {
    std::error_code ec;
    auto s = fs::file_size(p, ec);
    return ec ? 0 : (uint64_t)s;
}

long long FileMTime(const fs::path& p) {
    return FileUtils::GetFileModTimeTicks(ToUtf8(p)).value_or(0);
}

// Content that deflate would barely shrink: stored as-is to save CPU time.
bool IsAlreadyCompressed(const fs::path& p) {
    static const std::set<std::string> exts = {
        ".fseq", ".mp3", ".m4a", ".aac", ".ogg", ".opus", ".flac", ".wma",
        ".mp4", ".m4v", ".mov", ".mkv", ".avi", ".webm", ".wmv", ".mpg", ".mpeg",
        ".jpg", ".jpeg", ".png", ".gif", ".webp", ".heic",
        ".zip", ".xsqz", ".7z", ".gz", ".zst", ".rar"
    };
    return exts.count(Lower(ToUtf8(p.extension()))) > 0;
}

bool IsMediaToHash(const fs::path& p) {
    std::string ext = Lower(ToUtf8(p.extension()));
    return ext == ".fseq" || ext == ".mp3" || ext == ".wav" || ext == ".ogg" || ext == ".m4a" ||
           ext == ".flac" || ext == ".aac" || ext == ".mp4" || ext == ".mov" || ext == ".mkv";
}

fs::path ResolveMedia(const std::string& ref, const fs::path& showRoot, const std::list<std::string>& mediaDirs) {
    if (ref.empty()) {
        return {};
    }
    std::string fixed = FileUtils::ToValidUTF8(ref);
    auto exists = [](const fs::path& p) {
        return FileExists(ToUtf8(p), false);
    };
    if (FileUtils::IsAbsoluteOrRootedPath(fixed)) {
        fs::path p = FromUtf8(fixed);
        if (exists(p)) {
            return p;
        }
    } else {
        std::string rel = fixed;
        std::replace(rel.begin(), rel.end(), '\\', '/');
        if (fs::path p = showRoot / FromUtf8(rel); exists(p)) {
            return p;
        }
        for (const auto& d : mediaDirs) {
            if (fs::path p = FromUtf8(d) / FromUtf8(rel); exists(p)) {
                return p;
            }
        }
    }
    fs::path name = FromUtf8(FileUtils::GetFilenameFromPath(fixed));
    if (name.empty()) {
        return {};
    }
    if (fs::path p = showRoot / name; exists(p)) {
        return p;
    }
    for (const auto& d : mediaDirs) {
        if (fs::path p = FromUtf8(d) / name; exists(p)) {
            return p;
        }
    }
    return {};
}

// minizip file callbacks over a std::filesystem::path, so Unicode output
// paths work on Windows whatever the process code page is. zipOpen2_64's
// "pathname" is a pointer to that path.
voidpf ZCALLBACK ZOpen(voidpf, const void* filename, int mode) {
    const fs::path* p = static_cast<const fs::path*>(filename);
    const bool write = (mode & ZLIB_FILEFUNC_MODE_CREATE) != 0;
    const bool rw = (mode & ZLIB_FILEFUNC_MODE_EXISTING) != 0;
#ifdef _WIN32
    const wchar_t* m = write ? L"wb" : (rw ? L"r+b" : L"rb");
    return _wfopen(p->c_str(), m);
#else
    const char* m = write ? "wb" : (rw ? "r+b" : "rb");
    return fopen(p->c_str(), m);
#endif
}

uLong ZCALLBACK ZRead(voidpf, voidpf stream, void* buf, uLong size) {
    return (uLong)fread(buf, 1, size, (FILE*)stream);
}

uLong ZCALLBACK ZWrite(voidpf, voidpf stream, const void* buf, uLong size) {
    return (uLong)fwrite(buf, 1, size, (FILE*)stream);
}

ZPOS64_T ZCALLBACK ZTell(voidpf, voidpf stream) {
#ifdef _WIN32
    return (ZPOS64_T)_ftelli64((FILE*)stream);
#else
    return (ZPOS64_T)ftello((FILE*)stream);
#endif
}

long ZCALLBACK ZSeek(voidpf, voidpf stream, ZPOS64_T offset, int origin) {
    int o = SEEK_SET;
    if (origin == ZLIB_FILEFUNC_SEEK_CUR) {
        o = SEEK_CUR;
    } else if (origin == ZLIB_FILEFUNC_SEEK_END) {
        o = SEEK_END;
    }
#ifdef _WIN32
    return _fseeki64((FILE*)stream, (__int64)offset, o) == 0 ? 0 : -1;
#else
    return fseeko((FILE*)stream, (off_t)offset, o) == 0 ? 0 : -1;
#endif
}

int ZCALLBACK ZClose(voidpf, voidpf stream) {
    return fclose((FILE*)stream);
}

int ZCALLBACK ZError(voidpf, voidpf stream) {
    return ferror((FILE*)stream);
}

zlib_filefunc64_def PathFileFuncs() {
    zlib_filefunc64_def ff{};
    ff.zopen64_file = ZOpen;
    ff.zread_file = ZRead;
    ff.zwrite_file = ZWrite;
    ff.ztell64_file = ZTell;
    ff.zseek64_file = ZSeek;
    ff.zclose_file = ZClose;
    ff.zerror_file = ZError;
    ff.opaque = nullptr;
    return ff;
}

void FillZipTime(zip_fileinfo& zi, long long mtime) {
    time_t t = (time_t)mtime;
    if (t == 0) {
        t = time(nullptr);
    }
    struct tm lt {};
#ifdef _WIN32
    localtime_s(&lt, &t);
#else
    localtime_r(&t, &lt);
#endif
    zi.tmz_date.tm_sec = lt.tm_sec;
    zi.tmz_date.tm_min = lt.tm_min;
    zi.tmz_date.tm_hour = lt.tm_hour;
    zi.tmz_date.tm_mday = lt.tm_mday;
    zi.tmz_date.tm_mon = lt.tm_mon;
    zi.tmz_date.tm_year = lt.tm_year + 1900;
    zi.dosDate = 0;
}

constexpr uLong ZIP_FLAG_UTF8 = 1 << 11;

bool OpenEntry(zipFile zf, const std::string& name, long long mtime, bool store, bool large) {
    zip_fileinfo zi{};
    FillZipTime(zi, mtime);
    return zipOpenNewFileInZip4_64(zf, name.c_str(), &zi, nullptr, 0, nullptr, 0, nullptr,
                                   store ? 0 : Z_DEFLATED, store ? 0 : Z_DEFAULT_COMPRESSION,
                                   0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   nullptr, 0, 0, ZIP_FLAG_UTF8, large ? 1 : 0) == ZIP_OK;
}

std::string FormatUtc(time_t t, const char* fmt) {
    struct tm g {};
#ifdef _WIN32
    gmtime_s(&g, &t);
#else
    gmtime_r(&t, &g);
#endif
    char buf[32];
    strftime(buf, sizeof(buf), fmt, &g);
    return buf;
}

// Normalised relative path, or empty when the entry would escape the
// destination (absolute, drive-qualified or "..").
std::string SafeRelative(std::string p) {
    std::replace(p.begin(), p.end(), '\\', '/');
    if (p.empty() || p.front() == '/' || p.find(':') != std::string::npos) {
        return {};
    }
    std::string out;
    size_t start = 0;
    while (start <= p.size()) {
        size_t end = p.find('/', start);
        if (end == std::string::npos) {
            end = p.size();
        }
        std::string part = p.substr(start, end - start);
        if (part == "..") {
            return {};
        }
        if (!part.empty() && part != ".") {
            if (!out.empty()) {
                out += '/';
            }
            out += part;
        }
        start = end + 1;
    }
    return out;
}

// Restored files keep their original dates, so a restored .fseq still reads
// as newer than its sequence.
void SetMTimeFromZip(const fs::path& p, const tm_unz& d) {
    struct tm t {};
    t.tm_sec = (int)d.tm_sec;
    t.tm_min = (int)d.tm_min;
    t.tm_hour = (int)d.tm_hour;
    t.tm_mday = (int)d.tm_mday;
    t.tm_mon = (int)d.tm_mon;
    t.tm_year = (int)d.tm_year - 1900;
    t.tm_isdst = -1;
    time_t tt = mktime(&t);
    if (tt == (time_t)-1) {
        return;
    }
    auto delta = std::chrono::system_clock::from_time_t(tt) - std::chrono::system_clock::now();
    auto ft = fs::file_time_type::clock::now() + std::chrono::duration_cast<fs::file_time_type::duration>(delta);
    std::error_code ec;
    fs::last_write_time(p, ft, ec);
}

std::string JsonString(const nlohmann::json& o, const char* key) {
    auto it = o.find(key);
    return (it != o.end() && it->is_string()) ? it->get<std::string>() : std::string();
}

uint64_t JsonUInt(const nlohmann::json& o, const char* key) {
    auto it = o.find(key);
    return (it != o.end() && it->is_number_unsigned()) ? it->get<uint64_t>() : 0;
}

} // namespace

std::vector<std::string> ShowArchivePlan::StaleOrMissingFseq() const {
    std::vector<std::string> res;
    if (!options.includeFseq) {
        return res;
    }
    for (const auto& s : sequences) {
        if (s.fseqEntry < 0 || !s.fseqUpToDate) {
            res.push_back(s.name);
        }
    }
    return res;
}

namespace ShowArchiver {

std::string FormatUtcIso(time_t t) {
    return FormatUtc(t, "%Y-%m-%dT%H:%M:%SZ");
}

std::string FormatUtcForKey(time_t t) {
    return FormatUtc(t, "%Y-%m-%d-%H%M%S");
}

ShowArchivePlan Scan(const ShowArchiveOptions& options) {
    ShowArchivePlan plan;
    plan.options = options;

    ObtainAccessToURL(options.showDir);
    const fs::path showRoot = FromUtf8(options.showDir).lexically_normal();
    std::error_code ec;
    if (options.showDir.empty() || !fs::is_directory(showRoot, ec)) {
        plan.warnings.push_back("Show folder not found: " + options.showDir);
        return plan;
    }

    std::set<std::string> excluded;
    for (const auto& n : options.excludeDirNames) {
        excluded.insert(Lower(n));
    }

    std::map<std::string, int> byPath;
    std::set<std::string> usedZipNames;
    auto addEntry = [&](const fs::path& src, const std::string& zipPath) -> int {
        std::string key = PathKey(src);
        if (auto it = byPath.find(key); it != byPath.end()) {
            return it->second;
        }
        ShowArchiveEntry e;
        e.source = src;
        e.zipPath = zipPath;
        e.size = FileSize(src);
        e.mtime = FileMTime(src);
        e.hash = IsMediaToHash(src);
        plan.totalBytes += e.size;
        plan.entries.push_back(std::move(e));
        int idx = (int)plan.entries.size() - 1;
        byPath[key] = idx;
        usedZipNames.insert(Lower(zipPath));
        return idx;
    };
    // Unique name under a top-level folder for files that live outside the show.
    auto externalZipName = [&](const std::string& folder, const fs::path& src) {
        std::string base = ToUtf8(src.filename());
        std::string name = folder + "/" + base;
        for (int i = 2; usedZipNames.count(Lower(name)); ++i) {
            name = folder + "/" + std::to_string(i) + "_" + base;
        }
        return name;
    };

    std::vector<fs::path> xsqFiles;
    fs::recursive_directory_iterator it(showRoot, fs::directory_options::skip_permission_denied, ec);
    if (ec) {
        plan.warnings.push_back("Unable to read show folder: " + ec.message());
        return plan;
    }
    for (; it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) {
            plan.warnings.push_back("Error while reading show folder: " + ec.message());
            ec.clear();
            continue;
        }
        const auto& de = *it;
        std::error_code sec;
        if (de.is_symlink(sec)) {
            if (de.is_directory(sec)) {
                it.disable_recursion_pending();
            }
            continue;
        }
        if (de.is_directory(sec)) {
            if (excluded.count(Lower(ToUtf8(de.path().filename())))) {
                it.disable_recursion_pending();
            }
            continue;
        }
        if (!de.is_regular_file(sec)) {
            continue;
        }
        const std::string ext = Lower(ToUtf8(de.path().extension()));
        if (!options.includeFseq && ext == ".fseq") {
            continue;
        }
        fs::path rel = de.path().lexically_relative(showRoot);
        addEntry(de.path(), "show/" + ToUtf8Generic(rel));
        if (ext == ".xsq") {
            xsqFiles.push_back(de.path());
        }
    }

    const bool fseqElsewhere = !options.fseqDir.empty() && PathKey(FromUtf8(options.fseqDir)) != PathKey(showRoot);
    fs::path fseqRoot;
    if (fseqElsewhere) {
        ObtainAccessToURL(options.fseqDir);
        fseqRoot = FromUtf8(options.fseqDir).lexically_normal();
    }
    for (const auto& d : options.mediaDirs) {
        ObtainAccessToURL(d);
    }

    std::sort(xsqFiles.begin(), xsqFiles.end());
    for (const auto& xsq : xsqFiles) {
        XsqFileInfo info = ScanXsqFile(ToUtf8(xsq));
        if (!info.isSequence) {
            continue;
        }
        ShowArchiveSequence seq;
        fs::path rel = xsq.lexically_relative(showRoot);
        seq.xsqZipPath = "show/" + ToUtf8Generic(rel);
        seq.name = ToUtf8Generic(fs::path(rel).replace_extension());
        seq.mediaRef = info.mediaFile;

        fs::path fseq = fseqElsewhere ? fseqRoot / xsq.stem() : xsq;
        fseq.replace_extension(".fseq");
        if (options.includeFseq && FileExists(ToUtf8(fseq), false)) {
            std::string zipName = IsInside(fseq, showRoot) ? "show/" + ToUtf8Generic(fseq.lexically_relative(showRoot))
                                                           : externalZipName("fseq", fseq);
            seq.fseqEntry = addEntry(fseq, zipName);
            seq.fseqUpToDate = plan.entries[seq.fseqEntry].mtime >= FileMTime(xsq);
        }

        if (!info.mediaFile.empty()) {
            fs::path media = ResolveMedia(info.mediaFile, showRoot, options.mediaDirs);
            if (media.empty()) {
                plan.warnings.push_back("Audio for " + seq.name + " not found: " + info.mediaFile);
            } else {
                std::string zipName = IsInside(media, showRoot) ? "show/" + ToUtf8Generic(media.lexically_relative(showRoot))
                                                                : externalZipName("media", media);
                seq.mediaEntry = addEntry(media, zipName);
                plan.entries[seq.mediaEntry].hash = true;
            }
        }
        plan.sequences.push_back(std::move(seq));
    }

    spdlog::info("ShowArchiver: scanned '{}': {} files, {} bytes, {} sequences",
                 options.showDir, plan.entries.size(), plan.totalBytes, plan.sequences.size());
    return plan;
}

ShowArchiveResult Create(ShowArchivePlan& plan, const fs::path& outZip, const ShowArchiveProgress& progress) {
    ShowArchiveResult res;

    zlib_filefunc64_def ff = PathFileFuncs();
    zipFile zf = zipOpen2_64(&outZip, APPEND_STATUS_CREATE, nullptr, &ff);
    if (zf == nullptr) {
        res.error = "Unable to create archive " + ToUtf8(outZip);
        spdlog::error("ShowArchiver: {}", res.error);
        return res;
    }

    const std::string outKey = PathKey(outZip);
    std::vector<char> buf(1024 * 1024);
    uint64_t done = 0;
    bool failed = false;

    for (auto& e : plan.entries) {
        e.written = false;
        e.sha256.clear();
        if (PathKey(e.source) == outKey) {
            continue;
        }
        if (progress && !progress(done, plan.totalBytes, e.zipPath)) {
            res.cancelled = true;
            break;
        }
        // Open the source before creating the entry: minizip cannot remove an
        // entry, so a failed read would otherwise leave a corrupt stub behind.
        const std::string srcUtf8 = ToUtf8(e.source);
        std::ifstream in;
        if (FileExists(srcUtf8)) {
            in.open(e.source, std::ios::binary);
        }
        if (!in.is_open()) {
            res.warnings.push_back("Could not read " + srcUtf8);
            done += e.size;
            continue;
        }
        if (!OpenEntry(zf, e.zipPath, e.mtime, IsAlreadyCompressed(e.source), e.size >= 0xFFFFFFFFull)) {
            res.error = "Unable to add " + e.zipPath + " to the archive";
            failed = true;
            break;
        }
        Sha256 hasher;
        bool writeOk = true;
        while (in) {
            in.read(buf.data(), (std::streamsize)buf.size());
            std::streamsize n = in.gcount();
            if (n <= 0) {
                break;
            }
            if (e.hash) {
                hasher.update(buf.data(), (size_t)n);
            }
            if (zipWriteInFileInZip(zf, buf.data(), (unsigned)n) != ZIP_OK) {
                writeOk = false;
                break;
            }
            done += (uint64_t)n;
            if (progress && !progress(done, plan.totalBytes, e.zipPath)) {
                res.cancelled = true;
                break;
            }
        }
        zipCloseFileInZip(zf);
        if (!writeOk) {
            res.error = "Write error while adding " + e.zipPath + " (disk full?)";
            failed = true;
            break;
        }
        if (res.cancelled) {
            break;
        }
        if (in.bad()) {
            res.warnings.push_back("Read error on " + srcUtf8 + ", archived copy may be incomplete");
        }
        e.written = true;
        if (e.hash) {
            e.sha256 = Sha256::ToHex(hasher.finish());
        }
    }

    for (const auto& [name, data] : plan.extraFiles) {
        if (failed || res.cancelled) {
            break;
        }
        if (!OpenEntry(zf, name, time(nullptr), IsAlreadyCompressed(FromUtf8(name)), false) ||
            zipWriteInFileInZip(zf, data.data(), (unsigned)data.size()) != ZIP_OK) {
            res.error = "Unable to write " + name + " into the archive";
            failed = true;
        }
        zipCloseFileInZip(zf);
    }

    if (!failed && !res.cancelled) {
        ShowArchiver::ManifestInfo info;
        info.showName = ToUtf8(FromUtf8(plan.options.showDir).lexically_normal().filename());
        info.createdUtc = FormatUtcIso(time(nullptr));
        std::string manifest = BuildManifest(plan, info);
        if (!OpenEntry(zf, "manifest.json", time(nullptr), false, false) ||
            zipWriteInFileInZip(zf, manifest.data(), (unsigned)manifest.size()) != ZIP_OK) {
            res.error = "Unable to write manifest.json into the archive";
            failed = true;
        }
        zipCloseFileInZip(zf);
    }

    std::string comment = "xLights " + xlights_version_string + " show backup";
    if (zipClose(zf, comment.c_str()) != ZIP_OK && !failed && !res.cancelled) {
        res.error = "Unable to finalize archive (disk full?)";
        failed = true;
    }

    std::error_code ec;
    if (failed || res.cancelled) {
        fs::remove(outZip, ec);
        spdlog::warn("ShowArchiver: archive {} ({})", res.cancelled ? "cancelled" : "failed", res.error);
        return res;
    }
    res.ok = true;
    res.archiveSize = FileSize(outZip);
    res.warnings.insert(res.warnings.begin(), plan.warnings.begin(), plan.warnings.end());
    spdlog::info("ShowArchiver: wrote {} ({} bytes, {} warnings)", ToUtf8(outZip), res.archiveSize, res.warnings.size());
    return res;
}

std::string BuildManifest(const ShowArchivePlan& plan, const ManifestInfo& info) {
    nlohmann::ordered_json j;
    j["format"] = "xlights-show-backup";
    j["formatVersion"] = 1;
    j["xlightsVersion"] = xlights_version_string;
    j["showName"] = info.showName;
    j["createdUtc"] = info.createdUtc;
    if (!info.archiveKey.empty()) {
        j["archive"] = { { "key", info.archiveKey }, { "size", info.archiveSize }, { "sha256", info.archiveSha256 } };
    }

    uint64_t bytes = 0;
    int files = 0;
    for (const auto& e : plan.entries) {
        if (e.written) {
            bytes += e.size;
            ++files;
        }
    }
    j["fileCount"] = files;
    j["totalBytes"] = bytes;
    j["fseqIncluded"] = plan.options.includeFseq;
    for (const auto& [name, data] : plan.extraFiles) {
        if (name == "thumbnail.png") {
            j["thumbnail"] = name;
        }
    }
    if (!info.thumbnailKey.empty()) {
        j["thumbnailKey"] = info.thumbnailKey;
    }

    auto fileJson = [&](int idx) -> nlohmann::ordered_json {
        if (idx < 0 || !plan.entries[idx].written) {
            return nullptr;
        }
        const auto& e = plan.entries[idx];
        nlohmann::ordered_json f = { { "path", e.zipPath }, { "size", e.size } };
        if (!e.sha256.empty()) {
            f["sha256"] = e.sha256;
        }
        return f;
    };

    nlohmann::ordered_json seqs = nlohmann::ordered_json::array();
    for (const auto& s : plan.sequences) {
        nlohmann::ordered_json js;
        js["name"] = s.name;
        js["xsq"] = s.xsqZipPath;
        auto fseq = fileJson(s.fseqEntry);
        if (!fseq.is_null()) {
            fseq["upToDate"] = s.fseqUpToDate;
        }
        js["fseq"] = fseq;
        js["media"] = fileJson(s.mediaEntry);
        seqs.push_back(js);
    }
    j["sequences"] = seqs;
    return j.dump(2);
}

BackupSummary ParseManifest(const std::string& json) {
    BackupSummary b;
    auto j = nlohmann::json::parse(json, nullptr, false);
    if (!j.is_object() || JsonString(j, "format") != "xlights-show-backup") {
        return b;
    }
    b.ok = true;
    b.showName = JsonString(j, "showName");
    b.createdUtc = JsonString(j, "createdUtc");
    b.xlightsVersion = JsonString(j, "xlightsVersion");
    b.thumbnailKey = JsonString(j, "thumbnailKey");
    b.fileCount = (int)JsonUInt(j, "fileCount");
    b.totalBytes = JsonUInt(j, "totalBytes");
    if (auto it = j.find("fseqIncluded"); it != j.end() && it->is_boolean()) {
        b.fseqIncluded = it->get<bool>();
    }
    if (auto it = j.find("archive"); it != j.end() && it->is_object()) {
        b.archiveKey = JsonString(*it, "key");
        b.archiveSize = JsonUInt(*it, "size");
        b.archiveSha256 = JsonString(*it, "sha256");
    }
    if (auto it = j.find("sequences"); it != j.end() && it->is_array()) {
        b.sequenceCount = (int)it->size();
    }
    return b;
}

ShowRestoreResult Extract(const fs::path& zip, const fs::path& destDir, const ShowArchiveProgress& progress) {
    ShowRestoreResult res;

    zlib_filefunc64_def ff = PathFileFuncs();
    unzFile uf = unzOpen2_64(&zip, &ff);
    if (uf == nullptr) {
        res.error = "Unable to open the archive " + ToUtf8(zip);
        return res;
    }

    struct Item {
        std::string name;
        uint64_t size = 0;
        tm_unz date{};
    };
    std::vector<Item> items;
    std::string manifest;
    uint64_t total = 0;
    std::vector<char> buf(1024 * 1024);
    std::vector<char> name(4096);

    for (int rc = unzGoToFirstFile(uf); rc == UNZ_OK; rc = unzGoToNextFile(uf)) {
        unz_file_info64 fi{};
        if (unzGetCurrentFileInfo64(uf, &fi, name.data(), (uLong)name.size(), nullptr, 0, nullptr, 0) != UNZ_OK) {
            res.error = "The archive is damaged";
            unzClose(uf);
            return res;
        }
        Item it;
        it.name = name.data();
        it.size = fi.uncompressed_size;
        it.date = fi.tmu_date;
        if (it.name == "manifest.json" && it.size < 64ull * 1024 * 1024 && unzOpenCurrentFile(uf) == UNZ_OK) {
            int n;
            while ((n = unzReadCurrentFile(uf, buf.data(), (unsigned)buf.size())) > 0) {
                manifest.append(buf.data(), (size_t)n);
            }
            unzCloseCurrentFile(uf);
        }
        total += it.size;
        items.push_back(std::move(it));
    }

    // .fseq files from a separate FSEQ folder go back next to their sequence,
    // where xLights looks for them when the FSEQ folder is the show folder.
    std::map<std::string, std::string> fseqTargets;
    auto j = nlohmann::json::parse(manifest, nullptr, false);
    if (j.is_object()) {
        if (auto seqs = j.find("sequences"); seqs != j.end() && seqs->is_array()) {
            for (const auto& sq : *seqs) {
                if (!sq.is_object()) {
                    continue;
                }
                auto f = sq.find("fseq");
                if (f == sq.end() || !f->is_object()) {
                    continue;
                }
                std::string xsq = JsonString(sq, "xsq");
                std::string fp = JsonString(*f, "path");
                if (StartsWith(fp, "fseq/") && StartsWith(xsq, "show/")) {
                    fs::path t = FromUtf8(xsq.substr(5));
                    t.replace_extension(".fseq");
                    fseqTargets[fp] = ToUtf8Generic(t);
                }
            }
        }
    }

    std::error_code ec;
    const bool createdDest = !fs::exists(destDir, ec);
    fs::create_directories(destDir, ec);
    if (!fs::is_directory(destDir, ec)) {
        res.error = "Unable to create the folder " + ToUtf8(destDir);
        unzClose(uf);
        return res;
    }
    auto space = fs::space(destDir, ec);
    if (!ec && space.available < total + 16ull * 1024 * 1024) {
        res.error = "Not enough free disk space in " + ToUtf8(destDir) + ": " + std::to_string(total / (1024 * 1024)) +
                    " MB needed, " + std::to_string(space.available / (1024 * 1024)) + " MB available.";
        unzClose(uf);
        if (createdDest) {
            fs::remove(destDir, ec);
        }
        return res;
    }

    std::vector<fs::path> written;
    uint64_t done = 0;
    bool failed = false;
    size_t index = 0;
    bool separateFseq = false;
    bool externalMedia = false;

    for (int rc = unzGoToFirstFile(uf); rc == UNZ_OK && index < items.size(); rc = unzGoToNextFile(uf), ++index) {
        const Item& it = items[index];
        std::string target;
        if (StartsWith(it.name, "show/")) {
            target = it.name.substr(5);
        } else if (StartsWith(it.name, "fseq/")) {
            auto t = fseqTargets.find(it.name);
            target = t != fseqTargets.end() ? t->second : it.name.substr(5);
        } else if (StartsWith(it.name, "media/")) {
            target = "External Media/" + it.name.substr(6);
        } else {
            done += it.size;
            continue;
        }
        const bool isDir = !it.name.empty() && it.name.back() == '/';
        std::string rel = SafeRelative(target);
        if (rel.empty()) {
            if (!isDir) {
                res.warnings.push_back("Skipped " + it.name + ": invalid path");
            }
            done += it.size;
            continue;
        }
        fs::path out = destDir / FromUtf8(rel);
        if (isDir) {
            fs::create_directories(out, ec);
            continue;
        }
        if (progress && !progress(done, total, rel)) {
            res.cancelled = true;
            break;
        }
        if (fs::exists(out, ec)) {
            res.warnings.push_back("Skipped " + rel + ": a file with that name already exists");
            done += it.size;
            continue;
        }
        fs::create_directories(out.parent_path(), ec);
        if (unzOpenCurrentFile(uf) != UNZ_OK) {
            res.error = "Unable to read " + it.name + " from the archive";
            failed = true;
            break;
        }
        std::ofstream o(out, std::ios::binary | std::ios::trunc);
        if (!o.is_open()) {
            unzCloseCurrentFile(uf);
            res.error = "Unable to create " + ToUtf8(out);
            failed = true;
            break;
        }
        written.push_back(out);
        int n;
        while ((n = unzReadCurrentFile(uf, buf.data(), (unsigned)buf.size())) > 0) {
            o.write(buf.data(), n);
            if (!o) {
                break;
            }
            done += (uint64_t)n;
            if (progress && !progress(done, total, rel)) {
                res.cancelled = true;
                break;
            }
        }
        const bool writeOk = (bool)o;
        o.close();
        int closeRc = unzCloseCurrentFile(uf);
        if (res.cancelled) {
            break;
        }
        if (n < 0 || closeRc == UNZ_CRCERROR) {
            res.error = "The archive is damaged: " + it.name + " could not be read correctly";
            failed = true;
            break;
        }
        if (!writeOk) {
            res.error = "Write error on " + ToUtf8(out) + " (disk full?)";
            failed = true;
            break;
        }
        SetMTimeFromZip(out, it.date);
        ++res.filesWritten;
        separateFseq |= StartsWith(it.name, "fseq/");
        externalMedia |= StartsWith(it.name, "media/");
    }
    unzClose(uf);

    if (failed || res.cancelled) {
        if (createdDest) {
            fs::remove_all(destDir, ec);
        } else {
            for (const auto& p : written) {
                fs::remove(p, ec);
            }
        }
        spdlog::warn("ShowArchiver: restore {} ({})", res.cancelled ? "cancelled" : "failed", res.error);
        return res;
    }
    if (separateFseq) {
        res.warnings.push_back("This show used a separate FSEQ folder. The .fseq files were placed next to their "
                               "sequences: after opening the show, check the FSEQ folder in Preferences > Sequences.");
    }
    if (externalMedia) {
        res.warnings.push_back("Audio files that were outside the show folder were placed in \"External Media\".");
    }
    res.ok = true;
    spdlog::info("ShowArchiver: restored {} files from {} to {}", res.filesWritten, ToUtf8(zip), ToUtf8(destDir));
    return res;
}

} // namespace ShowArchiver

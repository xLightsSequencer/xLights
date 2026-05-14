/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CurlManager.h"
#include "CachedFileDownloader.h"
#include "utils/ExternalHooks.h"

#include <log.h>
#include <mutex>

#include <filesystem>
#include <fstream>
#include <chrono>
#include <memory>
#include <algorithm>
#include <cstdlib>

#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

#define LONGCACHEDAYS 5

static std::string CreateTempFile(const std::string& prefix, const std::string& extension) {
    auto tempDir = std::filesystem::temp_directory_path();
    static int counter = 0;
    std::string name = prefix + "_" + std::to_string(getpid()) + "_" + std::to_string(counter++);
    if (!extension.empty()) name += "." + extension;
    return (tempDir / name).string();
}

FileCacheItem::FileCacheItem(pugi::xml_node n)
{
    _url = n.attribute("URI").as_string();
    _fileName = n.attribute("FileName").as_string();
    _cacheFor = (CACHEFOR)n.attribute("CacheFor").as_int(0);
}

FileCacheItem::FileCacheItem(const std::string& url, CACHEFOR cacheFor, const std::string& forceType, std::function<bool(int)> progressCallback, int low, int high, bool keepProgress)
{
    _url = url;
    _cacheFor = cacheFor;
    Download(forceType, progressCallback, low, high, keepProgress);
}

void FileCacheItem::Save(std::ofstream& f)
{
    f << "  <item URI=\"" << _url
      << "\" FileName=\"" << _fileName
      << "\" CacheFor=\"" << _cacheFor
      << "\"/>\n";
}

void FileCacheItem::Download(const std::string& forceType, std::function<bool(int)> progressCallback, int low, int high, bool keepProgress)
{
    _fileName = DownloadURLToTemp(_url, forceType, progressCallback, low, high, keepProgress);
}

void FileCacheItem::Delete() const
{

    if (Exists())
    {
        spdlog::debug("Removing cached URL {}.", _url);
        std::error_code ec;
        std::filesystem::remove(_fileName, ec);
    }
}
bool FileCacheItem::Exists() const {
    return FileExists(_fileName);
}

void FileCacheItem::Touch() const {
    if (Exists()) {
        std::error_code ec;
        std::filesystem::last_write_time(_fileName, std::filesystem::file_time_type::clock::now(), ec);
    }
}

bool FileCacheItem::operator==(const std::string& url) const
{
    return _url == url;
}

namespace {
// Mutex-guarded slot for the platform-supplied URL fetcher hook.
std::mutex& url_fetcher_mutex() {
    static std::mutex m;
    return m;
}
CachedFileDownloader::URLFetcher& url_fetcher_slot() {
    static CachedFileDownloader::URLFetcher f;
    return f;
}
} // namespace

void CachedFileDownloader::SetURLFetcher(URLFetcher fetcher) {
    std::lock_guard<std::mutex> l(url_fetcher_mutex());
    url_fetcher_slot() = std::move(fetcher);
}

CachedFileDownloader::URLFetcher CachedFileDownloader::GetURLFetcher() {
    std::lock_guard<std::mutex> l(url_fetcher_mutex());
    return url_fetcher_slot();
}

// A major constraint of this function is that it does not support https
bool FileCacheItem::DownloadURL(const std::string& url, const std::string& filename, std::function<bool(int)> progressCallback, int low, int high, bool keepProgress)
{
    spdlog::debug("Making request to '{}' -> {}.", url, filename);
    // Platform override (iPad: NSURLSession). The override skips
    // the curl progress callback because iPad consumers don't
    // surface per-file progress for catalog fetches; the
    // catalog-wide progress is reported via VendorCatalog's
    // `Load` callback.
    if (auto fetcher = CachedFileDownloader::GetURLFetcher()) {
        return fetcher(url, filename);
    }
    std::function<bool(int)> progress;
    if (progressCallback != nullptr) {
        progress = [progressCallback, low, high, keepProgress](int pos) {
            int span = high - low;
            if (span <= 0) {
                span = 1;
            }
            int scaled = low + ((span * pos) / 1000);
            if (keepProgress && scaled >= high) {
                scaled = high - 1;
            }
            if (scaled < low) {
                scaled = low;
            }
            return progressCallback(scaled);
        };
    }
    return CurlManager::HTTPSGetFile(url, filename, "", "", 600, progress);
}

std::string FileCacheItem::DownloadURLToTemp(const std::string& url, const std::string& forceType, std::function<bool(int)> progressCallback, int low, int high, bool keepProgress)
{
    // Extract file extension from URL path
    std::string type;
    auto lastSlash = url.rfind('/');
    auto lastDot = url.rfind('.');
    if (lastDot != std::string::npos && (lastSlash == std::string::npos || lastDot > lastSlash)) {
        type = url.substr(lastDot + 1);
        // Strip query string if present
        auto qpos = type.find('?');
        if (qpos != std::string::npos) {
            type = type.substr(0, qpos);
        }
    }

    if (!forceType.empty()) type = forceType;

    std::string filename = CreateTempFile("xl", type);

    if (DownloadURL(url, filename, progressCallback, low, high, keepProgress)) {
        return filename;
    }

    return "";
}

void FileCacheItem::PurgeIfAged() const
{

    if (!Exists()) return;

    std::error_code ec;
    auto modTime = std::filesystem::last_write_time(_fileName, ec);
    if (ec) return;

    // Use file_clock for comparison since clock_cast may not be available
    auto fileNow = std::filesystem::file_time_type::clock::now();
    auto age = fileNow - modTime;
    auto ageHours = std::chrono::duration_cast<std::chrono::hours>(age).count();

    switch(_cacheFor)
    {
    case CACHETIME_SESSION:
        // do nothing
        break;
    case CACHETIME_DAY:
        {
            // Purge if more than 24 hours old (approximation of "not today")
            if (ageHours >= 24)
            {
                spdlog::debug("{} purged from file cache because it was not created today.", _url);
                Delete();
            }
        }
        break;
    case CACHETIME_FOREVER:
        // do nothing
        break;
    case CACHETIME_LONG:
        {
            auto ageDays = ageHours / 24;
            if (ageDays > LONGCACHEDAYS)
            {
                spdlog::debug("{} purged from file cache because it was more than {} days old: {}.", _url, LONGCACHEDAYS, ageDays);
                Delete();
            }
        }
        break;
    }
}

void CachedFileDownloader::SaveCache()
{
    std::lock_guard<std::recursive_mutex> lock(_cacheItemsLock);
    if (!Initialize())
    {
        return;
    }

    spdlog::debug("Saving File Cache {}.", _cacheFile);

    std::ofstream f(_cacheFile);
    if (f.is_open())
    {
        f << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        f << "<filecache>\n";

        int i = 0;
        for (const auto& it : _cacheItems)
        {
            if (it->ShouldSave())
            {
                it->Save(f);
                i++;
            }
        }

        f << "</filecache>";
        f.close();
        spdlog::debug("    File Cache {} items saved.", i);
    }
    else
    {
        spdlog::warn("    Problem saving File Cache.");
    }
}

void CachedFileDownloader::LoadCache()
{
    std::lock_guard<std::recursive_mutex> lock(_cacheItemsLock);
    _cacheItems.clear();

    if (!Initialize())
    {
        return;
    }

    spdlog::debug("Loading File Cache {}.", _cacheFile);

    std::error_code ec;
    if (FileExists(_cacheFile) && std::filesystem::file_size(_cacheFile, ec) > 0 && !ec)
    {
        pugi::xml_document d;
        if (d.load_file(_cacheFile.c_str()))
        {
            pugi::xml_node root = d.document_element();
            std::string rootName = root.name();
            std::transform(rootName.begin(), rootName.end(), rootName.begin(), ::tolower);
            if (root && rootName == "filecache")
            {
                spdlog::debug("   Cache opened.");
                for (pugi::xml_node n = root.first_child(); n; n = n.next_sibling())
                {
                    std::string nName = n.name();
                    std::transform(nName.begin(), nName.end(), nName.begin(), ::tolower);
                    if (nName == "item")
                        _cacheItems.push_back(new FileCacheItem(n));
                }
                spdlog::debug("   {} items loaded.", _cacheItems.size());
            }
        }
        else
        {
            spdlog::warn("File Cache was invalid.");
        }
    }
    else
    {
        spdlog::warn("File Cache does not exist.");
    }
}

FileCacheItem* CachedFileDownloader::Find(const std::string& url)
{
    std::lock_guard<std::recursive_mutex> lock(_cacheItemsLock);
    for (const auto& it : _cacheItems)
    {
        if (*it == url)
        {
            return it;
        }
    }

    return nullptr;
}

static std::unique_ptr<CachedFileDownloader> _defaultCache(nullptr);
static std::mutex _defaultCacheLock;
CachedFileDownloader& CachedFileDownloader::GetDefaultCache() {
    std::unique_lock<std::mutex> lock(_defaultCacheLock);
    if (_defaultCache.get() == nullptr) {
        std::unique_ptr<CachedFileDownloader> tmp(new CachedFileDownloader());
        _defaultCache = std::move(tmp);
    }
    return *_defaultCache.get();
}

CachedFileDownloader::CachedFileDownloader() : _cacheDir("")
{
    _initialised = false;
}
bool CachedFileDownloader::Initialize() {
    std::lock_guard<std::recursive_mutex> lock(_cacheItemsLock);
    if (_initialised) {
        return _enabled;
    }

    _initialised = true;

    #ifdef LINUX
    // On linux we disable caching because GetTempDir() fails spectacularly probably due to
    // a lack of wxWidgets initialisation when creating static objects
    _cacheFile="";
    spdlog::warn("CachedFileDownloaded disabled on Linux.");
    #else
    std::error_code ec;
    if (_cacheDir.empty() || !std::filesystem::is_directory(_cacheDir, ec))
    {
        _cacheDir = std::filesystem::temp_directory_path(ec).string();
    }

    if (!_cacheDir.empty())
    {
        _cacheFile = _cacheDir + "/xLightsCache.xml";
        _enabled = true;
    }
    else
    {
        spdlog::warn("CachedFileDownloaded unable to find a temp directory to use. Caching disabled.");
    }
    #endif // LINUX

    LoadCache();
    PurgeAgedItems();
    return _enabled;
}

CachedFileDownloader::~CachedFileDownloader()
{
    // Dont save ... this is done when the dialog is exited.
    //SaveCache();
}

void CachedFileDownloader::ClearCache()
{
    spdlog::debug("File Cache cleared.");
    std::lock_guard<std::recursive_mutex> lock(_cacheItemsLock);
    for (const auto& it :  _cacheItems)
    {
        it->Delete();
    }
}

void CachedFileDownloader::PurgeAgedItems()
{
    spdlog::debug("File Cache purging aged items.");
    std::lock_guard<std::recursive_mutex> lock(_cacheItemsLock);
    for (const auto& it : _cacheItems)
    {
        it->PurgeIfAged();
    }
}

int CachedFileDownloader::size() {
    std::lock_guard<std::recursive_mutex> lock(_cacheItemsLock);
    if (!Initialize()) {
        return 0;
    }
    return _cacheItems.size();
}

std::string CachedFileDownloader::GetFile(const std::string& url, CACHEFOR cacheFor, const std::string& forceType, std::function<bool(int)> progressCallback, int low, int high, bool keepProgress)
{
    if (url.empty()) {
        return "";
    }

    std::lock_guard<std::recursive_mutex> lock(_cacheItemsLock);
    if (!Initialize())
    {
        // because we dont have a valid place to save the cache we cant cache anything beyond this session
        cacheFor = CACHEFOR::CACHETIME_SESSION;
    }

    FileCacheItem* fci = Find(url);
    bool needsSave = false;
    if (fci == nullptr)
    {
        spdlog::debug("File Cache downloading file {}.", url);
        fci = new FileCacheItem(url, cacheFor, forceType, progressCallback, low, high, keepProgress);
        _cacheItems.push_back(fci);
        needsSave = true;
    }
    else if (!fci->Exists())
    {
        spdlog::debug("File Cache re-downloading file {}.", url);
        fci->Download(forceType, progressCallback, low, high, keepProgress);
        needsSave = true;
    }

    if (fci->GetFileName() == "")
    {
        spdlog::debug("File Cache file {} could not be retrieved.", url);
    }

    if (needsSave && fci->Exists())
    {
        SaveCache();
    }

    if (progressCallback != nullptr)
    {
        progressCallback(high);
    }

    return fci->GetFileName();
}

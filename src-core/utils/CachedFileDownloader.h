#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <list>
#include <mutex>
#include <functional>
#include <fstream>

#include <pugixml.hpp>

typedef enum
{
                CACHETIME_SESSION,
                CACHETIME_DAY,
                CACHETIME_LONG,
                CACHETIME_FOREVER
} CACHEFOR;

class FileCacheItem
{
    std::string _url;
    CACHEFOR _cacheFor;
    std::string _fileName;

public:
    FileCacheItem(pugi::xml_node n);
    FileCacheItem(const std::string& url, CACHEFOR cacheFor, const std::string& forceType = "", std::function<bool(int)> progressCallback = nullptr, int low = 0, int high = 100, bool keepProgress = false);
    void Save(std::ofstream& f);
    virtual ~FileCacheItem() {}
    void Download(const std::string& forceType = "", std::function<bool(int)> progressCallback = nullptr, int low = 0, int high = 100, bool keepProgress = false);
    bool Exists() const;
    void Touch() const;
    void Delete() const;
    std::string GetFileName() const { if (Exists()) return _fileName; else return ""; }
    bool operator==(const std::string& url) const;
    static bool DownloadURL(const std::string& url, const std::string& filename, std::function<bool(int)> progressCallback = nullptr, int low = 0, int high = 100, bool keepProgress = false);
    std::string DownloadURLToTemp(const std::string& url, const std::string& forceType = "", std::function<bool(int)> progressCallback = nullptr, int low = 0, int high = 100, bool keepProgress = false);
    void PurgeIfAged() const;
    bool ShouldSave() { PurgeIfAged();  return Exists() && (_cacheFor == CACHETIME_DAY || _cacheFor == CACHETIME_LONG); }
};

class CachedFileDownloader
{
    std::string _cacheDir;
    std::list<FileCacheItem*> _cacheItems;
    std::recursive_mutex _cacheItemsLock;
    std::string _cacheFile;
    bool _initialised;
    bool _enabled;

    void PurgeAgedItems();
    void SaveCache();
    void LoadCache();
    bool Initialize();
    FileCacheItem* Find(const std::string& url);

    CachedFileDownloader();
public:

    virtual ~CachedFileDownloader();
    // erase everything from cache
    void ClearCache();
    void Save() { SaveCache(); }
    // retrieve a file from cache … if not present filename will be ""
    std::string GetFile(const std::string& url, CACHEFOR cacheFor, const std::string& forceType = "", std::function<bool(int)> progressCallback = nullptr, int low = 0, int high = 100, bool keepProgress = false);
    int size();


    static CachedFileDownloader& GetDefaultCache();

    // Pluggable URL fetcher for `FileCacheItem::DownloadURL`. When
    // set, the cache calls this synchronous fetcher in place of
    // the bundled libcurl path. iPad installs an `NSURLSession`-
    // based fetcher at init because Apple's modern network stack
    // negotiates with several vendor-catalog servers (efl-designs,
    // buildalightshow, twinkle-forge, mattosdesigns,
    // ledpixelshow, …) that the bundled libcurl/Secure-Transport
    // combo trips over with SSL connect / 421 errors. Desktop and
    // Linux leave this nullptr → curl handles every fetch.
    //
    // The fetcher must write the response body to `filename` and
    // return `true` on success. Called from the same thread as
    // `GetFile`; iPad's fetcher uses a semaphore to block on the
    // URLSession completion.
    using URLFetcher = std::function<bool(const std::string& url,
                                          const std::string& filename)>;
    static void SetURLFetcher(URLFetcher fetcher);
    static URLFetcher GetURLFetcher();
};

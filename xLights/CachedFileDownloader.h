#ifndef CACHEDFILEDOWNLOADER_H
#define CACHEDFILEDOWNLOADER_H

#include <string>
#include <list>
#include <wx/uri.h>
#include <wx/filename.h>

class wxXmlNode;

typedef enum
{
                CACHETIME_SESSION,
                CACHETIME_DAY,
                CACHETIME_LONG,
                CACHETIME_FOREVER
} CACHEFOR;

class FileCacheItem
{
    wxURI _url;
    CACHEFOR _cacheFor;
    std::string _fileName;

public:
    FileCacheItem(wxXmlNode* n);
    FileCacheItem(wxURI url, CACHEFOR cacheFor);
    void Save(wxFile& f);
    virtual ~FileCacheItem() {}
    void Download();
    bool Exists() const { return wxFile::Exists(_fileName); }
    void Touch() const { if (Exists()) wxFileName(_fileName).Touch(); }
    void Delete() const;
    std::string GetFileName() const { if (Exists()) return _fileName; else return ""; }
    bool operator==(const wxURI& url) const;
    static bool DownloadURL(wxURI url, wxFileName filename);
    std::string DownloadURLToTemp(wxURI url);
    void PurgeIfAged() const;
    bool ShouldSave() { PurgeIfAged();  return Exists() && (_cacheFor == CACHETIME_DAY || _cacheFor == CACHETIME_LONG); }
};

class CachedFileDownloader
{
    std::string _cacheDir;
    std::list<FileCacheItem*> _cacheItems;
    std::string _cacheFile;
    bool _initialised;

    void PurgeAgedItems();
    void SaveCache();
    void LoadCache();
    FileCacheItem* Find(wxURI url);

public:

    CachedFileDownloader(const std::string cacheDir = "");
    virtual ~CachedFileDownloader();
    // erase everything from cache
    void ClearCache();
    void Save() { SaveCache(); }
    // retrieve a file from cache … if not present filename will be “”
    std::string GetFile(wxURI url, CACHEFOR cacheFor);
    int size() const { return _cacheItems.size(); }
};

#endif

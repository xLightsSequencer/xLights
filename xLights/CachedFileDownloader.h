#ifndef CACHEDFILEDOWNLOADER_H
#define CACHEDFILEDOWNLOADER_H

#include <string>
#include <list>
#include <wx/uri.h>
#include <wx/filename.h>

class wxProgressDialog;
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
    FileCacheItem(wxURI url, CACHEFOR cacheFor, wxProgressDialog* prog, int low, int high);
    void Save(wxFile& f);
    virtual ~FileCacheItem() {}
    void Download(wxProgressDialog* prog, int low, int high);
    bool Exists() const { return wxFile::Exists(_fileName); }
    void Touch() const { if (Exists()) wxFileName(_fileName).Touch(); }
    void Delete() const;
    std::string GetFileName() const { if (Exists()) return _fileName; else return ""; }
    bool operator==(const wxURI& url) const;
    static bool DownloadURL(wxURI url, wxFileName filename, wxProgressDialog* prog = nullptr, int low = 0, int high = 100);
    std::string DownloadURLToTemp(wxURI url, wxProgressDialog* prog, int low, int high);
    void PurgeIfAged() const;
    bool ShouldSave() { PurgeIfAged();  return Exists() && (_cacheFor == CACHETIME_DAY || _cacheFor == CACHETIME_LONG); }
};

class CachedFileDownloader
{
    std::string _cacheDir;
    std::list<FileCacheItem*> _cacheItems;
    std::string _cacheFile;
    bool _initialised;
    bool _enabled;

    void PurgeAgedItems();
    void SaveCache();
    void LoadCache();
    bool Initialize();
    FileCacheItem* Find(wxURI url);

public:

    CachedFileDownloader(const std::string cacheDir = "");
    virtual ~CachedFileDownloader();
    // erase everything from cache
    void ClearCache();
    void Save() { SaveCache(); }
    // retrieve a file from cache … if not present filename will be “”
    std::string GetFile(wxURI url, CACHEFOR cacheFor, wxProgressDialog* prog = nullptr, int low = 0, int high = 100);
    int size();
    
    
    static CachedFileDownloader& GetDefaultCache();
};

#endif

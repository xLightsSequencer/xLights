#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <list>
#include <wx/uri.h>
#include <wx/filename.h>
#include <mutex>

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
    FileCacheItem(wxURI url, CACHEFOR cacheFor, const wxString& forceType = "", wxProgressDialog* prog = nullptr, int low = 0, int high = 100, bool keepProgress = false);
    void Save(wxFile& f);
    virtual ~FileCacheItem() {}
    void Download(const wxString& forceType = "", wxProgressDialog* prog = nullptr, int low = 0, int high = 100, bool keepProgress = false);
    bool Exists() const;
    void Touch() const { if (Exists()) wxFileName(_fileName).Touch(); }
    void Delete() const;
    std::string GetFileName() const { if (Exists()) return _fileName; else return ""; }
    bool operator==(const wxURI& url) const;
    static bool DownloadURL(wxURI url, wxFileName filename, wxProgressDialog* prog = nullptr, int low = 0, int high = 100, bool keepProgress =  false);
    std::string DownloadURLToTemp(wxURI url, const wxString& forceType = "", wxProgressDialog* prog = nullptr, int low = 0, int high = 100, bool keepProgress = false);
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
    FileCacheItem* Find(wxURI url);

    CachedFileDownloader();
public:

    virtual ~CachedFileDownloader();
    // erase everything from cache
    void ClearCache();
    void Save() { SaveCache(); }
    // retrieve a file from cache … if not present filename will be “”
    std::string GetFile(wxURI url, CACHEFOR cacheFor, const wxString& forceType = "", wxProgressDialog* prog = nullptr, int low = 0, int high = 100, bool keepProgress = false);
    int size();
    
    
    static CachedFileDownloader& GetDefaultCache();
};

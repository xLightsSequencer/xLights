/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "utils/Curl.h"
#include "CachedFileDownloader.h"
#include "ExternalHooks.h"

#include <wx/wx.h>
#include <wx/protocol/http.h>
#include <wx/sstream.h>
#include "./utils/spdlog_macros.h"
#include <wx/dir.h>
#include <wx/xml/xml.h>
#include <wx/progdlg.h>

#include <memory>

#define LONGCACHEDAYS 5

FileCacheItem::FileCacheItem(wxXmlNode* n)
{
    _url = n->GetAttribute("URI", "");
    _fileName = n->GetAttribute("FileName", "");
    _cacheFor = (CACHEFOR)wxAtoi(n->GetAttribute("CacheFor", "0"));
}

FileCacheItem::FileCacheItem(wxURI url, CACHEFOR cacheFor, const wxString& forceType, wxProgressDialog* prog, int low, int high, bool keepProgress)
{
    _url = url;
    _cacheFor = cacheFor;
    Download(forceType, prog, low, high, keepProgress);
}

void FileCacheItem::Save(wxFile& f)
{
    f.Write("  <item URI=\"" + _url.BuildURI() +
        "\" FileName=\"" + _fileName +
        "\" CacheFor=\"" + wxString::Format("%d", _cacheFor) +
        "\"/>\n");
}

void FileCacheItem::Download(const wxString& forceType, wxProgressDialog* prog, int low, int high, bool keepProgress)
{
    _fileName = DownloadURLToTemp(_url, forceType, prog, low, high, keepProgress);
}

void FileCacheItem::Delete() const
{
    
    if (Exists())
    {
        LOG_DEBUG("Removing cached URL %s.", (const char*)_url.BuildURI().c_str());
        wxRemoveFile(_fileName);
    }
}
bool FileCacheItem::Exists() const {
    return FileExists(_fileName);
}

bool FileCacheItem::operator==(const wxURI& url) const
{
    return _url.BuildURI() == url.BuildURI();
}

// A major constraint of this function is that it does not support https
bool FileCacheItem::DownloadURL(wxURI url, wxFileName filename, wxProgressDialog* prog, int low, int high, bool keepProgress)
{
    

    LOG_DEBUG("Making request to '%s' -> %s.", (const char*)url.BuildURI().c_str(), (const char*)filename.GetFullPath().c_str());
    return Curl::HTTPSGetFile(url.BuildURI().ToStdString(), filename.GetFullPath().ToStdString(), "", "", 600, prog, keepProgress);
}

std::string FileCacheItem::DownloadURLToTemp(wxURI url, const wxString& forceType, wxProgressDialog* prog, int low, int high, bool keepProgress)
{
    wxString type = url.GetPath().AfterLast('.');
    if (type.Contains('/')) type = "";

    if (forceType != "") type = forceType;
    wxString fn = wxFileName::CreateTempFileName("xl");
    wxRemoveFile(fn);
    
    int pidx = fn.Last('.');
    int sidx = fn.Last('/');
    wxString filename;
    if (pidx > sidx) {  //name like /tmp/foo.x1
        filename = fn.BeforeLast('.') + "." + type;
    } else { //name like /var/tmp/org.xlights/x1foo
        filename = fn + "." + type;
    }
    filename.Replace(",", "");
    if (fn.BeforeLast('.') == "") {
        filename = fn + "." + type;
    }

    if (DownloadURL(url, filename, prog, low, high, keepProgress)) {
        return filename.ToStdString();
    }

    return "";
}

void FileCacheItem::PurgeIfAged() const
{
    

    if (!Exists()) return;

    wxDateTime modified = wxFileName(_fileName).GetModificationTime();
    switch(_cacheFor)
    {
    case CACHETIME_SESSION:
        // do nothing
        break;
    case CACHETIME_DAY:
        if (wxDateTime::Now().GetDateOnly() != modified.GetDateOnly())
        {
            LOG_DEBUG("%s purged from file cache because it was not created today.", (const char *)_url.BuildURI().c_str());
            Delete();
        }
        break;
    case CACHETIME_FOREVER:
        // do nothing
        break;
    case CACHETIME_LONG:
        wxTimeSpan age = wxDateTime::Now() - modified;
        if (age.GetDays() > LONGCACHEDAYS)
        {
            LOG_DEBUG("%s purged from file cache because it was more than %d days old: %d.", (const char *)_url.BuildURI().c_str(), LONGCACHEDAYS, age.GetDays());
            Delete();
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

    LOG_DEBUG("Saving File Cache %s.", (const char *)_cacheFile.c_str());

    wxFile f;
    if (f.Create(_cacheFile, true) && f.IsOpened())
    {
        wxString lit("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        f.Write(lit.c_str(), lit.size());
        lit = "<filecache>\n";
        f.Write(lit.c_str(), lit.size());

        int i = 0;
        for (const auto& it : _cacheItems)
        {
            if (it->ShouldSave())
            {
                it->Save(f);
                i++;
            }
        }

        lit = "</filecache>";
        f.Write(lit.c_str(), lit.size());
        f.Close();
        LOG_DEBUG("    File Cache %d items saved.", i);
    }
    else
    {
        LOG_WARN("    Problem saving File Cache.");
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

    LOG_DEBUG("Loading File Cache %s.", (const char *)_cacheFile.c_str());

    if (FileExists(_cacheFile) && wxFileName(_cacheFile).GetSize() > 0)
    {
        wxXmlDocument d;
        d.Load(_cacheFile);
        if (d.IsOk())
        {
            wxXmlNode* root = d.GetRoot();
            if (root != nullptr && root->GetName().Lower() == "filecache")
            {
                LOG_DEBUG("   Cache opened.");
                for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
                {
                    if (n->GetName().Lower() == "item")
                        _cacheItems.push_back(new FileCacheItem(n));
                }
                LOG_DEBUG("   %d items loaded.", _cacheItems.size());
            }
        }
        else
        {
            LOG_WARN("File Cache was invalid.");
        }
    }
    else
    {
        LOG_WARN("File Cache does not exist.");
    }
}

FileCacheItem* CachedFileDownloader::Find(wxURI url)
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
    LOG_WARN("CachedFileDownloaded disabled on Linux.");
    #else
    if (_cacheDir == "" || !wxDirExists(_cacheDir))
    {
        _cacheDir = wxFileName::GetTempDir();
    }

    if (_cacheDir != "")
    {
        _cacheFile = _cacheDir + "/xLightsCache.xml";
        _enabled = true;
    }
    else
    {
        LOG_WARN("CachedFileDownloaded unable to find a temp directory to use. Caching disabled.");
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
    

    LOG_DEBUG("File Cache cleared.");
    std::lock_guard<std::recursive_mutex> lock(_cacheItemsLock);
    for (const auto& it :  _cacheItems)
    {
        it->Delete();
    }
}

void CachedFileDownloader::PurgeAgedItems()
{
    

    LOG_DEBUG("File Cache purging aged items.");
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

std::string CachedFileDownloader::GetFile(wxURI url, CACHEFOR cacheFor, const wxString& forceType, wxProgressDialog* prog, int low, int high, bool keepProgress)
{
    
    std::lock_guard<std::recursive_mutex> lock(_cacheItemsLock);
    if (!Initialize())
    {
        // because we dont have a valid place to save the cache we cant cache anything beyond this session
        cacheFor = CACHEFOR::CACHETIME_SESSION;
    }

    FileCacheItem* fci = Find(url);
    if (fci == nullptr)
    {
        LOG_DEBUG("File Cache downloading file %s.", (const char *)url.BuildURI().c_str());
        fci = new FileCacheItem(url, cacheFor, forceType, prog, low, high, keepProgress);
        _cacheItems.push_back(fci);
    }
    else if (!fci->Exists())
    {
        LOG_DEBUG("File Cache re-downloading file %s.", (const char *)url.BuildURI().c_str());
        fci->Download(forceType, prog, low, high, keepProgress);
    }

    if (fci->GetFileName() == "")
    {
        LOG_DEBUG("File Cache file %s could not be retrieved.", (const char *)url.BuildURI().c_str());
    }

    if (prog != nullptr)
    {
        prog->Update(high);
    }

    return fci->GetFileName();
}

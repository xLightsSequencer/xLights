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
#include "spdlog/spdlog.h"
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
        spdlog::debug("Removing cached URL {}.", _url.BuildURI().ToStdString());
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
    spdlog::debug("Making request to '{}' -> {}.", url.BuildURI().ToStdString(), filename.GetFullPath().ToStdString());
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
            spdlog::debug("{} purged from file cache because it was not created today.", _url.BuildURI().ToStdString());
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
            spdlog::debug("{} purged from file cache because it was more than {} days old: {}.", _url.BuildURI().ToStdString(), LONGCACHEDAYS, age.GetDays());
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

    spdlog::debug("Saving File Cache {}.", _cacheFile);

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

    if (FileExists(_cacheFile) && wxFileName(_cacheFile).GetSize() > 0)
    {
        wxXmlDocument d;
        d.Load(_cacheFile);
        if (d.IsOk())
        {
            wxXmlNode* root = d.GetRoot();
            if (root != nullptr && root->GetName().Lower() == "filecache")
            {
                spdlog::debug("   Cache opened.");
                for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
                {
                    if (n->GetName().Lower() == "item")
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
    spdlog::warn("CachedFileDownloaded disabled on Linux.");
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
        spdlog::debug("File Cache downloading file {}.", url.BuildURI().ToStdString());
        fci = new FileCacheItem(url, cacheFor, forceType, prog, low, high, keepProgress);
        _cacheItems.push_back(fci);
    }
    else if (!fci->Exists())
    {
        spdlog::debug("File Cache re-downloading file {}.", url.BuildURI().ToStdString());
        fci->Download(forceType, prog, low, high, keepProgress);
    }

    if (fci->GetFileName() == "")
    {
        spdlog::debug("File Cache file {} could not be retrieved.", url.BuildURI().ToStdString());
    }

    if (prog != nullptr)
    {
        prog->Update(high);
    }

    return fci->GetFileName();
}

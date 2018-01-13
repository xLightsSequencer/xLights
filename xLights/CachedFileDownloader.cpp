#include "CachedFileDownloader.h"

#include <wx/wx.h>
#include <wx/protocol/http.h>
#include <wx/sstream.h>
#include <log4cpp/Category.hh>
#include <wx/dir.h>
#include <wx/xml/xml.h>

#define LONGCACHEDAYS 5

FileCacheItem::FileCacheItem(wxXmlNode* n)
{
    _url = n->GetAttribute("URI", "");
    _fileName = n->GetAttribute("FileName", "");
    _cacheFor = (CACHEFOR)wxAtoi(n->GetAttribute("CacheFor", "0"));
}

FileCacheItem::FileCacheItem(wxURI url, CACHEFOR cacheFor)
{
    _url = url;
    _cacheFor = cacheFor;
    Download();
}

void FileCacheItem::Save(wxFile& f)
{
    f.Write("  <item URI=\"" + _url.BuildURI() + 
        "\" FileName=\"" + _fileName + 
        "\" CacheFor=\"" + wxString::Format("%d", _cacheFor) +
        "\"/>\n");
}

void FileCacheItem::Download()
{
    _fileName = DownloadURLToTemp(_url);
}

void FileCacheItem::Delete() const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (Exists())
    {
        logger_base.debug("Removing cached URL %s.", (const char*)_url.BuildURI().c_str());
        wxRemoveFile(_fileName);
    }
}

bool FileCacheItem::operator==(const wxURI& url) const
{
    return _url.BuildURI() == url.BuildURI();
}

// A major constraint of this function is that it does not support https
bool FileCacheItem::DownloadURL(wxURI url, wxFileName filename) const
{
    bool ok = true;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (url.GetScheme() == "https")
    {
        logger_base.warn("Unable to retrieve '%s' as xLights cannot access https pages.", (const char *)url.GetPath().c_str());
    }

    wxHTTP http;
    http.SetMethod("GET");
    int port = 80;
    if (wxAtoi(url.GetPort()) != 0)
    {
        port = wxAtoi(url.GetPort());
    }
    bool connected = http.Connect(url.GetServer(), port);

    if (connected)
    {
        logger_base.debug("Making request to '%s'.", (const char *)url.BuildURI().c_str());
        wxStopWatch sw;
        wxInputStream *httpStream = http.GetInputStream(url.GetPath());

        if (http.GetError() == wxPROTO_NOERR)
        {
            logger_base.debug(" Result %d.", http.GetResponse());

            if (http.GetResponse() >= 300 && http.GetResponse() < 400)
            {
                wxDELETE(httpStream);

                wxURI redir = http.GetHeader("LOCATION");
                if (redir.GetPath() == url.GetPath())
                {
                    return false;
                }
                return DownloadURL(redir, filename);
            }

            wxFile f;
            long size = 0;
            if (f.Open(filename.GetFullPath(), wxFile::write))
            {
                wxByte buffer[65536];
                while (!httpStream->Eof() && httpStream->CanRead())
                {
                    httpStream->Read(buffer, sizeof(buffer));
                    f.Write(buffer, httpStream->LastRead());
                    size += httpStream->LastRead();
                }
                f.Close();
                logger_base.debug("   File downloaded %.1f kbytes in %ldms.", (float)size / 1024.0, sw.Time());
            }
            else
            {
                ok = false;
            }
        }
        else
        {
            logger_base.error("Unable to connect to '%s' : %d.", (const char *)url.GetPath().c_str(), http.GetError());
            ok = false;
        }

        wxDELETE(httpStream);
    }
    else
    {
        ok = false;
    }

    return ok;
}

std::string FileCacheItem::DownloadURLToTemp(wxURI url)
{
    wxString type = url.GetPath().AfterLast('.');
    wxString fn = wxFileName::CreateTempFileName("xl");
    wxRemoveFile(fn);
    wxString filename = fn.BeforeLast('.') + "." + type;
    if (fn.BeforeLast('.') == "")
    {
        filename = fn + "." + type;
    }

    if (DownloadURL(url, filename))
    {
        return filename.ToStdString();
    }

    return "";
}

void FileCacheItem::PurgeIfAged() const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

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
            logger_base.debug("%s purged from file cache because it was not created today.", (const char *)_url.BuildURI().c_str());
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
            logger_base.debug("%s purged from file cache because it was more than %d days old: %d.", (const char *)_url.BuildURI().c_str(), LONGCACHEDAYS, age.GetDays());
            Delete();
        }
        break;
    }
}

void CachedFileDownloader::SaveCache()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Saving File Cache %s.", (const char *)_cacheFile.c_str());

    wxFile f;
    if (f.Create(_cacheFile, true) && f.IsOpened())
    {
        wxString lit("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        f.Write(lit, lit.size());
        lit = "<filecache>\n";
        f.Write(lit, lit.size());

        int i = 0;
        for (auto it = _cacheItems.begin(); it != _cacheItems.end(); ++it)
        {
            if ((*it)->ShouldSave())
            {
                (*it)->Save(f);
                i++;
            }
        }

        lit = "</filecache>";
        f.Write(lit, lit.size());
        f.Close();
        logger_base.debug("    File Cache %d items saved.", i);
    }
    else
    {
        logger_base.warn("    Problem saving File Cache.");
    }
}

void CachedFileDownloader::LoadCache()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Loading File Cache %s.", (const char *)_cacheFile.c_str());
    _cacheItems.empty();

    if (wxFile::Exists(_cacheFile) && wxFileName(_cacheFile).GetSize() > 0)
    {
        wxXmlDocument d;
        d.Load(_cacheFile);
        if (d.IsOk())
        {
            wxXmlNode* root = d.GetRoot();
            if (root != nullptr && root->GetName().Lower() == "filecache")
            {
                logger_base.debug("   Cache opened.");
                for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
                {
                    if (n->GetName().Lower() == "item")
                        _cacheItems.push_back(new FileCacheItem(n));
                }
                logger_base.debug("   %d items loaded.", _cacheItems.size());
            }
        }
        else
        {
            logger_base.warn("File Cache was invalid.");
        }
    }
    else
    {
        logger_base.warn("File Cache does not exist.");
    }
}

FileCacheItem* CachedFileDownloader::Find(wxURI url)
{
    for (auto it = _cacheItems.begin(); it != _cacheItems.end(); ++it)
    {
        if (**it == url)
        {
            return *it;
        }
    }

    return nullptr;
}

CachedFileDownloader::CachedFileDownloader(const std::string cacheDir)
{
    _cacheDir = cacheDir;
    if (_cacheDir == "" || !wxDirExists(_cacheDir))
    {
        _cacheDir = wxFileName::GetTempDir();
    }

    _cacheFile = _cacheDir + "/xLightsCache.xml";

    LoadCache();
    PurgeAgedItems();
}

CachedFileDownloader::~CachedFileDownloader()
{
    // Dont save ... this is done when the dialog is exited.
    //SaveCache();
}

void CachedFileDownloader::ClearCache()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("File Cache cleared.");
    for (auto it=  _cacheItems.begin(); it != _cacheItems.end(); ++it)
    {
        (*it)->Delete();
    }
}

void CachedFileDownloader::PurgeAgedItems()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("File Cache purging aged items.");
    for (auto it=  _cacheItems.begin(); it != _cacheItems.end(); ++it)
    {
        (*it)->PurgeIfAged();
    }
}

std::string CachedFileDownloader::GetFile(wxURI url, CACHEFOR cacheFor)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    FileCacheItem* fci = Find(url);
    if (fci == nullptr)
    {
        logger_base.debug("File Cache downloading file %s.", (const char *)url.BuildURI().c_str());
        fci = new FileCacheItem(url, cacheFor);
        _cacheItems.push_back(fci);
    }
    else if (!fci->Exists())
    {
        logger_base.debug("File Cache re-downloading file %s.", (const char *)url.BuildURI().c_str());
        fci->Download();
    }

    if (fci->GetFileName() == "")
    {
        logger_base.debug("File Cache file %s could not be retrieved.", (const char *)url.BuildURI().c_str());
    }

    return fci->GetFileName();
}

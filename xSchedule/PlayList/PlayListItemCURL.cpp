/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/uri.h>
#include <wx/protocol/http.h>
#include <wx/sstream.h>
#include <wx/xml/xml.h>
#include <wx/notebook.h>

#include "PlayListItemCURL.h"
#include "PlayListItemCURLPanel.h"
#include "utils/Curl.h"

#include <log4cpp/Category.hh>

class CurlThread : public wxThread
{
    std::string _url;
    std::string _body;
    std::string _curlType;
    std::string _contenttype;

public:
    CurlThread(const std::string& url, const std::string& body, const std::string& type, const std::string& contenttype) : 
        _url(url), _body(body), _curlType(type), _contenttype(contenttype) { }

    virtual void* Entry() override
    {
        log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        logger_base.debug("PlayListCurl in thread.");

        logger_base.info("Calling URL %s.", (const char*)_url.c_str());

        if (_curlType == "POST")
        {
            auto res = Curl::HTTPSPost(_url, _body, "", "", _contenttype);
            logger_base.info("CURL POST : %s", (const char*)res.c_str());
        }
        else
        {
            auto res = Curl::HTTPSGet(_url);
            logger_base.info("CURL GET: %s", (const char*)res.c_str());
        }

        logger_base.debug("PlayListCurl thread done.");

        return nullptr;
    }
};

PlayListItemCURL::PlayListItemCURL(wxXmlNode* node) : PlayListItem(node)
{
    PlayListItemCURL::Load(node);
}

void PlayListItemCURL::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _url = node->GetAttribute("URL", "");
    _curlType = node->GetAttribute("CurlType", "GET");
    _body = node->GetAttribute("Body", "");
    _contentType = node->GetAttribute("ContentType", "");
}

PlayListItemCURL::PlayListItemCURL() : PlayListItem()
{
    _type = "PLICURL";
}

PlayListItem* PlayListItemCURL::Copy(const bool isClone) const
{
    PlayListItemCURL* res = new PlayListItemCURL();
    res->_url = _url;
    res->_curlType = _curlType;
    res->_body = _body;
    res->_started = false;
    res->_contentType = _contentType;
    PlayListItem::Copy(res, isClone);

    return res;
}

wxXmlNode* PlayListItemCURL::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("URL", _url);
    node->AddAttribute("CurlType", _curlType);
    node->AddAttribute("Body", _body);
    node->AddAttribute("ContentType", _contentType);

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemCURL::GetTitle() const
{
    return "CURL";
}

void PlayListItemCURL::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemCURLPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemCURL::GetNameNoTime() const
{
    if (_name != "") return _name;

	wxURI uri(_url);
	if (uri.GetServer() != "")
	{
		return uri.GetServer().ToStdString();
	}
	
    return "CURL";
}

std::string PlayListItemCURL::GetTooltip()
{
    return GetTagHint();
}

void PlayListItemCURL::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (ms >= _delay && !_started)
    {
        _started = true;

        std::string url = ReplaceTags(_url);
        std::string body = ReplaceTags(_body);

        if (_url == "")
        {
            logger_base.warn("PlayListItemCURL: URL '%s' invalid.", (const char*)url.c_str());
            return;
        }

        CurlThread* thread = new CurlThread(url, body, _curlType, _contentType);
        thread->Run();
        wxMicroSleep(1); // encourage the thread to run
    }
}

void PlayListItemCURL::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}

#include <wx/uri.h>
#include <wx/protocol/http.h>
#include <wx/sstream.h>
#include <wx/xml/xml.h>
#include <wx/notebook.h>

#include "PlayListItemCURL.h"
#include "PlayListItemCURLPanel.h"
#include "../xSMSDaemon/Curl.h"

#include <log4cpp/Category.hh>

PlayListItemCURL::PlayListItemCURL(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _url = "";
    _curltype = "GET";
    _body = "";
    _contentType = "";
    PlayListItemCURL::Load(node);
}

void PlayListItemCURL::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _url = node->GetAttribute("URL", "");
    _curltype = node->GetAttribute("Type", "GET");
    _body = node->GetAttribute("Body", "");
    _contentType = node->GetAttribute("ContentType", "");
}

PlayListItemCURL::PlayListItemCURL() : PlayListItem()
{
    _type = "PLICURL";
    _started = false;
    _url = "";
    _curltype = "GET";
    _body = "";
    _contentType = "";
}

PlayListItem* PlayListItemCURL::Copy() const
{
    PlayListItemCURL* res = new PlayListItemCURL();
    res->_url = _url;
    res->_curltype = _curltype;
    res->_body = _body;
    res->_started = false;
    res->_contentType = _contentType;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemCURL::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("URL", _url);
    node->AddAttribute("Type", _curltype);
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
    if (ms >= _delay && !_started)
    {
        _started = true;

        std::string url = ReplaceTags(_url);
        std::string body = ReplaceTags(_body);

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Calling URL %s.", (const char *)url.c_str());

        if (url == "")
        {
            logger_base.warn("URL '' invalid.", (const char *)url.c_str());
            return;
        }

        if (_curltype == "POST")
        {
            auto res = Curl::HTTPSPost(url, _body, "", "", _contentType);
            logger_base.info("CURL POST : %s", (const char*)res.c_str());
        }
        else
        {
            auto res = Curl::HTTPSGet(url);
            logger_base.info("CURL GET: %s", (const char*)res.c_str());
        }
    }
}

void PlayListItemCURL::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}
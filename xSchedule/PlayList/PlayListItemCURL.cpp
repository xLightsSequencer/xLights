#include "PlayListItemCURL.h"
#include "PlayListItemCURLPanel.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "PlayList.h"
#include "PlayListStep.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <log4cpp/Category.hh>
#include "../RunningSchedule.h"
#include <wx/uri.h>
#include <wx/protocol/http.h>
#include <wx/sstream.h>

PlayListItemCURL::PlayListItemCURL(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _url = "";
    _type = "GET";
    _body = "";
    PlayListItemCURL::Load(node);
}

void PlayListItemCURL::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _url = node->GetAttribute("URL", "");
    _type = node->GetAttribute("Type", "GET");
    _body = node->GetAttribute("Body", "");
}

PlayListItemCURL::PlayListItemCURL() : PlayListItem()
{
    _started = false;
    _url = "";
    _type = "GET";
    _body = "";
}

PlayListItem* PlayListItemCURL::Copy() const
{
    PlayListItemCURL* res = new PlayListItemCURL();
    res->_url = _url;
    res->_type = _type;
    res->_body = _body;
    res->_started = false;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemCURL::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLICURL");

    node->AddAttribute("URL", _url);
    node->AddAttribute("Type", _type);
    node->AddAttribute("Body", _body);

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
    return "Available variables:\n    %RUNNING_PLAYLIST% - current playlist\n    %RUNNING_PLAYLISTSTEP% - step name\n    %RUNNING_PLAYLISTSTEPMS% - Position in current step\n    %RUNNING_PLAYLISTSTEPMSLEFT% - Time left in current step\n    %RUNNING_SCHEDULE% - Name of schedule";
}

std::string PlayListItemCURL::PrepareString(const std::string s)
{
	wxString res(s);

        PlayList* pl = xScheduleFrame::GetScheduleManager()->GetRunningPlayList();
        if (pl != nullptr)
        {
            if (res.Contains("%RUNNING_PLAYLIST%"))
            {
                res.Replace("%RUNNING_PLAYLIST%", pl->GetNameNoTime(), true);
            }
            PlayListStep* pls = pl->GetRunningStep();
            if (pls != nullptr)
            {
                if (res.Contains("%RUNNING_PLAYLISTSTEP%"))
                {
                    res.Replace("%RUNNING_PLAYLISTSTEP%", pls->GetNameNoTime(), true);
                }
                if (res.Contains("%RUNNING_PLAYLISTSTEPMS%"))
                {
                    res.Replace("%RUNNING_PLAYLISTSTEPMS%", wxString::Format(wxT("%i"), pls->GetLengthMS()), true);
                }
                if (res.Contains("%RUNNING_PLAYLISTSTEPMSLEFT%"))
                {
                    res.Replace("%RUNNING_PLAYLISTSTEPMSLEFT%", wxString::Format(wxT("%i"), pls->GetLengthMS() - pls->GetPosition()), true);
                }
            }
        }
        if (res.Contains("%RUNNING_SCHEDULE%"))
        {
            RunningSchedule* rs = xScheduleFrame::GetScheduleManager()->GetRunningSchedule();
            if (rs != nullptr && rs->GetPlayList()->IsRunning())
            {
                res.Replace("%RUNNING_SCHEDULE%", rs->GetSchedule()->GetName(), true);
            }
        }
	
	return res.ToStdString();
}

void PlayListItemCURL::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (ms >= _delay && !_started)
    {
        _started = true;

        std::string url = PrepareString(_url);
        std::string body = PrepareString(_body);

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Calling URL %s.", (const char *)url.c_str());

        if (url == "")
        {
            logger_base.warn("URL '' invalid.", (const char *)url.c_str());
            return;
        }

        wxURI uri(url);

        wxHTTP http;
        http.SetTimeout(10);
        http.SetMethod(_type);
        if (http.Connect(uri.GetServer()))
        {
            if (_type == "POST")
            {
                http.SetPostText("application/x-www-form-urlencoded", _body);
            }
            wxString page = uri.GetPath() + "?" + uri.GetQuery();
            wxInputStream *httpStream = http.GetInputStream(page);
            if (http.GetError() == wxPROTO_NOERR)
            {
                wxString res;
                wxStringOutputStream out_stream(&res);
                httpStream->Read(out_stream);

                logger_base.info("CURL: %s", (const char *)res.c_str());
            }
            else
            {
                logger_base.error("CURL: Error getting page %s from %s.", (const char*)page.c_str(), (const char *)uri.GetServer().c_str());
            }

            if (_type == "POST")
            {
                http.SetPostText("", "");
            }
            wxDELETE(httpStream);
        }
        else
        {
            logger_base.error("CURL: Error connecting to %s.", (const char *)uri.GetServer().c_str());
        }
    }
}

void PlayListItemCURL::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}

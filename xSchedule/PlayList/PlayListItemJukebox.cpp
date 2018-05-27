#include "PlayListItemJukebox.h"
#include "PlayListItemJukeboxPanel.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <log4cpp/Category.hh>
#include <wx/msw/private.h>

PlayListItemJukebox::PlayListItemJukebox(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _jukeboxButton = 1;
    _sequence = "";
    SetEnumState(ENUMJUKEBOX::ENUM_STATE_DONE);
    PlayListItemJukebox::Load(node);
}

void PlayListItemJukebox::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _jukeboxButton = wxAtoi(node->GetAttribute("Button", "1"));
    _sequence = node->GetAttribute("Sequence", "");
}

PlayListItemJukebox::PlayListItemJukebox() : PlayListItem()
{
    _started = false;
    _jukeboxButton = 1;
    _sequence = "";
    SetEnumState(ENUMJUKEBOX::ENUM_STATE_DONE);
}

PlayListItem* PlayListItemJukebox::Copy() const
{
    PlayListItemJukebox* res = new PlayListItemJukebox();
    res->_jukeboxButton = _jukeboxButton;
    res->_sequence = _sequence;
    res->_started = false;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemJukebox::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIJukebox");

    node->AddAttribute("Button", wxString::Format("%d", _jukeboxButton));
    node->AddAttribute("Sequence", _sequence);

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemJukebox::GetTitle() const
{
    return "Play xLights Jukebox Button";
}

void PlayListItemJukebox::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemJukeboxPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemJukebox::GetNameNoTime() const
{
    if (_name != "") return _name;

    return "Play xLights Jukebox Button";
}

#ifdef __WXMSW__
BOOL CALLBACK EnumXLightsWindows(HWND hwnd, LPARAM lparam)
{
    wxString name = ::wxGetWindowText(hwnd);
    int num = wxAtoi(name);
    wxString numstr = wxString::Format("%d", num);
    PlayListItemJukebox* jb = (PlayListItemJukebox*)lparam;

    if (jb->GetEnumState() == ENUM_STATE_DONE)
    {
        return false;
    }

    if (name == "XLIGHTS_NOTEBOOK")
    {
        jb->SetEnumState(ENUMJUKEBOX::ENUM_STATE_NOTEBOOK);
        ::EnumChildWindows(hwnd, EnumXLightsWindows, lparam);
    }
    else if (name.StartsWith("XLIGHTS_SEQUENCER_TAB"))
    {
        auto comp = wxSplit(name, ':');
        if (comp.size() > 1)
        {
            if (comp[1] == jb->GetSequence() || jb->GetSequence() == "")
            {
                jb->SetEnumState(ENUMJUKEBOX::ENUM_STATE_SEQUENCERTAB);
                ::EnumChildWindows(hwnd, EnumXLightsWindows, lparam);
            }
            else
            {
                return false;
            }
        }
    }
    else if (name == "XLIGHTS_JUKEBOX")
    {
        jb->SetEnumState(ENUMJUKEBOX::ENUM_STATE_JUKEBOX);
        ::EnumChildWindows(hwnd, EnumXLightsWindows, lparam);
    }
    else if (name == numstr)
    {
        if (jb->GetEnumState() == ENUMJUKEBOX::ENUM_STATE_JUKEBOX && num == jb->GetButton())
        {
            jb->SetEnumState(ENUMJUKEBOX::ENUM_STATE_DONE);
            ::SetLastError(0);
            ::PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
            ::PostMessage(hwnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));
            return false;
        }
    }

    return true;
}

BOOL CALLBACK EnumTopLevelWindows(HWND hwnd, LPARAM lparam)
{
    wxString name = ::wxGetWindowText(hwnd);

    PlayListItemJukebox* jb = (PlayListItemJukebox*)lparam;
    if (jb->GetEnumState() == ENUM_STATE_DONE)
    {
        return false;
    }

    if (name.StartsWith("xLights "))
    {
        jb->SetEnumState(ENUMJUKEBOX::ENUM_STATE_XLIGHTS);
        ::EnumChildWindows(hwnd, EnumXLightsWindows, lparam);
    }
    return true;
}
#endif

void PlayListItemJukebox::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (ms >= _delay && !_started)
    {
        _started = true;

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Launching xLights Jukebox Button %d.", _jukeboxButton);

#ifdef __WXMSW__
        SetEnumState(ENUMJUKEBOX::ENUM_STATE_BEGIN);
        ::SetLastError(1);
        if (!::EnumWindows(EnumTopLevelWindows, (LPARAM)this) || ::GetLastError() > 0)
        {
            logger_base.error("Launching xLights Jukebox Button %d ... Failed to find jukebox window.", _jukeboxButton);
        }
        SetEnumState(ENUMJUKEBOX::ENUM_STATE_DONE);
#endif
#ifdef __WXOSX_MAC__
        // TODO ... I am not sure how to do this on OSX
        logger_base.error("    Jukebox Button functionality is not supported on OSX.");
#endif
#ifdef __LINUX__
        // TODO ... I am not sure how to do this on Linux
        logger_base.error("    Jukebox Button functionality is not supported on Linux.");
#endif
    }
}

void PlayListItemJukebox::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}
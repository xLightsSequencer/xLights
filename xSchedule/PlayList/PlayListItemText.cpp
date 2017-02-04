#include "PlayListItemText.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemTextPanel.h"
#include <log4cpp/Category.hh>
#include <wx/font.h>
#include "../FSEQFile.h"
#include "../MatrixMapper.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../ScheduleOptions.h"

PlayListItemText::PlayListItemText(wxXmlNode* node) : PlayListItem(node)
{
    _blendMode = APPLYMETHOD::METHOD_OVERWRITE;
    _colour = *wxWHITE;
    _durationMS = 50;
    _font = wxFont(9, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    _format = "";
    _text = "";
    _matrix = "";
    _movement = "None";
    _orientation = "Normal";
    _speed = 10;
    _type = "Normal";
    _x = 0;
    _y = 0;

    PlayListItemText::Load(node);
}

PlayListItemText::~PlayListItemText()
{
}

void PlayListItemText::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _blendMode = (APPLYMETHOD)wxAtoi(node->GetAttribute("ApplyMethod", ""));
    _colour = wxColour(node->GetAttribute("Colour", "WHITE"));
    _durationMS = wxAtol(node->GetAttribute("Duration", "50"));
    _font = wxFont(node->GetAttribute("Font", ""));
    _format = node->GetAttribute("Format", "").ToStdString();
    _text = node->GetAttribute("Text", "").ToStdString();
    _matrix = node->GetAttribute("Matrix", "").ToStdString();
    _movement = node->GetAttribute("Movement", "None").ToStdString();
    _orientation = node->GetAttribute("Orientation", "Normal").ToStdString();
    _type = node->GetAttribute("Type", "Normal").ToStdString();
    _speed = wxAtoi(node->GetAttribute("Speed", "10"));
    _x = wxAtoi(node->GetAttribute("X", "0"));
    _y = wxAtoi(node->GetAttribute("Y", "0"));
}

PlayListItemText::PlayListItemText() : PlayListItem()
{
    _blendMode = APPLYMETHOD::METHOD_OVERWRITE;
    _colour = *wxWHITE;
    _durationMS = 50;
    _font = wxFont(9, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    _format = "";
    _text = "";
    _matrix = "";
    _movement = "None";
    _orientation = "Normal";
    _speed = 10;
    _type = "Normal";
    _x = 0;
    _y = 0;
}

PlayListItem* PlayListItemText::Copy() const
{
    PlayListItemText* res = new PlayListItemText();
    res->_matrix = _matrix;
    res->_movement = _movement;
    res->_orientation = _orientation;
    res->_speed = _speed;
    res->_durationMS = _durationMS;
    res->_type = _type;
    res->_x = _x;
    res->_y = _y;
    res->_blendMode = _blendMode;
    res->_colour = _colour;
    res->_font = _font;
    res->_format = _format;
    res->_text = _text;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemText::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIText");

    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_blendMode));
    node->AddAttribute("Colour", _colour.GetAsString());
    node->AddAttribute("Duration", wxString::Format(wxT("%i"), _durationMS));
    node->AddAttribute("Font", _font.GetNativeFontInfoDesc());
    node->AddAttribute("Format", _format);
    node->AddAttribute("Text", _text);
    node->AddAttribute("Matrix", _matrix);
    node->AddAttribute("Movement", _movement);
    node->AddAttribute("Orientation", _orientation);
    node->AddAttribute("Type", _type);
    node->AddAttribute("Speed", wxString::Format(wxT("%i"), _speed));
    node->AddAttribute("X", wxString::Format(wxT("%i"), _y));
    node->AddAttribute("Y", wxString::Format(wxT("%i"), _x));

    PlayListItem::Save(node);

    return node;
}

void PlayListItemText::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemTextPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemText::GetTitle() const
{
    return "Text";
}

std::string PlayListItemText::GetNameNoTime() const
{
    if (_format != "")
    {
        return _format;
    }
    else if (_text != "")
    {
        return _text;
    }
    else
    {
        return _type;
    }
}

size_t PlayListItemText::GetDurationMS() const
{
    return _delay + _durationMS;
}

void PlayListItemText::Start()
{
    auto m = xScheduleFrame::GetScheduleManager()->GetOptions()->GetMatrices();
    for (auto it = m->begin(); it != m->end(); ++it)
    {
        if (wxString((*it)->GetName()).Lower() == wxString(_matrix).Lower())
        {
            _matrixMapper = *it;
            break;
        }
    }
}

void PlayListItemText::Stop()
{
}

std::string PlayListItemText::GetText(size_t ms)
{
    std::string res;

    wxString working = wxString(_format);

    wxTimeSpan plicountdown(_durationMS - ms);
    wxTimeSpan plicountup(ms);
    wxDateTime now = wxDateTime::Now();
    wxTimeSpan countdown;
    std::string minus = "";
    
    if (_type == "Normal")
    {
        // nothing to do
    }
    else if (_type == "Countdown")
    {
        wxDateTime targetDate;
        targetDate.ParseDateTime(_text);
        countdown = now - targetDate;

        if (now > targetDate)
        {
            minus = "-";
        }
    }

    working.Replace("%TEXT%", _text);

    // countdown to date
    working.Replace("%CDD_MINUS%", minus);
    working.Replace("%CDD_DAYS%", wxString::Format(wxT("%i"), countdown.GetDays()));
    working.Replace("%CDD_HOURS%", wxString::Format(wxT("%i"), countdown.GetHours()));
    working.Replace("%CDD_MINS%", wxString::Format(wxT("%i"), countdown.GetMinutes()));
    working.Replace("%CDD_SECS%", wxString::Format(wxT("%i"), countdown.GetSeconds()));
    working.Replace("%CDD_TSECS%", wxString::Format(wxT("%i"), countdown.GetMilliseconds() / 1000));
    working.Replace("%CDD_MS%", wxString::Format(wxT("%i"), countdown.GetMilliseconds() % 1000));

    // countdown to to item end
    working.Replace("%CD_DAYS%", wxString::Format(wxT("%i"), plicountdown.GetDays()));
    working.Replace("%CD_HOURS%", wxString::Format(wxT("%i"), plicountdown.GetHours()));
    working.Replace("%CD_MINS%", wxString::Format(wxT("%i"), plicountdown.GetMinutes()));
    working.Replace("%CD_SECS%", wxString::Format(wxT("%i"), plicountdown.GetSeconds()));
    working.Replace("%CD_TSECS%", wxString::Format(wxT("%i"), plicountdown.GetMilliseconds() / 1000));
    working.Replace("%CD_MS%", wxString::Format(wxT("%i"), plicountdown.GetMilliseconds() % 1000));

    // countup from item start
    working.Replace("%CU_DAYS%", wxString::Format(wxT("%i"), plicountup.GetDays()));
    working.Replace("%CU_HOURS%", wxString::Format(wxT("%i"), plicountup.GetHours()));
    working.Replace("%CU_MINS%", wxString::Format(wxT("%i"), plicountup.GetMinutes()));
    working.Replace("%CU_SECS%", wxString::Format(wxT("%i"), plicountup.GetSeconds()));
    working.Replace("%CU_TSECS%", wxString::Format(wxT("%i"), plicountup.GetMilliseconds() / 1000));
    working.Replace("%CU_MS%", wxString::Format(wxT("%i"), plicountup.GetMilliseconds() % 1000));

    // current date time
    working.Replace("%DAY%", wxString::Format(wxT("%i"), now.GetDay()));
    working.Replace("%MONTH%", wxString::Format(wxT("%i"), now.GetMonth()));
    working.Replace("%YEAR4%", wxString::Format(wxT("%i"), now.GetYear()));
    working.Replace("%YEAR2%", wxString::Format(wxT("%i"), now.GetYear() % 100));
    working.Replace("%HOUR24%", wxString::Format(wxT("%i"), now.GetHour()));
    working.Replace("%HOUR12%", wxString::Format(wxT("%i"), now.GetHour() % 12));
    working.Replace("%MIN%", wxString::Format(wxT("%i"), now.GetMinute()));
    working.Replace("%SEC%", wxString::Format(wxT("%i"), now.GetSecond()));
    working.Replace("%MS%", wxString::Format(wxT("%i"), now.GetMillisecond()));
    working.Replace("%AMPM%", now.GetHour() > 12 ? "PM" : "AM");

    return res;
}

wxPoint PlayListItemText::GetLocation(size_t ms)
{
    wxPoint res(_x, _y);

    if (_movement == "None")
    {
        
    }
    else if (_movement == "Right to Left")
    {
        res.x -= ms * _speed / 1000;
    }
    else if (_movement == "Left to Right")
    {
        res.x += ms * _speed / 1000;
    }
    else if (_movement == "Bottom to Top")
    {
        res.y += ms * _speed / 1000;
    }
    else if (_movement == "Top to Bottom")
    {
        res.y -= ms * _speed / 1000;
    }

    return res;
}

void PlayListItemText::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_matrixMapper == nullptr) return;

    if (outputframe && ms > _delay)
    {
        size_t effms = ms - _delay;

        wxBitmap bitmap;
        bitmap.Create(_matrixMapper->GetWidth(), _matrixMapper->GetWidth());
        wxMemoryDC dc(bitmap);

        // work out our Text
        std::string text = GetText(effms);

        // work out where to draw it
        wxPoint loc = GetLocation(effms);

        // draw the text into our DC
        dc.SetTextForeground(_colour);
        dc.SetFont(_font);
        wxSize size = dc.GetTextExtent(_text);
        if (_orientation == "Normal")
        {
            dc.DrawText(text, loc);
        }
        else if (_orientation == "Vertical Up")
        {
#pragma todo
        }
        else if (_orientation == "Vertical Down")
        {
#pragma todo
        }
        else if (_orientation == "Rotate Up 90")
        {
            dc.DrawRotatedText(text, loc, 90);
        }
        else if (_orientation == "Rotate Down 90")
        {
            dc.DrawRotatedText(text, loc, -90);
        }

        // write out the bitmap
        wxImage image = bitmap.ConvertToImage();
        for (int x = 0; x < _matrixMapper->GetWidth(); ++x)
        {
            for (int y = 0; y < _matrixMapper->GetHeight(); ++y)
            {
                wxByte* p = buffer + _matrixMapper->Map(x, y);

                SetPixel(p, image.GetRed(x, y), image.GetGreen(x, y), image.GetBlue(x, y), _blendMode);
            }
        }
    }
}

void PlayListItemText::SetPixel(wxByte* p, wxByte r, wxByte g, wxByte b, APPLYMETHOD blendMode)
{
    switch (blendMode)
    {
    case APPLYMETHOD::METHOD_OVERWRITE:
        *p = r;
        *(p + 1) = g;
        *(p + 2) = b;
        break;
    case APPLYMETHOD::METHOD_AVERAGE:
        *p = ((int)*p + (int)r) / 2;
        *(p + 1) = ((int)*(p + 1) + (int)g) / 2;
        *(p + 2) = ((int)*(p + 2) + (int)b) / 2;
        break;
    case APPLYMETHOD::METHOD_MASK:
        if (*p > 0 || *(p + 1) > 0 || *(p + 2))
        {
            *p = 0x00;
            *(p + 1) = 0x00;
            *(p + 2) = 0x00;
        }
        break;
    case APPLYMETHOD::METHOD_UNMASK:
        if (*p == 0 && *(p + 1) == 0 && *(p + 2) == 0)
        {
            *p = 0x00;
            *(p + 1) = 0x00;
            *(p + 2) = 0x00;
        }
        break;
    case APPLYMETHOD::METHOD_MAX:
        *p = std::max(*p, r);
        *(p + 1) = std::max(*(p + 1), g);
        *(p + 2) = std::max(*(p + 2), b);
        break;
    case APPLYMETHOD::METHOD_OVERWRITEIFBLACK:
        if (*p == 0 && *(p + 1) == 0 && *(p + 2) == 0)
        {
            *p = r;
            *(p+1) = g;
            *(p+2) = b;
        }
        break;
    }
}

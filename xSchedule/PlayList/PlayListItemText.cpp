/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemText.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/regex.h>
#include "PlayListItemTextPanel.h"
#include <log4cpp/Category.hh>
#include <wx/font.h>
#include "../MatrixMapper.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../ScheduleOptions.h"
#include "utils/Curl.h"

PlayListItemText::PlayListItemText(wxXmlNode* node) : PlayListItem(node)
{
    _parameter1 = "";
    _matrixMapper = nullptr;
    _font = nullptr;
    _blendMode = APPLYMETHOD::METHOD_OVERWRITEIFBLACK;
    _colour = *wxWHITE;
    _durationMS = 50;
    _font = new wxFont(12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL , wxFONTWEIGHT_NORMAL);
    _format = "";
    _text = "";
    _matrix = "";
    _movement = "None";
    _renderWhenBlank = true;
    _orientation = "Normal";
    _speed = 10;
    _texttype = "Normal";
    _x = 0;
    _y = 0;

    PlayListItemText::Load(node);
}

PlayListItemText::~PlayListItemText()
{
    if (_font != nullptr)
    {
        delete _font;
        _font = nullptr;
    }
}

void PlayListItemText::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _blendMode = (APPLYMETHOD)wxAtoi(node->GetAttribute("ApplyMethod", "1"));
    _colour = wxColour(node->GetAttribute("Colour", "WHITE"));
    _durationMS = wxAtol(node->GetAttribute("Duration", "50"));
    auto font = wxFont(node->GetAttribute("Font", ""));
    if (_font != nullptr)
    {
        delete _font;
        _font = nullptr;
    }
    _font = new wxFont(font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(), font.GetUnderlined(), font.GetFaceName(), font.GetEncoding());
    wxASSERT(_font->IsOk());
    _format = node->GetAttribute("Format", "").ToStdString();
    _text = node->GetAttribute("Text", "").ToStdString();
    _parameter1 = node->GetAttribute("Parameter1", "").ToStdString();
    _matrix = node->GetAttribute("Matrix", "").ToStdString();
    _movement = node->GetAttribute("Movement", "None").ToStdString();
    _renderWhenBlank = (node->GetAttribute("RenderWhenBlank", "True") == "True");
    _orientation = node->GetAttribute("Orientation", "Normal").ToStdString();
    _texttype = node->GetAttribute("Type", "Normal").ToStdString();
    _speed = wxAtoi(node->GetAttribute("Speed", "10"));
    _x = wxAtoi(node->GetAttribute("X", "0"));
    _y = wxAtoi(node->GetAttribute("Y", "0"));
}

void PlayListItemText::SetFont(wxFont* font)
{
    if (_font->GetPointSize() != font->GetPointSize() ||
        _font->GetFamily() != font->GetFamily() ||
        _font->GetStyle() != font->GetStyle() ||
        _font->GetWeight() != font->GetWeight() ||
        _font->GetUnderlined() != font->GetUnderlined() ||
        _font->GetFaceName() != font->GetFaceName() ||
        _font->GetEncoding() != font->GetEncoding()
        )
    {
        if (_font != nullptr)
        {
            delete _font;
            _font = nullptr;
        }
        _font = new wxFont(font->GetPointSize(), font->GetFamily(), font->GetStyle(), font->GetWeight(),
            font->GetUnderlined(), font->GetFaceName(), font->GetEncoding());
        wxASSERT(_font->IsOk());

        _changeCount++; 
    }
}

PlayListItemText::PlayListItemText() : PlayListItem()
{
    _type = "PLIText";
    _font = nullptr;
    _matrixMapper = nullptr;
    _blendMode = APPLYMETHOD::METHOD_OVERWRITEIFBLACK;
    _colour = *wxWHITE;
    _durationMS = 50;
    _font = new wxFont(12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    _format = "";
    _text = "";
    _matrix = "";
    _movement = "None";
    _renderWhenBlank = true;
    _orientation = "Normal";
    _speed = 10;
    _texttype = "Normal";
    _x = 0;
    _y = 0;
    _parameter1 = "";
}

PlayListItem* PlayListItemText::Copy(const bool isClone) const
{
    PlayListItemText* res = new PlayListItemText();
    res->_matrix = _matrix;
    res->_movement = _movement;
    res->_renderWhenBlank = _renderWhenBlank;
    res->_orientation = _orientation;
    res->_speed = _speed;
    res->_durationMS = _durationMS;
    res->_texttype = _texttype;
    res->_x = _x;
    res->_y = _y;
    res->_blendMode = _blendMode;
    res->_colour = _colour;
    res->_parameter1 = _parameter1;
    res->_font = new wxFont(_font->GetPointSize(), _font->GetFamily(), _font->GetStyle(), _font->GetWeight(),
        _font->GetUnderlined(), _font->GetFaceName(), _font->GetEncoding());
    res->_format = _format;
    res->_text = _text;
    PlayListItem::Copy(res, isClone);

    return res;
}

wxXmlNode* PlayListItemText::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_blendMode));
    node->AddAttribute("Colour", _colour.GetAsString());
    node->AddAttribute("Duration", wxString::Format(wxT("%i"), (long)_durationMS));
    node->AddAttribute("Font", _font->GetNativeFontInfoDesc());
    node->AddAttribute("Format", _format);
    node->AddAttribute("Text", _text);
    node->AddAttribute("Matrix", _matrix);
    node->AddAttribute("Movement", _movement);
    if (!_renderWhenBlank)
    {
        node->AddAttribute("RenderWhenBlank", "False");
    }
    node->AddAttribute("Orientation", _orientation);
    node->AddAttribute("Parameter1", _parameter1);
    node->AddAttribute("Type", _texttype);
    node->AddAttribute("Speed", wxString::Format(wxT("%i"), _speed));
    node->AddAttribute("X", wxString::Format(wxT("%i"), _x));
    node->AddAttribute("Y", wxString::Format(wxT("%i"), _y));

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
    if (_name != "")
    {
        return _name;
    }
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
        return _texttype;
    }
}

size_t PlayListItemText::GetDurationMS() const
{
    return _delay + _durationMS;
}

void PlayListItemText::Start(long stepLengthMS)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    PlayListItem::Start(stepLengthMS);

    _lastTwitterTime = 0;
    _lastTwitter = "";
    auto m = xScheduleFrame::GetScheduleManager()->GetOptions()->GetMatrices();
    for (auto it = m->begin(); it != m->end(); ++it)
    {
        if (wxString((*it)->GetName()).Lower() == wxString(_matrix).Lower())
        {
            _matrixMapper = *it;
            logger_base.debug("PlayListItemText %s matrix %s", (const char *)GetNameNoTime().c_str(), _matrixMapper->GetConfigDescription().c_str());
            logger_base.debug("    0,0 = %ld", _matrixMapper->Map(0, 0));
            logger_base.debug("    0,%d = %ld", _matrixMapper->GetHeight() - 1, _matrixMapper->Map(0, _matrixMapper->GetHeight() - 1));
            logger_base.debug("    %d,0 = %ld", _matrixMapper->GetWidth() - 1, _matrixMapper->Map(_matrixMapper->GetWidth()-1, 0));
            logger_base.debug("    %d,%d = %ld", _matrixMapper->GetWidth() - 1, _matrixMapper->GetHeight() - 1, _matrixMapper->Map(_matrixMapper->GetWidth() - 1, _matrixMapper->GetHeight() - 1));
            break;
        }
    }
}

void PlayListItemText::Stop()
{
}

std::string PlayListItemText::GetTooltip(const std::string& type)
{
    std::string tt = GetTagHint();
    
    tt += "\n\n    %TEXT% - text field\n\n";

    tt += "    Current Date/Time\n";
    tt += "        %DAY%, %MONTH%, %YEAR4%, %YEAR2%\n";
    tt += "        %HOUR24%, %HOUR12%, %MIN%, %SEC%, %MS%, %AMPM%\n\n";
    tt += "    Twitter followers - Parameter 1 = Twitter handle (without @\n";
    tt += "        %TWITTER_FOLLOWERS%\n\n";

    if (type == "Countdown")
    {
        tt += "    Time until countdowndate\n";
        tt += "        %CDD_DAYS%, %CDD_HOURS%, %CDD_MINS%, %CDD_SECS%, %CDD_MS%\n";
        tt += "        %CDD_TSECS% -total seconds until countdown date\n\n";
    }
    else if (type == "Countdown Seconds")
    {
        tt += "    Countdown from\n";
        tt += "        %CDS_TSECS% - total seconds until zero\n";
        tt += "        %CDS_MINS%, %CDS_SECS%\n\n";
    }
    else if (type == "File Read")
    {
        tt += "    File Read Data\n";
        tt += "        %FILE_DATA% - Text File Data\n\n";
    }

    tt += "    Time until playlist item end\n";
    tt += "        %CD_DAYS%, %CD_HOURS%, %CD_MINS%, %CD_SECS%, %CD_MS%\n";
    tt += "        %CD_TSECS% -total seconds until playlist item end\n\n";

    tt += "    Time since playlist item start\n";
    tt += "        %CU_DAYS%, %CU_HOURS%, %CU_MINS%, %CU_SECS%, %CU_MS%\n";
    tt += "        %CU_TSECS% -total seconds since playlist item start\n";

    return tt;
}

wxString PlayListItemText::GetText(size_t ms)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // if the text value starts with a backspace the drop the backspace and just return the string ignoring any formatting
    if (_text.size() > 0 && _text[0] == '\b')
    {
        return _text.substr(1);
    }

    wxString working = wxString(_format);

    wxTimeSpan plicountdown = wxTimeSpan::Milliseconds(_durationMS - ms);
    wxTimeSpan plicountup = wxTimeSpan::Milliseconds(ms);
    wxDateTime now = wxDateTime::Now();
    wxTimeSpan countdown;
    int cds = 0;

    if (_texttype == "Normal")
    {
        // nothing to do
    }
    else if (_texttype == "Countdown")
    {
        wxDateTime targetDate;
        targetDate.ParseDateTime(_text);
        if (!targetDate.IsValid())
        {
            targetDate.ParseDate(_text);
        }
        if (!targetDate.IsValid())
        {
            targetDate.ParseTime(_text);
            targetDate.SetDay(wxDateTime::Now().GetDay());
            targetDate.SetMonth(wxDateTime::Now().GetMonth());
            targetDate.SetYear(wxDateTime::Now().GetYear());
            if (targetDate.IsValid() && targetDate < wxDateTime::Now())
            {
                targetDate.Add(wxTimeSpan(24, 0, 0, 0));
            }
        }

        if (targetDate.IsValid())
        {
            countdown = targetDate - now;
        }
        else
        {
            countdown = wxTimeSpan(0);
        }
    }
    else if (_texttype == "Countdown Seconds")
    {
        int to = wxAtoi(_text);

        cds = to - ms / 1000;
    }
    else if (_texttype == "File Read")
    {
        wxFileInputStream input(_text);
        wxTextInputStream text(input);
        wxString fileData;
        while (input.IsOk() && !input.Eof())
        {
            fileData += text.ReadLine();
        }
        fileData = fileData.Trim().Trim(false);
        working.Replace("%FILE_DATA%", fileData);
    }

    working = ReplaceTags(working);
    working.Replace("%TEXT%", _text);

    if (working.Contains("%TWITTER_FOLLOWERS%"))
    {
        wxString followers = "ERROR";

        if (_parameter1 != "")
        {
            // we only update twitter first time ... otherwise we may pause playback
            if (_lastTwitter == "") // || wxGetUTCTime() - _lastTwitterTime > 60)
            {
                auto resp = Curl::HTTPSGet("https://cdn.syndication.twimg.com/widgets/followbutton/info.json?screen_names=" + _parameter1, "", "", 2);
                static wxRegEx f("\"followers_count\":([0-9]*)", wxRE_ADVANCED);
                if (f.Matches(resp))
                {
                    followers = f.GetMatch(resp, 1);
                }
                else
                {
                    logger_base.warn("Getting twitter followers failed: %s", (const char*)resp.c_str());
                }
                _lastTwitter = followers;
                _lastTwitterTime = wxGetUTCTime();
            }
            else {
                followers = _lastTwitter;
            }
        }
        else
        {
            logger_base.warn("No twitter account specified.");
        }
        working.Replace("%TWITTER_FOLLOWERS%", followers);
    }

    // countdown to date
    working.Replace("%CDD_DAYS%", wxString::Format(wxT("%i"), countdown.GetDays()));
    working.Replace("%CDD_HOURS%", wxString::Format(wxT("%02i"), abs(countdown.GetHours()) % 24));
    working.Replace("%CDD_MINS%", wxString::Format(wxT("%02i"), abs(countdown.GetMinutes()) % 60));
    working.Replace("%CDD_SECS%", wxString::Format(wxT("%02i"), abs((countdown.GetSeconds() % 60).ToLong())));
    working.Replace("%CDD_TSECS%", wxString::Format(wxT("%i"), abs((countdown.GetMilliseconds() / 1000).ToLong())));
    working.Replace("%CDD_MS%", wxString::Format(wxT("%03i"), abs((countdown.GetMilliseconds() % 1000).ToLong())));

    // countdown seconds
    working.Replace("%CDS_TSECS%", wxString::Format(wxT("%i"), cds));
    working.Replace("%CDS_MINS%", wxString::Format(wxT("%i"), cds / 60));
    working.Replace("%CDS_SECS%", wxString::Format(wxT("%i"), cds % 60));

    // countdown to to item end
    working.Replace("%CD_DAYS%", wxString::Format(wxT("%i"), plicountdown.GetDays()));
    working.Replace("%CD_HOURS%", wxString::Format(wxT("%02i"), plicountdown.GetHours() % 24));
    working.Replace("%CD_MINS%", wxString::Format(wxT("%02i"), plicountdown.GetMinutes() % 60));
    working.Replace("%CD_SECS%", wxString::Format(wxT("%02i"), (plicountdown.GetSeconds() % 60).ToLong()));
    working.Replace("%CD_TSECS%", wxString::Format(wxT("%i"), (plicountdown.GetMilliseconds() / 1000).ToLong()));
    working.Replace("%CD_MS%", wxString::Format(wxT("%03i"), (plicountdown.GetMilliseconds() % 1000).ToLong()));

    // countup from item start
    working.Replace("%CU_DAYS%", wxString::Format(wxT("%i"), plicountup.GetDays()));
    working.Replace("%CU_HOURS%", wxString::Format(wxT("%02i"), plicountup.GetHours() % 24));
    working.Replace("%CU_MINS%", wxString::Format(wxT("%02i"), plicountup.GetMinutes() % 60));
    working.Replace("%CU_SECS%", wxString::Format(wxT("%02i"), (plicountup.GetSeconds() % 60).ToLong()));
    working.Replace("%CU_TSECS%", wxString::Format(wxT("%i"), (plicountup.GetMilliseconds() / 1000).ToLong()));
    working.Replace("%CU_MS%", wxString::Format(wxT("%03i"), (plicountup.GetMilliseconds() % 1000).ToLong()));

    // current date time
    working.Replace("%DAY%", wxString::Format(wxT("%02i"), now.GetDay()));
    working.Replace("%MONTH%", wxString::Format(wxT("%02i"), now.GetMonth()+1));
    working.Replace("%YEAR4%", wxString::Format(wxT("%04i"), now.GetYear()));
    working.Replace("%YEAR2%", wxString::Format(wxT("%02i"), now.GetYear() % 100));
    working.Replace("%HOUR24%", wxString::Format(wxT("%02i"), now.GetHour()));
    working.Replace("%HOUR12%", wxString::Format(wxT("%02i"), now.GetHour() % 12));
    working.Replace("%MIN%", wxString::Format(wxT("%02i"), now.GetMinute()));
    working.Replace("%SEC%", wxString::Format(wxT("%02i"), now.GetSecond()));
    working.Replace("%MS%", wxString::Format(wxT("%03i"), now.GetMillisecond()));
    working.Replace("%AMPM%", now.GetHour() > 12 ? "PM" : "AM");

    working.Replace("\\\\", "!xyzzy!");
    working.Replace("\\t", "\t");
    working.Replace("\\n", "\n");
    working.Replace("!xyzzy!", "\\");

    return working;
}

wxPoint PlayListItemText::GetLocation(size_t ms, wxSize size)
{
    // x = 0 is LHS
    // y = 0 is TOP

    wxPoint res(_x + _matrixMapper->GetWidth() / 2 - size.x / 2, _matrixMapper->GetHeight() + (-1 * _y) - 1 - _matrixMapper->GetHeight() / 2 - size.y/2);

    if (_movement == "None")
    {

    }
    else if (_movement == "Right to Left")
    {
        int maxx = std::max(_matrixMapper->GetWidth(), size.x);
        res.x = -1 * (ms * _speed / 1000) + 2;

        if (maxx > 0)
        {
            res.x = res.x % maxx;
        }
    }
    else if (_movement == "Left to Right")
    {
        res.x = -size.x - 2 + _matrixMapper->GetWidth() + ms * _speed / 1000;

        if (size.x > 0)
        {
            while (res.x > _matrixMapper->GetWidth())
            {
                res.x -= size.x;
            }
        }
    }
    else if (_movement == "Bottom to Top")
    {
        int toty = _matrixMapper->GetHeight() + size.y;
        res.y = size.y;
        res.y -= ms * _speed / 1000;
        if (toty > 0)
        {
            while (res.y < -toty + size.y)
            {
                res.y += toty;
            }
        }
    }
    else if (_movement == "Top to Bottom")
    {
        int toty = _matrixMapper->GetHeight() + 2 * size.y;
        res.y = -toty + size.y;
        res.y += ms * _speed / 1000;
        if (toty > 0)
        {
            while (res.y > size.y)
            {
                res.y -= toty;
            }
        }
    }

    return res;
}

void PlayListItemText::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    // static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_matrixMapper == nullptr) return;

    if (outputframe && ms > _delay)
    {
        size_t effms = ms - _delay;

        // work out our Text
        wxString text = GetText(effms);

        // if newly set then wemake effms act as if it was zero now
        if (_newlySet) {
            _newlySet = false;
            _msAdj = effms;
            effms = 0;
        }
        else
        {
            effms -= _msAdj;
        }

        if (text == "" && !_renderWhenBlank)
        {
            // dont do anything
        }
        else
        {
            wxBitmap bitmap(_matrixMapper->GetWidth(), _matrixMapper->GetHeight());
            wxMemoryDC dc(bitmap);

            // draw the text into our DC
            dc.SetTextForeground(_colour);
            dc.SetFont(*_font);
            wxSize sz = dc.GetTextExtent(text);

            if (_orientation == "Normal")
            {
                // work out where to draw it
                wxPoint loc = GetLocation(effms, sz);
                dc.DrawText(text, loc);
            }
            else if (_orientation == "Vertical Up" || _orientation == "Vertical Down")
            {
                // work out where to draw it
                wxSize sz1(sz.GetHeight(), dc.GetCharHeight() * text.size());
                wxPoint loc = GetLocation(effms, sz1);
                int y = loc.y;
                for (auto c = text.begin(); c != text.end(); ++c)
                {
                    wxSize cSize = dc.GetTextExtent(*c);
                    int xoffset = cSize.GetWidth() / 2;
                    dc.DrawText(wxString(*c), loc.x - xoffset, y);
                    if (_orientation == "Vertical Down")
                    {
                        y += dc.GetCharHeight();
                    }
                    else
                    {
                        y -= dc.GetCharHeight();
                    }
                }
            }
            else if (_orientation == "Rotate Up 90")
            {
                wxSize sz1(sz.GetHeight(), sz.GetWidth());
                wxPoint loc = GetLocation(effms, sz1);
                dc.DrawRotatedText(text, loc, 90);
            }
            else if (_orientation == "Rotate Down 90")
            {
                wxSize sz1(sz.GetHeight(), sz.GetWidth());
                wxPoint loc = GetLocation(effms, sz1);
                dc.DrawRotatedText(text, loc, -90);
            }

            // write out the bitmap
            dc.SelectObject(wxNullBitmap);
            wxImage image = bitmap.ConvertToImage();
            for (int x = 0; x < _matrixMapper->GetWidth(); ++x)
            {
                for (int y = 0; y < _matrixMapper->GetHeight(); ++y)
                {
                    size_t bl = _matrixMapper->Map(x, _matrixMapper->GetHeight() - y - 1) - 1;

                    if (bl < size)
                    {
                        uint8_t* p = buffer + bl;

                        SetPixel(p, image.GetRed(x, y), image.GetGreen(x, y), image.GetBlue(x, y), _blendMode);
                    }
                    else
                    {
                        wxASSERT(false);
                    }
                }
            }
        }
    }
}

void PlayListItemText::SetPixel(uint8_t* p, uint8_t r, uint8_t g, uint8_t b, APPLYMETHOD blendMode)
{
    uint8_t rgb[3];
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;

    Blend(p, 3, rgb, 3, blendMode);
}

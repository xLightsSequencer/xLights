#include "PlayListItemScreenMap.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemScreenMapPanel.h"
#include <log4cpp/Category.hh>
#include "../FSEQFile.h"
#include "../MatrixMapper.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../ScheduleOptions.h"

PlayListItemScreenMap::PlayListItemScreenMap(wxXmlNode* node) : PlayListItem(node)
{
    _matrixMapper = nullptr;
    _blendMode = APPLYMETHOD::METHOD_OVERWRITEIFBLACK;
    _durationMS = 50;
    _matrix = "";
    _x = 0;
    _y = 0;
	_width = 0;
	_height = 0;
    _rescale = false;
    _quality = "High";

    PlayListItemScreenMap::Load(node);
}

PlayListItemScreenMap::~PlayListItemScreenMap()
{
}

void PlayListItemScreenMap::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _blendMode = (APPLYMETHOD)wxAtoi(node->GetAttribute("ApplyMethod", "1"));
    _durationMS = wxAtol(node->GetAttribute("Duration", "50"));
    _matrix = node->GetAttribute("Matrix", "").ToStdString();
    _x = wxAtoi(node->GetAttribute("X", "0"));
    _y = wxAtoi(node->GetAttribute("Y", "0"));
    _width = wxAtoi(node->GetAttribute("Width", "0"));
    _height = wxAtoi(node->GetAttribute("Height", "0"));
    _rescale = node->GetAttribute("Rescale", "False") == "True";
    _quality = node->GetAttribute("Quality", "High").ToStdString();
}

PlayListItemScreenMap::PlayListItemScreenMap() : PlayListItem()
{
    _matrixMapper = nullptr;
    _blendMode = APPLYMETHOD::METHOD_OVERWRITEIFBLACK;
    _durationMS = 50;
    _matrix = "";
    _x = 0;
    _y = 0;
	_width = 0;
	_height = 0;
    _rescale = false;
    _quality = "High";
}

PlayListItem* PlayListItemScreenMap::Copy() const
{
    PlayListItemScreenMap* res = new PlayListItemScreenMap();
    res->_matrix = _matrix;
    res->_durationMS = _durationMS;
    res->_x = _x;
    res->_y = _y;
    res->_width = _width;
    res->_height = _height;
    res->_blendMode = _blendMode;
    res->_quality = _quality;
    res->_rescale = _rescale;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemScreenMap::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIScreenMap");

    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_blendMode));
    node->AddAttribute("Duration", wxString::Format(wxT("%i"), (long)_durationMS));
    node->AddAttribute("Matrix", _matrix);
    node->AddAttribute("X", wxString::Format(wxT("%i"), _x));
    node->AddAttribute("Y", wxString::Format(wxT("%i"), _y));
    node->AddAttribute("Width", wxString::Format(wxT("%i"), _width));
    node->AddAttribute("Height", wxString::Format(wxT("%i"), _height));
    node->AddAttribute("Quality", _quality);
    if (_rescale)
    {
        node->AddAttribute("Rescale", "True");
    }

    PlayListItem::Save(node);

    return node;
}

void PlayListItemScreenMap::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemScreenMapPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemScreenMap::GetTitle() const
{
    return "Screen Map";
}

std::string PlayListItemScreenMap::GetNameNoTime() const
{
    if (_name != "")
    {
        return _name;
    }
    else
    {
        return GetTitle();
    }
}

size_t PlayListItemScreenMap::GetDurationMS() const
{
    return _delay + _durationMS;
}

void PlayListItemScreenMap::Start(long stepLengthMS)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    PlayListItem::Start(stepLengthMS);

    auto m = xScheduleFrame::GetScheduleManager()->GetOptions()->GetMatrices();
    for (auto it = m->begin(); it != m->end(); ++it)
    {
        if (wxString((*it)->GetName()).Lower() == wxString(_matrix).Lower())
        {
            _matrixMapper = *it;
            logger_base.debug("PlayListItemScreenMap %s matrix %s", (const char *)GetNameNoTime().c_str(), _matrixMapper->GetConfigDescription().c_str());
            logger_base.debug("    0,0 = %ld", _matrixMapper->Map(0, 0));
            logger_base.debug("    0,%d = %ld", _matrixMapper->GetHeight() - 1, _matrixMapper->Map(0, _matrixMapper->GetHeight() - 1));
            logger_base.debug("    %d,0 = %ld", _matrixMapper->GetWidth() - 1, _matrixMapper->Map(_matrixMapper->GetWidth()-1, 0));
            logger_base.debug("    %d,%d = %ld", _matrixMapper->GetWidth() - 1, _matrixMapper->GetHeight() - 1, _matrixMapper->Map(_matrixMapper->GetWidth() - 1, _matrixMapper->GetHeight() - 1));
            break;
        }
    }
}

void PlayListItemScreenMap::Stop()
{
}

void PlayListItemScreenMap::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    // static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_matrixMapper == nullptr) return;

    if (outputframe && ms > _delay)
    {
        //Create a DC for the whole screen area
        wxScreenDC dcScreen;

        wxSize sourceSize;
        if (_rescale)
        {
            sourceSize = wxSize(_width, _height);
        }
        else
        {
            sourceSize = wxSize(_matrixMapper->GetWidth(), _matrixMapper->GetHeight());
        }

        wxBitmap sourceBitmap(sourceSize.GetWidth(), sourceSize.GetHeight());
        wxMemoryDC dc(sourceBitmap);
        dc.SelectObject(sourceBitmap);

        dc.Blit(0, //Copy to this X coordinate
            0, //Copy to this Y coordinate
            sourceSize.GetWidth(), //Copy this width
            sourceSize.GetHeight(), //Copy this height
            &dcScreen, //From where do we copy?
            _x, //What's the X offset in the original DC?
            _y  //What's the Y offset in the original DC?
        );
        dc.SelectObject(wxNullBitmap);

        wxImage image;
        if (_rescale)
        {
            int swsQuality = -1;
            wxImageResizeQuality quality = VirtualMatrix::EncodeScalingQuality(_quality, swsQuality);
            image = sourceBitmap.ConvertToImage().Rescale(_matrixMapper->GetWidth(), _matrixMapper->GetHeight(), quality);
        }
        else
        {
            image = sourceBitmap.ConvertToImage();
        }

        for (int x = 0; x < _matrixMapper->GetWidth(); ++x)
        {
            for (int y = 0; y < _matrixMapper->GetHeight(); ++y)
            {
                size_t bl = _matrixMapper->Map(x, _matrixMapper->GetHeight() - y - 1) - 1;

                if (bl < size)
                {
                    wxByte* p = buffer + bl;

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

void PlayListItemScreenMap::SetPixel(wxByte* p, wxByte r, wxByte g, wxByte b, APPLYMETHOD blendMode)
{
    wxByte rgb[3];
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;

    Blend(p, 3, rgb, 3, blendMode);
}

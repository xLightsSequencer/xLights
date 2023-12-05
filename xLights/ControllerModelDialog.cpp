/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(ControllerModelDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wx/printdlg.h"
#include <wx/artprov.h>
#include <wx/config.h>
#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/dcscreen.h>
#include <wx/dnd.h>
#include <wx/file.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/numdlg.h>
#include <wx/position.h>

#include "ControllerModelDialog.h"
#include "Pixels.h"
#include "UtilFunctions.h"
#include "xLightsMain.h"
#include "controllers/ControllerCaps.h"
#include "controllers/ControllerUploadData.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "outputs/Controller.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/Output.h"

#include <log4cpp/Category.hh>
#include <cmath>

//(*IdInit(ControllerModelDialog)
const long ControllerModelDialog::ID_PANEL1 = wxNewId();
const long ControllerModelDialog::ID_SCROLLBAR1 = wxNewId();
const long ControllerModelDialog::ID_SCROLLBAR2 = wxNewId();
const long ControllerModelDialog::ID_STATICTEXT1 = wxNewId();
const long ControllerModelDialog::ID_SLIDER_BOX_SCALE = wxNewId();
const long ControllerModelDialog::ID_STATICTEXT2 = wxNewId();
const long ControllerModelDialog::ID_SLIDER_FONT_SCALE = wxNewId();
const long ControllerModelDialog::ID_TEXTCTRL1 = wxNewId();
const long ControllerModelDialog::ID_PANEL3 = wxNewId();
const long ControllerModelDialog::ID_CHECKBOX1 = wxNewId();
const long ControllerModelDialog::ID_PANEL2 = wxNewId();
const long ControllerModelDialog::ID_SCROLLBAR3 = wxNewId();
const long ControllerModelDialog::ID_PANEL4 = wxNewId();
const long ControllerModelDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

const long ControllerModelDialog::CONTROLLERModel_PRINT = wxNewId();
const long ControllerModelDialog::CONTROLLERModel_SAVE_CSV = wxNewId();
const long ControllerModelDialog::CONTROLLER_DMXCHANNEL = wxNewId();
const long ControllerModelDialog::CONTROLLER_CASCADEDOWNPORT = wxNewId();
const long ControllerModelDialog::CONTROLLER_DMXCHANNELCHAIN = wxNewId();
const long ControllerModelDialog::CONTROLLER_PROTOCOL = wxNewId();
const long ControllerModelDialog::CONTROLLER_REMOVEPORTMODELS = wxNewId();
const long ControllerModelDialog::CONTROLLER_MOVEMODELSTOPORT = wxNewId();
const long ControllerModelDialog::CONTROLLER_BRIGHTNESS = wxNewId();
const long ControllerModelDialog::CONTROLLER_BRIGHTNESSCLEAR = wxNewId();
const long ControllerModelDialog::CONTROLLER_REMOVEALLMODELS = wxNewId();
const long ControllerModelDialog::CONTROLLER_SMARTREMOTETYPE = wxNewId();
const long ControllerModelDialog::CONTROLLER_REMOVESMARTREMOTE = wxNewId();
const long ControllerModelDialog::CONTROLLER_SETSMARTREMOTE = wxNewId();
const long ControllerModelDialog::CONTROLLER_MODEL_STRINGS = wxNewId();
const long ControllerModelDialog::CONTROLLER_STARTNULLS = wxNewId();
const long ControllerModelDialog::CONTROLLER_ENDNULLS = wxNewId();
const long ControllerModelDialog::CONTROLLER_COLORORDER = wxNewId();
const long ControllerModelDialog::CONTROLLER_GROUPCOUNT = wxNewId();

BEGIN_EVENT_TABLE(ControllerModelDialog, wxDialog)
//(*EventTable(ControllerModelDialog)
//*)
END_EVENT_TABLE()

#pragma region Drawing Constants
#define TOP_BOTTOM_MARGIN ScaleWithSystemDPI(GetSystemContentScaleFactor(), 10 * _scale)
#define VERTICAL_GAP ScaleWithSystemDPI(GetSystemContentScaleFactor(), 5 * _scale)
#define VERTICAL_SIZE ScaleWithSystemDPI(GetSystemContentScaleFactor(), 40 * _scale)
#define LEFT_RIGHT_MARGIN TOP_BOTTOM_MARGIN
#define HORIZONTAL_GAP VERTICAL_GAP
#define HORIZONTAL_SIZE ScaleWithSystemDPI(GetSystemContentScaleFactor(), 120 * _scale)
#define CORNER_ROUNDING ScaleWithSystemDPI(GetSystemContentScaleFactor(), 5 * _scale)
#define MODEL_ICON_SIZE ScaleWithSystemDPI(GetSystemContentScaleFactor(), 16 * _scale)
// we add this gap to the left and right of the first and last models on a SR
#define SRX_GAP ScaleWithSystemDPI(GetSystemContentScaleFactor(), 11 * _scale)
// we add this gap to the top of ports which have a SR
#define SRY_GAP ScaleWithSystemDPI(GetSystemContentScaleFactor(), 5 * _scale)
// we add this gap to the bottom of ports which have a SR ... this is where the label goes
#define SRYLABEL_SIZE (4 * SRY_GAP)
#define FIRST_MODEL_GAP_MULTIPLIER 5
#pragma endregion

#pragma region Colours
wxColour __lightBlue(135, 206, 255, wxALPHA_OPAQUE);
wxColour __lightRed(255, 133, 133, wxALPHA_OPAQUE);
wxColour __lightYellow(255, 255, 133, wxALPHA_OPAQUE);
wxColour __lightPink(255, 133, 255, wxALPHA_OPAQUE);
wxColour __lightAqua(128, 255, 255, wxALPHA_OPAQUE);
wxColour __lightGrey(225, 225, 225);
wxColour __grey(128, 128, 128, wxALPHA_OPAQUE);
wxColour __darkGrey(100, 100, 100);
wxColour __charcoal(30, 30, 30);
wxColour __lightGreen(153, 255, 145, wxALPHA_OPAQUE);
wxColour __darkGreen(6, 60, 0, wxALPHA_OPAQUE);
wxColour __darkBlue(0, 0, 60, wxALPHA_OPAQUE);
wxColour __lightPurple(184, 150, 255, wxALPHA_OPAQUE);
wxColour __darkPurple(49, 0, 74, wxALPHA_OPAQUE);
wxColour __darkRed(60, 0, 0, wxALPHA_OPAQUE);
wxColour __darkPink(60, 0, 60, wxALPHA_OPAQUE);
wxColour __darkAqua(60, 90, 90, wxALPHA_OPAQUE);
wxColour __darkYellow(60, 60, 0, wxALPHA_OPAQUE);
wxColour __lightOrange(255, 201, 150, wxALPHA_OPAQUE);
wxColour __darkOrange(150, 54, 3, wxALPHA_OPAQUE);
wxColour __magenta(255, 0, 255, wxALPHA_OPAQUE);
wxColour __lightShadow(211, 211, 211);
wxColour __darkShadow(30, 24, 24);
wxColour __textForeground;
wxBrush __invalidBrush;
wxBrush __dropTargetBrush;
wxBrush __over40Brush;
wxPen __dropTargetPen;
wxPen __pixelPortOutlinePen;
wxPen __serialPortOutlinePen;
wxPen __vmPortOutlinePen;
wxPen __lpmPortOutlinePen;
wxPen __modelOutlinePen;
wxPen __modelOutlineLastDroppedPen;
wxBrush __modelSRNoneBrush;
wxBrush __modelSRABrush;
wxBrush __modelSRBBrush;
wxBrush __modelSRCBrush;
wxBrush __modelSRDBrush;
wxBrush __modelSREBrush;
wxBrush __modelSRFBrush;
wxBrush __modelShadowBrush;
wxColour __modelSRAText;
wxColour __modelSRBText;
wxColour __modelSRCText;
wxColour __modelSRDText;
wxColour __modelSREText;
wxColour __modelSRFText;
wxPen __backgroundPen;
wxBrush __backgroundBrush;

// only in ControllerModelDialog.......
double GetSystemContentScaleFactor()
{
#ifdef __WXOSX__
    return xlOSGetMainScreenContentScaleFactor();
#else
    return double(wxScreenDC().GetPPI().y) / 96.0;
#endif
}

double ScaleWithSystemDPI(double scalingFactor, double val)
{
#ifdef __WXOSX__
    // OSX handles all the scaling itself
    return val;
#else
    return val * scalingFactor;
#endif
}

void SetColours(bool printing)
{
    __invalidBrush.SetColour(__lightRed);
    __serialPortOutlinePen.SetColour(*wxGREEN);
    __over40Brush.SetColour(__lightOrange);
    __modelOutlinePen.SetColour(__grey);
    __modelOutlineLastDroppedPen.SetColour(__magenta);
    __modelOutlineLastDroppedPen.SetWidth(3);

    if (!printing && IsDarkMode()) {
        __modelOutlinePen.SetColour(__grey);
        __dropTargetBrush.SetColour(*wxBLUE);
        __dropTargetPen.SetColour(*wxBLUE);
        __over40Brush.SetColour(__darkOrange);
        __pixelPortOutlinePen.SetColour(*wxCYAN);
        __vmPortOutlinePen.SetColour(__lightBlue);
        __lpmPortOutlinePen.SetColour(__lightBlue);

        __modelSRNoneBrush.SetColour(__darkGrey);
        __modelSRABrush.SetColour(__darkGreen);
        __modelSRBBrush.SetColour(__darkPurple);
        __modelSRCBrush.SetColour(__darkOrange);
        __modelSRDBrush.SetColour(__darkAqua);
        __modelSREBrush.SetColour(__darkPink);
        __modelSRFBrush.SetColour(__darkYellow);
        __modelShadowBrush.SetColour(__darkShadow);
        __backgroundPen.SetColour(__charcoal);
        __backgroundBrush.SetColour(__charcoal);
        __modelSRAText = __lightGreen;
        __modelSRBText = __lightPurple;
        __modelSRCText = __lightOrange;
        __modelSRDText = __lightAqua;
        __modelSREText = __lightRed;
        __modelSRFText = __lightYellow;
        __textForeground = __lightGrey;
    } else {
        __modelOutlinePen.SetColour(__grey);
        __dropTargetBrush.SetColour(__lightBlue);
        __dropTargetPen.SetColour(__lightBlue);
        __pixelPortOutlinePen.SetColour(*wxRED);
        __modelSRNoneBrush.SetColour(*wxWHITE);
        __vmPortOutlinePen.SetColour(*wxBLUE);
        __lpmPortOutlinePen.SetColour(*wxBLUE);
        __modelSRABrush.SetColour(__lightGreen);
        __modelSRBBrush.SetColour(__lightPurple);
        __modelSRCBrush.SetColour(__lightOrange);
        __modelSRDBrush.SetColour(__lightAqua);
        __modelSREBrush.SetColour(__lightPink);
        __modelSRFBrush.SetColour(__lightYellow);
        __modelShadowBrush.SetColour(__lightShadow);
        __backgroundPen.SetColour(*wxWHITE);
        __backgroundBrush.SetColour(*wxWHITE);
        __modelSRAText = *wxBLACK;
        __modelSRBText = *wxBLACK;
        __modelSRCText = *wxBLACK;
        __modelSRDText = *wxBLACK;
        __modelSREText = *wxBLACK;
        __modelSRFText = *wxBLACK;
        __textForeground = *wxBLACK;
    }
}
#pragma endregion

#pragma region Object Classes

class BaseCMObject
{
public:
    const static int STYLE_PIXELS = 1;
    const static int STYLE_STRINGS = 2;
    const static int STYLE_CHANNELS = 4;
    enum class HITLOCATION { NONE,
                             LEFT,
                             RIGHT,
                             ALL };

protected:
    bool _selectable = false;
    bool _selected = false;
    bool _dragging = false;
    double _scale = 1;
    wxPoint _location = wxPoint(0, 0);
    wxSize _size = wxSize(ScaleWithSystemDPI(GetSystemContentScaleFactor(), _scale * 100),
                          ScaleWithSystemDPI(GetSystemContentScaleFactor(), _scale * 40));
    UDController* _cud = nullptr;
    ControllerCaps* _caps = nullptr;
    HITLOCATION _over = HITLOCATION::NONE;
    int _style = 0;
    bool _invalid = false;

public:
    BaseCMObject(UDController* cud, ControllerCaps* caps, wxPoint location, wxSize size, int style, double scale)
    {
        _style = style;
        _cud = cud;
        _caps = caps;
        _location = location;
        _size = size;
        _scale = scale;
        _invalid = false;
    }
    void SetLocationY(int y)
    {
        _location = wxPoint(_location.x, y);
    }
    int GetDisplayWidth() const
    {
        return _size.GetWidth();
    }
    virtual ~BaseCMObject()
    {}
    void SetInvalid(bool invalid)
    {
        _invalid = invalid;
    }
    HITLOCATION GetOver() const
    {
        return _over;
    }
    void SetOver(HITLOCATION hit)
    {
        _over = hit;
    }
    void SetHeight(int height)
    {
        _size = wxSize(_size.x, height);
    }
    HITLOCATION HitTest(wxPoint mouse)
    {
        if (mouse.x >= _location.x &&
            mouse.x <= _location.x + (_size.x / 2) &&
            mouse.y >= _location.y &&
            mouse.y <= _location.y + _size.y)
            return HITLOCATION::LEFT;
        if (mouse.x >= _location.x + (_size.x / 2) &&
            mouse.x <= _location.x + _size.x &&
            mouse.y >= _location.y &&
            mouse.y <= _location.y + _size.y)
            return HITLOCATION::RIGHT;
        return HITLOCATION::NONE;
    }
    virtual bool HitYTest(wxPoint mouse)
    {
        return (mouse.y >= _location.y &&
                mouse.y <= _location.y + _size.y);
    }
    bool BelowHitYTest(wxPoint mouse)
    {
        return (mouse.y < _location.y);
    }
    virtual void Draw(wxDC& dc, int portMargin, wxPoint mouse, wxPoint adjustedMouse, wxSize offset, float scale, bool printing, bool border, Model* lastDropped) = 0;
    virtual void DrawIcon(wxDC& dc, int portMargin, wxPoint mouse, wxPoint adjustedMouse, wxSize offset, float scale)
    {
    }

    void UpdateCUD(UDController* cud)
    {
        _cud = cud;
    }

    virtual void AddRightClickMenu(wxMenu& mnu, ControllerModelDialog* cmd)
    {}

    virtual bool HandlePopup(wxWindow* parent, wxCommandEvent& event, int id)
    {
        return false;
    }

    virtual std::string GetType() const = 0;
    wxRect GetRect() const
    {
        return wxRect(_location, _size);
    }
    void DrawTextLimited(wxDC& dc, const std::string& text, const wxPoint& pt, const wxSize& size)
    {
        dc.SetClippingRegion(pt, size);
        dc.DrawText(text, pt);
        dc.DestroyClippingRegion();
    }
    void SetSROffset(double x, double y)
    {
        _location = wxPoint(_location.x + x, _location.y + y);
    }
};

class PortCMObject : public BaseCMObject
{
public:
    enum class PORTTYPE { PIXEL,
                          SERIAL,
                          VIRTUAL_MATRIX,
                          PANEL_MATRIX };

protected:
    int _port = -1;
    PORTTYPE _type = PORTTYPE::PIXEL;

public:
    PortCMObject(PORTTYPE type, int port, UDController* cud, ControllerCaps* caps, wxPoint location, wxSize size, int style, bool invalid, double scale) :
        BaseCMObject(cud, caps, location, size, style, scale)
    {
        _invalid = invalid;
        _port = port;
        _type = type;
    }
    UDControllerPort* GetUDPort() const
    {
        if (_type == PORTTYPE::PIXEL) {
            return _cud->GetControllerPixelPort(_port);
        } else if (_type == PORTTYPE::SERIAL) {
            return _cud->GetControllerSerialPort(_port);
        } else if (_type == PORTTYPE::VIRTUAL_MATRIX) {
            return _cud->GetControllerVirtualMatrixPort(_port);
        } else if (_type == PORTTYPE::PANEL_MATRIX) {
            return _cud->GetControllerLEDPanelMatrixPort(_port);
        }
        return nullptr;
    }

    int GetBasePort() const
    {
        return ((_port - 1) / 4) * 4 + 1;
    }

    int GetMaxPortChannels() const
    {
        if (_caps == nullptr)
            return 9999999;

        if (_type == PORTTYPE::PIXEL) {
            return _caps->GetMaxPixelPortChannels();
        } else if (_type == PORTTYPE::SERIAL) {
            return _caps->GetMaxSerialPortChannels();
        }
        return 9999999;
    }

    int GetSmartRemoteCount() const
    {
        int count = 0;
        if (_type == PORTTYPE::PIXEL) {
            int basePort = ((_port - 1) / 4) * 4 + 1;
            for (int i = 0; i < 4; ++i) {
                count = std::max(count, _cud->GetControllerPixelPort(basePort + i)->GetSmartRemoteCount());
            }
        }
        return count;
    }

    virtual bool HitYTest(wxPoint mouse) override
    {
        int totaly = 0;

        bool mergeConsecutiveStrings = !_caps || _caps->MergeConsecutiveVirtualStrings();
        int vsc = GetUDPort()->GetRealVirtualStringCount();

        if (GetUDPort()->IsSmartRemotePort()) {
            int src = GetSmartRemoteCount();
            int empty = GetUDPort()->CountEmptySmartRemotesBefore(src + 1);
            if (mergeConsecutiveStrings) {
                totaly = src * (VERTICAL_SIZE + SRY_GAP + SRYLABEL_SIZE);

                totaly += (vsc - (src - empty)) * (VERTICAL_SIZE + VERTICAL_GAP);

            } else {
                totaly = VERTICAL_SIZE + SRY_GAP + SRYLABEL_SIZE;
            }
        } else {
            totaly = VERTICAL_SIZE;

            if (mergeConsecutiveStrings) {
                if (vsc > 1) {
                    totaly += (vsc - 1) * (VERTICAL_SIZE + VERTICAL_GAP);
                }
            }
        }

        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("HITY port %d %d-%d %s", _port, _location.y, _location.y + totaly, (mouse.y >= _location.y && mouse.y <= _location.y + totaly) ? "HIT" : "");

        return (mouse.y >= _location.y &&
                mouse.y <= _location.y + totaly);
    }

    int GetVirtualStringFromMouse(wxPoint mouse)
    {
        int vs = -1;
        int y = _location.y;

        while (mouse.y >= y) {
            y += VERTICAL_SIZE + VERTICAL_GAP;
            vs++;
        }

        return vs;
    }

    int GetModelCount(int sr) const
    {
        return GetUDPort()->GetModelCount(sr);
    }
    int GetModelCount() const
    {
        return GetUDPort()->GetModels().size();
    }
    Model* GetFirstModel() const
    {
        if (GetModelCount() == 0)
            return nullptr;
        return GetUDPort()->GetModels().front()->GetModel();
    }
    PORTTYPE GetPortType() const
    {
        return _type;
    }
    int GetPort() const
    {
        return _port;
    }
    virtual std::string GetType() const override
    {
        return "PORT";
    }
    virtual void Draw(wxDC& dc, int portMargin, wxPoint mouse, wxPoint adjustedMouse, wxSize offset, float scale, bool printing, bool border, Model* lastDropped) override
    {
        auto origBrush = dc.GetBrush();
        auto origPen = dc.GetPen();
        auto origText = dc.GetTextForeground();

        wxSize sz = _size;
        sz = sz.Scale(scale, scale);
        dc.SetTextForeground(__textForeground);

        UDControllerPort* p = GetUDPort();

        auto location = _location * scale;

        if (_style & STYLE_PIXELS) {
            // max is defined in pixels but the actual max is pixels * 3 channels. Using 4 channel pixels does not give you a larger number of channels
            if (_caps != nullptr && _caps->GetMaxPixelsAt40FPS() != -1) {
                // check for too many pixels for 40 FPS
                int max = _caps->GetMaxPixelsAt40FPS();
                if (p->AtLeastOneModelIsUsingSmartRemote())
                {
                    max = _caps->GetMaxPixelsAt40FPS_SR();
                }
                if (p->Channels() > max * 3) {
                    // draw indicator that this port is not able to achieve 40 FPS
                    dc.SetPen(*wxTRANSPARENT_PEN);
                    dc.SetBrush(__over40Brush);
                    dc.DrawRoundedRectangle(location + offset + wxSize(sz.x * 4 / 5 - 2, sz.y / 2 - 2), wxSize(sz.x / 5, sz.y / 2), CORNER_ROUNDING * scale);
                }
            }
        }

        if (!border) {
            dc.SetPen(*wxTRANSPARENT_PEN);
        } else if (_type == PORTTYPE::PIXEL) {
            dc.SetPen(__pixelPortOutlinePen);
        } else if (_type == PORTTYPE::VIRTUAL_MATRIX) {
            dc.SetPen(__vmPortOutlinePen);
        } else if (_type == PORTTYPE::PANEL_MATRIX) {
            dc.SetPen(__lpmPortOutlinePen);
        } else {
            dc.SetPen(__serialPortOutlinePen);
        }

        if (_over != HITLOCATION::NONE && !printing) {
            dc.SetBrush(__dropTargetBrush);
        } else if (_invalid) {
            dc.SetBrush(__invalidBrush);
        } else {
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
        }

        dc.DrawRoundedRectangle(location + offset, sz, CORNER_ROUNDING * scale);

        wxPoint pt = location + offset + wxSize(2, 2);
        if (_type == PORTTYPE::PIXEL) {
            DrawTextLimited(dc, wxString::Format("Pixel Port %d", _port), pt, sz - wxSize(4, 4));
        } else if (_type == PORTTYPE::VIRTUAL_MATRIX) {
            DrawTextLimited(dc, wxString::Format("Virtual Matrix %d", _port), pt, sz - wxSize(4, 4));
        } else if (_type == PORTTYPE::PANEL_MATRIX) {
            DrawTextLimited(dc, wxString::Format("LED Panel %d", _port), pt, sz - wxSize(4, 4));
        } else {
            DrawTextLimited(dc, wxString::Format("Serial Port %d", _port), pt, sz - wxSize(4, 4));
        }
        pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);

        if (_style & STYLE_PIXELS) {
            std::string label = "Pixels: ";
            wxSize szp = dc.GetTextExtent(label);
            DrawTextLimited(dc, label, pt, sz - wxSize(4, 4));
            pt += wxSize(szp.GetWidth(), 0);
            if (p->Channels() > GetMaxPortChannels()) {
                dc.SetTextForeground(*wxRED);
            }
            DrawTextLimited(dc, wxString::Format("%d", INTROUNDUPDIV(p->Channels(), GetChannelsPerPixel(p->GetProtocol()))), pt, sz - wxSize(pt.x + 2, 4));
            dc.SetTextForeground(__textForeground);
            pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
        }
        if (_style & STYLE_CHANNELS) {
            std::string label = "Channels: ";
            wxSize szp = dc.GetTextExtent(label);
            DrawTextLimited(dc, label, pt, sz - wxSize(4, 4));
            pt += wxSize(szp.GetWidth(), 0);
            if (p->Channels() > GetMaxPortChannels()) {
                dc.SetTextForeground(*wxRED);
            }
            DrawTextLimited(dc, wxString::Format("%d", p->Channels()), pt, sz - wxSize(pt.x + 2, 4));
            dc.SetTextForeground(__textForeground);
            pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
        }

        dc.SetBrush(origBrush);
        dc.SetPen(origPen);
        dc.SetTextForeground(origText);
    }
    virtual void AddRightClickMenu(wxMenu& mnu, ControllerModelDialog* cmd) override
    {
        if (_caps != nullptr) {
            if (_type == PORTTYPE::PIXEL && _caps->GetPixelProtocols().size() == 0)
                return;
            if (_type == PORTTYPE::SERIAL && _caps->GetSerialProtocols().size() == 0)
                return;
            if (_type == PORTTYPE::VIRTUAL_MATRIX && !_caps->SupportsVirtualMatrix())
                return;
            if (_type == PORTTYPE::PANEL_MATRIX && !_caps->SupportsLEDPanelMatrix())
                return;
        }
        mnu.AppendSeparator();
        if (_type == PORTTYPE::PIXEL || _type == PORTTYPE::SERIAL) {
            mnu.Append(ControllerModelDialog::CONTROLLER_PROTOCOL, "Set Protocol");
        }
        if (_caps != nullptr && (_type == PORTTYPE::PIXEL) && _caps->SupportsSmartRemotes() && (_caps->GetSmartRemoteTypes().size() > 1)) {
            mnu.Append(ControllerModelDialog::CONTROLLER_SMARTREMOTETYPE, "Set Smart Remote Type");
        }
        if (_caps != nullptr && (_type == PORTTYPE::PIXEL) && _caps->SupportsSmartRemotes()) {
            mnu.Append(ControllerModelDialog::CONTROLLER_SETSMARTREMOTE, "Set Smart Remote ID and Increment");
        }
        if (_caps != nullptr && (_type == PORTTYPE::PIXEL) && _caps->SupportsSmartRemotes() && GetSmartRemoteCount() > 0) {
            mnu.Append(ControllerModelDialog::CONTROLLER_REMOVESMARTREMOTE, "Remove Smart Remote");
        }
        mnu.Append(ControllerModelDialog::CONTROLLER_REMOVEPORTMODELS, "Remove All Models From Port");
        if (_caps != nullptr && ((_type == PORTTYPE::PIXEL && _caps->GetMaxPixelPort() > 1) || (_type == PORTTYPE::SERIAL && _caps->GetMaxSerialPort() > 1))) {
            mnu.Append(ControllerModelDialog::CONTROLLER_MOVEMODELSTOPORT, "Move All Models To Port");
        }
    }

    virtual bool HandlePopup(wxWindow* parent, wxCommandEvent& event, int id) override
    {
        if (id == ControllerModelDialog::CONTROLLER_REMOVEPORTMODELS) {
            UDControllerPort* port = nullptr;
            if (_type == PORTTYPE::PIXEL) {
                port = _cud->GetControllerPixelPort(GetPort());
            } else if (_type == PORTTYPE::VIRTUAL_MATRIX) {
                port = _cud->GetControllerVirtualMatrixPort(GetPort());
            } else if (_type == PORTTYPE::PANEL_MATRIX) {
                port = _cud->GetControllerLEDPanelMatrixPort(GetPort());
            } else {
                port = _cud->GetControllerSerialPort(GetPort());
            }
            if (port) {
                for (const auto& it : port->GetModels()) {
                    it->GetModel()->SetModelChain("");
                    if (it->GetModel()->GetControllerName() != "") {
                        it->GetModel()->SetControllerName(NO_CONTROLLER);
                    }
                    it->GetModel()->SetControllerPort(0);
                }
            }
            return true;
        } else if (id == ControllerModelDialog::CONTROLLER_MOVEMODELSTOPORT) {
            int max = _caps->GetMaxPixelPort();
            if (_type == PORTTYPE::SERIAL)
                max = _caps->GetMaxSerialPort();

            wxNumberEntryDialog dlg(parent, "Enter The Port To Move The Models To", "Port", "Port", GetPort(), 1, max);
            if (dlg.ShowModal() == wxID_OK) {
                if (_type == PORTTYPE::SERIAL) {
                    auto from = _cud->GetControllerSerialPort(GetPort());
                    auto to = _cud->GetControllerSerialPort(dlg.GetValue());
                    if (from->GetPort() != to->GetPort()) {
                        auto last = to->GetLastModel();
                        bool first = true;
                        for (const auto& it : from->GetModels()) {
                            if (first) {
                                it->GetModel()->SetModelChain("");
                                first = false;
                                if (last != nullptr) {
                                    it->GetModel()->SetModelChain(last->GetName());
                                }
                            }
                            it->GetModel()->SetControllerPort(to->GetPort());
                        }
                    }
                } else {
                    auto from = _cud->GetControllerPixelPort(GetPort());
                    auto to = _cud->GetControllerPixelPort(dlg.GetValue());
                    if (from->GetPort() != to->GetPort()) {
                        auto last = to->GetLastModel();
                        bool first = true;
                        for (const auto& it : from->GetModels()) {
                            if (first) {
                                it->GetModel()->SetModelChain("");
                                first = false;
                                if (last != nullptr) {
                                    it->GetModel()->SetModelChain(last->GetName());
                                }
                            }
                            it->GetModel()->SetControllerPort(to->GetPort());
                        }
                    }
                }
            }
            return true;
        } else if (id == ControllerModelDialog::CONTROLLER_PROTOCOL) {
            wxArrayString choices;
            if (_caps != nullptr) {
                if (_type == PORTTYPE::PIXEL) {
                    for (const auto& it : GetAllPixelTypes(_caps->GetPixelProtocols(), false, true, false)) {
                        choices.push_back(it);
                    }
                } else if (_type == PORTTYPE::SERIAL) {
                    for (const auto& it : GetAllSerialTypes(_caps->GetSerialProtocols())) {
                        choices.push_back(it);
                    }
                }
            } else {
                for (const auto& it : GetAllPixelTypes(true, true)) {
                    if (_type == PORTTYPE::PIXEL && IsPixelProtocol(it)) {
                        choices.push_back(it);
                    } else if (_type == PORTTYPE::SERIAL && !IsPixelProtocol(it)) {
                        choices.push_back(it);
                    }
                }
            }

            wxSingleChoiceDialog dlg(parent, "Port Protocol", "Protocol", choices);
            if (dlg.ShowModal() == wxID_OK) {
                if (_caps != nullptr && !_caps->SupportsMultipleSimultaneousOutputProtocols()) {
                    // We have to apply the protocol to all ports
                    if (_type == PORTTYPE::PIXEL) {
                        for (int i = 1; i <= _cud->GetMaxPixelPort(); i++) {
                            for (const auto& it : _cud->GetControllerPixelPort(i)->GetModels()) {
                                it->GetModel()->SetControllerProtocol(choices[dlg.GetSelection()]);
                            }
                        }
                    } else if (_type == PORTTYPE::SERIAL) {
                        for (int i = 1; i <= _cud->GetMaxSerialPort(); i++) {
                            for (const auto& it : _cud->GetControllerSerialPort(i)->GetModels()) {
                                it->GetModel()->SetControllerProtocol(choices[dlg.GetSelection()]);
                            }
                        }
                    }
                } else {
                    // We only need to apply the protocol to this port
                    for (const auto& it : GetUDPort()->GetModels()) {
                        it->GetModel()->SetControllerProtocol(choices[dlg.GetSelection()]);
                    }
                }
            }
            return true;
        } else if (id == ControllerModelDialog::CONTROLLER_SMARTREMOTETYPE) {
            if (_caps != nullptr) {
                wxArrayString choices;
                auto types = _caps->GetSmartRemoteTypes();
                std::transform(types.begin(), types.end(), std::back_inserter(choices),
                               [](auto const& str) { return str; });
                int selection{ -1 };
                if (GetUDPort()->GetModels().size() != 0 && nullptr != GetUDPort()->GetModels().front()) {
                    selection = choices.Index(GetUDPort()->GetModels().front()->GetSmartRemoteType());
                }
                wxSingleChoiceDialog dlg(parent, "Port Smart Remote Type", "Smart Remote Type", choices);
                if (selection >= 0 && selection < choices.size()) {
                    dlg.SetSelection(selection);
                }
                if (dlg.ShowModal() == wxID_OK) {
                    for (const auto& it : GetUDPort()->GetModels()) {
                        it->GetModel()->SetSmartRemoteType(choices[dlg.GetSelection()]);
                    }
                }
                return true;
            }
        } else if (id == ControllerModelDialog::CONTROLLER_REMOVESMARTREMOTE) {
            int basePort = GetBasePort();
            for (uint8_t p = 0; p < 4; ++p) {
                _cud->GetControllerPixelPort(basePort + p)->ClearSmartRemoteOnAllModels();
            }
            return true;
        } else if (id == ControllerModelDialog::CONTROLLER_SETSMARTREMOTE) {
            wxArrayString choices;
            int sr_count = _caps->GetSmartRemoteCount();
            if (_caps->GetVendor() == "HinksPix") {
                for (int i = 0; i < sr_count; i++) {
                    choices.Add(wxString::Format("%d", i));
                }
            } else {
                for (int i = 0; i < sr_count; i++) {
                    choices.Add(wxString(char(65 + i)));
                }
            }
            int selection{ -1 };
            if (GetUDPort()->GetModels().size() != 0 && nullptr != GetUDPort()->GetModels().front()) {
                selection = GetUDPort()->GetModels().front()->GetSmartRemote() - 1;//0=none, 1=A,Falcon/FPP, 1=0,HinksPix
            }
            wxSingleChoiceDialog dlg(parent, "Port Smart Remote ID", "Smart Remote ID", choices);
            if (selection >= 0 && selection < choices.size()) {
                dlg.SetSelection(selection);
            }
            if (dlg.ShowModal() == wxID_OK) {
                int startId = dlg.GetSelection();
                std::string lastName;
                for (const auto& it : GetUDPort()->GetModels()) {
                    if (lastName == it->GetModel()->Name()) {//skip multistring models sequentuial ports
                        continue;
                    }
                    it->GetModel()->SetSmartRemote(startId + 1);
                    int max_cas = std::min(it->GetModel()->GetSRMaxCascade(), (int)std::ceil(it->GetModel()->GetNumStrings() / 4.0));
                    max_cas = std::max(max_cas, 1);
                    startId += max_cas;
                    if (startId >= sr_count) {
                        startId = (sr_count - 1);
                    }
                    lastName = it->GetModel()->Name();
                }
            }
            return true;
        }
        return false;
    }
};

class SRCMObject : public BaseCMObject
{
    int _smartRemote = 0;
    std::string _name;
    UDControllerPort* _port = nullptr;
    UDController* _controller = nullptr;

public:
    SRCMObject(UDControllerPort* pp, int smartRemote, UDController* cud, ControllerCaps* caps, wxPoint location, wxSize size, int style, double scale, bool useNumbersAsName) :
        BaseCMObject(cud, caps, location, size, style, scale)
    {
        _port = pp;
        _controller = cud;

        if (useNumbersAsName) {
            _name = wxString::Format("%d", smartRemote - 1);
        } else {
            _name = wxString::Format("%c", 64 + smartRemote);
        }
        _smartRemote = smartRemote;
    }

    int GetBasePort() const
    {
        return ((_port->GetPort() - 1) / 4) * 4 + 1;
    }

    bool IsSameRemote(BaseCMObject* other) const
    {
        bool same = false;
        SRCMObject* sr = dynamic_cast<SRCMObject*>(other);

        if (sr != nullptr) {
            if (GetBasePort() == sr->GetBasePort() && GetSmartRemote() == sr->GetSmartRemote()) {
                same = true;
            }
        }

        return same;
    }

    void SetAllModelsToReceiver(int fromPort, int toPort, int fromSR, int toSR)
    {
        auto fromPP = _controller->GetControllerPixelPort(fromPort);
        auto toPP = _controller->GetControllerPixelPort(toPort);
        if (fromPP != nullptr && toPP != nullptr) {
            if (toPP->GetModelCount(toSR) == 0) {
                // the easy case ... no models on that sr
                for (auto& it : fromPP->GetModels()) {
                    if (it->GetSmartRemote() == fromSR && it->IsFirstModelString()) {
                        it->GetModel()->SetControllerPort(toPort);
                        it->GetModel()->SetSmartRemote(toSR);
                    }
                }
            } else {
                // the not so easy case ... find the last model on the port
                std::string lastName;
                for (auto& it : toPP->GetModels()) {
                    if (it->GetSmartRemote() == toSR) {
                        lastName = it->GetName();
                    }
                }
                for (auto& it : fromPP->GetModels()) {
                    if (it->GetSmartRemote() == fromSR && it->IsFirstModelString()) {
                        it->GetModel()->SetControllerPort(toPort);
                        if (lastName != "") {
                            it->GetModel()->SetModelChain(lastName);
                            lastName = "";
                        }
                        it->GetModel()->SetSmartRemote(toSR);
                    }
                }
            }
        }
    }

    void SetAllModelsToReceiver(UDControllerPort* port, int from, int to)
    {
        int por = ((_port->GetPort() - 1) / 4) * 4 + 1;
        for (uint8_t p = por; p < por + 4; ++p) {
            auto pp = _controller->GetControllerPixelPort(p);
            if (pp != nullptr) {
                if (pp->GetModelCount(to) == 0) {
                    // the easy case ... no models on that sr
                    for (auto& it : pp->GetModels()) {
                        if (it->GetSmartRemote() == from)
                            it->GetModel()->SetSmartRemote(to);
                    }
                } else {
                    // the not so easy case ... find the last model on the port
                    std::string lastName;
                    for (auto& it : pp->GetModels()) {
                        if (it->GetSmartRemote() == to) {
                            lastName = it->GetName();
                        }
                    }
                    for (auto& it : pp->GetModels()) {
                        if (it->GetSmartRemote() == from) {
                            if (lastName != "") {
                                it->GetModel()->SetModelChain(lastName);
                                lastName = "";
                            }
                            it->GetModel()->SetSmartRemote(to);
                        }
                    }
                }
            }
        }
    }

    UDControllerPort* GetPort() const
    {
        return _port;
    }

    std::string GetProtocol() const
    {
        if (_port != nullptr)
            return _port->GetProtocol();
        return "";
    }

    int GetSmartRemote() const
    {
        return _smartRemote;
    }

    std::string GetName() const
    {
        return _name;
    }

    int GetVirtualStringCount() const
    {
        int count = 0;
        if (_port != nullptr) {
            for (const auto& it : _port->GetVirtualStrings()) {
                if (!it->_isDummy && it->_smartRemote == _smartRemote)
                    ++count;
            }
        }
        return count;
    }

    int Channels() const
    {
        int count = 0;
        if (_port != nullptr) {
            for (const auto& it : _port->GetVirtualStrings()) {
                if (it->_smartRemote == _smartRemote)
                    count += it->Channels();
            }
        }
        return count;
    }

    float GetAmps(int defaultBrightness) const
    {
        if (_port != nullptr) {
            return _port->GetAmps(defaultBrightness, _smartRemote);
        }
        return 0.0f;
    }

    int StartChannel() const
    {
        if (_port != nullptr) {
            auto m = _port->GetFirstModel(_smartRemote);
            if (m != nullptr)
                return m->GetStartChannel();
        }
        return 0;
    }

    int GetUniverse() const
    {
        if (_port != nullptr) {
            auto m = _port->GetFirstModel(_smartRemote);
            if (m != nullptr)
                return m->GetUniverse();
        }
        return 0;
    }

    int GetUniverseStartChannel() const
    {
        if (_port != nullptr) {
            auto m = _port->GetFirstModel(_smartRemote);
            if (m != nullptr)
                return m->GetUniverseStartChannel();
        }
        return 0;
    }

    virtual std::string GetType() const override
    {
        return "SR";
    }
    virtual void Draw(wxDC& dc, int portMargin, wxPoint mouse, wxPoint adjustedMouse, wxSize offset, float scale, bool printing, bool border, Model* lastDropped) override
    {
        auto origBrush = dc.GetBrush();
        auto origPen = dc.GetPen();
        auto origText = dc.GetTextForeground();
        dc.SetTextForeground(__textForeground);

        if (!border) {
            dc.SetPen(*wxTRANSPARENT_PEN);
        } else {
            dc.SetPen(__modelOutlinePen);
        }

        switch (_smartRemote) {
        case 0:
            dc.SetBrush(__modelSRNoneBrush);
            break;
        case 1:
        case 7:
        case 13:
            dc.SetBrush(__modelSRABrush);
            dc.SetTextForeground(__modelSRAText);
            break;
        case 2:
        case 8:
        case 14:
            dc.SetBrush(__modelSRBBrush);
            dc.SetTextForeground(__modelSRBText);
            break;
        case 3:
        case 9:
        case 15:
            dc.SetBrush(__modelSRCBrush);
            dc.SetTextForeground(__modelSRCText);
            break;
        case 4:
        case 10:
        case 16:
            dc.SetBrush(__modelSRDBrush);
            dc.SetTextForeground(__modelSRDText);
            break;
        case 5:
        case 11:
        case 17:
            dc.SetBrush(__modelSREBrush);
            dc.SetTextForeground(__modelSREText);
            break;
        case 6:
        case 12:
        case 18:
            dc.SetBrush(__modelSRFBrush);
            dc.SetTextForeground(__modelSRFText);
            break;
        default:
            dc.SetBrush(__invalidBrush);
            break;
        }

        if (_invalid) {
            dc.SetBrush(__invalidBrush);
        }

        auto location = _location * scale;

        if (_over == HITLOCATION::ALL && !printing) {
            dc.SetBrush(__dropTargetBrush);
        }

        wxSize sz = _size;
        sz = sz.Scale(scale, scale);
        dc.DrawRectangle(location + offset, sz);

        if (_over != HITLOCATION::NONE && _over != HITLOCATION::ALL && !printing) {
            // need to draw partial drop over
            if (_over == HITLOCATION::LEFT) {
                dc.SetPen(__dropTargetPen);
                dc.SetBrush(__dropTargetBrush);
                wxSize ssz = wxSize((sz.x - 2) / 2, sz.y - 2);
                dc.DrawRectangle(location + offset + wxSize(1, 1), ssz);
            } else if (_over == HITLOCATION::RIGHT) {
                dc.SetPen(__dropTargetPen);
                dc.SetBrush(__dropTargetBrush);
                wxSize ssz = wxSize((sz.x - 2) / 2, sz.y - 2);
                dc.DrawRectangle(location + offset + wxSize((sz.x / 2), 1), ssz);
            }
        }

        wxPoint pt = location + wxSize(2, (_size.y - 2 - 3 * SRY_GAP) * scale) + offset;
        DrawTextLimited(dc, _name, pt, sz - wxSize(4, 4));
        // pt += wxSize(0, ((VERTICAL_SIZE + SRY_GAP + SRYLABEL_SIZE) * scale) / 2);

        dc.SetBrush(origBrush);
        dc.SetPen(origPen);
        dc.SetTextForeground(origText);
    }

    virtual void DrawIcon(wxDC& dc, int portMargin, wxPoint mouse, wxPoint adjustedMouse, wxSize offset, float scale) override
    {
        auto origBrush = dc.GetBrush();
        auto origPen = dc.GetPen();
        auto origText = dc.GetTextForeground();
        dc.SetTextForeground(__textForeground);

        dc.SetPen(*wxTRANSPARENT_PEN);

        switch (_smartRemote) {
        case 0:
            dc.SetBrush(__modelSRNoneBrush);
            break;
        case 1:
        case 7:
        case 13:
            dc.SetBrush(__modelSRABrush);
            dc.SetTextForeground(__modelSRAText);
            break;
        case 2:
        case 8:
        case 14:
            dc.SetBrush(__modelSRBBrush);
            dc.SetTextForeground(__modelSRBText);
            break;
        case 3:
        case 9:
        case 15:
            dc.SetBrush(__modelSRCBrush);
            dc.SetTextForeground(__modelSRCText);
            break;
        case 4:
        case 10:
        case 16:
            dc.SetBrush(__modelSRDBrush);
            dc.SetTextForeground(__modelSRDText);
            break;
        case 5:
        case 11:
        case 17:
            dc.SetBrush(__modelSREBrush);
            dc.SetTextForeground(__modelSREText);
            break;
        case 6:
        case 12:
        case 18:
            dc.SetBrush(__modelSRFBrush);
            dc.SetTextForeground(__modelSRFText);
            break;
        default:
            dc.SetBrush(__invalidBrush);
            break;
        }

        auto location = _location * scale;

        wxSize sz = wxSize(32, 32);
        dc.DrawRectangle(location + offset, sz);

        wxPoint pt = location + wxSize(2, (sz.y - 2 - 3 * SRY_GAP) * scale) + offset;
        DrawTextLimited(dc, _name, pt, sz - wxSize(4, 4));
        // pt += wxSize(0, ((VERTICAL_SIZE + SRY_GAP + SRYLABEL_SIZE) * scale) / 2);

        dc.SetBrush(origBrush);
        dc.SetPen(origPen);
        dc.SetTextForeground(origText);
    }

    virtual void AddRightClickMenu(wxMenu& mnu, ControllerModelDialog* cmd) override
    {
        if (_caps != nullptr && _caps->SupportsSmartRemotes()) {
            wxMenu* srMenu = new wxMenu();

            int srcount = _caps->GetSmartRemoteCount();

            auto mi = srMenu->AppendRadioItem(wxNewId(), "None");
            if (_smartRemote == 0)
                mi->Check();
            if (_caps->GetVendor() == "HinksPix") {
                for (int i = 0; i < srcount; i++) {
                    mi = srMenu->AppendRadioItem(wxNewId(), wxString::Format("%d", i));
                    if (_smartRemote == i + 1)
                        mi->Check();
                }
            } else {
                for (int i = 0; i < srcount; i++) {
                    mi = srMenu->AppendRadioItem(wxNewId(), wxString(char(65 + i)));
                    if (_smartRemote == i + 1)
                        mi->Check();
                }
            }

            srMenu->Connect(wxEVT_MENU, (wxObjectEventFunction)&ControllerModelDialog::OnPopupCommand, nullptr, cmd);
            mnu.AppendSubMenu(srMenu, "Smart Remote");
        }
    }

    virtual bool HandlePopup(wxWindow* parent, wxCommandEvent& event, int id) override
    {
        wxString label = ((wxMenu*)event.GetEventObject())->GetLabelText(id);
        if (label == "None") {
            SetAllModelsToReceiver(_port, _smartRemote, 0);
            return true;
        } else if ((label >= "0" && label <= "9") || (label >= "10" && label <= "19")) {
            SetAllModelsToReceiver(_port, _smartRemote, wxAtoi(label) + 1);
            return true;
        } else if (label >= "A" && label <= "Z") {
            SetAllModelsToReceiver(_port, _smartRemote, int(label[0]) - 64);
            return true;
        }
        return false;
    }
};

class ModelCMObject : public BaseCMObject
{
protected:
    ModelManager* _mm = nullptr;
    std::string _name;
    bool _outline = false;
    bool _main = false;
    std::string _displayName;
    int _string = 0;
    UDControllerPort* _port = nullptr;
    int _virtualString;
    bool _isShadowed = false;
    bool _isShadowFor = false;

public:
    ModelCMObject(UDControllerPort* port, int virtualString, const std::string& name, const std::string displayName, ModelManager* mm, UDController* cud, ControllerCaps* caps, wxPoint location, wxSize size, int style, double scale) :
        BaseCMObject(cud, caps, location, size, style, scale), _mm(mm), _port(port), _virtualString(virtualString)
    {
        _name = name;
        _main = name == displayName;
        _displayName = displayName;
        if (name != displayName && name + "-str-1" == displayName) {
            _main = true;
        }
        if (name != displayName) {
            _string = wxAtoi(AfterLast(displayName, '-')) - 1;
        }
        auto cmn = displayName.substr(0, displayName.find("-str-"));
        if (GetModel() != nullptr) {
            _isShadowed = GetModel()->GetModelManager().IsModelShadowing(GetModel());
            _isShadowFor = GetModel()->GetShadowModelFor() != "";
        } else if (cmn != "" && (*mm)[cmn] != nullptr) {
            auto m = (*mm)[cmn];
            _isShadowed = mm->IsModelShadowing(m);
            _isShadowFor = m->GetShadowModelFor() != "";
        }
    }

    std::string GetName() const
    {
        return _name;
    }
    std::string GetDisplayName() const
    {
        return _displayName;
    }
    int GetVirtualString() const
    {
        return _virtualString;
    }
    int GetString() const
    {
        return _string;
    }
    int GetSmartRemote() const
    {
        return GetUDModel()->GetSmartRemote();
    }
    UDControllerPort* GetPort() const
    {
        return _port;
    }
    bool NameStartsWith(char c)
    {
        if (_name == "")
            return false;
        char cn = _name[0];
        if (cn >= 65 && cn <= 90)
            cn += 32;

        return cn == c;
    }
    UDControllerPortModel* GetUDModel() const
    {
        if (_cud == nullptr)
            return nullptr;
        return _cud->GetControllerPortModel(_name, _string);
    }
    Model* AlwaysGetModel() const
    {
        return _mm->GetModel(_name);
    }
    Model* GetModel() const
    {
        if (_main) {
            return _mm->GetModel(_name);
        } else {
            return nullptr;
        }
    }
    bool IsMain() const
    {
        return _main;
    }
    bool IsOutline() const
    {
        return _outline;
    }
    bool IsPortSR(int p, int sr) const
    {
        return _port->GetPort() == p && GetSmartRemote() == sr;
    }
    virtual std::string GetType() const override
    {
        return "MODEL";
    }
    virtual void Draw(wxDC& dc, int portMargin, wxPoint mouse, wxPoint adjustedMouse, wxSize offset, float scale, bool printing, bool border, Model* lastDropped) override
    {
        auto origBrush = dc.GetBrush();
        auto origPen = dc.GetPen();
        auto origText = dc.GetTextForeground();
        dc.SetTextForeground(__textForeground);

        Model* m = _mm->GetModel(_name);
        UDControllerPortModel* udcpm = GetUDModel();

        if (!border) {
            dc.SetPen(*wxTRANSPARENT_PEN);
        } else {
            if (!printing && m == lastDropped && _main) {
                dc.SetPen(__modelOutlineLastDroppedPen);
            } else {
                dc.SetPen(__modelOutlinePen);
            }
        }

        if (udcpm != nullptr) {
            int maxSR = 15;
            if (_caps != nullptr)
                maxSR = _caps->GetSmartRemoteCount();

            if (udcpm->GetSmartRemote() > maxSR) {
                dc.SetBrush(__invalidBrush);
            } else {
                dc.SetBrush(__modelSRNoneBrush);
            }
        } else {
            dc.SetBrush(__modelSRNoneBrush);
        }

        if (_invalid) {
            dc.SetBrush(__invalidBrush);
        }

        _outline = false;
        if (_dragging && !printing) {
            dc.SetBrush(wxColour(255, 255, 128));
        } else if (mouse.x > portMargin && HitTest(adjustedMouse) != HITLOCATION::NONE && !printing) {
            _outline = true;
            dc.SetPen(wxPen(dc.GetPen().GetColour(), 3));
        }

        auto location = _location * scale;

        wxSize sz = _size;
        sz = sz.Scale(scale, scale);
        if (_isShadowed) {
            dc.SetPen(wxPen(dc.GetPen().GetColour(), dc.GetPen().GetWidth(), wxPENSTYLE_LONG_DASH));
            dc.SetBrush(__modelShadowBrush);
        }
        if (_isShadowFor) {
            dc.SetPen(wxPen(dc.GetPen().GetColour(), dc.GetPen().GetWidth(), wxPENSTYLE_LONG_DASH));
        }
        dc.DrawRectangle(location + offset, sz);
        if (_over == HITLOCATION::LEFT) {
            dc.SetPen(__dropTargetPen);
            dc.SetBrush(__dropTargetBrush);
            wxSize ssz = wxSize((sz.x - 2) / 2, sz.y - 2);
            dc.DrawRectangle(location + offset + wxSize(1, 1), ssz);
        } else if (_over == HITLOCATION::RIGHT) {
            dc.SetPen(__dropTargetPen);
            dc.SetBrush(__dropTargetBrush);
            wxSize ssz = wxSize((sz.x - 2) / 2, sz.y - 2);
            dc.DrawRectangle(location + offset + wxSize((sz.x / 2), 1), ssz);
        }

        wxPoint pt = location + wxSize(2, 2) + offset;
        DrawTextLimited(dc, _displayName, pt, sz - wxSize(4, 4));
        pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
        if (m != nullptr) {
            auto iconType = "xlART_" + m->GetDisplayAs() + "_ICON";
            int iconSize = MODEL_ICON_SIZE;
            if (iconSize > 24) {
                iconSize = 32;
            } else {
                iconSize = 16;
            }

            wxBitmapBundle bmp = wxArtProvider::GetBitmapBundle(wxART_MAKE_ART_ID_FROM_STR(iconType), wxART_BUTTON, wxDefaultSize);
            if (bmp.IsOk()) {
#ifdef __WXOSX__
                dc.DrawBitmap(bmp.GetBitmap(wxSize(iconSize * 2, iconSize * 2)), location.x + sz.x - ScaleWithSystemDPI(GetSystemContentScaleFactor(), 3) - iconSize, pt.y);
#else
                int i2 = ScaleWithSystemDPI(GetSystemContentScaleFactor(), iconSize);
                wxIcon icon = bmp.GetIcon(wxSize(i2, i2));
                dc.DrawIcon(icon, location.x + sz.x - ScaleWithSystemDPI(GetSystemContentScaleFactor(), 3) - iconSize, pt.y);
#endif
            }

            if (udcpm != nullptr) {
                uint32_t chs = udcpm->Channels();
                if (_style & STYLE_PIXELS) {
                    if (udcpm->GetModel()->GetDisplayAs() == "Channel Block" || udcpm->GetModel()->SingleChannel || udcpm->GetModel()->SingleNode) {
                        DrawTextLimited(dc, wxString::Format("Channels: %ld", (long)chs), pt, sz - wxSize(4, 4));
                    } else {
                        DrawTextLimited(dc, wxString::Format("Pixels: %ld", (long)chs / udcpm->GetChannelsPerPixel()), pt, sz - wxSize(4, 4));
                    }
                    pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
                }
                if (_style & STYLE_CHANNELS) {
                    DrawTextLimited(dc, wxString::Format("Channels: %ld [%d]", (long)chs, udcpm->GetDMXChannelOffset()), pt, sz - wxSize(4, 4));
                    pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
                }
            }
            if (_style & STYLE_STRINGS) {
                DrawTextLimited(dc, wxString::Format("Strings: %d", m->GetNumPhysicalStrings()), pt, sz - wxSize(4, 4));
                pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
            }
        }

        dc.SetBrush(origBrush);
        dc.SetPen(origPen);
        dc.SetTextForeground(origText);
    }

    virtual void AddRightClickMenu(wxMenu& mnu, ControllerModelDialog* cmd) override
    {
        if (_caps != nullptr && GetModel() != nullptr && GetModel()->IsPixelProtocol()) {
            if (!GetModel()->HasSingleNode(GetModel()->GetStringType()) && GetModel()->SupportsChangingStringCount()) {
                mnu.AppendSeparator();
                mnu.Append(ControllerModelDialog::CONTROLLER_MODEL_STRINGS, "Change String Count");
            }

            if (_caps->SupportsSmartRemotes()) {
                wxMenu* srMenu = new wxMenu();
                mnu.AppendSeparator();

                int srcount = _caps->GetSmartRemoteCount();

                auto mi = srMenu->AppendRadioItem(wxNewId(), "None");
                if (GetModel()->GetSmartRemote() == 0){
                    mi->Check();
                }

                for (int i = 0; i < srcount; i++) {
                    if (_caps->GetVendor() == "HinksPix") {
                        mi = srMenu->AppendRadioItem(wxNewId(), wxString::Format("%d", i), "SR Port");
                    } else {
                        mi = srMenu->AppendRadioItem(wxNewId(), wxString(char(65 + i)), "SR Port");
                    }
                    if (GetModel()->GetSmartRemote() == i + 1) {
                        mi->Check();
                    }
                }

                if (_caps->GetSmartRemoteTypes().size() > 1) {
                    wxMenu* srType = new wxMenu();
                    for (auto const& smtype : _caps->GetSmartRemoteTypes()) {
                        mi = srType->AppendRadioItem(wxNewId(), smtype);
                        if (GetModel()->GetSmartRemoteType() == smtype) {
                            mi->Check();
                        }
                    }
                    srMenu->AppendSeparator();
                    srMenu->AppendSubMenu(srType, "Type");
                    srType->Connect(wxEVT_MENU, (wxObjectEventFunction)&ControllerModelDialog::OnPopupCommand, nullptr, cmd);
                }
                srMenu->AppendSeparator();
                mi = srMenu->AppendCheckItem(ControllerModelDialog::CONTROLLER_CASCADEDOWNPORT, "Cascade Down Port");
                mi->Check(GetModel()->GetSRCascadeOnPort());

                wxMenu* srMax = new wxMenu();
                for (int i = 0; i < srcount; i++) {
                    mi = srMax->AppendRadioItem(wxNewId(), wxString::Format("%d", i + 1), "Cascade");
                    if (GetModel()->GetSRMaxCascade() == i + 1) {
                        mi->Check();
                    }
                }

                srMenu->AppendSubMenu(srMax, "Cascaded Remotes");
                srMax->Connect(wxEVT_MENU, (wxObjectEventFunction)&ControllerModelDialog::OnPopupCommand, nullptr, cmd);

                srMenu->Connect(wxEVT_MENU, (wxObjectEventFunction)&ControllerModelDialog::OnPopupCommand, nullptr, cmd);
                mnu.AppendSubMenu(srMenu, "Smart Remote");
            }

            if (_caps->SupportsPixelPortBrightness()) {
                mnu.AppendSeparator();
                mnu.Append(ControllerModelDialog::CONTROLLER_BRIGHTNESS, "Set Brightness");
                if (GetModel()->IsControllerBrightnessSet()) {
                    mnu.Append(ControllerModelDialog::CONTROLLER_BRIGHTNESSCLEAR, "Clear Brightness");
                }
            }
            if (_caps->SupportsPixelPortNullPixels()) {
                mnu.Append(ControllerModelDialog::CONTROLLER_STARTNULLS, "Set Start Nulls");
            }
            if (_caps->SupportsPixelPortEndNullPixels()) {
                mnu.Append(ControllerModelDialog::CONTROLLER_ENDNULLS, "Set End Nulls");
            }
            if (_caps->SupportsPixelPortColourOrder()) {
                mnu.Append(ControllerModelDialog::CONTROLLER_COLORORDER, "Set Color Order");
            }
            if (_caps->SupportsPixelPortGrouping()) {
                mnu.Append(ControllerModelDialog::CONTROLLER_GROUPCOUNT, "Set Group Count");
            }
            
        } else if (GetModel() != nullptr && GetModel()->IsSerialProtocol()) {
            mnu.AppendSeparator();
            mnu.Append(ControllerModelDialog::CONTROLLER_DMXCHANNEL, "Set Channel");
            mnu.Append(ControllerModelDialog::CONTROLLER_DMXCHANNELCHAIN, "Set Channel and Chain");
        }
    }

    virtual bool HandlePopup(wxWindow* parent, wxCommandEvent& event, int id) override
    {
        if (GetModel() == nullptr)
            return false;

        if (id == ControllerModelDialog::CONTROLLER_CASCADEDOWNPORT) {
            GetModel()->SetSRCascadeOnPort(((wxMenu*)event.GetEventObject())->IsChecked(id));
            return true;
        } else if (id == ControllerModelDialog::CONTROLLER_DMXCHANNEL) {
            wxNumberEntryDialog dlg(parent, "Enter the DMX channel", "Channel", "DMX Channel", GetModel()->GetControllerDMXChannel(), 1, 512);
            if (dlg.ShowModal() == wxID_OK) {
                GetModel()->SetControllerDMXChannel(dlg.GetValue());
            }
            return true;
        } else if (id == ControllerModelDialog::CONTROLLER_DMXCHANNELCHAIN) {
            wxNumberEntryDialog dlg(parent, "Enter the DMX channel", "Channel", "DMX Channel", GetModel()->GetControllerDMXChannel(), 1, 512);
            if (dlg.ShowModal() == wxID_OK) {
                auto port = _cud->GetPortContainingModel(GetModel());
                bool start = false;
                auto ch = dlg.GetValue();
                for (const auto& it : port->GetModels()) {
                    if (start || it->GetModel() == GetModel()) {
                        it->GetModel()->SetControllerDMXChannel(ch);
                        ch += it->Channels();
                        start = true;
                    }
                }
            }
            return true;
        } else if (id == ControllerModelDialog::CONTROLLER_BRIGHTNESS) {
            wxNumberEntryDialog dlg(parent, "Enter the Model Brightness", "Brightness", "Model Brightness", GetModel()->GetControllerBrightness(), 0, 100);
            if (dlg.ShowModal() == wxID_OK) {
                GetModel()->SetControllerBrightness(dlg.GetValue());
            }
            return true;
        } else if (id == ControllerModelDialog::CONTROLLER_STARTNULLS) {
            wxNumberEntryDialog dlg(parent, "Enter the Model Start Nulls", "Start Nulls", "Start Nulls", GetModel()->GetControllerStartNulls(), 0, 100);
            if (dlg.ShowModal() == wxID_OK) {
                GetModel()->SetControllerStartNulls(dlg.GetValue());
            }
            return true;
        } else if (id == ControllerModelDialog::CONTROLLER_ENDNULLS) {
            wxNumberEntryDialog dlg(parent, "Enter the End Nulls", "End Nulls", "Model End Nulls", GetModel()->GetControllerEndNulls(), 0, 100);
            if (dlg.ShowModal() == wxID_OK) {
                GetModel()->SetControllerEndNulls(dlg.GetValue());
            }
            return true;
        } else if (id == ControllerModelDialog::CONTROLLER_COLORORDER) {
            auto choices = Model::CONTROLLER_COLORORDER;
            int selection = choices.Index(GetModel()->GetControllerColorOrder());
            wxSingleChoiceDialog dlg(parent, "Model Color Order", "Color Order", choices);
            if (selection >= 0 && selection < choices.size()) {
                dlg.SetSelection(selection);
            }
            if (dlg.ShowModal() == wxID_OK) {
                GetModel()->SetControllerColorOrder(choices[dlg.GetSelection()]);
            }
            return true;
        } else if (id == ControllerModelDialog::CONTROLLER_GROUPCOUNT) {
            wxNumberEntryDialog dlg(parent, "Enter the Group Count", "Group Count", "Model Group Count", GetModel()->GetControllerGroupCount(), 1, 500);
            if (dlg.ShowModal() == wxID_OK) {
                GetModel()->SetControllerGroupCount(dlg.GetValue());
            }
            return true;
        } else if (id == ControllerModelDialog::CONTROLLER_BRIGHTNESSCLEAR) {
            GetModel()->ClearControllerBrightness();
            return true;
        } else if (id == ControllerModelDialog::CONTROLLER_MODEL_STRINGS) {
            wxNumberEntryDialog dlg(parent, "Set String Count", "String Count", "Model String Count", GetModel()->GetNumPhysicalStrings(), 1, 48);
            if (dlg.ShowModal() == wxID_OK) {
                std::string mess;
                if (!GetModel()->ChangeStringCount(dlg.GetValue(), mess)) {
                    DisplayError(mess, parent);
                }
            }
            return true;
        } else {
            wxString label = ((wxMenu*)event.GetEventObject())->GetLabelText(id);
            wxString title = ((wxMenu*)event.GetEventObject())->GetHelpString(id);
            auto const types = GetModel()->GetSmartRemoteTypes();
            if (std::find(types.begin(), types.end(), label.ToStdString()) != types.end()) {
                int const port = GetModel()->GetControllerPort();
                int const sm = GetModel()->GetSmartRemote();
                int lowPort = (((port - 1) / 4) * 4) + 1;
                int highPort = lowPort + 3 < _cud->GetMaxPixelPort() ? lowPort + 3 : _cud->GetMaxPixelPort();
                for (int i = lowPort; i <= highPort; i++) {
                    for (const auto& it : _cud->GetControllerPixelPort(i)->GetModels()) {
                        if (it->GetModel()->GetSmartRemote() == sm) {
                            it->GetModel()->SetSmartRemoteType(label);
                        }
                    }
                }
                return true;
            } else if (label == "None") {
                GetModel()->SetSmartRemote(0);
                return true;
            } else if (title == "SR Port") {
                if (label >= "A" && label <= "Z") {
                    GetModel()->SetSmartRemote(int(label[0]) - 64);
                    return true;
                }
                GetModel()->SetSmartRemote(wxAtoi(label) + 1);
                return true;
            } else if (title == "Cascade") {
                int max = wxAtoi(label);
                if (max >= 1) {
                    GetModel()->SetSRMaxCascade(max);
                    return true;
                }
            }
        }
        return false;
    }
};
#pragma endregion

#pragma region Drop Target
class CMDTextDropTarget : public wxTextDropTarget
{
public:
    CMDTextDropTarget(std::list<BaseCMObject*>* objects, ControllerModelDialog* owner, wxPanel* target, bool anywhere, double& scale, ControllerCaps* caps) :
        _owner(owner),
        _scale(scale),
        _objects(objects),
        _target(target),
        _anywhere(anywhere),
        _caps(caps){};

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override
    {
        if (data == "")
            return false;

        wxPoint mouse = wxPoint(x, y);
        mouse += _owner->GetScrollPosition(_target);

        if (data.StartsWith("Model:")) {
            _owner->DropFromModels(mouse, data.AfterFirst(':'), _target);
            return true;
        } else if (data.StartsWith("Controller:")) {
            _owner->DropFromController(mouse, data.AfterFirst(':').ToStdString(), _target);
            return true;
        }
        return false;
    }

#define SCROLLMARGIN 20
#define SCROLLBY 20
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override
    {
        // handle auto scroll while dragging
        if (y > _target->GetSize().GetHeight() - SCROLLMARGIN) {
            if (_owner->Scroll(_target, 0, SCROLLBY))
                return wxDragNone;
        } else if (y < SCROLLMARGIN) {
            if (_owner->Scroll(_target, 0, -SCROLLBY))
                return wxDragNone;
        } else if (x < SCROLLMARGIN) {
            if (_owner->Scroll(_target, -SCROLLBY, 0))
                return wxDragNone;
        } else if (x > _target->GetSize().GetWidth() - SCROLLMARGIN) {
            if (_owner->Scroll(_target, SCROLLBY, 0))
                return wxDragNone;
        }

        if (_anywhere)
            return wxDragMove;

        wxDragResult res = wxDragNone;
        BaseCMObject* port = nullptr;
        BaseCMObject* sr = nullptr;

        wxPoint mouse(x, y);
        mouse += _owner->GetScrollPosition(_target);

        int portMargin = _objects->front()->GetRect().GetRight() + 2;

        for (const auto& it : *_objects) {
            it->SetOver(BaseCMObject::HITLOCATION::NONE);
        }

        for (const auto& it : *_objects) {
            auto m = dynamic_cast<ModelCMObject*>(it);
            auto sro = dynamic_cast<SRCMObject*>(it);
            // we can only be over a model if we are to the right of the port labels
            if (it->GetType() == "MODEL" && x > portMargin && it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE && !_owner->IsDragging(m) && (m == nullptr || m->IsMain())) {
                it->SetOver(it->HitTest(mouse));
                res = wxDragMove;
                if (port != nullptr)
                    port->SetOver(BaseCMObject::HITLOCATION::NONE);
                if (sr != nullptr)
                    sr->SetOver(BaseCMObject::HITLOCATION::NONE);
                port = nullptr;
                sr = nullptr;
                break;
            } else if (it->GetType() == "SR" && x > portMargin && it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE && !sro->IsSameRemote(_owner->GetDragging())) {
                // if we are dragging model then it should be all ... but if it is a SR then it should be the hit test result
                it->SetOver(BaseCMObject::HITLOCATION::ALL);
                if (port != nullptr)
                    port->SetOver(BaseCMObject::HITLOCATION::NONE);
                sr = it;
                port = nullptr;
                res = wxDragMove;
            } else if (it->GetType() == "PORT" && it->HitYTest(wxPoint(x, y + _owner->GetScrollPosition(_target).y))) {
                if (sr == nullptr) {
                    it->SetOver(BaseCMObject::HITLOCATION::ALL);
                    port = it;
                    if (sr != nullptr)
                        sr->SetOver(BaseCMObject::HITLOCATION::NONE);
                    sr = nullptr;
                    res = wxDragMove;
                }
            }
        }

        _target->Refresh();

        return res;
    }

    std::list<BaseCMObject*>* _objects = nullptr;
    ControllerModelDialog* _owner = nullptr;
    wxPanel* _target = nullptr;
    bool _anywhere = false;
    double& _scale;
    ControllerCaps* _caps = nullptr;
};
#pragma endregion

#pragma region Printing
ControllerModelPrintout::ControllerModelPrintout(ControllerModelDialog* controllerDialog, const wxString& title, wxSize boxSize, wxSize panelSize) :
    _controllerDialog(controllerDialog),
    _page_count(1),
    _page_count_w(1),
    _page_count_h(1),
    _orient(wxPORTRAIT),
    _paper_type(wxPAPER_LETTER),
    _max_x(600),
    _max_y(800),
    _box_size(boxSize),
    _panel_size(panelSize)
{}

bool ControllerModelPrintout::OnPrintPage(int pageNum)
{
    int x_page = (pageNum - 1) % _page_count_w;
    int y_page = (pageNum - 1) / _page_count_w;
    int startX = x_page * _max_x;
    int startY = y_page * _max_y;

    wxString pagename = wxString::Format("Page %d-%d", y_page + 1, x_page + 1);

    wxBitmap bmp = _controllerDialog->RenderPicture(startY, startX, _max_x, _max_y, pagename);
    wxDC* dc = GetDC();

    MapScreenSizeToPage();
    FitThisSizeToPageMargins(bmp.GetSize(), _page_setup);
    dc->DrawBitmap(bmp, 0, 0);
    return true;
}

bool ControllerModelPrintout::HasPage(int pageNum)
{
    if (pageNum >= 1 && pageNum <= _page_count)
        return true;
    else
        return false;
}

void ControllerModelPrintout::OnBeginPrinting()
{
    wxSize paperSize = _page_setup.GetPaperSize();

    // I'm using arbitrary scale factors
    if (_orient == wxPORTRAIT) {
        _max_x = paperSize.GetWidth() * 3.0;
        _max_y = paperSize.GetHeight() * 3.0;
    } else {
        _max_x = paperSize.GetHeight() * 3.0;
        _max_y = paperSize.GetWidth() * 3.0;
    }

    FitThisSizeToPageMargins(wxSize(_max_x, _max_y), _page_setup);

    int boxPerPageH = _max_y / _box_size.GetY();
    _max_y = (boxPerPageH)*_box_size.GetY();
    _page_count_h = std::ceil((float)_panel_size.GetY() / (float)_max_y);

    int boxPerPageW = _max_x / _box_size.GetX();
    _max_x = (boxPerPageW)*_box_size.GetX();
    _page_count_w = std::ceil((float)_panel_size.GetX() / (float)_max_x);

    _page_count = _page_count_w * _page_count_h;
}

void ControllerModelPrintout::preparePrint(const bool showPageSetupDialog)
{
    _page_setup.SetMarginTopLeft(wxPoint(16, 16));
    _page_setup.SetMarginBottomRight(wxPoint(16, 16));
    if (showPageSetupDialog) {
        wxPageSetupDialog dialog(NULL, &_page_setup);
        if (dialog.ShowModal() == wxID_OK) {
            _page_setup = dialog.GetPageSetupData();
            _orient = _page_setup.GetPrintData().GetOrientation();
            _paper_type = _page_setup.GetPrintData().GetPaperId();
        }
    } else {
        // don't show page setup dialog, use default values
        wxPrintData printdata;
        printdata.SetPrintMode(wxPRINT_MODE_PRINTER);
        printdata.SetOrientation(wxPORTRAIT); // wxPORTRAIT, wxLANDSCAPE
        printdata.SetNoCopies(1);
        printdata.SetPaperId(wxPAPER_LETTER);

        _page_setup = wxPageSetupDialogData(printdata);
        _orient = printdata.GetOrientation();
        _paper_type = printdata.GetPaperId();
    }
}
#pragma endregion

#pragma region Constructor
ControllerModelDialog::ControllerModelDialog(wxWindow* parent, UDController* cud, ModelManager* mm, Controller* controller, wxWindowID id, const wxPoint& pos, const wxSize& size) :
    _cud(cud),
    _controller(controller),
    _mm(mm),
    _xLights((xLightsFrame*)parent)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    //(*Initialize(ControllerModelDialog)
    wxBoxSizer* BoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer5;

    Create(parent, wxID_ANY, _("Controller Visualiser"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX | wxMAXIMIZE_BOX, _T("wxID_ANY"));
    SetClientSize(wxSize(500, 500));
    FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxTAB_TRAVERSAL, _T("ID_SPLITTERWINDOW1"));
    SplitterWindow1->SetMinimumPaneSize(50);
    SplitterWindow1->SetSashGravity(1);
    Panel3 = new wxPanel(SplitterWindow1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer5->AddGrowableCol(0);
    FlexGridSizer5->AddGrowableRow(0);
    PanelController = new wxPanel(Panel3, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE, _T("ID_PANEL1"));
    FlexGridSizer5->Add(PanelController, 1, wxALL | wxEXPAND, 0);
    ScrollBar_Controller_V = new wxScrollBar(Panel3, ID_SCROLLBAR1, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL, wxDefaultValidator, _T("ID_SCROLLBAR1"));
    ScrollBar_Controller_V->SetScrollbar(0, 1, 10, 1);
    FlexGridSizer5->Add(ScrollBar_Controller_V, 1, wxALL | wxEXPAND, 0);
    ScrollBar_Controller_H = new wxScrollBar(Panel3, ID_SCROLLBAR2, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL, wxDefaultValidator, _T("ID_SCROLLBAR2"));
    ScrollBar_Controller_H->SetScrollbar(0, 1, 10, 1);
    FlexGridSizer5->Add(ScrollBar_Controller_H, 1, wxALL | wxEXPAND, 0);
    FlexGridSizer5->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    StaticText1 = new wxStaticText(Panel3, ID_STATICTEXT1, _("Box Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    BoxSizer1->Add(StaticText1, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE, 5);
    Slider_Box_Scale = new wxSlider(Panel3, ID_SLIDER_BOX_SCALE, 10, 1, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BOX_SCALE"));
    Slider_Box_Scale->SetTickFreq(1);
    Slider_Box_Scale->SetToolTip(_("Box Size"));
    BoxSizer1->Add(Slider_Box_Scale, 1, wxALL | wxEXPAND, 1);
    StaticText2 = new wxStaticText(Panel3, ID_STATICTEXT2, _("Font Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    BoxSizer1->Add(StaticText2, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE, 5);
    Slider_Font_Scale = new wxSlider(Panel3, ID_SLIDER_FONT_SCALE, 15, 1, 72, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_FONT_SCALE"));
    Slider_Font_Scale->SetTickFreq(1);
    Slider_Font_Scale->SetToolTip(_("Font Size"));
    BoxSizer1->Add(Slider_Font_Scale, 1, wxALL | wxEXPAND, 1);
    FlexGridSizer5->Add(BoxSizer1, 1, wxALL | wxEXPAND, 3);
    FlexGridSizer5->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Check = new wxTextCtrl(Panel3, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(-1, 100), wxTE_MULTILINE | wxTE_READONLY | wxALWAYS_SHOW_SB, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer5->Add(TextCtrl_Check, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer5->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Panel3->SetSizer(FlexGridSizer5);
    FlexGridSizer5->Fit(Panel3);
    FlexGridSizer5->SetSizeHints(Panel3);
    Panel4 = new wxPanel(SplitterWindow1, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer3->AddGrowableCol(0);
    FlexGridSizer3->AddGrowableRow(1);
    CheckBox_HideOtherControllerModels = new wxCheckBox(Panel4, ID_CHECKBOX1, _("Hide models assigned to other controllers"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_HideOtherControllerModels->SetValue(false);
    FlexGridSizer3->Add(CheckBox_HideOtherControllerModels, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    FlexGridSizer2->AddGrowableRow(0);
    PanelModels = new wxPanel(Panel4, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE, _T("ID_PANEL2"));
    FlexGridSizer2->Add(PanelModels, 1, wxALL | wxEXPAND, 0);
    ScrollBar_Models = new wxScrollBar(Panel4, ID_SCROLLBAR3, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL, wxDefaultValidator, _T("ID_SCROLLBAR3"));
    ScrollBar_Models->SetScrollbar(0, 1, 10, 1);
    FlexGridSizer2->Add(ScrollBar_Models, 1, wxALL | wxEXPAND, 0);
    FlexGridSizer3->Add(FlexGridSizer2, 1, wxALL | wxEXPAND, 0);
    Panel4->SetSizer(FlexGridSizer3);
    FlexGridSizer3->Fit(Panel4);
    FlexGridSizer3->SetSizeHints(Panel4);
    SplitterWindow1->SplitVertically(Panel3, Panel4);
    SplitterWindow1->SetSashPosition(1000);
    FlexGridSizer1->Add(SplitterWindow1, 1, wxALL | wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    SetSizer(FlexGridSizer1);
    Layout();

    PanelController->Connect(wxEVT_PAINT, (wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerPaint, 0, this);
    PanelController->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerKeyDown, 0, this);
    PanelController->Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerLeftDown, 0, this);
    PanelController->Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerLeftUp, 0, this);
    PanelController->Connect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerLeftDClick, 0, this);
    PanelController->Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerRightDown, 0, this);
    PanelController->Connect(wxEVT_MOTION, (wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerMouseMove, 0, this);
    PanelController->Connect(wxEVT_ENTER_WINDOW, (wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerMouseEnter, 0, this);
    PanelController->Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerMouseLeave, 0, this);
    PanelController->Connect(wxEVT_MOUSEWHEEL, (wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerMouseWheel, 0, this);
    PanelController->Connect(wxEVT_SIZE, (wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerResize, 0, this);
    Connect(ID_SCROLLBAR1, wxEVT_SCROLL_TOP | wxEVT_SCROLL_BOTTOM | wxEVT_SCROLL_LINEUP | wxEVT_SCROLL_LINEDOWN | wxEVT_SCROLL_PAGEUP | wxEVT_SCROLL_PAGEDOWN | wxEVT_SCROLL_THUMBTRACK | wxEVT_SCROLL_THUMBRELEASE | wxEVT_SCROLL_CHANGED, (wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_VScroll);
    Connect(ID_SCROLLBAR1, wxEVT_SCROLL_THUMBTRACK, (wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_VScrollThumbTrack);
    Connect(ID_SCROLLBAR1, wxEVT_SCROLL_CHANGED, (wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_VScrollChanged);
    Connect(ID_SCROLLBAR2, wxEVT_SCROLL_TOP | wxEVT_SCROLL_BOTTOM | wxEVT_SCROLL_LINEUP | wxEVT_SCROLL_LINEDOWN | wxEVT_SCROLL_PAGEUP | wxEVT_SCROLL_PAGEDOWN | wxEVT_SCROLL_THUMBTRACK | wxEVT_SCROLL_THUMBRELEASE | wxEVT_SCROLL_CHANGED, (wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_HScroll);
    Connect(ID_SCROLLBAR2, wxEVT_SCROLL_THUMBTRACK, (wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_HScrollThumbTrack);
    Connect(ID_SCROLLBAR2, wxEVT_SCROLL_CHANGED, (wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_HScrollChanged);
    Connect(ID_SLIDER_BOX_SCALE, wxEVT_COMMAND_SLIDER_UPDATED, (wxObjectEventFunction)&ControllerModelDialog::OnSlider_ScaleCmdSliderUpdated);
    Connect(ID_SLIDER_FONT_SCALE, wxEVT_COMMAND_SLIDER_UPDATED, (wxObjectEventFunction)&ControllerModelDialog::OnSlider_ScaleCmdSliderUpdated);
    Connect(ID_CHECKBOX1, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&ControllerModelDialog::OnCheckBox_HideOtherControllerModelsClick);
    PanelModels->Connect(wxEVT_PAINT, (wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsPaint, 0, this);
    PanelModels->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsKeyDown, 0, this);
    PanelModels->Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsLeftDown, 0, this);
    PanelModels->Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsLeftUp, 0, this);
    PanelModels->Connect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsLeftDClick, 0, this);
    PanelModels->Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsRightDown, 0, this);
    PanelModels->Connect(wxEVT_MOTION, (wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsMouseMove, 0, this);
    PanelModels->Connect(wxEVT_ENTER_WINDOW, (wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsMouseEnter, 0, this);
    PanelModels->Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsMouseLeave, 0, this);
    PanelModels->Connect(wxEVT_MOUSEWHEEL, (wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsMouseWheel, 0, this);
    PanelModels->Connect(wxEVT_SIZE, (wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsResize, 0, this);
    Connect(ID_SCROLLBAR3, wxEVT_SCROLL_TOP | wxEVT_SCROLL_BOTTOM | wxEVT_SCROLL_LINEUP | wxEVT_SCROLL_LINEDOWN | wxEVT_SCROLL_PAGEUP | wxEVT_SCROLL_PAGEDOWN | wxEVT_SCROLL_THUMBTRACK | wxEVT_SCROLL_THUMBRELEASE | wxEVT_SCROLL_CHANGED, (wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_ModelsScroll);
    Connect(ID_SCROLLBAR3, wxEVT_SCROLL_THUMBTRACK, (wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_ModelsScrollThumbTrack);
    Connect(ID_SCROLLBAR3, wxEVT_SCROLL_CHANGED, (wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_ModelsScrollChanged);
    //*)

    ::SetColours(false);

    PanelController->SetBackgroundStyle(wxBG_STYLE_PAINT);
    PanelModels->SetBackgroundStyle(wxBG_STYLE_PAINT);

    Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&ControllerModelDialog::OnKeyDown, 0, this);
    TextCtrl_Check->Bind(wxEVT_KEY_DOWN, &ControllerModelDialog::OnKeyDown, this);
    CheckBox_HideOtherControllerModels->Bind(wxEVT_KEY_DOWN, &ControllerModelDialog::OnKeyDown, this);
    ScrollBar_Controller_H->Bind(wxEVT_KEY_DOWN, &ControllerModelDialog::OnKeyDown, this);
    ScrollBar_Controller_V->Bind(wxEVT_KEY_DOWN, &ControllerModelDialog::OnKeyDown, this);
    ScrollBar_Models->Bind(wxEVT_KEY_DOWN, &ControllerModelDialog::OnKeyDown, this);

    // PanelController->SetBackgroundStyle(wxBG_STYLE_PAINT);
    // PanelModels->SetBackgroundStyle(wxBG_STYLE_PAINT);

    wxSize panel4Size = wxSize(std::max((int)(LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + LEFT_RIGHT_MARGIN + ScrollBar_Models->GetSize().GetWidth() + 10), (int)CheckBox_HideOtherControllerModels->GetSize().GetWidth()), -1);
    Panel4->SetMinSize(panel4Size);
    SetMinSize(wxSize(800, 400));
    SetSize(wxSize(1200, 800));
    SplitterWindow1->SetSashPosition(1000);

    if (_cud == nullptr) {
        logger_base.crit("ControllerModelDialog created with no ControllerUploadData ... this is not going to end well.");
    }

    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        CheckBox_HideOtherControllerModels->SetValue(config->ReadBool("ControllerModelHideOtherControllerModels", false));
        Slider_Box_Scale->SetValue(config->ReadLong("ControllerModelBoxScale", 10));
        Slider_Font_Scale->SetValue(config->ReadLong("ControllerModelFontScale", 15));
        _scale = Slider_Box_Scale->GetValue() / 10.0;

        SplitterWindow1->SetSashPosition(config->ReadLong("ControllerModelSashPosition", 1000));

        wxPoint loc;
        wxSize sz;
        LoadWindowPosition("ControllerModelDialogPosition", sz, loc);
        if (loc.x != -1) {
            SetPosition(loc);
            SetSize(sz);
            Layout();
        }
        EnsureWindowHeaderIsOnScreen(this);
    }

    _autoLayout = _controller->IsAutoLayout();

    _title = controller->GetLongDescription();
    SetLabel(_title);

    _caps = ControllerCaps::GetControllerConfig(_controller);

    CMDTextDropTarget* cmdt = new CMDTextDropTarget(&_controllers, this, PanelController, false, _scale, _caps);
    PanelController->SetDropTarget(cmdt);

    cmdt = new CMDTextDropTarget(&_models, this, PanelModels, true, _scale, _caps);
    PanelModels->SetDropTarget(cmdt);

    bool changed = false;

    if (_autoLayout) {
        // If you are doing auto layout then all models must have controller name set ... this may much up model chaining but it has to be done
        // or things will get funky
        if (_cud->SetAllModelsToControllerName(_controller->GetName())) {
            wxMessageBox("At least one model had to have its controller name set because you are using Auto Layout. This may have mucked up the order of model chaining on some ports and you will need to fix that up here.", wxEmptyString, 5, parent);
            changed = true;
        }
    } else {
        CheckBox_HideOtherControllerModels->SetValue(false);
        CheckBox_HideOtherControllerModels->Enable(false);
    }

    if (_caps != nullptr) {
        changed |= _cud->SetAllModelsToValidProtocols(_caps->GetPixelProtocols(), _caps->GetSerialProtocols(), !_caps->SupportsMultipleSimultaneousOutputProtocols());

        if (!_caps->SupportsSmartRemotes()) {
            changed |= _cud->ClearSmartRemoteOnAllModels();
        }
    }

    // force all models to be re-laid out if auto layout models is selected
    if (_controller->IsAutoLayout()) {
        _xLights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ControllerModelDialog::ControllerModelDialog");
        _xLights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerModelDialog::ControllerModelDialog");
    }

    if (changed) {
        // Now need to let all the recalculations work
        while (!_xLights->DoAllWork()) {
            // dont get into a redraw loop from here
            _xLights->GetOutputModelManager()->RemoveWork("ASAP", OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW);
        }
    }

    ReloadModels();
    Layout();

    PanelModels->SetFocus();
}

ControllerModelDialog::~ControllerModelDialog()
{
    SaveWindowPosition("ControllerModelDialogPosition", this);
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("ControllerModelSashPosition", SplitterWindow1->GetSashPosition());

    while (_models.size() > 0) {
        delete _models.front();
        _models.pop_front();
    }

    while (_controllers.size() > 0) {
        delete _controllers.front();
        _controllers.pop_front();
    }

    //(*Destroy(ControllerModelDialog)
    //*)
}
#pragma endregion

bool ModelSortName(const BaseCMObject* first, const BaseCMObject* second)
{
    auto mm1 = dynamic_cast<const ModelCMObject*>(first);
    auto mm2 = dynamic_cast<const ModelCMObject*>(second);

    if (mm1 == nullptr || mm2 == nullptr) {
        return first->GetType() < second->GetType();
    }

    wxString m1 = mm1->GetName();
    m1.LowerCase();
    wxString m2 = mm2->GetName();
    m2.LowerCase();

    return NumberAwareStringCompare(m1, m2) < 0;
}

void ControllerModelDialog::ReloadModels()
{
    _cud->Rescan(true);
    std::string check;
    if (_caps != nullptr) {
        _cud->Check(_caps, check);
    }

    while (_models.size() > 0) {
        delete _models.front();
        _models.pop_front();
    }

    while (_controllers.size() > 0) {
        delete _controllers.front();
        _controllers.pop_front();
    }

    FixDMXChannels();

    for (const auto& it : *_mm) {
        if (it.second->GetDisplayAs() != "ModelGroup") {
            if (_controller->ContainsChannels(it.second->GetFirstChannel(), it.second->GetLastChannel())) {
                auto shadows = it.second->GetShadowedBy();
                if (shadows.size() > 0) {
                    std::string sh;
                    for (const auto& it : shadows) {
                        if (sh != "")
                            sh += ", ";
                        sh += it;
                    }
                    check += "WARN: " + it.second->Name() + " is shadowed by " + sh + ".\n ";
                }
            }
        }
    }

    TextCtrl_Check->SetValue(check);

    for (const auto& it : *_mm) {
        if (it.second->GetDisplayAs() != "ModelGroup") {
            if (_cud->GetControllerPortModel(it.second->GetName(), 0) == nullptr &&
                ((_autoLayout && !CheckBox_HideOtherControllerModels->GetValue()) || // hide models on other controllers not set
                 ((_autoLayout && CheckBox_HideOtherControllerModels->GetValue() && (it.second->GetController() == nullptr || _controller->GetName() == it.second->GetControllerName() || it.second->GetControllerName() == "" || it.second->GetControllerName() == NO_CONTROLLER || _controller->ContainsChannels(it.second->GetFirstChannel(), it.second->GetLastChannel()))) ||
                  _controller->ContainsChannels(it.second->GetFirstChannel(), it.second->GetLastChannel())))) {
                _models.push_back(new ModelCMObject(nullptr, 0, it.second->GetName(), it.second->GetName(), _mm, _cud, _caps, wxPoint(5, 0), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_STRINGS, _scale));
            }
        }
    }

    _models.sort(ModelSortName);

    int y = TOP_BOTTOM_MARGIN;
    for (auto it : _models) {
        it->SetLocationY(y);
        y += VERTICAL_GAP + VERTICAL_SIZE;
    }

    _scale = (Slider_Box_Scale->GetValue() / 10.0);
    _modelsy = y;

    int panely = PanelModels->GetSize().y;
    ScrollBar_Models->SetRange(_modelsy);
    ScrollBar_Models->SetPageSize(_modelsy / 10);
    ScrollBar_Models->SetThumbSize(panely);
    PanelModels->Refresh();

    y = VERTICAL_GAP;

    // go through and mark any ports in blocks where at least one model is on a smart remote ... we need to leave more space on these ports for the SR visual element
    _cud->TagSmartRemotePorts();

    std::list<int> pixelPortsWithSmartRemotes;

    bool mergeConsecutiveVirtualStrings = _caps == nullptr ? true : _caps->MergeConsecutiveVirtualStrings();

    int maxx = 0;
    for (int i = 0; i < std::max((_caps == nullptr ? 0 : _caps->GetMaxPixelPort()), _cud->GetMaxPixelPort()); i++) {
        auto cmp = new PortCMObject(PortCMObject::PORTTYPE::PIXEL, i + 1, _cud, _caps, wxPoint(LEFT_RIGHT_MARGIN, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_PIXELS, i + 1 > (_caps == nullptr ? _cud->GetMaxPixelPort() : _caps->GetMaxPixelPort()), _scale);
        _controllers.push_back(cmp);

        auto pp = _cud->GetControllerPixelPort(i + 1);
        if (pp != nullptr) {
            int currentSmartRemote = 0;
            int smartRemoteCount = _cud->GetSmartRemoteCount(i + 1);

            if (!mergeConsecutiveVirtualStrings) {
                if (pp->IsSmartRemotePort()) {
                    y += SRY_GAP; // this adds the gap for FPP controllers
                }
            }

            if (_caps != nullptr && _caps->SupportsUniversePerString() && _controller->GetType() == CONTROLLER_ETHERNET) {
                pp->SetSeparateUniverses(((ControllerEthernet*)_controller)->IsUniversePerString());
            }

            if (_caps == nullptr || _caps->SupportsVirtualStrings()) {
                pp->CreateVirtualStrings(mergeConsecutiveVirtualStrings);
                if (pp->GetVirtualStringCount() == 0) {
                    y += VERTICAL_GAP + VERTICAL_SIZE;
                    if (mergeConsecutiveVirtualStrings) {
                        if (pp->IsSmartRemotePort()) {
                            y += SRY_GAP + SRYLABEL_SIZE;
                            for (int sr = 1; sr < smartRemoteCount; ++sr) {
                                y += VERTICAL_GAP + VERTICAL_SIZE + SRY_GAP + SRYLABEL_SIZE;
                            }
                        }
                    }
                } else {
                    int vs = 0;
                    int x = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + FIRST_MODEL_GAP_MULTIPLIER * HORIZONTAL_GAP;
                    for (const auto& it : pp->GetVirtualStrings()) {
                        for (const auto& it2 : it->_models) {
                            if (pp->IsSmartRemotePort() && it2->GetSmartRemote() > currentSmartRemote) {
                                if (mergeConsecutiveVirtualStrings) {
                                    if (currentSmartRemote > 0) {
                                        y += SRYLABEL_SIZE;
                                    }
                                    for (int i = currentSmartRemote; i < it2->GetSmartRemote(); ++i) {
                                        y += SRY_GAP;
                                        if (i != it2->GetSmartRemote() - 1) {
                                            y += SRYLABEL_SIZE;
                                        }
                                    }
                                }
                            }

                            if (it2->GetModel() != nullptr) {
                                if (it2->GetModel()->GetSmartRemote() != 0)
                                    pixelPortsWithSmartRemotes.push_back(i + 1);
                                auto cmm = new ModelCMObject(pp, vs, it2->GetModel()->GetName(), it2->GetName(), _mm, _cud, _caps, wxPoint(x, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_PIXELS, _scale);
                                _controllers.push_back(cmm);
                                x += HORIZONTAL_SIZE + HORIZONTAL_GAP;
                            }

                            currentSmartRemote = it2->GetSmartRemote();
                        }
                        if (x > maxx)
                            maxx = x;
                        if (mergeConsecutiveVirtualStrings) {
                            y += VERTICAL_GAP + VERTICAL_SIZE;
                            x = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + FIRST_MODEL_GAP_MULTIPLIER * HORIZONTAL_GAP;
                        }
                    }
                    if (!mergeConsecutiveVirtualStrings) {
                        y += VERTICAL_GAP + VERTICAL_SIZE;
                    } else {
                        if (pp->IsSmartRemotePort()) {
                            y += SRYLABEL_SIZE;
                        }
                        for (int sr = currentSmartRemote; sr < smartRemoteCount; ++sr) {
                            y += VERTICAL_GAP + VERTICAL_SIZE + SRY_GAP + SRYLABEL_SIZE;
                        }
                    }
                }
            } else {
                int x = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + FIRST_MODEL_GAP_MULTIPLIER * HORIZONTAL_GAP;
                for (const auto& it : pp->GetModels()) {
                    if (it->GetModel() != nullptr) {
                        auto cmm = new ModelCMObject(pp, 0, it->GetModel()->GetName(), it->GetName(), _mm, _cud, _caps, wxPoint(x, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_PIXELS, _scale);
                        _controllers.push_back(cmm);
                        x += HORIZONTAL_SIZE + HORIZONTAL_GAP;
                    }
                }
                if (x > maxx)
                    maxx = x;
                y += VERTICAL_GAP + VERTICAL_SIZE;
            }

            if (!mergeConsecutiveVirtualStrings) {
                if (pp->IsSmartRemotePort()) {
                    y += SRYLABEL_SIZE; // this adds the gap for FPP controllers
                }

                // Now add in the smart receivers for the port ... as they are all in a line things are easier
                for (int sr = 1; sr <= smartRemoteCount; ++sr) {
                    wxPoint start(999999, 999999);
                    wxSize size(0, 0);
                    int empty = pp->CountEmptySmartRemotesBefore(sr);
                    for (auto& it : _controllers) {
                        if (it->GetType() == "MODEL" && static_cast<ModelCMObject*>(it)->IsPortSR(i + 1, sr)) {
                            it->SetSROffset((sr - 1) * 2 * SRX_GAP /* - (sr > 1 ? SRX_GAP : 0) */ + empty * (HORIZONTAL_GAP), 0);
                            start = wxPoint(std::min(start.x, (int)(it->GetRect().x - SRX_GAP)), std::min(start.y, (int)(it->GetRect().y - SRY_GAP)));
                            size = wxSize(std::max((int)(it->GetRect().GetRight() - start.x + SRX_GAP), size.x), std::max((int)(it->GetRect().y + it->GetRect().height - start.y + SRY_GAP + SRYLABEL_SIZE - VERTICAL_GAP), size.y));
                        }
                    }

                    if (size == wxSize(0, 0)) {
                        // we need to do special things if there are no models
                        size = wxSize(2 * SRX_GAP, SRY_GAP + VERTICAL_SIZE + SRYLABEL_SIZE);

                        // need to find last end before this one
                        int x = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + FIRST_MODEL_GAP_MULTIPLIER * HORIZONTAL_GAP + (sr - 1) * (2 * SRX_GAP + HORIZONTAL_GAP) - SRX_GAP;
                        for (const auto& it : _controllers) {
                            if (it->GetType() == "MODEL") {
                                int msr = static_cast<ModelCMObject*>(it)->GetSmartRemote();
                                int mp = static_cast<ModelCMObject*>(it)->GetPort()->GetPort();
                                if (msr < sr && mp == pp->GetPort()) {
                                    x = std::max(x, (int)(it->GetRect().GetRight() + (sr - msr) * (2 * SRX_GAP + HORIZONTAL_GAP) - SRX_GAP));
                                }
                            }
                        }
                        start = wxPoint(x, cmp->GetRect().y);
                    }

                    auto csr = new SRCMObject(pp, sr, _cud, _caps, start, size, BaseCMObject::STYLE_PIXELS, _scale, _controller->GetVendor() == "HinksPix");
                    _controllers.push_back(csr);
                }
            } else {
                // Now add in the smart receivers for the port
                for (int sr = 1; sr <= smartRemoteCount; ++sr) {
                    wxPoint start(999999, 999999);
                    wxSize size(0, 0);
                    int porty = 0;
                    for (const auto& it : _controllers) {
                        if (it->GetType() == "PORT" && static_cast<PortCMObject*>(it)->GetPort() == i + 1) {
                            porty = it->GetRect().y;
                        }
                        if (it->GetType() == "MODEL" && static_cast<ModelCMObject*>(it)->IsPortSR(i + 1, sr)) {
                            start = wxPoint(std::min(start.x, (int)(it->GetRect().x - SRX_GAP)), std::min(start.y, (int)(it->GetRect().y - SRY_GAP)));
                            size = wxSize(std::max((int)(it->GetRect().GetRight() - start.x + SRX_GAP), size.x), std::max((int)(it->GetRect().y + it->GetRect().height - start.y + SRY_GAP + SRYLABEL_SIZE - VERTICAL_GAP), size.y));
                        }
                    }

                    if (size == wxSize(0, 0)) {
                        int x = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + FIRST_MODEL_GAP_MULTIPLIER * HORIZONTAL_GAP - SRX_GAP;
                        int y = porty;
                        int lastSR = 0;
                        for (const auto& it : pp->GetVirtualStrings()) {
                            if (it->_smartRemote == sr) {
                                break;
                            } else {
                                lastSR = it->_smartRemote;
                                y += VERTICAL_GAP + VERTICAL_SIZE;
                            }
                        }
                        for (int j = 1; j < sr; ++j) {
                            y += SRY_GAP + SRYLABEL_SIZE;
                        }
                        for (int j = lastSR + 1; j < sr; ++j) {
                            y += VERTICAL_GAP + VERTICAL_SIZE;
                        }
                        start = wxPoint(x, y);
                        size = wxSize(2 * SRX_GAP, SRY_GAP + SRYLABEL_SIZE + VERTICAL_SIZE);
                    }

                    auto csr = new SRCMObject(pp, sr, _cud, _caps, start, size, BaseCMObject::STYLE_PIXELS, _scale, _controller->GetVendor() == "HinksPix");
                    _controllers.push_back(csr);
                }
            }
        }
    }

    for (int i = 0; i < std::max((_caps == nullptr ? 0 : _caps->GetMaxSerialPort()), _cud->GetMaxSerialPort()); i++) {
        _controllers.push_back(new PortCMObject(PortCMObject::PORTTYPE::SERIAL, i + 1, _cud, _caps, wxPoint(LEFT_RIGHT_MARGIN, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_CHANNELS, i + 1 > (_caps == nullptr ? _cud->GetMaxSerialPort() : _caps->GetMaxSerialPort()), _scale));
        auto sp = _cud->GetControllerSerialPort(i + 1);
        if (sp != nullptr) {
            int x = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + FIRST_MODEL_GAP_MULTIPLIER * HORIZONTAL_GAP;
            for (const auto& it : sp->GetModels()) {
                auto cmm = new ModelCMObject(sp, 0, it->GetName(), it->GetName(), _mm, _cud, _caps, wxPoint(x, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_CHANNELS, _scale);
                _controllers.push_back(cmm);
                x += HORIZONTAL_SIZE + HORIZONTAL_GAP;
            }
            if (x > maxx)
                maxx = x;
        }
        y += VERTICAL_GAP + VERTICAL_SIZE;
    }
    for (int i = 0; i < std::max((_caps == nullptr ? 0 : _caps->GetMaxVirtualMatrixPort()), _cud->GetMaxVirtualMatrixPort()); i++) {
        _controllers.push_back(new PortCMObject(PortCMObject::PORTTYPE::VIRTUAL_MATRIX, i + 1, _cud, _caps,
                                                wxPoint(LEFT_RIGHT_MARGIN, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE),
                                                BaseCMObject::STYLE_CHANNELS, false, _scale));
        auto sp = _cud->GetControllerVirtualMatrixPort(i + 1);
        if (sp != nullptr) {
            int x = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + FIRST_MODEL_GAP_MULTIPLIER * HORIZONTAL_GAP;
            for (const auto& it : sp->GetModels()) {
                auto cmm = new ModelCMObject(sp, 0, it->GetName(), it->GetName(), _mm, _cud, _caps, wxPoint(x, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE),
                                             BaseCMObject::STYLE_CHANNELS, _scale);
                _controllers.push_back(cmm);
                x += HORIZONTAL_SIZE + HORIZONTAL_GAP;
            }
            if (x > maxx)
                maxx = x;
        }
        y += VERTICAL_GAP + VERTICAL_SIZE;
    }

    if (_caps != nullptr && _caps->SupportsLEDPanelMatrix()) {
        _controllers.push_back(new PortCMObject(PortCMObject::PORTTYPE::PANEL_MATRIX, 1, _cud, _caps,
                                                wxPoint(LEFT_RIGHT_MARGIN, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE),
                                                BaseCMObject::STYLE_CHANNELS, false, _scale));
        auto sp = _cud->GetControllerLEDPanelMatrixPort(1);
        if (sp != nullptr) {
            int x = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + FIRST_MODEL_GAP_MULTIPLIER * HORIZONTAL_GAP;
            for (const auto& it : sp->GetModels()) {
                auto cmm = new ModelCMObject(sp, 0, it->GetName(), it->GetName(), _mm, _cud, _caps, wxPoint(x, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE),
                                             BaseCMObject::STYLE_CHANNELS, _scale);
                _controllers.push_back(cmm);
                x += HORIZONTAL_SIZE + HORIZONTAL_GAP;
            }
            if (x > maxx)
                maxx = x;
        }
        y += VERTICAL_GAP + VERTICAL_SIZE;
    }

    y -= VERTICAL_GAP;
    y += TOP_BOTTOM_MARGIN;
    maxx -= HORIZONTAL_GAP;
    maxx += LEFT_RIGHT_MARGIN;

    _controllersx = maxx;
    _controllersy = y;

    int panelx = PanelController->GetSize().x;
    ScrollBar_Controller_H->SetRange(_controllersx);
    ScrollBar_Controller_H->SetPageSize(_controllersx / 10);
    ScrollBar_Controller_H->SetThumbSize(panelx);

    panely = PanelController->GetSize().y;
    ScrollBar_Controller_V->SetRange(_controllersy);
    ScrollBar_Controller_V->SetPageSize(_controllersy / 10);
    ScrollBar_Controller_V->SetThumbSize(panely);

    // scan through all ports if any models are on smart remotes then all must be
    auto last = std::unique(pixelPortsWithSmartRemotes.begin(), pixelPortsWithSmartRemotes.end());
    pixelPortsWithSmartRemotes.erase(last, pixelPortsWithSmartRemotes.end());

    for (auto p : pixelPortsWithSmartRemotes) {
        for (const auto& it : _controllers) {
            auto m = dynamic_cast<ModelCMObject*>(it);
            if (m != nullptr && m->GetModel() != nullptr &&
                m->GetModel()->IsPixelProtocol() &&
                m->GetModel()->GetControllerPort() == p &&
                m->GetModel()->GetSmartRemote() == 0) {
                m->SetInvalid(true);
            }
        }
    }

    PanelController->Refresh();
    PanelModels->Refresh();
}

void ControllerModelDialog::OnPopupCommand(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int id = event.GetId();
    if (id == CONTROLLERModel_PRINT) {
        PrintScreen();
    } else if (id == CONTROLLERModel_SAVE_CSV) {
        SaveCSV();
    } else if (id == CONTROLLER_REMOVEALLMODELS) {
        if (wxMessageBox("Are you sure You want to remove all the models from this Controller?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
            return;
        }
        logger_base.debug("removing all models from controller.");
        for (const auto& it : _controllers) {
            ModelCMObject* m = dynamic_cast<ModelCMObject*>(it);
            if (m != nullptr && m->GetModel() != nullptr) {
                // Removing a model from the controller
                if (_autoLayout) {
                    m->GetModel()->SetModelChain("");
                    if (m->GetModel()->GetControllerName() != "") {
                        m->GetModel()->SetControllerName(NO_CONTROLLER);
                    }
                }
                m->GetModel()->SetControllerPort(0);
            }
        }
        ReloadModels();
    } else if (_popup != nullptr) {
        if (_popup->HandlePopup(this, event, id)) {
            while (!_xLights->DoAllWork()) {
                // dont get into a redraw loop from here
                _xLights->GetOutputModelManager()->RemoveWork("ASAP", OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW);
            }
            ReloadModels();
        }
    }
}

void ControllerModelDialog::PrintScreen()
{
    int panX = 0;
    int panY = 0;
    for (const auto& it : _controllers) {
        if ((it->GetRect().GetY() + it->GetRect().GetHeight()) > panY) {
            panY = it->GetRect().GetY() + it->GetRect().GetHeight() + 5;
        }

        if ((it->GetRect().GetX() + it->GetRect().GetWidth()) > panX) {
            panX = it->GetRect().GetY() + it->GetRect().GetHeight() + 5;
        }
    }

    ControllerModelPrintout printout(this, _title, wxSize(HORIZONTAL_SIZE + HORIZONTAL_GAP, VERTICAL_SIZE + VERTICAL_GAP), wxSize(panX, panY));
    printout.preparePrint(true);
    wxPrintDialogData printDialogData(printout.getPrintData());
    wxPrinter printer(&printDialogData);

    if (!printer.Print(this, &printout, true)) {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
            DisplayError(wxString::Format("Problem printing controller layout. %d", wxPrinter::GetLastError()).ToStdString(), this);
        }
    } else {
        printDialogData = printer.GetPrintDialogData();
    }
}

wxBitmap ControllerModelDialog::RenderPicture(int startY, int startX, int width, int height, wxString const& pageName)
{
    ::SetColours(true);

    wxBitmap bitmap;

    float maxx = width * 1.1;
    float maxy = height * 1.1;

    bitmap.Create(maxx, maxy);

    wxMemoryDC dc;
    dc.SelectObject(bitmap);

    dc.SetTextForeground(__textForeground);

    dc.SetPen(*wxWHITE_PEN);
    dc.SetBrush(*wxWHITE_BRUSH);

    dc.DrawRectangle(wxPoint(0, 0), bitmap.GetSize());

    dc.SetDeviceOrigin(0, 0);

    wxFont font = wxFont(wxSize(0, getFontSize()), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("Arial"), wxFONTENCODING_DEFAULT);
    dc.SetFont(font);

    int rowPos = TOP_BOTTOM_MARGIN;
    dc.DrawText(wxString::Format("%s %s", _title, pageName), LEFT_RIGHT_MARGIN, rowPos);
    rowPos += ((VERTICAL_SIZE / 2)) + (VERTICAL_GAP);

    int endY = startY + height;
    int endX = startX + width;

    // draw ports and smart receivers first
    for (const auto& it : _controllers) {
        if (it->GetType() != "MODEL") {
            if (it->GetRect().GetY() > startY && it->GetRect().GetY() < endY &&
                it->GetRect().GetX() > startX && it->GetRect().GetX() < endX) {
                it->Draw(dc, 0, wxPoint(0, 0), wxPoint(0, 0), wxSize(-startX, rowPos - startY), 1, true, true, nullptr);
            }
        }
    }

    for (const auto& it : _controllers) {
        if (it->GetType() == "MODEL") {
            if (it->GetRect().GetY() > startY && it->GetRect().GetY() < endY &&
                it->GetRect().GetX() > startX && it->GetRect().GetX() < endX) {
                it->Draw(dc, 0, wxPoint(0, 0), wxPoint(0, 0), wxSize(-startX, rowPos - startY), 1, true, true, nullptr);
            }
        }
    }

    ::SetColours(false);

    return bitmap;
}

void ControllerModelDialog::SaveCSV()
{
    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written

    wxString cleanName = _controller->GetShortDescription();
    cleanName.Replace(".", "_");
    cleanName += ".csv";
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, cleanName, wxEmptyString, "Export files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty())
        return;

    wxFile f(filename);

    if (!f.Create(filename, true) || !f.IsOpened()) {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
        return;
    }

    wxString const header = _controller->GetShortDescription() + "\n";
    f.Write(header);
    int columSize = 0;
    std::vector<std::vector<std::string>> const lines = _cud->ExportAsCSV(ExportSettings::GetSettings(this), _controller->GetDefaultBrightnessUnderFullControl(), columSize);
    for (const auto& line : lines) {
        for (const auto& col : line) {
            f.Write(col);
            f.Write(',');
        }
        f.Write('\n');
    }
    f.Close();
}

// Ensure DMX channels increase as you move left to right
void ControllerModelDialog::FixDMXChannels()
{
    bool changed = false;
    for (int i = 0; i < _cud->GetMaxSerialPort(); i++) {
        int next = -1;
        auto p = _cud->GetControllerSerialPort(i + 1);
        for (const auto& it : p->GetModels()) {
            if (next == -1) {
                next = it->GetModel()->GetControllerDMXChannel();
                if (next == 0)
                    next = 1;
                next += it->GetModel()->GetChanCount();
            } else {
                if (it->GetModel()->GetControllerDMXChannel() < next) {
                    changed = true;
                    it->GetModel()->SetControllerDMXChannel(next);
                }
                next = it->GetModel()->GetControllerDMXChannel() + it->GetModel()->GetChanCount();
            }
        }
    }

    while (!_xLights->DoAllWork()) {
        // dont get into a redraw loop from here
        _xLights->GetOutputModelManager()->RemoveWork("ASAP", OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW);
    }

    // if we changed anything we need to scan it one more time
    if (changed) {
        _cud->Rescan(true);
    }
}

PortCMObject* ControllerModelDialog::GetPortContainingModel(Model* m)
{
    PortCMObject* lastPort = nullptr;
    for (const auto& it : _controllers) {
        auto mm = dynamic_cast<ModelCMObject*>(it);
        if (mm != nullptr && mm->GetModel() == m) {
            return lastPort;
        } else {
            auto p = dynamic_cast<PortCMObject*>(it);
            if (p != nullptr)
                lastPort = p;
        }
    }
    return nullptr;
}

ModelCMObject* ControllerModelDialog::GetModelsCMObject(Model* m)
{
    for (const auto& it : _controllers) {
        auto mm = dynamic_cast<ModelCMObject*>(it);
        if (mm != nullptr && mm->GetModel() == m) {
            return mm;
        }
    }
    return nullptr;
}

void ControllerModelDialog::DropModelFromModelsPaneOnModel(ModelCMObject* dropped, Model* on, bool rhs)
{
    if (dropped == nullptr)
        return;

    auto m = dropped->GetModel();
    if (m == nullptr)
        return;

    auto port = GetPortContainingModel(on);
    wxASSERT(port != nullptr);
    if (port == nullptr)
        return;

    m->SetControllerPort(port->GetPort());
    if (port->GetPortType() == PortCMObject::PORTTYPE::PIXEL) {
        if (port->GetModelCount() == 0) {
            if (_caps != nullptr && !_caps->IsValidPixelProtocol(m->GetControllerProtocol()) && _caps->GetPixelProtocols().size() > 0)
                m->SetControllerProtocol(_caps->GetPixelProtocols().front());
            if (!m->IsPixelProtocol())
                m->SetControllerProtocol("ws2811");
        } else {
            m->SetControllerProtocol(on->GetControllerProtocol());
        }
        if (_caps != nullptr) {
            if (!_caps->SupportsSmartRemotes()) {
                m->SetSmartRemote(0);
            } else {
                if (port->GetUDPort()->AtLeastOneModelIsUsingSmartRemote()) {
                    m->SetSmartRemote(1);
                }
            }
        }
    } else if (port->GetPortType() == PortCMObject::PORTTYPE::VIRTUAL_MATRIX) {
        m->SetControllerProtocol("Virtual Matrix");
        m->SetSmartRemote(0);
    } else if (port->GetPortType() == PortCMObject::PORTTYPE::PANEL_MATRIX) {
        m->SetControllerProtocol("LED Panel Matrix");
        m->SetSmartRemote(0);
    } else {
        if (port->GetModelCount() == 0) {
            if (_caps != nullptr && !_caps->IsValidSerialProtocol(m->GetControllerProtocol()) && _caps->GetSerialProtocols().size() > 0)
                m->SetControllerProtocol(_caps->GetSerialProtocols().front());
            if (!m->IsSerialProtocol())
                m->SetControllerProtocol("dmx");
            if (m->GetControllerDMXChannel() == 0)
                m->SetControllerDMXChannel(1);
        } else {
            m->SetControllerProtocol(on->GetControllerProtocol());
        }
    }

    if (_autoLayout) {
        m->SetControllerName(_controller->GetName());
    }

    // dropped on a model
    if (_autoLayout && on != nullptr) {
        m->SetSmartRemote(on->GetSmartRemote());
        if (!rhs) {
            if (port->GetPortType() == PortCMObject::PORTTYPE::SERIAL) {
                m->SetModelChain("");
                on->SetModelChain("");
                Model* last = port->GetUDPort()->GetModelBefore(on);
                int nextch = on->GetControllerDMXChannel() - m->GetChanCount();
                if (nextch < 1)
                    nextch = 1;
                if (last != nullptr) {
                    if (last->GetControllerDMXChannel() + last->GetChanCount() > (size_t)nextch) {
                        nextch = last->GetControllerDMXChannel() + last->GetChanCount();
                    }
                }
                if (m->GetControllerDMXChannel() < nextch)
                    m->SetControllerDMXChannel(nextch);
                Model* next = on;
                last = m;
                while (next != nullptr) {
                    nextch = last->GetControllerDMXChannel() + last->GetChanCount();
                    if (next->GetControllerDMXChannel() < nextch)
                        next->SetControllerDMXChannel(nextch);
                    last = next;
                    next = port->GetUDPort()->GetModelAfter(last);
                }
            } else {
                m->SetModelChain(on->GetModelChain());
                on->SetModelChain(">" + m->GetName());
            }
        } else {
            Model* next = port->GetUDPort()->GetModelAfter(on);
            if (next != nullptr) {
                next->SetModelChain(">" + m->GetName());
            }
            m->SetModelChain(">" + on->GetName());

            if (port->GetPortType() == PortCMObject::PORTTYPE::SERIAL) {
                m->SetModelChain("");
                on->SetModelChain("");
                if (next != nullptr)
                    next->SetModelChain("");
                int nextch = on->GetControllerDMXChannel() + on->GetChanCount();
                m->SetControllerDMXChannel(nextch);
                Model* last = m;
                while (next != nullptr) {
                    nextch = last->GetControllerDMXChannel() + last->GetChanCount();
                    if (next->GetControllerDMXChannel() < nextch)
                        next->SetControllerDMXChannel(nextch);
                    last = next;
                    next = port->GetUDPort()->GetModelAfter(last);
                }
            }
        }
    }

    _lastDropped = dropped->GetModel();
}

void ControllerModelDialog::DropFromModels(const wxPoint& location, const std::string& name, wxPanel* target)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    auto m = _mm->GetModel(name);
    if (m == nullptr) {
        logger_base.debug("Model '%s' was dropped from models but it could not be found.", (const char*)name.c_str());
        _lastDropped = nullptr;
        ClearOver(PanelController, _controllers);
        return;
    }

    logger_base.debug("Model '%s' dropped from models pane.", (const char*)name.c_str());

    // model dragged from models
    if (target == PanelModels) {
        // do nothing
        logger_base.debug("   onto the models pane ... nothing to do.");
    } else {
        logger_base.debug("    onto the controller pane.");

        auto port = GetControllerPortAtLocation(location);
        auto sr = GetControllerSRAtLocation();
        if (port != nullptr) {
            logger_base.debug("    onto port %d.", port->GetPort());
            m->SetControllerPort(port->GetPort());
            if (port->GetPortType() == PortCMObject::PORTTYPE::PIXEL) {
                if (port->GetModelCount() == 0) {
                    if (_caps != nullptr && !_caps->IsValidPixelProtocol(m->GetControllerProtocol()) && _caps->GetPixelProtocols().size() > 0) {
                        // try to find a compatible protocol
                        auto np = ChooseBestControllerPixel(_caps->GetPixelProtocols(), m->GetControllerProtocol());
                        if (np != "") {
                            m->SetControllerProtocol(np);
                        } else {
                            m->SetControllerProtocol(_caps->GetPixelProtocols().front());
                        }
                    }
                    if (!m->IsPixelProtocol())
                        m->SetControllerProtocol("ws2811");
                } else {
                    m->SetControllerProtocol(port->GetFirstModel()->GetControllerProtocol());
                }
                if (sr != nullptr) {
                    m->SetSmartRemote(sr->GetSmartRemote());
                } else {
                    if (_caps != nullptr) {
                        if (!_caps->SupportsSmartRemotes()) {
                            m->SetSmartRemote(0);
                        } else {
                            if (port->GetUDPort()->AtLeastOneModelIsUsingSmartRemote()) {
                                m->SetSmartRemote(1);
                            }
                        }
                    }
                }
            } else if (port->GetPortType() == PortCMObject::PORTTYPE::VIRTUAL_MATRIX) {
                m->SetControllerProtocol("Virtual Matrix");
                m->SetSmartRemote(0);
            } else if (port->GetPortType() == PortCMObject::PORTTYPE::PANEL_MATRIX) {
                m->SetControllerProtocol("LED Panel Matrix");
                m->SetSmartRemote(0);
            } else {
                if (port->GetModelCount() == 0) {
                    if (_caps != nullptr && !_caps->IsValidSerialProtocol(m->GetControllerProtocol()) && _caps->GetSerialProtocols().size() > 0) {
                        // try to find a compatible protocol
                        auto np = ChooseBestControllerSerial(_caps->GetSerialProtocols(), m->GetControllerProtocol());
                        if (np != "") {
                            m->SetControllerProtocol(np);
                        } else {
                            m->SetControllerProtocol(_caps->GetSerialProtocols().front());
                        }
                    }
                    if (!m->IsSerialProtocol())
                        m->SetControllerProtocol("dmx");
                    if (m->GetControllerDMXChannel() == 0)
                        m->SetControllerDMXChannel(1);
                } else {
                    m->SetControllerProtocol(port->GetFirstModel()->GetControllerProtocol());
                }
            }

            if (_autoLayout) {
                m->SetControllerName(_controller->GetName());
            }

            auto ob = GetControllerToDropOn();
            ModelCMObject* mob = dynamic_cast<ModelCMObject*>(ob);
            BaseCMObject::HITLOCATION hit = BaseCMObject::HITLOCATION::NONE;
            if (ob != nullptr)
                hit = ob->HitTest(location);

            if (mob == nullptr || !mob->IsMain()) {
                if (sr != nullptr) {
                    // dropped on a sr
                    logger_base.debug("    Processing it as a drop onto the SR ... so setting it to end.");

                    if (_autoLayout) {
                        UDControllerPortModel* lastModel = nullptr;
                        for (const auto& it : port->GetUDPort()->GetModels()) {
                            if (it->GetSmartRemote() == sr->GetSmartRemote()) {
                                lastModel = it;
                            }
                        }
                        if (lastModel != nullptr) {
                            m->SetModelChain(">" + lastModel->GetName());
                        }
                    }
                } else {
                    logger_base.debug("    Processing it as a drop onto the port ... so setting it to end.");

                    // dropped on a port .. or not on the first string of a model
                    // If no model already there put it at the beginning ... else chain it to end
                    if (_autoLayout) {
                        auto fmud = port->GetUDPort()->GetLastModel();
                        if (fmud != nullptr && fmud->IsFirstModelString()) {
                            Model* lm = fmud->GetModel();
                            if (lm != nullptr) {
                                if (port->GetPortType() == PortCMObject::PORTTYPE::SERIAL) {
                                    m->SetModelChain("");
                                    int nextch = lm->GetControllerDMXChannel() + lm->GetChanCount();
                                    if (m->GetControllerDMXChannel() < nextch)
                                        m->SetControllerDMXChannel(nextch);
                                } else {
                                    m->SetModelChain(">" + lm->GetName());
                                    if (port->GetUDPort()->IsSmartRemotePort()) {
                                        m->SetSmartRemote(port->GetSmartRemoteCount());
                                    }
                                }
                            }
                        } else {
                            m->SetModelChain("");
                            if (m->GetControllerDMXChannel() == 0)
                                m->SetControllerDMXChannel(1);
                        }
                    }
                }
            } else {
                Model* droppedOn = _mm->GetModel(mob->GetName());
                logger_base.debug("    Dropped onto model %s.", (const char*)droppedOn->GetName().c_str());

                // dropped on a model
                if (_autoLayout) {
                    m->SetSmartRemote(droppedOn->GetSmartRemote());
                    if (hit == BaseCMObject::HITLOCATION::LEFT) {
                        logger_base.debug("    On the left hand side.");
                        logger_base.debug("    Left of %s which comes after %s.", (const char*)droppedOn->GetName().c_str(), (const char*)droppedOn->GetModelChain().c_str());
                        if (port->GetPortType() == PortCMObject::PORTTYPE::SERIAL) {
                            m->SetModelChain("");
                            droppedOn->SetModelChain("");
                            Model* last = port->GetUDPort()->GetModelBefore(droppedOn);
                            int nextch = droppedOn->GetControllerDMXChannel() - m->GetChanCount();
                            if (nextch < 1)
                                nextch = 1;
                            if (last != nullptr) {
                                if (last->GetControllerDMXChannel() + last->GetChanCount() > (size_t)nextch) {
                                    nextch = last->GetControllerDMXChannel() + last->GetChanCount();
                                }
                            }
                            if (m->GetControllerDMXChannel() < nextch)
                                m->SetControllerDMXChannel(nextch);
                            Model* next = droppedOn;
                            last = m;
                            while (next != nullptr) {
                                nextch = last->GetControllerDMXChannel() + last->GetChanCount();
                                if (next->GetControllerDMXChannel() < nextch)
                                    next->SetControllerDMXChannel(nextch);
                                last = next;
                                next = port->GetUDPort()->GetModelAfter(last);
                            }
                        } else {
                            m->SetModelChain(droppedOn->GetModelChain());
                            droppedOn->SetModelChain(">" + m->GetName());
                        }
                    } else {
                        logger_base.debug("    On the right hand side.");
                        Model* next = port->GetUDPort()->GetModelAfter(droppedOn);
                        if (next != nullptr) {
                            logger_base.debug("    Right of %s which comes before %s.", (const char*)droppedOn->GetName().c_str(), (const char*)next->GetName().c_str());
                            next->SetModelChain(">" + m->GetName());
                        } else {
                            logger_base.debug("    Right of %s.", (const char*)droppedOn->GetName().c_str());
                        }
                        m->SetModelChain(">" + droppedOn->GetName());

                        if (port->GetPortType() == PortCMObject::PORTTYPE::SERIAL) {
                            m->SetModelChain("");
                            droppedOn->SetModelChain("");
                            if (next != nullptr)
                                next->SetModelChain("");
                            int nextch = droppedOn->GetControllerDMXChannel() + droppedOn->GetChanCount();
                            m->SetControllerDMXChannel(nextch);
                            Model* last = m;
                            while (next != nullptr) {
                                nextch = last->GetControllerDMXChannel() + last->GetChanCount();
                                if (next->GetControllerDMXChannel() < nextch)
                                    next->SetControllerDMXChannel(nextch);
                                last = next;
                                next = port->GetUDPort()->GetModelAfter(last);
                            }
                        }
                    }
                }
            }
            _lastDropped = m;
        } else {
            logger_base.debug("    but not onto a port ... so nothing to do.");
            _lastDropped = nullptr;
        }

        ClearOver(PanelController, _controllers);

        while (!_xLights->DoAllWork()) {
            // dont get into a redraw loop from here
            _xLights->GetOutputModelManager()->RemoveWork("ASAP", OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW);
        }
        ReloadModels();
    }
}

bool CloseTo(const wxPoint& p1, const wxPoint& p2)
{
    return (std::abs(p1.x - p2.x) < 3) && (std::abs(p1.y - p2.y) < 3);
}

void ControllerModelDialog::DropFromController(const wxPoint& location, const std::string& name, wxPanel* target)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (StartsWith(name, "SR:")) {
        // this is a SR being dropped

        if (target == PanelModels) {
            // we dont do anything
            _lastDropped = nullptr;
        } else {
            // dragging a SR somewhere

            auto port = GetControllerPortAtLocation(location);
            auto sr = GetControllerSRAtLocation();
            auto dr = dynamic_cast<SRCMObject*>(GetDragging());
            int basePortFrom = dr->GetBasePort();
            int srFrom = dr->GetSmartRemote();
            int basePortTo = -1;
            int srTo = -1;

            if (sr != nullptr && !sr->IsSameRemote(GetDragging())) {
                // dropped on another SR

                // move all these models to these ports and remotes
                basePortTo = sr->GetBasePort();
                srTo = sr->GetSmartRemote();

            } else if (port != nullptr && port->GetBasePort() != dr->GetBasePort()) {
                // dropped on a port
                basePortTo = port->GetBasePort();

                // if port has no SR with this name or it is empty then just put all these models on these ports
                if (port->GetModelCount(srFrom) == 0) {
                    srTo = dr->GetSmartRemote();
                }
                // else add this remote after the last remote on this port
                else {
                    if (_caps != nullptr && port->GetSmartRemoteCount() + 1 <= _caps->GetSmartRemoteCount()) {
                        srTo = port->GetSmartRemoteCount() + 1;
                    }
                }
            }

            if (basePortTo != -1 && srTo != -1) {
                // move the models
                for (uint8_t i = 0; i < 4; ++i) {
                    dr->SetAllModelsToReceiver(basePortFrom + i, basePortTo + i, srFrom, srTo);
                }
            }

            if (sr != nullptr)
                sr->SetOver(BaseCMObject::HITLOCATION::NONE);
            if (port != nullptr)
                port->SetOver(BaseCMObject::HITLOCATION::NONE);

            while (!_xLights->DoAllWork()) {
                // dont get into a redraw loop from here
                _xLights->GetOutputModelManager()->RemoveWork("ASAP", OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW);
            }
            ReloadModels();
        }

        _dragStartLocation = wxPoint(-99999, -99999);

        ClearOver(PanelController, _controllers);

        return;
    }

    logger_base.debug("Model %s dropped from controllers pane.", (const char*)name.c_str());

    // model dragged from controllers
    auto m = _mm->GetModel(name);
    if (m == nullptr) {
        _lastDropped = nullptr;
        _dragStartLocation = wxPoint(-99999, -99999);

        ClearOver(PanelController, _controllers);

        return;
    }

    if (target == PanelModels) {
        logger_base.debug("   onto the models pane ... so remove the model from the controller.");
        // Removing a model from the controller
        if (_autoLayout) {
            // get the model after this model
            Model* nextFrom = _cud->GetModelAfter(m);
            if (nextFrom != nullptr) {
                nextFrom->SetModelChain(m->GetModelChain());
            }

            if (m->GetControllerName() != "") {
                m->SetControllerName(NO_CONTROLLER);
            }
            m->SetModelChain("");
        }
        m->SetControllerPort(0);
        _lastDropped = nullptr;
    } else {
        // we only do something if the model was actually moved
        if (!CloseTo(location, _dragStartLocation)) {
            logger_base.debug("    onto the controller pane.");

            auto port = GetControllerPortAtLocation(location);
            auto sr = GetControllerSRAtLocation();
            if (port != nullptr) {
                logger_base.debug("    onto port %d.", port->GetPort());
                m->SetControllerPort(port->GetPort());
                if (sr != nullptr) {
                    m->SetSmartRemote(sr->GetSmartRemote());
                }
                if (port->GetPortType() == PortCMObject::PORTTYPE::PIXEL) {
                    if (port->GetModelCount() == 0) {
                        if (_caps != nullptr && !_caps->IsValidPixelProtocol(m->GetControllerProtocol()) && _caps->GetPixelProtocols().size() > 0) {
                            auto np = ChooseBestControllerPixel(_caps->GetPixelProtocols(), m->GetControllerProtocol());
                            if (np != "") {
                                m->SetControllerProtocol(np);
                            } else {
                                m->SetControllerProtocol(_caps->GetPixelProtocols().front());
                            }
                        }
                        if (!m->IsPixelProtocol())
                            m->SetControllerProtocol("ws2811");
                    } else {
                        m->SetControllerProtocol(port->GetFirstModel()->GetControllerProtocol());
                    }
                } else if (port->GetPortType() == PortCMObject::PORTTYPE::VIRTUAL_MATRIX) {
                    m->SetControllerProtocol("Virtual Matrix");
                    m->SetSmartRemote(0);
                } else if (port->GetPortType() == PortCMObject::PORTTYPE::PANEL_MATRIX) {
                    m->SetControllerProtocol("LED Panel Matrix");
                    m->SetSmartRemote(0);
                } else {
                    if (port->GetModelCount() == 0) {
                        if (_caps != nullptr && !_caps->IsValidSerialProtocol(m->GetControllerProtocol()) && _caps->GetSerialProtocols().size() > 0) {
                            auto np = ChooseBestControllerSerial(_caps->GetSerialProtocols(), m->GetControllerProtocol());
                            if (np != "") {
                                m->SetControllerProtocol(np);
                            } else {
                                m->SetControllerProtocol(_caps->GetSerialProtocols().front());
                            }
                        }
                        if (!m->IsSerialProtocol())
                            m->SetControllerProtocol("dmx");
                    } else {
                        if (port->GetModelCount() == 0) {
                            if (_caps != nullptr && !_caps->IsValidSerialProtocol(m->GetControllerProtocol()) && _caps->GetSerialProtocols().size() > 0)
                                m->SetControllerProtocol(_caps->GetSerialProtocols().front());
                            if (!m->IsSerialProtocol())
                                m->SetControllerProtocol("dmx");
                        } else {
                            m->SetControllerProtocol(port->GetFirstModel()->GetControllerProtocol());
                        }
                    }
                }
                if (_autoLayout) {
                    m->SetControllerName(_controller->GetName());
                }

                auto ob = GetControllerToDropOn();
                ModelCMObject* mob = dynamic_cast<ModelCMObject*>(ob);
                BaseCMObject::HITLOCATION hit = BaseCMObject::HITLOCATION::NONE;
                if (ob != nullptr)
                    hit = ob->HitTest(location);

                if (mob != nullptr && mob->GetModel() == m) {
                    // dropped onto ourselves ... nothing to do
                } else if (mob == nullptr || !mob->IsMain()) {
                    logger_base.debug("    Processing it as a drop onto the port ... so setting it to beginning.");

                    // dropped on a port .. or not on the first string of a model
                    // If no model already there put it at the beginning ... else chain it
                    if (_autoLayout) {
                        UDControllerPort* cudp = port->GetUDPort();

                        // Because we are moving a model already in a chain we need to patch that over first
                        Model* nextfrom = _cud->GetModelAfter(m);
                        if (nextfrom != nullptr) {
                            logger_base.debug("    Model %s was removed from existing chain so %s now chains to %s", (const char*)m->GetName().c_str(), (const char*)nextfrom->GetName().c_str(), (const char*)m->GetModelChain().c_str());
                            nextfrom->SetModelChain(m->GetModelChain());
                        }

                        auto fmud = cudp->GetLastModel();
                        if (fmud == nullptr) {
                            m->SetModelChain("");
                            if (port->GetPortType() == PortCMObject::PORTTYPE::SERIAL) {
                                m->SetControllerDMXChannel(1);
                            }
                        } else {
                            if (fmud != nullptr && fmud->IsFirstModelString()) {
                                Model* lm = fmud->GetModel();
                                if (lm != nullptr) {
                                    m->SetModelChain(">" + lm->GetName());
                                }
                            }
                            if (port->GetPortType() == PortCMObject::PORTTYPE::SERIAL) {
                                m->SetModelChain("");
                                Model* last = port->GetUDPort()->GetLastModel()->GetModel();
                                if (last == m) {
                                    last = port->GetUDPort()->GetModelBefore(last);
                                }
                                if (last == nullptr) {
                                    // nothing to do
                                } else {
                                    int nextch = last->GetControllerDMXChannel() + last->GetChanCount();
                                    if (m->GetControllerDMXChannel() < nextch) {
                                        m->SetControllerDMXChannel(nextch);
                                    }
                                }
                            }
                        }
                    }
                    _lastDropped = m;
                } else {
                    Model* droppedOn = _mm->GetModel(mob->GetName());
                    logger_base.debug("    Dropped onto model %s.", (const char*)droppedOn->GetName().c_str());

                    // dropped on a model
                    if (_autoLayout) {
                        m->SetSmartRemote(droppedOn->GetSmartRemote());
                        // Because we are moving a model already in a chain we need to patch that over first
                        Model* nextfrom = _cud->GetModelAfter(m);
                        if (nextfrom != nullptr) {
                            if ((nextfrom == droppedOn && hit == BaseCMObject::HITLOCATION::LEFT) ||
                                (">" + droppedOn->GetName() == m->GetModelChain())) {
                                logger_base.debug("    Model did not actually move.");
                            } else {
                                logger_base.debug("    Model %s was removed from existing chain so %s now chains to %s", (const char*)m->GetName().c_str(), (const char*)nextfrom->GetName().c_str(), (const char*)m->GetModelChain().c_str());
                                nextfrom->SetModelChain(m->GetModelChain());
                            }
                        }

                        if (hit == BaseCMObject::HITLOCATION::LEFT) {
                            logger_base.debug("    On the left hand side.");
                            logger_base.debug("    Left of %s which comes after %s.", (const char*)droppedOn->GetName().c_str(), (const char*)droppedOn->GetModelChain().c_str());
                            if (droppedOn->GetModelChain() != ">" + m->GetName()) {
                                m->SetModelChain(droppedOn->GetModelChain());
                                droppedOn->SetModelChain(">" + m->GetName());
                            }
                            if (port->GetPortType() == PortCMObject::PORTTYPE::SERIAL) {
                                m->SetModelChain("");
                                droppedOn->SetModelChain("");
                                Model* last = port->GetUDPort()->GetModelBefore(droppedOn);
                                int nextch = droppedOn->GetControllerDMXChannel() - m->GetChanCount();
                                if (nextch < 1)
                                    nextch = 1;
                                if (last != nullptr) {
                                    if (last->GetControllerDMXChannel() + last->GetChanCount() > (size_t)nextch) {
                                        nextch = last->GetControllerDMXChannel() + last->GetChanCount();
                                    }
                                }
                                if (m->GetControllerDMXChannel() < nextch) {
                                    m->SetControllerDMXChannel(nextch);
                                } else if (droppedOn->GetControllerDMXChannel() - (int)m->GetChanCount() < m->GetControllerDMXChannel()) {
                                    m->SetControllerDMXChannel(nextch);
                                }
                                Model* next = droppedOn;
                                last = m;
                                while (next != nullptr) {
                                    nextch = last->GetControllerDMXChannel() + last->GetChanCount();
                                    if (next != m && next->GetControllerDMXChannel() < nextch)
                                        next->SetControllerDMXChannel(nextch);
                                    last = next;
                                    next = port->GetUDPort()->GetModelAfter(last);
                                }
                            }
                        } else {
                            logger_base.debug("    On the right hand side.");
                            if (m->GetModelChain() != ">" + droppedOn->GetName()) {
                                Model* next = port->GetUDPort()->GetModelAfter(droppedOn);
                                if (next != nullptr) {
                                    logger_base.debug("    Right of %s which comes before %s.", (const char*)droppedOn->GetName().c_str(), (const char*)next->GetName().c_str());
                                    next->SetModelChain(">" + m->GetName());
                                } else {
                                    logger_base.debug("    Right of %s.", (const char*)droppedOn->GetName().c_str());
                                }
                                m->SetModelChain(">" + droppedOn->GetName());
                                if (port->GetPortType() == PortCMObject::PORTTYPE::SERIAL) {
                                    m->SetModelChain("");
                                    droppedOn->SetModelChain("");
                                    if (next != nullptr)
                                        next->SetModelChain("");
                                    int nextch = droppedOn->GetControllerDMXChannel() + droppedOn->GetChanCount();
                                    m->SetControllerDMXChannel(nextch);
                                    Model* last = m;
                                    while (next != nullptr) {
                                        nextch = last->GetControllerDMXChannel() + last->GetChanCount();
                                        if (next != m && next->GetControllerDMXChannel() < nextch)
                                            next->SetControllerDMXChannel(nextch);
                                        last = next;
                                        next = port->GetUDPort()->GetModelAfter(last);
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                logger_base.debug("    but not onto a port ... so nothing to do.");
            }
        }
    }

    _dragStartLocation = wxPoint(-99999, -99999);

    ClearOver(PanelController, _controllers);

    while (!_xLights->DoAllWork()) {
        // dont get into a redraw loop from here
        _xLights->GetOutputModelManager()->RemoveWork("ASAP", OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW);
    }
    ReloadModels();
}

void ControllerModelDialog::OnPanelControllerLeftDown(wxMouseEvent& event)
{
    int portMargin = _controllers.front()->GetRect().GetRight() + 2;

    wxPoint mouse = event.GetPosition();

    if (mouse.x < portMargin)
        return;

    mouse += GetScrollPosition(PanelController);

    for (const auto& it : _controllers) {
        if (it->GetType() == "MODEL" && it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE) {
            auto m = dynamic_cast<ModelCMObject*>(it);
            if (m->IsMain()) {
                // when a model is clicked on then it becomes the last dropped
                if (_lastDropped != m->GetModel()) {
                    // redraw the model that used to be last dropped
                    if (_lastDropped != nullptr) {
                        auto mm = GetModelsCMObject(_lastDropped);
                        if (mm != nullptr) {
                            wxRect rect = mm->GetRect();
                            rect.Offset(-1 * GetScrollPosition(PanelController));
                            PanelController->RefreshRect(rect);
                        }
                    }

                    _lastDropped = m->GetModel();

                    // redraw the new last dropped
                    wxRect rect = it->GetRect();
                    rect.Offset(-1 * GetScrollPosition(PanelController));
                    PanelController->RefreshRect(rect);
                }

                _dragStartLocation = mouse;

                wxTextDataObject dragData("Controller:" + ((ModelCMObject*)it)->GetName());

                wxBitmap bmp(32, 32);
                wxMemoryDC dc;
                dc.SelectObject(bmp);
                it->Draw(dc, portMargin, wxPoint(-4, -4), wxPoint(-4, -4), wxSize(-1 * it->GetRect().GetLeft(), -1 * it->GetRect().GetTop()), 1, false, false, _lastDropped);

#ifdef __linux__
                wxIcon dragCursor;
                dragCursor.CopyFromBitmap(bmp.ConvertToImage());
#else
                wxCursor dragCursor(bmp.ConvertToImage());
#endif

                wxDropSource dragSource(this, dragCursor, dragCursor, dragCursor);

                if (PanelController->GetToolTipText() != "") {
                    PanelController->SetToolTip("");
                }
                if (PanelModels->GetToolTipText() != "") {
                    PanelModels->SetToolTip("");
                }

                _dragging = m;

                dragSource.SetData(dragData);
                dragSource.DoDragDrop(wxDragMove);

                _dragging = nullptr;
            }
            break;
        } else if (it->GetType() == "SR" && it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE) {
            auto sr = dynamic_cast<SRCMObject*>(it);

            _dragStartLocation = mouse;

            wxTextDataObject dragData("Controller:SR:" + sr->GetName());

            wxBitmap bmp(32, 32);
            wxMemoryDC dc;
            dc.SelectObject(bmp);
            sr->DrawIcon(dc, portMargin, wxPoint(-4, -4), wxPoint(-4, -4), wxSize(-1 * sr->GetRect().GetLeft(), -1 * sr->GetRect().GetTop()), 1);

#ifdef __linux__
            wxIcon dragCursor;
            dragCursor.CopyFromBitmap(bmp.ConvertToImage());
#else
            wxCursor dragCursor(bmp.ConvertToImage());
#endif

            wxDropSource dragSource(this, dragCursor, dragCursor, dragCursor);

            if (PanelController->GetToolTipText() != "") {
                PanelController->SetToolTip("");
            }
            if (PanelModels->GetToolTipText() != "") {
                PanelModels->SetToolTip("");
            }

            _dragging = sr;

            dragSource.SetData(dragData);
            dragSource.DoDragDrop(wxDragMove);

            _dragging = nullptr;
            break;
        }
    }
}

bool ControllerModelDialog::MaybeSetSmartRemote(wxKeyEvent& event)
{
    if (!event.ControlDown() && !event.CmdDown())
        return false;

    int keyCode = event.GetKeyCode();
    if (keyCode >= 65 && keyCode <= 90)
        keyCode += 32;

    int remote = 0;
    if (keyCode == 32) {
        // this will set to none
    } else if (keyCode >= 97 && keyCode <= 127) {
        remote = keyCode - 32 - 64;
    } else {
        return false;
    }

    if (_lastDropped != nullptr && _lastDropped->GetControllerCaps()->SupportsSmartRemotes() && remote <= _lastDropped->GetControllerCaps()->GetSmartRemoteCount()) {
        _lastDropped->SetSmartRemote(remote);

        while (!_xLights->DoAllWork()) {
            // dont get into a redraw loop from here
            _xLights->GetOutputModelManager()->RemoveWork("ASAP", OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW);
        }
        ReloadModels();

        return true;
    }

    return false;
}

void ControllerModelDialog::OnPanelControllerKeyDown(wxKeyEvent& event)
{
    if (!MaybeSetSmartRemote(event)) {
        ScrollToKey(event.GetKeyCode());
    }

    if (event.GetKeyCode() == WXK_ESCAPE) {
        EndDialog(wxCLOSE);
    }
    event.Skip();
}

void ControllerModelDialog::OnKeyDown(wxKeyEvent& event)
{
    if (!MaybeSetSmartRemote(event)) {
        ScrollToKey(event.GetKeyCode());
    }

    if (event.GetKeyCode() == WXK_ESCAPE) {
        EndDialog(wxCLOSE);
    }
    event.Skip();
}

void ControllerModelDialog::OnPanelModelsKeyDown(wxKeyEvent& event)
{
    if (!MaybeSetSmartRemote(event)) {
        ScrollToKey(event.GetKeyCode());
    }

    if (event.GetKeyCode() == WXK_ESCAPE) {
        EndDialog(wxCLOSE);
    }
    event.Skip();
}

void ControllerModelDialog::OnPanelControllerLeftUp(wxMouseEvent& event)
{
    _dragging = nullptr;
    ClearOver(PanelController, _controllers);
}

void ControllerModelDialog::OnPanelControllerLeftDClick(wxMouseEvent& event)
{
    wxPoint mouse = event.GetPosition();
    wxPoint adjustedMouse = mouse + GetScrollPosition(PanelController);

    ModelCMObject* cm = dynamic_cast<ModelCMObject*>(GetControllerCMObjectAt(mouse, adjustedMouse));
    if (cm != nullptr) {
        DropFromController(wxPoint(0, 0), cm->GetName(), PanelModels);

        while (!_xLights->DoAllWork()) {
            // dont get into a redraw loop from here
            _xLights->GetOutputModelManager()->RemoveWork("ASAP", OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW);
        }
        ReloadModels();
    }
}

void ControllerModelDialog::OnPanelControllerMouseMove(wxMouseEvent& event)
{
    wxPoint mouse = event.GetPosition();
    mouse += GetScrollPosition(PanelController);

    if (_dragging != nullptr) {
        auto m = dynamic_cast<ModelCMObject*>(_dragging);
        if (m != nullptr) {
            bool handled = false;
            // handle ports first
            for (const auto& it : _controllers) {
                if (it->GetType() == "PORT") {
                    auto hit = it->HitTest(mouse);
                    if (hit != BaseCMObject::HITLOCATION::NONE) {
                        handled = true;
                        it->SetOver(BaseCMObject::HITLOCATION::RIGHT);
                        wxRect rect = it->GetRect();
                        rect.Offset(-1 * GetScrollPosition(PanelController));
                        PanelController->RefreshRect(rect);
                    } else {
                        ClearOver(PanelController, _controllers);
                    }
                }
            }

            // now models if not handled
            for (const auto& it : _controllers) {
                if (it->GetType() != "PORT") {
                    auto hit = it->HitTest(mouse);
                    if (!handled && hit != BaseCMObject::HITLOCATION::NONE) {
                        if (it->GetOver() != hit) {
                            if (it->GetType() == "SR") {
                                // when over an SR we highlight it all
                                it->SetOver(BaseCMObject::HITLOCATION::ALL);
                            } else {
                                // when over a model we just highlight the side
                                it->SetOver(hit);
                            }
                            wxRect rect = it->GetRect();
                            rect.Offset(-1 * GetScrollPosition(PanelController));
                            PanelController->RefreshRect(rect);
                        }
                    } else {
                        ClearOver(PanelController, _controllers);
                    }
                }
            }
        } else {
            auto sr = dynamic_cast<SRCMObject*>(_dragging);
            if (sr != nullptr) {
                bool handled = false;
                // handle ports first
                for (const auto& it : _controllers) {
                    if (it->GetType() == "PORT") {
                        auto hit = it->HitTest(mouse);
                        if (hit != BaseCMObject::HITLOCATION::NONE) {
                            handled = true;
                            it->SetOver(BaseCMObject::HITLOCATION::RIGHT);
                            wxRect rect = it->GetRect();
                            rect.Offset(-1 * GetScrollPosition(PanelController));
                            PanelController->RefreshRect(rect);
                        } else {
                            ClearOver(PanelController, _controllers);
                        }
                    }
                }

                // now SRs if not handled
                for (const auto& it : _controllers) {
                    if (it->GetType() == "SR") {
                        auto hit = it->HitTest(mouse);
                        if (!handled && hit != BaseCMObject::HITLOCATION::NONE) {
                            auto sr = static_cast<SRCMObject*>(it);
                            // If over A or over the max SR then dont highlight an option which is not possible
                            if (sr->GetSmartRemote() == 1 && hit == BaseCMObject::HITLOCATION::LEFT) {
                                hit = BaseCMObject::HITLOCATION::ALL;
                            } else if (_caps != nullptr && sr->GetSmartRemote() == _caps->GetSmartRemoteCount() && hit == BaseCMObject::HITLOCATION::RIGHT) {
                                hit = BaseCMObject::HITLOCATION::ALL;
                            }
                            if (it->GetOver() != hit) {
                                it->SetOver(hit);
                                wxRect rect = it->GetRect();
                                rect.Offset(-1 * GetScrollPosition(PanelController));
                                PanelController->RefreshRect(rect);
                            }
                        } else {
                            ClearOver(PanelController, _controllers);
                        }
                    }
                }
            }
        }
    } else {
        std::string tt = "";

        if (event.GetPosition().x < _controllers.front()->GetRect().GetRight() + 2) {
            for (const auto& it : _controllers) {
                if (it->GetType() == "PORT") {
                    if (it->HitYTest(mouse)) {
                        auto port = dynamic_cast<PortCMObject*>(it);
                        if (port != nullptr) {
                            tt = GetPortTooltip(port->GetUDPort(), port->GetVirtualStringFromMouse(mouse));
                        }
                        break;
                    }
                }
            }
        } else {
            for (const auto& it : _controllers) {
                bool ishit = it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE;
                auto m = dynamic_cast<ModelCMObject*>(it);
                auto sr = dynamic_cast<SRCMObject*>(it);
                if (ishit || (m != nullptr && m->IsOutline())) {
                    wxRect rect = it->GetRect();
                    rect.Offset(-1 * GetScrollPosition(PanelController));
                    PanelController->RefreshRect(rect);
                    if (ishit) {
                        if (m != nullptr) {
                            tt = GetModelTooltip(m);
                        } else if (sr != nullptr) {
                            tt = GetSRTooltip(sr);
                        }
                    }
                    break;
                }
            }
        }

        if (PanelController->GetToolTipText() != tt) {
            PanelController->SetToolTip(tt);
        }
    }
}

void ControllerModelDialog::OnPanelControllerMouseEnter(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelControllerMouseLeave(wxMouseEvent& event)
{
    ClearOver(PanelController, _controllers);
}

void ControllerModelDialog::ClearOver(wxPanel* panel, std::list<BaseCMObject*> list)
{
    for (const auto& it : list) {
        if (it->GetOver() != BaseCMObject::HITLOCATION::NONE) {
            it->SetOver(BaseCMObject::HITLOCATION::NONE);
            wxRect rect = it->GetRect();
            rect.Offset(-1 * GetScrollPosition(panel));
            panel->RefreshRect(rect);
        }
    }
}

std::string ControllerModelDialog::GetPortTooltip(UDControllerPort* port, int virtualString)
{
    std::string protocol;
    if (port->GetProtocol() != "") {
        protocol = wxString::Format("Protocol: %s\n", port->GetProtocol());
    }

    std::string vs;
    std::string sc;
    std::string sr;
    std::string sa;

    if (_caps != nullptr && !_caps->MergeConsecutiveVirtualStrings() && port->GetVirtualStringCount() > 1) {
        vs = wxString::Format("Virtual Strings: %d\n", port->GetVirtualStringCount());
    } else if (port->GetVirtualStringCount() > 1) {
        vs = wxString::Format("Virtual String: %d\n", virtualString + 1);
    }

    if (port->Channels() > 0 && port->GetType() == "Pixel") {
        sa = wxString::Format("Estimated Current Draw: %0.2fA\n", port->GetAmps(_controller->GetDefaultBrightnessUnderFullControl()));
    }

    if (port->GetVirtualStringCount() <= 1 || virtualString < 0 || (_caps != nullptr && !_caps->MergeConsecutiveVirtualStrings())) {
        if (port->GetModelCount() > 0 && port->Channels() > 0) {
            sc = wxString::Format("Start Channel: %d (#%d:%d)\nChannels: %d (Pixels %d)",
                                  port->GetStartChannel(),
                                  port->GetUniverse(),
                                  port->GetUniverseStartChannel(),
                                  port->Channels(),
                                  INTROUNDUPDIV(port->Channels(), GetChannelsPerPixel(port->GetProtocol())));
        }
    } else {
        auto pvs = port->GetVirtualString(virtualString);
        if (pvs == nullptr) {
        } else {
            switch (pvs->_smartRemote) {
            case 1:
                sr = "Smart Remote: A\n";
                break;
            case 2:
                sr = "Smart Remote: B\n";
                break;
            case 3:
                sr = "Smart Remote: C\n";
                break;
            default:
                break;
            }
            if (pvs->Channels() > 0) {
                sc = wxString::Format("Start Channel: %d (#%d:%d)\nChannels: %d (Pixels %d)",
                                      pvs->_startChannel,
                                      pvs->_universe,
                                      pvs->_universeStartChannel,
                                      pvs->Channels(),
                                      INTROUNDUPDIV(pvs->Channels(), pvs->_channelsPerPixel));
            }
        }
    }

    std::string fps40;
    if (_caps != nullptr && _caps->GetMaxPixelsAt40FPS() != -1) {
        int max = _caps->GetMaxPixelsAt40FPS();
        if (port->AtLeastOneModelIsUsingSmartRemote())
        {
            max = _caps->GetMaxPixelsAt40FPS_SR();
        }
        if (port->Channels() > max * 3)
        {
            fps40 = "\nToo many pixels for 40FPS.";
        }
    }

    return wxString::Format("Port: %d\nType: %s\n%s%s%s%s%s%s", port->GetPort(), port->GetType(), sa, protocol, sr, vs, sc, fps40);
}

std::string ControllerModelDialog::GetSRTooltip(SRCMObject* sr)
{
    std::string protocol;
    if (sr->GetProtocol() != "") {
        protocol = wxString::Format("Protocol: %s\n", sr->GetProtocol());
    }

    std::string vs;
    std::string sc;
    std::string srn;
    std::string sa;

    if (sr->GetVirtualStringCount() > 1) {
        vs = wxString::Format("Virtual Strings: %d\n", sr->GetVirtualStringCount());
    }

    if (sr->Channels() > 0) {
        sa = wxString::Format("Estimated Current Draw: %0.2fA\n", sr->GetAmps(_controller->GetDefaultBrightnessUnderFullControl()));
    }

    srn = "Smart Remote: " + sr->GetName() + "\n";
    if (sr->Channels() > 0) {
        sc = wxString::Format("Start Channel: %d (#%d:%d)\nChannels: %d (Pixels %d)",
                              sr->StartChannel(),
                              sr->GetUniverse(),
                              sr->GetUniverseStartChannel(),
                              sr->Channels(),
                              INTROUNDUPDIV(sr->Channels(), GetChannelsPerPixel(sr->GetProtocol())));
    }

    return wxString::Format("Port: %d\n%s%s%s%s%s", sr->GetPort()->GetPort(), srn, protocol, vs, sa, sc);
}

std::string ControllerModelDialog::GetModelTooltip(ModelCMObject* mob)
{
    std::string shadow;
    std::string controllerName;
    std::string type;
    std::string protocol;
    std::string description;
    int32_t channelOffset;
    std::string ip;
    std::string universe;
    std::string inactive;
    std::string baud;
    int startUniverse;
    int endUniverse;
    bool isSubsequentString = false;

    auto m = mob->GetModel();
    if (m == nullptr && mob->GetString() > 0) {
        // this is a 2nd+ string on a model
        isSubsequentString = true;
        m = _mm->GetModel(mob->GetName());
        if (m == nullptr)
            return "";
    }

    wxString sr;
    if (m->GetSmartRemote() != 0) {
        sr = "Smart Remote: ";
        sr += m->GetSmartRemoteLetterForString(mob->GetString() + 1);
        if (!isSubsequentString) {
            sr += wxString::Format("\nSmart Remote Type: %s", m->GetSmartRemoteType());
            sr += wxString::Format("\nSmart Remote Cascade Down Port: %s", toStr(m->GetSRCascadeOnPort()));
            sr += wxString::Format("\nSmart Remote Cascade Length: %d", m->GetSRMaxCascade());
        }
        sr += "\n";
    }

    _xLights->GetControllerDetailsForChannel(m->GetFirstChannel() + 1, controllerName, type, protocol, description,
                                             channelOffset, ip, universe, inactive, baud, startUniverse, endUniverse);

    if (m->IsShadowModel()) {
        shadow = "Shadowing '" + m->GetShadowModelFor() + "'\n";
    } else {
        auto shadows = m->GetShadowedBy();
        if (shadows.size() > 0) {
            std::string sh;
            for (const auto& it : shadows) {
                if (sh != "")
                    sh += ", ";
                sh += it;
            }
            shadow = "Shadowed By: " + sh + "\n";
        }
    }

    std::string mdescription;
    if (m->description != "") {
        mdescription = "\nDescription: '" + m->description + "'";
    }

    auto om = _xLights->GetOutputManager();

    std::string usc;
    std::string dmx;
    std::string stringSettings;
    if (m->IsSerialProtocol()) {
        dmx = wxString::Format("\nChannel %d", m->GetControllerDMXChannel());
        UDControllerPortModel* udm = mob->GetUDModel();
        if (udm != nullptr) {
            auto ep = dynamic_cast<ControllerEthernet*>(_controller);
            if (ep != nullptr) {
                if (m->GetStartChannelInDisplayFormat(om)[0] != '#' && (ep->GetProtocol() == OUTPUT_E131 || ep->GetProtocol() == OUTPUT_ARTNET)) {
                    usc = wxString::Format(" [#%d:%d]",
                                           udm->GetUniverse(),
                                           udm->GetUniverseStartChannel());
                }
            }
        }
    } else if (m->IsMatrixProtocol()) {
        // Any extra matrix properties to display?
    } else {
        UDControllerPortModel* udm = mob->GetUDModel();
        if (udm != nullptr) {
            stringSettings += wxString::Format("\nEstimated Current Draw: %0.2fA", udm->GetAmps(_controller->GetDefaultBrightnessUnderFullControl()));
            if (udm->GetBrightness(-1) != -1) {
                stringSettings += wxString::Format("\nBrightness: %d%%", udm->GetBrightness(-1));
            }
            if (udm->GetColourOrder("xxx") != "xxx") {
                stringSettings += wxString::Format("\nColor Order: %s", udm->GetColourOrder(""));
            }
            if (udm->GetDirection("xxx") != "xxx") {
                stringSettings += wxString::Format("\nDirection: %s", udm->GetDirection(""));
            }
            if (udm->GetGamma(-1) != -1) {
                stringSettings += wxString::Format("\nGamma: %.1f", (float)udm->GetGamma(0));
            }
            if (udm->GetGroupCount(-1) != -1) {
                stringSettings += wxString::Format("\nGrouping: %d", udm->GetGroupCount(0));
            }
            if (udm->GetStartNullPixels(-1) != -1) {
                stringSettings += wxString::Format("\nStart Null Pixels: %d", udm->GetStartNullPixels(0));
            }
            if (udm->GetEndNullPixels(-1) != -1) {
                stringSettings += wxString::Format("\nEnd Null Pixels: %d", udm->GetEndNullPixels(0));
            }
            if (udm->GetSmartTs(-1) != -1) {
                stringSettings += wxString::Format("\nSmart Ts: %d", udm->GetSmartTs(0));
            }

            auto ep = dynamic_cast<ControllerEthernet*>(_controller);
            if (ep != nullptr) {
                if (m->GetStartChannelInDisplayFormat(om)[0] != '#' && (ep->GetProtocol() == OUTPUT_E131 || ep->GetProtocol() == OUTPUT_ARTNET)) {
                    usc = wxString::Format(" [#%d:%d]",
                                           udm->GetUniverse(),
                                           udm->GetUniverseStartChannel());
                }
            }
        }
    }

    // Not a fan of this code here but I dont have a better place for it right now
    std::string special;
    if (_controller->GetVendor() == "HinksPix" && _controller->GetModel() == "PRO") {
        if (m->GetSmartRemote() != 0) {
            int absPort = m->GetControllerPort();
            int absSM = m->GetSmartRemote();
            std::string SMType = m->GetSmartRemoteType();

            if (mob->GetPort() != nullptr) {
                absPort = mob->GetPort()->GetPort();
                absSM = m->GetSmartRemoteForString(mob->GetString() + 1);
                SMType = mob->GetPort()->GetSmartRemoteType(absSM);
            }

            int port4 = (absPort - 1) % 4 + 1;
            int port16 = ((absPort - 1) % 4) * 4 + ((absSM - 1) % 4) + 1;
            if (SMType.find("16") != std::string::npos && SMType.find("ac") == std::string::npos) {
                special = wxString::Format("\nHinksPix 16 Port Long Range Port : %d", port16).ToStdString();
            } else {
                special = wxString::Format("\nHinksPix 4 Port Long Range Port : %d", port4).ToStdString();
            }
        }
    }

    std::string sccc;
    if (isSubsequentString) {
        if (usc != "") {
            sccc = wxString::Format("Start Channel: %s\n", usc).ToStdString();
        }
    } else {
        sccc = wxString::Format("Start Channel: %s%s\nEnd Channel %s\n", m->GetStartChannelInDisplayFormat(om), usc, m->GetLastChannelInStartChannelFormat(om)).ToStdString();
    }

    std::string mc;
    if (!isSubsequentString) {
        mc = wxString::Format("Model Chain : % s\n", m->GetModelChain() == "" ? "Beginning" : m->GetModelChain()).ToStdString();
    }

    if (_autoLayout) {
        return wxString::Format("Name: %s\n%sController Name: %s\n%s%sStrings %d\n%sPort: %d\nProtocol: %s%s%s%s%s",
                                mob->GetDisplayName(), shadow, controllerName, mc, sccc,
                                m->GetNumPhysicalStrings(), sr, mob->GetPort() != nullptr ? (mob->GetPort()->GetPort() == 0 ? m->GetControllerPort() : mob->GetPort()->GetPort()) : m->GetControllerPort(), m->GetControllerProtocol(), dmx, mdescription, stringSettings, special)
            .ToStdString();
    } else {
        return wxString::Format("name: %s\n%sController Name: %s\nIP/Serial: %s\n%sStrings %d\nSmart Remote: %s\nPort: %d\nProtocol: %s%s%s%s%s",
                                mob->GetDisplayName(), shadow, controllerName, universe, sccc,
                                m->GetNumPhysicalStrings(), sr, mob->GetPort() != nullptr ? (mob->GetPort()->GetPort() == 0 ? m->GetControllerPort() : mob->GetPort()->GetPort()) : m->GetControllerPort(), m->GetControllerProtocol(), dmx, mdescription, stringSettings, special)
            .ToStdString();
    }
}

bool ControllerModelDialog::Scroll(wxPanel* panel, int scrollByX, int scrollByY)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool scrolled = false;

    // we dont scroll models panel
    if (panel == PanelModels)
        return scrolled;

    int maxx = ScrollBar_Controller_H->GetRange();
    int maxy = ScrollBar_Controller_V->GetRange();

    logger_base.debug("Current Scroll Position: %d, %d", ScrollBar_Controller_H->GetThumbPosition(), ScrollBar_Controller_V->GetThumbPosition());
    logger_base.debug("Scroll Range: %d, %d", maxx, maxy);
    logger_base.debug("Scroll By: %d, %d", scrollByX, scrollByY);

    if (scrollByX > 0 && ScrollBar_Controller_H->GetThumbPosition() < maxx - SCROLLBY) {
        scrolled = true;
        ScrollBar_Controller_H->SetThumbPosition(ScrollBar_Controller_H->GetThumbPosition() + scrollByX);
        if (ScrollBar_Controller_H->GetThumbPosition() > maxx) {
            ScrollBar_Controller_H->SetThumbPosition(maxx);
        }
    } else if (scrollByX < 0 && ScrollBar_Controller_H->GetThumbPosition() > 0) {
        scrolled = true;
        ScrollBar_Controller_H->SetThumbPosition(ScrollBar_Controller_H->GetThumbPosition() + scrollByX);
        if (ScrollBar_Controller_H->GetThumbPosition() < 0) {
            ScrollBar_Controller_H->SetThumbPosition(0);
        }
    }

    if (scrollByY > 0 && ScrollBar_Controller_V->GetThumbPosition() < maxy - SCROLLBY) {
        scrolled = true;
        ScrollBar_Controller_V->SetThumbPosition(ScrollBar_Controller_V->GetThumbPosition() + scrollByY);
        if (ScrollBar_Controller_V->GetThumbPosition() > maxy) {
            ScrollBar_Controller_V->SetThumbPosition(maxy);
        }
    } else if (scrollByY < 0 && ScrollBar_Controller_V->GetThumbPosition() > 0) {
        scrolled = true;
        ScrollBar_Controller_V->SetThumbPosition(ScrollBar_Controller_V->GetThumbPosition() + scrollByY);
        if (ScrollBar_Controller_V->GetThumbPosition() < 0) {
            ScrollBar_Controller_V->SetThumbPosition(0);
        }
    }

    if (scrolled)
        PanelController->Refresh();

    return scrolled;
}

wxPoint ControllerModelDialog::GetScrollPosition(wxPanel* panel) const
{
    if (panel == PanelModels) {
        return wxPoint(0, ScrollBar_Models->GetThumbPosition());
    }
    return wxPoint(ScrollBar_Controller_H->GetThumbPosition(), ScrollBar_Controller_V->GetThumbPosition());
}

void ControllerModelDialog::OnPanelControllerRightDown(wxMouseEvent& event)
{
    wxPoint mouse = event.GetPosition();
    wxPoint adjustedMouse = mouse + GetScrollPosition(PanelController);
    mouse += wxPoint(0, GetScrollPosition(PanelController).y);

    wxMenu mnu;
    mnu.Append(CONTROLLERModel_PRINT, "Print");
    mnu.Append(CONTROLLERModel_SAVE_CSV, "Save As CSV...");

    if (_cud->HasModels()) {
        mnu.Append(CONTROLLER_REMOVEALLMODELS, "Remove All Models From Controller");
    }

    BaseCMObject* cm = GetControllerCMObjectAt(mouse, adjustedMouse);
    if (cm != nullptr) {
        cm->AddRightClickMenu(mnu, this);
    }
    _popup = cm;

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&ControllerModelDialog::OnPopupCommand, nullptr, this);
    PopupMenu(&mnu);
}

void ControllerModelDialog::OnPanelControllerPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(PanelController);

    dc.SetPen(__backgroundPen);
    dc.SetBrush(__backgroundBrush);
    dc.DrawRectangle(0, 0, PanelController->GetSize().GetWidth(), PanelController->GetSize().GetHeight());

    if (_controllers.size() == 0)
        return;

    int xOffset = ScrollBar_Controller_H->GetThumbPosition();
    int yOffset = ScrollBar_Controller_V->GetThumbPosition();

    dc.SetDeviceOrigin(-xOffset, -yOffset);

    wxPoint mouse = PanelController->ScreenToClient(wxGetMousePosition());
    wxPoint adjustedMouse = mouse + GetScrollPosition(PanelController);
    mouse += wxPoint(0, GetScrollPosition(PanelController).y);
    int portMargin = _controllers.front()->GetRect().GetRight() + 2;

    wxFont font = wxFont(wxSize(0, getFontSize()), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("Arial"), wxFONTENCODING_DEFAULT);
    dc.SetFont(font);

    // draw the SR first
    for (const auto& it : _controllers) {
        if (it->GetType() == "SR") {
            it->Draw(dc, portMargin, mouse, adjustedMouse, wxSize(0, 0), 1, false, true, _lastDropped);
        }
    }

    // now draw the models
    for (const auto& it : _controllers) {
        if (it->GetType() == "MODEL") {
            it->Draw(dc, portMargin, mouse, adjustedMouse, wxSize(0, 0), 1, false, true, _lastDropped);
        }
    }

    // now undo the offset and draw the ports
    dc.SetDeviceOrigin(0, -yOffset);
    dc.SetPen(__backgroundPen);
    dc.SetBrush(__backgroundBrush);
    dc.DrawRectangle(0, 0, _controllers.front()->GetRect().GetRight() + 2, _controllers.back()->GetRect().GetBottom() + 10);
    for (const auto& it : _controllers) {
        if (it->GetType() == "PORT") {
            it->Draw(dc, portMargin, mouse, adjustedMouse, wxSize(0, 0), 1, false, true, _lastDropped);
        }
    }

    if (_caps != nullptr && _caps->GetNumberOfBanks() > 1) {
        std::vector<wxColor> colours = { *wxRED, *wxGREEN, *wxBLUE, *wxYELLOW, wxColour(0xFF, 0x00, 0xFF) };
        int bankSize = _caps->GetBankSize();
        int barX = (GetPixelPort(1)->GetRect().GetLeft() - 2) / 2;
        for (int i = 0; i < _caps->GetNumberOfBanks(); i++) {
            int topPort = i * bankSize + 1;
            int bottomPort = std::min((i + 1) * bankSize, _caps->GetMaxPixelPort());
            auto tp = GetPixelPort(topPort);
            auto bp = GetPixelPort(bottomPort);
            if (tp != nullptr && bp != nullptr) {
                int topPortY = tp->GetRect().GetTop();
                int bottomPortY = bp->GetRect().GetBottom();
                dc.SetBrush(wxBrush(colours[i % colours.size()]));
                dc.DrawRectangle(0, topPortY, barX, bottomPortY - topPortY);
            }
        }
    }

    if (_caps != nullptr && _caps->SupportsRemotes()) {
#define PORTS_PER_REMOTE 4
        std::vector<wxColor> colours = { *wxRED, *wxGREEN, *wxBLUE, *wxYELLOW, wxColour(0xFF, 0x00, 0xFF) };
        wxColor replacementColour = *wxCYAN;
        int barX1 = (GetPixelPort(1)->GetRect().GetLeft() - 2) / 2;
        int barX2 = GetPixelPort(1)->GetRect().GetLeft() - 2;
        int bankSize = -1;
        if (_caps->GetNumberOfBanks() > 1) {
            bankSize = _caps->GetBankSize();
        }
        for (int i = 0; i < _caps->GetMaxPixelPort() / PORTS_PER_REMOTE; i++) {
            int topPort = i * PORTS_PER_REMOTE + 1;
            int bottomPort = std::min((i + 1) * PORTS_PER_REMOTE, _caps->GetMaxPixelPort());
            auto tp = GetPixelPort(topPort);
            auto bp = GetPixelPort(bottomPort);
            if (tp != nullptr && bp != nullptr) {
                int topPortY = tp->GetRect().GetTop();
                int bottomPortY = bp->GetRect().GetBottom();
                wxColor c = colours[i % colours.size()];
                if (bankSize != -1) {
                    int bank = topPort / bankSize;
                    if (colours[bank % colours.size()] == c) {
                        c = replacementColour;
                    }
                }
                dc.SetBrush(wxBrush(c));
                dc.DrawRectangle(barX1, topPortY, barX2 - barX1, bottomPortY - topPortY);
            }
        }
    }
}

PortCMObject* ControllerModelDialog::GetPixelPort(int port) const
{
    for (const auto& it : _controllers) {
        if (it->GetType() == "PORT") {
            auto p = dynamic_cast<PortCMObject*>(it);
            if (p->GetPortType() == PortCMObject::PORTTYPE::PIXEL && p->GetPort() == port) {
                return p;
            }
        }
    }
    return nullptr;
}

void ControllerModelDialog::OnScrollBar_Controller_HScroll(wxScrollEvent& event)
{
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_HScrollThumbTrack(wxScrollEvent& event)
{
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_HScrollChanged(wxScrollEvent& event)
{
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_VScroll(wxScrollEvent& event)
{
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_VScrollThumbTrack(wxScrollEvent& event)
{
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_VScrollChanged(wxScrollEvent& event)
{
    PanelController->Refresh();
}

#pragma region Models Panel
void ControllerModelDialog::OnScrollBar_ModelsScroll(wxScrollEvent& event)
{
    PanelModels->Refresh();
}

void ControllerModelDialog::OnScrollBar_ModelsScrollThumbTrack(wxScrollEvent& event)
{
    PanelModels->Refresh();
}

void ControllerModelDialog::OnScrollBar_ModelsScrollChanged(wxScrollEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Models Scrollbar: %d", ScrollBar_Models->GetThumbPosition());
    PanelModels->Refresh();
}

void ControllerModelDialog::OnPanelModelsPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(PanelModels);

    dc.SetPen(__backgroundPen);
    dc.SetBrush(__backgroundBrush);
    dc.DrawRectangle(0, 0, PanelModels->GetSize().GetWidth(), PanelModels->GetSize().GetHeight());

    int yOffset = ScrollBar_Models->GetThumbPosition();

    dc.SetDeviceOrigin(0, -yOffset);

    wxPoint mouse = PanelModels->ScreenToClient(wxGetMousePosition());
    mouse += GetScrollPosition(PanelModels);
    wxFont font = wxFont(wxSize(0, getFontSize()), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("Arial"), wxFONTENCODING_DEFAULT);
    dc.SetFont(font);

    for (const auto& it : _models) {
        it->Draw(dc, 0, mouse, mouse, wxSize(0, 0), 1, false, true, nullptr);
    }
}

void ControllerModelDialog::ScrollToKey(int keyCode)
{
    if (keyCode >= 65 && keyCode <= 90)
        keyCode += 32;
    for (const auto& it : _models) {
        ModelCMObject* m = dynamic_cast<ModelCMObject*>(it);
        if (m != nullptr && m->NameStartsWith((char)keyCode)) {
            ScrollBar_Models->SetThumbPosition(m->GetRect().GetPosition().y);
            PanelModels->Refresh();
            break;
        }
    }
}

void ControllerModelDialog::OnPanelModelsLeftDown(wxMouseEvent& event)
{
    wxPoint mouse = event.GetPosition();

    mouse += GetScrollPosition(PanelModels);

    for (const auto& it : _models) {
        auto m = static_cast<ModelCMObject*>(it);
        if (it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE) {
            wxTextDataObject dragData("Model:" + m->GetName());

            wxBitmap bmp(32, 32);
            wxMemoryDC dc;
            dc.SelectObject(bmp);
            it->Draw(dc, 0, wxPoint(-4, -4), wxPoint(-4, -4), wxSize(-1 * it->GetRect().GetLeft(), -1 * it->GetRect().GetTop()), 1, false, false, nullptr);

#ifdef __linux__
            wxIcon dragCursor;
            dragCursor.CopyFromBitmap(bmp.ConvertToImage());
#else
            wxCursor dragCursor(bmp.ConvertToImage());
#endif

            wxDropSource dragSource(this, dragCursor, dragCursor, dragCursor);

            _dragging = m;

            if (PanelController->GetToolTipText() != "") {
                PanelController->SetToolTip("");
            }
            if (PanelModels->GetToolTipText() != "") {
                PanelModels->SetToolTip("");
            }

            dragSource.SetData(dragData);
            dragSource.DoDragDrop(wxDragMove);

            _dragging = nullptr;
            break;
        }
    }
}

void ControllerModelDialog::OnPanelModelsLeftUp(wxMouseEvent& event)
{
    _dragging = nullptr;
    ClearOver(PanelController, _controllers);
}

void ControllerModelDialog::OnPanelModelsLeftDClick(wxMouseEvent& event)
{
    if (_lastDropped == nullptr)
        return;

    wxPoint mouse = event.GetPosition();
    mouse += GetScrollPosition(PanelModels);

    ModelCMObject* cm = dynamic_cast<ModelCMObject*>(GetModelsCMObjectAt(mouse));
    if (cm != nullptr) {
        auto m = cm->GetName();
        DropModelFromModelsPaneOnModel(cm, _lastDropped, true);

        while (!_xLights->DoAllWork()) {
            // dont get into a redraw loop from here
            _xLights->GetOutputModelManager()->RemoveWork("ASAP", OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW);
        }
        ReloadModels();

        for (const auto& it : _controllers) {
            cm = dynamic_cast<ModelCMObject*>(it);
            if (cm != nullptr && cm->GetName() == m) {
                EnsureSelectedModelIsVisible(cm);
                break;
            }
        }
    }
}

void ControllerModelDialog::EnsureSelectedModelIsVisible(ModelCMObject* cm)
{
    auto left = cm->GetRect().GetLeft();
    auto right = cm->GetRect().GetRight();
    auto top = cm->GetRect().GetTop();
    auto bottom = cm->GetRect().GetBottom();

    auto pos = GetScrollPosition(PanelController);

    bool scrolled = false;

    int x = ScrollBar_Controller_H->GetThumbPosition();
    int y = ScrollBar_Controller_V->GetThumbPosition();

    if (left < pos.x + LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + FIRST_MODEL_GAP_MULTIPLIER * HORIZONTAL_GAP) {
        x = left - 10;
        scrolled = true;
    } else if (right > pos.x + PanelController->GetSize().GetWidth()) {
        x = right - PanelController->GetSize().GetWidth() + 10;
        scrolled = true;
    }

    if (bottom > pos.y + PanelController->GetSize().GetHeight()) {
        y = bottom - PanelController->GetSize().GetHeight() + 10;
        scrolled = true;
    } else if (top < pos.y) {
        y = top - 10;
        scrolled = true;
    }

    if (scrolled) {
        ScrollBar_Controller_H->SetThumbPosition(x);
        ScrollBar_Controller_V->SetThumbPosition(y);
        PanelController->Refresh();
    }
}

void ControllerModelDialog::OnPanelModelsRightDown(wxMouseEvent& event)
{
    wxPoint mouse = event.GetPosition();
    mouse += GetScrollPosition(PanelModels);

    wxMenu mnu;

    BaseCMObject* cm = GetModelsCMObjectAt(mouse);
    if (cm != nullptr) {
        cm->AddRightClickMenu(mnu, this);
    }
    _popup = cm;

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&ControllerModelDialog::OnPopupCommand, nullptr, this);
    PopupMenu(&mnu);
}

void ControllerModelDialog::OnPanelModelsMouseMove(wxMouseEvent& event)
{
    wxPoint mouse = event.GetPosition();
    mouse += GetScrollPosition(PanelModels);

    std::string tt = "";
    if (_dragging == nullptr) {
        for (const auto& it : _models) {
            bool ishit = it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE;

            if (ishit || ((ModelCMObject*)it)->IsOutline()) {
                wxRect rect = it->GetRect();
                rect.Offset(-1 * GetScrollPosition(PanelModels));
                PanelModels->RefreshRect(rect);
                if (ishit) {
                    auto m = dynamic_cast<ModelCMObject*>(it);
                    if (m != nullptr) {
                        tt = GetModelTooltip(m);
                    }
                }
            }
        }
    }
    if (PanelModels->GetToolTipText() != tt) {
        PanelModels->SetToolTip(tt);
    }
}

void ControllerModelDialog::OnPanelModelsMouseWheel(wxMouseEvent& event)
{
    int position = ScrollBar_Models->GetThumbPosition();
    position -= event.GetWheelRotation();
    ScrollBar_Models->SetThumbPosition(position);
    PanelModels->Refresh();
}

void ControllerModelDialog::OnPanelModelsResize(wxSizeEvent& event)
{
    int panely = PanelModels->GetSize().y;
    ScrollBar_Models->SetRange(_modelsy);
    ScrollBar_Models->SetPageSize(_modelsy / 10);
    ScrollBar_Models->SetThumbSize(panely);
    PanelModels->Refresh();
}

void ControllerModelDialog::OnPanelModelsMouseEnter(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelModelsMouseLeave(wxMouseEvent& event)
{
}
#pragma endregion

#pragma region Controller Ports Panel
void ControllerModelDialog::OnPanelControllerMouseWheel(wxMouseEvent& event)
{
    bool vertical = true;
    if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL) {
        vertical = false;
    }
    if (event.ShiftDown()) {
        vertical = !vertical;
    }
    if (!vertical) {
        int position = ScrollBar_Controller_H->GetThumbPosition();
        position += event.GetWheelRotation();
        ScrollBar_Controller_H->SetThumbPosition(position);
    } else {
        int position = ScrollBar_Controller_V->GetThumbPosition();
        position -= event.GetWheelRotation();
        ScrollBar_Controller_V->SetThumbPosition(position);
    }

    PanelController->Refresh();
}

void ControllerModelDialog::OnPanelControllerResize(wxSizeEvent& event)
{
    int panelx = PanelController->GetSize().x;
    ScrollBar_Controller_H->SetRange(_controllersx);
    ScrollBar_Controller_H->SetPageSize(_controllersx / 10);
    ScrollBar_Controller_H->SetThumbSize(panelx);

    int panely = PanelController->GetSize().y;
    ScrollBar_Controller_V->SetRange(_controllersy);
    ScrollBar_Controller_V->SetPageSize(_controllersy / 10);
    ScrollBar_Controller_V->SetThumbSize(panely);
    PanelController->Refresh();
}
#pragma endregion

// gets the object which has its highlight set
BaseCMObject* ControllerModelDialog::GetControllerToDropOn()
{
    for (const auto& it : _controllers) {
        if (it->GetOver() != BaseCMObject::HITLOCATION::NONE) {
            return it;
        }
    }
    return nullptr;
}

BaseCMObject* ControllerModelDialog::GetControllerCMObjectAt(wxPoint mouse, wxPoint adjustedMouse)
{
    // look for ports first
    for (const auto& it : _controllers) {
        if (it->GetType() == "PORT") {
            if (it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE)
                return it;
        }
    }

    // if in the port region and nothing found then we hit nothing
    if (mouse.x < _controllers.front()->GetRect().GetRight() + 2)
        return nullptr;

    // now check for models as these take priority
    for (const auto& it : _controllers) {
        if (it->GetType() == "MODEL" && it->HitTest(adjustedMouse) != BaseCMObject::HITLOCATION::NONE)
            return it;
    }

    // now srs
    for (const auto& it : _controllers) {
        if (it->GetType() == "SR" && it->HitTest(adjustedMouse) != BaseCMObject::HITLOCATION::NONE)
            return it;
    }
    return nullptr;
}

BaseCMObject* ControllerModelDialog::GetModelsCMObjectAt(wxPoint mouse)
{
    for (const auto& it : _models) {
        if (it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE)
            return it;
    }
    return nullptr;
}

PortCMObject* ControllerModelDialog::GetControllerPortAtLocation(wxPoint mouse)
{
    BaseCMObject* last = nullptr;
    for (const auto& it : _controllers) {
        if (it->GetType() == "PORT") {
            if (it->HitYTest(mouse))
                return dynamic_cast<PortCMObject*>(it);
            last = it;
        }
        if (it->BelowHitYTest(mouse))
            return dynamic_cast<PortCMObject*>(last);
    }
    return nullptr;
}

SRCMObject* ControllerModelDialog::GetControllerSRAtLocation()
{
    return dynamic_cast<SRCMObject*>(GetControllerToDropOn());
}

void ControllerModelDialog::OnCheckBox_HideOtherControllerModelsClick(wxCommandEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("ControllerModelHideOtherControllerModels", CheckBox_HideOtherControllerModels->IsChecked());
    ReloadModels();
}

void ControllerModelDialog::OnSlider_ScaleCmdSliderUpdated(wxScrollEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("ControllerModelBoxScale", Slider_Box_Scale->GetValue());
    config->Write("ControllerModelFontScale", Slider_Font_Scale->GetValue());
    ReloadModels();
}

double ControllerModelDialog::getFontSize()
{
    return ScaleWithSystemDPI(GetSystemContentScaleFactor(), Slider_Font_Scale->GetValue());
}

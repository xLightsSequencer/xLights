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

#include <wx/dcclient.h>
#include <wx/menu.h>
#include <wx/position.h>
#include <wx/dcmemory.h>
#include <wx/dcscreen.h>
#include <wx/image.h>
#include <wx/file.h>
#include <wx/dnd.h>
#include <wx/dcbuffer.h>
#include <wx/numdlg.h>
#include <wx/artprov.h>

#include "ControllerModelDialog.h"
#include "xLightsMain.h"
#include "UtilFunctions.h"
#include "outputs/Output.h"
#include "controllers/ControllerUploadData.h"
#include "controllers/ControllerCaps.h"
#include "models/ModelManager.h"
#include "models/Model.h"
#include "outputs/Controller.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>

//(*IdInit(ControllerModelDialog)
const long ControllerModelDialog::ID_PANEL1 = wxNewId();
const long ControllerModelDialog::ID_SCROLLBAR1 = wxNewId();
const long ControllerModelDialog::ID_SCROLLBAR2 = wxNewId();
const long ControllerModelDialog::ID_TEXTCTRL1 = wxNewId();
const long ControllerModelDialog::ID_PANEL3 = wxNewId();
const long ControllerModelDialog::ID_CHECKBOX1 = wxNewId();
const long ControllerModelDialog::ID_PANEL2 = wxNewId();
const long ControllerModelDialog::ID_SCROLLBAR3 = wxNewId();
const long ControllerModelDialog::ID_PANEL4 = wxNewId();
//*)

const long ControllerModelDialog::CONTROLLERModel_PRINT = wxNewId();
const long ControllerModelDialog::CONTROLLERModel_SAVE_CSV = wxNewId();
const long ControllerModelDialog::CONTROLLER_SMARTREMOTE_None = wxNewId();
const long ControllerModelDialog::CONTROLLER_SMARTREMOTE_A = wxNewId();
const long ControllerModelDialog::CONTROLLER_SMARTREMOTE_B = wxNewId();
const long ControllerModelDialog::CONTROLLER_SMARTREMOTE_C = wxNewId();
const long ControllerModelDialog::CONTROLLER_SMARTREMOTE_ABC = wxNewId();
const long ControllerModelDialog::CONTROLLER_SMARTREMOTE_BC = wxNewId();
const long ControllerModelDialog::CONTROLLER_DMXCHANNEL = wxNewId();
const long ControllerModelDialog::CONTROLLER_PROTOCOL = wxNewId();
const long ControllerModelDialog::CONTROLLER_BRIGHTNESS = wxNewId();
const long ControllerModelDialog::CONTROLLER_BRIGHTNESSCLEAR = wxNewId();

BEGIN_EVENT_TABLE(ControllerModelDialog,wxDialog)
	//(*EventTable(ControllerModelDialog)
	//*)
END_EVENT_TABLE()

#pragma region Drawing Constants
#define TOP_BOTTOM_MARGIN ScaleWithSystemDPI(GetSystemContentScaleFactor(), 10)
#define VERTICAL_GAP ScaleWithSystemDPI(GetSystemContentScaleFactor(), 5)
#define VERTICAL_SIZE ScaleWithSystemDPI(GetSystemContentScaleFactor(), 40)
#define LEFT_RIGHT_MARGIN TOP_BOTTOM_MARGIN
#define HORIZONTAL_GAP VERTICAL_GAP
#define HORIZONTAL_SIZE ScaleWithSystemDPI(GetSystemContentScaleFactor(), 120)
#define CORNER_ROUNDING ScaleWithSystemDPI(GetSystemContentScaleFactor(), 5)
#define FONT_PIXEL_SIZE ScaleWithSystemDPI(GetSystemContentScaleFactor(), 15)
#define MODEL_ICON_SIZE ScaleWithSystemDPI(GetSystemContentScaleFactor(), 16)
#pragma endregion

#pragma region Colours
wxColour __lightBlue(185, 246, 250, wxALPHA_OPAQUE);
wxBrush __dropTargetBrush(__lightBlue);
wxPen __dropTargetPen(__lightBlue);
wxColour __lightRed(255, 133, 133, wxALPHA_OPAQUE);
wxBrush __invalidBrush(__lightRed);
wxPen __pixelPortOutlinePen(*wxRED);
wxPen __serialPortOutlinePen(*wxGREEN);
wxColour __grey(128, 128, 128, wxALPHA_OPAQUE);
wxPen __modelOutlinePen(__grey);
wxBrush __modelSRNoneBrush(*wxWHITE);
wxColour __lightGreen(153, 255, 145, wxALPHA_OPAQUE);
wxBrush __modelSRABrush(__lightGreen);
wxColour __lightPurple(184, 150, 255, wxALPHA_OPAQUE);
wxBrush __modelSRBBrush(__lightPurple);
wxColour __lightOrange(255, 201, 150, wxALPHA_OPAQUE);
wxBrush __modelSRCBrush(__lightOrange);
#pragma endregion

#pragma region Object Classes

class BaseCMObject
{
public:
    const static int STYLE_PIXELS = 1;
    const static int STYLE_STRINGS = 2;
    const static int STYLE_CHANNELS = 4;
    enum class HITLOCATION { NONE, LEFT, RIGHT };

protected:
    bool _selectable = false;
    bool _selected = false;
    bool _dragging = false;
    wxPoint _location = wxPoint(0,0);
    wxSize _size = wxSize(ScaleWithSystemDPI(GetSystemContentScaleFactor(), 100),
                          ScaleWithSystemDPI(GetSystemContentScaleFactor(), 40));
    UDController* _cud = nullptr;
    ControllerCaps* _caps = nullptr;
    HITLOCATION _over = HITLOCATION::NONE;
    int _style;
    bool _invalid;

public:

    BaseCMObject(UDController* cud, ControllerCaps* caps, wxPoint location, wxSize size, int style) {
        _style = style;
        _cud = cud;
        _caps = caps;
        _location = location;
        _size = size;
        _invalid = false;
    }
    virtual ~BaseCMObject() {}
    void SetInvalid(bool invalid) { _invalid = invalid; }
    HITLOCATION GetOver() const { return _over; }
    void SetOver(HITLOCATION hit) { _over = hit; }
    HITLOCATION HitTest(wxPoint mouse) {
        if (mouse.x >= _location.x &&
            mouse.x <= _location.x + (_size.x / 2) &&
            mouse.y >= _location.y &&
            mouse.y <= _location.y + _size.y) return HITLOCATION::LEFT;
        if (mouse.x >= _location.x + (_size.x / 2) &&
            mouse.x <= _location.x + _size.x &&
            mouse.y >= _location.y &&
            mouse.y <= _location.y + _size.y)  return HITLOCATION::RIGHT;
        return HITLOCATION::NONE;
    }
    virtual bool HitYTest(wxPoint mouse) {
        return (mouse.y >= _location.y &&
            mouse.y <= _location.y + _size.y);
    }
    bool BelowHitYTest(wxPoint mouse) {
        return (mouse.y < _location.y);
    }
    virtual void Draw(wxDC& dc, wxPoint mouse, wxSize offset, float scale, bool printing = false, bool border = true) = 0;
    void UpdateCUD(UDController* cud) { _cud = cud; }
    virtual void AddRightClickMenu(wxMenu& mnu) {}
    virtual bool HandlePopup(wxWindow* parent, int id) { return false; }
    virtual std::string GetType() const = 0;
    wxRect GetRect() const { return wxRect(_location, _size); }
    void DrawTextLimited(wxDC& dc, const std::string& text, const wxPoint& pt, const wxSize& size) {
        dc.SetClippingRegion(pt, size);
        dc.DrawText(text, pt);
        dc.DestroyClippingRegion();
    }
};

class PortCMObject : public BaseCMObject
{
public:
    enum class PORTTYPE { PIXEL, SERIAL };
protected:
    int _port = -1;
    PORTTYPE _type = PORTTYPE::PIXEL;
public:

    PortCMObject(PORTTYPE type, int port, UDController* cud, ControllerCaps* caps, wxPoint location, wxSize size, int style, bool invalid) :
        BaseCMObject(cud, caps, location, size, style) {
        _invalid = invalid;
        _port = port;
        _type = type;
    }
    UDControllerPort* GetUDPort() const {
        if (_type == PORTTYPE::PIXEL) {
            return _cud->GetControllerPixelPort(_port);
        }
        else {
            return _cud->GetControllerSerialPort(_port);
        }
    }
    int GetMaxPortChannels() const {
        if (_caps == nullptr) return 9999999;

        if (_type == PORTTYPE::PIXEL) {
            return _caps->GetMaxPixelPortChannels();
        }
        else {
            return _caps->GetMaxSerialPortChannels();
        }
    }
    virtual bool HitYTest(wxPoint mouse) override {
        int totaly = VERTICAL_SIZE;

        if (GetUDPort()->GetVirtualStringCount() > 1) {
            totaly += (GetUDPort()->GetVirtualStringCount() - 1)* (VERTICAL_SIZE + VERTICAL_GAP);
        }

        return (mouse.y >= _location.y &&
            mouse.y <= _location.y + totaly);
    }

    int GetModelCount() const {
        return GetUDPort()->GetModels().size();
    }
    Model* GetFirstModel() const {
        if (GetModelCount() == 0) return nullptr;
        return GetUDPort()->GetModels().front()->GetModel();
    }
    PORTTYPE GetPortType() const { return _type; }
    int GetPort() const { return _port; }
    virtual std::string GetType() const override { return "PORT"; }
    virtual void Draw(wxDC& dc, wxPoint mouse, wxSize offset, float scale, bool printing = false, bool border = true) override {
        auto origBrush = dc.GetBrush();
        auto origPen = dc.GetPen();
        auto origText = dc.GetTextForeground();

        wxSize sz = _size;
        sz = sz.Scale(scale, scale);
        dc.SetTextForeground(*wxBLACK);

        UDControllerPort* p = GetUDPort();
        if (!border) {
            dc.SetPen(*wxTRANSPARENT_PEN);
        }
        else if (_type == PORTTYPE::PIXEL) {
            dc.SetPen(__pixelPortOutlinePen);
        }
        else {
            dc.SetPen(__serialPortOutlinePen);
        }

        if (_over != HITLOCATION::NONE && !printing) {
            dc.SetBrush(__dropTargetBrush);
        }
        else if (_invalid) {
            dc.SetBrush(__invalidBrush);
        }

        auto location = _location * scale;
        dc.DrawRoundedRectangle(location + offset, sz, CORNER_ROUNDING * scale);

        wxPoint pt = location + offset + wxSize(2, 2);
        if (_type == PORTTYPE::PIXEL) {
            DrawTextLimited(dc, wxString::Format("Pixel Port %d", _port), pt, sz - wxSize(4,4));
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
            DrawTextLimited(dc, wxString::Format("%d", p->Channels() / 3), pt, sz - wxSize(pt.x + 2, 4));
            dc.SetTextForeground(*wxBLACK);
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
            dc.SetTextForeground(*wxBLACK);
            pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
        }

        dc.SetBrush(origBrush);
        dc.SetPen(origPen);
        dc.SetTextForeground(origText);
    }
    virtual void AddRightClickMenu(wxMenu& mnu) override
    {
        if (_caps != nullptr)
        {
            if (_type == PORTTYPE::PIXEL && _caps->GetPixelProtocols().size() == 0) return;
            if (_type == PORTTYPE::SERIAL && _caps->GetSerialProtocols().size() == 0) return;
        }
        mnu.AppendSeparator();
        mnu.Append(ControllerModelDialog::CONTROLLER_PROTOCOL, "Set Protocol");
    }
    virtual bool HandlePopup(wxWindow* parent, int id) override {
        if (id == ControllerModelDialog::CONTROLLER_PROTOCOL)
        {
            wxArrayString choices;
            if (_caps != nullptr)
            {
                if (_type == PORTTYPE::PIXEL)
                {
                    for (const auto& it : _caps->GetPixelProtocols())
                    {
                        choices.push_back(it);
                    }
                }
                else
                {
                    for (const auto& it : _caps->GetSerialProtocols())
                    {
                        choices.push_back(it);
                    }
                }
            }
            else
            {
                for (const auto& it : Model::GetProtocols())
                {
                    if (_type == PORTTYPE::PIXEL && Model::IsPixelProtocol(it))
                    {
                        choices.push_back(it);
                    }
                    else if (_type == PORTTYPE::SERIAL && !Model::IsPixelProtocol(it))
                    {
                        choices.push_back(it);
                    }
                }
            }

            wxSingleChoiceDialog dlg(parent, "Port Protocol", "Protocol", choices);
            if (dlg.ShowModal() == wxID_OK)
            {
                if (_caps != nullptr && !_caps->SupportsMultipleSimultaneousOutputProtocols()){

                    // We have to apply the protocol to all ports
                    if (_type == PORTTYPE::PIXEL)
                    {
                        for (int i = 1; i <= _cud->GetMaxPixelPort(); i++)
                        {
                            for (const auto& it : _cud->GetControllerPixelPort(i)->GetModels())
                            {
                                it->GetModel()->SetControllerProtocol(choices[dlg.GetSelection()]);
                            }
                        }
                    }
                    else
                    {
                        for (int i = 1; i <= _cud->GetMaxSerialPort(); i++)
                        {
                            for (const auto& it : _cud->GetControllerSerialPort(i)->GetModels())
                            {
                                it->GetModel()->SetControllerProtocol(choices[dlg.GetSelection()]);
                            }
                        }
                    }
                }
                else {
                    // We only need to apply the protocol to this port
                    for (const auto& it : GetUDPort()->GetModels())
                    {
                        it->GetModel()->SetControllerProtocol(choices[dlg.GetSelection()]);
                    }
                }
            }
            return true;
        }
        return false;
    }
};

class ModelCMObject : public BaseCMObject {
protected:
    ModelManager* _mm = nullptr;
    std::string _name;
    bool _outline = false;
    bool _main = false;
    std::string _displayName;
    int _string = 0;
public:
    ModelCMObject(const std::string& name, const std::string displayName, ModelManager* mm, UDController* cud, ControllerCaps* caps, wxPoint location, wxSize size, int style) :
        BaseCMObject(cud, caps, location, size, style), _mm(mm) {
        _name = name;
        _main = name == displayName;
        _displayName = displayName;
        if (name != displayName && name + "-str-1" == displayName) {
            _main = true;
        }
        if (name != displayName) {
            _string = wxAtoi(AfterLast(displayName, '-')) -1;
        }
    }

    std::string GetName() const { return _name; }
    std::string GetDisplayName() const { return _displayName; }

    bool NameStartsWith(char c) {
        if (_name == "") return false;
        char cn = _name[0];
        if (cn >= 65 && cn <= 90) cn += 32;

        return cn == c;
    }
    UDControllerPortModel* GetUDModel() {
        if (_cud == nullptr) return nullptr;
        return _cud->GetControllerPortModel(_name, _string);
    }
    Model* GetModel() const {
        if (_main) {
            return _mm->GetModel(_name);
        }
        else {
            return nullptr;
        }
    }
    bool IsMain() const { return _main; }
    bool IsOutline() const { return _outline; }
    virtual std::string GetType() const override { return "MODEL"; }
    virtual void Draw(wxDC& dc, wxPoint mouse, wxSize offset, float scale, bool printing = false, bool border = true) override {
        auto origBrush = dc.GetBrush();
        auto origPen = dc.GetPen();
        auto origText = dc.GetTextForeground();
        dc.SetTextForeground(*wxBLACK);

        Model* m = _mm->GetModel(_name);
        UDControllerPortModel* udcpm = GetUDModel();

        if (!border) {
            dc.SetPen(*wxTRANSPARENT_PEN);
        }
        else {
            dc.SetPen(__modelOutlinePen);
        }

        if (udcpm != nullptr) {
            switch (udcpm->GetSmartRemote()) {
            case 0:
                dc.SetBrush(__modelSRNoneBrush);
                break;
            case 1:
                dc.SetBrush(__modelSRABrush);
                break;
            case 2:
                dc.SetBrush(__modelSRBBrush);
                break;
            case 3:
                dc.SetBrush(__modelSRCBrush);
                break;
            }
        }

        if (_invalid) {
            dc.SetBrush(__invalidBrush);
        }

        _outline = false;
        if (_dragging && !printing) {
            dc.SetBrush(wxColour(255, 255, 128));
        }
        else if (HitTest(mouse) != HITLOCATION::NONE && !printing) {
            _outline = true;
            dc.SetPen(wxPen(dc.GetPen().GetColour(), 3));
        }

        auto location = _location * scale;

        wxSize sz = _size;
        sz = sz.Scale(scale, scale);
        dc.DrawRectangle(location + offset, sz);
        if (_over == HITLOCATION::LEFT) {
            dc.SetPen(__dropTargetPen);
            dc.SetBrush(__dropTargetBrush);
            wxSize ssz = wxSize((sz.x-2) / 2, sz.y-2);
            dc.DrawRectangle(location + offset + wxSize(1,1), ssz);
        }
        else if (_over == HITLOCATION::RIGHT) {
            dc.SetPen(__dropTargetPen);
            dc.SetBrush(__dropTargetBrush);
            wxSize ssz = wxSize((sz.x - 2) / 2, sz.y - 2);
            dc.DrawRectangle(location + offset + wxSize((sz.x / 2), 1), ssz);
        }

        wxPoint pt = location + wxSize(2, 2) + offset;
        DrawTextLimited(dc, _displayName, pt, sz - wxSize(4,4));
        pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
        if (m != nullptr) {
            auto iconType = "xlART_" + m->GetDisplayAs() + "_ICON";
            int iconSize = MODEL_ICON_SIZE;
            if (iconSize > 24) {
                iconSize = 32;
            } else {
                iconSize = 16;
            }

            wxBitmap bmp =  wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(iconType), wxART_LIST, wxDefaultSize);
            if (bmp.IsOk()) {
                int height = bmp.GetScaledHeight();
                if (height > iconSize) {
                    wxImage img = bmp.ConvertToImage();
                    img.Rescale(iconSize, iconSize);
                    bmp = wxBitmap(img);
                }
#ifdef __WXOSX__
                dc.DrawBitmap(bmp, location.x + sz.x - ScaleWithSystemDPI(GetSystemContentScaleFactor(), 3) - bmp.GetScaledWidth(), pt.y);
#else
                wxIcon icon;
                icon.CopyFromBitmap(bmp);
                dc.DrawIcon(icon, location.x + sz.x - ScaleWithSystemDPI(GetSystemContentScaleFactor(), 3) - bmp.GetScaledWidth(), pt.y);
#endif
            }

            if (udcpm != nullptr) {
                uint32_t chs = udcpm->Channels();
                if (_style & STYLE_PIXELS) {
                    DrawTextLimited(dc, wxString::Format("Pixels: %ld", (long)chs / udcpm->GetChannelsPerPixel()), pt, sz - wxSize(4, 4));
                    pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
                }
                if (_style & STYLE_CHANNELS) {
                    DrawTextLimited(dc, wxString::Format("Channels: %ld", (long)chs), pt, sz - wxSize(4, 4));
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
    virtual void AddRightClickMenu(wxMenu& mnu) override {
        if (_caps != nullptr && GetModel() != nullptr && GetModel()->IsPixelProtocol())
        {
            if(_caps->SupportsSmartRemotes())
            {
                mnu.AppendSeparator();
                int sr = GetModel()->GetSmartRemote();
                auto mi = mnu.AppendRadioItem(ControllerModelDialog::CONTROLLER_SMARTREMOTE_None, "None");
                mi->Check(sr == 0);
                mi = mnu.AppendRadioItem(ControllerModelDialog::CONTROLLER_SMARTREMOTE_A, "*A*->b->c");
                mi->Check(sr == 1);
                mi = mnu.AppendRadioItem(ControllerModelDialog::CONTROLLER_SMARTREMOTE_B, "a->*B*->c");
                mi->Check(sr == 2);
                mi = mnu.AppendRadioItem(ControllerModelDialog::CONTROLLER_SMARTREMOTE_C, "a->b->*C*");
                mi->Check(sr == 3);
                mi = mnu.AppendRadioItem(ControllerModelDialog::CONTROLLER_SMARTREMOTE_ABC, "*A*->*B*->*C*");
                mi->Check(sr == 4);
                mi = mnu.AppendRadioItem(ControllerModelDialog::CONTROLLER_SMARTREMOTE_BC, "a->*B*->*C*");
                mi->Check(sr == 5);
            }
            if (_caps->SupportsPixelPortBrightness())
            {
                mnu.AppendSeparator();
                mnu.Append(ControllerModelDialog::CONTROLLER_BRIGHTNESS, "Set Brightness");
                if (GetModel()->IsControllerBrightnessSet()) {
                    mnu.Append(ControllerModelDialog::CONTROLLER_BRIGHTNESSCLEAR, "Clear Brightness");
                }
            }
        }
        else if (GetModel() != nullptr && GetModel()->IsSerialProtocol()) {
            mnu.AppendSeparator();
            mnu.Append(ControllerModelDialog::CONTROLLER_DMXCHANNEL, "Set Channel");
        }
    }
    virtual bool HandlePopup(wxWindow* parent, int id) override {
        if (GetModel() == nullptr) return false;

        if (id == ControllerModelDialog::CONTROLLER_SMARTREMOTE_None) {
            GetModel()->SetSmartRemote(0);
            return true;
        }
        else if (id == ControllerModelDialog::CONTROLLER_SMARTREMOTE_A) {
            GetModel()->SetSmartRemote(1);
            return true;
        }
        else if (id == ControllerModelDialog::CONTROLLER_SMARTREMOTE_B) {
            GetModel()->SetSmartRemote(2);
            return true;
        }
        else if (id == ControllerModelDialog::CONTROLLER_SMARTREMOTE_C) {
            GetModel()->SetSmartRemote(3);
            return true;
        }
        else if (id == ControllerModelDialog::CONTROLLER_SMARTREMOTE_ABC) {
            GetModel()->SetSmartRemote(4);
            return true;
        }
        else if (id == ControllerModelDialog::CONTROLLER_SMARTREMOTE_BC) {
            GetModel()->SetSmartRemote(5);
            return true;
        }
        else if (id == ControllerModelDialog::CONTROLLER_DMXCHANNEL) {
            wxNumberEntryDialog dlg(parent, "Enter the DMX channel", "Channel", "DMX Channel", GetModel()->GetControllerDMXChannel(), 1, 512);
            if (dlg.ShowModal() == wxID_OK) {
                GetModel()->SetControllerDMXChannel(dlg.GetValue());
            }
            return true;
        }
        else if (id == ControllerModelDialog::CONTROLLER_BRIGHTNESS) {
            wxNumberEntryDialog dlg(parent, "Enter the Model Brightness", "Brightness", "Model Brightness", GetModel()->GetControllerBrightness(), 0, 100);
            if (dlg.ShowModal() == wxID_OK) {
                GetModel()->SetControllerBrightness(dlg.GetValue());
            }
            return true;
        }
        else if (id == ControllerModelDialog::CONTROLLER_BRIGHTNESSCLEAR) {
            GetModel()->ClearControllerBrightness();
            return true;
        }
        return false;
    }
};
#pragma endregion

#pragma region Drop Target
class CMDTextDropTarget : public wxTextDropTarget
{
public:
    CMDTextDropTarget(std::list<BaseCMObject*>* objects, ControllerModelDialog* owner, wxPanel* target, bool anywhere) { _owner = owner; _objects = objects; _target = target; _anywhere = anywhere; };

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override {
        if (data == "") return false;

        wxPoint mouse = wxPoint(x, y);
        mouse += _owner->GetScrollPosition(_target);

        if (data.StartsWith("Model:")) {
            _owner->DropFromModels(mouse, data.AfterFirst(':'), _target);
            return true;
        }
        else if (data.StartsWith("Controller:")) {
            _owner->DropFromController(mouse, data.AfterFirst(':').ToStdString(), _target);
            return true;
        }
        return false;
    }

    #define SCROLLMARGIN 20
    #define SCROLLBY 20
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override {
        // handle auto scroll while dragging
        if (y > _target->GetSize().GetHeight() - SCROLLMARGIN) {
            if (_owner->Scroll(_target, 0, SCROLLBY)) return wxDragNone;
        }
        else if (y < SCROLLMARGIN) {
            if (_owner->Scroll(_target, 0, -SCROLLBY)) return wxDragNone;
        }
        else if (x < SCROLLMARGIN) {
            if (_owner->Scroll(_target, -SCROLLBY, 0)) return wxDragNone;
        }
        else if (x > _target->GetSize().GetWidth() - SCROLLMARGIN) {
            if (_owner->Scroll(_target, SCROLLBY, 0)) return wxDragNone;
        }

        if (_anywhere) return wxDragMove;

        wxDragResult res = wxDragNone;
        BaseCMObject* port = nullptr;

        wxPoint mouse(x, y);
        mouse +=_owner->GetScrollPosition(_target);

        for (const auto& it : *_objects) {
            auto m = dynamic_cast<ModelCMObject*>(it);
            if (it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE && !_owner->IsDragging(m) && (m == nullptr || m->IsMain())) {
                it->SetOver(it->HitTest(mouse));
                _owner->Draw(_target, it, mouse);
                res  = wxDragMove;
            }
            else if (it->GetType() == "PORT" && it->HitYTest(mouse)) {
                port = it;
            }
            else if (it->GetOver() != BaseCMObject::HITLOCATION::NONE) {
                it->SetOver(BaseCMObject::HITLOCATION::NONE);
                _owner->Draw(_target, it, wxPoint(LEFT_RIGHT_MARGIN + 5, y));
            }
        }

        if (res == wxDragNone && port != nullptr) {
            port->SetOver(BaseCMObject::HITLOCATION::RIGHT);
            _owner->Draw(_target, port, wxPoint(LEFT_RIGHT_MARGIN + 5, y));
            res = wxDragMove;
        }
        return res;
    }

    std::list<BaseCMObject*>* _objects = nullptr;
    ControllerModelDialog* _owner = nullptr;
    wxPanel* _target = nullptr;
    bool _anywhere = false;
};
#pragma endregion

ControllerModelPrintout::ControllerModelPrintout(ControllerModelDialog* controllerDialog) {
    _controllerDialog = controllerDialog;
}

bool ControllerModelPrintout::OnPrintPage(int pageNum) {
    if (pageNum != 1) return false;

    wxRect rect = GetLogicalPageRect();
    wxBitmap bmp;
    bmp.Create(rect.GetWidth() * 0.95f, rect.GetHeight() * 0.95f);
    _controllerDialog->RenderPicture(bmp, true);

    wxDC* dc = GetDC();
    dc->DrawBitmap(bmp, 0, 0);

    return true;
}

ControllerModelDialog::ControllerModelDialog(wxWindow* parent, UDController* cud, ModelManager* mm, Controller* controller, wxWindowID id,const wxPoint& pos,const wxSize& size) :
    _cud(cud), _mm(mm), _controller(controller), _xLights((xLightsFrame*)parent) {
	//(*Initialize(ControllerModelDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;

	Create(parent, wxID_ANY, _("Controller Visualiser"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	SetClientSize(wxSize(500,500));
	FlexGridSizer1 = new wxFlexGridSizer(2, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	Panel3 = new wxPanel(this, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer5->AddGrowableRow(0);
	PanelController = new wxPanel(Panel3, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER, _T("ID_PANEL1"));
	FlexGridSizer5->Add(PanelController, 1, wxALL|wxEXPAND, 0);
	ScrollBar_Controller_V = new wxScrollBar(Panel3, ID_SCROLLBAR1, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL, wxDefaultValidator, _T("ID_SCROLLBAR1"));
	ScrollBar_Controller_V->SetScrollbar(0, 1, 10, 1);
	FlexGridSizer5->Add(ScrollBar_Controller_V, 1, wxALL|wxEXPAND, 0);
	ScrollBar_Controller_H = new wxScrollBar(Panel3, ID_SCROLLBAR2, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL, wxDefaultValidator, _T("ID_SCROLLBAR2"));
	ScrollBar_Controller_H->SetScrollbar(0, 1, 10, 1);
	FlexGridSizer5->Add(ScrollBar_Controller_H, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer5->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl_Check = new wxTextCtrl(Panel3, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(-1,100), wxTE_MULTILINE|wxTE_READONLY|wxALWAYS_SHOW_SB, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer5->Add(TextCtrl_Check, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel3->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(Panel3);
	FlexGridSizer5->SetSizeHints(Panel3);
	FlexGridSizer1->Add(Panel3, 1, wxALL|wxEXPAND, 5);
	Panel4 = new wxPanel(this, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(1);
	CheckBox_HideOtherControllerModels = new wxCheckBox(Panel4, ID_CHECKBOX1, _("Hide models assigned to other controllers"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_HideOtherControllerModels->SetValue(false);
	FlexGridSizer3->Add(CheckBox_HideOtherControllerModels, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	PanelModels = new wxPanel(Panel4, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER, _T("ID_PANEL2"));
	FlexGridSizer2->Add(PanelModels, 1, wxALL|wxEXPAND, 0);
	ScrollBar_Models = new wxScrollBar(Panel4, ID_SCROLLBAR3, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL, wxDefaultValidator, _T("ID_SCROLLBAR3"));
	ScrollBar_Models->SetScrollbar(0, 1, 10, 1);
	FlexGridSizer2->Add(ScrollBar_Models, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer3->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	Panel4->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel4);
	FlexGridSizer3->SetSizeHints(Panel4);
	FlexGridSizer1->Add(Panel4, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	PanelController->Connect(wxEVT_PAINT,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerPaint,0,this);
	PanelController->Connect(wxEVT_KEY_DOWN,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerKeyDown,0,this);
	PanelController->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerLeftDown,0,this);
	PanelController->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerLeftUp,0,this);
	PanelController->Connect(wxEVT_LEFT_DCLICK,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerLeftDClick,0,this);
	PanelController->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerRightDown,0,this);
	PanelController->Connect(wxEVT_MOTION,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerMouseMove,0,this);
	PanelController->Connect(wxEVT_ENTER_WINDOW,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerMouseEnter,0,this);
	PanelController->Connect(wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerMouseLeave,0,this);
	PanelController->Connect(wxEVT_MOUSEWHEEL,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerMouseWheel,0,this);
	PanelController->Connect(wxEVT_SIZE,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerResize,0,this);
	Connect(ID_SCROLLBAR1,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_VScroll);
	Connect(ID_SCROLLBAR1,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_VScrollThumbTrack);
	Connect(ID_SCROLLBAR1,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_VScrollChanged);
	Connect(ID_SCROLLBAR2,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_HScroll);
	Connect(ID_SCROLLBAR2,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_HScrollThumbTrack);
	Connect(ID_SCROLLBAR2,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_HScrollChanged);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ControllerModelDialog::OnCheckBox_HideOtherControllerModelsClick);
	PanelModels->Connect(wxEVT_PAINT,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsPaint,0,this);
	PanelModels->Connect(wxEVT_KEY_DOWN,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsKeyDown,0,this);
	PanelModels->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsLeftDown,0,this);
	PanelModels->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsLeftUp,0,this);
	PanelModels->Connect(wxEVT_LEFT_DCLICK,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsLeftDClick,0,this);
	PanelModels->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsRightDown,0,this);
	PanelModels->Connect(wxEVT_MOTION,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsMouseMove,0,this);
	PanelModels->Connect(wxEVT_ENTER_WINDOW,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsMouseEnter,0,this);
	PanelModels->Connect(wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsMouseLeave,0,this);
	PanelModels->Connect(wxEVT_MOUSEWHEEL,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsMouseWheel,0,this);
	PanelModels->Connect(wxEVT_SIZE,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsResize,0,this);
	Connect(ID_SCROLLBAR3,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_ModelsScroll);
	Connect(ID_SCROLLBAR3,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_ModelsScrollThumbTrack);
	Connect(ID_SCROLLBAR3,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_ModelsScrollChanged);
	//*)

    Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&ControllerModelDialog::OnKeyDown, 0, this);

    //PanelController->SetBackgroundStyle(wxBG_STYLE_PAINT);
    //PanelModels->SetBackgroundStyle(wxBG_STYLE_PAINT);

    wxSize panel4Size = wxSize(std::max((int)(LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + LEFT_RIGHT_MARGIN + ScrollBar_Models->GetSize().GetWidth() + 10), (int)CheckBox_HideOtherControllerModels->GetSize().GetWidth()), -1);
    Panel4->SetMinSize(panel4Size);
    SetMinSize(wxSize(800, 400));
    SetSize(wxSize(1200, 800));

    _autoLayout = _controller->IsAutoLayout();

    _title = controller->GetLongDescription();
    SetLabel(_title);

    _caps = ControllerCaps::GetControllerConfig(_controller);

    CMDTextDropTarget* cmdt = new CMDTextDropTarget(&_controllers, this, PanelController, false);
    PanelController->SetDropTarget(cmdt);

    cmdt = new CMDTextDropTarget(&_models, this, PanelModels, true);
    PanelModels->SetDropTarget(cmdt);

    bool changed = false;

    if (_autoLayout) {
        // If you are doing auto layout then all models must have controller name set ... this may much up model chaining but it has to be done
        // or things will get funky
        if (_cud->SetAllModelsToControllerName(_controller->GetName())) {
            wxMessageBox("At least one model had to have its controller name set because you are using Auto Layout. This may have mucked up the order of model chaining on some ports and you will need to fix that up here.");
            changed = true;
        }
    }
    else
    {
        CheckBox_HideOtherControllerModels->Enable(false);
    }

    if (_caps != nullptr)
    {
        changed |= _cud->SetAllModelsToValidProtocols(_caps->GetPixelProtocols(), _caps->GetSerialProtocols(), !_caps->SupportsMultipleSimultaneousOutputProtocols());

        if (!_caps->SupportsSmartRemotes())
        {
            changed |= _cud->ClearSmartRemoteOnAllModels();
        }
    }

    if (changed) {
        // Now need to let all the recalculations work
        while (!_xLights->DoAllWork());
    }

    ReloadModels();
    Layout();
}

ControllerModelDialog::~ControllerModelDialog() {
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

void ControllerModelDialog::ReloadModels()
{
    _cud->Rescan(true);
    std::string check;
    if (_caps != nullptr) {
        _cud->Check(_caps, check);
        TextCtrl_Check->SetValue(check);
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

    int y = TOP_BOTTOM_MARGIN;
    for (const auto& it : *_mm) {
        if (it.second->GetDisplayAs() != "ModelGroup") {
            if (_cud->GetControllerPortModel(it.second->GetName(), 0) == nullptr &&
                ((_autoLayout && !CheckBox_HideOtherControllerModels->GetValue()) ||
                 ((_autoLayout && CheckBox_HideOtherControllerModels->GetValue() && (it.second->GetController() == nullptr || _controller->ContainsChannels(it.second->GetFirstChannel(), it.second->GetLastChannel()))) ||
                 _controller->ContainsChannels(it.second->GetFirstChannel(), it.second->GetLastChannel())))) {
                _models.push_back(new ModelCMObject(it.second->GetName(), it.second->GetName(), _mm, _cud, _caps, wxPoint(5, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_STRINGS));
                y += VERTICAL_GAP + VERTICAL_SIZE;
            }
        }
    }

    _modelsy = y;

    int panely = PanelModels->GetSize().y;
    ScrollBar_Models->SetRange(_modelsy);
    ScrollBar_Models->SetPageSize(_modelsy / 10);
    ScrollBar_Models->SetThumbSize(panely);
    PanelModels->Refresh();

    y = VERTICAL_GAP;

    std::list<int> pixelPortsWithSmartRemotes;

    int maxx = 0;
    for (int i = 0; i < std::max((_caps == nullptr ? 0 : _caps->GetMaxPixelPort()), _cud->GetMaxPixelPort()); i++) {
        auto cmp = new PortCMObject(PortCMObject::PORTTYPE::PIXEL, i + 1, _cud, _caps, wxPoint(LEFT_RIGHT_MARGIN, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_PIXELS, i+1 > (_caps == nullptr ? _cud->GetMaxPixelPort() : _caps->GetMaxPixelPort()));
        _controllers.push_back(cmp);

        auto pp = _cud->GetControllerPixelPort(i + 1);
        if (pp != nullptr) {
            if (_caps == nullptr || _caps->SupportsVirtualStrings()) {
                pp->CreateVirtualStrings(_caps == nullptr ? true : _caps->MergeConsecutiveVirtualStrings());
                if (pp->GetVirtualStringCount() == 0) {
                    y += VERTICAL_GAP + VERTICAL_SIZE;
                }
                else {
                    for (const auto& it : pp->GetVirtualStrings()) {
                        int x = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + HORIZONTAL_GAP;
                        for (const auto& it2 : it->_models) {
                            if (it2->GetModel() != nullptr) {
                                if (it2->GetModel()->GetSmartRemote() != 0) pixelPortsWithSmartRemotes.push_back(i + 1);
                                auto cmm = new ModelCMObject(it2->GetModel()->GetName(), it2->GetName(), _mm, _cud, _caps, wxPoint(x, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_PIXELS);
                                _controllers.push_back(cmm);
                                x += HORIZONTAL_SIZE + HORIZONTAL_GAP;
                            }
                        }
                        if (x > maxx) maxx = x;
                        y += VERTICAL_GAP + VERTICAL_SIZE;
                    }
                }
            }
            else {
                int x = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + HORIZONTAL_GAP;
                for (const auto& it : pp->GetModels()) {
                    if (it->GetModel() != nullptr) {
                        auto cmm = new ModelCMObject(it->GetModel()->GetName(), it->GetName(), _mm, _cud, _caps, wxPoint(x, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_PIXELS);
                        _controllers.push_back(cmm);
                        x += HORIZONTAL_SIZE + HORIZONTAL_GAP;
                    }
                }
                if (x > maxx) maxx = x;
                y += VERTICAL_GAP + VERTICAL_SIZE;
            }
        }
    }

    for (int i = 0; i < std::max((_caps == nullptr ? 0 : _caps->GetMaxSerialPort()), _cud->GetMaxSerialPort()); i++) {
        _controllers.push_back(new PortCMObject(PortCMObject::PORTTYPE::SERIAL, i + 1, _cud, _caps, wxPoint(LEFT_RIGHT_MARGIN, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_CHANNELS, i + 1 > (_caps == nullptr ? _cud->GetMaxSerialPort() : _caps->GetMaxSerialPort())));
        auto sp = _cud->GetControllerSerialPort(i + 1);
        if (sp != nullptr) {
            int x = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + HORIZONTAL_GAP;
            for (const auto& it : sp->GetModels()) {
                auto cmm = new ModelCMObject(it->GetName(), it->GetName(), _mm, _cud, _caps, wxPoint(x, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_CHANNELS);
                _controllers.push_back(cmm);
                x += HORIZONTAL_SIZE + HORIZONTAL_GAP;
            }
            if (x > maxx) maxx = x;
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

void ControllerModelDialog::OnPopupCommand(wxCommandEvent &event) {
    int id = event.GetId();
    if (id == CONTROLLERModel_PRINT) {
        static wxPrintDialogData printDialogData;
        wxPrinter printer(&printDialogData);

        ControllerModelPrintout printout(this);

        if (!printer.Print(this, &printout, true)) {
            if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
                DisplayError(wxString::Format("Problem printing controller layout. %d", wxPrinter::GetLastError()).ToStdString(), this);
            }
        }
        else {
            printDialogData = printer.GetPrintDialogData();
        }
    }
    else if (id == CONTROLLERModel_SAVE_CSV) {
        SaveCSV();
    }
    else if (_popup != nullptr) {
        if (_popup->HandlePopup(this, id)) {
            while (!_xLights->DoAllWork());
            ReloadModels();
        }
    }
}

void ControllerModelDialog::RenderPicture(wxBitmap& bitmap, bool printer) {

    float maxx = ScrollBar_Controller_H->GetRange();
    float maxy = ScrollBar_Controller_V->GetRange();
    wxSize dcSize = bitmap.GetSize() - wxSize(10 * LEFT_RIGHT_MARGIN, 10 * TOP_BOTTOM_MARGIN);
    float scale = std::min((float)dcSize.x / maxx, (float)dcSize.y / maxy);

    wxMemoryDC dc;
    dc.SelectObject(bitmap);

    dc.SetTextForeground(*wxBLACK);

    dc.SetPen(*wxWHITE_PEN);
    dc.SetBrush(*wxWHITE_BRUSH);

    dc.DrawRectangle(wxPoint(0, 0), bitmap.GetScaledSize());

    dc.SetDeviceOrigin(0, 0);

    int fontSize = 10 * scale;
    wxFont font = wxFont(wxSize(0, fontSize), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("Arial"), wxFONTENCODING_DEFAULT);
    dc.SetFont(font);

    int rowPos = TOP_BOTTOM_MARGIN * scale;
    dc.DrawText(_title, LEFT_RIGHT_MARGIN * scale, rowPos);
    rowPos += ((VERTICAL_SIZE / 2) * scale) + (VERTICAL_GAP * scale);

    for (const auto& it : _controllers) {
        it->Draw(dc, wxPoint(0, 0), wxSize(0, rowPos), scale, true);
    }
}

void ControllerModelDialog::SaveCSV() {
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, _title, wxEmptyString, "Export files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty()) return;

    std::vector<wxString> lines;
    int columnSize = 0;

    for (int i = 1; i <= _cud->GetMaxPixelPort(); i++) {
        wxString line = wxString::Format("Pixel Port %d,", i);

        if (columnSize < _cud->GetControllerPixelPort(i)->GetModels().size())
            columnSize = _cud->GetControllerPixelPort(i)->GetModels().size();

        for (const auto& it : _cud->GetControllerPixelPort(i)->GetModels()) {
            if (it->GetSmartRemote() > 0) {
                char remote = ('@' + it->GetSmartRemote());
                line += "Remote ";
                line += remote;
                line += ":";
            }
            line += it->GetName();
            line += ",";
        }

        line += "\n";
        lines.push_back(line);
    }
    lines.push_back("\n");
    for (int i = 1; i <= _cud->GetMaxSerialPort(); i++) {
        if (columnSize < _cud->GetControllerSerialPort(i)->GetModels().size())
            columnSize = _cud->GetControllerSerialPort(i)->GetModels().size();

        wxString line = wxString::Format("Serial Port %d,", i);
        for (const auto& it : _cud->GetControllerSerialPort(i)->GetModels()) {
            line += it->GetName();
            line += ",";
        }
        line += "\n";
        lines.push_back(line);
    }

    wxFile f(filename);

    if (!f.Create(filename, true) || !f.IsOpened()) {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
        return;
    }

    wxString header = _title + "\nOutput,";
    for (int i = 1; i <= columnSize; i++) {
        header += wxString::Format("Model %d,", i);
    }
    header += "\n";
    f.Write(header);
    for (const auto& line : lines) {
        f.Write(line);
    }
    f.Close();
}

// Ensure DMX channels increase as you move left to right
void ControllerModelDialog::FixDMXChannels() {
    bool changed = false;
    for (int i = 0; i < _cud->GetMaxSerialPort(); i++) {
        int next = -1;
        auto p = _cud->GetControllerSerialPort(i + 1);
        for (const auto& it : p->GetModels()) {
            if (next == -1) {
                next = it->GetModel()->GetControllerDMXChannel();
                if (next == 0) next = 1;
                next += it->GetModel()->GetChanCount();
            }
            else {
                if (it->GetModel()->GetControllerDMXChannel() < next) {
                    changed = true;
                    it->GetModel()->SetControllerDMXChannel(next);
                }
                next = it->GetModel()->GetControllerDMXChannel() + it->GetModel()->GetChanCount();
            }
        }
    }

    while (!_xLights->DoAllWork());

    // if we changed anything we need to scan it one more time
    if (changed) {
        _cud->Rescan(true);
    }
}

void ControllerModelDialog::DropFromModels(const wxPoint& location, const std::string& name, wxPanel* target) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    auto m = _mm->GetModel(name);
    wxASSERT(m != nullptr);

    logger_base.debug("Model %s dropped from models pane.", (const char*)name.c_str());

    // model dragged from models
    if (target == PanelModels) {
        // do nothing
        logger_base.debug("   onto the models pane ... nothing to do.");
    }
    else {
        logger_base.debug("    onto the controller pane.");

        auto port = GetControllerPortAtLocation(location);
        if (port != nullptr) {
            logger_base.debug("    onto port %d.", port->GetPort());
            m->SetControllerPort(port->GetPort());
            if (port->GetPortType() == PortCMObject::PORTTYPE::PIXEL) {
                if (port->GetModelCount() == 0) {
                    if (_caps != nullptr && !_caps->IsValidPixelProtocol(m->GetControllerProtocol())) m->SetControllerProtocol(_caps->GetPixelProtocols().front());
                    if (!m->IsPixelProtocol()) m->SetControllerProtocol("WS2811");
                }
                else {
                    m->SetControllerProtocol(port->GetFirstModel()->GetControllerProtocol());
                }
            }
            else {
                if (port->GetModelCount() == 0) {
                    if (_caps != nullptr && !_caps->IsValidSerialProtocol(m->GetControllerProtocol())) m->SetControllerProtocol(_caps->GetSerialProtocols().front());
                    if (!m->IsSerialProtocol()) m->SetControllerProtocol("DMX");
                }
                else {
                    m->SetControllerProtocol(port->GetFirstModel()->GetControllerProtocol());
                }
            }

            if (_autoLayout) {
                m->SetControllerName(_controller->GetName());
            }

            auto ob = GetControllerCMObjectAt(location);
            ModelCMObject* mob = dynamic_cast<ModelCMObject*>(ob);
            BaseCMObject::HITLOCATION hit = BaseCMObject::HITLOCATION::NONE;
            if (ob != nullptr) hit = ob->HitTest(location);

            if (mob == nullptr || !mob->IsMain()) {
                logger_base.debug("    Processing it as a drop onto the port ... so setting it to end.");

                // dropped on a port .. or not on the first string of a model
                // If no model already there put it at the beginning ... else chain it to end
                if (_autoLayout) {
                    auto fmud = port->GetUDPort()->GetLastModel();
                    if (fmud != nullptr && fmud->IsFirstModelString()) {
                        Model* lm = fmud->GetModel();
                        if (lm != nullptr) {
                            m->SetModelChain(">" + lm->GetName());
                        }
                    }
                    else {
                        m->SetModelChain("");
                    }
                }
            }
            else {
                Model* droppedOn = _mm->GetModel(mob->GetName());
                logger_base.debug("    Dropped onto model %s.", (const char*)droppedOn->GetName().c_str());

                // dropped on a model
                if (_autoLayout) {
                    m->SetSmartRemote(droppedOn->GetSmartRemote());
                    if (hit == BaseCMObject::HITLOCATION::LEFT) {
                        logger_base.debug("    On the left hand side.");
                        logger_base.debug("    Left of %s which comes after %s.", (const char*)droppedOn->GetName().c_str(), (const char*)droppedOn->GetModelChain().c_str());
                        m->SetModelChain(droppedOn->GetModelChain());
                        droppedOn->SetModelChain(">" + m->GetName());
                    }
                    else {
                        logger_base.debug("    On the right hand side.");
                        Model* next = port->GetUDPort()->GetModelAfter(droppedOn);
                        if (next != nullptr) {
                            logger_base.debug("    Right of %s which comes before %s.", (const char*)droppedOn->GetName().c_str(), (const char*)next->GetName().c_str());
                            next->SetModelChain(">" + m->GetName());
                        }
                        else {
                            logger_base.debug("    Right of %s.", (const char*)droppedOn->GetName().c_str());
                        }
                        m->SetModelChain(">" + droppedOn->GetName());
                    }
                }
            }
        }
        else {
            logger_base.debug("    but not onto a port ... so nothing to do.");
        }

        while (!_xLights->DoAllWork());
        ReloadModels();
    }
}

void ControllerModelDialog::DropFromController(const wxPoint& location, const std::string& name, wxPanel* target) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Model %s dropped from controllers pane.", (const char*)name.c_str());

    // model dragged from controllers
    auto m = _mm->GetModel(name);
    if (m == nullptr) return;

    if (target == PanelModels) {
        logger_base.debug("   onto the models pane ... so remove the model from the controller.");
        // Removing a model from the controller
        if (_autoLayout) {
            m->SetControllerName("");
            m->SetModelChain("");
        }
        m->SetControllerPort(0);
    }
    else {
        logger_base.debug("    onto the controller pane.");

        auto port = GetControllerPortAtLocation(location);
        if (port != nullptr) {
            logger_base.debug("    onto port %d.", port->GetPort());
            m->SetControllerPort(port->GetPort());
            if (port->GetPortType() == PortCMObject::PORTTYPE::PIXEL) {
                if (port->GetModelCount() == 0) {
                    if (_caps != nullptr && !_caps->IsValidPixelProtocol(m->GetControllerProtocol())) m->SetControllerProtocol(_caps->GetPixelProtocols().front());
                    if (!m->IsPixelProtocol()) m->SetControllerProtocol("WS2811");
                }
                else {
                    m->SetControllerProtocol(port->GetFirstModel()->GetControllerProtocol());
                }
            }
            else {
                if (port->GetModelCount() == 0) {
                    if (_caps != nullptr && !_caps->IsValidSerialProtocol(m->GetControllerProtocol())) m->SetControllerProtocol(_caps->GetSerialProtocols().front());
                    if (!m->IsSerialProtocol()) m->SetControllerProtocol("DMX");
                }
                else {
                    m->SetControllerProtocol(port->GetFirstModel()->GetControllerProtocol());
                }
            }

            if (_autoLayout) {
                m->SetControllerName(_controller->GetName());
            }

            auto ob = GetControllerCMObjectAt(location);
            ModelCMObject* mob = dynamic_cast<ModelCMObject*>(ob);
            BaseCMObject::HITLOCATION hit = BaseCMObject::HITLOCATION::NONE;
            if (ob != nullptr) hit = ob->HitTest(location);

            if (mob == nullptr || !mob->IsMain()) {
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
                    }
                    else {
                        if (fmud != nullptr && fmud->IsFirstModelString()) {
                            Model* lm = fmud->GetModel();
                            if (lm != nullptr) {
                                m->SetModelChain(">" + lm->GetName());
                            }
                        }
                    }
                }
            }
            else {
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
                        }
                        else {
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
                    }
                    else {
                        logger_base.debug("    On the right hand side.");
                        if (m->GetModelChain() != ">" + droppedOn->GetName()) {
                            Model* next = port->GetUDPort()->GetModelAfter(droppedOn);
                            if (next != nullptr) {
                                logger_base.debug("    Right of %s which comes before %s.", (const char*)droppedOn->GetName().c_str(), (const char*)next->GetName().c_str());
                                next->SetModelChain(">" + m->GetName());
                            }
                            else {
                                logger_base.debug("    Right of %s.", (const char*)droppedOn->GetName().c_str());
                            }
                            m->SetModelChain(">" + droppedOn->GetName());
                        }
                    }
                }
            }
        }
        else {
            logger_base.debug("    but not onto a port ... so nothing to do.");
        }
    }

    while (!_xLights->DoAllWork());
    ReloadModels();
}

void ControllerModelDialog::OnPanelControllerLeftDown(wxMouseEvent& event)
{
    wxPoint mouse = event.GetPosition();
    mouse += GetScrollPosition(PanelController);

    for (const auto& it : _controllers) {
        if (it->GetType() == "MODEL" && it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE) {
            auto m = dynamic_cast<ModelCMObject*>(it);
            if (m->IsMain()) {
                wxTextDataObject dragData("Controller:" + ((ModelCMObject*)it)->GetName());

                wxBitmap bmp(32, 32);
                wxMemoryDC dc;
                dc.SelectObject(bmp);
                it->Draw(dc, wxPoint(-4, -4), wxSize(-1 * it->GetRect().GetLeft(), -1 * it->GetRect().GetTop()), 1, false, false);

#ifdef __linux__
                wxIcon dragCursor;
                dragCursor.CopyFromBitmap(bmp.ConvertToImage());
#else
                wxCursor dragCursor(bmp.ConvertToImage());
#endif

                wxDropSource dragSource(this, dragCursor, dragCursor, dragCursor);

                _dragging = dynamic_cast<ModelCMObject*>(it);

                dragSource.SetData(dragData);
                dragSource.DoDragDrop(wxDragMove);

                _dragging = nullptr;
            }
            break;
        }
    }
}

void ControllerModelDialog::OnPanelControllerKeyDown(wxKeyEvent& event)
{
    ScrollToKey(event.GetKeyCode());
}

void ControllerModelDialog::OnPanelControllerLeftUp(wxMouseEvent& event)
{
    _dragging = nullptr;
}

void ControllerModelDialog::OnPanelControllerLeftDClick(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelControllerMouseMove(wxMouseEvent& event)
{
    wxPoint mouse = event.GetPosition();
    mouse += GetScrollPosition(PanelController);

    if (_dragging != nullptr) {
        for (const auto& it : _controllers) {
            auto hit = it->HitTest(mouse);
            if (hit != BaseCMObject::HITLOCATION::NONE) {
                if (it->GetType() == "PORT") {
                    it->SetOver(BaseCMObject::HITLOCATION::RIGHT);
                    wxRect rect = it->GetRect();
                    rect.Offset(-1 * GetScrollPosition(PanelController));
                    PanelController->RefreshRect(rect);
                }
                else {
                    if (it->GetOver() != hit) {
                        it->SetOver(hit);
                        wxRect rect = it->GetRect();
                        rect.Offset(-1 * GetScrollPosition(PanelController));
                        PanelController->RefreshRect(rect);
                    }
                }
            }
            else {
                ClearOver(PanelController, _controllers);
            }
        }
    }
    else {
        std::string tt = "";
        for (const auto& it : _controllers) {
            bool ishit = it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE;
            auto m = dynamic_cast<ModelCMObject*>(it);
            if (ishit || (m != nullptr && m->IsOutline())) {
                wxRect rect = it->GetRect();
                rect.Offset(-1 * GetScrollPosition(PanelController));
                PanelController->RefreshRect(rect);
                if (ishit) {
                    if (m != nullptr) {
                        tt = GetModelTooltip(m);
                    }
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

void ControllerModelDialog::OnPanelControllerMouseLeave(wxMouseEvent& event) {
    ClearOver(PanelController, _controllers);
}

void ControllerModelDialog::ClearOver(wxPanel* panel, std::list<BaseCMObject*> list) {
    for (const auto& it : list) {
        if (it->GetOver() != BaseCMObject::HITLOCATION::NONE) {
            it->SetOver(BaseCMObject::HITLOCATION::NONE);
            wxRect rect = it->GetRect();
            rect.Offset(-1 * GetScrollPosition(panel));
            panel->RefreshRect(rect);
        }
    }
}

std::string ControllerModelDialog::GetModelTooltip(ModelCMObject* mob)
{
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

    auto m = mob->GetModel();
    if (m == nullptr) return "";

    std::string sr;
    switch (m->GetSmartRemote()) {
    case 0:
        sr = "None";
        break;
    case 1:
        sr = "A";
        break;
    case 2:
        sr = "B";
        break;
    case 3:
        sr = "C";
        break;
    case 4:
        sr = "A->B->C";
        break;
    case 5:
        sr = "B->C";
        break;
    default:
        sr = "error";
        break;
    }
    _xLights->GetControllerDetailsForChannel(m->GetFirstChannel() + 1, controllerName, type, protocol, description,
                                             channelOffset, ip, universe, inactive, baud, startUniverse, endUniverse);

    std::string dmx;
    std::string stringSettings;
    if (m->IsSerialProtocol()) {
        dmx = wxString::Format("\nChannel %d", m->GetControllerDMXChannel());
    }
    else {
        UDControllerPortModel* udm = mob->GetUDModel();
        if (udm != nullptr) {
            if (udm->GetBrightness(-1) != -1) {
                stringSettings += wxString::Format("\nBrightness: %d%%", udm->GetBrightness(-1));
            }
            if (udm->GetColourOrder("xxx") != "xxx")
            {
                stringSettings += wxString::Format("\nColor Order: %s", udm->GetColourOrder(""));
            }
            if (udm->GetDirection("xxx") != "xxx")
            {
                stringSettings += wxString::Format("\nDirection: %s", udm->GetDirection(""));
            }
            if (udm->GetGamma(-1) != -1)
            {
                stringSettings += wxString::Format("\nGamma: %.1f", (float)udm->GetGamma(0) / 10.0);
            }
            if (udm->GetGroupCount(-1) != -1)
            {
                stringSettings += wxString::Format("\nGrouping: %d", udm->GetGroupCount(0));
            }
            if (udm->GetNullPixels(-1) != -1)
            {
                stringSettings += wxString::Format("\nNull Pixels: %d", udm->GetNullPixels(0));
            }
        }
    }

    auto om = _xLights->GetOutputManager();
    if (_autoLayout) {
        return wxString::Format("Name: %s\nController Name: %s\nModel Chain: %s\nStart Channel: %s\nEnd Channel %s\nStrings %d\nSmart Remote: %s\nPort: %d\nProtocol: %s%s%s",
            mob->GetDisplayName(), controllerName, m->GetModelChain() == "" ? "Beginning" : m->GetModelChain(), m->GetStartChannelInDisplayFormat(om),
            m->GetLastChannelInStartChannelFormat(om),
            m->GetNumPhysicalStrings(), sr, m->GetControllerPort(), m->GetControllerProtocol(), dmx, stringSettings).ToStdString();
    }
    else {
        return wxString::Format("name: %s\nController Name: %s\nIP/Serial: %s\nStart Channel: %s\nEnd Channel %s\nStrings %d\nSmart Remote: %s\nPort: %d\nProtocol: %s%s%s",
            mob->GetDisplayName(), controllerName, universe, m->GetStartChannelInDisplayFormat(om), m->GetLastChannelInStartChannelFormat(om),
            m->GetNumPhysicalStrings(), sr, m->GetControllerPort(), m->GetControllerProtocol(), dmx, stringSettings).ToStdString();
    }
}

bool ControllerModelDialog::Scroll(wxPanel* panel, int scrollByX, int scrollByY)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool scrolled = false;

    // we dont scroll models panel
    if (panel == PanelModels) return scrolled;

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
    }
    else if (scrollByX < 0 && ScrollBar_Controller_H->GetThumbPosition() > 0) {
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
    }
    else if (scrollByY < 0 && ScrollBar_Controller_V->GetThumbPosition() > 0) {
        scrolled = true;
        ScrollBar_Controller_V->SetThumbPosition(ScrollBar_Controller_V->GetThumbPosition() + scrollByY);
        if (ScrollBar_Controller_V->GetThumbPosition() < 0) {
            ScrollBar_Controller_V->SetThumbPosition(0);
        }
    }

    if (scrolled) PanelController->Refresh();

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
    mouse += GetScrollPosition(PanelController);

    wxMenu mnu;
    mnu.Append(CONTROLLERModel_PRINT, "Print");
    mnu.Append(CONTROLLERModel_SAVE_CSV, "Save As CSV...");

    BaseCMObject* cm = GetControllerCMObjectAt(mouse);
    if (cm != nullptr) {
        cm->AddRightClickMenu(mnu);
    }
    _popup = cm;

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&ControllerModelDialog::OnPopupCommand, nullptr, this);
    PopupMenu(&mnu);
}

void ControllerModelDialog::OnPanelControllerPaint(wxPaintEvent& event)
{
    //wxAutoBufferedPaintDC dc(PanelController);
    wxPaintDC dc(PanelController);

    int xOffset = ScrollBar_Controller_H->GetThumbPosition();
    int yOffset = ScrollBar_Controller_V->GetThumbPosition();

    dc.SetDeviceOrigin(-xOffset, -yOffset);

    wxPoint mouse = PanelController->ScreenToClient(wxGetMousePosition());
    mouse += GetScrollPosition(PanelController);

    wxFont font = wxFont(wxSize(0, FONT_PIXEL_SIZE), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("Arial"), wxFONTENCODING_DEFAULT);
    dc.SetFont(font);

    for (const auto& it : _controllers) {
        it->Draw(dc, mouse, wxSize(0,0), 1, false);
    }
}

void ControllerModelDialog::Draw(wxPanel* panel, BaseCMObject* object, wxPoint mouse)
{
    wxClientDC dc(panel);

    int xOffset = ScrollBar_Controller_H->GetThumbPosition();
    int yOffset = ScrollBar_Controller_V->GetThumbPosition();

    if (panel == PanelModels) {
        xOffset = 0;
        yOffset = ScrollBar_Models->GetThumbPosition();
    }
    dc.SetDeviceOrigin(-xOffset, -yOffset);
    wxFont font = wxFont(wxSize(0, FONT_PIXEL_SIZE), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("Arial"), wxFONTENCODING_DEFAULT);
    dc.SetFont(font);

    object->Draw(dc, mouse, wxSize(0, 0), 1, false);
}

void ControllerModelDialog::OnScrollBar_Controller_HScroll(wxScrollEvent& event) {
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_HScrollThumbTrack(wxScrollEvent& event) {
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_HScrollChanged(wxScrollEvent& event) {
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_VScroll(wxScrollEvent& event) {
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_VScrollThumbTrack(wxScrollEvent& event) {
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_VScrollChanged(wxScrollEvent& event) {
    PanelController->Refresh();
}

#pragma region Models Panel
void ControllerModelDialog::OnScrollBar_ModelsScroll(wxScrollEvent& event) {
    PanelModels->Refresh();
}

void ControllerModelDialog::OnScrollBar_ModelsScrollThumbTrack(wxScrollEvent& event) {
    PanelModels->Refresh();
}

void ControllerModelDialog::OnScrollBar_ModelsScrollChanged(wxScrollEvent& event) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Models Scrollbar: %d", ScrollBar_Models->GetThumbPosition());
    PanelModels->Refresh();
}

void ControllerModelDialog::OnPanelModelsPaint(wxPaintEvent& event)
{
    //wxAutoBufferedPaintDC dc(PanelModels);
    wxPaintDC dc(PanelModels);

    int yOffset = ScrollBar_Models->GetThumbPosition();

    dc.SetDeviceOrigin(0, -yOffset);

    wxPoint mouse = PanelModels->ScreenToClient(wxGetMousePosition());
    mouse += GetScrollPosition(PanelModels);
    wxFont font = wxFont(wxSize(0, FONT_PIXEL_SIZE), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("Arial"), wxFONTENCODING_DEFAULT);
    dc.SetFont(font);

    for (const auto& it : _models) {
        it->Draw(dc, mouse, wxSize(0, 0), 1, false);
    }
}

void ControllerModelDialog::ScrollToKey(int keyCode)
{
    if (keyCode >= 65 && keyCode <= 90) keyCode += 32;
    for (const auto& it : _models)
    {
        ModelCMObject* m = dynamic_cast<ModelCMObject*>(it);
        if (m != nullptr && m->NameStartsWith((char)keyCode))
        {
            ScrollBar_Models->SetThumbPosition(m->GetRect().GetPosition().y);
            PanelModels->Refresh();
            break;
        }
    }
}

void ControllerModelDialog::OnKeyDown(wxKeyEvent& event)
{
    ScrollToKey(event.GetKeyCode());
}

void ControllerModelDialog::OnPanelModelsKeyDown(wxKeyEvent& event)
{
    ScrollToKey(event.GetKeyCode());
}

void ControllerModelDialog::OnPanelModelsLeftDown(wxMouseEvent& event) {
    wxPoint mouse = event.GetPosition();
    mouse += GetScrollPosition(PanelModels);

    for (const auto& it : _models) {
        if (it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE) {
            wxTextDataObject dragData("Model:" + ((ModelCMObject*)it)->GetName());

            wxBitmap bmp(32,32);
            wxMemoryDC dc;
            dc.SelectObject(bmp);
            it->Draw(dc, wxPoint(-4, -4), wxSize(-1 * it->GetRect().GetLeft(), -1 * it->GetRect().GetTop()), 1, false, false);

#ifdef __linux__
            wxIcon dragCursor;
            dragCursor.CopyFromBitmap(bmp.ConvertToImage());
#else
            wxCursor dragCursor(bmp.ConvertToImage());
#endif

            wxDropSource dragSource(this, dragCursor, dragCursor, dragCursor);

            _dragging = dynamic_cast<ModelCMObject*>(it);

            dragSource.SetData(dragData);
            dragSource.DoDragDrop(wxDragMove);

            _dragging = nullptr;
            break;
        }
    }
}

void ControllerModelDialog::OnPanelModelsLeftUp(wxMouseEvent& event) {
    _dragging = nullptr;
}

void ControllerModelDialog::OnPanelModelsLeftDClick(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelModelsRightDown(wxMouseEvent& event) {
    wxPoint mouse = event.GetPosition();
    mouse += GetScrollPosition(PanelModels);

    wxMenu mnu;

    BaseCMObject* cm = GetModelsCMObjectAt(mouse);
    if (cm != nullptr) {
        cm->AddRightClickMenu(mnu);
    }
    _popup = cm;

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&ControllerModelDialog::OnPopupCommand, nullptr, this);
    PopupMenu(&mnu);
}

void ControllerModelDialog::OnPanelModelsMouseMove(wxMouseEvent& event) {
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

void ControllerModelDialog::OnPanelModelsMouseWheel(wxMouseEvent& event) {
    int position = ScrollBar_Models->GetThumbPosition();
    position -= event.GetWheelRotation();
    ScrollBar_Models->SetThumbPosition(position);
    PanelModels->Refresh();
}

void ControllerModelDialog::OnPanelModelsResize(wxSizeEvent& event) {
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
void ControllerModelDialog::OnPanelControllerMouseWheel(wxMouseEvent& event) {
    if (event.ShiftDown()) {
        int position = ScrollBar_Controller_H->GetThumbPosition();
        position -= event.GetWheelRotation();
        ScrollBar_Controller_H->SetThumbPosition(position);
    }
    else {
        int position = ScrollBar_Controller_V->GetThumbPosition();
        position -= event.GetWheelRotation();
        ScrollBar_Controller_V->SetThumbPosition(position);
    }

    PanelController->Refresh();
}

void ControllerModelDialog::OnPanelControllerResize(wxSizeEvent& event) {
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

BaseCMObject* ControllerModelDialog::GetControllerCMObjectAt(wxPoint mouse) {
    for (const auto& it : _controllers) {
        if (it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE) return it;
    }
    return nullptr;
}

BaseCMObject* ControllerModelDialog::GetModelsCMObjectAt(wxPoint mouse) {
    for (const auto& it : _models) {
        if (it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE) return it;
    }
    return nullptr;
}

PortCMObject* ControllerModelDialog::GetControllerPortAtLocation(wxPoint mouse) {
    BaseCMObject* last = nullptr;
    for (const auto& it : _controllers) {
        if (it->GetType() == "PORT") {
            if (it->HitYTest(mouse)) return dynamic_cast<PortCMObject*>(it);
            last = it;
        }
        if (it->BelowHitYTest(mouse)) return dynamic_cast<PortCMObject*>(last);
    }
    return nullptr;
}

void ControllerModelDialog::OnCheckBox_HideOtherControllerModelsClick(wxCommandEvent& event)
{
    ReloadModels();
}

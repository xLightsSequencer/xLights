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

#include "ControllerModelDialog.h"
#include "xLightsMain.h"
#include "UtilFunctions.h"
#include "outputs/Output.h"
#include "controllers/ControllerUploadData.h"
#include "controllers/ControllerCaps.h"
#include "models/ModelManager.h"
#include "models/Model.h"
#include "outputs/Controller.h"

#include <log4cpp/Category.hh>

//(*IdInit(ControllerModelDialog)
const long ControllerModelDialog::ID_PANEL1 = wxNewId();
const long ControllerModelDialog::ID_SCROLLBAR1 = wxNewId();
const long ControllerModelDialog::ID_SCROLLBAR2 = wxNewId();
const long ControllerModelDialog::ID_PANEL3 = wxNewId();
const long ControllerModelDialog::ID_PANEL2 = wxNewId();
const long ControllerModelDialog::ID_SCROLLBAR3 = wxNewId();
const long ControllerModelDialog::ID_PANEL4 = wxNewId();
//*)

const long ControllerModelDialog::CONTROLLERModel_PRINT = wxNewId();
const long ControllerModelDialog::CONTROLLERModel_SAVE_CSV = wxNewId();

BEGIN_EVENT_TABLE(ControllerModelDialog,wxDialog)
	//(*EventTable(ControllerModelDialog)
	//*)
END_EVENT_TABLE()

#define TOP_BOTTOM_MARGIN 10
#define VERTICAL_GAP 5
#define VERTICAL_SIZE 40
#define LEFT_RIGHT_MARGIN TOP_BOTTOM_MARGIN
#define HORIZONTAL_GAP VERTICAL_GAP
#define HORIZONTAL_SIZE 120
#define CORNER_ROUNDING 5
#define PRINTSCALE 8.0

#pragma region Object Classes

wxColour __lightBlue(185, 246, 250, wxALPHA_OPAQUE);
wxBrush __dropTargetBrush(__lightBlue);
wxPen __dropTargetPen(__lightBlue);
wxColour __lightRed(255, 133, 133, wxALPHA_OPAQUE);
wxBrush __invalidBrush(__lightRed);

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
    wxSize _size = wxSize(100,40);
    UDController* _cud = nullptr;
    ControllerCaps* _caps = nullptr;
    HITLOCATION _over = HITLOCATION::NONE;
    int _style;

public:

    BaseCMObject(UDController* cud, ControllerCaps* caps, wxPoint location, wxSize size, int style) {
        _style = style;
        _cud = cud;
        _caps = caps;
        _location = location;
        _size = size;
    }
    virtual ~BaseCMObject() {}
    HITLOCATION GetOver() const { return _over; }
    void SetOver(HITLOCATION hit) { _over = hit; }
    HITLOCATION HitTest(wxPoint mouse)
    {
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
    bool HitYTest(wxPoint mouse)
    {
        return (mouse.y >= _location.y &&
            mouse.y <= _location.y + _size.y);
    }
    bool BelowHitYTest(wxPoint mouse)
    {
        return (mouse.y < _location.y);
    }
    virtual void Draw(wxDC& dc, wxPoint mouse, wxSize offset, int scale, bool printing = false, bool border = true) = 0;
    void UpdateCUD(UDController* cud) { _cud = cud; }
    virtual void AddRightClickMenu(wxMenu& mnu) {}
    virtual void HandlePopup(int id) {}
    virtual std::string GetType() const = 0;
    wxRect GetRect() const { return wxRect(_location, _size); }
    void DrawTextLimited(wxDC& dc, const std::string& text, wxPoint& pt, wxSize& size)
    {
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
    bool _invalid;
public:

    PortCMObject(PORTTYPE type, int port, UDController* cud, ControllerCaps* caps, wxPoint location, wxSize size, int style, bool invalid) : BaseCMObject(cud, caps, location, size, style)
    {
        _invalid = invalid;
        _port = port;
        _type = type;
    }
    PORTTYPE GetPortType() const { return _type; }
    int GetPort() const { return _port; }
    virtual std::string GetType() const override { return "PORT"; }
    virtual void Draw(wxDC& dc, wxPoint mouse, wxSize offset, int scale, bool printing = false, bool border = true) override
    {
        auto origBrush = dc.GetBrush();
        auto origPen = dc.GetPen();
        auto origText = dc.GetTextForeground();

        wxSize sz = _size.Scale(scale, scale);

        UDControllerPort* p = nullptr;
        if (!border)
        {
            dc.SetPen(*wxTRANSPARENT_PEN);
        }
        else if (_type == PORTTYPE::PIXEL) {
            dc.SetPen(*wxRED_PEN);
            p = _cud->GetControllerPixelPort(_port);
        }
        else {
            dc.SetPen(*wxGREEN_PEN);
            p = _cud->GetControllerSerialPort(_port);
        }

        if (_over != HITLOCATION::NONE && !printing)
        {
            dc.SetBrush(__dropTargetBrush);
        }
        else if (_invalid)
        {
            dc.SetBrush(__invalidBrush);
        }
        dc.DrawRoundedRectangle(_location + offset, sz, CORNER_ROUNDING * scale);

        wxPoint pt = _location + wxSize(2, 2) + offset;
        if (_type == PORTTYPE::PIXEL) {
            DrawTextLimited(dc, wxString::Format("Pixel Port %d", _port), pt, _size - wxSize(4,4));
        }
        else {
            DrawTextLimited(dc, wxString::Format("Serial Port %d", _port), pt, _size - wxSize(4, 4));
        }
        pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);

        if (_style & STYLE_PIXELS) {
            std::string label = "Pixels: ";
            wxSize szp = dc.GetTextExtent(label);
            DrawTextLimited(dc, label, pt, _size - wxSize(4, 4));
            pt += wxSize(szp.GetWidth(), 0);
            if (p->Channels() > _caps->GetMaxPixelPortChannels())
            {
                dc.SetTextForeground(*wxRED);
            }
            DrawTextLimited(dc, wxString::Format("%d", p->Channels() / 3), pt, _size - wxSize(pt.x + 2, 4));
            dc.SetTextForeground(*wxBLACK);
            pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
        }
        if (_style & STYLE_CHANNELS) {
            std::string label = "Channels: ";
            wxSize szp = dc.GetTextExtent(label);
            DrawTextLimited(dc, label, pt, _size - wxSize(4, 4));
            pt += wxSize(szp.GetWidth(), 0);
            if (p->Channels() > _caps->GetMaxPixelPortChannels())
            {
                dc.SetTextForeground(*wxRED);
            }
            DrawTextLimited(dc, wxString::Format("%d", p->Channels()), pt, _size - wxSize(pt.x + 2, 4));
            dc.SetTextForeground(*wxBLACK);
            pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
        }

        dc.SetBrush(origBrush);
        dc.SetPen(origPen);
        dc.SetTextForeground(origText);
    }
};

class ModelCMObject : public BaseCMObject {
protected:
    ModelManager* _mm = nullptr;
    std::string _name;
    bool _outline = false;
    bool _main = false;
    std::string _displayName;
public:
    ModelCMObject(const std::string& name, const std::string displayName, ModelManager* mm, UDController* cud, ControllerCaps* caps, wxPoint location, wxSize size, int style) :
        BaseCMObject(cud, caps, location, size, style), _mm(mm) {
        _name = name;
        _main = name == displayName;
        _displayName = displayName;
        if (name != displayName && name + "-str-1" == displayName) {
            _main = true;
        }
    }

    std::string GetName() const { return _name; }
    std::string GetDisplayName() const { return _displayName; }

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
    virtual void Draw(wxDC& dc, wxPoint mouse, wxSize offset, int scale, bool printing = false, bool border = true) override {
        auto origBrush = dc.GetBrush();
        auto origPen = dc.GetPen();
        auto origText = dc.GetTextForeground();

        Model* m = _mm->GetModel(_name);

        UDControllerPortModel* udcpm = _cud->GetControllerPortModel(_name);

        if (!border) {
            dc.SetPen(*wxTRANSPARENT_PEN);
        }
        else {
            dc.SetPen(*wxGREY_PEN);
        }

        if (udcpm != nullptr) {
            switch (udcpm->GetSmartRemote())
            {
            case 0:
                dc.SetBrush(*wxWHITE_BRUSH);
                break;
            case 1:
                dc.SetBrush(*wxGREEN_BRUSH);
                break;
            case 2:
                dc.SetBrush(*wxCYAN_BRUSH);
                break;
            case 3:
                dc.SetBrush(*wxLIGHT_GREY_BRUSH);
                break;
            }
        }

        _outline = false;
        if (_dragging && !printing)
        {
            dc.SetBrush(wxColour(255, 255, 128));
        }
        else if (HitTest(mouse) != HITLOCATION::NONE && !printing)
        {
            _outline = true;
            dc.SetPen(wxPen(dc.GetPen().GetColour(), 3));
        }

        wxSize sz = _size.Scale(scale, scale);
        dc.DrawRectangle(_location + offset, sz);
        if (_over == HITLOCATION::LEFT)
        {
            dc.SetPen(__dropTargetPen);
            dc.SetBrush(__dropTargetBrush);
            wxSize ssz = wxSize((sz.x-2) / 2, sz.y-2);
            dc.DrawRectangle(_location + offset + wxSize(1,1), ssz);
        }
        else if (_over == HITLOCATION::RIGHT)
        {
            dc.SetPen(__dropTargetPen);
            dc.SetBrush(__dropTargetBrush);
            wxSize ssz = wxSize((sz.x - 2) / 2, sz.y - 2);
            dc.DrawRectangle(_location + offset + wxSize((sz.x / 2), 1), ssz);
        }

        wxPoint pt = _location + wxSize(2, 2) + offset;
        DrawTextLimited(dc, _displayName, pt, _size - wxSize(4,4));
        pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
        if (m != nullptr) {
            if (_style & STYLE_PIXELS)
            {
                DrawTextLimited(dc, wxString::Format("Pixels: %d", m->GetChanCount() / 3), pt, _size - wxSize(4, 4));
                pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
            }
            if (_style & STYLE_STRINGS)
            {
                DrawTextLimited(dc, wxString::Format("Strings: %d", m->GetNumPhysicalStrings()), pt, _size - wxSize(4, 4));
                pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
            }
            if (_style & STYLE_CHANNELS)
            {
                DrawTextLimited(dc, wxString::Format("Strings: %d", m->GetChanCount()), pt, _size - wxSize(4, 4));
                pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
            }
        }

        dc.SetBrush(origBrush);
        dc.SetPen(origPen);
        dc.SetTextForeground(origText);
    }
    virtual void AddRightClickMenu(wxMenu& mnu) override
    {
        //wxMenuItem* mi = mnu.Append(CONTROLLERModel_SAVE_CSV, "Save As CSV...");
        //mi->SetRefData((wxObjectRefData*)this); // this a bit dodgy
    }
    virtual void HandlePopup(int id) override
    {
        //if (id == xxx)
        //{

        //}
    }
};

#pragma endregion

class CMDTextDropTarget : public wxTextDropTarget
{
public:
    CMDTextDropTarget(std::list<BaseCMObject*>* objects, ControllerModelDialog* owner, wxPanel* target, bool anywhere) { _owner = owner; _objects = objects; _target = target; _anywhere = anywhere; };

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override
    {
        if (data == "") return false;

        wxPoint mouse = wxPoint(x, y);
        mouse += _owner->GetScrollPosition(_target);

        if (data.StartsWith("Model:"))
        {
            _owner->DropFromModels(mouse, data.AfterFirst(':'), _target);
            return true;
        }
        else if (data.StartsWith("Controller:"))
        {
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
        if (y > _target->GetSize().GetHeight() - SCROLLMARGIN)
        {
            if (_owner->Scroll(_target, 0, SCROLLBY)) return wxDragNone;
        }
        else if (y < SCROLLMARGIN)
        {
            if (_owner->Scroll(_target, 0, -SCROLLBY)) return wxDragNone;
        }
        else if (x < SCROLLMARGIN)
        {
            if (_owner->Scroll(_target, -SCROLLBY, 0)) return wxDragNone;
        }
        else if (x > _target->GetSize().GetWidth() - SCROLLMARGIN)
        {
            if (_owner->Scroll(_target, SCROLLBY, 0)) return wxDragNone;
        }

        if (_anywhere) return wxDragMove;

        wxDragResult res = wxDragNone;
        BaseCMObject* port = nullptr;

        wxPoint mouse(x, y);
        mouse +=_owner->GetScrollPosition(_target);

        for (const auto& it : *_objects)
        {
            auto m = dynamic_cast<ModelCMObject*>(it);
            if (it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE && !_owner->IsDragging(m))
            {
                it->SetOver(it->HitTest(mouse));
                _owner->Draw(_target, it, mouse);
                res  = wxDragMove;
            }
            else if (it->GetType() == "PORT" && it->HitYTest(mouse))
            {
                port = it;
            }
            else if (it->GetOver() != BaseCMObject::HITLOCATION::NONE)
            {
                it->SetOver(BaseCMObject::HITLOCATION::NONE);
                _owner->Draw(_target, it, wxPoint(LEFT_RIGHT_MARGIN + 5, y));
            }
        }

        if (res == wxDragNone && port != nullptr)
        {
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

ControllerModelPrintout::ControllerModelPrintout(ControllerModelDialog* controllerDialog)
{
    _controllerDialog = controllerDialog;
}

bool ControllerModelPrintout::OnPrintPage(int pageNum)
{
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
    _cud(cud), _mm(mm), _controller(controller), _xLights((xLightsFrame*)parent)
{
	//(*Initialize(ControllerModelDialog)
	wxFlexGridSizer* FlexGridSizer1;
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
	Panel3->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(Panel3);
	FlexGridSizer5->SetSizeHints(Panel3);
	FlexGridSizer1->Add(Panel3, 1, wxALL|wxEXPAND, 5);
	Panel4 = new wxPanel(this, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(0);
	PanelModels = new wxPanel(Panel4, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER, _T("ID_PANEL2"));
	FlexGridSizer3->Add(PanelModels, 1, wxALL|wxEXPAND, 0);
	ScrollBar_Models = new wxScrollBar(Panel4, ID_SCROLLBAR3, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL, wxDefaultValidator, _T("ID_SCROLLBAR3"));
	ScrollBar_Models->SetScrollbar(0, 1, 10, 1);
	FlexGridSizer3->Add(ScrollBar_Models, 1, wxALL|wxEXPAND, 0);
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

    wxSize panel4Size = wxSize(LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + LEFT_RIGHT_MARGIN + ScrollBar_Models->GetSize().GetWidth() + 10, -1);
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

    if (_autoLayout)
    {
        // If you are doing auto layout then all models must have controller name set ... this may much up model chaining but it has to be done
        // or things will get funky
        if (_cud->SetAllModelsToControllerName(_controller->GetName()))
        {
            wxMessageBox("At least one model had to have its controller name set because you are using Auto Layout. This may have mucked up the order of model chaining on some ports and you will need to fix that up here.");

            // Now need to let all the recalculations work
            while (!_xLights->DoAllWork());
        }
    }

    ReloadModels();
    Layout();
}

ControllerModelDialog::~ControllerModelDialog()
{
    while (_models.size() > 0)
    {
        delete _models.front();
        _models.pop_front();
    }

    while (_controllers.size() > 0)
    {
        delete _controllers.front();
        _controllers.pop_front();
    }

    //(*Destroy(ControllerModelDialog)
	//*)
}

void ControllerModelDialog::ReloadModels()
{
    _cud->Rescan();

    while (_models.size() > 0)
    {
        delete _models.front();
        _models.pop_front();
    }

    while (_controllers.size() > 0)
    {
        delete _controllers.front();
        _controllers.pop_front();
    }

    int y = TOP_BOTTOM_MARGIN;
    for (const auto& it : *_mm)
    {
        if (it.second->GetDisplayAs() != "ModelGroup") {
            if (_cud->GetControllerPortModel(it.second->GetName()) == nullptr && (_autoLayout || _controller->ContainsChannels(it.second->GetFirstChannel(), it.second->GetLastChannel()))) {
                _models.push_back(new ModelCMObject(it.second->GetName(), it.second->GetName(), _mm, _cud, _caps, wxPoint(5, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_STRINGS));
                y += VERTICAL_GAP + VERTICAL_SIZE;
            }
        }
    }

    ScrollBar_Models->SetRange(y);
    ScrollBar_Models->SetPageSize(y / 10);

    y = VERTICAL_GAP;

    int maxx = 0;
    for (int i = 0; i < std::max(_caps->GetMaxPixelPort(), _cud->GetMaxPixelPort()); i++)
    {
        auto cmp = new PortCMObject(PortCMObject::PORTTYPE::PIXEL, i + 1, _cud, _caps, wxPoint(LEFT_RIGHT_MARGIN, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_PIXELS, i+1 > _caps->GetMaxPixelPort());
        _controllers.push_back(cmp);

        auto pp = _cud->GetControllerPixelPort(i + 1);
        if (pp != nullptr)
        {
            if (_caps->SupportsVirtualStrings())
            {
                pp->CreateVirtualStrings(_caps->MergeConsecutiveVirtualStrings());
                if (pp->GetVirtualStringCount() == 0)
                {
                    y += VERTICAL_GAP + VERTICAL_SIZE;
                }
                else
                {
                    for (const auto& it : pp->GetVirtualStrings())
                    {
                        int x = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + HORIZONTAL_GAP;
                        for (const auto& it2 : it->_models)
                        {
                            auto cmm = new ModelCMObject(it2->GetModel()->GetName(), it2->GetName(), _mm, _cud, _caps, wxPoint(x, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_PIXELS);
                            _controllers.push_back(cmm);
                            x += HORIZONTAL_SIZE + HORIZONTAL_GAP;
                        }
                        if (x > maxx) maxx = x;
                        y += VERTICAL_GAP + VERTICAL_SIZE;
                    }
                }
            }
            else
            {
                int x = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + HORIZONTAL_GAP;
                for (const auto& it : pp->GetModels())
                {
                    auto cmm = new ModelCMObject(it->GetModel()->GetName(), it->GetName(), _mm, _cud, _caps, wxPoint(x, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_PIXELS);
                    _controllers.push_back(cmm);
                    x += HORIZONTAL_SIZE + HORIZONTAL_GAP;
                }
                if (x > maxx) maxx = x;
                y += VERTICAL_GAP + VERTICAL_SIZE;
            }
        }
    }

    for (int i = 0; i < std::max(_caps->GetMaxSerialPort(), _cud->GetMaxSerialPort()); i++)
    {
        _controllers.push_back(new PortCMObject(PortCMObject::PORTTYPE::SERIAL, i + 1, _cud, _caps, wxPoint(LEFT_RIGHT_MARGIN, y), wxSize(HORIZONTAL_SIZE, VERTICAL_SIZE), BaseCMObject::STYLE_CHANNELS, i + 1 > _caps->GetMaxSerialPort()));
        auto sp = _cud->GetControllerSerialPort(i + 1);
        if (sp != nullptr)
        {
            int x = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + HORIZONTAL_GAP;
            for (const auto& it : sp->GetModels())
            {
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

    ScrollBar_Controller_V->SetRange(y);
    ScrollBar_Controller_V->SetPageSize(y / 10);
    ScrollBar_Controller_H->SetRange(maxx);
    ScrollBar_Controller_H->SetPageSize(maxx / 10);

    PanelController->Refresh();
    PanelModels->Refresh();
}

void ControllerModelDialog::OnPopupCommand(wxCommandEvent &event)
{
    int id = event.GetId();
    if (id == CONTROLLERModel_PRINT)
    {
        static wxPrintDialogData printDialogData;
        wxPrinter printer(&printDialogData);

        ControllerModelPrintout printout(this);

        if (!printer.Print(this, &printout, true))
        {
            if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
            {
                DisplayError(wxString::Format("Problem printing wiring. %d", wxPrinter::GetLastError()).ToStdString(), this);
            }
        }
        else
        {
            printDialogData = printer.GetPrintDialogData();
        }
    }
    else if (id == CONTROLLERModel_SAVE_CSV)
    {
        SaveCSV();
    }
    else
    {
        BaseCMObject* cm = (BaseCMObject *)event.GetEventObject()->GetRefData();
        cm->HandlePopup(id);
    }
}

void ControllerModelDialog::RenderPicture(wxBitmap& bitmap, bool printer)
{
    wxMemoryDC dc;
    dc.SelectObject(bitmap);

    dc.SetTextForeground(*wxWHITE);

    dc.SetPen(*wxWHITE_PEN);
    dc.SetBrush(*wxWHITE_BRUSH);

    dc.DrawRectangle(wxPoint(0, 0), bitmap.GetScaledSize());

    dc.SetDeviceOrigin(0, 0);

    int fontSize = 10 * PRINTSCALE;
    wxFont font = wxFont(fontSize, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("Arial"), wxFONTENCODING_DEFAULT);
    dc.SetFont(font);

    int rowPos = TOP_BOTTOM_MARGIN * PRINTSCALE;
    dc.DrawText(_title, LEFT_RIGHT_MARGIN * PRINTSCALE, rowPos);
    rowPos += ((VERTICAL_SIZE / 2) * PRINTSCALE) + (VERTICAL_GAP * PRINTSCALE);

    for (const auto& it : _controllers)
    {
        it->Draw(dc, wxPoint(0, 0), wxSize(0, rowPos), PRINTSCALE, true);
    }
}

void ControllerModelDialog::SaveCSV()
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, _title, wxEmptyString, "Export files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty()) return;

    std::vector<wxString> lines;
    int columnSize = 0;

    for (int i = 1; i <= _cud->GetMaxPixelPort(); i++)
    {
        wxString line = wxString::Format("Pixel Port %d,", i);

        if (columnSize < _cud->GetControllerPixelPort(i)->GetModels().size())
            columnSize = _cud->GetControllerPixelPort(i)->GetModels().size();

        for (const auto& it : _cud->GetControllerPixelPort(i)->GetModels())
        {
            if (it->GetSmartRemote() > 0)
            {
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
    for (int i = 1; i <= _cud->GetMaxSerialPort(); i++)
    {
        if (columnSize < _cud->GetControllerSerialPort(i)->GetModels().size())
            columnSize = _cud->GetControllerSerialPort(i)->GetModels().size();

        wxString line = wxString::Format("Serial Port %d,", i);
        for (const auto& it : _cud->GetControllerSerialPort(i)->GetModels())
        {
            line += it->GetName();
            line += ",";
        }
        line += "\n";
        lines.push_back(line);
    }

    wxFile f(filename);

    if (!f.Create(filename, true) || !f.IsOpened())
    {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
        return;
    }

    wxString header = _title + "\nOutput,";
    for (int i = 1; i <= columnSize; i++)
    {
        header += wxString::Format("Model %d,", i);
    }
    header += "\n";
    f.Write(header);
    for (const auto& line : lines)
    {
        f.Write(line);
    }
    f.Close();
}

void ControllerModelDialog::DropFromModels(const wxPoint& location, const std::string& name, wxPanel* target)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    auto m = _mm->GetModel(name);
    wxASSERT(m != nullptr);

    logger_base.debug("Model %s dropped from models pane.", (const char*)name.c_str());

    // model dragged from models
    if (target == PanelModels)
    {
        // do nothing
        logger_base.debug("   onto the models pane ... nothing to do.");
    }
    else
    {
        logger_base.debug("    onto the controller pane.");

        auto port = GetControllerPortAtLocation(location);
        if (port != nullptr)
        {
            logger_base.debug("    onto port %d.", port->GetPort());
            m->SetControllerPort(port->GetPort());
            if (port->GetPortType() == PortCMObject::PORTTYPE::PIXEL)
            {
                if (!m->IsPixelProtocol()) m->SetControllerProtocol("WS2811");
            }
            else
            {
                if (m->IsPixelProtocol() || m->GetControllerProtocol() == "") m->SetControllerProtocol("DMX");
            }

            if (_autoLayout)
            {
                m->SetControllerName(_controller->GetName());
            }

            auto ob = GetControllerCMObjectAt(location);
            ModelCMObject* mob = dynamic_cast<ModelCMObject*>(ob);
            BaseCMObject::HITLOCATION hit = BaseCMObject::HITLOCATION::NONE;
            if (ob != nullptr) hit = ob->HitTest(location);

            if (mob == nullptr || !mob->IsMain())
            {
                logger_base.debug("    Processing it as a drop onto the port ... so setting it to beginning.");

                // dropped on a port .. or not on the first string of a model
                // If no model already there put it at the beginning ... else chain it
                if (_autoLayout)
                {
                    auto fmud = _cud->GetControllerPixelPort(port->GetPort())->GetFirstModel();
                    Model* fm = nullptr;
                    if (fmud != nullptr && fmud->IsFirstModelString())
                    {
                        fm = fmud->GetModel();
                    }
                    m->SetModelChain("Beginning");
                    if (fm != nullptr)
                    {
                        fm->SetModelChain(">" + m->GetName());
                        logger_base.debug("    Old beginning Model %s now chains to our dropped model.", (const char*)fm->GetName().c_str());
                    }
                }
            }
            else
            {
                Model* droppedOn = _mm->GetModel(mob->GetName());
                logger_base.debug("    Dropped onto model %s.", (const char*)droppedOn->GetName().c_str());

                // dropped on a model
                if (_autoLayout)
                {
                    if (hit == BaseCMObject::HITLOCATION::LEFT)
                    {
                        logger_base.debug("    On the left hand side.");
                        m->SetModelChain(droppedOn->GetModelChain());
                        droppedOn->SetModelChain(">" + m->GetName());
                    }
                    else
                    {
                        logger_base.debug("    On the right hand side.");
                        Model* next = nullptr;
                        if (port->GetPortType() == PortCMObject::PORTTYPE::PIXEL)
                        {
                            next = _cud->GetControllerPixelPort(port->GetPort())->GetModelAfter(droppedOn);
                        }
                        else
                        {
                            next = _cud->GetControllerSerialPort(port->GetPort())->GetModelAfter(droppedOn);
                        }
                        if (next != nullptr)
                        {
                            next->SetModelChain(">" + m->GetName());
                        }
                        m->SetModelChain(">" + droppedOn->GetName());
                    }
                }
            }
        }
        else
        {
            logger_base.debug("    but not onto a port ... so nothing to do.");
        }
        while (!_xLights->DoAllWork());
        ReloadModels();
    }
}

void ControllerModelDialog::DropFromController(const wxPoint& location, const std::string& name, wxPanel* target)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Model %s dropped from controllers pane.", (const char*)name.c_str());

    // model dragged from controllers
    auto m = _mm->GetModel(name);
    wxASSERT(m != nullptr);

    if (target == PanelModels)
    {
        logger_base.debug("   onto the models pane ... so remove the model from the controller.");
        // Removing a model from the controller
        if (_autoLayout)
        {
            m->SetControllerName("");
        }
        m->SetControllerPort(0);
    }
    else
    {
        logger_base.debug("    onto the controller pane.");

        auto port = GetControllerPortAtLocation(location);
        if (port != nullptr)
        {
            logger_base.debug("    onto port %d.", port->GetPort());
            m->SetControllerPort(port->GetPort());
            if (port->GetPortType() == PortCMObject::PORTTYPE::PIXEL)
            {
                if (!m->IsPixelProtocol()) m->SetControllerProtocol("WS2811");
            }
            else
            {
                if (m->IsPixelProtocol() || m->GetControllerProtocol() == "") m->SetControllerProtocol("DMX");
            }

            if (_autoLayout)
            {
                m->SetControllerName(_controller->GetName());
            }

            auto ob = GetControllerCMObjectAt(location);
            ModelCMObject* mob = dynamic_cast<ModelCMObject*>(ob);
            BaseCMObject::HITLOCATION hit = BaseCMObject::HITLOCATION::NONE;
            if (ob != nullptr) hit = ob->HitTest(location);

            if (mob == nullptr || !mob->IsMain())
            {
                logger_base.debug("    Processing it as a drop onto the port ... so setting it to beginning.");

                // dropped on a port .. or not on the first string of a model
                // If no model already there put it at the beginning ... else chain it
                if (_autoLayout)
                {
                    auto fmud = _cud->GetControllerPixelPort(port->GetPort())->GetFirstModel();
                    Model* fm = nullptr;
                    if (fmud != nullptr && fmud->IsFirstModelString())
                    {
                        fm = fmud->GetModel();
                    }
                    m->SetModelChain("Beginning");
                    if (fm != nullptr)
                    {
                        fm->SetModelChain(">" + m->GetName());
                        logger_base.debug("    Old beginning Model %s now chains to our dropped model.", (const char*)fm->GetName().c_str());
                    }
                }
            }
            else
            {
                Model* droppedOn = _mm->GetModel(mob->GetName());
                logger_base.debug("    Dropped onto model %s.", (const char*)droppedOn->GetName().c_str());

                // dropped on a model
                if (_autoLayout)
                {
                    if (hit == BaseCMObject::HITLOCATION::LEFT)
                    {
                        logger_base.debug("    On the left hand side.");
                        if (droppedOn->GetModelChain() != ">" + m->GetName())
                        {
                            m->SetModelChain(droppedOn->GetModelChain());
                            droppedOn->SetModelChain(">" + m->GetName());
                        }
                    }
                    else
                    {
                        logger_base.debug("    On the right hand side.");
                        if (m->GetModelChain() != ">" + droppedOn->GetName())
                        {
                            Model* next = nullptr;
                            if (port->GetPortType() == PortCMObject::PORTTYPE::PIXEL)
                            {
                                next = _cud->GetControllerPixelPort(port->GetPort())->GetModelAfter(droppedOn);
                            }
                            else
                            {
                                next = _cud->GetControllerSerialPort(port->GetPort())->GetModelAfter(droppedOn);
                            }
                            if (next != nullptr)
                            {
                                next->SetModelChain(">" + m->GetName());
                            }
                            m->SetModelChain(">" + droppedOn->GetName());
                        }
                    }
                }
            }
        }
        else
        {
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

    for (const auto& it : _controllers)
    {
        if (it->GetType() == "MODEL" && it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE)
        {
            auto m = dynamic_cast<ModelCMObject*>(it);
            if (m->IsMain())
            {
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
    if (_dragging != nullptr)
    {
        for (const auto& it : _controllers)
        {
            auto hit = it->HitTest(event.GetPosition());
            if (hit != BaseCMObject::HITLOCATION::NONE)
            {
                if (it->GetType() == "PORT")
                {
                    it->SetOver(BaseCMObject::HITLOCATION::RIGHT);
                    PanelController->RefreshRect(it->GetRect());
                }
                else
                {
                    if (it->GetOver() != hit)
                    {
                        it->SetOver(hit);
                        PanelController->RefreshRect(it->GetRect());
                    }
                }
            }
            else
            {
                ClearOver(PanelController, _controllers);
            }
        }
    }
    else
    {
        std::string tt = "";
        for (const auto& it : _controllers)
        {
            bool ishit = it->HitTest(event.GetPosition()) != BaseCMObject::HITLOCATION::NONE;
            auto m = dynamic_cast<ModelCMObject*>(it);
            if (ishit || (m != nullptr && m->IsOutline()))
            {
                PanelController->RefreshRect(it->GetRect());
                if (ishit)
                {
                    if (m != nullptr)
                    {
                        tt = GetModelTooltip(m);
                    }
                }
            }
        }
        if (PanelController->GetToolTipText() != tt)
        {
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
    for (const auto& it : list)
    {
        if (it->GetOver() != BaseCMObject::HITLOCATION::NONE)
        {
            it->SetOver(BaseCMObject::HITLOCATION::NONE);
            panel->RefreshRect(it->GetRect());
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

    _xLights->GetControllerDetailsForChannel(m->GetFirstChannel(), controllerName, type, protocol, description,
                                             channelOffset, ip, universe, inactive, baud, startUniverse, endUniverse);
    auto om = _xLights->GetOutputManager();
    if (_autoLayout)
    {
        return wxString::Format("Name: %s\nController Name: %s\nModel Chain: %s\nStart Channel: %s\nEnd Channel %s\nStrings %d",
            mob->GetDisplayName(), controllerName, m->GetModelChain() == "" ? "Beginning" : m->GetModelChain(), m->GetStartChannelInDisplayFormat(om), m->GetLastChannelInStartChannelFormat(om),
            m->GetNumPhysicalStrings()).ToStdString();
    }
    else
    {
        return wxString::Format("name: %s\nController Name: %s\nIP/Serial: %s\nStart Channel: %s\nEnd Channel %s\nStrings %d",
            mob->GetDisplayName(), controllerName, universe, m->GetStartChannelInDisplayFormat(om), m->GetLastChannelInStartChannelFormat(om),
            m->GetNumPhysicalStrings()).ToStdString();
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

    if (scrollByX > 0 && ScrollBar_Controller_H->GetThumbPosition() < maxx - SCROLLBY)
    {
        scrolled = true;
        ScrollBar_Controller_H->SetThumbPosition(ScrollBar_Controller_H->GetThumbPosition() + scrollByX);
        if (ScrollBar_Controller_H->GetThumbPosition() > maxx) {
            ScrollBar_Controller_H->SetThumbPosition(maxx);
        }
    }
    else if (scrollByX < 0 && ScrollBar_Controller_H->GetThumbPosition() > 0)
    {
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
    if (panel == PanelModels)
    {
        return wxPoint(0, ScrollBar_Models->GetThumbPosition());
    }
    return wxPoint(ScrollBar_Controller_H->GetThumbPosition(), ScrollBar_Controller_V->GetThumbPosition());
}

void ControllerModelDialog::OnPanelControllerRightDown(wxMouseEvent& event)
{
    wxMenu mnu;
    mnu.Append(CONTROLLERModel_PRINT, "Print");
    mnu.Append(CONTROLLERModel_SAVE_CSV, "Save As CSV...");

    BaseCMObject* cm = GetControllerCMObjectAt(event.GetPosition());
    if (cm != nullptr)
    {
        cm->AddRightClickMenu(mnu);
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&ControllerModelDialog::OnPopupCommand, nullptr, this);
    PopupMenu(&mnu);
}

void ControllerModelDialog::OnPanelControllerPaint(wxPaintEvent& event)
{
    wxPaintDC dc(PanelController);

    int xOffset = ScrollBar_Controller_H->GetThumbPosition();
    int yOffset = ScrollBar_Controller_V->GetThumbPosition();

    dc.SetDeviceOrigin(-xOffset, -yOffset);

    for (const auto& it : _controllers)
    {
        it->Draw(dc, PanelController->ScreenToClient(wxGetMousePosition()), wxSize(0,0), 1, false);
    }
}

void ControllerModelDialog::Draw(wxPanel* panel, BaseCMObject* object, wxPoint mouse)
{
    wxClientDC dc(panel);

    int xOffset = ScrollBar_Controller_H->GetThumbPosition();
    int yOffset = ScrollBar_Controller_V->GetThumbPosition();

    if (panel == PanelModels)
    {
        xOffset = 0;
        yOffset = ScrollBar_Models->GetThumbPosition();
    }
    dc.SetDeviceOrigin(-xOffset, -yOffset);
    object->Draw(dc, mouse, wxSize(0, 0), 1, false);
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
    PanelModels->Refresh();
}

void ControllerModelDialog::OnPanelModelsPaint(wxPaintEvent& event)
{
    wxPaintDC dc(PanelModels);

    int yOffset = ScrollBar_Models->GetThumbPosition();

    dc.SetDeviceOrigin(0, -yOffset);

    for (const auto& it : _models)
    {
        it->Draw(dc, PanelModels->ScreenToClient(wxGetMousePosition()), wxSize(0, 0), 1, false);
    }
}

void ControllerModelDialog::OnPanelModelsKeyDown(wxKeyEvent& event)
{
}

void ControllerModelDialog::OnPanelModelsLeftDown(wxMouseEvent& event)
{
    wxPoint mouse = event.GetPosition();
    mouse += GetScrollPosition(PanelModels);

    for (const auto& it : _models)
    {
        if (it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE)
        {
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

void ControllerModelDialog::OnPanelModelsLeftUp(wxMouseEvent& event)
{
    _dragging = nullptr;
}

void ControllerModelDialog::OnPanelModelsLeftDClick(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelModelsRightDown(wxMouseEvent& event)
{
    wxMenu mnu;

    BaseCMObject* cm = GetModelsCMObjectAt(event.GetPosition());
    if (cm != nullptr)
    {
        cm->AddRightClickMenu(mnu);
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&ControllerModelDialog::OnPopupCommand, nullptr, this);
    PopupMenu(&mnu);
}

void ControllerModelDialog::OnPanelModelsMouseMove(wxMouseEvent& event)
{
    std::string tt = "";
    if (_dragging == nullptr)
    {
        for (const auto& it : _models)
        {
            bool ishit = it->HitTest(event.GetPosition()) != BaseCMObject::HITLOCATION::NONE;

            if (ishit || ((ModelCMObject*)it)->IsOutline())
            {
                PanelModels->RefreshRect(it->GetRect());
                if (ishit)
                {
                    auto m = dynamic_cast<ModelCMObject*>(it);
                    if (m != nullptr)
                    {
                        tt = GetModelTooltip(m);
                    }
                }
            }
        }
    }
    if (PanelModels->GetToolTipText() != tt)
    {
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
    int pw;
    int ph;
    PanelModels->GetSize(&pw, &ph);
    int h = (float)ph / ScrollBar_Controller_V->GetRange() * (ph - 80);
    int w = (float)pw / ScrollBar_Controller_H->GetRange() * (pw - 80);
    ScrollBar_Controller_H->SetThumbSize(w);
    ScrollBar_Controller_V->SetThumbSize(h);
    PanelController->Refresh();
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
    if (event.ShiftDown())
    {
        int position = ScrollBar_Controller_H->GetThumbPosition();
        position -= event.GetWheelRotation();
        ScrollBar_Controller_H->SetThumbPosition(position);
    }
    else
    {
        int position = ScrollBar_Controller_V->GetThumbPosition();
        position -= event.GetWheelRotation();
        ScrollBar_Controller_V->SetThumbPosition(position);
    }

    PanelController->Refresh();
}

void ControllerModelDialog::OnPanelControllerResize(wxSizeEvent& event)
{
    int pw;
    int ph;
    PanelController->GetSize(&pw, &ph);
    int h = (float)ph / ScrollBar_Controller_V->GetRange() * (ph - 80);
    int w = (float)pw / ScrollBar_Controller_H->GetRange() * (pw - 80);
    ScrollBar_Controller_H->SetThumbSize(w);
    ScrollBar_Controller_V->SetThumbSize(h);
    PanelController->Refresh();
}
#pragma endregion

BaseCMObject* ControllerModelDialog::GetControllerCMObjectAt(wxPoint mouse)
{
    for (const auto& it : _controllers)
    {
        //auto m = dynamic_cast<ModelCMObject*>(it);

        if (it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE) return it;
            //&& (m == nullptr || m->IsMain())) return it;
    }
    return nullptr;
}

BaseCMObject* ControllerModelDialog::GetModelsCMObjectAt(wxPoint mouse)
{
    for (const auto& it : _models)
    {
        if (it->HitTest(mouse) != BaseCMObject::HITLOCATION::NONE) return it;
    }
    return nullptr;
}

PortCMObject* ControllerModelDialog::GetControllerPortAtLocation(wxPoint mouse)
{
    BaseCMObject* last = nullptr;
    for (const auto& it : _controllers)
    {
        if (it->GetType() == "PORT")
        {
            if (it->HitYTest(mouse)) return dynamic_cast<PortCMObject*>(it);
            last = it;
        }
        if (it->BelowHitYTest(mouse)) return dynamic_cast<PortCMObject*>(last);
    }
    return nullptr;
}

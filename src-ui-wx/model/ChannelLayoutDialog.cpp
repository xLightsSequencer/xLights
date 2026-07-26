/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ChannelLayoutDialog.h"

//(*InternalHeaders(ChannelLayoutDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <algorithm>

#include <wx/font.h>
#include <wx/html/htmlwin.h>
#include <wx/html/htmprint.h>

#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"

namespace {
constexpr int ZOOM_MIN = 50;
constexpr int ZOOM_MAX = 300;
constexpr int ZOOM_STEP = 10;
}

//(*IdInit(ChannelLayoutDialog)
const long ChannelLayoutDialog::ID_BUTTON1 = wxNewId();
const long ChannelLayoutDialog::ID_BUTTON_OPEN_IN_BROWSER = wxNewId();
const long ChannelLayoutDialog::ID_SLIDER_ZOOM = wxNewId();
const long ChannelLayoutDialog::ID_STATICTEXT_ZOOM = wxNewId();
const long ChannelLayoutDialog::ID_HTMLWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ChannelLayoutDialog,wxDialog)
    //(*EventTable(ChannelLayoutDialog)
    //*)
END_EVENT_TABLE()

ChannelLayoutDialog::ChannelLayoutDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    //(*Initialize(ChannelLayoutDialog)
    wxBoxSizer* BoxSizer1;
    wxButton* Button_Print;
    wxFlexGridSizer* FlexGridSizer1;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, id, _("Node Layout"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    BoxSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Print = new wxButton(this, ID_BUTTON1, _("Print"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    BoxSizer1->Add(Button_Print, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonOpenInBrower = new wxButton(this, ID_BUTTON_OPEN_IN_BROWSER, _("Open in Browser"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_OPEN_IN_BROWSER"));
    BoxSizer1->Add(ButtonOpenInBrower, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SliderZoom = new wxSlider(this, ID_SLIDER_ZOOM, 100, ZOOM_MIN, ZOOM_MAX, wxDefaultPosition, wxSize(180,-1), 0, wxDefaultValidator, _T("ID_SLIDER_ZOOM"));
    SliderZoom->SetToolTip(_("Zoom. Hold Ctrl and use the mouse wheel over the node layout."));
    BoxSizer1->Add(SliderZoom, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextZoom = new wxStaticText(this, ID_STATICTEXT_ZOOM, _("100%"), wxDefaultPosition, wxSize(50,-1), wxALIGN_RIGHT, _T("ID_STATICTEXT_ZOOM"));
    BoxSizer1->Add(StaticTextZoom, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    HtmlWindow1 = new wxHtmlWindow(this, ID_HTMLWINDOW1, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO, _T("ID_HTMLWINDOW1"));
    HtmlWindow1->SetMinSize(wxSize(500,400));
    FlexGridSizer1->Add(HtmlWindow1, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ChannelLayoutDialog::OnButton_PrintClick);
    Connect(ID_BUTTON_OPEN_IN_BROWSER,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ChannelLayoutDialog::OnButtonOpenInBrowerClick);
    Connect(ID_SLIDER_ZOOM,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ChannelLayoutDialog::OnSliderZoomCmdScroll);
    //*)
    HtmlEasyPrint=new wxHtmlEasyPrinting("xLights Printing", this);
    HtmlWindow1->Bind(wxEVT_MOUSEWHEEL, &ChannelLayoutDialog::OnHtmlMouseWheel, this);
}

ChannelLayoutDialog::~ChannelLayoutDialog()
{
    //(*Destroy(ChannelLayoutDialog)
    //*)
    delete HtmlEasyPrint;
}

void ChannelLayoutDialog::OnButton_PrintClick(wxCommandEvent& event)
{
    HtmlEasyPrint->SetStandardFonts(7, "Arial");
    HtmlEasyPrint->PageSetup();
    HtmlEasyPrint->PreviewText(HtmlSource);
}

void ChannelLayoutDialog::SetHtmlSource(wxString& html)
{
    HtmlSource=html;
    HtmlWindow1->SetPage(html);
}

void ChannelLayoutDialog::OnButtonOpenInBrowerClick(wxCommandEvent& event)
{
	ViewTempFile(HtmlSource, "ModelHTML", "html");
}

void ChannelLayoutDialog::OnSliderZoomCmdScroll(wxCommandEvent& event)
{
    ApplyZoom(SliderZoom->GetValue());
}

void ChannelLayoutDialog::OnHtmlMouseWheel(wxMouseEvent& event)
{
    if (!event.ControlDown() || event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL) {
        event.Skip();
        return;
    }

    const int wheelDelta = event.GetWheelDelta();
    if (wheelDelta <= 0) {
        return;
    }

    _wheelRotation += event.GetWheelRotation();
    const int steps = _wheelRotation / wheelDelta;
    _wheelRotation %= wheelDelta;
    if (steps != 0) {
        ApplyZoom(_zoomPercent + steps * ZOOM_STEP);
    }
}

void ChannelLayoutDialog::ApplyZoom(int zoomPercent)
{
    zoomPercent = std::clamp(zoomPercent, ZOOM_MIN, ZOOM_MAX);
    if (zoomPercent == _zoomPercent) {
        return;
    }

    int viewX = 0;
    int viewY = 0;
    HtmlWindow1->GetViewStart(&viewX, &viewY);

    const int previousZoom = _zoomPercent;
    _zoomPercent = zoomPercent;
    SliderZoom->SetValue(_zoomPercent);
    StaticTextZoom->SetLabel(wxString::Format("%d%%", _zoomPercent));

    const int defaultFontSize = std::max(10, wxNORMAL_FONT->GetPointSize());
    const int fontSize = std::max(1, (defaultFontSize * _zoomPercent + 50) / 100);
    HtmlWindow1->SetStandardFonts(fontSize);
    HtmlWindow1->Scroll(
        (viewX * _zoomPercent + previousZoom / 2) / previousZoom,
        (viewY * _zoomPercent + previousZoom / 2) / previousZoom);
}

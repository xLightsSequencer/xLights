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
#include <array>
#include <string>

#include <wx/font.h>
#include <wx/html/htmlwin.h>
#include <wx/html/htmprint.h>

#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"

namespace {
constexpr int ZOOM_MIN = 10;
constexpr int ZOOM_MAX = 300;
constexpr int ZOOM_STEP = 10;

// mirrors wx's own wxGetDefaultHTMLFontSize() so 100% renders identically to an unzoomed page
int DefaultHtmlFontSize() {
    return std::max(10, wxNORMAL_FONT->GetPointSize());
}
}

//(*IdInit(ChannelLayoutDialog)
const long ChannelLayoutDialog::ID_BUTTON1 = wxNewId();
const long ChannelLayoutDialog::ID_BUTTON_OPEN_IN_BROWSER = wxNewId();
const long ChannelLayoutDialog::ID_BUTTON_FIT = wxNewId();
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
    ButtonFit = new wxButton(this, ID_BUTTON_FIT, _("Fit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_FIT"));
    ButtonFit->SetToolTip(_("Fit the complete node layout in the window."));
    BoxSizer1->Add(ButtonFit, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    SliderZoom = new wxSlider(this, ID_SLIDER_ZOOM, 100, ZOOM_MIN, ZOOM_MAX, wxDefaultPosition, wxSize(180,-1), 0, wxDefaultValidator, _T("ID_SLIDER_ZOOM"));
    SliderZoom->SetToolTip(_("Zoom. Hold Ctrl/Cmd and use the mouse wheel over the node layout."));
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
    Connect(ID_BUTTON_FIT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ChannelLayoutDialog::OnButtonFitClick);
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
    // set the fonts while the window is still empty; SetFonts reparses the page,
    // so doing it after SetPage would parse a large layout twice
    ApplyZoom(_zoomPercent);
    HtmlWindow1->SetPage(GetDisplayHtml());
}

void ChannelLayoutDialog::OnButtonOpenInBrowerClick(wxCommandEvent& event)
{
	ViewTempFile(HtmlSource, "ModelHTML", "html");
}

void ChannelLayoutDialog::OnSliderZoomCmdScroll(wxCommandEvent& event)
{
    ApplyZoom(SliderZoom->GetValue());
}

void ChannelLayoutDialog::OnButtonFitClick(wxCommandEvent& event)
{
    FitNodeLayoutToWindow();
}

void ChannelLayoutDialog::OnHtmlMouseWheel(wxMouseEvent& event)
{
    if (!event.CmdDown() || event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL) {
        event.Skip();
        return;
    }

    const int wheelDelta = event.GetWheelDelta();
    if (wheelDelta <= 0) {
        event.Skip();
        return;
    }

    _wheelRotation += event.GetWheelRotation();
    const int steps = _wheelRotation / wheelDelta;
    _wheelRotation %= wheelDelta;
    if (steps != 0) {
        ApplyZoom(_zoomPercent + steps * ZOOM_STEP);
    }
}

int ChannelLayoutDialog::ZoomToFontSize(int zoomPercent) const
{
    return std::max(1, (DefaultHtmlFontSize() * zoomPercent + 50) / 100);
}

int ChannelLayoutDialog::FontSizeToZoom(int fontSize) const
{
    const int defaultFontSize = DefaultHtmlFontSize();
    int zoomPercent = std::clamp((fontSize * 100 + defaultFontSize / 2) / defaultFontSize, ZOOM_MIN, ZOOM_MAX);
    while (zoomPercent > ZOOM_MIN && ZoomToFontSize(zoomPercent) > fontSize) {
        --zoomPercent;
    }
    while (zoomPercent < ZOOM_MAX && ZoomToFontSize(zoomPercent) < fontSize) {
        ++zoomPercent;
    }
    return zoomPercent;
}

void ChannelLayoutDialog::ApplyZoom(int zoomPercent)
{
    _zoomPercent = std::clamp(zoomPercent, ZOOM_MIN, ZOOM_MAX);
    if (SliderZoom->GetValue() != _zoomPercent) {
        SliderZoom->SetValue(_zoomPercent);
    }
    StaticTextZoom->SetLabel(wxString::Format("%d%%", _zoomPercent));
    ApplyFontSize(ZoomToFontSize(_zoomPercent));
}

void ChannelLayoutDialog::ApplyFontSize(int fontSize)
{
    // SetFonts reparses and relays out the whole page, which is expensive on a large
    // grid, and many zoom percentages map to the same point size
    if (fontSize == _fontSize) {
        return;
    }
    _fontSize = fontSize;

    int viewX = 0;
    int viewY = 0;
    HtmlWindow1->GetViewStart(&viewX, &viewY);

    // slots 2-7 reproduce wx's own wxBuildFontSizes() so everything outside the node
    // grid keeps its normal size; only slot 1, which the grid cells use, follows the zoom
    const int defaultFontSize = DefaultHtmlFontSize();
    const std::array<int, 7> fontSizes {
        fontSize,
        std::max(1, defaultFontSize * 83 / 100),
        defaultFontSize,
        std::max(1, defaultFontSize * 120 / 100),
        std::max(1, defaultFontSize * 144 / 100),
        std::max(1, defaultFontSize * 173 / 100),
        std::max(1, defaultFontSize * 2)
    };
    HtmlWindow1->SetFonts(wxNORMAL_FONT->GetFaceName(), wxEmptyString, fontSizes.data());
    HtmlWindow1->Scroll(viewX, viewY);
}

void ChannelLayoutDialog::FitNodeLayoutToWindow()
{
    int minimum = ZoomToFontSize(ZOOM_MIN);
    int maximum = ZoomToFontSize(ZOOM_MAX);
    int bestFit = minimum;

    while (minimum <= maximum) {
        const int candidate = (minimum + maximum) / 2;
        ApplyFontSize(candidate);

        const wxSize contentSize = HtmlWindow1->GetVirtualSize();
        const wxSize windowSize = HtmlWindow1->GetClientSize();
        if (contentSize.GetWidth() <= windowSize.GetWidth() && contentSize.GetHeight() <= windowSize.GetHeight()) {
            bestFit = candidate;
            minimum = candidate + 1;
        } else {
            maximum = candidate - 1;
        }
    }

    ApplyZoom(FontSizeToZoom(bestFit));
    HtmlWindow1->Scroll(0, 0);
}

wxString ChannelLayoutDialog::GetDisplayHtml() const
{
    const std::string src = HtmlSource.utf8_string();
    const std::string tableStartTag = "<table border=1>";
    const size_t tableStart = src.find(tableStartTag);
    if (tableStart == std::string::npos) {
        return HtmlSource;
    }

    size_t tableEnd = src.find("</table>", tableStart + tableStartTag.length());
    if (tableEnd == std::string::npos) {
        tableEnd = src.length();
    }

    // built in one pass rather than inserting into the source: a big model has tens of
    // thousands of cells and each insert would shift the whole remaining document
    std::string html;
    html.reserve(src.length() * 5 / 4);
    html.append(src, 0, tableStart);

    size_t pos = tableStart;
    while (true) {
        const size_t cellStart = src.find("<td", pos);
        if (cellStart == std::string::npos || cellStart >= tableEnd) {
            break;
        }
        const size_t openingTagEnd = src.find('>', cellStart);
        if (openingTagEnd == std::string::npos) {
            break;
        }
        const size_t cellEnd = src.find("</td>", openingTagEnd);
        if (cellEnd == std::string::npos || cellEnd > tableEnd) {
            break;
        }

        html.append(src, pos, openingTagEnd + 1 - pos);
        html += "<font size=1>";
        html.append(src, openingTagEnd + 1, cellEnd - (openingTagEnd + 1));
        html += "</font>";
        pos = cellEnd;
    }
    html.append(src, pos, std::string::npos);

    return wxString::FromUTF8(html);
}

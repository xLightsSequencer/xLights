/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ColourReplaceDialog.h"

//(*InternalHeaders(ColourReplaceDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/odcombo.h>
#include <wx/clrpicker.h>
#include <wx/bmpbuttn.h>
#include <wx/pen.h>
#include <wx/brush.h>
#include <wx/dc.h>

#include "Color.h"
#include "xLightsMain.h"

#define ZERO 0

//(*IdInit(ColourReplaceDialog)
const long ColourReplaceDialog::ID_STATICTEXT1 = wxNewId();
const long ColourReplaceDialog::ID_CUSTOM1 = wxNewId();
const long ColourReplaceDialog::ID_STATICTEXT2 = wxNewId();
const long ColourReplaceDialog::ID_CHECKBOX1 = wxNewId();
const long ColourReplaceDialog::ID_BUTTON1 = wxNewId();
const long ColourReplaceDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ColourReplaceDialog,wxDialog)
	//(*EventTable(ColourReplaceDialog)
	//*)
END_EVENT_TABLE()

class ColoursList : public wxOwnerDrawnComboBox
{
public:
    ColoursList(wxWindow* parent, wxWindowID id,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxValidator& validator = wxDefaultValidator,
               const wxString& name = "ColoursList") :
        wxOwnerDrawnComboBox(parent, id, wxEmptyString, pos, size, 0, nullptr, style | wxCB_READONLY, validator, name)
    {
    }

    virtual wxCoord OnMeasureItem(size_t item) const
    {
        return 18;
    }

    virtual wxCoord OnMeasureItemWidth(size_t item) const
    {
        return GetSize().GetWidth();
    }

    virtual void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const
    {
        if (item == wxNOT_FOUND)
            return;

        wxString s = GetString(item);

        if (s.Contains("Active")) {
            ColorCurve cc(s.ToStdString());
            dc.DrawBitmap(cc.GetImage(GetSize().GetWidth(), rect.GetHeight() - 1, false), GetSize().GetWidth(), rect.GetTop());
        } else {
            xlColor c;
            c.SetFromString(s.ToStdString());
            wxPen p(c.asWxColor());
            wxBrush b(c.asWxColor());
            dc.SetPen(p);
            dc.SetBrush(b);
            dc.DrawRectangle(0, rect.GetTop(), GetSize().GetWidth(), rect.GetHeight() - 1);
        }
    }
};

ColourReplaceDialog::ColourReplaceDialog(xLightsFrame* frame, wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _frame = frame;

	//(*Initialize(ColourReplaceDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, id, _("Replace Color"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Find Color:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_ExistingColour = new ColoursList(this,ID_CUSTOM1,wxDefaultPosition,wxDefaultSize,ZERO,wxDefaultValidator,_T("ID_CUSTOM1"));
	FlexGridSizer1->Add(BitmapButton_ExistingColour, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Replace Color:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_SelectedEffectsOnly = new wxCheckBox(this, ID_CHECKBOX1, _("Selected Effects Only"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_SelectedEffectsOnly->SetValue(false);
	FlexGridSizer1->Add(CheckBox_SelectedEffectsOnly, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Replace = new wxButton(this, ID_BUTTON1, _("Replace"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button_Replace, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Close = new wxButton(this, ID_BUTTON2, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(Button_Close, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ColourReplaceDialog::OnCheckBox_SelectedEffectsOnlyClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColourReplaceDialog::OnButton_ReplaceClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColourReplaceDialog::OnButton_CloseClick);
	//*)

    UpdateExistingColours();

    _ccb = new ColorCurveButton(this, wxNewId(), wxNullBitmap, wxDefaultPosition, wxSize(21, 21), wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, "IDCOLOUR");
    FlexGridSizer2->Add(_ccb, 0, wxALL | wxEXPAND, 0);
    Connect(wxID_ANY, EVT_CC_CHANGED, (wxObjectEventFunction)&ColourReplaceDialog::OnCCChanged, 0, this);
}

ColourReplaceDialog::~ColourReplaceDialog()
{
	//(*Destroy(ColourReplaceDialog)
	//*)
}

void ColourReplaceDialog::UpdateExistingColours()
{
    auto usedColours = _frame->GetSequenceElements().GetUsedColours(CheckBox_SelectedEffectsOnly->GetValue());

    if (BitmapButton_ExistingColour->GetCount() != 0) {
        BitmapButton_ExistingColour->Clear();
    }

    for (const auto& it : usedColours) {
        BitmapButton_ExistingColour->AppendString(it);
    }

    if (BitmapButton_ExistingColour->GetCount() > 0) {
        BitmapButton_ExistingColour->SetSelection(0);
    }

    Refresh();
}

void ColourReplaceDialog::OnButton_ReplaceClick(wxCommandEvent& event)
{
    std::string to;
    if (_ccb->GetValue()->IsActive()) {
        to = _ccb->GetValue()->Serialise();
    } else {
        to = _ccb->GetBackgroundColour().GetAsString(wxC2S_HTML_SYNTAX);
    }

    int replaced = _frame->GetSequenceElements().ReplaceColours(_frame, BitmapButton_ExistingColour->GetString(BitmapButton_ExistingColour->GetSelection()), to, CheckBox_SelectedEffectsOnly->GetValue());

    wxMessageBox(wxString::Format("%d colors replaced.", replaced), "Replaced");

    UpdateExistingColours();
}

void ColourReplaceDialog::OnButton_CloseClick(wxCommandEvent& event)
{
    EndDialog(wxCLOSE);
}

void ColourReplaceDialog::OnCCChanged(wxCommandEvent& event)
{
    //ColorCurveButton* w = (ColorCurveButton*)event.GetEventObject();
    Refresh();
}

void ColourReplaceDialog::OnCheckBox_SelectedEffectsOnlyClick(wxCommandEvent& event)
{
    UpdateExistingColours();
}

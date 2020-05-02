/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "TreePanel.h"
#include "EffectPanelUtils.h"

//(*InternalHeaders(TreePanel)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(TreePanel)
const long TreePanel::ID_STATICTEXT_Tree_Branches = wxNewId();
const long TreePanel::ID_SLIDER_Tree_Branches = wxNewId();
const long TreePanel::IDD_TEXTCTRL_Tree_Branches = wxNewId();
const long TreePanel::ID_BITMAPBUTTON_SLIDER_Tree_Branches = wxNewId();
const long TreePanel::ID_STATICTEXT_Tree_Speed = wxNewId();
const long TreePanel::ID_SLIDER_Tree_Speed = wxNewId();
const long TreePanel::IDD_TEXTCTRL_Tree_Speed = wxNewId();
const long TreePanel::ID_CHECKBOX_Tree_ShowLights = wxNewId();
//*)

BEGIN_EVENT_TABLE(TreePanel,wxPanel)
	//(*EventTable(TreePanel)
	//*)
END_EVENT_TABLE()

TreePanel::TreePanel(wxWindow* parent)
{
	//(*Initialize(TreePanel)
	BulkEditTextCtrl* TextCtrl59;
	BulkEditTextCtrl* TextCtrl60;
	wxFlexGridSizer* FlexGridSizer71;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer71 = new wxFlexGridSizer(5, 4, 0, 0);
	FlexGridSizer71->AddGrowableCol(1);
	StaticText86 = new wxStaticText(this, ID_STATICTEXT_Tree_Branches, _("Number Branches"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Tree_Branches"));
	FlexGridSizer71->Add(StaticText86, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Tree_Branches = new BulkEditSlider(this, ID_SLIDER_Tree_Branches, 3, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Tree_Branches"));
	FlexGridSizer71->Add(Slider_Tree_Branches, 1, wxALL|wxEXPAND, 5);
	TextCtrl59 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Tree_Branches, _("3"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Tree_Branches"));
	TextCtrl59->SetMaxLength(3);
	FlexGridSizer71->Add(TextCtrl59, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_TreeBranches = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Tree_Branches, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Tree_Branches"));
	BitmapButton_TreeBranches->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer71->Add(BitmapButton_TreeBranches, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText182 = new wxStaticText(this, ID_STATICTEXT_Tree_Speed, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Tree_Speed"));
	FlexGridSizer71->Add(StaticText182, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Tree_Speed = new BulkEditSlider(this, ID_SLIDER_Tree_Speed, 10, 1, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Tree_Speed"));
	FlexGridSizer71->Add(Slider_Tree_Speed, 1, wxALL|wxEXPAND, 5);
	TextCtrl60 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Tree_Speed, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Tree_Speed"));
	TextCtrl60->SetMaxLength(3);
	FlexGridSizer71->Add(TextCtrl60, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer71->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer71->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox1 = new BulkEditCheckBox(this, ID_CHECKBOX_Tree_ShowLights, _("Show Tree Lights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Tree_ShowLights"));
	CheckBox1->SetValue(false);
	FlexGridSizer71->Add(CheckBox1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer71->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer71->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer71);
	FlexGridSizer71->Fit(this);
	FlexGridSizer71->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Tree_Branches,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TreePanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_TREE");
}

TreePanel::~TreePanel()
{
	//(*Destroy(TreePanel)
	//*)
}

PANEL_EVENT_HANDLERS(TreePanel)

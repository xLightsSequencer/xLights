/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "LorConvertDialog.h"

//(*InternalHeaders(LorConvertDialog)
#include <wx/button.h>
#include <wx/string.h>
#include <wx/intl.h>
//*)

//(*IdInit(LorConvertDialog)
const long LorConvertDialog::ID_STATICTEXT17 = wxNewId();
const long LorConvertDialog::ID_CHECKBOX_OFF_AT_END = wxNewId();
const long LorConvertDialog::ID_STATICTEXT20 = wxNewId();
const long LorConvertDialog::ID_CHECKBOX_MAP_EMPTY_CHANNELS = wxNewId();
const long LorConvertDialog::ID_STATICTEXT33 = wxNewId();
const long LorConvertDialog::ID_CHECKBOX_LOR_WITH_NO_CHANNELS = wxNewId();
//*)

BEGIN_EVENT_TABLE(LorConvertDialog,wxDialog)
	//(*EventTable(LorConvertDialog)
	//*)
END_EVENT_TABLE()

LorConvertDialog::LorConvertDialog(wxWindow* parent)
{
	//(*Initialize(LorConvertDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText17 = new wxStaticText(this, ID_STATICTEXT17, _("All channels off at end:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT17"));
	FlexGridSizer2->Add(StaticText17, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBoxOffAtEnd = new wxCheckBox(this, ID_CHECKBOX_OFF_AT_END, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_OFF_AT_END"));
	CheckBoxOffAtEnd->SetValue(false);
	FlexGridSizer2->Add(CheckBoxOffAtEnd, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText20 = new wxStaticText(this, ID_STATICTEXT20, _("Map Empty LMS Channels:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT20"));
	FlexGridSizer2->Add(StaticText20, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBoxMapEmptyChannels = new wxCheckBox(this, ID_CHECKBOX_MAP_EMPTY_CHANNELS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MAP_EMPTY_CHANNELS"));
	CheckBoxMapEmptyChannels->SetValue(false);
	FlexGridSizer2->Add(CheckBoxMapEmptyChannels, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText31 = new wxStaticText(this, ID_STATICTEXT33, _("Map LMS Channels with no network:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT33"));
	FlexGridSizer2->Add(StaticText31, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	MapLORChannelsWithNoNetwork = new wxCheckBox(this, ID_CHECKBOX_LOR_WITH_NO_CHANNELS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_LOR_WITH_NO_CHANNELS"));
	MapLORChannelsWithNoNetwork->SetValue(false);
	FlexGridSizer2->Add(MapLORChannelsWithNoNetwork, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

LorConvertDialog::~LorConvertDialog()
{
	//(*Destroy(LorConvertDialog)
	//*)
}


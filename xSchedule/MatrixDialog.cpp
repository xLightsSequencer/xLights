/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MatrixDialog.h"
#include "RGBEffects.h"
#include "ScheduleManager.h"
#include "xScheduleMain.h"
#include "../xLights/outputs/OutputManager.h"

//(*InternalHeaders(MatrixDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MatrixDialog)
const long MatrixDialog::ID_STATICTEXT7 = wxNewId();
const long MatrixDialog::ID_TEXTCTRL1 = wxNewId();
const long MatrixDialog::ID_STATICTEXT9 = wxNewId();
const long MatrixDialog::ID_CHOICE_MODEL = wxNewId();
const long MatrixDialog::ID_STATICTEXT1 = wxNewId();
const long MatrixDialog::ID_SPINCTRL1 = wxNewId();
const long MatrixDialog::ID_STATICTEXT4 = wxNewId();
const long MatrixDialog::ID_SPINCTRL2 = wxNewId();
const long MatrixDialog::ID_STATICTEXT2 = wxNewId();
const long MatrixDialog::ID_SPINCTRL3 = wxNewId();
const long MatrixDialog::ID_STATICTEXT3 = wxNewId();
const long MatrixDialog::ID_TEXTCTRL2 = wxNewId();
const long MatrixDialog::ID_STATICTEXT8 = wxNewId();
const long MatrixDialog::ID_STATICTEXT5 = wxNewId();
const long MatrixDialog::ID_CHOICE1 = wxNewId();
const long MatrixDialog::ID_STATICTEXT6 = wxNewId();
const long MatrixDialog::ID_CHOICE2 = wxNewId();
const long MatrixDialog::ID_BUTTON1 = wxNewId();
const long MatrixDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(MatrixDialog, wxDialog)
//(*EventTable(MatrixDialog)
//*)
END_EVENT_TABLE()

MatrixDialog::MatrixDialog(wxWindow* parent, OutputManager* outputManager, std::string& name, std::string& orientation, std::string& startingLocation, int& stringLength, int& strings, int& strandsPerString, std::string& startChannel, std::string& fromModel, wxWindowID id, const wxPoint& pos, const wxSize& size) :
    _name(name), _stringLength(stringLength), _strings(strings), _strandsPerString(strandsPerString), _startChannel(startChannel), _orientation(orientation), _startingLocation(startingLocation), _fromModel(fromModel) {
    _outputManager = outputManager;

    //(*Initialize(MatrixDialog)
    wxBoxSizer* BoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;

    Create(parent, id, _("Matrix Configuration"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer1->AddGrowableCol(1);
    StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer1->Add(StaticText7, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Name = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer1->Add(TextCtrl_Name, 1, wxALL | wxEXPAND, 5);
    StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("From Model:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer1->Add(StaticText9, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    Choice_FromModel = new wxChoice(this, ID_CHOICE_MODEL, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_MODEL"));
    Choice_FromModel->SetSelection(Choice_FromModel->Append(_("Manually Define")));
    FlexGridSizer1->Add(Choice_FromModel, 1, wxALL | wxEXPAND, 5);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Strings:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticText1, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_Strings = new wxSpinCtrl(this, ID_SPINCTRL1, _T("16"), wxDefaultPosition, wxDefaultSize, 0, 1, 500, 16, _T("ID_SPINCTRL1"));
    SpinCtrl_Strings->SetValue(_T("16"));
    FlexGridSizer1->Add(SpinCtrl_Strings, 1, wxALL | wxEXPAND, 5);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("String Length:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer1->Add(StaticText4, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_StringLength = new wxSpinCtrl(this, ID_SPINCTRL2, _T("150"), wxDefaultPosition, wxDefaultSize, 0, 1, 65535, 150, _T("ID_SPINCTRL2"));
    SpinCtrl_StringLength->SetValue(_T("150"));
    FlexGridSizer1->Add(SpinCtrl_StringLength, 1, wxALL | wxEXPAND, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Strands Per String:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer1->Add(StaticText2, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_StrandsPerString = new wxSpinCtrl(this, ID_SPINCTRL3, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 8192, 1, _T("ID_SPINCTRL3"));
    SpinCtrl_StrandsPerString->SetValue(_T("1"));
    FlexGridSizer1->Add(SpinCtrl_StrandsPerString, 1, wxALL | wxEXPAND, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Start Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer1->Add(StaticText3, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    TextCtrl_StartChannel = new wxTextCtrl(this, ID_TEXTCTRL2, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FlexGridSizer2->Add(TextCtrl_StartChannel, 1, wxALL | wxEXPAND, 5);
    StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("1"), wxDefaultPosition, wxSize(60, -1), 0, _T("ID_STATICTEXT8"));
    FlexGridSizer2->Add(StaticText8, 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL | wxEXPAND, 5);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Start Location:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer1->Add(StaticText5, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    Choice_StartLocation = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    Choice_StartLocation->SetSelection(Choice_StartLocation->Append(_("Bottom Left")));
    Choice_StartLocation->Append(_("Bottom Right"));
    Choice_StartLocation->Append(_("Top Left"));
    Choice_StartLocation->Append(_("Top Right"));
    FlexGridSizer1->Add(Choice_StartLocation, 1, wxALL | wxEXPAND, 5);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Orientation:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer1->Add(StaticText6, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    Choice_Orientation = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    Choice_Orientation->SetSelection(Choice_Orientation->Append(_("Vertical")));
    Choice_Orientation->Append(_("Horizontal"));
    FlexGridSizer1->Add(Choice_Orientation, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer1->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    Button_Ok->SetDefault();
    BoxSizer1->Add(Button_Ok, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    BoxSizer1->Add(Button_Cancel, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(BoxSizer1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_TEXTCTRL1, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&MatrixDialog::OnTextCtrl_NameText);
    Connect(ID_CHOICE_MODEL, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&MatrixDialog::OnChoice_FromModelSelect);
    Connect(ID_SPINCTRL1, wxEVT_COMMAND_SPINCTRL_UPDATED, (wxObjectEventFunction)&MatrixDialog::OnSpinCtrl_StringsChange);
    Connect(ID_SPINCTRL2, wxEVT_COMMAND_SPINCTRL_UPDATED, (wxObjectEventFunction)&MatrixDialog::OnSpinCtrl_StringLengthChange);
    Connect(ID_SPINCTRL3, wxEVT_COMMAND_SPINCTRL_UPDATED, (wxObjectEventFunction)&MatrixDialog::OnSpinCtrl_StrandsPerStringChange);
    Connect(ID_TEXTCTRL2, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&MatrixDialog::OnTextCtrl_StartChannelText);
    Connect(ID_CHOICE1, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&MatrixDialog::OnChoice_StartLocationSelect);
    Connect(ID_CHOICE2, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&MatrixDialog::OnChoice_OrientationSelect);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&MatrixDialog::OnButton_OkClick);
    Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&MatrixDialog::OnButton_CancelClick);
    //*)

    SetEscapeId(Button_Cancel->GetId());

    PopulateModels();

    TextCtrl_Name->SetValue(_name);
    Choice_Orientation->SetStringSelection(_orientation);
    Choice_StartLocation->SetStringSelection(_startingLocation);
    TextCtrl_StartChannel->SetValue(_startChannel);
    SpinCtrl_StrandsPerString->SetValue(_strandsPerString);
    SpinCtrl_StringLength->SetValue(_stringLength);
    SpinCtrl_Strings->SetValue(_strings);
    if (_fromModel != "") {
        Choice_FromModel->SetStringSelection(_fromModel);
        RGBEffects effects;
        wxXmlNode* node = effects.GetModel(Choice_FromModel->GetStringSelection().ToStdString());
        TextCtrl_StartChannel->SetValue(node->GetAttribute("StartChannel", ""));
        long sc = _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
        if (sc == 0 || sc > xScheduleFrame::GetScheduleManager()->GetTotalChannels()) {
            StaticText8->SetLabel("Invalid");
        } else {
            StaticText8->SetLabel(wxString::Format("%ld", (long)sc));
        }
    } else {
        Choice_FromModel->SetSelection(0);
    }

    ValidateWindow();
}

MatrixDialog::~MatrixDialog() {
    //(*Destroy(MatrixDialog)
    //*)
}

void MatrixDialog::OnButton_OkClick(wxCommandEvent& event) {
    _name = TextCtrl_Name->GetValue().ToStdString();
    _orientation = Choice_Orientation->GetStringSelection().ToStdString();
    _startingLocation = Choice_StartLocation->GetStringSelection().ToStdString();
    _startChannel = TextCtrl_StartChannel->GetValue();
    _strandsPerString = SpinCtrl_StrandsPerString->GetValue();
    _stringLength = SpinCtrl_StringLength->GetValue();
    _strings = SpinCtrl_Strings->GetValue();
    if (Choice_FromModel->GetSelection() == 0) {
        _fromModel = "";
    } else {
        _fromModel = Choice_FromModel->GetStringSelection().ToStdString();
    }

    EndDialog(wxID_OK);
}

void MatrixDialog::PopulateModels() {
    // Load models from RGBEffects file ... adding all matrices, trees and custom models
    RGBEffects effects;

    for (const auto& it : effects.GetModels("Horiz Matrix")) {
        Choice_FromModel->Append(it);
    }

    for (const auto& it : effects.GetModels("Vert Matrix")) {
        Choice_FromModel->Append(it);
    }

    for (const auto& it : effects.GetModels("Tree 360")) {
        Choice_FromModel->Append(it);
    }

    for (const auto& it : effects.GetModels("Tree Flat")) {
        Choice_FromModel->Append(it);
    }

    for (const auto& it : effects.GetModels("Tree Ribbon")) {
        Choice_FromModel->Append(it);
    }

    for (const auto& it : effects.GetModels("Custom")) {
        wxXmlNode* node = effects.GetModel(it);
        // we only add 2d custom models
        if (node->GetAttribute("Depth") == "1")
            Choice_FromModel->Append(it);
    }

    for (const auto& it : effects.GetModels("Sphere")) {
        Choice_FromModel->Append(it);
    }
}

void MatrixDialog::OnButton_CancelClick(wxCommandEvent& event) {
    EndDialog(wxID_CANCEL);
}

void MatrixDialog::OnTextCtrl_StartChannelText(wxCommandEvent& event) {
    long sc = _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
    if (sc == 0 || sc > xScheduleFrame::GetScheduleManager()->GetTotalChannels()) {
        StaticText8->SetLabel("Invalid");
    } else {
        StaticText8->SetLabel(wxString::Format("%ld", (long)sc));
    }
    ValidateWindow();
}

void MatrixDialog::ValidateWindow() {
    bool valid = true;

    if (TextCtrl_Name->GetValue() == "") {
        valid = false;
    }

    if (Choice_FromModel->GetSelection() == 0) {
        Choice_Orientation->Enable(true);
        Choice_StartLocation->Enable(true);
        SpinCtrl_StrandsPerString->Enable(true);
        SpinCtrl_StringLength->Enable(true);
        SpinCtrl_Strings->Enable(true);
        TextCtrl_StartChannel->Enable(true);

        if (SpinCtrl_Strings->GetValue() == 0 || SpinCtrl_StringLength->GetValue() == 0 || SpinCtrl_StrandsPerString->GetValue() == 0 || Choice_StartLocation->GetSelection() == -1 || Choice_Orientation->GetSelection() == -1) {
            valid = false;
        }
    } else {
        Choice_Orientation->Enable(false);
        Choice_StartLocation->Enable(false);
        SpinCtrl_StrandsPerString->Enable(false);
        SpinCtrl_StringLength->Enable(false);
        SpinCtrl_Strings->Enable(false);
        TextCtrl_StartChannel->Enable(false);
    }

    if (StaticText8->GetLabel() == "Invalid") {
        valid = false;
    }

    Button_Ok->Enable(valid);
}

void MatrixDialog::OnChoice_FromModelSelect(wxCommandEvent& event) {
    if (Choice_FromModel->GetSelection() != 0) {
        RGBEffects effects;
        wxXmlNode* node = effects.GetModel(Choice_FromModel->GetStringSelection().ToStdString());
        TextCtrl_StartChannel->SetValue(node->GetAttribute("StartChannel", ""));
        long sc = _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
        if (sc == 0 || sc > xScheduleFrame::GetScheduleManager()->GetTotalChannels()) {
            StaticText8->SetLabel("Invalid");
        } else {
            StaticText8->SetLabel(wxString::Format("%ld", (long)sc));
        }
        if (node->GetAttribute("DisplayAs") == "Custom") {
            SpinCtrl_Strings->SetValue("0");
            SpinCtrl_StringLength->SetValue("0");
            SpinCtrl_StrandsPerString->SetValue("0");
            Choice_Orientation->SetSelection(-1);
            Choice_StartLocation->SetSelection(-1);
        } else {
            SpinCtrl_Strings->SetValue(wxAtoi(node->GetAttribute("parm1", "0")));
            SpinCtrl_StringLength->SetValue(wxAtoi(node->GetAttribute("parm2", "0")));
            SpinCtrl_StrandsPerString->SetValue(wxAtoi(node->GetAttribute("parm3", "1")));
            if (node->GetAttribute("DisplayAs") == "Horix Matrix") {
                Choice_Orientation->SetStringSelection("Horizontal");
            } else {
                Choice_Orientation->SetStringSelection("Vertical");
            }
            std::string startSide = node->GetAttribute("StartSide", "B");
            std::string dir = node->GetAttribute("Dir", "L");
            if (startSide == "B" && dir == "L") {
                Choice_StartLocation->SetStringSelection("Bottom Left");
            } else if (startSide == "B" && dir == "R") {
                Choice_StartLocation->SetStringSelection("Bottom Right");
            } else if (startSide == "T" && dir == "L") {
                Choice_StartLocation->SetStringSelection("Top Left");
            } else {
                Choice_StartLocation->SetStringSelection("Top Right");
            }
        }
    }

    ValidateWindow();
}

void MatrixDialog::OnChoice_OrientationSelect(wxCommandEvent& event) {
    ValidateWindow();
}

void MatrixDialog::OnChoice_StartLocationSelect(wxCommandEvent& event) {
    ValidateWindow();
}

void MatrixDialog::OnSpinCtrl_StrandsPerStringChange(wxSpinEvent& event) {
    ValidateWindow();
}

void MatrixDialog::OnSpinCtrl_StringLengthChange(wxSpinEvent& event) {
    ValidateWindow();
}

void MatrixDialog::OnSpinCtrl_StringsChange(wxSpinEvent& event) {
    ValidateWindow();
}

void MatrixDialog::OnTextCtrl_NameText(wxCommandEvent& event) {
    ValidateWindow();
}

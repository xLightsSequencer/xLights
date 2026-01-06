/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "VirtualMatrixDialog.h"
#include "RGBEffects.h"
#include "ScheduleManager.h"
#include "xScheduleMain.h"
#include "../xLights/outputs/OutputManager.h"
#include "PlayList/VideoWindowPositionDialog.h"

//(*InternalHeaders(VirtualMatrixDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "./utils/spdlog_macros.h"

//(*IdInit(VirtualMatrixDialog)
const long VirtualMatrixDialog::ID_STATICTEXT7 = wxNewId();
const long VirtualMatrixDialog::ID_TEXTCTRL1 = wxNewId();
const long VirtualMatrixDialog::ID_STATICTEXT8 = wxNewId();
const long VirtualMatrixDialog::ID_CHOICE3 = wxNewId();
const long VirtualMatrixDialog::ID_STATICTEXT1 = wxNewId();
const long VirtualMatrixDialog::ID_SPINCTRL1 = wxNewId();
const long VirtualMatrixDialog::ID_STATICTEXT4 = wxNewId();
const long VirtualMatrixDialog::ID_SPINCTRL2 = wxNewId();
const long VirtualMatrixDialog::ID_STATICTEXT9 = wxNewId();
const long VirtualMatrixDialog::ID_CHOICE4 = wxNewId();
const long VirtualMatrixDialog::ID_STATICTEXT2 = wxNewId();
const long VirtualMatrixDialog::ID_CHOICE1 = wxNewId();
const long VirtualMatrixDialog::ID_STATICTEXT5 = wxNewId();
const long VirtualMatrixDialog::ID_CHOICE2 = wxNewId();
const long VirtualMatrixDialog::ID_STATICTEXT3 = wxNewId();
const long VirtualMatrixDialog::ID_TEXTCTRL2 = wxNewId();
const long VirtualMatrixDialog::ID_STATICTEXT6 = wxNewId();
const long VirtualMatrixDialog::ID_BUTTON3 = wxNewId();
const long VirtualMatrixDialog::ID_CHECKBOX1 = wxNewId();
const long VirtualMatrixDialog::ID_BUTTON1 = wxNewId();
const long VirtualMatrixDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(VirtualMatrixDialog, wxDialog)
//(*EventTable(VirtualMatrixDialog)
//*)
END_EVENT_TABLE()

VirtualMatrixDialog::VirtualMatrixDialog(wxWindow* parent, OutputManager* outputManager, std::string& name, std::string& rotation, std::string& pixelChannels, std::string& quality, wxSize& vmsize, wxPoint& vmlocation, int& width, int& height, bool& topMost, std::string& startChannel, bool& useMatrixSize, int& matrixMultiplier, ScheduleOptions* options, std::string& fromModel, wxWindowID id, const wxPoint& pos, const wxSize& size) :
    _name(name), _width(width), _height(height), _topMost(topMost), _useMatrixSize(useMatrixSize), _matrixMultiplier(matrixMultiplier), _startChannel(startChannel), _size(vmsize), _location(vmlocation), _rotation(rotation), _pixelChannels(pixelChannels), _quality(quality), _fromModel(fromModel) {
    _outputManager = outputManager;
    _tempSize = _size;
    _tempLocation = _location;
    _options = options;

    //(*Initialize(VirtualMatrixDialog)
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
    StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("From Model:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer1->Add(StaticText8, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    Choice_FromModel = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    Choice_FromModel->SetSelection(Choice_FromModel->Append(_("Manually Define")));
    FlexGridSizer1->Add(Choice_FromModel, 1, wxALL | wxEXPAND, 5);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Height:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticText1, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_Height = new wxSpinCtrl(this, ID_SPINCTRL1, _T("16"), wxDefaultPosition, wxDefaultSize, 0, 1, 5000, 16, _T("ID_SPINCTRL1"));
    SpinCtrl_Height->SetValue(_T("16"));
    FlexGridSizer1->Add(SpinCtrl_Height, 1, wxALL | wxEXPAND, 5);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Width:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer1->Add(StaticText4, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_Width = new wxSpinCtrl(this, ID_SPINCTRL2, _T("32"), wxDefaultPosition, wxDefaultSize, 0, 1, 5000, 32, _T("ID_SPINCTRL2"));
    SpinCtrl_Width->SetValue(_T("32"));
    FlexGridSizer1->Add(SpinCtrl_Width, 1, wxALL | wxEXPAND, 5);
    StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Pixel channels:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer1->Add(StaticText9, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    Choice_PixelChannels = new wxChoice(this, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
    Choice_PixelChannels->SetSelection(Choice_PixelChannels->Append(_("RGB")));
    Choice_PixelChannels->Append(_("RGBW"));
    FlexGridSizer1->Add(Choice_PixelChannels, 1, wxALL | wxEXPAND, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Rotation:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer1->Add(StaticText2, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    Choice_Rotation = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    Choice_Rotation->SetSelection(Choice_Rotation->Append(_("None")));
    Choice_Rotation->Append(_("90 CW"));
    Choice_Rotation->Append(_("90 CCW"));
    Choice_Rotation->Append(_("Flip Vertical"));
    Choice_Rotation->Append(_("Flip Horizontal"));
    FlexGridSizer1->Add(Choice_Rotation, 1, wxALL | wxEXPAND, 5);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Scaling Quality:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer1->Add(StaticText5, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    Choice_Quality = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    Choice_Quality->Append(_("Normal"));
    Choice_Quality->SetSelection(Choice_Quality->Append(_("High")));
    Choice_Quality->Append(_("Bicubic"));
    Choice_Quality->Append(_("Bilinear"));
    Choice_Quality->Append(_("Box Average"));
    FlexGridSizer1->Add(Choice_Quality, 1, wxALL | wxEXPAND, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Start Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer1->Add(StaticText3, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    TextCtrl_StartChannel = new wxTextCtrl(this, ID_TEXTCTRL2, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FlexGridSizer2->Add(TextCtrl_StartChannel, 1, wxALL | wxEXPAND, 5);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("1"), wxDefaultPosition, wxSize(60, -1), 0, _T("ID_STATICTEXT6"));
    FlexGridSizer2->Add(StaticText6, 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer1->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Button_Position = new wxButton(this, ID_BUTTON3, _("Position Window"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer1->Add(Button_Position, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Topmost = new wxCheckBox(this, ID_CHECKBOX1, _("Topmost Window"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_Topmost->SetValue(true);
    FlexGridSizer1->Add(CheckBox_Topmost, 1, wxALL | wxEXPAND, 5);
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

    Connect(ID_TEXTCTRL1, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&VirtualMatrixDialog::OnTextCtrl_NameText);
    Connect(ID_CHOICE3, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&VirtualMatrixDialog::OnChoice_FromModelSelect);
    Connect(ID_SPINCTRL1, wxEVT_COMMAND_SPINCTRL_UPDATED, (wxObjectEventFunction)&VirtualMatrixDialog::OnSpinCtrl_HeightChange);
    Connect(ID_SPINCTRL2, wxEVT_COMMAND_SPINCTRL_UPDATED, (wxObjectEventFunction)&VirtualMatrixDialog::OnSpinCtrl_WidthChange);
    Connect(ID_CHOICE4, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&VirtualMatrixDialog::OnChoice_PixelChannelsSelect);
    Connect(ID_CHOICE1, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&VirtualMatrixDialog::OnChoice_RotationSelect);
    Connect(ID_CHOICE2, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&VirtualMatrixDialog::OnChoice_QualitySelect);
    Connect(ID_TEXTCTRL2, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&VirtualMatrixDialog::OnTextCtrl_StartChannelText);
    Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&VirtualMatrixDialog::OnButton_PositionClick);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&VirtualMatrixDialog::OnButton_OkClick);
    Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&VirtualMatrixDialog::OnButton_CancelClick);
    //*)

    // Choice_Quality->Append("Fast Bilinear");
    // Choice_Quality->Append("SWS Bilinear");
    // Choice_Quality->Append("SWS Bicubic");
    // Choice_Quality->Append("X");
    // Choice_Quality->Append("Point");
    // Choice_Quality->Append("Area");
    // Choice_Quality->Append("Bicublin");
    // Choice_Quality->Append("Gauss");
    // Choice_Quality->Append("Sinc");
    // Choice_Quality->Append("Lanczos");
    // Choice_Quality->Append("Spline");

    SetEscapeId(Button_Cancel->GetId());

    PopulateModels();

    Choice_Quality->SetStringSelection("Bilinear");

    TextCtrl_Name->SetValue(_name);
    Choice_Rotation->SetStringSelection(_rotation);
    Choice_PixelChannels->SetStringSelection(_pixelChannels);
    Choice_Quality->SetStringSelection(_quality);
    TextCtrl_StartChannel->SetValue(_startChannel);
    SpinCtrl_Width->SetValue(_width);
    SpinCtrl_Height->SetValue(_height);
    CheckBox_Topmost->SetValue(_topMost);
    if (_fromModel != "") {
        Choice_FromModel->SetStringSelection(_fromModel);
        RGBEffects effects;
        wxXmlNode* node = effects.GetModel(Choice_FromModel->GetStringSelection().ToStdString());
        TextCtrl_StartChannel->SetValue(node->GetAttribute("StartChannel", ""));
        long sc = _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
        if (sc == 0 || sc > xScheduleFrame::GetScheduleManager()->GetTotalChannels()) {
            StaticText6->SetLabel("Invalid");
        } else {
            StaticText6->SetLabel(wxString::Format("%ld", (long)sc));
        }
    } else {
        Choice_FromModel->SetSelection(0);
    }

    ValidateWindow();
}

VirtualMatrixDialog::~VirtualMatrixDialog() {
    //(*Destroy(VirtualMatrixDialog)
    //*)
}

void VirtualMatrixDialog::PopulateModels() {
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

void VirtualMatrixDialog::OnButton_OkClick(wxCommandEvent& event) {
    _name = TextCtrl_Name->GetValue().ToStdString();
    _rotation = Choice_Rotation->GetStringSelection().ToStdString();
    _pixelChannels = Choice_PixelChannels->GetStringSelection().ToStdString();
    _quality = Choice_Quality->GetStringSelection().ToStdString();
    _startChannel = TextCtrl_StartChannel->GetValue().ToStdString();
    _width = SpinCtrl_Width->GetValue();
    _height = SpinCtrl_Height->GetValue();
    _topMost = CheckBox_Topmost->GetValue();
    _size = _tempSize;
    _location = _tempLocation;
    if (Choice_FromModel->GetSelection() == 0) {
        _fromModel = "";
    } else {
        _fromModel = Choice_FromModel->GetStringSelection().ToStdString();
    }

    
    LOG_INFO("Virtual matrix position window saved location (%d, %d) size (%d, %d).", _location.x, _location.y, _size.x, _size.y);

    EndDialog(wxID_OK);
}

void VirtualMatrixDialog::OnButton_CancelClick(wxCommandEvent& event) {
    EndDialog(wxID_CANCEL);
}

void VirtualMatrixDialog::OnButton_PositionClick(wxCommandEvent& event) {
    VideoWindowPositionDialog dlg(this, _options, wxID_ANY, _tempLocation, _tempSize, wxSize(SpinCtrl_Width->GetValue(), SpinCtrl_Height->GetValue()), _useMatrixSize, _matrixMultiplier);

    if (dlg.ShowModal() == wxID_OK) {
        _tempLocation = dlg.GetDesiredPosition();
        _tempSize = dlg.GetDesiredSize();
        _useMatrixSize = dlg.CheckBox_SetSizeBasedOnMatrix->GetValue();
        _matrixMultiplier = dlg.SpinCtrl_SizeMultiplier->GetValue();
    }
}

void VirtualMatrixDialog::OnTextCtrl_StartChannelText(wxCommandEvent& event) {
    long sc = _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
    if (sc == 0 || sc > xScheduleFrame::GetScheduleManager()->GetTotalChannels()) {
        StaticText6->SetLabel("Invalid");
    } else {
        StaticText6->SetLabel(wxString::Format("%ld", (long)sc));
    }
    ValidateWindow();
}

void VirtualMatrixDialog::ValidateWindow() {
    bool valid = true;

    if (TextCtrl_Name->GetValue() == "") {
        valid = false;
    }

    if (Choice_FromModel->GetSelection() == 0) {
        Choice_Rotation->Enable(true);
        Choice_PixelChannels->Enable(true);
        SpinCtrl_Width->Enable(true);
        SpinCtrl_Height->Enable(true);
        TextCtrl_StartChannel->Enable(true);

        if (SpinCtrl_Width->GetValue() == 0 || SpinCtrl_Height->GetValue() == 0 || Choice_PixelChannels->GetSelection() == -1 || Choice_Rotation->GetSelection() == -1) {
            valid = false;
        }
    } else {
        Choice_Rotation->Enable(false);
        Choice_PixelChannels->Enable(false);
        SpinCtrl_Width->Enable(false);
        SpinCtrl_Height->Enable(false);
        TextCtrl_StartChannel->Enable(false);
    }

    if (StaticText6->GetLabel() == "Invalid") {
        valid = false;
    }

    Button_Ok->Enable(valid);
}

void VirtualMatrixDialog::OnChoice_FromModelSelect(wxCommandEvent& event) {
    if (Choice_FromModel->GetSelection() != 0) {
        RGBEffects effects;
        wxXmlNode* node = effects.GetModel(Choice_FromModel->GetStringSelection().ToStdString());
        TextCtrl_StartChannel->SetValue(node->GetAttribute("StartChannel", ""));
        long sc = _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
        if (sc == 0 || sc > xScheduleFrame::GetScheduleManager()->GetTotalChannels()) {
            StaticText6->SetLabel("Invalid");
        } else {
            StaticText6->SetLabel(wxString::Format("%ld", (long)sc));
        }
        Choice_Rotation->SetSelection(-1);
        Choice_PixelChannels->SetSelection(0);
        if (node->GetAttribute("DisplayAs") == "Custom") {
            SpinCtrl_Width->SetValue(node->GetAttribute("parm1", "0"));
            SpinCtrl_Height->SetValue(node->GetAttribute("parm2", "0"));
        } else {
            long strings = wxAtol(node->GetAttribute("parm1", "0"));
            long nodesPerString = wxAtol(node->GetAttribute("parm2", "0"));
            long strandsPerString = wxAtol(node->GetAttribute("parm3", "1"));
            if (node->GetAttribute("DisplayAs") == "Horix Matrix") {
                SpinCtrl_Width->SetValue(strings / strandsPerString);
                SpinCtrl_Height->SetValue(strings * strandsPerString);
            } else {
                SpinCtrl_Height->SetValue(strings / strandsPerString);
                SpinCtrl_Width->SetValue(strings * strandsPerString);
            }
        }
    }

    ValidateWindow();
}

void VirtualMatrixDialog::OnTextCtrl_NameText(wxCommandEvent& event) {
    ValidateWindow();
}

void VirtualMatrixDialog::OnSpinCtrl_HeightChange(wxSpinEvent& event) {
    ValidateWindow();
}

void VirtualMatrixDialog::OnSpinCtrl_WidthChange(wxSpinEvent& event) {
    ValidateWindow();
}

void VirtualMatrixDialog::OnChoice_PixelChannelsSelect(wxCommandEvent& event) {
    ValidateWindow();
}

void VirtualMatrixDialog::OnChoice_RotationSelect(wxCommandEvent& event) {
    ValidateWindow();
}

void VirtualMatrixDialog::OnChoice_QualitySelect(wxCommandEvent& event) {
    ValidateWindow();
}

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "LinkJukeboxButtonDialog.h"
#include "sequencer/MainSequencer.h"
#include "JukeboxPanel.h"

//(*InternalHeaders(LinkJukeboxButtonDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(LinkJukeboxButtonDialog)
const long LinkJukeboxButtonDialog::ID_STATICTEXT1 = wxNewId();
const long LinkJukeboxButtonDialog::ID_TEXTCTRL1 = wxNewId();
const long LinkJukeboxButtonDialog::ID_STATICTEXT6 = wxNewId();
const long LinkJukeboxButtonDialog::ID_TEXTCTRL2 = wxNewId();
const long LinkJukeboxButtonDialog::ID_CHECKBOX1 = wxNewId();
const long LinkJukeboxButtonDialog::ID_RADIOBUTTON2 = wxNewId();
const long LinkJukeboxButtonDialog::ID_STATICTEXT2 = wxNewId();
const long LinkJukeboxButtonDialog::ID_CHOICE1 = wxNewId();
const long LinkJukeboxButtonDialog::ID_STATICTEXT3 = wxNewId();
const long LinkJukeboxButtonDialog::ID_CHOICE2 = wxNewId();
const long LinkJukeboxButtonDialog::ID_STATICTEXT4 = wxNewId();
const long LinkJukeboxButtonDialog::ID_CHOICE3 = wxNewId();
const long LinkJukeboxButtonDialog::ID_RADIOBUTTON1 = wxNewId();
const long LinkJukeboxButtonDialog::ID_STATICTEXT5 = wxNewId();
const long LinkJukeboxButtonDialog::ID_CHOICE4 = wxNewId();
const long LinkJukeboxButtonDialog::ID_BUTTON3 = wxNewId();
const long LinkJukeboxButtonDialog::ID_BUTTON1 = wxNewId();
const long LinkJukeboxButtonDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(LinkJukeboxButtonDialog,wxDialog)
	//(*EventTable(LinkJukeboxButtonDialog)
	//*)
END_EVENT_TABLE()

void LinkJukeboxButtonDialog::ValidateWindow()
{
    if (RadioButton_ED->GetValue())
    {
        Choice_Description->Enable();
        Choice_Model->Disable();
        Choice_Layer->Disable();
        Choice_Time->Disable();
        if (Choice_Description->GetCount() == 0)
        {
            Button_Ok->Disable();
        }
        else
        {
            Button_Ok->Enable();
        }
        if (Choice_Description->GetSelection() >= 0)
        {
            Button_Unlink->Enable();
        }
        else 
        {
            Button_Unlink->Disable();
        }
    }
    else
    {
        Choice_Description->Disable();
        Choice_Model->Enable();
        Choice_Layer->Enable();
        Choice_Time->Enable();
        if (Choice_Model->GetCount() == 0 ||
            Choice_Model->GetSelection() < 0 ||
            Choice_Layer->GetCount() == 0 ||
            Choice_Layer->GetSelection() < 0 ||
            Choice_Time->GetCount() == 0 ||
            Choice_Time->GetSelection() < 0)
        {
            Button_Ok->Disable();
        }
        else
        {
            Button_Ok->Enable();
        }
        if (Choice_Model->GetSelection() >= 0)
        {
            Button_Unlink->Enable();
        }
        else
        {
            Button_Unlink->Disable();
            Button_Ok->Enable();
        }
    }
}

void LinkJukeboxButtonDialog::LoadChoices()
{
    if (RadioButton_ED->GetValue() && Choice_Description->GetCount() == 0)
    {
        auto descs = _mainSequencer->GetAllEffectDescriptions();
        descs.sort();

        for (auto it = descs.begin(); it != descs.end(); ++it)
        {
            Choice_Description->Insert(*it, Choice_Description->GetCount());
        }
    }
    else if (RadioButton_MLT->GetValue())
    {
        if (Choice_Model->GetCount() == 0)
        {
            auto elements = _mainSequencer->GetAllElementNamesWithEffects();
            elements.sort();

            for (auto it = elements.begin(); it != elements.end(); ++it)
            {
                Choice_Model->Insert(*it, Choice_Model->GetCount());
            }
            if (Choice_Model->GetCount() == 1)
            {
                Choice_Model->Select(0);
            }
        }

        if (Choice_Model->GetCount() > 0 && Choice_Model->GetSelection() >= 0 && Choice_Layer->GetCount() == 0)
        {
            std::list<int> layers;
            int layerCount = _mainSequencer->GetElementLayerCount(Choice_Model->GetStringSelection().ToStdString(), &layers);
            for (int i = 1; i <= layerCount; i++)
            {
                if (std::find(layers.begin(), layers.end(), i - 1) != layers.end())
                {
                    Choice_Layer->Insert(wxString::Format("%d", i), Choice_Layer->GetCount());
                }
            }
            if (Choice_Layer->GetCount() == 1)
            {
                Choice_Layer->Select(0);
            }
        }

        if (Choice_Layer->GetCount() > 0 && Choice_Layer->GetSelection() >= 0 && Choice_Time->GetCount() == 0)
        {
            auto effects = _mainSequencer->GetElementLayerEffects(Choice_Model->GetStringSelection().ToStdString(), wxAtoi(Choice_Layer->GetStringSelection()) - 1);
            for (auto it = effects.begin(); it != effects.end(); ++it)
            {
                Choice_Time->Insert(wxString::Format("%d", (*it)->GetStartTimeMS()), Choice_Time->GetCount());
            }
            if (Choice_Time->GetCount() == 1)
            {
                Choice_Time->Select(0);
            }
        }
    }
}

LinkJukeboxButtonDialog::LinkJukeboxButtonDialog(wxWindow* parent, int button, ButtonControl* buttonControl, MainSequencer* mainSequencer, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _mainSequencer = mainSequencer;

	//(*Initialize(LinkJukeboxButtonDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;

	Create(parent, id, _("Link effect"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Button:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Number = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(TextCtrl_Number, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Tooltip:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Tooltip = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer2->Add(TextCtrl_Tooltip, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_LoopEffect = new wxCheckBox(this, ID_CHECKBOX1, _("Loop effect playback"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_LoopEffect->SetValue(true);
	FlexGridSizer2->Add(CheckBox_LoopEffect, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	RadioButton_MLT = new wxRadioButton(this, ID_RADIOBUTTON2, _("Model/Layer/Time"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
	FlexGridSizer1->Add(RadioButton_MLT, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Model:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer3->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Model = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer3->Add(Choice_Model, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Layer:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer3->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Layer = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	FlexGridSizer3->Add(Choice_Layer, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Time:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer3->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Time = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	FlexGridSizer3->Add(Choice_Time, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	RadioButton_ED = new wxRadioButton(this, ID_RADIOBUTTON1, _("Effect Description"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
	FlexGridSizer1->Add(RadioButton_ED, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Description:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer4->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Description = new wxChoice(this, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
	FlexGridSizer4->Add(Choice_Description, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	Button_Unlink = new wxButton(this, ID_BUTTON3, _("Unlink"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer1->Add(Button_Unlink, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer5->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer5->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_RADIOBUTTON2,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&LinkJukeboxButtonDialog::OnRadioButton_Select);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&LinkJukeboxButtonDialog::OnChoice_ModelSelect);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&LinkJukeboxButtonDialog::OnChoice_LayerSelect);
	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&LinkJukeboxButtonDialog::OnChoice_TimeSelect);
	Connect(ID_RADIOBUTTON1,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&LinkJukeboxButtonDialog::OnRadioButton_Select);
	Connect(ID_CHOICE4,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&LinkJukeboxButtonDialog::OnChoice_DescriptionSelect);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LinkJukeboxButtonDialog::OnButton_UnlinkClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LinkJukeboxButtonDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LinkJukeboxButtonDialog::OnButton_CancelClick);
	//*)

    TextCtrl_Number->SetValue(wxString::Format("%d", button));

    RadioButton_MLT->SetValue(true);
    LoadChoices();

    if (buttonControl != nullptr)
    {
        CheckBox_LoopEffect->SetValue(buttonControl->_loop);
        TextCtrl_Tooltip->SetValue(buttonControl->_tooltip);
        if (buttonControl->_type == ButtonControl::LOOKUPTYPE::LTDESCRIPTION)
        {
            RadioButton_ED->SetValue(true);
            ValidateWindow();
            LoadChoices();
            Choice_Description->SetStringSelection(buttonControl->_description);
        }
        else
        {
            RadioButton_MLT->SetValue(true);
            ValidateWindow();
            LoadChoices();
            Choice_Model->SetStringSelection(buttonControl->_element);
            LoadChoices();
            Choice_Layer->SetStringSelection(wxString::Format("%d", buttonControl->_layer));
            LoadChoices();
            Choice_Time->SetStringSelection(wxString::Format("%d", buttonControl->_time));
        }
    }

    SetEscapeId(Button_Cancel->GetId());

    ValidateWindow();
}

LinkJukeboxButtonDialog::~LinkJukeboxButtonDialog()
{
	//(*Destroy(LinkJukeboxButtonDialog)
	//*)
}


void LinkJukeboxButtonDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void LinkJukeboxButtonDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void LinkJukeboxButtonDialog::OnChoice_DescriptionSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void LinkJukeboxButtonDialog::OnChoice_TimeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void LinkJukeboxButtonDialog::OnChoice_LayerSelect(wxCommandEvent& event)
{
    Choice_Time->Clear();
    LoadChoices();
    ValidateWindow();
}

void LinkJukeboxButtonDialog::OnChoice_ModelSelect(wxCommandEvent& event)
{
    Choice_Layer->Clear();
    Choice_Time->Clear();
    LoadChoices();
    ValidateWindow();
}

void LinkJukeboxButtonDialog::OnRadioButton_Select(wxCommandEvent& event)
{
    LoadChoices();
    ValidateWindow();
}

void LinkJukeboxButtonDialog::OnButton_UnlinkClick(wxCommandEvent& event)
{
    Choice_Model->Select(-1);
    Choice_Layer->Clear();
    Choice_Time->Clear();
    Choice_Description->Select(-1);
    LoadChoices();
    ValidateWindow();
}

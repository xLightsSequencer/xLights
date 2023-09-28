/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SelectPanel.h"

#include "sequencer/SequenceElements.h"
#include "sequencer/EffectsGrid.h"
#include "sequencer/MainSequencer.h"
#include "sequencer/Effect.h"
#include "sequencer/Element.h"
#include "UtilFunctions.h"
#include "Color.h"

//(*InternalHeaders(SelectPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(SelectPanel)
const long SelectPanel::ID_STATICTEXT1 = wxNewId();
const long SelectPanel::ID_COMBOBOX_SELECT_EFFECT = wxNewId();
const long SelectPanel::ID_STATICTEXT2 = wxNewId();
const long SelectPanel::ID_LISTBOX_SELECT_MODELS = wxNewId();
const long SelectPanel::ID_BUTTON_SELECT_MODEL_ALL = wxNewId();
const long SelectPanel::ID_STATICTEXT6 = wxNewId();
const long SelectPanel::ID_STATICTEXT3 = wxNewId();
const long SelectPanel::ID_TEXTCTRL_SELECT_STARTTIME = wxNewId();
const long SelectPanel::ID_STATICTEXT5 = wxNewId();
const long SelectPanel::ID_TEXTCTRL_SELECT_ENDTIME = wxNewId();
const long SelectPanel::ID_BUTTON_SELECT_ALL_TIME = wxNewId();
const long SelectPanel::ID_STATICTEXT7 = wxNewId();
const long SelectPanel::ID_COLOURPICKERCTRL_SELECT = wxNewId();
const long SelectPanel::ID_SLIDER_COLOR_SENSITIVITY = wxNewId();
const long SelectPanel::ID_BUTTON_SELECT_ALL_COLOR = wxNewId();
const long SelectPanel::ID_STATICTEXT4 = wxNewId();
const long SelectPanel::ID_LISTCTRL_Select_Effects = wxNewId();
const long SelectPanel::ID_BUTTON_SELECT_EFFECT_ALL = wxNewId();
const long SelectPanel::ID_BUTTON_SELECT_REFRESH = wxNewId();
//*)

BEGIN_EVENT_TABLE(SelectPanel,wxPanel)
	//(*EventTable(SelectPanel)
	//*)
END_EVENT_TABLE()

SelectPanel::SelectPanel(SequenceElements* elements, MainSequencer* sequencer, wxWindow* parent,wxWindowID id)
{
	//(*Initialize(SelectPanel)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	Hide();
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(4);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Effect Type:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ComboBox_Select_Effect = new wxComboBox(this, ID_COMBOBOX_SELECT_EFFECT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT|wxCB_READONLY|wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_COMBOBOX_SELECT_EFFECT"));
	FlexGridSizer1->Add(ComboBox_Select_Effect, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Model:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ListBox_Select_Models = new wxListBox(this, ID_LISTBOX_SELECT_MODELS, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_EXTENDED|wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX_SELECT_MODELS"));
	FlexGridSizer1->Add(ListBox_Select_Models, 1, wxALL|wxEXPAND, 5);
	Button_Select_Model_All = new wxButton(this, ID_BUTTON_SELECT_MODEL_ALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_MODEL_ALL"));
	FlexGridSizer1->Add(Button_Select_Model_All, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Time:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Start"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	TextCtrl_Select_StartTime = new wxTextCtrl(this, ID_TEXTCTRL_SELECT_STARTTIME, _("000.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_SELECT_STARTTIME"));
	TextCtrl_Select_StartTime->SetMaxLength(7);
	TextCtrl_Select_StartTime->SetMinSize(wxDLG_UNIT(this,wxSize(35,-1)));
	FlexGridSizer2->Add(TextCtrl_Select_StartTime, 1, wxALL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("End"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	TextCtrl_Select_EndTime = new wxTextCtrl(this, ID_TEXTCTRL_SELECT_ENDTIME, _("000.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_SELECT_ENDTIME"));
	TextCtrl_Select_EndTime->SetMaxLength(7);
	TextCtrl_Select_EndTime->SetMinSize(wxDLG_UNIT(this,wxSize(35,-1)));
	FlexGridSizer2->Add(TextCtrl_Select_EndTime, 1, wxALL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Select_All_Time = new wxButton(this, ID_BUTTON_SELECT_ALL_TIME, _("Select All "), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_ALL_TIME"));
	FlexGridSizer1->Add(Button_Select_All_Time, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Color:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	ColourPickerCtrlSelect = new wxColourPickerCtrl(this, ID_COLOURPICKERCTRL_SELECT, wxColour(0,0,0), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_COLOURPICKERCTRL_SELECT"));
	BoxSizer1->Add(ColourPickerCtrlSelect, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SliderColorSensitivity = new wxSlider(this, ID_SLIDER_COLOR_SENSITIVITY, 255, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_COLOR_SENSITIVITY"));
	SliderColorSensitivity->SetToolTip(_("Sensitivity of Color Match"));
	BoxSizer1->Add(SliderColorSensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxEXPAND, 5);
	Button_Select_All_Color = new wxButton(this, ID_BUTTON_SELECT_ALL_COLOR, _("Reset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_ALL_COLOR"));
	FlexGridSizer1->Add(Button_Select_All_Color, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Effects\nby Time:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ListCtrl_Select_Effects = new wxListCtrl(this, ID_LISTCTRL_Select_Effects, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxLC_SORT_ASCENDING, wxDefaultValidator, _T("ID_LISTCTRL_Select_Effects"));
	FlexGridSizer1->Add(ListCtrl_Select_Effects, 1, wxALL|wxEXPAND, 5);
	Button_Select_Effect_All = new wxButton(this, ID_BUTTON_SELECT_EFFECT_ALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_EFFECT_ALL"));
	FlexGridSizer1->Add(Button_Select_Effect_All, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Select_Refresh = new wxButton(this, ID_BUTTON_SELECT_REFRESH, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_REFRESH"));
	FlexGridSizer1->Add(Button_Select_Refresh, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_COMBOBOX_SELECT_EFFECT,wxEVT_COMMAND_COMBOBOX_SELECTED,(wxObjectEventFunction)&SelectPanel::OnComboBox_Select_EffectSelected);
	Connect(ID_COMBOBOX_SELECT_EFFECT,wxEVT_COMMAND_COMBOBOX_DROPDOWN,(wxObjectEventFunction)&SelectPanel::OnComboBox_Select_EffectDropdown);
	Connect(ID_LISTBOX_SELECT_MODELS,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&SelectPanel::OnListBox_Select_ModelsSelect);
	Connect(ID_BUTTON_SELECT_MODEL_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SelectPanel::OnButton_Select_Model_AllClick);
	Connect(ID_TEXTCTRL_SELECT_STARTTIME,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SelectPanel::OnTextCtrl_Select_StartTimeText);
	Connect(ID_TEXTCTRL_SELECT_ENDTIME,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SelectPanel::OnTextCtrl_Select_EndTimeText);
	Connect(ID_BUTTON_SELECT_ALL_TIME,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SelectPanel::OnButton_Select_All_TimeClick);
	Connect(ID_COLOURPICKERCTRL_SELECT,wxEVT_COMMAND_COLOURPICKER_CHANGED,(wxObjectEventFunction)&SelectPanel::OnColourPickerCtrlSelectColourChanged);
	Connect(ID_SLIDER_COLOR_SENSITIVITY,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&SelectPanel::OnSliderColorSensitivityCmdSliderUpdated);
	Connect(ID_BUTTON_SELECT_ALL_COLOR,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SelectPanel::OnButton_Select_All_ColorClick);
	Connect(ID_LISTCTRL_Select_Effects,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&SelectPanel::OnListCtrl_Select_EffectsItemSelect);
	Connect(ID_LISTCTRL_Select_Effects,wxEVT_COMMAND_LIST_ITEM_DESELECTED,(wxObjectEventFunction)&SelectPanel::OnListCtrl_Select_EffectsItemDeselect);
	Connect(ID_BUTTON_SELECT_EFFECT_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SelectPanel::OnButton_Select_Effect_AllClick);
	Connect(ID_BUTTON_SELECT_REFRESH,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SelectPanel::OnButton_Select_RefreshClick);
	//*)

    mSequenceElements = elements;
    mainSequencer = sequencer;

    ListCtrl_Select_Effects->AppendColumn("", wxLIST_FORMAT_LEFT, 1000);
}

SelectPanel::~SelectPanel()
{
	//(*Destroy(SelectPanel)
	//*)
}

void SelectPanel::populateModelsList(const std::string& effectType)
{
//    if (effectType.empty()) return; //original behavior (require a type)
    std::vector<wxString> models;

    for (int i = 0; i < mSequenceElements->GetElementCount(); i++) {
        Element* el = mSequenceElements->GetElement(i);
        if (el->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            continue;
        }

        for (int i = 0; i < el->GetEffectLayerCount(); ++i) {
            EffectLayer* elay = el->GetEffectLayer(i);
			if ((effectType.empty() && elay->HasEffects()) || elay->HasEffectsByType(effectType)) {
                models.push_back(el->GetFullName());
                break;
            }
        }

        if (el->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            ModelElement* mel = dynamic_cast<ModelElement*>(el);
            if (mel != nullptr) {
                for (int x = 0; x < mel->GetSubModelAndStrandCount(); ++x) {
                    SubModelElement* sme = mel->GetSubModel(x);
                    if (sme != nullptr) {
                        for (size_t j = 0; j < sme->GetEffectLayerCount(); j++) {
                            EffectLayer* elay = sme->GetEffectLayer(j);
                            if ((effectType.empty() && elay->HasEffects()) || elay->HasEffectsByType(effectType)) {
                                models.push_back(sme->GetFullName());
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    std::sort(models.begin(), models.end());
    ListBox_Select_Models->Set(models);

    if (ListBox_Select_Models->GetCount() == 1) {
        ListBox_Select_Models->SetSelection(0);
        populateEffectsList();
    }

    TextCtrl_Select_EndTime->SetValue(wxString::Format("%05.1f", (mainSequencer->PanelTimeLine->GetTimeLength() / 1000.0)));
}

void SelectPanel::populateEffectsList()
{
    ListCtrl_Select_Effects->ClearAll();
    ListCtrl_Select_Effects->AppendColumn("", wxLIST_FORMAT_LEFT, 1000);

    wxArrayInt modelsSelected;
    ListBox_Select_Models->GetSelections(modelsSelected);

    auto const& type = ComboBox_Select_Effect->GetValue().ToStdString();

    if (modelsSelected.size() != 0) {
        auto const[starttime, endtime] = GetStartAndEndTime();
        std::vector<std::string> models;
        for (auto value : modelsSelected) {
            auto const& modelname = ListBox_Select_Models->GetString(value);
            Element* el = mSequenceElements->GetElement(modelname);
            if (el == nullptr || el->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                continue;
            }

            wxString tmpname;
            if (modelsSelected.size() > 1) {
                tmpname = modelname;
            }

            for (int i = 0; i < el->GetEffectLayerCount(); ++i) {
                EffectLayer* elay = el->GetEffectLayer(i);
                std::vector<Effect*> effs = type.empty()?
					elay->GetAllEffectsByTime(starttime, endtime):
					elay->GetEffectsByTypeAndTime(type, starttime, endtime);
                for (Effect* eff : effs) {
                    if (ContainsColor(eff)) {
                        auto id = ListCtrl_Select_Effects->InsertItem(ListCtrl_Select_Effects->GetItemCount(), wxString::Format("[%s,%s] %s", FORMATTIME(eff->GetStartTimeMS()), FORMATTIME(eff->GetEndTimeMS()), tmpname));
                        ListCtrl_Select_Effects->SetItemPtrData(id, (wxUIntPtr)eff);
                    }
                }
            }

            if (el->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                ModelElement* mel = dynamic_cast<ModelElement*>(el);
                if (mel != nullptr) {
                    for (int x = 0; x < mel->GetSubModelAndStrandCount(); ++x) {
                        SubModelElement* sme = mel->GetSubModel(x);
                        if (sme != nullptr) {
                            for (size_t j = 0; j < sme->GetEffectLayerCount(); j++) {
                                EffectLayer* elay = sme->GetEffectLayer(j);
                                std::vector<Effect*> effs = type.empty()?
									elay->GetAllEffectsByTime(starttime, endtime):
									elay->GetEffectsByTypeAndTime(type, starttime, endtime);
                                for (Effect* eff : effs) {
                                    if (ContainsColor(eff)) {
                                        auto id = ListCtrl_Select_Effects->InsertItem(ListCtrl_Select_Effects->GetItemCount() , wxString::Format("[%s,%s] %s", FORMATTIME(eff->GetStartTimeMS()), FORMATTIME(eff->GetEndTimeMS()), tmpname));
                                        ListCtrl_Select_Effects->SetItemPtrData(id, (wxUIntPtr)eff);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (ListCtrl_Select_Effects->GetItemCount() == 1) {
            ListCtrl_Select_Effects->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            SelectEffects();
        }
    }
}

void SelectPanel::SelectEffects()
{
    wxArrayInt effectsSelected;
    for (uint32_t i = 0; i < ListCtrl_Select_Effects->GetItemCount(); ++i) {
        if (ListCtrl_Select_Effects->GetItemState(i, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED) {
            effectsSelected.Add(i);
        }
    }

    if (effectsSelected.size() != 0) {
        bool first = true;
        mSequenceElements->UnSelectAllEffects();

        for (auto value : effectsSelected) {
            Effect* eff = (Effect*)ListCtrl_Select_Effects->GetItemData(value);
            if (eff != nullptr) {
                eff->SetSelected(EFFECT_SELECTED);
                if (first) {
                    mainSequencer->PanelEffectGrid->RaiseSelectedEffectChanged(eff, false);
                    first = false;
                }
            }
        }
        mainSequencer->PanelEffectGrid->Refresh();
    }
}

void SelectPanel::OnButton_Select_RefreshClick(wxCommandEvent& event)
{
    if (!ListCtrl_Select_Effects->GetItemCount()) OnComboBox_Select_EffectDropdown(event); //kludge: compensate for missed evt
    populateModelsList(event.GetString().ToStdString());
    populateEffectsList();
}

void SelectPanel::OnComboBox_Select_ModelsTextEnter(wxCommandEvent& event)
{
    populateModelsList(event.GetString().ToStdString());
}

void SelectPanel::OnListBox_Select_ModelsSelect(wxCommandEvent& event)
{
    populateEffectsList();
}

void SelectPanel::OnButton_Select_Model_AllClick(wxCommandEvent& event)
{
    for (size_t i = 0; i < ListBox_Select_Models->GetCount(); ++i) {
        ListBox_Select_Models->SetSelection(i);
    }
    populateEffectsList();
}

void SelectPanel::OnButton_Select_Effect_AllClick(wxCommandEvent& event)
{
    for (size_t i = 0; i < ListCtrl_Select_Effects->GetItemCount(); ++i) {
        ListCtrl_Select_Effects->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
    SelectEffects();
}

void SelectPanel::OnTextCtrl_Select_StartTimeText(wxCommandEvent& event)
{
    populateEffectsList();
}

void SelectPanel::OnTextCtrl_Select_EndTimeText(wxCommandEvent& event)
{
    populateEffectsList();
}

std::pair< int, int > SelectPanel::GetStartAndEndTime()
{
    //convert from string value of seconds to int of milliseconds
    int startTime = wxAtoi(TextCtrl_Select_StartTime->GetValue()) * 1000;
    int endTime = wxAtoi(TextCtrl_Select_EndTime->GetValue()) * 1000;
    return std::make_pair(startTime, endTime);
}

void SelectPanel::OnButton_Select_All_TimeClick(wxCommandEvent& event)
{
    TextCtrl_Select_StartTime->SetValue("000.0");
    TextCtrl_Select_EndTime->SetValue(wxString::Format("%05.1f", (mainSequencer->PanelTimeLine->GetTimeLength() / 1000.0)));
}

void SelectPanel::OnComboBox_Select_EffectDropdown(wxCommandEvent& event)
{
    GetEffectTypes();
}

void SelectPanel::OnComboBox_Select_EffectSelected(wxCommandEvent& event)
{
    populateModelsList(ComboBox_Select_Effect->GetValue().ToStdString());
}

void SelectPanel::GetEffectTypes()
{
    auto const& types = mSequenceElements->GetAllUsedEffectTypes();
    std::vector<wxString> keys;
    for (std::string const& typ : types) {
        keys.push_back(typ);
    }
    std::sort(keys.begin(), keys.end());
    ComboBox_Select_Effect->Set(keys);
}

void SelectPanel::ClearData()
{
    ComboBox_Select_Effect->Clear();
    ListCtrl_Select_Effects->ClearAll();
    ListCtrl_Select_Effects->AppendColumn("", wxLIST_FORMAT_LEFT, 1000);
    ListBox_Select_Models->Clear();
}

bool SelectPanel::ContainsColor(Effect* eff) const
{
    int const diff = SliderColorSensitivity->GetValue();

    xlColor const search_color = xlColor(ColourPickerCtrlSelect->GetColour());

    // if the effect has no colour then process it as white
    if (eff->GetPalette().size() == 0) {
        if (std::abs(search_color.Red() - xlWHITE.Red()) <= diff &&
            std::abs(search_color.Green() - xlWHITE.Green()) <= diff &&
            std::abs(search_color.Blue() - xlWHITE.Blue()) <= diff) {
            return true;
        }
    }
    else {
        for (auto const& color : eff->GetPalette()) {
            if (std::abs(search_color.Red() - color.Red()) <= diff &&
                std::abs(search_color.Green() - color.Green()) <= diff &&
                std::abs(search_color.Blue() - color.Blue()) <= diff) {
                return true;
            }
        }
    }
    return false;
}

void SelectPanel::OnColourPickerCtrlSelectColourChanged(wxColourPickerEvent& event)
{
    SliderColorSensitivity->SetValue(0);
    populateEffectsList();
}

void SelectPanel::OnSliderColorSensitivityCmdSliderUpdated(wxScrollEvent& event)
{
    populateEffectsList();
}

void SelectPanel::OnButton_Select_All_ColorClick(wxCommandEvent& event)
{
    ColourPickerCtrlSelect->SetColour(*wxBLACK);
    SliderColorSensitivity->SetValue(255);
}

void SelectPanel::OnListCtrl_Select_EffectsItemSelect(wxListEvent& event)
{
    SelectEffects();
    ListCtrl_Select_Effects->SetFocus();
}

void SelectPanel::OnListCtrl_Select_EffectsItemDeselect(wxListEvent& event)
{
    SelectEffects();
    ListCtrl_Select_Effects->SetFocus();
}

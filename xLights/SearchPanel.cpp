/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SearchPanel.h"

#include "sequencer/SequenceElements.h"
#include "sequencer/EffectsGrid.h"
#include "sequencer/MainSequencer.h"
#include "sequencer/Effect.h"
#include "sequencer/Element.h"
#include "UtilFunctions.h"

//(*InternalHeaders(SearchPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/regex.h>

//(*IdInit(SearchPanel)
const long SearchPanel::ID_STATICTEXT1 = wxNewId();
const long SearchPanel::ID_COMBOBOX_SEARCH_MODEL = wxNewId();
const long SearchPanel::ID_CHECKBOX_SEARCH_REGEX = wxNewId();
const long SearchPanel::ID_STATICTEXT2 = wxNewId();
const long SearchPanel::ID_TEXTCTRL_SEARCH = wxNewId();
const long SearchPanel::ID_BUTTON_SEARCH_FIND = wxNewId();
const long SearchPanel::ID_LISTCTRL_Results = wxNewId();
const long SearchPanel::ID_BUTTON_SELECT_ALL = wxNewId();
const long SearchPanel::ID_STATICTEXT_COUNT = wxNewId();
//*)

BEGIN_EVENT_TABLE(SearchPanel,wxPanel)
	//(*EventTable(SearchPanel)
	//*)
END_EVENT_TABLE()

SearchPanel::SearchPanel(SequenceElements* elements, MainSequencer* sequencer, wxWindow* parent,wxWindowID id)
{
	//(*Initialize(SearchPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxSize(574,376), wxTAB_TRAVERSAL, _T("wxID_ANY"));
	SetMinSize(wxSize(-1,-1));
	Hide();
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(2);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Model:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ComboBox_Search_Model = new wxComboBox(this, ID_COMBOBOX_SEARCH_MODEL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT|wxCB_READONLY|wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_COMBOBOX_SEARCH_MODEL"));
	FlexGridSizer1->Add(ComboBox_Search_Model, 1, wxALL|wxEXPAND, 5);
	CheckBox_Search_Regex = new wxCheckBox(this, ID_CHECKBOX_SEARCH_REGEX, _("Regex"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_SEARCH_REGEX"));
	CheckBox_Search_Regex->SetValue(false);
	FlexGridSizer1->Add(CheckBox_Search_Regex, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Search:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlSearch = new wxTextCtrl(this, ID_TEXTCTRL_SEARCH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_SEARCH"));
	FlexGridSizer1->Add(TextCtrlSearch, 1, wxALL|wxEXPAND, 5);
	Button_Search_Find = new wxButton(this, ID_BUTTON_SEARCH_FIND, _("Find"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SEARCH_FIND"));
	FlexGridSizer1->Add(Button_Search_Find, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ListCtrl_Results = new wxListCtrl(this, ID_LISTCTRL_Results, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxLC_SORT_ASCENDING, wxDefaultValidator, _T("ID_LISTCTRL_Results"));
	FlexGridSizer1->Add(ListCtrl_Results, 1, wxALL|wxEXPAND, 5);
	ButtonSelectAll = new wxButton(this, ID_BUTTON_SELECT_ALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_ALL"));
	FlexGridSizer1->Add(ButtonSelectAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Count = new wxStaticText(this, ID_STATICTEXT_COUNT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_COUNT"));
	FlexGridSizer1->Add(StaticText_Count, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(ID_COMBOBOX_SEARCH_MODEL,wxEVT_COMMAND_COMBOBOX_DROPDOWN,(wxObjectEventFunction)&SearchPanel::OnComboBox_Search_ModelDropdown);
	Connect(ID_TEXTCTRL_SEARCH,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&SearchPanel::OnTextCtrlSearchTextEnter);
	Connect(ID_BUTTON_SEARCH_FIND,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SearchPanel::OnButton_Search_FindClick);
	Connect(ID_LISTCTRL_Results,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&SearchPanel::OnListCtrl_ResultsItemSelect);
	Connect(ID_LISTCTRL_Results,wxEVT_COMMAND_LIST_ITEM_DESELECTED,(wxObjectEventFunction)&SearchPanel::OnListCtrl_ResultsItemDeselect);
	Connect(ID_BUTTON_SELECT_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SearchPanel::OnButtonSelectAllClick);
	//*)

    mSequenceElements = elements;
    mainSequencer = sequencer;

    ListCtrl_Results->AppendColumn("", wxLIST_FORMAT_LEFT, 1000);
}

SearchPanel::~SearchPanel()
{
	//(*Destroy(SearchPanel)
	//*)
}

void SearchPanel::ClearData()
{
    ComboBox_Search_Model->Clear();
    TextCtrlSearch->Clear();
    ListCtrl_Results->ClearAll();
    ListCtrl_Results->AppendColumn("", wxLIST_FORMAT_LEFT, 1000);
}

void SearchPanel::OnButton_Search_FindClick(wxCommandEvent& event)
{
    FindSettings();
}

void SearchPanel::OnTextCtrlSearchTextEnter(wxCommandEvent& event)
{
    FindSettings();
}

void SearchPanel::OnComboBox_Search_ModelDropdown(wxCommandEvent& event)
{
    PopulateModelsList();
}

void SearchPanel::OnButtonSelectAllClick(wxCommandEvent& event)
{
    for (uint32_t i = 0; i < ListCtrl_Results->GetItemCount(); ++i) {
        ListCtrl_Results->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
    SelectEffects();
}

std::vector<wxString> SearchPanel::GetModelList() const
{
    std::vector<wxString> models;

    for (int i = 0; i < mSequenceElements->GetElementCount(); i++) {
        auto* el = mSequenceElements->GetElement(i);
        if (el->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            continue;
        }

        for (int i = 0; i < el->GetEffectLayerCount(); ++i) {
            auto* elay = el->GetEffectLayer(i);
            if (elay->HasEffects()) {
                models.emplace_back(el->GetFullName());
                break;
            }
        }

        if (el->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            auto* mel = dynamic_cast<ModelElement*>(el);
            if (mel != nullptr) {
                for (int x = 0; x < mel->GetSubModelAndStrandCount(); ++x) {
                    auto* sme = mel->GetSubModel(x);
                    if (sme != nullptr) {
                        for (size_t j = 0; j < sme->GetEffectLayerCount(); j++) {
                            auto* elay = sme->GetEffectLayer(j);
                            if (elay->HasEffects()) {
                                models.emplace_back(sme->GetFullName());
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    std::sort(models.begin(), models.end());
    return models;
}

void SearchPanel::PopulateModelsList()
{
    auto models = GetModelList();
    models.insert(models.begin(), "<<ALL>>");
    ComboBox_Search_Model->Set(models);
}

void SearchPanel::FindSettings()
{
    ListCtrl_Results->ClearAll();
    ListCtrl_Results->AppendColumn("", wxLIST_FORMAT_LEFT, 1000);

    auto const& search = TextCtrlSearch->GetValue();

    if (search.IsEmpty()) {
        return;
    }
    auto regex = CheckBox_Search_Regex->IsChecked();
    std::vector<wxString> models;

    auto const& selmodel = ComboBox_Search_Model->GetValue();
    if (selmodel == "<<ALL>>") {
        models = GetModelList();
    } else {
        models.push_back(selmodel);
    }

    for (auto const& model : models) {
        auto* el = mSequenceElements->GetElement(model);
        if (el == nullptr || el->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            continue;
        }

        wxString tmpname;
        if (models.size() > 1) {
            tmpname = model;
        }
        std::string value;
        for (int i = 0; i < el->GetEffectLayerCount(); ++i) {
            auto* elay = el->GetEffectLayer(i);
            auto effs = elay->GetEffects();
            for (auto* eff : effs) {
                if (ContainsSetting(eff, search, regex, value)) {
                    auto id = ListCtrl_Results->InsertItem(ListCtrl_Results->GetItemCount(), wxString::Format("%s [%s,%s] %s %s", value, FORMATTIME(eff->GetStartTimeMS()), FORMATTIME(eff->GetEndTimeMS()), eff->GetEffectName(), tmpname));
                    ListCtrl_Results->SetItemPtrData(id, (wxUIntPtr)eff);
                }
            }
        }

        if (el->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            auto* mel = dynamic_cast<ModelElement*>(el);
            if (mel != nullptr) {
                for (int x = 0; x < mel->GetSubModelAndStrandCount(); ++x) {
                    auto* sme = mel->GetSubModel(x);
                    if (sme != nullptr) {
                        for (size_t j = 0; j < sme->GetEffectLayerCount(); j++) {
                            auto* elay = sme->GetEffectLayer(j);
                            auto effs = elay->GetEffects();
                            for (auto* eff : effs) {
                                if (ContainsSetting(eff, search, regex, value)) {
                                    auto id = ListCtrl_Results->InsertItem(ListCtrl_Results->GetItemCount() , wxString::Format("%s [%s,%s] %s %s", value, FORMATTIME(eff->GetStartTimeMS()), FORMATTIME(eff->GetEndTimeMS()), eff->GetEffectName(), tmpname));
                                    ListCtrl_Results->SetItemPtrData(id, (wxUIntPtr)eff);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    StaticText_Count->SetLabel(wxString::Format("%d Effects Found", ListCtrl_Results->GetItemCount()));
    if (ListCtrl_Results->GetItemCount() == 1) {
        ListCtrl_Results->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SelectEffects();
    }
}

bool SearchPanel::ContainsSetting(Effect* eff, std::string const& search, bool regex, std::string &value) const
{
    value.clear();

    auto compare = [&](std::string const& val) {
        if (regex) {
            wxRegEx re(search, wxRE_ADVANCED | wxRE_NEWLINE);
            return re.Matches(val);
        }
        return ::Contains(::Lower(val), ::Lower(search));
    };

    for (auto [key, setting] : eff->GetSettings()) {
        std::string cmpvalue{ key + "=" + setting };
        if (compare(cmpvalue)) {
            value = cmpvalue;
            return true;
        }
    }
    for (auto [key, setting] : eff->GetPaletteMap()) {
        std::string cmpvalue{ key + "=" + setting };
        if (compare(cmpvalue)) {
            value = cmpvalue;
            return true;
        }
    }
    return false;
}

void SearchPanel::SelectEffects()
{
    wxArrayInt effectsSelected;
    for (uint32_t i = 0; i < ListCtrl_Results->GetItemCount(); ++i) {
        if (ListCtrl_Results->GetItemState(i, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED) {
            effectsSelected.Add(i);
        }
    }

    if (!effectsSelected.empty()) {
        bool first = true;
        mSequenceElements->UnSelectAllEffects();

        for (auto value : effectsSelected) {
            auto* eff = (Effect*)ListCtrl_Results->GetItemData(value);
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

void SearchPanel::OnListCtrl_ResultsItemSelect(wxListEvent& event)
{
    SelectEffects();
    ListCtrl_Results->SetFocus();
}

void SearchPanel::OnListCtrl_ResultsItemDeselect(wxListEvent& event)
{
    SelectEffects();
    ListCtrl_Results->SetFocus();
}

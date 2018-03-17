#include "SelectPanel.h"

#include "sequencer/SequenceElements.h"
#include "sequencer/EffectsGrid.h"
#include "sequencer/MainSequencer.h"
#include "sequencer/Effect.h"
#include "sequencer/Element.h"
#include "UtilFunctions.h"

//(*InternalHeaders(SelectPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(SelectPanel)
const long SelectPanel::ID_STATICTEXT1 = wxNewId();
const long SelectPanel::ID_COMBOBOX_SELECT_MODELS = wxNewId();
const long SelectPanel::ID_BUTTON_SELECT_SEARCH = wxNewId();
const long SelectPanel::ID_STATICTEXT2 = wxNewId();
const long SelectPanel::ID_LISTBOX_SELECT_FOUND = wxNewId();
const long SelectPanel::ID_BUTTON_SELECT_MODEL_ALL = wxNewId();
const long SelectPanel::ID_STATICTEXT6 = wxNewId();
const long SelectPanel::ID_STATICTEXT3 = wxNewId();
const long SelectPanel::ID_TEXTCTRL_SELECT_STARTTIME = wxNewId();
const long SelectPanel::ID_STATICTEXT5 = wxNewId();
const long SelectPanel::ID_TEXTCTRL_SELECT_ENDTIME = wxNewId();
const long SelectPanel::ID_BUTTON_SELECT_ALL_TIME = wxNewId();
const long SelectPanel::ID_STATICTEXT4 = wxNewId();
const long SelectPanel::ID_LISTBOX_SELECT_TYPES = wxNewId();
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
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	Hide();
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(3);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Search"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ComboBox_Select_Models = new wxComboBox(this, ID_COMBOBOX_SELECT_MODELS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT|wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_COMBOBOX_SELECT_MODELS"));
	FlexGridSizer1->Add(ComboBox_Select_Models, 1, wxALL|wxEXPAND, 5);
	Button_Select_Search = new wxButton(this, ID_BUTTON_SELECT_SEARCH, _("Search"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_SEARCH"));
	FlexGridSizer1->Add(Button_Select_Search, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Model"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ListBox_Select_Found = new wxListBox(this, ID_LISTBOX_SELECT_FOUND, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_EXTENDED|wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX_SELECT_FOUND"));
	FlexGridSizer1->Add(ListBox_Select_Found, 1, wxALL|wxEXPAND, 5);
	Button_Select_Model_All = new wxButton(this, ID_BUTTON_SELECT_MODEL_ALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_MODEL_ALL"));
	FlexGridSizer1->Add(Button_Select_Model_All, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Time"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Start"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	TextCtrl_Select_StartTime = new wxTextCtrl(this, ID_TEXTCTRL_SELECT_STARTTIME, _("0.000"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_SELECT_STARTTIME"));
	TextCtrl_Select_StartTime->SetMaxLength(7);
	TextCtrl_Select_StartTime->SetMinSize(wxDLG_UNIT(this,wxSize(35,-1)));
	FlexGridSizer2->Add(TextCtrl_Select_StartTime, 1, wxALL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("End"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	TextCtrl_Select_EndTime = new wxTextCtrl(this, ID_TEXTCTRL_SELECT_ENDTIME, _("0.000"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_SELECT_ENDTIME"));
	TextCtrl_Select_EndTime->SetMaxLength(7);
	TextCtrl_Select_EndTime->SetMinSize(wxDLG_UNIT(this,wxSize(35,-1)));
	FlexGridSizer2->Add(TextCtrl_Select_EndTime, 1, wxALL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Select_All_Time = new wxButton(this, ID_BUTTON_SELECT_ALL_TIME, _("Select All "), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_ALL_TIME"));
	FlexGridSizer1->Add(Button_Select_All_Time, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Effects"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ListBox_Select_Types = new wxListBox(this, ID_LISTBOX_SELECT_TYPES, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_EXTENDED|wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX_SELECT_TYPES"));
	FlexGridSizer1->Add(ListBox_Select_Types, 1, wxALL|wxEXPAND, 5);
	Button_Select_Effect_All = new wxButton(this, ID_BUTTON_SELECT_EFFECT_ALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_EFFECT_ALL"));
	FlexGridSizer1->Add(Button_Select_Effect_All, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Select_Refresh = new wxButton(this, ID_BUTTON_SELECT_REFRESH, _("Reset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_REFRESH"));
	FlexGridSizer1->Add(Button_Select_Refresh, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_COMBOBOX_SELECT_MODELS,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&SelectPanel::OnComboBox_Select_ModelsTextEnter);
	Connect(ID_BUTTON_SELECT_SEARCH,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SelectPanel::OnButton_Select_SearchClick);
	Connect(ID_LISTBOX_SELECT_FOUND,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&SelectPanel::OnListBox_Select_FoundSelect);
	Connect(ID_BUTTON_SELECT_MODEL_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SelectPanel::OnButton_Select_Model_AllClick);
	Connect(ID_TEXTCTRL_SELECT_STARTTIME,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SelectPanel::OnTextCtrl_Select_StartTimeText);
	Connect(ID_TEXTCTRL_SELECT_ENDTIME,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SelectPanel::OnTextCtrl_Select_EndTimeText);
	Connect(ID_BUTTON_SELECT_ALL_TIME,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SelectPanel::OnButton_Select_All_TimeClick);
	Connect(ID_LISTBOX_SELECT_TYPES,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&SelectPanel::OnListBox_Select_TypesSelect);
	Connect(ID_BUTTON_SELECT_EFFECT_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SelectPanel::OnButton_Select_Effect_AllClick);
	Connect(ID_BUTTON_SELECT_REFRESH,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SelectPanel::OnButton_Select_RefreshClick);
	//*)

    mSequenceElements = elements;
    mainSequencer = sequencer;
}

SelectPanel::~SelectPanel()
{
	//(*Destroy(SelectPanel)
	//*)
}

void SelectPanel::ReloadModels()
{
    std::vector<wxString> keys;

    // This causes undesirable effects
    // mSequenceElements->UnSelectAllEffects();

    for (size_t i = 0; i < mSequenceElements->GetElementCount(mSequenceElements->GetCurrentView()); i++)
    {
        if (mSequenceElements->GetElement(i, mSequenceElements->GetCurrentView())->GetType() == ELEMENT_TYPE_TIMING)
            continue;
        keys.push_back(mSequenceElements->GetElement(i, mSequenceElements->GetCurrentView())->GetName());
    }

    keys.erase( std::remove_if( keys.begin(), keys.end(), [](wxString const& s) { return s.IsEmpty(); }), keys.end());

    std::sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());

    if (keys.size() != 0)
    {
        ComboBox_Select_Models->Set(keys);
    }
    TextCtrl_Select_EndTime->SetValue(std::to_string(double(mainSequencer->PanelTimeLine->GetTimeLength() / 1000.0)).substr(0,7));
}

void SelectPanel::populateModelsList(const std::string& elementName)
{
    if (elementName.empty())return;

    if (ComboBox_Select_Models->FindString(elementName, false) == -1)
        ComboBox_Select_Models->AppendString(elementName);

    mSequenceElements->UnSelectAllEffects();
    ListBox_Select_Types->Clear();

    std::vector<wxString> keys;
    std::vector< Element * > els = GetElements(elementName);

    std::transform(std::begin(els), std::end(els), std::back_inserter(keys),
        [](auto const& row) { return wxString(row->GetFullName()); });

    if (keys.size() != 0)
    {
        ListBox_Select_Found->Set(keys);
        if (keys.size() == 1)
        {
            ListBox_Select_Found->SetSelection(0);
            populateEffectsList();
        }
    }
    else
    {
        ListBox_Select_Found->Clear();
    }
}

void SelectPanel::populateEffectsList()
{
    wxArrayInt modelsSelected;
    ListBox_Select_Found->GetSelections(modelsSelected);

    if (modelsSelected.size() != 0)
    {
        std::string modelText;

        auto startendtime = GetStartAndEndTime();

        for (auto value : modelsSelected) {
            int i = value;

            if (!modelText.empty())
                modelText += "||";
            modelText += ListBox_Select_Found->GetString(value);
        }

        mSequenceElements->UnSelectAllEffects();
        std::vector<wxString> keys;
        std::vector< Element * > els = GetElements(modelText);

        for (Element * el : els)
        {
            for (int i = 0; i < el->GetEffectLayerCount(); ++i) {
                EffectLayer* elay = el->GetEffectLayer(i);
                std::vector<Effect*> effs = elay->GetAllEffectsByTime(startendtime.first, startendtime.second);

                std::transform(std::begin(effs), std::end(effs), std::back_inserter(keys),
                    [](auto const& row) { return wxString(row->GetEffectName()); });
            }
            if (el->GetType() == ELEMENT_TYPE_STRAND) {
                StrandElement *strEl = dynamic_cast<StrandElement*>(el);
                if (strEl != nullptr) {
                    for (int n = 0; n < strEl->GetNodeLayerCount(); n++) {
                        NodeLayer* nlayer = strEl->GetNodeLayer(n);
                        std::vector<Effect*> effs = nlayer->GetAllEffectsByTime(startendtime.first, startendtime.second);

                        std::transform(std::begin(effs), std::end(effs), std::back_inserter(keys),
                            [](auto const& row) { return wxString(row->GetEffectName()); });
                    }
                }
            }
        }

        std::sort(keys.begin(), keys.end());
        keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
        keys.erase(std::remove_if(keys.begin(), keys.end(), [](wxString const& s) { return s.IsEmpty(); }), keys.end());

        if (keys.size() != 0) {
            ListBox_Select_Types->Set(keys);
            if (keys.size() == 1)
                ListBox_Select_Types->SetSelection(0);

            SelectEffects();
        }
        else {
            ListBox_Select_Types->Clear();
            mainSequencer->PanelEffectGrid->Refresh();
        }
    }
}

void SelectPanel::SelectEffects()
{
    wxArrayInt modelsSelected;
    wxArrayInt effectsSelected;
    ListBox_Select_Found->GetSelections(modelsSelected);
    ListBox_Select_Types->GetSelections(effectsSelected);

    if (modelsSelected.size() != 0 && effectsSelected.size() != 0) {
        std::string modelText;

        for (auto value : modelsSelected) {
            if (!modelText.empty())
                modelText += "||";
            modelText += ListBox_Select_Found->GetString(value);
        }

        mSequenceElements->UnSelectAllEffects();

        std::vector< Element * > els = GetElements(modelText);

        bool first = true;

        auto startendtime = GetStartAndEndTime();

        for (auto value : effectsSelected) {
            const std::string& effectText = ListBox_Select_Types->GetString(value).ToStdString();
            for (Element * el : els) {
                for (int i = 0; i < el->GetEffectLayerCount(); ++i) {
                    if (el->GetEffectLayer(i)->SelectEffectByTypeInTimeRange(effectText, startendtime.first, startendtime.second) != 0 && first) {
                        mainSequencer->PanelEffectGrid->RaiseSelectedEffectChanged(el->GetEffectLayer(i)->GetEffectsByTypeAndTime(effectText, startendtime.first, startendtime.second)[0], false);
                        first = false;
                    }
                }

                if (el->GetType() == ELEMENT_TYPE_STRAND) {
                    StrandElement *strEl = dynamic_cast<StrandElement*>(el);
                    if (strEl != nullptr) {
                        for (int n = 0; n < strEl->GetNodeLayerCount(); n++) {
                            if(strEl->GetNodeLayer(n)->SelectEffectByTypeInTimeRange(effectText, startendtime.first, startendtime.second) != 0 && first) {
                                mainSequencer->PanelEffectGrid->RaiseSelectedEffectChanged(strEl->GetNodeLayer(n)->GetEffectsByTypeAndTime(effectText, startendtime.first, startendtime.second)[0], false);
                                first = false;
                            }
                        }
                    }
                }
            }
        }
        mainSequencer->PanelEffectGrid->Refresh();
    }
}

std::vector< Element * > SelectPanel::GetElements(const std::string& elementName)
{
    std::vector< Element * > elements;
    if (elementName.empty())return elements;
    elements = mSequenceElements->SearchForElements(elementName, mSequenceElements->GetCurrentView());
    return elements;
}

void SelectPanel::OnListBox_Select_TypesSelect(wxCommandEvent& event)
{
    SelectEffects();
}

void SelectPanel::OnButton_Select_RefreshClick(wxCommandEvent& event)
{
    ReloadModels();
}

void SelectPanel::OnComboBox_Select_ModelsTextEnter(wxCommandEvent& event)
{
    populateModelsList(event.GetString().ToStdString());
}

void SelectPanel::OnListBox_Select_FoundSelect(wxCommandEvent& event)
{
    populateEffectsList();
}

void SelectPanel::OnButton_Select_SearchClick(wxCommandEvent& event)
{
    populateModelsList(ComboBox_Select_Models->GetValue().ToStdString());
}

void SelectPanel::OnButton_Select_Model_AllClick(wxCommandEvent& event)
{
    for (int i = 0; i < ListBox_Select_Found->GetCount(); ++i)
        ListBox_Select_Found->SetSelection(i);
    populateEffectsList();
}

void SelectPanel::OnButton_Select_Effect_AllClick(wxCommandEvent& event)
{
    for (int i = 0; i < ListBox_Select_Types->GetCount(); ++i)
        ListBox_Select_Types->SetSelection(i);
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
    TextCtrl_Select_StartTime->SetValue("0.000");
    TextCtrl_Select_EndTime->SetValue(std::to_string(double(mainSequencer->PanelTimeLine->GetTimeLength() / 1000.0)).substr(0, 7));
}

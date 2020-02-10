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
const long SelectPanel::ID_STATICTEXT4 = wxNewId();
const long SelectPanel::ID_LISTBOX_SELECT_EFFECTS = wxNewId();
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
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Effect Type:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
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
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Effects\nby Time:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ListBox_Select_Effects = new wxListBox(this, ID_LISTBOX_SELECT_EFFECTS, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_EXTENDED|wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX_SELECT_EFFECTS"));
	FlexGridSizer1->Add(ListBox_Select_Effects, 1, wxALL|wxEXPAND, 5);
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
	Connect(ID_LISTBOX_SELECT_EFFECTS,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&SelectPanel::OnListBox_Select_EffectsSelect);
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

void SelectPanel::populateModelsList(const std::string& effectType)
{
    if (effectType.empty())return;

    std::vector<wxString> models;

    for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
    {
        Element* el = mSequenceElements->GetElement(i);
        if (el->GetType() == ELEMENT_TYPE_TIMING)
            continue;

        for (int i = 0; i < el->GetEffectLayerCount(); ++i)
        {
            EffectLayer* elay = el->GetEffectLayer(i);
            if (elay->HasEffectsByType(effectType))
            {
                models.push_back(el->GetFullName());
                break;
            }
        }
        if (el->GetType() == ELEMENT_TYPE_STRAND)
        {
            StrandElement* strEl = dynamic_cast<StrandElement*>(el);
            if (strEl != nullptr)
            {
                for (int n = 0; n < strEl->GetNodeLayerCount(); n++)
                {
                    NodeLayer* nlayer = strEl->GetNodeLayer(n);
                    if (nlayer->HasEffectsByType(effectType))
                    {
                        models.push_back(strEl->GetFullName());
                        break;
                    }
                }
            }
        }
    }
    std::sort(models.begin(), models.end());
    ListBox_Select_Models->Set(models);

    if (ListBox_Select_Models->GetCount() == 1)
    {
        ListBox_Select_Models->SetSelection(0);
        populateEffectsList();
    }

    TextCtrl_Select_EndTime->SetValue(wxString::Format("%.5f", (mainSequencer->PanelTimeLine->GetTimeLength() / 1000.0)));
}

void SelectPanel::populateEffectsList()
{
    ListBox_Select_Effects->Clear();
    wxArrayInt modelsSelected;
    ListBox_Select_Models->GetSelections(modelsSelected);

    auto const& type = ComboBox_Select_Effect->GetValue().ToStdString();

    if (modelsSelected.size() != 0)
    {
        auto const startendtime = GetStartAndEndTime();
        std::vector<std::string> models;
        for (auto value : modelsSelected)
        {
            auto const& modelname = ListBox_Select_Models->GetString(value);
            Element* el = mSequenceElements->GetElement(modelname);
            if (el == nullptr || el->GetType() == ELEMENT_TYPE_TIMING)
                continue;

            wxString tmpname;
            if (modelsSelected.size() > 1)
                tmpname = modelname;

            for (int i = 0; i < el->GetEffectLayerCount(); ++i)
            {
                EffectLayer* elay = el->GetEffectLayer(i);
                std::vector<Effect*> effs = elay->GetEffectsByTypeAndTime(type, startendtime.first, startendtime.second);
                for (Effect* eff : effs)
                    ListBox_Select_Effects->Append(wxString::Format("[%05.1fs,%05.1fs] %s", eff->GetStartTimeMS() / 1000.0, eff->GetEndTimeMS() / 1000.0, tmpname),(void * )eff);
            }
            if (el->GetType() == ELEMENT_TYPE_STRAND)
            {
                StrandElement* strEl = dynamic_cast<StrandElement*>(el);
                if (strEl != nullptr)
                {
                    for (int n = 0; n < strEl->GetNodeLayerCount(); n++)
                    {
                        NodeLayer* nlayer = strEl->GetNodeLayer(n);
                        std::vector<Effect*> effs = nlayer->GetEffectsByTypeAndTime(type, startendtime.first, startendtime.second);
                        for (Effect* eff : effs)
                            ListBox_Select_Effects->Append(wxString::Format("[%05.1fs,%05.1fs] %s", eff->GetStartTimeMS() / 1000.0, eff->GetEndTimeMS() / 1000.0, modelname), (void*)eff);
                    }
                }
            }
        }

        if (ListBox_Select_Effects->GetCount() == 1)
        {
            ListBox_Select_Effects->SetSelection(0);
            SelectEffects();
        }
    }
}

void SelectPanel::SelectEffects()
{
    wxArrayInt effectsSelected;
    ListBox_Select_Effects->GetSelections(effectsSelected);

    if (effectsSelected.size() != 0)
    {
        bool first = true;
        mSequenceElements->UnSelectAllEffects();

        for (auto value : effectsSelected)
        {
            Effect* eff = (Effect*)ListBox_Select_Effects->GetClientData(value);
            if (eff != nullptr)
            {
                eff->SetSelected(EFFECT_SELECTED);
                if (first)
                {
                    mainSequencer->PanelEffectGrid->RaiseSelectedEffectChanged(eff, false);
                    first = false;
                }
            }
        }
        mainSequencer->PanelEffectGrid->Refresh();
    }
}

void SelectPanel::OnListBox_Select_EffectsSelect(wxCommandEvent& event)
{
    SelectEffects();
}

void SelectPanel::OnButton_Select_RefreshClick(wxCommandEvent& event)
{
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
    for (size_t i = 0; i < ListBox_Select_Models->GetCount(); ++i)
        ListBox_Select_Models->SetSelection(i);
    populateEffectsList();
}

void SelectPanel::OnButton_Select_Effect_AllClick(wxCommandEvent& event)
{
    for (size_t i = 0; i < ListBox_Select_Effects->GetCount(); ++i)
        ListBox_Select_Effects->SetSelection(i);
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
    TextCtrl_Select_EndTime->SetValue(wxString::Format("%.5f", (mainSequencer->PanelTimeLine->GetTimeLength() / 1000.0)));
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
    for (std::string const& typ : types)
        keys.push_back(typ);
    std::sort(keys.begin(), keys.end());
    ComboBox_Select_Effect->Set(keys);
}

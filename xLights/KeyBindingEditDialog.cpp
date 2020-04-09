
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(KeyBindingEditDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)
#include <wx/headercol.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "KeyBindingEditDialog.h"
#include "KeyBindings.h"
#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "xLightsMain.h"

//(*IdInit(KeyBindingEditDialog)
const long KeyBindingEditDialog::ID_STATICTEXT1 = wxNewId();
const long KeyBindingEditDialog::ID_CHOICE1 = wxNewId();
const long KeyBindingEditDialog::ID_LISTCTRL1 = wxNewId();
const long KeyBindingEditDialog::ID_PANEL1 = wxNewId();
const long KeyBindingEditDialog::ID_BUTTON1 = wxNewId();
const long KeyBindingEditDialog::ID_BUTTON3 = wxNewId();
const long KeyBindingEditDialog::ID_BUTTON2 = wxNewId();
const long KeyBindingEditDialog::ID_BUTTON_CANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(KeyBindingEditDialog,wxDialog)
	//(*EventTable(KeyBindingEditDialog)
	//*)
END_EVENT_TABLE()

KeyBindingEditDialog::KeyBindingEditDialog(xLightsFrame* parent, KeyBindingMap* keyBindings, EffectManager* effectManager, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	_xLights = parent;
	_keyBindings = keyBindings;
	_effectManager = effectManager;

	//(*Initialize(KeyBindingEditDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;

	Create(parent, id, _("Edit Keybindings"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Scope:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer4->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Scope = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer4->Add(Choice_Scope, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	ListCtrl_Bindings = new wxListCtrl(this, ID_LISTCTRL1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL, wxDefaultValidator, _T("ID_LISTCTRL1"));
	FlexGridSizer1->Add(ListCtrl_Bindings, 1, wxALL|wxEXPAND, 2);
	Panel_Properties = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(0);
	Panel_Properties->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel_Properties);
	FlexGridSizer3->SetSizeHints(Panel_Properties);
	FlexGridSizer1->Add(Panel_Properties, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_AddEffect = new wxButton(this, ID_BUTTON1, _("Add Effect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer5->Add(Button_AddEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonAddPreset = new wxButton(this, ID_BUTTON3, _("Add Preset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer5->Add(ButtonAddPreset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonAddApplySetting = new wxButton(this, ID_BUTTON2, _("Add Apply Setting"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer5->Add(ButtonAddApplySetting, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Close = new wxButton(this, ID_BUTTON_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CANCEL"));
	FlexGridSizer2->Add(Button_Close, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&KeyBindingEditDialog::OnChoice_ScopeSelect);
	Connect(ID_LISTCTRL1,wxEVT_COMMAND_LIST_DELETE_ITEM,(wxObjectEventFunction)&KeyBindingEditDialog::OnListCtrl_BindingsDeleteItem);
	Connect(ID_LISTCTRL1,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&KeyBindingEditDialog::OnListCtrl_BindingsItemSelect);
	Connect(ID_LISTCTRL1,wxEVT_COMMAND_LIST_ITEM_FOCUSED,(wxObjectEventFunction)&KeyBindingEditDialog::OnListCtrl_BindingsItemFocused);
	Connect(ID_LISTCTRL1,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&KeyBindingEditDialog::OnListCtrl_BindingsKeyDown);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&KeyBindingEditDialog::OnButton_AddEffectClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&KeyBindingEditDialog::OnButtonAddPresetClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&KeyBindingEditDialog::OnButtonAddApplySettingClick);
	Connect(ID_BUTTON_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&KeyBindingEditDialog::OnButton_CancelClick);
	//*)

	Panel_Properties->SetMinSize(wxSize(500, -1));
	Layout();

	//Choice_Scope->AppendString("Controller");
	Choice_Scope->AppendString("Layout");
	Choice_Scope->AppendString("Sequencer");
	Choice_Scope->AppendString("All tabs");

	Choice_Scope->SetStringSelection("Sequencer");

	ListCtrl_Bindings->AppendColumn("Type");
	ListCtrl_Bindings->AppendColumn("Key", wxLIST_FORMAT_CENTRE, wxLIST_AUTOSIZE_USEHEADER);
	ListCtrl_Bindings->AppendColumn("Control", wxLIST_FORMAT_CENTRE, wxLIST_AUTOSIZE_USEHEADER);
	ListCtrl_Bindings->AppendColumn("Alt", wxLIST_FORMAT_CENTRE, wxLIST_AUTOSIZE_USEHEADER);
	ListCtrl_Bindings->AppendColumn("Shift", wxLIST_FORMAT_CENTRE, wxLIST_AUTOSIZE_USEHEADER);
	ListCtrl_Bindings->AppendColumn("Details");

	LoadList();

	ListCtrl_Bindings->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);
	ListCtrl_Bindings->SetColumnWidth(1, wxCOL_WIDTH_AUTOSIZE);
	ListCtrl_Bindings->SetColumnWidth(5, wxCOL_WIDTH_AUTOSIZE);

	_propertyGrid = new wxPropertyGrid(Panel_Properties, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		// Here are just some of the supported window styles
		//wxPG_AUTO_SORT | // Automatic sorting after items added
		wxPG_SPLITTER_AUTO_CENTER | // Automatically center splitter until user manually adjusts it
		// Default style
		wxPG_DEFAULT_STYLE);
	_propertyGrid->SetExtraStyle(wxWS_EX_PROCESS_IDLE | wxPG_EX_HELP_AS_TOOLTIPS);
	FlexGridSizer3->Add(_propertyGrid, 1, wxALL | wxEXPAND, 5);
	_propertyGrid->Connect(wxEVT_PG_CHANGED, (wxObjectEventFunction)&KeyBindingEditDialog::OnControllerPropertyGridChange, 0, this);
	_propertyGrid->SetValidationFailureBehavior(wxPG_VFB_MARK_CELL | wxPG_VFB_BEEP);

	SetSize(1200, 700);
}

int KeyBindingEditDialog::GetSelectedKeyBindingIndex() const {

    return ListCtrl_Bindings->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}

void KeyBindingEditDialog::SetKeyBindingProperties() {

	int index = GetSelectedKeyBindingIndex();

    _propertyGrid->Freeze();

    // save property grid location
    auto save = _propertyGrid->SaveEditableState();
    wxString selProp = "";
    if (_propertyGrid->GetSelection() != nullptr) {
        selProp = _propertyGrid->GetSelection()->GetName();
    }

    _propertyGrid->Clear();

	if (index < 0) {
		_propertyGrid->Thaw();
		return;
	}

	int id = (int)ListCtrl_Bindings->GetItemData(index);

	if (id < 0) {
		_propertyGrid->Thaw();
		return;
	}

	KeyBinding& b = _keyBindings->GetBinding(id);

	wxPGProperty* p = _propertyGrid->Append(new wxStringProperty("Type", "KBType", b.GetType()));
	p->ChangeFlag(wxPG_PROP_READONLY, true);
	p->SetHelpString(b.GetTip());

	int k = b.GetKey();
	if (k >= 65 && k <= 90) k += 32;

	wxPGChoices choices;
	int val = 0;
	choices.Add("");
	for (const auto& it : KeyBinding::GetPossibleKeys()) {
		if (it == k) val = choices.GetCount();
		choices.Add(KeyBinding::EncodeKey(it, false));
	}
	p = _propertyGrid->Append(new wxEnumProperty("Key", "KBKey", choices, val));

	p = _propertyGrid->Append(new wxBoolProperty("Control", "KBControl", b.RequiresControl()));
	p->SetEditor("CheckBox");

	p = _propertyGrid->Append(new wxBoolProperty("Alt", "KBAlt", b.RequiresAlt()));
	p->SetEditor("CheckBox");

	p = _propertyGrid->Append(new wxBoolProperty("Shift", "KBShift", b.RequiresShift()));
	p->SetEditor("CheckBox");

	if (b.GetType() == "EFFECT")
	{
		wxPGChoices effchoices;
		val = 0;
		effchoices.Add("");
		for (const auto& it : *_effectManager) {
			if (it->Name() == b.GetEffectName()) val = effchoices.GetCount();
			effchoices.Add(it->Name());
		}

		p = _propertyGrid->Append(new wxEnumProperty("Effect", "KBEffect", effchoices, val));
	}

	if (b.GetType() == "EFFECT" || b.GetType() == "APPLYSETTING")
	{
		p = _propertyGrid->Append(new wxStringProperty("Effect Setting", "KBEffectSetting", b.GetEffectString()));
	}

	if (b.GetType() == "PRESET")
	{
		wxPGChoices presetchoices;
		val = 0;
		presetchoices.Add("");
		for (const auto& it : _xLights->GetPresets()) {
			if (it == b.GetEffectName()) val = presetchoices.GetCount();
			presetchoices.Add(it);
		}
		p = _propertyGrid->Append(new wxEnumProperty("Preset", "KBPreset", presetchoices, val));
	}

	// restore property grid location
    _propertyGrid->RestoreEditableState(save);
    if (selProp != "") {
        auto p = _propertyGrid->GetPropertyByName(selProp);
        if (p != nullptr) _propertyGrid->EnsureVisible(p);
    }

    _propertyGrid->Thaw();

    // This has to be done when the Property editor is not frozen ... as it is ignored if called when frozen
    _propertyGrid->ExpandAll();
}

void KeyBindingEditDialog::OnControllerPropertyGridChange(wxPropertyGridEvent& event) {

	int index = GetSelectedKeyBindingIndex();
	if (index < 0)
	{
		wxASSERT(false);
		return;
	}

	int id = (int)ListCtrl_Bindings->GetItemData(index);
	if (id < 0)
	{
		wxASSERT(false);
		return;
	}

	KeyBinding& b = _keyBindings->GetBinding(id);

	wxString name = event.GetPropertyName();

	if (name == "KBKey") {
		if (event.GetValue().GetLong() == 0) {
			b.SetKey(WXK_NONE);
		}
		else
		{
			auto key = KeyBinding::GetPossibleKeys()[event.GetValue().GetLong() - 1];
			b.SetKey(key);
		}
	}
	else if (name == "KBControl") {
		b.SetControl(event.GetValue().GetBool());
	}
	else if (name == "KBAlt") {
		b.SetAlt(event.GetValue().GetBool());
	}
	else if (name == "KBShift") {
		b.SetShift(event.GetValue().GetBool());
	}
	else if (name == "KBEffect") {
		if (event.GetValue().GetLong() == 0) {
			b.SetEffectName("");
		}
		else {
			int i = 1;
			for (const auto& it : *_effectManager) {
				if (i == event.GetValue().GetLong()) {
					b.SetEffectName(it->Name());
					break;
				}
				i++;
			}
		}
	}
	else if (name == "KBEffectSetting") {
		b.SetEffectString(event.GetValue().GetString());
	}
	else if (name == "KBPreset")
	{
		if (event.GetValue().GetLong() == 0) {
			b.SetEffectName("");
		}
		else {
			int i = 1;
			for (const auto& it : _xLights->GetPresets()) {
				if (i == event.GetValue().GetLong()) {
					b.SetEffectName(it);
					break;
				}
				i++;
			}
		}
	}

	LoadList();
	SelectKey(id);
	SetKeyBindingProperties();
}

KeyBindingEditDialog::~KeyBindingEditDialog()
{
	//(*Destroy(KeyBindingEditDialog)
	//*)
}

KBSCOPE EncodeScope(std::string scope)
{
	if (scope == "Controller") return KBSCOPE::Setup;
	if (scope == "Layout") return KBSCOPE::Layout;
	if (scope == "Sequencer") return KBSCOPE::Sequence;
	return KBSCOPE::All;
}

void KeyBindingEditDialog::LoadList()
{
	ListCtrl_Bindings->Freeze();
	auto pos = ListCtrl_Bindings->GetScrollPos(wxVERTICAL);
	ListCtrl_Bindings->DeleteAllItems();
	for (const auto& it : _keyBindings->GetBindings())
	{
		if (it.InScope(EncodeScope(Choice_Scope->GetStringSelection())))
		{
			auto item = ListCtrl_Bindings->InsertItem(ListCtrl_Bindings->GetItemCount(), it.GetType());
			ListCtrl_Bindings->SetItem(item, 1, it.EncodeKey(it.GetKey(), it.RequiresShift()));
			ListCtrl_Bindings->SetItem(item, 2, it.RequiresControl() ? _("Y") : _(""));
			ListCtrl_Bindings->SetItem(item, 3, it.RequiresAlt() ? _("Y") : _(""));
			ListCtrl_Bindings->SetItem(item, 4, it.RequiresShift() ? _("Y") : _(""));
			if (it.GetEffectName() != "" && it.GetEffectString() != "")
			{
				ListCtrl_Bindings->SetItem(item, 5, it.GetEffectName() + ":" + it.GetEffectString());
			}
			else if (it.GetEffectString() != "")
			{
				ListCtrl_Bindings->SetItem(item, 5, it.GetEffectString());
			}
			else if (it.GetEffectName() != "")
			{
				ListCtrl_Bindings->SetItem(item, 5, it.GetEffectName());
			}
			ListCtrl_Bindings->SetItemData(item, it.GetId());
			if (it.GetKey() != WXK_NONE && _keyBindings->IsDuplicateKey(it))
			{
				ListCtrl_Bindings->SetItemTextColour(item, *wxRED);
			}
		}
	}
	ListCtrl_Bindings->Thaw();
	ListCtrl_Bindings->SetScrollPos(wxVERTICAL, pos);
	ListCtrl_Bindings->Refresh();
}

void KeyBindingEditDialog::OnButton_CancelClick(wxCommandEvent& event)
{
	EndDialog(wxID_CLOSE);
}

void KeyBindingEditDialog::OnChoice_ScopeSelect(wxCommandEvent& event)
{
	LoadList();
	SetKeyBindingProperties();
}

void KeyBindingEditDialog::OnListCtrl_BindingsItemFocused(wxListEvent& event)
{
	SetKeyBindingProperties();
}

void KeyBindingEditDialog::OnListCtrl_BindingsItemSelect(wxListEvent& event)
{
	SetKeyBindingProperties();
}

void KeyBindingEditDialog::OnListCtrl_BindingsKeyDown(wxListEvent& event)
{
	if (event.GetKeyCode() == WXK_DELETE)
	{
		int index = GetSelectedKeyBindingIndex();
		if (index >= 0) {
			int id = (int)ListCtrl_Bindings->GetItemData(index);
			if (id >= 0) {
				KeyBinding& b = _keyBindings->GetBinding(id);
				if (b.GetType() == "EFFECT" || b.GetType() == "PRESET" || b.GetType() == "APPLYSETTING")
				{
					_keyBindings->DeleteKey(id);
				}
				else
				{
					b.SetKey("");
					b.SetShift(false);
					b.SetAlt(false);
					b.SetControl(false);
				}
				LoadList();
			}
		}
	}
}

void KeyBindingEditDialog::OnListCtrl_BindingsDeleteItem(wxListEvent& event)
{
}

void KeyBindingEditDialog::SelectKey(int id)
{
	// unselect everything
	int item = ListCtrl_Bindings->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	while (item >= 0) {
		ListCtrl_Bindings->SetItemState(item, 0, wxLIST_STATE_SELECTED);
		item = ListCtrl_Bindings->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	}

	// remove the focus from all items
	item = ListCtrl_Bindings->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
	if (item >= 0) {
		ListCtrl_Bindings->SetItemState(item, 0, wxLIST_STATE_FOCUSED);
	}

	for (int i = 0; i < ListCtrl_Bindings->GetItemCount(); i++) {
		auto iid = ListCtrl_Bindings->GetItemData(i);

		if (iid == id) {
			ListCtrl_Bindings->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
			ListCtrl_Bindings->EnsureVisible(i);
			break;
		}
	}
}

void KeyBindingEditDialog::OnButton_AddEffectClick(wxCommandEvent& event)
{
	int id = _keyBindings->AddKey(KeyBinding(_(""), false, _("On"), _(""), _("2020.15"), false, false, false));
	LoadList();
	SelectKey(id);
	SetKeyBindingProperties();
}

void KeyBindingEditDialog::OnButtonAddApplySettingClick(wxCommandEvent& event)
{
	int id = _keyBindings->AddKey(KeyBinding(false, _(""), _(""), _("2020.15"), false, false, false));
	LoadList();
	SelectKey(id);
	SetKeyBindingProperties();
}

void KeyBindingEditDialog::OnButtonAddPresetClick(wxCommandEvent& event)
{
	std::string empty;
	int id = _keyBindings->AddKey(KeyBinding(false, _(""), _(""), false, false, false));
	LoadList();
	SelectKey(id);
	SetKeyBindingProperties();
}

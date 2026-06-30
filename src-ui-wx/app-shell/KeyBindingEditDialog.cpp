
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(KeyBindingEditDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)
#include <wx/headercol.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/display.h>
#include <wx/settings.h>
#include <wx/srchctrl.h>
#include <wx/tokenzr.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/font.h>
#include <map>
#include <vector>
#include <cctype>

#include "KeyBindingEditDialog.h"
#include "KeyBindings.h"
#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "xLightsMain.h"
#include "shared/utils/xlPropertyGrid.h"

//(*IdInit(KeyBindingEditDialog)
const long KeyBindingEditDialog::ID_STATICTEXT1 = wxNewId();
const long KeyBindingEditDialog::ID_CHOICE1 = wxNewId();
const long KeyBindingEditDialog::ID_LISTCTRL1 = wxNewId();
const long KeyBindingEditDialog::ID_PANEL1 = wxNewId();
const long KeyBindingEditDialog::ID_BUTTON1 = wxNewId();
const long KeyBindingEditDialog::ID_BUTTON3 = wxNewId();
const long KeyBindingEditDialog::ID_BUTTON2 = wxNewId();
const long KeyBindingEditDialog::ID_BUTTON_SAVE = wxNewId();
const long KeyBindingEditDialog::ID_BUTTON_CANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(KeyBindingEditDialog,wxDialog)
	//(*EventTable(KeyBindingEditDialog)
	//*)
END_EVENT_TABLE()

namespace {
// Modal editor for a single key binding. Edits native controls; ApplyTo() writes
// the result back to the live binding only when the user accepts (wxID_OK).
class KeyBindingPopupEditor : public wxDialog
{
public:
    KeyBindingPopupEditor(wxWindow* parent, const KeyBinding& b, EffectManager* em, xLightsFrame* xl)
        : wxDialog(parent, wxID_ANY, _("Edit Shortcut"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
          _type(b.GetType())
    {
        auto* grid = new wxFlexGridSizer(0, 2, 6, 10);
        grid->AddGrowableCol(1);

        grid->Add(new wxStaticText(this, wxID_ANY, _("Key:")), 0, wxALIGN_CENTER_VERTICAL);
        _key = new wxChoice(this, wxID_ANY);
        _key->Append(_("(none)"));
        int sel = 0;
        int k = b.GetKey();
        if (k >= 'A' && k <= 'Z') k += 32;
        for (const auto& it : KeyBinding::GetPossibleKeys()) {
            _key->Append(KeyBinding::EncodeKey(it, false));
            _keys.push_back(it);
            if (it == k) sel = (int)_keys.size();
        }
        _key->SetSelection(sel);
        grid->Add(_key, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, _("Modifiers:")), 0, wxALIGN_TOP | wxTOP, 4);
        auto* mods = new wxBoxSizer(wxVERTICAL);
#ifdef __WXOSX__
        _ctrl  = new wxCheckBox(this, wxID_ANY, L"Command ⌘");
        _alt   = new wxCheckBox(this, wxID_ANY, L"Option ⌥");
        _shift = new wxCheckBox(this, wxID_ANY, L"Shift ⇧");
        _rctrl = new wxCheckBox(this, wxID_ANY, L"Control ⌃");
#else
        _ctrl  = new wxCheckBox(this, wxID_ANY, _("Control"));
        _alt   = new wxCheckBox(this, wxID_ANY, _("Alt"));
        _shift = new wxCheckBox(this, wxID_ANY, _("Shift"));
        _rctrl = new wxCheckBox(this, wxID_ANY, _("macOS Ctrl"));
#endif
        _ctrl->SetValue(b.RequiresControl());
        _alt->SetValue(b.RequiresAlt());
        _shift->SetValue(b.RequiresShift());
        _rctrl->SetValue(b.RequiresRawControl());
        mods->Add(_ctrl);
        mods->Add(_alt);
        mods->Add(_shift);
        mods->Add(_rctrl);
        grid->Add(mods, 1, wxEXPAND);

        if (_type == "EFFECT") {
            grid->Add(new wxStaticText(this, wxID_ANY, _("Effect:")), 0, wxALIGN_CENTER_VERTICAL);
            _effect = new wxChoice(this, wxID_ANY);
            _effect->Append("");
            for (const auto& it : *em) {
                _effect->Append(it->Name());
                if (it->Name() == b.GetEffectName()) _effect->SetSelection(_effect->GetCount() - 1);
            }
            if (_effect->GetSelection() == wxNOT_FOUND) _effect->SetSelection(0);
            grid->Add(_effect, 1, wxEXPAND);
        }
        if (_type == "EFFECT" || _type == "APPLYSETTING") {
            grid->Add(new wxStaticText(this, wxID_ANY, _("Effect Setting:")), 0, wxALIGN_CENTER_VERTICAL);
            _setting = new wxTextCtrl(this, wxID_ANY, b.GetEffectString());
            grid->Add(_setting, 1, wxEXPAND);
        }
        if (_type == "PRESET") {
            grid->Add(new wxStaticText(this, wxID_ANY, _("Preset:")), 0, wxALIGN_CENTER_VERTICAL);
            _preset = new wxChoice(this, wxID_ANY);
            _preset->Append("");
            for (const auto& it : xl->GetPresets()) {
                _preset->Append(it);
                if (it == b.GetEffectName()) _preset->SetSelection(_preset->GetCount() - 1);
            }
            if (_preset->GetSelection() == wxNOT_FOUND) _preset->SetSelection(0);
            grid->Add(_preset, 1, wxEXPAND);
        }

        // Header: the friendly name (prominent) and its description. The raw
        // action/type isn't shown - it's not meaningful to most users.
        auto* nameText = new wxStaticText(this, wxID_ANY, KeyBindingEditDialog::FriendlyName(b.GetType()));
        wxFont nameFont = nameText->GetFont();
        nameFont.MakeBold();
        nameFont.SetPointSize(nameFont.GetPointSize() + 3);
        nameText->SetFont(nameFont);

        auto* descText = new wxStaticText(this, wxID_ANY, b.GetTip());
        descText->Wrap(440);

        auto* top = new wxBoxSizer(wxVERTICAL);
        top->Add(nameText, 0, wxLEFT | wxRIGHT | wxTOP, 14);
        top->Add(descText, 0, wxLEFT | wxRIGHT | wxTOP, 6);
        top->Add(new wxStaticLine(this, wxID_ANY), 0, wxEXPAND | wxALL, 12);
        top->Add(grid, 1, wxEXPAND | wxLEFT | wxRIGHT, 14);
        top->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 12);
        SetSizerAndFit(top);
        SetMinSize(GetSize());
        CenterOnParent();
    }

    void ApplyTo(KeyBinding& b) const
    {
        const int s = _key->GetSelection();
        if (s <= 0) {
            b.SetKey(WXK_NONE);
        } else {
            b.SetKey(_keys[s - 1]);
        }
        b.SetControl(_ctrl->GetValue());
        b.SetAlt(_alt->GetValue());
        b.SetShift(_shift->GetValue());
        b.SetRawControl(_rctrl->GetValue());
        if (_effect != nullptr) b.SetEffectName(_effect->GetStringSelection().ToStdString());
        if (_preset != nullptr) b.SetEffectName(_preset->GetStringSelection().ToStdString());
        if (_setting != nullptr) b.SetEffectString(_setting->GetValue().ToStdString());
    }

private:
    std::string _type;
    wxChoice* _key = nullptr;
    std::vector<wxKeyCode> _keys;
    wxCheckBox* _ctrl = nullptr;
    wxCheckBox* _alt = nullptr;
    wxCheckBox* _shift = nullptr;
    wxCheckBox* _rctrl = nullptr;
    wxChoice* _effect = nullptr;
    wxChoice* _preset = nullptr;
    wxTextCtrl* _setting = nullptr;
};
} // namespace

KeyBindingEditDialog::KeyBindingEditDialog(xLightsFrame* parent, KeyBindingMap* keyBindings, EffectManager* effectManager, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	_xLights = parent;
	_keyBindings = keyBindings;
	_effectManager = effectManager;

	//(*Initialize(KeyBindingEditDialog)
	wxFlexGridSizer* FlexGridSizer1;
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
	FlexGridSizer5 = new wxFlexGridSizer(0, 5, 0, 0);
	Button_AddEffect = new wxButton(this, ID_BUTTON1, _("Add Effect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer5->Add(Button_AddEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonAddPreset = new wxButton(this, ID_BUTTON3, _("Add Preset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer5->Add(ButtonAddPreset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonAddApplySetting = new wxButton(this, ID_BUTTON2, _("Add Apply Setting"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer5->Add(ButtonAddApplySetting, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonSave = new wxButton(this, ID_BUTTON_SAVE, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE"));
	FlexGridSizer5->Add(ButtonSave, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Close = new wxButton(this, ID_BUTTON_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CANCEL"));
	FlexGridSizer5->Add(Button_Close, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	Connect(ID_BUTTON_SAVE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&KeyBindingEditDialog::OnButtonSaveClick);
	Connect(ID_BUTTON_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&KeyBindingEditDialog::OnButton_CancelClick);
	//*)

	Panel_Properties->SetMinSize(wxSize(500, -1));
	Layout();

	Choice_Scope->AppendString("All");
	//Choice_Scope->AppendString("Controller");
	Choice_Scope->AppendString("Layout");
	Choice_Scope->AppendString("Sequencer");
	Choice_Scope->AppendString("All tabs");

	Choice_Scope->SetStringSelection("All");

    // Live filter for the bindings list. Added here (outside the wxSmith guard)
    // and stacked under the Scope row, so no .wxs change is needed.
    FlexGridSizer4->Add(new wxStaticText(this, wxID_ANY, _("Filter:")), 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    _filterCtrl = new wxSearchCtrl(this, wxID_ANY);
    _filterCtrl->ShowCancelButton(true);
    _filterCtrl->SetDescriptiveText(_("Filter actions, shortcuts or descriptions"));
    FlexGridSizer4->Add(_filterCtrl, 1, wxALL | wxEXPAND, 5);
    _filterCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { _filter = _filterCtrl->GetValue().Lower(); LoadList(); });
    _filterCtrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, [this](wxCommandEvent&) { _filterCtrl->ChangeValue(""); _filter.clear(); LoadList(); });
    Layout();

    ListCtrl_Bindings->AppendColumn("Action");
    ListCtrl_Bindings->AppendColumn("Shortcut", wxLIST_FORMAT_CENTRE, wxLIST_AUTOSIZE_USEHEADER);
    ListCtrl_Bindings->AppendColumn("Details");

    // wxListCtrl has no per-row tooltip, so track the hovered row and show
    // that binding's raw type + description as the control tooltip.
    ListCtrl_Bindings->Bind(wxEVT_MOTION, &KeyBindingEditDialog::OnListMouseMotion, this);
    // Modeless: clean up on close instead of EndModal.
    Bind(wxEVT_CLOSE_WINDOW, &KeyBindingEditDialog::OnClose, this);
    SetName(WINDOW_NAME); // so the menu handler can find/reuse this instance

    LoadList();

    ListCtrl_Bindings->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);
    ListCtrl_Bindings->SetColumnWidth(1, wxCOL_WIDTH_AUTOSIZE);
    ListCtrl_Bindings->SetColumnWidth(2, wxCOL_WIDTH_AUTOSIZE);

	_propertyGrid = new xlPropertyGrid(Panel_Properties, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		// Here are just some of the supported window styles
		//wxPG_AUTO_SORT | // Automatic sorting after items added
		wxPG_SPLITTER_AUTO_CENTER | // Automatically center splitter until user manually adjusts it
		// Default style
		wxPG_DEFAULT_STYLE);
	_propertyGrid->SetExtraStyle(wxWS_EX_PROCESS_IDLE | wxPG_EX_HELP_AS_TOOLTIPS);
	FlexGridSizer3->Add(_propertyGrid, 1, wxALL | wxEXPAND, 5);
	_propertyGrid->Connect(wxEVT_PG_CHANGED, (wxObjectEventFunction)&KeyBindingEditDialog::OnControllerPropertyGridChange, 0, this);
    _propertyGrid->SetValidationFailureBehavior(wxPGVFBFlags::MarkCell | wxPGVFBFlags::Beep);

	// Constrain dialog size to fit within the display's client area
	int targetWidth = 1200;
	int targetHeight = 700;
	int d = wxDisplay::GetFromWindow(this);
	if (d < 0) d = 0;
	wxDisplay display(d);
	if (display.IsOk()) {
		wxRect displayRect = display.GetClientArea();
		// Leave some margin (50 pixels) around the edges
		int maxWidth = displayRect.GetWidth() - 100;
		int maxHeight = displayRect.GetHeight() - 100;
		if (targetWidth > maxWidth) targetWidth = maxWidth;
		if (targetHeight > maxHeight) targetHeight = maxHeight;
	}
	SetSize(targetWidth, targetHeight);

	// Edit... button, plus double-click / Enter on a row (ITEM_ACTIVATED), open
	// the popup editor for the selected binding. The button is disabled while
	// nothing is selected.
	_editButton = new wxButton(this, wxID_ANY, _("Edit..."));
	FlexGridSizer5->Insert(0, _editButton, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
	_editButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { DoEditSelected(); });
	ListCtrl_Bindings->Bind(wxEVT_LIST_ITEM_ACTIVATED, [this](wxListEvent&) { DoEditSelected(); });
	ListCtrl_Bindings->Bind(wxEVT_LIST_ITEM_SELECTED, [this](wxListEvent& e) { e.Skip(); UpdateEditEnabled(); });
	ListCtrl_Bindings->Bind(wxEVT_LIST_ITEM_DESELECTED, [this](wxListEvent& e) { e.Skip(); UpdateEditEnabled(); });
	// Stretch the last (Details) column to fill the list width so rows/zebra
	// extend full width when the window is resized.
	ListCtrl_Bindings->Bind(wxEVT_SIZE, [this](wxSizeEvent& e) {
		e.Skip();
		if (ListCtrl_Bindings->GetColumnCount() < 3) return;
		const int total = ListCtrl_Bindings->GetClientSize().GetWidth();
		const int used = ListCtrl_Bindings->GetColumnWidth(0) + ListCtrl_Bindings->GetColumnWidth(1);
		if (total - used > 120) ListCtrl_Bindings->SetColumnWidth(2, total - used);
	});

	// Editing now happens in a popup, so drop the right-hand property panel and
	// rebuild the layout as a single column (scope/filter row, full-width list,
	// buttons). Hiding the panel alone left its grid column behind.
	Panel_Properties->Hide();
	FlexGridSizer1->Detach(FlexGridSizer4);
	FlexGridSizer1->Detach(ListCtrl_Bindings);
	FlexGridSizer1->Detach(FlexGridSizer5);
	auto* colSizer = new wxBoxSizer(wxVERTICAL);
	colSizer->Add(FlexGridSizer4, 0, wxEXPAND | wxALL, 2);
	colSizer->Add(ListCtrl_Bindings, 1, wxEXPAND | wxALL, 2);
	colSizer->Add(FlexGridSizer5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
	SetSizer(colSizer, true);
	SetMinSize(wxSize(550, 400));
	SetSize(wxSize(800, 700));
	Layout();
}

int KeyBindingEditDialog::GetSelectedKeyBindingIndex() const {

    return ListCtrl_Bindings->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}

void KeyBindingEditDialog::UpdateEditEnabled()
{
    if (_editButton != nullptr) _editButton->Enable(GetSelectedKeyBindingIndex() >= 0);
}

void KeyBindingEditDialog::RefreshRow(long index, const KeyBinding& b)
{
    ListCtrl_Bindings->SetItem(index, 1, RenderShortcut(b));
    ListCtrl_Bindings->SetItem(index, 2, BuildDetails(b));
    RefreshDuplicateHighlights();
}

void KeyBindingEditDialog::DoEditSelected()
{
    int index = GetSelectedKeyBindingIndex();
    if (index < 0) return;
    int id = (int)ListCtrl_Bindings->GetItemData(index);
    if (id < 0) return;

    KeyBinding& b = _keyBindings->GetBinding(id);
    KeyBindingPopupEditor editor(this, b, _effectManager, _xLights);
    if (editor.ShowModal() == wxID_OK) {
        editor.ApplyTo(b);
        RefreshRow(index, b);
    }
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
	p->ChangeFlag(wxPGFlags::ReadOnly, true);
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
	_propertyGrid->Append(new wxEnumProperty("Key", "KBKey", choices, val));

	p = _propertyGrid->Append(new wxBoolProperty("Control", "KBControl", b.RequiresControl()));
	p->SetEditor("CheckBox");

	p = _propertyGrid->Append(new wxBoolProperty("Alt", "KBAlt", b.RequiresAlt()));
	p->SetEditor("CheckBox");

	p = _propertyGrid->Append(new wxBoolProperty("Shift", "KBShift", b.RequiresShift()));
	p->SetEditor("CheckBox");

    p = _propertyGrid->Append(new wxBoolProperty("RawControl", "KBRawControl", b.RequiresRawControl()));
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

		_propertyGrid->Append(new wxEnumProperty("Effect", "KBEffect", effchoices, val));
	}

	if (b.GetType() == "EFFECT" || b.GetType() == "APPLYSETTING")
	{
		_propertyGrid->Append(new wxStringProperty("Effect Setting", "KBEffectSetting", b.GetEffectString()));
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
		_propertyGrid->Append(new wxEnumProperty("Preset", "KBPreset", presetchoices, val));
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
    else if (name == "KBRawControl") {
        b.SetRawControl(event.GetValue().GetBool());
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

	// Update the edited row in place. Don't rebuild the list/property grid: Type
	// is read-only so the property set never changes, and a rebuild re-creates
	// the enum editors and visibly flashes them. The grid already shows the edit.
	ListCtrl_Bindings->SetItem(index, 1, RenderShortcut(b));
	ListCtrl_Bindings->SetItem(index, 2, BuildDetails(b));
	RefreshDuplicateHighlights();
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
	const wxColour evenRow = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX);
	const wxColour txt = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT);
	const wxColour oddRow((evenRow.Red()*92 + txt.Red()*8) / 100,
	                      (evenRow.Green()*92 + txt.Green()*8) / 100,
	                      (evenRow.Blue()*92 + txt.Blue()*8) / 100);
	const wxString scopeSel = Choice_Scope->GetStringSelection();
	const bool showAll = (scopeSel == "All");
	const KBSCOPE scope = EncodeScope(scopeSel);
	for (const auto& it : _keyBindings->GetBindings())
	{
		if (!showAll && !it.InScope(scope))
			continue;

		const wxString friendly = FriendlyName(it.GetType());
		const wxString shortcut = RenderShortcut(it);
		const wxString details = BuildDetails(it);

		// Whitespace-tokenised AND filter over action / type / shortcut / details.
		if (!_filter.empty()) {
			const wxString hay = (friendly + " " + it.GetType() + " " + shortcut + " " + details).Lower();
			bool match = true;
			wxStringTokenizer tok(_filter, " ");
			while (tok.HasMoreTokens()) {
				if (hay.Find(tok.GetNextToken()) == wxNOT_FOUND) { match = false; break; }
			}
			if (!match) continue;
		}

		auto item = ListCtrl_Bindings->InsertItem(ListCtrl_Bindings->GetItemCount(), friendly);
		ListCtrl_Bindings->SetItem(item, 1, shortcut);
		ListCtrl_Bindings->SetItem(item, 2, details);
		ListCtrl_Bindings->SetItemData(item, it.GetId());
		// Zebra striping using theme-aware colours (works in light and dark).
		ListCtrl_Bindings->SetItemBackgroundColour(item, (item % 2 == 0) ? evenRow : oddRow);
		if (it.GetKey() != WXK_NONE && _keyBindings->IsDuplicateKey(it))
		{
			ListCtrl_Bindings->SetItemTextColour(item, *wxRED);
		}
	}
	if (ListCtrl_Bindings->GetItemCount() > 0 &&
	    ListCtrl_Bindings->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED) < 0) {
		ListCtrl_Bindings->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
	ListCtrl_Bindings->Thaw();
	ListCtrl_Bindings->SetScrollPos(wxVERTICAL, pos);
	ListCtrl_Bindings->Refresh();
	UpdateEditEnabled();
}

wxString KeyBindingEditDialog::BuildDetails(const KeyBinding& b) const
{
	wxString details = b.GetTip();
	wxString effect;
	if (b.GetEffectName() != "" && b.GetEffectString() != "") {
		effect = b.GetEffectName() + ":" + b.GetEffectString();
	} else if (b.GetEffectString() != "") {
		effect = b.GetEffectString();
	} else if (b.GetEffectName() != "") {
		effect = b.GetEffectName();
	}
	if (!effect.empty()) {
		details = details.empty() ? effect : details + " (" + effect + ")";
	}
	return details;
}

// Re-colour duplicate-key rows in place (changing a key can create or resolve a
// clash on another row) without rebuilding the list.
void KeyBindingEditDialog::RefreshDuplicateHighlights()
{
	const wxColour normal = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT);
	for (long i = 0; i < ListCtrl_Bindings->GetItemCount(); ++i) {
		const KeyBinding& rb = _keyBindings->GetBinding((int)ListCtrl_Bindings->GetItemData(i));
		const bool dup = rb.GetKey() != WXK_NONE && _keyBindings->IsDuplicateKey(rb);
		ListCtrl_Bindings->SetItemTextColour(i, dup ? *wxRED : normal);
	}
}

wxString KeyBindingEditDialog::FriendlyName(const std::string& type)
{
    // Most enum names humanise cleanly (split on '_', title-case); override the
    // handful that don't read well that way.
    static const std::map<std::string, wxString> overrides = {
        { "AUDIO_FULL_SPEED", "Audio: Full Speed" },
        { "AUDIO_F_1_5_SPEED", "Audio: 1.5x Speed" },
        { "AUDIO_F_2_SPEED", "Audio: 2x Speed" },
        { "AUDIO_F_3_SPEED", "Audio: 3x Speed" },
        { "AUDIO_F_4_SPEED", "Audio: 4x Speed" },
        { "AUDIO_S_3_4_SPEED", "Audio: 3/4 Speed" },
        { "AUDIO_S_1_2_SPEED", "Audio: 1/2 Speed" },
        { "AUDIO_S_1_4_SPEED", "Audio: 1/4 Speed" },
        { "VALUECURVES_TOGGLE", "Value Curves Panel" },
        { "EXPORT_MODEL_CAD", "Export Model (CAD)" },
        { "EXPORT_LAYOUT_DXF", "Export Layout (DXF)" },
        { "FPP_CONNECT", "FPP Connect" },
        { "FOCUS_SEQUENCER", "Focus Effects Grid" },
    };
    auto o = overrides.find(type);
    if (o != overrides.end()) return o->second;

    wxString out;
    bool newWord = true;
    for (char c : type) {
        if (c == '_') {
            out += ' ';
            newWord = true;
        } else if (newWord) {
            out += (char)std::toupper((unsigned char)c);
            newWord = false;
        } else {
            out += (char)std::tolower((unsigned char)c);
        }
    }
    return out;
}

wxString KeyBindingEditDialog::RenderShortcut(const KeyBinding& b)
{
    if (b.GetKey() == WXK_NONE) return "(unassigned)";
    wxString mods;
#ifdef __WXOSX__
    if (b.RequiresControl()) mods += wxUniChar(0x2318);    // Command
    if (b.RequiresRawControl()) mods += wxUniChar(0x2303); // Control
    if (b.RequiresAlt()) mods += wxUniChar(0x2325);        // Option
    if (b.RequiresShift()) mods += wxUniChar(0x21E7);      // Shift
#else
    if (b.RequiresControl()) mods += "Ctrl+";
    if (b.RequiresRawControl()) mods += "RCtrl+";
    if (b.RequiresAlt()) mods += "Alt+";
    if (b.RequiresShift()) mods += "Shift+";
#endif
    return mods + b.EncodeKey(b.GetKey(), false);
}

void KeyBindingEditDialog::OnListMouseMotion(wxMouseEvent& event)
{
    event.Skip();
    int flags = 0;
    long item = ListCtrl_Bindings->HitTest(event.GetPosition(), flags);
    if (item == _tooltipItem) return;
    _tooltipItem = item;
    if (item == wxNOT_FOUND) {
        ListCtrl_Bindings->UnsetToolTip();
        return;
    }
    long id = ListCtrl_Bindings->GetItemData(item);
    for (const auto& b : _keyBindings->GetBindings()) {
        if ((long)b.GetId() == id) {
            wxString tip = FriendlyName(b.GetType()) + "  [" + wxString(b.GetType()) + "]";
            if (!b.GetTip().empty()) tip += "\n" + wxString(b.GetTip());
            ListCtrl_Bindings->SetToolTip(tip);
            return;
        }
    }
    ListCtrl_Bindings->UnsetToolTip();
}

void KeyBindingEditDialog::OnButton_CancelClick(wxCommandEvent& event)
{
	Close();
}

void KeyBindingEditDialog::OnClose(wxCloseEvent& event)
{
	Destroy();
}

void KeyBindingEditDialog::OnChoice_ScopeSelect(wxCommandEvent& event)
{
	LoadList();
}

void KeyBindingEditDialog::OnListCtrl_BindingsItemFocused(wxListEvent& event)
{
}

void KeyBindingEditDialog::OnListCtrl_BindingsItemSelect(wxListEvent& event)
{
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
                    b.SetRawControl(false);
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

		if (iid == (wxUIntPtr)id) {
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
	DoEditSelected();
}

void KeyBindingEditDialog::OnButtonAddApplySettingClick(wxCommandEvent& event)
{
	int id = _keyBindings->AddKey(KeyBinding(false, _(""), _(""), _("2020.15"), false, false, false, false));
	LoadList();
	SelectKey(id);
	DoEditSelected();
}

void KeyBindingEditDialog::OnButtonAddPresetClick(wxCommandEvent& event)
{
	std::string empty;
	int id = _keyBindings->AddKey(KeyBinding(false, _(""), _(""), false, false, false, false));
	LoadList();
	SelectKey(id);
	DoEditSelected();
}

void KeyBindingEditDialog::OnButtonSaveClick(wxCommandEvent& event)
{
    _keyBindings->Save();
}


/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/intl.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/display.h>
#include <wx/listctrl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/srchctrl.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/tokenzr.h>
#include <wx/font.h>
#include <map>
#include <string>
#include <vector>
#include <cctype>

#include "KeyBindingsSettingsPanel.h"
#include "KeyBindings.h"
#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "xLightsMain.h"
#include "sequencer/MainSequencer.h"

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
        auto* nameText = new wxStaticText(this, wxID_ANY, KeyBindingsSettingsPanel::FriendlyName(b.GetType()));
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

KeyBindingsSettingsPanel::KeyBindingsSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    Create(parent, id, pos, size, wxTAB_TRAVERSAL, _T("KeyBindingsSettingsPanel"));
    _xLights = f;
    _keyBindings = &f->GetMainSequencer()->keyBindings;
    _effectManager = &f->GetEffectManager();

    auto* topSizer = new wxBoxSizer(wxVERTICAL);

    auto* topRow = new wxFlexGridSizer(0, 2, 0, 0);
    topRow->AddGrowableCol(1);
    topRow->Add(new wxStaticText(this, wxID_ANY, _("Category:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    Choice_Category = new wxChoice(this, wxID_ANY);
    Choice_Category->AppendString(_("All"));
    Choice_Category->AppendString(_("Effects / Wheel of Effects"));
    Choice_Category->AppendString(_("Presets"));
    Choice_Category->AppendString(_("Apply Settings"));
    Choice_Category->AppendString(_("Commands"));
    Choice_Category->SetStringSelection(_("All"));
    topRow->Add(Choice_Category, 1, wxALL | wxEXPAND, 5);
    topRow->Add(new wxStaticText(this, wxID_ANY, _("Scope:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    Choice_Scope = new wxChoice(this, wxID_ANY);
    Choice_Scope->AppendString("All");
    Choice_Scope->AppendString("Layout");
    Choice_Scope->AppendString("Sequencer");
    Choice_Scope->AppendString("All tabs");
    Choice_Scope->SetStringSelection("All");
    topRow->Add(Choice_Scope, 1, wxALL | wxEXPAND, 5);
    topRow->Add(new wxStaticText(this, wxID_ANY, _("Filter:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    _filterCtrl = new wxSearchCtrl(this, wxID_ANY);
    _filterCtrl->ShowCancelButton(true);
    _filterCtrl->SetDescriptiveText(_("Filter actions, shortcuts or descriptions"));
    topRow->Add(_filterCtrl, 1, wxALL | wxEXPAND, 5);
    topSizer->Add(topRow, 0, wxEXPAND);

    ListCtrl_Bindings = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 360), wxLC_REPORT | wxLC_SINGLE_SEL);
    ListCtrl_Bindings->AppendColumn("Action");
    ListCtrl_Bindings->AppendColumn("Shortcut", wxLIST_FORMAT_CENTRE);
    ListCtrl_Bindings->AppendColumn("Details");
    topSizer->Add(ListCtrl_Bindings, 1, wxEXPAND | wxALL, 4);

    auto* btnRow = new wxBoxSizer(wxHORIZONTAL);
    _editButton = new wxButton(this, wxID_ANY, _("Edit..."));
    auto* addEffect = new wxButton(this, wxID_ANY, _("Add Effect"));
    auto* addPreset = new wxButton(this, wxID_ANY, _("Add Preset"));
    auto* addApply = new wxButton(this, wxID_ANY, _("Add Apply Setting"));
    btnRow->Add(_editButton, 0, wxRIGHT, 6);
    btnRow->Add(addEffect, 0, wxRIGHT, 6);
    btnRow->Add(addPreset, 0, wxRIGHT, 6);
    btnRow->Add(addApply, 0);
    topSizer->Add(btnRow, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 6);

    auto* note = new wxStaticText(this, wxID_ANY,
        _("Effects assigned a key binding also appear on the Wheel of Effects in the "
          "sequencer — double-click an empty spot on the effect grid to open it."));
    note->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    note->Wrap(520);
    topSizer->Add(note, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

    SetSizer(topSizer);
    SetMinSize(wxSize(560, 420));

    LoadList();
    ListCtrl_Bindings->SetColumnWidth(0, wxLIST_AUTOSIZE);
    ListCtrl_Bindings->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
    ListCtrl_Bindings->SetColumnWidth(2, wxLIST_AUTOSIZE);

    Choice_Category->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) { LoadList(); });
    Choice_Scope->Bind(wxEVT_CHOICE, &KeyBindingsSettingsPanel::OnChoice_ScopeSelect, this);
    _filterCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { _filter = _filterCtrl->GetValue().Lower(); LoadList(); });
    _filterCtrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, [this](wxCommandEvent&) { _filterCtrl->ChangeValue(""); _filter.clear(); LoadList(); });
    ListCtrl_Bindings->Bind(wxEVT_LIST_KEY_DOWN, &KeyBindingsSettingsPanel::OnListCtrl_BindingsKeyDown, this);
    ListCtrl_Bindings->Bind(wxEVT_MOTION, &KeyBindingsSettingsPanel::OnListMouseMotion, this);
    ListCtrl_Bindings->Bind(wxEVT_LIST_ITEM_ACTIVATED, [this](wxListEvent&) { DoEditSelected(); });
    ListCtrl_Bindings->Bind(wxEVT_LIST_ITEM_SELECTED, [this](wxListEvent& e) { e.Skip(); UpdateEditEnabled(); });
    ListCtrl_Bindings->Bind(wxEVT_LIST_ITEM_DESELECTED, [this](wxListEvent& e) { e.Skip(); UpdateEditEnabled(); });
    ListCtrl_Bindings->Bind(wxEVT_SIZE, [this](wxSizeEvent& e) {
        e.Skip();
        if (ListCtrl_Bindings->GetColumnCount() < 3) return;
        const int total = ListCtrl_Bindings->GetClientSize().GetWidth();
        const int used = ListCtrl_Bindings->GetColumnWidth(0) + ListCtrl_Bindings->GetColumnWidth(1);
        if (total - used > 120) ListCtrl_Bindings->SetColumnWidth(2, total - used);
    });
    _editButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { DoEditSelected(); });
    addEffect->Bind(wxEVT_BUTTON, &KeyBindingsSettingsPanel::OnButton_AddEffectClick, this);
    addPreset->Bind(wxEVT_BUTTON, &KeyBindingsSettingsPanel::OnButtonAddPresetClick, this);
    addApply->Bind(wxEVT_BUTTON, &KeyBindingsSettingsPanel::OnButtonAddApplySettingClick, this);

    UpdateEditEnabled();
}

int KeyBindingsSettingsPanel::GetSelectedKeyBindingIndex() const {

    return ListCtrl_Bindings->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}

void KeyBindingsSettingsPanel::UpdateEditEnabled()
{
    if (_editButton != nullptr) _editButton->Enable(GetSelectedKeyBindingIndex() >= 0);
}

void KeyBindingsSettingsPanel::RefreshRow(long index, const KeyBinding& b)
{
    ListCtrl_Bindings->SetItem(index, 1, RenderShortcut(b));
    ListCtrl_Bindings->SetItem(index, 2, BuildDetails(b));
    RefreshDuplicateHighlights();
}

void KeyBindingsSettingsPanel::DoEditSelected()
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

KeyBindingsSettingsPanel::~KeyBindingsSettingsPanel()
{
	//(*Destroy(KeyBindingsSettingsPanel)
	//*)
}

wxString KeyBindingsSettingsPanel::CategoryOf(const std::string& type)
{
    if (type == "EFFECT") return "Effects / Wheel of Effects";
    if (type == "PRESET") return "Presets";
    if (type == "APPLYSETTING") return "Apply Settings";
    return "Commands";
}

KBSCOPE EncodeScope(std::string scope)
{
	if (scope == "Controller") return KBSCOPE::Setup;
	if (scope == "Layout") return KBSCOPE::Layout;
	if (scope == "Sequencer") return KBSCOPE::Sequence;
	return KBSCOPE::All;
}

void KeyBindingsSettingsPanel::LoadList()
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
	const wxString categorySel = Choice_Category->GetStringSelection();
	const bool allCategories = categorySel.empty() || categorySel == "All";
	for (const auto& it : _keyBindings->GetBindings())
	{
		if (!showAll && !it.InScope(scope))
			continue;
		if (!allCategories && CategoryOf(it.GetType()) != categorySel)
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

wxString KeyBindingsSettingsPanel::BuildDetails(const KeyBinding& b) const
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
void KeyBindingsSettingsPanel::RefreshDuplicateHighlights()
{
	const wxColour normal = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT);
	for (long i = 0; i < ListCtrl_Bindings->GetItemCount(); ++i) {
		const KeyBinding& rb = _keyBindings->GetBinding((int)ListCtrl_Bindings->GetItemData(i));
		const bool dup = rb.GetKey() != WXK_NONE && _keyBindings->IsDuplicateKey(rb);
		ListCtrl_Bindings->SetItemTextColour(i, dup ? *wxRED : normal);
	}
}

wxString KeyBindingsSettingsPanel::FriendlyName(const std::string& type)
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

wxString KeyBindingsSettingsPanel::RenderShortcut(const KeyBinding& b)
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

void KeyBindingsSettingsPanel::OnListMouseMotion(wxMouseEvent& event)
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

void KeyBindingsSettingsPanel::OnChoice_ScopeSelect(wxCommandEvent& event)
{
	LoadList();
}

void KeyBindingsSettingsPanel::OnListCtrl_BindingsKeyDown(wxListEvent& event)
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

void KeyBindingsSettingsPanel::SelectKey(int id)
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

void KeyBindingsSettingsPanel::OnButton_AddEffectClick(wxCommandEvent& event)
{
	int id = _keyBindings->AddKey(KeyBinding(_(""), false, _("On"), _(""), _("2020.15"), false, false, false));
	LoadList();
	SelectKey(id);
	DoEditSelected();
}

void KeyBindingsSettingsPanel::OnButtonAddApplySettingClick(wxCommandEvent& event)
{
	int id = _keyBindings->AddKey(KeyBinding(false, _(""), _(""), _("2020.15"), false, false, false, false));
	LoadList();
	SelectKey(id);
	DoEditSelected();
}

void KeyBindingsSettingsPanel::OnButtonAddPresetClick(wxCommandEvent& event)
{
	std::string empty;
	int id = _keyBindings->AddKey(KeyBinding(false, _(""), _(""), false, false, false, false));
	LoadList();
	SelectKey(id);
	DoEditSelected();
}


bool KeyBindingsSettingsPanel::TransferDataToWindow() {
    LoadList();
    return true;
}

bool KeyBindingsSettingsPanel::TransferDataFromWindow() {
    if (_keyBindings != nullptr) _keyBindings->Save();
    return true;
}
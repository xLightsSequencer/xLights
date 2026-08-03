
/***************************************************************
 * This source file comes from the xLights project
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
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <cctype>

#include "KeyBindingsSettingsPanel.h"
#include "app-shell/KeyBindings.h"
#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "xLightsMain.h"
#include "sequencer/MainSequencer.h"


KeyBindingsSettingsPanel::KeyBindingsSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    Create(parent, id, pos, size, wxTAB_TRAVERSAL, _T("KeyBindingsSettingsPanel"));
    _xLights = f;
    _liveKeyBindings = &f->GetMainSequencer()->keyBindings;
    _working = *_liveKeyBindings;   // edit a copy; commit only on Save
    _keyBindings = &_working;
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

    // wxDataViewListCtrl gives native drag-reorder with a drop line, and keys
    // live in the two right-most columns (modifiers + key). Grip glyph column
    // (only on effect rows) signals which rows can be dragged.
    _dvList = new wxDataViewListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 360), wxDV_ROW_LINES | wxDV_SINGLE);
    _colGrip = _dvList->AppendTextColumn(wxEmptyString, wxDATAVIEW_CELL_INERT, FromDIP(30), wxALIGN_CENTER);
    _colAction = _dvList->AppendTextColumn(_("Action"), wxDATAVIEW_CELL_INERT, FromDIP(170), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
    _colPos = _dvList->AppendTextColumn(_("Position"), wxDATAVIEW_CELL_INERT, FromDIP(84), wxALIGN_CENTER); // wheel clock position (effects)
    _colDetails = _dvList->AppendTextColumn(_("Details"), wxDATAVIEW_CELL_INERT, FromDIP(260), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
    _colMods = _dvList->AppendTextColumn(_("Modifiers"), wxDATAVIEW_CELL_INERT, FromDIP(84), wxALIGN_CENTER);
    _colKey = _dvList->AppendTextColumn(_("Key"), wxDATAVIEW_CELL_INERT, FromDIP(64), wxALIGN_CENTER);
    // Trailing spacer: wxDataViewCtrl stretches its LAST column to fill the
    // control, which made Key absurdly wide and squeezed the others. Giving it
    // an empty column to stretch instead lets every real column keep its width.
    _dvList->AppendTextColumn(wxEmptyString, wxDATAVIEW_CELL_INERT, FromDIP(1), wxALIGN_LEFT);
    _dvList->EnableDragSource(wxDF_UNICODETEXT);
    _dvList->EnableDropTarget(wxDF_UNICODETEXT);
    _dvList->Bind(wxEVT_SIZE, [this](wxSizeEvent& e) { e.Skip(); FitColumns(); });
    // List on the left, edit column on the right (like the original dialog).
    auto* mainRow = new wxBoxSizer(wxHORIZONTAL);
    mainRow->Add(_dvList, 1, wxEXPAND | wxALL, 4);

    auto* editBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Edit selected binding"));
    _editorTitle = new wxStaticText(this, wxID_ANY, _("Select a binding to edit it."));
    _editorTitle->Wrap(FromDIP(230));
    editBox->Add(_editorTitle, 0, wxALL, 4);

    auto* grid = new wxFlexGridSizer(0, 2, 5, 8);
    grid->AddGrowableCol(1);
    auto addLabel = [&](const wxString& t) { grid->Add(new wxStaticText(this, wxID_ANY, t), 0, wxALIGN_CENTER_VERTICAL); };

    addLabel(_("Key:"));
    _shortcutField = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, FromDIP(wxSize(96, -1)), wxTE_READONLY | wxTE_CENTRE);
    _shortcutField->SetHint(_("press a key"));
    grid->Add(_shortcutField, 0); // not wxEXPAND: a single keystroke needs no more
    addLabel(_("Control:"));
    _cbControl = new wxCheckBox(this, wxID_ANY, wxEmptyString);
    grid->Add(_cbControl, 0);
    addLabel(_("Alt:"));
    _cbAlt = new wxCheckBox(this, wxID_ANY, wxEmptyString);
    grid->Add(_cbAlt, 0);
    addLabel(_("Shift:"));
    _cbShift = new wxCheckBox(this, wxID_ANY, wxEmptyString);
    grid->Add(_cbShift, 0);
    addLabel(_("Raw Control:"));
    _cbRawControl = new wxCheckBox(this, wxID_ANY, wxEmptyString);
    grid->Add(_cbRawControl, 0);

    _valueLabel = new wxStaticText(this, wxID_ANY, _("Effect:"));
    grid->Add(_valueLabel, 0, wxALIGN_CENTER_VERTICAL);
    _effectChoice = new wxChoice(this, wxID_ANY);
    _effectChoice->SetMaxSize(FromDIP(wxSize(220, -1)));
    _effectChoice->Append("");
    for (const auto& it : *_effectManager) _effectChoice->Append(it->Name());
    _presetChoice = new wxChoice(this, wxID_ANY);
    _presetChoice->SetMaxSize(FromDIP(wxSize(220, -1)));
    _presetChoice->Append("");
    for (const auto& it : f->GetPresets()) _presetChoice->Append(it);
    _settingCtrl = new wxTextCtrl(this, wxID_ANY, "");
    auto* valueSizer = new wxBoxSizer(wxHORIZONTAL);
    valueSizer->Add(_effectChoice, 1, wxEXPAND);
    valueSizer->Add(_presetChoice, 1, wxEXPAND);
    valueSizer->Add(_settingCtrl, 1, wxEXPAND);
    grid->Add(valueSizer, 1, wxEXPAND);
    editBox->Add(grid, 0, wxEXPAND | wxALL, 4);

    _clearShortcutBtn = new wxButton(this, wxID_ANY, _("Clear Shortcut"));
    editBox->Add(_clearShortcutBtn, 0, wxLEFT | wxTOP | wxBOTTOM, 4);

    // Fixed-width right column. Its content (wrapped description text, and the
    // wheel appearing/disappearing) would otherwise change its width on every
    // selection, resizing the list and resetting the Details column.
    const int kRightW = FromDIP(260);
    _editorTitle->SetMinSize(wxSize(kRightW - FromDIP(20), -1));
    editBox->SetMinSize(wxSize(kRightW, -1));
    auto* rightCol = new wxBoxSizer(wxVERTICAL);
    rightCol->SetMinSize(wxSize(kRightW, -1));
    rightCol->Add(editBox, 0, wxEXPAND);
    _wheelPanel = new wxPanel(this, wxID_ANY);
    _wheelPanel->SetMinSize(wxSize(kRightW - FromDIP(16), FromDIP(230)));
    _wheelPanel->SetBackgroundStyle(wxBG_STYLE_PAINT);
    _wheelPanel->Bind(wxEVT_PAINT, &KeyBindingsSettingsPanel::OnPaintWheel, this);
    _wheelBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Wheel of Effects"));
    _wheelBox->Add(_wheelPanel, 1, wxEXPAND | wxALL, 2);
    rightCol->Add(_wheelBox, 1, wxEXPAND | wxTOP, 8);
    mainRow->Add(rightCol, 0, wxEXPAND | wxLEFT | wxRIGHT, 4);
    topSizer->Add(mainRow, 1, wxEXPAND);

    auto* btnRow = new wxBoxSizer(wxHORIZONTAL);
    _moveUpBtn = new wxButton(this, wxID_ANY, wxString(wxUniChar(0x2191)) + _(" Move Up"));
    _moveDownBtn = new wxButton(this, wxID_ANY, wxString(wxUniChar(0x2193)) + _(" Move Down"));
    auto* addEffect = new wxButton(this, wxID_ANY, _("Add Effect"));
    auto* addPreset = new wxButton(this, wxID_ANY, _("Add Preset"));
    auto* addApply = new wxButton(this, wxID_ANY, _("Add Apply Setting"));
    auto* removeBtn = new wxButton(this, wxID_ANY, _("Remove"));
    btnRow->Add(_moveUpBtn, 0, wxRIGHT, 6);
    btnRow->Add(_moveDownBtn, 0, wxRIGHT, 16);
    btnRow->Add(addEffect, 0, wxRIGHT, 6);
    btnRow->Add(addPreset, 0, wxRIGHT, 6);
    btnRow->Add(addApply, 0, wxRIGHT, 16);
    btnRow->Add(removeBtn, 0);
    topSizer->Add(btnRow, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 6);
    _moveUpBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { MoveSelectedEffect(-1); });
    _moveDownBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { MoveSelectedEffect(1); });
    removeBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { RemoveSelected(); });

    auto* note = new wxStaticText(this, wxID_ANY,
        _("Select a binding, click Key and press a key, and tick the modifiers. "
          "Effect bindings (grip shown) can be reordered with Move Up/Down or by "
          "dragging - the order carries to the Wheel of Effects."));
    note->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    note->Wrap(700);
    topSizer->Add(note, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

    SetSizer(topSizer);
    // Wide enough for the list columns plus the right-hand editor + wheel.
    SetMinSize(FromDIP(wxSize(1060, 620)));

    LoadList();

    Choice_Category->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) { LoadList(); PopulateEditor(); UpdateWheelVisibility(); });
    Choice_Scope->Bind(wxEVT_CHOICE, &KeyBindingsSettingsPanel::OnChoice_ScopeSelect, this);
    _filterCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { _filter = _filterCtrl->GetValue().Lower(); LoadList(); PopulateEditor(); });
    _filterCtrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, [this](wxCommandEvent&) { _filterCtrl->ChangeValue(""); _filter.clear(); LoadList(); PopulateEditor(); });
    _dvList->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, [this](wxDataViewEvent&) { PopulateEditor(); });
    _dvList->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, [this](wxDataViewEvent&) { if (_shortcutField->IsEnabled()) _shortcutField->SetFocus(); });
    _dvList->Bind(wxEVT_DATAVIEW_ITEM_BEGIN_DRAG, &KeyBindingsSettingsPanel::OnDvBeginDrag, this);
    _dvList->Bind(wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE, &KeyBindingsSettingsPanel::OnDvDropPossible, this);
    _dvList->Bind(wxEVT_DATAVIEW_ITEM_DROP, &KeyBindingsSettingsPanel::OnDvDrop, this);
    _shortcutField->Bind(wxEVT_KEY_DOWN, &KeyBindingsSettingsPanel::OnShortcutKey, this);
    _clearShortcutBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { ClearShortcut(); });
    auto onModifier = [this](wxCommandEvent&) {
        KeyBinding* b = SelectedBinding();
        if (b == nullptr) return;
        b->SetControl(_cbControl->GetValue());
        b->SetAlt(_cbAlt->GetValue());
        b->SetShift(_cbShift->GetValue());
        b->SetRawControl(_cbRawControl->GetValue());
        int index = GetSelectedKeyBindingIndex();
        if (index >= 0) RefreshRow(index, *b);
    };
    _cbControl->Bind(wxEVT_CHECKBOX, onModifier);
    _cbAlt->Bind(wxEVT_CHECKBOX, onModifier);
    _cbShift->Bind(wxEVT_CHECKBOX, onModifier);
    _cbRawControl->Bind(wxEVT_CHECKBOX, onModifier);
    _effectChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) { ApplyEffectChoice(); });
    _presetChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) { ApplyEffectChoice(); });
    _settingCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { ApplyEffectChoice(); });
    addEffect->Bind(wxEVT_BUTTON, &KeyBindingsSettingsPanel::OnButton_AddEffectClick, this);
    addPreset->Bind(wxEVT_BUTTON, &KeyBindingsSettingsPanel::OnButtonAddPresetClick, this);
    addApply->Bind(wxEVT_BUTTON, &KeyBindingsSettingsPanel::OnButtonAddApplySettingClick, this);

    PopulateEditor();
    UpdateWheelVisibility();
    // Fit again once the dialog has actually been laid out - at this point the
    // list still reports its pre-layout width.
    CallAfter([this]() { FitColumns(); });
}

KeyBinding* KeyBindingsSettingsPanel::SelectedBinding() const
{
    const long id = RowBindingId(GetSelectedKeyBindingIndex());
    if (id < 0) return nullptr;
    return &_keyBindings->GetBinding((int)id);
}

long KeyBindingsSettingsPanel::RowBindingId(int row) const
{
    if (row < 0 || row >= (int)_rowIds.size()) return -1;
    return _rowIds[row];
}

void KeyBindingsSettingsPanel::SyncModifierChecks(const KeyBinding& b)
{
    _cbControl->SetValue(b.RequiresControl());
    _cbAlt->SetValue(b.RequiresAlt());
    _cbShift->SetValue(b.RequiresShift());
    _cbRawControl->SetValue(b.RequiresRawControl());
}

void KeyBindingsSettingsPanel::PopulateEditor()
{
    if (_wheelPanel != nullptr) _wheelPanel->Refresh(); // highlight the selection
    KeyBinding* b = SelectedBinding();
    const bool has = (b != nullptr);
    _shortcutField->Enable(has);
    _clearShortcutBtn->Enable(has);
    _cbControl->Enable(has);
    _cbAlt->Enable(has);
    _cbShift->Enable(has);
    _cbRawControl->Enable(has);
    const bool selEffect = has && b->GetType() == "EFFECT";
    if (_moveUpBtn != nullptr) _moveUpBtn->Enable(selEffect);
    if (_moveDownBtn != nullptr) _moveDownBtn->Enable(selEffect);
    if (!has) {
        _editorTitle->SetLabel(_("Select a binding to edit it."));
        _shortcutField->ChangeValue("");
        _cbControl->SetValue(false);
        _cbAlt->SetValue(false);
        _cbShift->SetValue(false);
        _cbRawControl->SetValue(false);
        _effectChoice->Hide();
        _presetChoice->Hide();
        _settingCtrl->Hide();
        _valueLabel->Hide();
        if (_lastEditorShape != 0) {
            _lastEditorShape = 0;
            Layout();
        }
        return;
    }

    const std::string type = b->GetType();
    _editorTitle->SetLabel(FriendlyName(type) + "\n" + wxString::FromUTF8(b->GetTip()));
    _editorTitle->Wrap(_editorTitle->GetMinSize().GetWidth());
    _shortcutField->ChangeValue(RenderKey(*b));
    SyncModifierChecks(*b);

    const bool isEffect = (type == "EFFECT");
    const bool isPreset = (type == "PRESET");
    const bool isSetting = (type == "EFFECT" || type == "APPLYSETTING");
    _effectChoice->Show(isEffect);
    _presetChoice->Show(isPreset);
    _settingCtrl->Show(isSetting);
    _valueLabel->Show(isEffect || isPreset || isSetting);
    if (isEffect) {
        _valueLabel->SetLabel(_("Effect:"));
        if (_effectChoice->SetStringSelection(b->GetEffectName()) == false) _effectChoice->SetSelection(0);
    } else if (isPreset) {
        _valueLabel->SetLabel(_("Preset:"));
        if (_presetChoice->SetStringSelection(b->GetEffectName()) == false) _presetChoice->SetSelection(0);
    }
    if (isSetting) {
        if (!isEffect) _valueLabel->SetLabel(_("Setting:"));
        _settingCtrl->ChangeValue(b->GetEffectString());
    }
    // Only relayout when the visible control set changed - laying out on every
    // row click resized the list and reset the Details column width.
    const int shape = 1 | (isEffect ? 2 : 0) | (isPreset ? 4 : 0) | (isSetting ? 8 : 0);
    if (shape != _lastEditorShape) {
        _lastEditorShape = shape;
        Layout();
    }
}

void KeyBindingsSettingsPanel::OnShortcutKey(wxKeyEvent& event)
{
    KeyBinding* b = SelectedBinding();
    if (b == nullptr) { event.Skip(); return; }
    const int kc = event.GetKeyCode();
    // Ignore pure modifier presses - wait for the actual key.
    if (kc == WXK_SHIFT || kc == WXK_CONTROL || kc == WXK_ALT || kc == WXK_RAW_CONTROL || kc == WXK_NONE) {
        return;
    }
    if (kc == WXK_ESCAPE) { return; } // leave unchanged
    b->SetKey((wxKeyCode)kc);
    // Reflect any modifiers held during the press into the checkboxes too, so
    // pressing the whole combo works as well as ticking modifiers by hand.
    b->SetShift(event.ShiftDown());
    b->SetAlt(event.AltDown());
    b->SetControl(event.CmdDown());        // Cmd on macOS, Ctrl elsewhere
    b->SetRawControl(event.RawControlDown()); // physical Ctrl on macOS
    _shortcutField->ChangeValue(RenderKey(*b));
    SyncModifierChecks(*b);
    // Assigning a key enables the binding, which can put it on the wheel and
    // renumber every other effect's slot - rebuild the list, not just one row.
    const int keptId = (int)b->GetId();
    LoadList();
    SelectKey(keptId);
}

void KeyBindingsSettingsPanel::ClearShortcut()
{
    KeyBinding* b = SelectedBinding();
    if (b == nullptr) return;
    b->SetKey(WXK_NONE);
    b->SetShift(false);
    b->SetAlt(false);
    b->SetControl(false);
    b->SetRawControl(false);
    _shortcutField->ChangeValue(RenderKey(*b));
    SyncModifierChecks(*b);
    // Clearing the key disables the binding, dropping it off the wheel and
    // renumbering the rest - same full rebuild as assigning one.
    const int keptId = (int)b->GetId();
    LoadList();
    SelectKey(keptId);
}

void KeyBindingsSettingsPanel::ApplyEffectChoice()
{
    KeyBinding* b = SelectedBinding();
    if (b == nullptr) return;
    const std::string type = b->GetType();
    if (type == "EFFECT") {
        b->SetEffectName(_effectChoice->GetStringSelection().ToStdString());
    } else if (type == "PRESET") {
        b->SetEffectName(_presetChoice->GetStringSelection().ToStdString());
    }
    if (type == "EFFECT" || type == "APPLYSETTING") {
        b->SetEffectString(_settingCtrl->GetValue().ToStdString());
    }
    int index = GetSelectedKeyBindingIndex();
    if (index >= 0) RefreshRow(index, *b);
}

void KeyBindingsSettingsPanel::OnDvBeginDrag(wxDataViewEvent& event)
{
    // Only effect bindings are reorderable.
    const long id = RowBindingId(_dvList->ItemToRow(event.GetItem()));
    if (id < 0 || _keyBindings->GetBinding((int)id).GetType() != "EFFECT") {
        event.Veto();
        _dragFromId = -1;
        return;
    }
    _dragFromId = id;
    event.SetDataObject(new wxTextDataObject(wxString::Format("%ld", id)));
    event.SetDragFlags(wxDrag_DefaultMove);
}

void KeyBindingsSettingsPanel::MoveSelectedEffect(int delta)
{
    KeyBinding* sel = SelectedBinding();
    if (sel == nullptr || sel->GetType() != "EFFECT") return;
    const long id = (long)sel->GetId();
    auto& v = _keyBindings->GetBindings();
    int selIdx = -1;
    for (int i = 0; i < (int)v.size(); ++i) if ((long)v[i].GetId() == id) { selIdx = i; break; }
    if (selIdx < 0) return;
    // Swap with the nearest other EFFECT binding in the requested direction,
    // so the effects reorder among themselves regardless of the filter.
    int j = selIdx + delta;
    while (j >= 0 && j < (int)v.size() && v[j].GetType() != "EFFECT") j += delta;
    if (j < 0 || j >= (int)v.size()) return;
    std::swap(v[selIdx], v[j]);
    LoadList();
    SelectKey((int)id);
    PopulateEditor();
}

void KeyBindingsSettingsPanel::OnDvDropPossible(wxDataViewEvent& event)
{
    // wxDataViewCtrl draws the drop line only where a drop is allowed - allow
    // it only onto another effect row.
    const long id = RowBindingId(_dvList->ItemToRow(event.GetItem()));
    if (id < 0 || _keyBindings->GetBinding((int)id).GetType() != "EFFECT") {
        event.Veto();
    }
}

void KeyBindingsSettingsPanel::OnDvDrop(wxDataViewEvent& event)
{
    const long fromId = _dragFromId;
    _dragFromId = -1;
    if (fromId < 0) return;
    const long toId = RowBindingId(_dvList->ItemToRow(event.GetItem()));
    if (toId < 0 || toId == fromId) return;
    if (_keyBindings->GetBinding((int)toId).GetType() != "EFFECT") return; // effects only

    // Move the dragged binding to just before the drop target in the backing
    // vector, then rebuild the list. Persisted order = vector order.
    auto& v = _keyBindings->GetBindings();
    auto findById = [&](long id) -> int {
        for (int i = 0; i < (int)v.size(); ++i) if ((long)v[i].GetId() == id) return i;
        return -1;
    };
    int from = findById(fromId);
    int to = findById(toId);
    if (from < 0 || to < 0 || from == to) return;
    KeyBinding moved = v[from];
    v.erase(v.begin() + from);
    if (from < to) --to;
    v.insert(v.begin() + to, moved);
    LoadList();
    SelectKey((int)fromId);
    PopulateEditor();
}

void KeyBindingsSettingsPanel::RemoveSelected()
{
    KeyBinding* b = SelectedBinding();
    if (b == nullptr) return;
    // Only the user-added effect/preset/apply bindings can be removed; the
    // built-in command bindings are fixed (matches the old Delete-key rule).
    const std::string type = b->GetType();
    if (type != "EFFECT" && type != "PRESET" && type != "APPLYSETTING") { wxBell(); return; }
    const long id = (long)b->GetId();
    auto& v = _keyBindings->GetBindings();
    for (int i = 0; i < (int)v.size(); ++i) {
        if ((long)v[i].GetId() == id) { v.erase(v.begin() + i); break; }
    }
    LoadList();
    PopulateEditor();
}

int KeyBindingsSettingsPanel::GetSelectedKeyBindingIndex() const {
    return _dvList->GetSelectedRow(); // wxNOT_FOUND (-1) when nothing is selected
}

void KeyBindingsSettingsPanel::RefreshRow(int row, const KeyBinding& b)
{
    if (row < 0) return;
    _dvList->SetValue(wxVariant(BuildDetails(b)), row, 3);
    _dvList->SetValue(wxVariant(RenderModifiers(b)), row, 4);
    _dvList->SetValue(wxVariant(RenderKey(b)), row, 5);
    RefreshDuplicateHighlights(); // re-applies the conflict marker across rows
}

KeyBindingsSettingsPanel::~KeyBindingsSettingsPanel()
{
	//(*Destroy(KeyBindingsSettingsPanel)
	//*)
}

// Category dropdown index (0 = All): 1 Effects, 2 Presets, 3 Apply Settings,
// 4 Commands. Compared by index so the filter is locale-independent (the visible
// labels are translated via _()).
int KeyBindingsSettingsPanel::CategoryIndexOf(const std::string& type)
{
    if (type == "EFFECT") return 1;
    if (type == "PRESET") return 2;
    if (type == "APPLYSETTING") return 3;
    return 4;
}

static KBSCOPE EncodeScope(const wxString& scope)
{
	if (scope == "Controller") return KBSCOPE::Setup;
	if (scope == "Layout") return KBSCOPE::Layout;
	if (scope == "Sequencer") return KBSCOPE::Sequence;
	return KBSCOPE::All;
}

void KeyBindingsSettingsPanel::LoadList()
{
	_dvList->Freeze();
	_dvList->DeleteAllItems();
	_rowIds.clear();
	const wxString scopeSel = Choice_Scope->GetStringSelection();
	const bool showAll = (scopeSel == "All");
	const KBSCOPE scope = EncodeScope(scopeSel);
	const int categorySel = Choice_Category->GetSelection(); // 0 = All

	// Collect the visible rows first so the Action column can be shown
	// alphabetically regardless of the bindings' storage order.
	struct Row {
		wxString friendly;
		wxString shortcut;
		wxString details;
		long id;
		const KeyBinding* binding;
	};
	std::vector<Row> rows;
	for (const auto& it : _keyBindings->GetBindings())
	{
		if (!showAll && !it.InScope(scope))
			continue;
		if (categorySel > 0 && CategoryIndexOf(it.GetType()) != categorySel)
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

		rows.push_back({ friendly, shortcut, details, (long)it.GetId(), &it });
	}

	// No alphabetical sort: rows follow the backing vector order so drag-to-
	// reorder is meaningful and persists.
	const wxString grip = wxUniChar(0x2261);   // triple-bar drag handle glyph
	const wxString warn = wxString(wxUniChar(0x26A0)) + " "; // conflict marker

	// Wheel-of-Effects clock position. Only the bindings the wheel actually
	// takes get a position; effects past the cut-off are flagged as off-wheel.
	// Exact clock time, not the nearest hour: with 18 slots several adjacent
	// slots round to the same hour, so reordering looked like it did nothing.
	auto clockLabel = [](int i, int n) -> wxString {
		if (n <= 0) return wxEmptyString;
		const int mins = (int)std::lround(i * 720.0 / n); // 720 min = 12 hours
		int hour = (mins / 60) % 12;
		if (hour == 0) hour = 12;
		return wxString::Format("%d:%02d", hour, mins % 60);
	};
	std::map<long, wxString> effectPos;
	{
		const auto wheel = WheelBindings();
		for (int i = 0; i < (int)wheel.size(); ++i) {
			effectPos[(long)wheel[i]->GetId()] = clockLabel(i, (int)wheel.size());
		}
	}

	for (const auto& r : rows)
	{
		// Grip only on effect rows - they're the only reorderable ones.
		const bool reorderable = (r.binding->GetType() == "EFFECT");
		const bool dup = r.binding->GetKey() != WXK_NONE && _keyBindings->IsDuplicateKey(*r.binding);
		wxVector<wxVariant> cols;
		cols.push_back(wxVariant(reorderable ? grip : wxString()));
		cols.push_back(wxVariant(r.friendly));
		// Distinguish the two reasons an effect has no wheel slot: it has no
		// key yet (a keyless binding is disabled, so the wheel skips it), or
		// it sits past the wheel's slot limit.
		wxString posText;
		if (reorderable) {
			if (effectPos.count(r.id)) posText = effectPos[r.id];
			else if (r.binding->GetKey() == WXK_NONE) posText = _("needs a key");
			else posText = _("not on wheel");
		}
		cols.push_back(wxVariant(posText));
		cols.push_back(wxVariant(r.details));
		cols.push_back(wxVariant(RenderModifiers(*r.binding)));
		cols.push_back(wxVariant((dup ? warn : wxString()) + RenderKey(*r.binding)));
		cols.push_back(wxVariant(wxString())); // trailing spacer column
		_dvList->AppendItem(cols);
		_rowIds.push_back(r.id);
	}
	if (rows.empty()) {
		// Explain the blank table rather than leaving the user guessing - some
		// Category/Scope pairs genuinely have no bindings (e.g. effects are all
		// Sequencer-scoped, so Effects + Layout is always empty).
		wxVector<wxVariant> cols;
		cols.push_back(wxVariant(wxString()));
		cols.push_back(wxVariant(_("(none)")));
		cols.push_back(wxVariant(wxString()));
		cols.push_back(wxVariant(_("No bindings match this Category / Scope / Filter combination.")));
		cols.push_back(wxVariant(wxString()));
		cols.push_back(wxVariant(wxString()));
		cols.push_back(wxVariant(wxString()));
		_dvList->AppendItem(cols);
		_rowIds.push_back(-1); // sentinel: not a real binding, never selectable
	}
	if (!_rowIds.empty() && _rowIds[0] >= 0 && _dvList->GetSelectedRow() == wxNOT_FOUND) {
		_dvList->SelectRow(0);
	}
	_dvList->Thaw();
	if (_wheelPanel != nullptr) _wheelPanel->Refresh(); // effect order may have changed
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

// Re-apply the conflict marker (a changed key can create or resolve a clash on
// another row) by rewriting each row's Key cell in place.
void KeyBindingsSettingsPanel::RefreshDuplicateHighlights()
{
	const wxString warn = wxString(wxUniChar(0x26A0)) + " ";
	for (int i = 0; i < (int)_rowIds.size(); ++i) {
		const KeyBinding& rb = _keyBindings->GetBinding((int)_rowIds[i]);
		const bool dup = rb.GetKey() != WXK_NONE && _keyBindings->IsDuplicateKey(rb);
		_dvList->SetValue(wxVariant((dup ? warn : wxString()) + RenderKey(rb)), i, 5);
	}
}

// Give Details the space left over after the fixed columns. SetWidth can fire
// another size event, so guard against re-entering.
void KeyBindingsSettingsPanel::FitColumns()
{
	if (_dvList == nullptr || _fitting || _colDetails == nullptr) return;
	const int client = _dvList->GetClientSize().GetWidth();
	if (client < FromDIP(200)) return; // not laid out yet - a stale width here
	                                   // made Details too wide and pushed the
	                                   // Modifiers/Key columns off-screen
	_fitting = true;
	const int wMods = FromDIP(84), wKey = FromDIP(64);
	const int wGrip = _colGrip->IsHidden() ? 0 : FromDIP(30);
	const int wPos = _colPos->IsHidden() ? 0 : FromDIP(84);
	const int wAction = _colAction->IsHidden() ? 0 : FromDIP(170);
	// Set every width explicitly so nothing can end up collapsed.
	if (wGrip) _colGrip->SetWidth(wGrip);
	if (wAction) _colAction->SetWidth(wAction);
	if (wPos) _colPos->SetWidth(wPos);
	_colMods->SetWidth(wMods);
	_colKey->SetWidth(wKey);
	// Reserve room for the vertical scrollbar and the spacer column, otherwise
	// the row overflows and the right-hand columns scroll out of view.
	const int reserve = FromDIP(30);
	const int avail = client - (wGrip + wAction + wPos + wMods + wKey) - reserve;
	// Cap Details so Modifiers/Key are always on screen at the default size -
	// long descriptions ellipsize rather than pushing the key columns off the
	// right edge. The spacer column absorbs anything left over.
	_colDetails->SetWidth(std::min(std::max(avail, FromDIP(140)), FromDIP(300)));
	_fitting = false;
}

void KeyBindingsSettingsPanel::RebuildScopeChoices()
{
	if (Choice_Scope == nullptr) return;
	const int cat = Choice_Category->GetSelection();

	// Which scopes actually occur among the bindings in this category?
	bool hasLayout = false, hasSeq = false, hasAllTabs = false;
	for (const auto& b : _keyBindings->GetBindings()) {
		if (cat > 0 && CategoryIndexOf(b.GetType()) != cat) continue;
		switch (b.GetScope()) {
		case KBSCOPE::Layout:   hasLayout = true; break;
		case KBSCOPE::Sequence: hasSeq = true; break;
		case KBSCOPE::All:      hasAllTabs = true; break;
		default: break;
		}
	}

	// If every binding in this category shares one scope, the per-scope entry
	// would just duplicate "All" - offer only "All".
	const int distinct = (hasLayout ? 1 : 0) + (hasSeq ? 1 : 0) + (hasAllTabs ? 1 : 0);
	const wxString prev = Choice_Scope->GetStringSelection();
	Choice_Scope->Freeze();
	Choice_Scope->Clear();
	Choice_Scope->AppendString("All");
	if (distinct > 1) {
		if (hasLayout) Choice_Scope->AppendString("Layout");
		if (hasSeq) Choice_Scope->AppendString("Sequencer");
		if (hasAllTabs) Choice_Scope->AppendString("All tabs");
	}
	if (Choice_Scope->SetStringSelection(prev) == false) {
		Choice_Scope->SetStringSelection("All"); // previous scope no longer offered
		Choice_Scope->Thaw();
		LoadList();
		return;
	}
	Choice_Scope->Thaw();
}

// The wheel and the clock Position column only describe effect bindings, so
// show them just for that category.
void KeyBindingsSettingsPanel::UpdateWheelVisibility()
{
	const int cat = Choice_Category->GetSelection();
	const bool show = (cat == 1); // Effects / Wheel of Effects

	RebuildScopeChoices();

	if (_colGrip != nullptr && _colPos != nullptr && _colAction != nullptr) {
		_colGrip->SetHidden(!show); // drag grip - effects only
		_colPos->SetHidden(!show);  // clock Position - effects only
		// In the Effects view every Action cell just reads "Effect" - the
		// effect's identity is in Details/Position, so drop the dead column.
		_colAction->SetHidden(show);
		FitColumns();
	}
	if (_wheelBox == nullptr) return;
	if (_wheelBox->IsShown(size_t(0)) != show) {
		_wheelBox->ShowItems(show);
		Layout();
	}
}

std::vector<const KeyBinding*> KeyBindingsSettingsPanel::WheelBindings() const
{
	// Mirrors EffectsGrid.cpp's selection exactly: enabled, Sequencer-scoped
	// EFFECT bindings, first kWheelSlots only.
	std::vector<const KeyBinding*> out;
	for (const auto& kb : _keyBindings->GetBindings()) {
		if (!kb.IsDisabled() && kb.GetType() == "EFFECT" && kb.InScope(KBSCOPE::Sequence)) {
			out.push_back(&kb);
			if ((int)out.size() >= kWheelSlots) break;
		}
	}
	return out;
}

void KeyBindingsSettingsPanel::OnPaintWheel(wxPaintEvent&)
{
	constexpr double kPi = 3.14159265358979323846;
	wxAutoBufferedPaintDC dc(_wheelPanel);
	dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));
	dc.Clear();
	std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
	if (gc == nullptr) return;

	// Exactly what the sequencer's wheel would show (first kWheelSlots enabled,
	// Sequencer-scoped effects), sector 0 at 12 o'clock.
	struct E { long id; wxString name; };
	std::vector<E> effs;
	for (const auto* b : WheelBindings()) {
		wxString nm = b->GetEffectName().empty() ? _("(unset)") : wxString::FromUTF8(b->GetEffectName());
		effs.push_back({ (long)b->GetId(), nm });
	}

	const wxSize sz = _wheelPanel->GetClientSize();
	const double cx = sz.x / 2.0, cy = sz.y / 2.0;
	// Leave room for the outside labels, but keep a usable circle even when the
	// panel is short (a too-eager margin used to shrink it away to nothing).
	const double margin = std::min((double)FromDIP(44), std::min(cx, cy) * 0.45);
	const double R = std::min(cx, cy) - margin;
	const wxColour fg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
	const wxColour hi(0, 120, 215);
	if (R < 8) return;

	gc->SetPen(wxPen(wxColour(fg.Red(), fg.Green(), fg.Blue(), 90), 1));
	gc->SetBrush(*wxTRANSPARENT_BRUSH);
	gc->DrawEllipse(cx - R, cy - R, 2 * R, 2 * R);

	if (effs.empty()) {
		gc->SetFont(*wxNORMAL_FONT, wxColour(fg.Red(), fg.Green(), fg.Blue(), 150));
		double tw, th;
		const wxString msg = _("No effects assigned");
		gc->GetTextExtent(msg, &tw, &th);
		gc->DrawText(msg, cx - tw / 2, cy - th / 2);
		return;
	}

	const long selId = SelectedBinding() != nullptr ? (long)SelectedBinding()->GetId() : -1;
	const int N = (int)effs.size();
	gc->SetFont(*wxSMALL_FONT, fg);
	for (int i = 0; i < N; ++i) {
		const double ang = -kPi / 2.0 + i * 2.0 * kPi / N; // 12 o'clock, clockwise
		const double x = cx + R * std::cos(ang);
		const double y = cy + R * std::sin(ang);
		const bool sel = effs[i].id == selId;
		const double dot = sel ? FromDIP(7.0) : FromDIP(4.0);
		gc->SetPen(*wxTRANSPARENT_PEN);
		gc->SetBrush(wxBrush(sel ? hi : fg));
		gc->DrawEllipse(x - dot, y - dot, 2 * dot, 2 * dot);

		gc->SetFont(*wxSMALL_FONT, sel ? hi : fg);
		double tw, th;
		gc->GetTextExtent(effs[i].name, &tw, &th);
		double lx = cx + (R + FromDIP(8)) * std::cos(ang);
		double ly = cy + (R + FromDIP(8)) * std::sin(ang);
		const double c = std::cos(ang);
		if (c < -0.3) lx -= tw;            // left side: right-align
		else if (c <= 0.3) lx -= tw / 2.0; // top/bottom: centre
		gc->DrawText(effs[i].name, lx, ly - th / 2.0);
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
        { "COPY_MODEL_LAYERS_TO_MODELS", "Copy Layers/SubModels to Models" },
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

wxString KeyBindingsSettingsPanel::RenderModifiers(const KeyBinding& b)
{
    if (b.GetKey() == WXK_NONE) return wxEmptyString;
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
    return mods;
}

wxString KeyBindingsSettingsPanel::RenderKey(const KeyBinding& b)
{
    if (b.GetKey() == WXK_NONE) return _("(unassigned)");
    return b.EncodeKey(b.GetKey(), false);
}

wxString KeyBindingsSettingsPanel::RenderShortcut(const KeyBinding& b)
{
    if (b.GetKey() == WXK_NONE) return _("(unassigned)");
    return RenderModifiers(b) + RenderKey(b);
}

void KeyBindingsSettingsPanel::OnChoice_ScopeSelect(wxCommandEvent& event)
{
	LoadList();
}

void KeyBindingsSettingsPanel::SelectKey(int id)
{
	for (int i = 0; i < (int)_rowIds.size(); ++i) {
		if (_rowIds[i] == (long)id) {
			_dvList->SelectRow((unsigned)i);
			_dvList->EnsureVisible(_dvList->RowToItem(i));
			return;
		}
	}
}

// A freshly added binding is invisible if the current Category/Scope/Filter
// excludes it (which made the Add buttons look like they did nothing). Switch
// the view to the matching category and clear the filter, then select it.
void KeyBindingsSettingsPanel::RevealBinding(int id, int category)
{
	if (_filterCtrl != nullptr) {
		_filterCtrl->ChangeValue("");
		_filter.clear();
	}
	Choice_Category->SetSelection(category);
	RebuildScopeChoices();
	Choice_Scope->SetStringSelection("All");
	LoadList();
	SelectKey(id);
	PopulateEditor();
	UpdateWheelVisibility();
	if (_shortcutField != nullptr) _shortcutField->SetFocus();
}

void KeyBindingsSettingsPanel::OnButton_AddEffectClick(wxCommandEvent& event)
{
	int id = _keyBindings->AddKey(KeyBinding(_(""), false, _("On"), _(""), _("2020.15"), false, false, false));
	RevealBinding(id, 1); // Effects
}

void KeyBindingsSettingsPanel::OnButtonAddApplySettingClick(wxCommandEvent& event)
{
	int id = _keyBindings->AddKey(KeyBinding(false, _(""), _(""), _("2020.15"), false, false, false, false));
	RevealBinding(id, 3); // Apply Settings
}

void KeyBindingsSettingsPanel::OnButtonAddPresetClick(wxCommandEvent& event)
{
	int id = _keyBindings->AddKey(KeyBinding(false, _(""), _(""), false, false, false, false));
	RevealBinding(id, 2); // Presets
}


bool KeyBindingsSettingsPanel::TransferDataToWindow() {
    LoadList();
    return true;
}

bool KeyBindingsSettingsPanel::TransferDataFromWindow() {
    // Staged edits are committed explicitly via CommitChanges() (Save button),
    // never automatically - so a closed-without-saving dialog changes nothing.
    return true;
}

void KeyBindingsSettingsPanel::CommitChanges() {
    if (_liveKeyBindings == nullptr) return;
    _liveKeyBindings->GetBindings() = _working.GetBindings();
    _liveKeyBindings->Save();
}
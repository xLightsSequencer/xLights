#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/panel.h>
#include <wx/dataview.h> // wxDataViewListCtrl - native drag-reorder + drop line

#include <string>
#include <vector>

#include "app-shell/KeyBindings.h" // KeyBindingMap held by value (working copy)

class KeyBindingMap;
class KeyBinding;
class EffectManager;
class xLightsFrame;
class wxButton;
class wxChoice;
class wxListCtrl;
class wxSearchCtrl;
class wxListEvent;
class wxCommandEvent;
class wxMouseEvent;

// The Key Bindings editor body, hosted by the modeless KeyBindingsDialog.
// Filterable, category/scope-scoped bindings list on the left; the selected
// binding is edited inline on the right (key captured by keypress + modifier
// checkboxes), with a live Wheel-of-Effects preview for effect bindings.
// Edits are staged on a working copy and only written to the live KeyBindingMap
// by CommitChanges() (the dialog's Save button) - Cancel discards them.
class KeyBindingsSettingsPanel : public wxPanel
{
    EffectManager* _effectManager = nullptr;
    KeyBindingMap* _keyBindings = nullptr;    // points at _working (edits are staged)
    KeyBindingMap* _liveKeyBindings = nullptr; // the real map; only touched on Save
    KeyBindingMap _working;                    // edited copy; discarded on Cancel
    xLightsFrame* _xLights = nullptr;

    void LoadList();
    wxString BuildDetails(const KeyBinding& b) const;
    void RefreshDuplicateHighlights();
    void RefreshRow(int row, const KeyBinding& b);
    int GetSelectedKeyBindingIndex() const; // selected row, or -1
    long RowBindingId(int row) const;       // binding id for a row, or -1
    void SelectKey(int id);
    void RemoveSelected();                  // delete the selected effect binding

    // Inline editor (replaces the old modal popup): the selected binding is
    // edited directly below the list - shortcut captured by keypress, and the
    // effect/preset/setting chosen with a compact control.
    KeyBinding* SelectedBinding() const;
    void PopulateEditor();
    void OnShortcutKey(wxKeyEvent& event);
    void ClearShortcut();
    void ApplyEffectChoice();

    // Reorder the effect bindings (drag with a drop line, or Move Up/Down).
    // Only EFFECT-type bindings reorder - they drive the Wheel of Effects.
    void OnDvBeginDrag(wxDataViewEvent& event);
    void OnDvDropPossible(wxDataViewEvent& event);
    void OnDvDrop(wxDataViewEvent& event);
    void MoveSelectedEffect(int delta);
    long _dragFromId = -1;
    wxButton* _moveUpBtn = nullptr;
    wxButton* _moveDownBtn = nullptr;

    wxStaticText* _editorTitle = nullptr;
    wxTextCtrl* _shortcutField = nullptr; // read-only; captures the base key
    wxButton* _clearShortcutBtn = nullptr;
    wxCheckBox* _cbControl = nullptr;     // Command on macOS
    wxCheckBox* _cbAlt = nullptr;
    wxCheckBox* _cbShift = nullptr;
    wxCheckBox* _cbRawControl = nullptr;  // physical Control on macOS
    wxStaticText* _valueLabel = nullptr;
    wxChoice* _effectChoice = nullptr;
    wxChoice* _presetChoice = nullptr;
    wxTextCtrl* _settingCtrl = nullptr;
    void SyncModifierChecks(const KeyBinding& b);
    // Which value controls were last shown. Re-laying out on every selection
    // change resized the list (and reset the Details column width), so only
    // relayout when the editor's shape actually changes.
    int _lastEditorShape = -1;

    // Live mini Wheel-of-Effects preview: draws each effect at its clock
    // position, highlighting the selected one, and repaints on reorder. Only
    // shown for the Effects category - it means nothing for other bindings.
    wxPanel* _wheelPanel = nullptr;
    wxSizer* _wheelBox = nullptr;
    void OnPaintWheel(wxPaintEvent& event);
    // The bindings the sequencer's Wheel of Effects actually shows: the first
    // kWheelSlots enabled, Sequencer-scoped EFFECT bindings, in order (see
    // EffectsGrid.cpp). Effects past that cut-off never appear on the wheel.
    static constexpr int kWheelSlots = 18;
    std::vector<const KeyBinding*> WheelBindings() const;
    void UpdateWheelVisibility();
    void FitColumns();      // size Details to the leftover space
    // Rebuild the Scope dropdown so it only offers scopes that actually occur
    // in the currently selected Category (no more empty combinations).
    void RebuildScopeChoices();
    // Make a just-added binding visible (switch category, clear filter, select).
    void RevealBinding(int id, int category);
    bool _fitting = false;  // re-entrancy guard (SetWidth can re-fire wxEVT_SIZE)

    static wxString RenderShortcut(const KeyBinding& b);
    static wxString RenderModifiers(const KeyBinding& b); // just the modifier symbols
    static wxString RenderKey(const KeyBinding& b);       // just the base key

    wxChoice* Choice_Category = nullptr; // filters the list by binding kind
    wxChoice* Choice_Scope = nullptr;
    wxDataViewListCtrl* _dvList = nullptr;
    // Hold the columns by pointer: wxDataViewCtrl's positional GetColumn(i)
    // shifts once a column is hidden, which silently hit the wrong column.
    wxDataViewColumn* _colGrip = nullptr;
    wxDataViewColumn* _colAction = nullptr;
    wxDataViewColumn* _colPos = nullptr;
    wxDataViewColumn* _colDetails = nullptr;
    wxDataViewColumn* _colMods = nullptr;
    wxDataViewColumn* _colKey = nullptr;
    std::vector<long> _rowIds;            // row index -> binding id
    wxSearchCtrl* _filterCtrl = nullptr;
    wxString _filter; // lower-cased; whitespace-tokenised AND match in LoadList

    void OnChoice_ScopeSelect(wxCommandEvent& event);
    void OnButton_AddEffectClick(wxCommandEvent& event);
    void OnButtonAddApplySettingClick(wxCommandEvent& event);
    void OnButtonAddPresetClick(wxCommandEvent& event);

    // Broad category a binding falls into ("Effects", "Presets",
    // "Apply Settings" or "Commands"), used by the Category dropdown filter.
    static int CategoryIndexOf(const std::string& type);

public:
    // Human-readable name for a binding type (e.g. "Timing: Add").
    static wxString FriendlyName(const std::string& type);

    KeyBindingsSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~KeyBindingsSettingsPanel();

    virtual bool TransferDataFromWindow() override;
    virtual bool TransferDataToWindow() override;

    // Apply the staged edits to the real key-binding map and persist. Called
    // by the dialog's Save button; Cancel/close just discards the working copy.
    void CommitChanges();
};

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

#include <string>

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

// Preferences page for viewing and editing keyboard shortcuts. Hosts the same
// filterable, scope-scoped bindings list the old Key Bindings dialog used, and
// launches a modal popup editor for a single binding. Edits apply to the live
// KeyBindingMap immediately; they are persisted (keyBindings.Save()) when the
// preferences dialog is accepted (TransferDataFromWindow).
class KeyBindingsSettingsPanel : public wxPanel
{
    EffectManager* _effectManager = nullptr;
    KeyBindingMap* _keyBindings = nullptr;
    xLightsFrame* _xLights = nullptr;

    void LoadList();
    wxString BuildDetails(const KeyBinding& b) const;
    void RefreshDuplicateHighlights();
    void DoEditSelected();
    void RefreshRow(long index, const KeyBinding& b);
    void UpdateEditEnabled();
    int GetSelectedKeyBindingIndex() const;
    void SelectKey(int id);

    static wxString RenderShortcut(const KeyBinding& b);
    void OnListMouseMotion(wxMouseEvent& event);
    long _tooltipItem = -1;

    wxChoice* Choice_Category = nullptr; // filters the list by binding kind
    wxChoice* Choice_Scope = nullptr;
    wxListCtrl* ListCtrl_Bindings = nullptr;
    wxSearchCtrl* _filterCtrl = nullptr;
    wxString _filter; // lower-cased; whitespace-tokenised AND match in LoadList
    wxButton* _editButton = nullptr; // disabled when nothing is selected

    void OnChoice_ScopeSelect(wxCommandEvent& event);
    void OnListCtrl_BindingsKeyDown(wxListEvent& event);
    void OnButton_AddEffectClick(wxCommandEvent& event);
    void OnButtonAddApplySettingClick(wxCommandEvent& event);
    void OnButtonAddPresetClick(wxCommandEvent& event);

    // Broad category a binding falls into ("Effects", "Presets",
    // "Apply Settings" or "Commands"), used by the Category dropdown filter.
    static int CategoryIndexOf(const std::string& type);

public:
    // Public so the popup editor can label a binding with its friendly name.
    static wxString FriendlyName(const std::string& type);

    KeyBindingsSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~KeyBindingsSettingsPanel();

    virtual bool TransferDataFromWindow() override;
    virtual bool TransferDataToWindow() override;
};

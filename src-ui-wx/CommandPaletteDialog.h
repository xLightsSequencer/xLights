#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>

#include <vector>
#include <string>

class wxMenuBar;
class EffectManager;
class CommandPaletteListBox;

struct CommandEntry {
    wxString name;
    wxString category;
    int id = 0;
    wxString shortcut;
    bool isEffect = false;
    wxString effectName;
};

class CommandPaletteDialog : public wxDialog
{
    friend class CommandPaletteListBox;

public:
    CommandPaletteDialog(wxWindow* parent, wxMenuBar* menuBar, EffectManager* effectManager = nullptr);
    virtual ~CommandPaletteDialog() = default;
    void EndModal(int retCode) override;

    int GetSelectedCommandId() const { return _selectedCommandId; }
    bool IsEffectSelected() const { return _isEffectSelected; }
    wxString GetSelectedEffectName() const { return _selectedEffectName; }
    wxString GetSelectedCommandName() const { return _selectedCommandName; }

private:
    void BuildCommandList(wxMenuBar* menuBar);
    void CollectMenuItems(wxMenu* menu, const wxString& category);
    void AddEffectEntries(EffectManager* effectManager);
    void UpdateResults();
    void ActivateSelection();
    bool FuzzyMatch(const wxString& text, const wxString& pattern, int& score) const;

    void OnSearchTextChanged(wxCommandEvent& event);
    void OnSearchKeyDown(wxKeyEvent& event);
    void OnListDoubleClick(wxCommandEvent& event);
    void OnListKeyDown(wxKeyEvent& event);

    wxTextCtrl* _searchCtrl = nullptr;
    CommandPaletteListBox* _listBox = nullptr;

    std::vector<CommandEntry> _allCommands;
    std::vector<size_t> _filteredIndices;
    int _selectedCommandId = wxID_NONE;
    bool _isEffectSelected = false;
    wxString _selectedEffectName;
    wxString _selectedCommandName;
};

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

#include <map>
#include <string>
#include <vector>

#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

class SequenceElements;
class ModelManager;
class TimingElement;

class MovingHeadSongDialog : public wxDialog {
public:
    struct LabelFlags {
        bool chorus = false;
        bool peak = false;
    };

    MovingHeadSongDialog(wxWindow* parent, SequenceElements& elements, ModelManager& models);

    std::string GetBarsTrack() const;
    std::string GetSectionsTrack() const; // empty when none chosen
    std::map<std::string, LabelFlags> GetLabelFlags() const;
    std::vector<std::string> GetSelectedModels() const; // left to right
    const std::vector<std::string>& GetAllModels() const { return _headNames; } // left to right, ticked or not
    long GetLeadInMS() const;
    void SaveSettings() const;
    int GetBurstEveryBars() const { return _burstEvery == nullptr ? 4 : _burstEvery->GetValue(); }
    bool GetUseSlew() const { return _useSlew != nullptr && _useSlew->GetValue(); }
    int GetChorusBeats() const { return _rhythm == nullptr ? 2 : (_rhythm->GetSelection() == 0 ? 1 : (_rhythm->GetSelection() == 2 ? 4 : 2)); }
    bool GetChorusLeftRight() const { return _pairing != nullptr && _pairing->GetSelection() == 1; }

private:
    void OnSectionsChanged(wxCommandEvent& event);
    void RebuildLabelRows();
    void LoadSettings();
    std::vector<std::string> LabelsOf(const std::string& track) const;

    SequenceElements& _elements;
    wxChoice* _bars = nullptr;
    wxChoice* _sections = nullptr;
    wxStaticText* _labelInfo = nullptr;
    wxScrolledWindow* _labelPanel = nullptr;
    wxFlexGridSizer* _labelSizer = nullptr;
    wxCheckListBox* _heads = nullptr;
    wxSpinCtrl* _leadIn = nullptr;
    wxChoice* _pairing = nullptr;
    wxSpinCtrl* _burstEvery = nullptr;
    wxCheckBox* _useSlew = nullptr;
    wxChoice* _rhythm = nullptr;

    struct LabelRow {
        std::string label;
        wxCheckBox* chorus;
        wxCheckBox* peak;
    };
    std::vector<LabelRow> _rows;
    std::vector<std::string> _headNames;
    std::string _savedLabelTrack;
    std::map<std::string, LabelFlags> _savedLabels;
};

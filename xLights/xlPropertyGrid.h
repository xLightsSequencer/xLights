/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#pragma once

#include <set>
#include <wx/propgrid/propgrid.h>
#include <wx/combo.h>

// This is to workaround a crash in wxPropertyGrid where doing
// the DeletePendingObjects() during the OnIdle handling could
// result in objects being deleted while they are being painted
// and other issues.  By turning off the Idle events
// and calling the OnIdle (or DeletePendingEvents) at "safe"
// times, we can prevent the crashes

class xlPropertyGrid : public wxPropertyGrid {
public:
    xlPropertyGrid(wxWindow *parent, wxWindowID id = wxID_ANY,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = wxPG_DEFAULT_STYLE,
                   const wxString& name = wxASCII_STR(wxPropertyGridNameStr)) :
        wxPropertyGrid(parent, id, pos, size, style, name) {
            Connect(wxEVT_IDLE,(wxObjectEventFunction)&xlPropertyGrid::OnIdle, 0, this);
    }
    virtual ~xlPropertyGrid() {
    }
    
    void HideDeletingComboPopups() {
        if (!m_deletedEditorObjects.empty()) {
            for (auto o : m_deletedEditorObjects) {
                wxComboCtrl *combo = dynamic_cast<wxComboCtrl*>(o);
                if (combo) {
                    combo->HidePopup();
                    combo->Hide();
                }
            }
        }
    }
    void OnIdle(wxIdleEvent& event) {
        HideDeletingComboPopups();
        wxPropertyGrid::OnIdle(event);
    }
    virtual void Clear() override {
        HideDeletingComboPopups();
        if (!m_deletedEditorObjects.empty()) {
            for (auto o : m_deletedEditorObjects) {
                wxComboCtrl *combo = dynamic_cast<wxComboCtrl*>(o);
                if (combo) {
                    combo->HidePopup();
                    combo->Hide();
                }
            }
        }
        wxPropertyGrid::Clear();
    }
};

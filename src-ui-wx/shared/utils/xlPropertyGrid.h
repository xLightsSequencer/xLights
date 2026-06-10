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
#include <wx/propgrid/propgridiface.h>
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
            // Tab traversal: stock wxPropertyGrid sends focus to the next sibling
            // widget on Tab (via Navigate()), so users have to click their way
            // back into the grid to edit the next property. Intercept Tab via
            // CHAR_HOOK on the grid and walk to the next/previous visible
            // property instead, matching the behavior of every other property
            // editor (Excel, Visual Studio properties pane, etc.).
            Bind(wxEVT_CHAR_HOOK, &xlPropertyGrid::OnCharHook, this);
    }
    virtual ~xlPropertyGrid() {
    }
    
    void HideDeletingComboPopups() {
        // HidePopup() pumps events (the popup's focus-loss drives an
        // OnMeasureItem/OnComboItemPaint repaint, and an idle/clear can re-enter
        // here). Re-entering while we iterate m_deletedEditorObjects would mutate
        // the vector mid-walk and re-hide a half-destroyed combo -- the exact
        // "deleted while painted" hazard this class exists to avoid. Guard it.
        if (m_hidingComboPopups) {
            return;
        }
        m_hidingComboPopups = true;
        for (auto o : m_deletedEditorObjects) {
            wxComboCtrl *combo = dynamic_cast<wxComboCtrl*>(o);
            if (combo) {
                if (combo->IsPopupShown()) {
                    combo->HidePopup();
                }
                combo->Hide();
            }
        }
        m_hidingComboPopups = false;
    }
    void OnIdle(wxIdleEvent& event) {
        HideDeletingComboPopups();
        wxPropertyGrid::OnIdle(event);
    }
    virtual void Clear() override {
#ifdef __WXGTK__
        // On GTK, HidePopup() pumps the event queue. Doing that while this
        // window (or an ancestor) is Frozen() has been observed to wedge GTK's
        // idle-driven repaint permanently (xlights #6215/#4175) - clicks still
        // register but nothing ever redraws again until restart. Defer to the
        // OnIdle handler below, which runs once Thaw() lets idle events flow.
        if (!IsFrozen()) {
            HideDeletingComboPopups();
        }
#else
        HideDeletingComboPopups();
#endif
        wxPropertyGrid::Clear();
    }

    void OnCharHook(wxKeyEvent& event) {
        // Only interested in plain Tab / Shift+Tab; Ctrl/Alt+Tab passes through.
        if (event.GetKeyCode() != WXK_TAB || event.ControlDown() || event.AltDown()) {
            event.Skip();
            return;
        }
        // If no editor is open, let wx do its default traversal so Tab can
        // exit the grid normally when the user is just navigating.
        if (!IsEditorFocused()) {
            event.Skip();
            return;
        }
        wxPGProperty* sel = GetSelection();
        if (sel == nullptr) {
            event.Skip();
            return;
        }
        wxPropertyGridIterator it = GetIterator(wxPG_ITERATE_VISIBLE, sel);
        if (event.ShiftDown()) {
            --it;
        } else {
            ++it;
        }
        if (it.AtEnd()) {
            // Boundary: at the first/last property — fall through to default
            // traversal so the user can Tab out of the grid entirely.
            event.Skip();
            return;
        }
        // Commit the current editor's value, then jump to the next property.
        // The commit fires wxEVT_PG_CHANGED, which queues a model reload that
        // rebuilds the whole grid and invalidates property pointers. Defer the
        // focus shift via CallAfter so it runs once the rebuild has settled,
        // and look up the target by name (pointers from before the rebuild
        // are dead).
        const wxString nextName = (*it)->GetName();
        CommitChangesFromEditor();
        CallAfter([this, nextName]() {
            if (wxPGProperty* p = GetPropertyByName(nextName)) {
                SelectProperty(p, true);
                EnsureVisible(p);
            }
        });
    }

private:
    bool m_hidingComboPopups = false;
};

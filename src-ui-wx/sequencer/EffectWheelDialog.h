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
#include <wx/graphics.h>
#include <vector>
#include <string>
#include "app-shell/KeyBindings.h"

class EffectWheelDialog : public wxDialog {
public:
    EffectWheelDialog(wxWindow* parent, const std::vector<const KeyBinding*>& bindings);
    virtual ~EffectWheelDialog() = default;

    void PositionAtMouse(const wxPoint& mousePos);
    const KeyBinding* GetSelectedKeyBinding() const { return m_selectedBinding; }

private:
    void OnPaint(wxPaintEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnKillFocus(wxFocusEvent& event);
    void OnKeyDown(wxKeyEvent& event);

    int GetSectorAtMouse(const wxPoint& pos);
    std::string GetEffectNameFromBinding(const KeyBinding* kb);

    std::vector<const KeyBinding*> m_bindings;
    const KeyBinding* m_selectedBinding;
    int m_hoveredSector;
    wxPoint m_center;

    // Radius constants
    int m_outerRadius;
    int m_innerRadius;

    DECLARE_EVENT_TABLE()
};

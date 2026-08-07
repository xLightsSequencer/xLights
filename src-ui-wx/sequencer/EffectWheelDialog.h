#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the GitHub commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in GitHub
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
    enum class HitZone { None, Spoke, PageLive, PageDead, Exit };
    struct HitResult {
        HitZone zone = HitZone::None;
        int index = -1;
    };

    void OnPaint(wxPaintEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnShow(wxShowEvent& event);

    HitResult HitTest(const wxPoint& pos) const;
    std::string GetEffectNameFromBinding(const KeyBinding* kb);
    void SetCircularShape(int radius);

    int PageCount() const;
    std::vector<const KeyBinding*> CurrentPageBindings() const;

    std::vector<const KeyBinding*> m_bindings;
    const KeyBinding* m_selectedBinding;
    int m_hoveredSector;
    int m_hoveredPage;
    int m_currentPage;
    wxPoint m_center;

    int m_outerRadius;
    int m_innerRadius;
    int m_exitRadius;
    bool m_centerHovered;

    // EffectsGrid.cpp truncates the incoming list to at most kPageSize * kMaxPages
    // (72), so PageCount() can never exceed kMaxPages.
    static constexpr int kPageSize = 18;
    static constexpr int kMaxPages = 4;

    DECLARE_EVENT_TABLE()
};

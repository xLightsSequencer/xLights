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

#include "layout/ControllerTreeUtils.h"

using namespace ControllerTree;

class xLightsFrame;
class Controller;
class wxTreeListCtrl;
class wxTreeListEvent;

// Read-only reference view: every defined controller column, for every
// controller, in a curated fixed order (see CTRL_FULL_COLUMN_ORDER). Opened
// via the docked ControllerListPanel's "Show All Info" button. No editing, no
// context menu, no drag/drop, no column visibility toggling — see the docked
// ControllerListPanel for all of that.
class ControllerFullColumnsDialog : public wxDialog {
public:
    ControllerFullColumnsDialog(wxWindow* parent, xLightsFrame* frame);
    ~ControllerFullColumnsDialog() override;

private:
    void CreateTree();
    void PopulateAllControllers();
    void OnItemExpanding(wxTreeListEvent& event);

    xLightsFrame* _frame = nullptr;
    wxTreeListCtrl* _tree = nullptr;
    ControllerTree::ControllerLedIcons _ledIcons;
};

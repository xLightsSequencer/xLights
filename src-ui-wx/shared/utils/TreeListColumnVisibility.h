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

#include <string>
#include <vector>

class wxTreeListCtrl;

// Adds a right-click menu on the tree's column headers to show/hide columns.
// Column 0 is never hidden and not offered. Hidden state persists in the
// xLights config under configKey as a comma-separated list of hidden column
// titles and is re-applied immediately by this call.
//
// defaultHiddenTitles names columns that should start hidden the first time
// they are seen (e.g. newly added columns on an existing config). Once a
// column has been seen, its visibility is fully user-controlled like any
// other column; this only affects first appearance, tracked in a companion
// "<configKey>Seen" config entry.
void InstallTreeListColumnVisibility(wxTreeListCtrl* tree, const std::string& configKey,
                                      const std::vector<std::string>& defaultHiddenTitles = {});

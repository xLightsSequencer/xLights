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

#include <wx/bitmap.h>
#include <wx/bmpbndl.h>
#include <wx/clntdata.h>
#include <wx/colour.h>
#include <wx/treelist.h>
#include <wx/vector.h>

#include <cstddef>
#include <cstdint>
#include <string>

class Controller;
class xLightsFrame;

// title: displayed column header. id: stable identifier matching
// ControllerColumnLabel()'s switch and the Controller::GetColumnNLabel()
// virtuals; independent of display position. Shared by ControllerListPanel
// (which orders/hides columns per user preference) and
// ControllerFullColumnsDialog (which always shows every column, in its own
// curated order — see CTRL_FULL_COLUMN_ORDER).
struct ControllerColumnDef {
    const char* title;
    int id;
};

extern const ControllerColumnDef CTRL_COLS[];
extern const size_t CTRL_COL_COUNT;

// Fixed display order for ControllerFullColumnsDialog: identity/description,
// then addressing, then upload/behavior settings, then everything else.
// Contains each id in CTRL_COLS exactly once.
extern const int CTRL_FULL_COLUMN_ORDER[];
extern const size_t CTRL_FULL_COLUMN_ORDER_COUNT;

std::string ControllerColumnLabel(const Controller* c, int id);

// Linear lookup of a column's display title by its stable id; used when
// iterating CTRL_FULL_COLUMN_ORDER (which stores ids, not titles).
const char* ControllerColumnTitle(int id);

wxBitmap CreateLedBitmap(const wxColour& color, int size);

wxBitmapBundle CreateLedBitmapBundle(const wxColour& color, int size);

// Image-list indices for the three ping-status LED bitmaps, as returned by
// AppendControllerLedIcons().
struct ControllerLedIcons {
    int idxGreen = -1;
    int idxRed = -1;
    int idxGray = -1;
};

struct ControllerLedBitmaps {
    wxBitmapBundle green;
    wxBitmapBundle red;
    wxBitmapBundle gray;
};
ControllerLedBitmaps CreateControllerLedBitmaps(int size);

ControllerLedIcons AppendControllerLedIcons(wxVector<wxBitmapBundle>& bundles);

enum class ControllerPingBucket { Green, Red, Gray };
ControllerPingBucket ClassifyControllerPing(const Controller* c);

int GetControllerIconIndex(const Controller* c, xLightsFrame* frame, const ControllerLedIcons& icons);

class CtrlRowData : public wxClientData {
public:
    enum class Kind { Placeholder, Port, Model };
    Kind kind;
    std::string controllerName;
    std::string modelName;                 // Kind::Model only
    int stringIndex = -1;                  // Kind::Model: UDControllerPortModel::GetString()
    int32_t startChannel = 0;              // Kind::Model: absolute channel range
    int32_t endChannel = 0;
    bool isShadow = false;                 // Kind::Model: model shadows another
    std::string shadowOf;                  // main model name when isShadow

    CtrlRowData(Kind k, const std::string& ctrl) : kind(k), controllerName(ctrl) {}
};

// Populates the port/model children of a controller's top-level tree row on
// first expansion (replaces the "loading..." placeholder). Shared by
// ControllerListPanel and ControllerFullColumnsDialog so both trees expand
// controllers identically. No-op if ctrlItem's first child is not the
// placeholder (i.e. already populated).
void PopulateControllerPorts(wxTreeListCtrl* tree, wxTreeListItem ctrlItem, xLightsFrame* frame);

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

#include <wx/window.h>
#include <wx/statbox.h>
#include <wx/settings.h>
#include <wx/font.h>

// Section headings on the preferences pages are wxStaticBox captions. On macOS
// the native box caption uses a smaller "small system font", which looks out of
// place next to the full-size control labels on the same page. Re-font every
// wxStaticBox caption on the panel to the standard GUI font (bold) so section
// titles match the label size and read as intentional headings. Call once after
// the panel's controls have been created.
inline void StylePreferenceSectionHeaders(wxWindow* panel) {
    if (panel == nullptr) return;
    wxFont f = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    f.MakeBold();
    for (wxWindow* child : panel->GetChildren()) {
        // wxStaticBox has its own wxWidgets RTTI, so IsKindOf is reliable here
        // even in builds without C++ RTTI.
        if (child != nullptr && child->IsKindOf(wxCLASSINFO(wxStaticBox))) {
            child->SetFont(f);
        }
    }
}

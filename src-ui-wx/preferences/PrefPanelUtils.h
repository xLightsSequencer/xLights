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
#include <wx/stattext.h>
#include <wx/font.h>

// Build a section heading for a preferences page. Uses the panel's normal
// control font made bold, so headings read at the same size as the labels
// beneath them. Preferred over a wxStaticBox caption, whose native font macOS
// renders smaller and refuses to override via SetFont.
inline wxStaticText* MakePreferenceSectionHeader(wxWindow* parent, const wxString& title) {
    auto* header = new wxStaticText(parent, wxID_ANY, title);
    wxFont f = header->GetFont();
    f.MakeBold();
    header->SetFont(f);
    return header;
}

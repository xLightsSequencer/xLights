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

#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <string>
#include <vector>

class SelectModelDialog : public wxDialog
{
public:
    SelectModelDialog(wxWindow* parent, const std::vector<std::string>& modelNames);
    virtual ~SelectModelDialog() = default;

    std::string GetSelectedModel() const;
    bool ShouldAddAlias() const;

private:
    wxListBox* ListBoxModels = nullptr;
    wxCheckBox* CheckBoxAddAlias = nullptr;
};

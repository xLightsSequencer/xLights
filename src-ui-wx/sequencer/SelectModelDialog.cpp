/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SelectModelDialog.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

SelectModelDialog::SelectModelDialog(wxWindow* parent, const std::vector<std::string>& modelNames)
    : wxDialog(parent, wxID_ANY, "Select Model")
{
    wxFlexGridSizer* sizer = new wxFlexGridSizer(0, 1, 0, 0);
    sizer->AddGrowableCol(0);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Choose the model to use instead:"),
               0, wxALL | wxALIGN_LEFT, 10);

    wxArrayString choices;
    for (const auto& name : modelNames)
        choices.Add(wxString(name));

    ListBoxModels = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(300, 200), choices, wxLB_SINGLE | wxLB_SORT);
    sizer->Add(ListBoxModels, 0, wxALL | wxEXPAND, 5);

    CheckBoxAddAlias = new wxCheckBox(this, wxID_ANY, "Also add alias to model (auto-remap future sequences)");
    CheckBoxAddAlias->SetValue(true);
    sizer->Add(CheckBoxAddAlias, 0, wxALL | wxALIGN_LEFT, 5);

    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();
    buttons->AddButton(new wxButton(this, wxID_OK));
    buttons->AddButton(new wxButton(this, wxID_CANCEL));
    buttons->Realize();
    sizer->Add(buttons, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

    SetSizer(sizer);
    sizer->Fit(this);
    sizer->SetSizeHints(this);
    SetEscapeId(wxID_CANCEL);
    Centre();

    ListBoxModels->Bind(wxEVT_LISTBOX_DCLICK, [this](wxCommandEvent&) {
        EndModal(wxID_OK);
    });
}

std::string SelectModelDialog::GetSelectedModel() const
{
    int sel = ListBoxModels->GetSelection();
    if (sel == wxNOT_FOUND)
        return "";
    return ListBoxModels->GetString(sel).ToStdString();
}

bool SelectModelDialog::ShouldAddAlias() const
{
    return CheckBoxAddAlias->IsChecked();
}

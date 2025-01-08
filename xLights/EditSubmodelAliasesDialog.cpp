/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "EditSubmodelAliasesDialog.h"
#include "models/Model.h"
#include "models/SubModel.h"

#include <log4cpp/Category.hh>

//(*InternalHeaders(EditSubmodelAliasesDialog)
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/artprov.h>

//(*IdInit(EditSubmodelAliasesDialog)
const wxWindowID EditSubmodelAliasesDialog::ID_LISTBOX1 = wxNewId();
const wxWindowID EditSubmodelAliasesDialog::ID_BITMAPBUTTONUP = wxNewId();
const wxWindowID EditSubmodelAliasesDialog::ID_BITMAPBUTTONDOWN = wxNewId();
const wxWindowID EditSubmodelAliasesDialog::ID_BUTTON1 = wxNewId();
const wxWindowID EditSubmodelAliasesDialog::ID_BUTTON2 = wxNewId();
const wxWindowID EditSubmodelAliasesDialog::ID_BUTTON3 = wxNewId();
const wxWindowID EditSubmodelAliasesDialog::ID_BUTTON4 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EditSubmodelAliasesDialog, wxDialog)
//(*EventTable(EditSubmodelAliasesDialog)
//*)
END_EVENT_TABLE()

EditSubmodelAliasesDialog::EditSubmodelAliasesDialog(wxWindow* parent, Model* m, wxString submodelname, wxWindowID id, const wxPoint& pos, const wxSize& size) : _m(m)
{
    //(*Initialize(EditSubmodelAliasesDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;

    Create(parent, wxID_ANY, _("SubModel Aliases"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    ListBoxAliases = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SINGLE, wxDefaultValidator, _T("ID_LISTBOX1"));
    FlexGridSizer1->Add(ListBoxAliases, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
    ButtonMoveUp = new wxBitmapButton(this, ID_BITMAPBUTTONUP, wxArtProvider::GetBitmapBundle("wxART_GO_UP", wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTONUP"));
    FlexGridSizer4->Add(ButtonMoveUp, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonMoveDown = new wxBitmapButton(this, ID_BITMAPBUTTONDOWN, wxArtProvider::GetBitmapBundle("wxART_GO_DOWN", wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTONDOWN"));
    FlexGridSizer4->Add(ButtonMoveDown, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    ButtonAdd = new wxButton(this, ID_BUTTON1, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer2->Add(ButtonAdd, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonDelete = new wxButton(this, ID_BUTTON2, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer2->Add(ButtonDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    ButtonOk = new wxButton(this, ID_BUTTON3, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    ButtonOk->SetDefault();
    FlexGridSizer3->Add(ButtonOk, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonCancel = new wxButton(this, ID_BUTTON4, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer3->Add(ButtonCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_LISTBOX1, wxEVT_COMMAND_LISTBOX_SELECTED, (wxObjectEventFunction)&EditSubmodelAliasesDialog::OnListBoxAliasesSelect);
    Connect(ID_BITMAPBUTTONDOWN, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EditSubmodelAliasesDialog::OnButtonMoveDownClick);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EditSubmodelAliasesDialog::OnButtonAddClick);
    Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EditSubmodelAliasesDialog::OnButtonDeleteClick);
    Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EditSubmodelAliasesDialog::OnButtonOkClick);
    Connect(ID_BUTTON4, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EditSubmodelAliasesDialog::OnButtonCancelClick);
    //*)

    _sm = _m->GetSubModel(submodelname);
    if (_sm != nullptr) {
        for (const auto& it : _sm->GetAliases()) {
            ListBoxAliases->Append(it);
        }
    }
    else
    {

        EndDialog(wxCANCEL);
    }

    SetSizerAndFit(FlexGridSizer1);
    SetEscapeId(ID_BUTTON4);

    ValidateWindow();
}

EditSubmodelAliasesDialog::~EditSubmodelAliasesDialog()
{
    //(*Destroy(EditSubmodelAliasesDialog)
    //*)
}

void EditSubmodelAliasesDialog::ValidateWindow()
{
    if (ListBoxAliases->GetSelection() >= 0) {
        ButtonDelete->Enable();
    } else {
        ButtonDelete->Disable();
    }
    if (ListBoxAliases->GetSelection() >= 0 && ListBoxAliases->GetSelection() < ListBoxAliases->GetCount()) {
        ButtonMoveUp->Enable(true);
        ButtonMoveDown->Enable(true);
    } else {
        ButtonMoveUp->Enable(false);
        ButtonMoveDown->Enable(false);
    }
}

void EditSubmodelAliasesDialog::OnButtonAddClick(wxCommandEvent& event)
{
    wxTextEntryDialog te(this, "Alias to add", "Add an alias");

    if (te.ShowModal() == wxID_OK) {
        auto add = te.GetValue().Lower();

        bool found = false;
        for (size_t i = 0; !found && i < ListBoxAliases->GetCount(); ++i) {
            if (ListBoxAliases->GetString(i) == add)
                found = true;
        }

        if (!found) {
            ListBoxAliases->Append(add);
        }
    }
    ValidateWindow();
}

void EditSubmodelAliasesDialog::OnButtonDeleteClick(wxCommandEvent& event)
{
    if (ListBoxAliases->GetSelection() >= 0) {
        ListBoxAliases->Delete(ListBoxAliases->GetSelection());
    }
    ValidateWindow();
}

void EditSubmodelAliasesDialog::OnButtonOkClick(wxCommandEvent& event)
{
    std::list<std::string> aliases;
    for (size_t i = 0; i < ListBoxAliases->GetCount(); ++i) {
        aliases.push_back(ListBoxAliases->GetString(i));
    }
    if (_sm != nullptr) _sm->SetAliases(aliases);
    EndDialog(wxID_OK);
}

void EditSubmodelAliasesDialog::OnButtonCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void EditSubmodelAliasesDialog::OnListBoxAliasesSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void EditSubmodelAliasesDialog::OnButtonMoveUpClick(wxCommandEvent& event) {
    if (ListBoxAliases->GetCount() == 0) {
        return;
    }

    int selectedIndex = ListBoxAliases->GetSelection();

    if (selectedIndex != wxNOT_FOUND && selectedIndex > 0) {
        wxString selectedItem = ListBoxAliases->GetString(selectedIndex);
        ListBoxAliases->Delete(selectedIndex);
        ListBoxAliases->Insert(selectedItem, selectedIndex - 1);
        ListBoxAliases->SetSelection(selectedIndex - 1);
    }
}

void EditSubmodelAliasesDialog::OnButtonMoveDownClick(wxCommandEvent& event) {
    if (ListBoxAliases->GetCount() == 0) {
        return;
    }

    int selectedIndex = ListBoxAliases->GetSelection();

    if (selectedIndex != wxNOT_FOUND && selectedIndex < (ListBoxAliases->GetCount() - 1)) {
        wxString selectedItem = ListBoxAliases->GetString(selectedIndex);
        ListBoxAliases->Delete(selectedIndex);
        ListBoxAliases->Insert(selectedItem, selectedIndex + 1);
        ListBoxAliases->SetSelection(selectedIndex + 1);
    }
}

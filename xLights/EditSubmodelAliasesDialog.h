#ifndef EDITSUBMODELALIASESDIALOG_H
#define EDITSUBMODELALIASESDIALOG_H

//(*Headers(EditSubmodelAliasesDialog)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
//*)

class Model;

class EditSubmodelAliasesDialog : public wxDialog
{
public:
    EditSubmodelAliasesDialog(wxWindow* parent, Model* m, wxString name, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~EditSubmodelAliasesDialog();

    //(*Declarations(EditSubmodelAliasesDialog)
    wxBitmapButton* ButtonMoveDown;
    wxBitmapButton* ButtonMoveUp;
    wxButton* ButtonAdd;
    wxButton* ButtonCancel;
    wxButton* ButtonDelete;
    wxButton* ButtonOk;
    wxListBox* ListBoxAliases;
    //*)

protected:
    Model* _m = nullptr;
    Model* _sm = nullptr;
    void ValidateWindow();

    //(*Identifiers(EditSubmodelAliasesDialog)
    static const wxWindowID ID_LISTBOX1;
    static const wxWindowID ID_BITMAPBUTTONUP;
    static const wxWindowID ID_BITMAPBUTTONDOWN;
    static const wxWindowID ID_BUTTON1;
    static const wxWindowID ID_BUTTON2;
    static const wxWindowID ID_BUTTON3;
    static const wxWindowID ID_BUTTON4;
    //*)

private:
    //(*Handlers(EditSubmodelAliasesDialog)
    void OnButtonAddClick(wxCommandEvent& event);
    void OnButtonDeleteClick(wxCommandEvent& event);
    void OnButtonOkClick(wxCommandEvent& event);
    void OnButtonCancelClick(wxCommandEvent& event);
    void OnListBoxAliasesSelect(wxCommandEvent& event);
    void OnButtonMoveUpClick(wxCommandEvent& event);
    void OnButtonMoveDownClick(wxCommandEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};

#endif

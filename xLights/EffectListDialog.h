#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(EffectListDialog)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/listbox.h>
//*)

class EffectListDialog: public wxDialog
{
public:

    EffectListDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~EffectListDialog();

    //(*Declarations(EffectListDialog)
    wxButton* Button_Rename;
    wxListBox* ListBox1;
    wxButton* Button_Delete;
    //*)

protected:

    //(*Identifiers(EffectListDialog)
    static const long ID_LISTBOX1;
    static const long ID_BUTTON3;
    static const long ID_BUTTON2;
    //*)

private:

    //(*Handlers(EffectListDialog)
    void OnButton_RenameClick(wxCommandEvent& event);
    void OnButton_DeleteClick(wxCommandEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};

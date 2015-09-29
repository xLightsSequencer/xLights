#ifndef ADDSHOWDIALOG_H
#define ADDSHOWDIALOG_H

//(*Headers(AddShowDialog)
#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/listbox.h>
//*)

class AddShowDialog: public wxDialog
{
public:

    AddShowDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~AddShowDialog();
    bool StartBeforeEnd();
    bool IsPlaylistSelected();
    wxString PartialEventCode();

















    //(*Declarations(AddShowDialog)
    wxCheckBox* CheckBoxLastItem;
    wxCheckBox* CheckBoxFirstItem;
    wxSpinCtrl* SpinCtrlStartMinute;
    wxStaticText* StaticText1;
    wxStaticText* StaticText10;
    wxStaticText* StaticText3;
    wxCheckBox* CheckBoxRandom;
    wxStaticText* StaticText8;
    wxListBox* ListBoxDates;
    wxStaticText* StaticText7;
    wxSpinCtrl* SpinCtrlEndMinute;
    wxStaticText* StaticText4;
    wxStaticText* StaticText5;
    wxStaticText* StaticText2;
    wxCheckBox* CheckBoxRepeat;
    wxStaticText* StaticText6;
    wxStaticText* StaticText9;
    wxSpinCtrl* SpinCtrlEndHour;
    wxChoice* ChoicePlayList;
    wxSpinCtrl* SpinCtrlStartHour;
    //*)

protected:

    //(*Identifiers(AddShowDialog)
    static const long ID_STATICTEXT1;
    static const long ID_CHOICE_PLAYLIST;
    static const long ID_STATICTEXT5;
    static const long ID_SPINCTRL1;
    static const long ID_STATICTEXT8;
    static const long ID_SPINCTRL2;
    static const long ID_STATICTEXT6;
    static const long ID_SPINCTRL3;
    static const long ID_STATICTEXT9;
    static const long ID_SPINCTRL4;
    static const long ID_STATICTEXT2;
    static const long ID_CHECKBOX_REPEAT;
    static const long ID_STATICTEXT3;
    static const long ID_CHECKBOX_FIRST_ITEM;
    static const long ID_STATICTEXT4;
    static const long ID_CHECKBOX_LAST_ITEM;
    static const long ID_STATICTEXT7;
    static const long ID_CHECKBOX_RANDOM;
    static const long ID_STATICTEXT10;
    static const long ID_LISTBOX1;
    //*)

private:

    //(*Handlers(AddShowDialog)
    //*)

    DECLARE_EVENT_TABLE()
};

#endif

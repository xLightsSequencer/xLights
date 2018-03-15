#ifndef SEQELEMENTMISMATCHDIALOG_H
#define SEQELEMENTMISMATCHDIALOG_H

//(*Headers(SeqElementMismatchDialog)
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class SeqElementMismatchDialog: public wxDialog
{
public:

    SeqElementMismatchDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~SeqElementMismatchDialog();

    //(*Declarations(SeqElementMismatchDialog)
    wxChoice* ChoiceModels;
    wxRadioButton* RadioButtonDelete;
    wxRadioButton* RadioButtonMap;
    wxRadioButton* RadioButtonRename;
    wxStaticText* StaticTextMessage;
    //*)

protected:

    //(*Identifiers(SeqElementMismatchDialog)
    static const long ID_STATICTEXT1;
    static const long ID_RADIOBUTTON2;
    static const long ID_RADIOBUTTON1;
    static const long ID_RADIOBUTTON3;
    static const long ID_CHOICE1;
    //*)

private:

    //(*Handlers(SeqElementMismatchDialog)
    void OnChoiceModelsSelect(wxCommandEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};

#endif

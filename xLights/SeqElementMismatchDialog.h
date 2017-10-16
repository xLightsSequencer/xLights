#ifndef SEQELEMENTMISMATCHDIALOG_H
#define SEQELEMENTMISMATCHDIALOG_H

//(*Headers(SeqElementMismatchDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobut.h>
#include <wx/choice.h>
#include <wx/dialog.h>
//*)

class SeqElementMismatchDialog: public wxDialog
{
public:

    SeqElementMismatchDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~SeqElementMismatchDialog();

    //(*Declarations(SeqElementMismatchDialog)
    wxRadioButton* RadioButtonRename;
    wxRadioButton* RadioButtonMap;
    wxStaticText* StaticTextMessage;
    wxChoice* ChoiceModels;
    wxRadioButton* RadioButtonDelete;
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

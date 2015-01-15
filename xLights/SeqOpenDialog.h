#ifndef SEQOPENDIALOG_H
#define SEQOPENDIALOG_H

//(*Headers(SeqOpenDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobut.h>
#include <wx/choice.h>
#include <wx/dialog.h>
//*)

class SeqOpenDialog: public wxDialog
{
public:

    SeqOpenDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~SeqOpenDialog();

    //(*Declarations(SeqOpenDialog)
    wxChoice* ChoiceSeqXMLFiles;
    wxChoice* SeqChoiceTiming;
    wxRadioButton* RadioButtonXML;
    wxChoice* ChoiceSeqBinaryFiles;
    wxRadioButton* RadioButtonBinary;
    //*)

protected:

    //(*Identifiers(SeqOpenDialog)
    static const long ID_RADIOBUTTON1;
    static const long ID_CHOICE1;
    static const long ID_RADIOBUTTON4;
    static const long ID_CHOICE3;
    static const long ID_CHOICE2;
    //*)

private:

    //(*Handlers(SeqOpenDialog)
    void OnRadioButtonBinarySelect(wxCommandEvent& event);
    void OnRadioButtonXMLSelect(wxCommandEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};

#endif

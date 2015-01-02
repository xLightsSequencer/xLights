#ifndef SEQPARMSDIALOG_H
#define SEQPARMSDIALOG_H

//(*Headers(SeqParmsDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checklst.h>
#include <wx/dialog.h>
//*)


class SeqParmsDialog: public wxDialog
{
public:

    SeqParmsDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~SeqParmsDialog();

    //(*Declarations(SeqParmsDialog)
    wxCheckListBox* CheckListBox1;
    wxStaticText* StaticText_Filename;
    //*)

protected:

    //(*Identifiers(SeqParmsDialog)
    static const long ID_STATICTEXT_FILENAME;
    static const long ID_CHECKLISTBOX1;
    //*)

private:

    //(*Handlers(SeqParmsDialog)
    void OnSpinCtrlBaseChannelChange(wxSpinEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};

#endif

#ifndef SEQEXPORTDIALOG_H
#define SEQEXPORTDIALOG_H

//(*Headers(SeqExportDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/dialog.h>
//*)

class SeqExportDialog: public wxDialog
{
public:

    SeqExportDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~SeqExportDialog();

    //(*Declarations(SeqExportDialog)
    wxTextCtrl* TextCtrlFilename;
    wxStaticText* StaticText1;
    wxStaticText* StaticText3;
    wxChoice* ChoiceFormat;
    //*)

protected:

    //(*Identifiers(SeqExportDialog)
    static const long ID_STATICTEXT1;
    static const long ID_CHOICE1;
    static const long ID_STATICTEXT3;
    static const long ID_TEXTCTRL2;
    //*)

private:

    //(*Handlers(SeqExportDialog)
    //*)

    DECLARE_EVENT_TABLE()
};

#endif

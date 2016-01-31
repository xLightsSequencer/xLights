#ifndef SEQEXPORTDIALOG_H
#define SEQEXPORTDIALOG_H

//(*Headers(SeqExportDialog)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
//*)

class SeqExportDialog: public wxDialog
{
public:

    SeqExportDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~SeqExportDialog();
    void ModelExportTypes();

    //(*Declarations(SeqExportDialog)
    wxTextCtrl* TextCtrlFilename;
    wxChoice* ChoiceFormat;
    wxStaticText* StaticText1;
    wxStaticText* StaticText3;
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
    void OnChoiceFormatSelect(wxCommandEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};

#endif

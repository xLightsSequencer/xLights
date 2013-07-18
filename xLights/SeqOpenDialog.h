#ifndef SEQOPENDIALOG_H
#define SEQOPENDIALOG_H

//(*Headers(SeqOpenDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobox.h>
#include <wx/spinctrl.h>
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
    wxChoice* ChoiceMediaFiles;
    wxRadioButton* RadioButtonNewMusic;
    wxStaticText* StaticText2;
    wxRadioButton* RadioButtonXlights;
    wxStaticText* StaticText1;
    wxStaticText* StaticText3;
    wxSpinCtrl* SpinCtrlDuration;
    wxRadioBox* RadioBoxTimmingChoice;
    wxRadioButton* RadioButtonNewAnim;
    wxChoice* ChoiceSeqFiles;
    //*)

protected:

    //(*Identifiers(SeqOpenDialog)
    static const long ID_RADIOBUTTON1;
    static const long ID_STATICTEXT1;
    static const long ID_CHOICE1;
    static const long ID_RADIOBUTTON2;
    static const long ID_STATICTEXT2;
    static const long ID_CHOICE2;
    static const long ID_RADIOBOX1;
    static const long ID_RADIOBUTTON3;
    static const long ID_STATICTEXT3;
    static const long ID_SPINCTRL1;
    //*)

private:

    //(*Handlers(SeqOpenDialog)
    void OnChoiceSeqFilesSelect(wxCommandEvent& event);
    void OnChoiceMediaFilesSelect(wxCommandEvent& event);
    void OnRadioButton1Select(wxCommandEvent& event);
    void OnRadioButton2Select(wxCommandEvent& event);
    void OnRadioBox1Select(wxCommandEvent& event);
    void OnRadioBox1Select1(wxCommandEvent& event);
    void OnRadioButtonXlightsSelect(wxCommandEvent& event);
    void OnRadioButtonNewMusicSelect(wxCommandEvent& event);
    void OnRadioButtonNewAnimSelect(wxCommandEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};

#endif

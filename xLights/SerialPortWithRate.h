#ifndef SERIALPORTWITHRATE_H
#define SERIALPORTWITHRATE_H

//(*Headers(SerialPortWithRate)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
//*)

class SerialPortWithRate: public wxDialog
{
public:

    SerialPortWithRate(wxWindow* parent);
    virtual ~SerialPortWithRate();
    void ProtocolChange();
    wxString GetRateString();

    //(*Declarations(SerialPortWithRate)
    wxStaticText* StaticTextPort;
    wxStaticText* StaticTextExplanation;
    wxStaticText* StaticText1;
    wxStaticText* StaticText3;
    wxTextCtrl* TextCtrlLastChannel;
    wxChoice* ChoicePort;
    wxStaticText* StaticTextRate;
    wxChoice* ChoiceProtocol;
    wxChoice* ChoiceBaudRate;
    //*)


protected:

    //(*Identifiers(SerialPortWithRate)
    static const long ID_CHOICE_PROTOCOL;
    static const long ID_STATICTEXT_EXPLANATION;
    static const long ID_STATICTEXT_PORT;
    static const long ID_CHOICE_PORT;
    static const long ID_STATICTEXT_RATE;
    static const long ID_CHOICE_BAUD_RATE;
    static const long ID_STATICTEXT3;
    static const long ID_TEXTCTRL_LAST_CHANNEL;
    static const long ID_STATICTEXT1;
    //*)

private:

    //(*Handlers(SerialPortWithRate)
    void OnChoiceProtocolSelect(wxCommandEvent& event);
    //*)

    wxFlexGridSizer* MainSizer;
    void PopulatePortChooser(wxArrayString *chooser);
    void SetLabel(const wxString& newlabel);

    DECLARE_EVENT_TABLE()
};

#endif

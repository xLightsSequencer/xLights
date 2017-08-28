#ifndef SERIALPORTWITHRATE_H
#define SERIALPORTWITHRATE_H

//(*Headers(SerialPortWithRate)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class SerialOutput;
class OutputManager;

class SerialPortWithRate: public wxDialog
{
    SerialOutput* _original;
    SerialOutput** _serial;
    OutputManager* _outputManager;
    void ValidateWindow();

public:

    SerialPortWithRate(wxWindow* parent, SerialOutput** serial, OutputManager* outputManager, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~SerialPortWithRate();
    void ProtocolChange();

    //(*Declarations(SerialPortWithRate)
    wxChoice* ChoiceProtocol;
    wxButton* Button_Ok;
    wxTextCtrl* TextCtrl_Description;
    wxStaticText* StaticText2;
    wxStaticText* StaticTextRate;
    wxChoice* ChoicePort;
    wxStaticText* StaticText1;
    wxStaticText* StaticText3;
    wxCheckBox* CheckBox_SuppressDuplicates;
    wxTextCtrl* TextCtrlLastChannel;
    wxButton* Button_Cancel;
    wxStaticText* StaticTextExplanation;
    wxChoice* ChoiceBaudRate;
    wxStaticText* StaticTextPort;
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
    static const long ID_STATICTEXT2;
    static const long ID_TEXTCTRL_DESCRIPTION;
    static const long ID_CHECKBOX1;
    static const long ID_BUTTON1;
    static const long ID_BUTTON2;
    //*)

private:

    //(*Handlers(SerialPortWithRate)
    void OnChoiceProtocolSelect(wxCommandEvent& event);
    void OnTextCtrl_DescriptionText(wxCommandEvent& event);
    void OnButton_OkClick(wxCommandEvent& event);
    void OnButton_CancelClick(wxCommandEvent& event);
    void OnChoicePortSelect(wxCommandEvent& event);
    void OnChoiceBaudRateSelect(wxCommandEvent& event);
    void OnTextCtrlLastChannelText(wxCommandEvent& event);
    //*)

    wxFlexGridSizer* MainSizer;
    DECLARE_EVENT_TABLE()
};

#endif

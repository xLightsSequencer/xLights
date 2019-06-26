#ifndef E131DIALOG_H
#define E131DIALOG_H

//(*Headers(E131Dialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class E131Output;
class OutputManager;

class E131Dialog: public wxDialog
{
    E131Output* _e131;
    OutputManager* _outputManager;
    void ValidateWindow();

public:

    E131Dialog(wxWindow* parent, E131Output* e131, OutputManager* outputManager, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~E131Dialog();

    //(*Declarations(E131Dialog)
    wxButton* Button_Cancel;
    wxButton* Button_Ok;
    wxButton* VisualizeButton;
    wxCheckBox* CheckBox_Auto_Channels;
    wxCheckBox* CheckBox_SuppressDuplicates;
    wxCheckBox* MultiE131CheckBox;
    wxChoice* ControllerChoice;
    wxRadioButton* RadioButtonMulticast;
    wxRadioButton* RadioButtonUnicast;
    wxSpinCtrl* SpinCtrl_LastChannel;
    wxSpinCtrl* SpinCtrl_NumUniv;
    wxSpinCtrl* SpinCtrl_Priority;
    wxSpinCtrl* SpinCtrl_StartUniv;
    wxStaticText* DescriptionStaticText;
    wxTextCtrl* FPPProxyIP;
    wxTextCtrl* TextCtrlIpAddr;
    wxTextCtrl* TextCtrl_Description;
    //*)

protected:

    //(*Identifiers(E131Dialog)
    static const long ID_STATICTEXT4;
    static const long ID_RADIOBUTTON1;
    static const long ID_RADIOBUTTON2;
    static const long ID_TEXTCTRL_IP_ADDR;
    static const long ID_SPINCTRL1;
    static const long ID_SPINCTRL2;
    static const long ID_CHECKBOX1;
    static const long ID_SPINCTRL_LAST_CHANNEL;
    static const long ID_TEXTCTRL_DESCRIPTION;
    static const long ID_CHECKBOX2;
    static const long ID_SPINCTRL_PRIORITY;
    static const long ID_CHOICE1;
    static const long ID_CHECKBOX_AUTO_CHANNELS;
    static const long ID_TEXTCTRL1;
    static const long ID_BUTTON1;
    static const long ID_BUTTON2;
    static const long ID_BUTTON3;
    //*)

private:

    //(*Handlers(E131Dialog)
    void OnRadioButtonUnicastSelect(wxCommandEvent& event);
    void OnRadioButtonMulticastSelect(wxCommandEvent& event);
    void OnSpinCtrl_NumUnivChange(wxSpinEvent& event);
    void OnTextCtrl_DescriptionText(wxCommandEvent& event);
    void OnTextCtrlIpAddrText(wxCommandEvent& event);
    void OnMultiE131CheckBoxClick(wxCommandEvent& event);
    void OnButton_OkClick(wxCommandEvent& event);
    void OnButton_CancelClick(wxCommandEvent& event);
    void OnVisualizeButtonClick(wxCommandEvent& event);
    void OnControllerChoiceSelect(wxCommandEvent& event);
    //*)

    void SaveFields();
    DECLARE_EVENT_TABLE()
};
#endif

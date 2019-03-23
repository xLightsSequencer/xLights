#ifndef ZCPPDIALOG_H
#define ZCPPDIALOG_H

//(*Headers(ZCPPDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class ZCPPOutput;
class OutputManager;
class ModelManager;

class ZCPPDialog: public wxDialog
{
    ZCPPOutput* _zcpp;
    OutputManager* _outputManager;
    ModelManager* _modelManager;
    void ValidateWindow();

public:

    ZCPPDialog(wxWindow* parent, ZCPPOutput* zcpp, OutputManager* outputManager, ModelManager* modelManager, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~ZCPPDialog();

    //(*Declarations(ZCPPDialog)
    wxButton* Button_Cancel;
    wxButton* Button_Ok;
    wxButton* Button_Visualise;
    wxCheckBox* CheckBoxAutoSizeOutput;
    wxCheckBox* CheckBox_Multicast;
    wxCheckBox* CheckBox_SupportSmartRemotes;
    wxCheckBox* CheckBox_SupportVirtualStrings;
    wxCheckBox* CheckBox_SuppressDuplicates;
    wxCheckBox* CheckBox_SuppressSendingConfig;
    wxSpinCtrl* SpinCtrl_Channels;
    wxStaticText* StaticText10;
    wxStaticText* StaticText1;
    wxStaticText* StaticText2;
    wxStaticText* StaticText3;
    wxStaticText* StaticText4;
    wxStaticText* StaticText5;
    wxStaticText* StaticText6;
    wxStaticText* StaticText7;
    wxStaticText* StaticText8;
    wxStaticText* StaticText9;
    wxTextCtrl* TextCtrlIpAddr;
    wxTextCtrl* TextCtrl_Description;
    //*)

protected:

    //(*Identifiers(ZCPPDialog)
    static const long ID_STATICTEXT4;
    static const long ID_STATICTEXT1;
    static const long ID_TEXTCTRL_IP_ADDR;
    static const long ID_STATICTEXT3;
    static const long ID_SPINCTRL2;
    static const long ID_STATICTEXT8;
    static const long ID_TEXTCTRL_DESCRIPTION;
    static const long ID_STATICTEXT9;
    static const long ID_CHECKBOX2;
    static const long ID_STATICTEXT2;
    static const long ID_CHECKBOX1;
    static const long ID_STATICTEXT5;
    static const long ID_CHECKBOX3;
    static const long ID_STATICTEXT6;
    static const long ID_CHECKBOX4;
    static const long ID_STATICTEXT7;
    static const long ID_CHECKBOX5;
    static const long ID_STATICTEXT10;
    static const long ID_CHECKBOX6;
    static const long ID_BUTTON3;
    static const long ID_BUTTON1;
    static const long ID_BUTTON2;
    //*)

private:

    //(*Handlers(ZCPPDialog)
    void OnRadioButtonUnicastSelect(wxCommandEvent& event);
    void OnRadioButtonMulticastSelect(wxCommandEvent& event);
    void OnSpinCtrl_NumUnivChange(wxSpinEvent& event);
    void OnTextCtrl_DescriptionText(wxCommandEvent& event);
    void OnTextCtrlIpAddrText(wxCommandEvent& event);
    void OnMultiE131CheckBoxClick(wxCommandEvent& event);
    void OnButton_OkClick(wxCommandEvent& event);
    void OnButton_CancelClick(wxCommandEvent& event);
    void OnSpinCtrl_ChannelsChange(wxSpinEvent& event);
    void OnButton_VisualiseClick(wxCommandEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};

#endif

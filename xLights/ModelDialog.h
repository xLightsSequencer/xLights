#ifndef MODELDIALOG_H
#define MODELDIALOG_H

//(*Headers(ModelDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/radiobut.h>
#include <wx/grid.h>
#include <wx/choice.h>
#include <wx/dialog.h>
//*)

class ModelDialog: public wxDialog
{
public:

    ModelDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
    virtual ~ModelDialog();
    void UpdateLabels();
    void UpdateStartChannels();

    //(*Declarations(ModelDialog)
    wxStaticText* StaticText9;
    wxSpinCtrl* SpinCtrl_parm2;
    wxSpinCtrl* SpinCtrl_parm1;
    wxStaticText* StaticText_Strings;
    wxTextCtrl* TextCtrl_Name;
    wxStaticText* StaticText2;
    wxStaticText* StaticText6;
    wxSpinCtrl* SpinCtrl_parm3;
    wxStaticText* StaticText8;
    wxStaticText* StaticText1;
    wxStaticText* StaticText3;
    wxRadioButton* RadioButton_TopLeft;
    wxRadioButton* RadioButton_BotLeft;
    wxCheckBox* CheckBox_MyDisplay;
    wxStaticText* StaticText5;
    wxStaticText* StaticText7;
    wxChoice* Choice_Order;
    wxCheckBox* cbIndividualStartNumbers;
    wxChoice* Choice_Antialias;
    wxGrid* gridStartChannels;
    wxChoice* Choice_DisplayAs;
    wxStaticText* StaticText4;
    wxRadioButton* RadioButton_BotRight;
    wxSpinCtrl* SpinCtrl_StartChannel;
    wxRadioButton* RadioButton_TopRight;
    wxStaticText* StaticText_Strands;
    wxStaticText* StaticText_Nodes;
    //*)

protected:

    //(*Identifiers(ModelDialog)
    static const long ID_STATICTEXT1;
    static const long ID_TEXTCTRL1;
    static const long ID_STATICTEXT5;
    static const long ID_CHOICE1;
    static const long ID_STATICTEXT2;
    static const long ID_SPINCTRL1;
    static const long ID_STATICTEXT3;
    static const long ID_SPINCTRL2;
    static const long ID_STATICTEXT4;
    static const long ID_SPINCTRL3;
    static const long ID_STATICTEXT6;
    static const long ID_SPINCTRL4;
    static const long ID_STATICTEXT7;
    static const long ID_CHOICE2;
    static const long ID_STATICTEXT8;
    static const long ID_RADIOBUTTON1;
    static const long ID_RADIOBUTTON2;
    static const long ID_STATICTEXT11;
    static const long ID_RADIOBUTTON4;
    static const long ID_RADIOBUTTON3;
    static const long ID_STATICTEXT9;
    static const long ID_CHOICE3;
    static const long ID_STATICTEXT10;
    static const long ID_CHECKBOX1;
    static const long ID_STATICTEXT12;
    static const long ID_CHECKBOX2;
    static const long ID_GRID1;
    //*)

private:

    //(*Handlers(ModelDialog)
    void OnChoice_DisplayAsSelect(wxCommandEvent& event);
    void OncbIndividualStartNumbersClick(wxCommandEvent& event);
    void OnSpinCtrl_parm1Change(wxSpinEvent& event);
    void OnSpinCtrl_parm2Change(wxSpinEvent& event);
    void OnSpinCtrl_StartChannelChange(wxSpinEvent& event);
    //*)
    void SetDefaultStartChannels();
    void UpdateRowCount();

    void SetReadOnly(bool);
    DECLARE_EVENT_TABLE()
};

#endif

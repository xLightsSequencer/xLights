#ifndef VIRTUALMATRIXDIALOG_H
#define VIRTUALMATRIXDIALOG_H

//(*Headers(VirtualMatrixDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class OutputManager;

class VirtualMatrixDialog : public wxDialog
{
    std::string& _name;
    int& _width;
    int& _height;
    bool& _topMost;
    bool& _useMatrixSize;
    int& _matrixMultiplier;
    std::string& _startChannel;
    wxSize& _size;
    wxPoint& _location;
    std::string& _rotation;
    std::string& _quality;
    OutputManager* _outputManager;
    wxSize _tempSize;
    wxPoint _tempLocation;

public:

    VirtualMatrixDialog(wxWindow* parent, OutputManager* outputManager, std::string& name, std::string& rotation, std::string& quality, wxSize& vmsize, wxPoint& vmlocation, int& width, int& height, bool& topMost, std::string& _startChannel, bool& useMatrixSize, int& matrixMultiplier, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~VirtualMatrixDialog();

    //(*Declarations(VirtualMatrixDialog)
    wxButton* Button_Ok;
    wxTextCtrl* TextCtrl_Name;
    wxStaticText* StaticText2;
    wxStaticText* StaticText6;
    wxStaticText* StaticText1;
    wxStaticText* StaticText3;
    wxSpinCtrl* SpinCtrl_Height;
    wxCheckBox* CheckBox_Topmost;
    wxStaticText* StaticText5;
    wxStaticText* StaticText7;
    wxButton* Button_Cancel;
    wxChoice* Choice_Quality;
    wxTextCtrl* TextCtrl_StartChannel;
    wxStaticText* StaticText4;
    wxButton* Button_Position;
    wxChoice* Choice_Rotation;
    wxSpinCtrl* SpinCtrl_Width;
    //*)

protected:

    //(*Identifiers(VirtualMatrixDialog)
    static const long ID_STATICTEXT7;
    static const long ID_TEXTCTRL1;
    static const long ID_STATICTEXT1;
    static const long ID_SPINCTRL1;
    static const long ID_STATICTEXT4;
    static const long ID_SPINCTRL2;
    static const long ID_STATICTEXT2;
    static const long ID_CHOICE1;
    static const long ID_STATICTEXT5;
    static const long ID_CHOICE2;
    static const long ID_STATICTEXT3;
    static const long ID_TEXTCTRL2;
    static const long ID_STATICTEXT6;
    static const long ID_BUTTON3;
    static const long ID_CHECKBOX1;
    static const long ID_BUTTON1;
    static const long ID_BUTTON2;
    //*)

private:

    //(*Handlers(VirtualMatrixDialog)
    void OnButton_OkClick(wxCommandEvent& event);
    void OnButton_CancelClick(wxCommandEvent& event);
    void OnButton_PositionClick(wxCommandEvent& event);
    void OnTextCtrl_StartChannelText(wxCommandEvent& event);
    //*)

    void ValidateWindow();

    DECLARE_EVENT_TABLE()
};

#endif

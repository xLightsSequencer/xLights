#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*Headers(VirtualMatrixDialog)
 #include <wx/button.h>
 #include <wx/checkbox.h>
 #include <wx/choice.h>
 #include <wx/dialog.h>
 #include <wx/sizer.h>
 #include <wx/spinctrl.h>
 #include <wx/stattext.h>
 #include <wx/textctrl.h>
 //*)

class OutputManager;
class ScheduleOptions;

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
    std::string& _pixelChannels;
    std::string& _quality;
    OutputManager* _outputManager;
    wxSize _tempSize;
    wxPoint _tempLocation;
    ScheduleOptions* _options = nullptr;

public:

    VirtualMatrixDialog(wxWindow* parent, OutputManager* outputManager, std::string& name, std::string& rotation, std::string& pixelChannels, std::string& quality, wxSize& vmsize, wxPoint& vmlocation, int& width, int& height, bool& topMost, std::string& _startChannel, bool& useMatrixSize, int& matrixMultiplier, ScheduleOptions* options, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~VirtualMatrixDialog();

    //(*Declarations(VirtualMatrixDialog)
    wxButton* Button_Cancel;
    wxButton* Button_Ok;
    wxButton* Button_Position;
    wxCheckBox* CheckBox_Topmost;
    wxChoice* Choice_PixelChannels;
    wxChoice* Choice_Quality;
    wxChoice* Choice_Rotation;
    wxSpinCtrl* SpinCtrl_Height;
    wxSpinCtrl* SpinCtrl_Width;
    wxStaticText* StaticText1;
    wxStaticText* StaticText2;
    wxStaticText* StaticText3;
    wxStaticText* StaticText4;
    wxStaticText* StaticText5;
    wxStaticText* StaticText6;
    wxStaticText* StaticText7;
    wxStaticText* StaticText9;
    wxTextCtrl* TextCtrl_Name;
    wxTextCtrl* TextCtrl_StartChannel;
    //*)

protected:

    //(*Identifiers(VirtualMatrixDialog)
    static const long ID_STATICTEXT7;
    static const long ID_TEXTCTRL1;
    static const long ID_STATICTEXT1;
    static const long ID_SPINCTRL1;
    static const long ID_STATICTEXT4;
    static const long ID_SPINCTRL2;
    static const long ID_STATICTEXT9;
    static const long ID_CHOICE4;
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

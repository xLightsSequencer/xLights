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

//(*Headers(ChannelLayoutDialog)
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class wxHtmlEasyPrinting;
class wxHtmlWindow;

class ChannelLayoutDialog: public wxDialog
{
public:

    ChannelLayoutDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~ChannelLayoutDialog();

    wxHtmlEasyPrinting* HtmlEasyPrint;
    void SetHtmlSource(wxString& html);

    //(*Declarations(ChannelLayoutDialog)
    wxButton* ButtonOpenInBrower;
    wxHtmlWindow* HtmlWindow1;
    //*)

protected:

    //(*Identifiers(ChannelLayoutDialog)
    static const long ID_BUTTON1;
    static const long ID_BUTTON_OPEN_IN_BROWSER;
    static const long ID_HTMLWINDOW1;
    //*)

private:

    //(*Handlers(ChannelLayoutDialog)
    void OnButton_PrintClick(wxCommandEvent& event);
    void OnButtonOpenInBrowerClick(wxCommandEvent& event);
    //*)

    wxString HtmlSource;

    DECLARE_EVENT_TABLE()
};

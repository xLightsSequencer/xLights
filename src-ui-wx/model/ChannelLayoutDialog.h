#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(ChannelLayoutDialog)
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/slider.h>
#include <wx/stattext.h>
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
    wxButton* ButtonFit;
    wxButton* ButtonOpenInBrower;
    wxHtmlWindow* HtmlWindow1;
    wxSlider* SliderZoom;
    wxStaticText* StaticTextZoom;
    //*)

protected:

    //(*Identifiers(ChannelLayoutDialog)
    static const long ID_BUTTON1;
    static const long ID_BUTTON_OPEN_IN_BROWSER;
    static const long ID_BUTTON_FIT;
    static const long ID_SLIDER_ZOOM;
    static const long ID_STATICTEXT_ZOOM;
    static const long ID_HTMLWINDOW1;
    //*)

private:

    //(*Handlers(ChannelLayoutDialog)
    void OnButton_PrintClick(wxCommandEvent& event);
    void OnButtonOpenInBrowerClick(wxCommandEvent& event);
    void OnButtonFitClick(wxCommandEvent& event);
    void OnSliderZoomCmdScroll(wxCommandEvent& event);
    //*)
    void OnHtmlMouseWheel(wxMouseEvent& event);
    void ApplyZoom(int zoomPercent);
    void ApplyFontSize(int fontSize);
    int ZoomToFontSize(int zoomPercent) const;
    int FontSizeToZoom(int fontSize) const;
    void FitNodeLayoutToWindow();
    wxString GetDisplayHtml() const;

    wxString HtmlSource;
    int _zoomPercent { 100 };
    int _fontSize { 0 };
    int _wheelRotation { 0 };

    DECLARE_EVENT_TABLE()
};

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

//(*Headers(TipOfTheDayDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/html/htmlwin.h>
#include <wx/sizer.h>
//*)

#include <wx/xml/xml.h>

//#define USE_WEBVIEW_FOR_TOD

class TipOfDayThread;
class TODTracker;
class xlCachedHtmlWindow;

#ifdef USE_WEBVIEW_FOR_TOD
class wxWebView;
#endif

class TipOfTheDayDialog: public wxDialog
{
    TipOfDayThread* _thread = nullptr;
    
    const std::string &GetTODXMLFile() const { return todFileLocation; }
    bool GetTODAtLevel(wxXmlDocument& doc, TODTracker& tracker, const std::string& level);
    std::string todFileLocation;
    
	public:

		TipOfTheDayDialog(const std::string& url = "", wxWindow* parent = nullptr, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~TipOfTheDayDialog();
        void SetTODXMLFile(const std::string &f) { todFileLocation = f; }
        bool DoTipOfDay(bool force);
        void PrepTipOfDay(wxWindow* notify);
        void ClearVisited();

		//(*Declarations(TipOfTheDayDialog)
		wxButton* Button_Next;
		wxButton* CloseButton;
		wxCheckBox* ShowTipsCheckbox;
		wxFlexGridSizer* FlexGridSizer1;
		xlCachedHtmlWindow* HtmlWindow1;
		//*)

    
#ifdef USE_WEBVIEW_FOR_TOD
        wxWebView* webView = nullptr;
#endif

	protected:

		//(*Identifiers(TipOfTheDayDialog)
		static const long ID_HTMLWINDOW1;
		static const long ID_ShowTips_CHECKBOX;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

		bool IsLevelGreaterOrEqualTo(const std::string& act, const std::string& min);
        std::string BuildURL(const std::string& url) const;
        void LoadURL(const std::string& url);

	private:

		//(*Handlers(TipOfTheDayDialog)
		void OnButton_NextClick(wxCommandEvent& event);
		void OnCloseButtonClick(wxCommandEvent& event);
		void OnShowTipsCheckboxClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

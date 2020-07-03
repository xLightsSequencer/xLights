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
 
//(*Headers(VendorMusicDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/hyperlink.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/treectrl.h>
//*)

#include <wx/xml/xml.h>
#include <wx/filename.h>
#include <list>
#include <string>
#include <wx/uri.h>
#include "CachedFileDownloader.h"
#include "VendorMusicHelpers.h"

class VendorMusicDialog: public wxDialog
{
    std::list<MSLVendor*> _vendors;
    std::string _showFolder;
    wxImage _vendorImage;

    wxXmlDocument* GetXMLFromURL(wxURI url, std::string& filename) const;
    bool LoadTree(std::string hash);
    bool AddSequenceLyric(wxTreeItemId v, MSLVendor* vendor, std::string hash);
    bool AddSequenceLyricInCategory(wxTreeItemId v, MSLVendor* vendor, std::string category);
    void ValidateWindow();
    void PopulateVendorPanel(MSLVendor* vendor);
    void PopulateSequenceLyricPanel(MSLSequenceLyric* msl);
    void LoadImage(wxStaticBitmap* sb, wxImage* img) const;
    void AddHierachy(wxTreeItemId v, MSLVendor* vendor, std::list<MSLVendorCategory*> categories);

	public:

		VendorMusicDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~VendorMusicDialog();
        bool DlgInit(std::string hash, std::string _showFolder);
        static CachedFileDownloader& GetCache();

		//(*Declarations(VendorMusicDialog)
		wxButton* Button_Download;
		wxButton* Button_Search;
		wxHyperlinkCtrl* HyperlinkCtrl_Facebook;
		wxHyperlinkCtrl* HyperlinkCtrl_MusicLink;
		wxHyperlinkCtrl* HyperlinkCtrl_VideoLink;
		wxHyperlinkCtrl* HyperlinkCtrl_WebLink;
		wxHyperlinkCtrl* HyperlinkCtrl_Website;
		wxNotebook* NotebookPanels;
		wxPanel* Panel1;
		wxPanel* Panel3;
		wxPanel* PanelVendor;
		wxPanel* Panel_Item;
		wxSplitterWindow* SplitterWindow1;
		wxStaticBitmap* StaticBitmap_VendorImage;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxTextCtrl* TextCtrl_ModelDetails;
		wxTextCtrl* TextCtrl_Search;
		wxTextCtrl* TextCtrl_VendorDetails;
		wxTreeCtrl* TreeCtrl_Navigator;
		//*)

	protected:

		//(*Identifiers(VendorMusicDialog)
		static const long ID_TREECTRL1;
		static const long ID_TEXTCTRL3;
		static const long ID_BUTTON2;
		static const long ID_PANEL3;
		static const long ID_STATICBITMAP1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT8;
		static const long ID_HYPERLINKCTRL4;
		static const long ID_STATICTEXT4;
		static const long ID_HYPERLINKCTRL2;
		static const long ID_PANEL2;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT1;
		static const long ID_HYPERLINKCTRL1;
		static const long ID_STATICTEXT7;
		static const long ID_HYPERLINKCTRL3;
		static const long ID_STATICTEXT2;
		static const long ID_HYPERLINKCTRL5;
		static const long ID_BUTTON1;
		static const long ID_PANEL4;
		static const long ID_NOTEBOOK1;
		static const long ID_PANEL1;
		static const long ID_SPLITTERWINDOW1;
		//*)

	private:

		//(*Handlers(VendorMusicDialog)
		void OnHyperlinkCtrl_ModelWebLinkClick(wxCommandEvent& event);
		void OnButton_InsertModelClick(wxCommandEvent& event);
		void OnNotebookPanelsPageChanged(wxNotebookEvent& event);
		void OnTreeCtrl_NavigatorItemActivated(wxTreeEvent& event);
		void OnTreeCtrl_NavigatorSelectionChanged(wxTreeEvent& event);
		void OnHyperlinkCtrl_WebsiteClick(wxCommandEvent& event);
		void OnHyperlinkCtrl_FacebookClick(wxCommandEvent& event);
		void OnClose(wxCloseEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnHyperlinkCtrl_MusicLinkClick(wxCommandEvent& event);
		void OnHyperlinkCtrl_VideoLinkClick(wxCommandEvent& event);
		void OnInit(wxInitDialogEvent& event);
		void OnTextCtrl_SearchText(wxCommandEvent& event);
		void OnButton_SearchClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

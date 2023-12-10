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

//(*Headers(ShaderDownloadDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/hyperlink.h>
#include <wx/listctrl.h>
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

class wxProgressDialog;
class MShader;

class ShaderDownloadDialog: public wxDialog
{
    std::list<MShader*> _shaders;
    std::string _shaderFile;
    int _currImage = -1;
    wxImage _shaderImage;

    wxXmlDocument* GetXMLFromURL(wxURI url, std::string& filename, wxProgressDialog* prog, int low, int high) const;
    bool LoadTree(wxProgressDialog* prog, int low = 0, int high = 100);
    void AddShaders(wxTreeItemId v, MShader* vendor);
    void ValidateWindow();
    void PopulateShaderPanel(MShader* vendor);
    void LoadShaderImage(std::list<wxFileName> imageFiles, int image);
    void LoadImage(wxStaticBitmap* sb, wxImage* img) const;
    void DoDownload(const wxTreeItemId& treeitem);

	public:

		ShaderDownloadDialog(wxWindow* parent, wxWindowID id=wxID_ANY, const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ShaderDownloadDialog();
        std::string GetShaderFile() const { return _shaderFile; }
        bool DlgInit(wxProgressDialog* prog, int low, int high);
        static CachedFileDownloader& GetCache() { return CachedFileDownloader::GetDefaultCache(); }

		//(*Declarations(ShaderDownloadDialog)
		wxButton* Button_Download;
		wxHyperlinkCtrl* HyperlinkCtrl_Website;
		wxListView* ListView_Sites;
		wxPanel* Panel1;
		wxPanel* Panel3;
		wxSplitterWindow* SplitterWindow1;
		wxStaticBitmap* StaticBitmap_VendorImage;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_Details;
		wxTreeCtrl* TreeCtrl_Navigator;
		//*)

	protected:

		//(*Identifiers(ShaderDownloadDialog)
		static const long ID_TREECTRL1;
		static const long ID_STATICTEXT1;
		static const long ID_LISTVIEW1;
		static const long ID_PANEL3;
		static const long ID_STATICBITMAP1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_HYPERLINKCTRL4;
		static const long ID_BUTTON4;
		static const long ID_PANEL1;
		static const long ID_SPLITTERWINDOW1;
		//*)

	private:

		//(*Handlers(ShaderDownloadDialog)
		void OnTreeCtrl_NavigatorItemActivated(wxTreeEvent& event);
		void OnTreeCtrl_NavigatorSelectionChanged(wxTreeEvent& event);
		void OnHyperlinkCtrl_WebsiteClick(wxCommandEvent& event);
		void OnClose(wxCloseEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnButton_DownloadClick(wxCommandEvent& event);
		void OnListView_SitesItemActivated(wxListEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

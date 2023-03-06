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
 
//(*Headers(VendorModelDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
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

class wxProgressDialog;
class MVendor;
class MModel;
class MModelWiring;
class MVendorCategory;

class VendorModelDialog: public wxDialog
{
    std::list<MVendor*> _vendors;
    std::string _modelFile;
	std::string _showFolder;
    int _currImage = -1;
    wxImage _vendorImage;
    wxImage _modelImage;
    static CachedFileDownloader _cache;

    wxXmlDocument* GetXMLFromURL(wxURI url, std::string& filename, wxProgressDialog* prog, int low, int high, bool keepProgress) const;
    bool LoadTree(wxProgressDialog* prog, int low = 0, int high = 100);
    void AddHierachy(wxTreeItemId v, MVendor* vendor, std::list<MVendorCategory*> categories);
    void AddModels(wxTreeItemId v, MVendor* vendor, std::string categoryId);
    void ValidateWindow();
    void PopulateVendorPanel(MVendor* vendor);
    void PopulateModelPanel(MModel* vendor);
    void PopulateModelPanel(MModelWiring* vendor);
    void LoadModelImage(std::list<wxFileName> imageFiles, int image);
    void LoadImage(wxStaticBitmap* sb, wxImage* img) const;
    bool DeleteEmptyCategories(wxTreeItemId& parent);
    bool IsVendorSuppressed(const std::string& vendor);
    void SuppressVendor(const std::string& vendor, bool suppress);
	void DownloadModel(MModelWiring* wiring);

	public:

		VendorModelDialog(wxWindow* parent, const std::string& showFolder, wxWindowID id=wxID_ANY, const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~VendorModelDialog();
        std::string GetModelFile() const { return _modelFile; }
        bool DlgInit(wxProgressDialog* prog, int low, int high);
        bool FindModelFile(const std::string &vendor, const std::string &model);
        static CachedFileDownloader& GetCache() { return _cache; }

		//(*Declarations(VendorModelDialog)
		wxButton* Button_InsertModel;
		wxButton* Button_Next;
		wxButton* Button_Prior;
		wxButton* Button_Search;
		wxCheckBox* CheckBox_DontDownload;
		wxHyperlinkCtrl* HyperlinkCtrl_Facebook;
		wxHyperlinkCtrl* HyperlinkCtrl_ModelWebLink;
		wxHyperlinkCtrl* HyperlinkCtrl_Website;
		wxNotebook* NotebookPanels;
		wxPanel* ItemImagePanel;
		wxPanel* Panel1;
		wxPanel* Panel3;
		wxPanel* PanelVendor;
		wxPanel* Panel_Item;
		wxSplitterWindow* SplitterWindow1;
		wxStaticBitmap* StaticBitmap_ModelImage;
		wxStaticBitmap* StaticBitmap_VendorImage;
		wxStaticText* StaticText2;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxTextCtrl* TextCtrl_ModelDetails;
		wxTextCtrl* TextCtrl_Search;
		wxTextCtrl* TextCtrl_VendorDetails;
		wxTreeCtrl* TreeCtrl_Navigator;
		//*)

	protected:

		//(*Identifiers(VendorModelDialog)
		static const long ID_TREECTRL1;
		static const long ID_TEXTCTRL3;
		static const long ID_BUTTON4;
		static const long ID_PANEL3;
		static const long ID_CHECKBOX1;
		static const long ID_STATICBITMAP1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT8;
		static const long ID_HYPERLINKCTRL4;
		static const long ID_STATICTEXT4;
		static const long ID_HYPERLINKCTRL2;
		static const long ID_PANEL2;
		static const long ID_BUTTON2;
		static const long ID_STATICBITMAP2;
		static const long ID_BUTTON3;
		static const long ID_PANEL5;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT7;
		static const long ID_HYPERLINKCTRL3;
		static const long ID_BUTTON1;
		static const long ID_PANEL4;
		static const long ID_NOTEBOOK1;
		static const long ID_PANEL1;
		static const long ID_SPLITTERWINDOW1;
		//*)

	private:

		//(*Handlers(VendorModelDialog)
		void OnHyperlinkCtrl_ModelWebLinkClick(wxCommandEvent& event);
		void OnButton_NextClick(wxCommandEvent& event);
		void OnButton_PriorClick(wxCommandEvent& event);
		void OnButton_InsertModelClick(wxCommandEvent& event);
		void OnNotebookPanelsPageChanged(wxNotebookEvent& event);
		void OnTreeCtrl_NavigatorItemActivated(wxTreeEvent& event);
		void OnTreeCtrl_NavigatorSelectionChanged(wxTreeEvent& event);
		void OnHyperlinkCtrl_eMailClick(wxCommandEvent& event);
		void OnHyperlinkCtrl_WebsiteClick(wxCommandEvent& event);
		void OnHyperlinkCtrl_FacebookClick(wxCommandEvent& event);
		void OnClose(wxCloseEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnCheckBox_DontDownloadClick(wxCommandEvent& event);
		void OnTextCtrl_SearchText(wxCommandEvent& event);
		void OnButton_SearchClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

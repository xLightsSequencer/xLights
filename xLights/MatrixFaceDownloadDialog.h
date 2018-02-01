#ifndef MATRIXFACEDOWNLOADDIALOG_H
#define MATRIXFACEDOWNLOADDIALOG_H

//(*Headers(MatrixFaceDownloadDialog)
#include <wx/treectrl.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/splitter.h>
#include <wx/panel.h>
#include <wx/hyperlink.h>
#include <wx/statbmp.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include <wx/xml/xml.h>
#include <wx/filename.h>
#include <list>
#include <string>
#include <wx/uri.h>
#include "CachedFileDownloader.h"

class MVendor;
class MModel;
class MModelWiring;
class MVendorCategory;

class MatrixFaceDownloadDialog: public wxDialog
{
    std::list<MFace*> _faces;
    std::string _faceFile;
    int _currImage = -1;
    wxImage _faceImage;
    static CachedFileDownloader _cache;

    wxXmlDocument* GetXMLFromURL(wxURI url, std::string& filename) const;
    bool LoadTree();
    void AddHierachy(wxTreeItemId v, std::list<MFaceCategory*> categories);
    void AddFaces(wxTreeItemId v, std::string categoryId);
    void ValidateWindow();
    void PopulateFacePanel(MFace* face);
    void LoadFaceImage(std::list<wxFileName> imageFiles, int image);
    void LoadImage(wxStaticBitmap* sb, wxImage* img) const;

	public:

		MatrixFaceDownloadDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MatrixFaceDownloadDialog();
        std::string GetFaceFile() const { return _faceFile; }
        bool DlgInit();
        static CachedFileDownloader& GetCache() { return _cache; }

		//(*Declarations(MatrixFaceDownloadDialog)
		wxTextCtrl* TextCtrl_ModelDetails;
		wxNotebook* NotebookPanels;
		wxHyperlinkCtrl* HyperlinkCtrl_Website;
		wxStaticBitmap* StaticBitmap_VendorImage;
		wxHyperlinkCtrl* HyperlinkCtrl_Facebook;
		wxTreeCtrl* TreeCtrl_Navigator;
		wxStaticText* StaticText2;
		wxPanel* PanelVendor;
		wxStaticText* StaticText6;
		wxPanel* ItemImagePanel;
		wxPanel* Panel1;
		wxButton* Button_Prior;
		wxStaticBitmap* StaticBitmap_ModelImage;
		wxPanel* Panel3;
		wxButton* Button_Next;
		wxStaticText* StaticText5;
		wxPanel* Panel_Item;
		wxTextCtrl* TextCtrl_VendorDetails;
		wxHyperlinkCtrl* HyperlinkCtrl_ModelWebLink;
		wxSplitterWindow* SplitterWindow1;
		wxButton* Button_InsertModel;
		//*)

	protected:

		//(*Identifiers(MatrixFaceDownloadDialog)
		static const long ID_TREECTRL1;
		static const long ID_PANEL3;
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

		//(*Handlers(MatrixFaceDownloadDialog)
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
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

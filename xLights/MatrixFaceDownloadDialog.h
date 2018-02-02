#ifndef MATRIXFACEDOWNLOADDIALOG_H
#define MATRIXFACEDOWNLOADDIALOG_H

//(*Headers(MatrixFaceDownloadDialog)
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/splitter.h>
#include <wx/panel.h>
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

class MFace;
class MFaceCategory;

class MatrixFaceDownloadDialog: public wxDialog
{
    std::list<MFaceCategory*> _categories;
    std::list<MFace*> _faces;
    std::string _faceFile;
    int _currImage = -1;
    wxImage _faceImage;
    static CachedFileDownloader _cache;
    int _width;
    int _height;

    wxXmlDocument* GetXMLFromURL(wxURI url, std::string& filename) const;
    bool LoadTree();
    void AddHierachy(wxTreeItemId v, std::list<MFaceCategory*> categories);
    void AddFaces(wxTreeItemId v, std::string categoryId);
    void ValidateWindow();
    void PopulateFacePanel(MFace* face);
    void LoadFaceImage(wxFileName imageFile);
    void LoadImage(wxStaticBitmap* sb, wxImage* img) const;
    std::list<MFace*> GetFaces(const std::string& categoryId);

	public:

		MatrixFaceDownloadDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MatrixFaceDownloadDialog();
        std::string GetFaceFile() const { return _faceFile; }
        bool DlgInit(int width, int height);
        static CachedFileDownloader& GetCache() { return _cache; }

		//(*Declarations(MatrixFaceDownloadDialog)
		wxButton* Button_InsertFace;
		wxTreeCtrl* TreeCtrl_Navigator;
		wxPanel* Panel1;
		wxPanel* Panel3;
		wxTextCtrl* TextCtrl_FaceDetails;
		wxSplitterWindow* SplitterWindow1;
		wxStaticBitmap* StaticBitmap_FaceImage;
		wxCheckBox* CheckBox_FilterUnsuitable;
		//*)

	protected:

		//(*Identifiers(MatrixFaceDownloadDialog)
		static const long ID_TREECTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_PANEL3;
		static const long ID_STATICBITMAP1;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		static const long ID_PANEL1;
		static const long ID_SPLITTERWINDOW1;
		//*)

	private:

		//(*Handlers(MatrixFaceDownloadDialog)
		void OnTreeCtrl_NavigatorItemActivated(wxTreeEvent& event);
		void OnTreeCtrl_NavigatorSelectionChanged(wxTreeEvent& event);
		void OnClose(wxCloseEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnButton_InsertFaceClick(wxCommandEvent& event);
		void OnCheckBox_FilterUnsuitableClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

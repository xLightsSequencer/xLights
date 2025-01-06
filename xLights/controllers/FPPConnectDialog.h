#ifndef FPPCONNECTDIALOG_H
#define FPPCONNECTDIALOG_H

#include <wx/progdlg.h>
#include <list>

//(*Headers(FPPConnectDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
//*)

#include <wx/treelist.h>
#include <wx/dataview.h>
#include "FPP.h"

class OutputManager;
class wxProgressDialog;


class FPPConnectDialog: public wxDialog
{
    void SaveSettings(bool onlyInsts = false);
    void ApplySavedHostSettings();
    wxString Fixitup(wxString val);

	public:

		FPPConnectDialog(wxWindow* parent, OutputManager* outputManager, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~FPPConnectDialog();

		//(*Declarations(FPPConnectDialog)
		wxButton* AddFPPButton;
		wxButton* Button_Upload;
		wxButton* ReDiscover;
		wxCheckBox* KeepWinOpen;
		wxChoice* ChoiceFilter;
		wxChoice* ChoiceFolder;
		wxFlexGridSizer* FPPInstanceSizer;
		wxPanel* CheckListBoxHolder;
		wxPanel* Panel1;
		wxScrolledWindow* FPPInstanceList;
		wxSplitterWindow* SplitterWindow1;
		wxStaticText* Selected_Label;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		//*)

        wxTreeListCtrl* CheckListBox_Sequences;

	protected:

		//(*Identifiers(FPPConnectDialog)
		static const wxWindowID ID_SCROLLEDWINDOW1;
		static const wxWindowID ID_STATICTEXT1;
		static const wxWindowID ID_CHOICE_FILTER;
		static const wxWindowID ID_STATICTEXT2;
		static const wxWindowID ID_CHOICE_FOLDER;
		static const wxWindowID ID_STATICTEXT3;
		static const wxWindowID ID_PANEL2;
		static const wxWindowID ID_PANEL1;
		static const wxWindowID ID_SPLITTERWINDOW1;
		static const wxWindowID ID_BUTTON1;
		static const wxWindowID ID_BUTTON2;
		static const wxWindowID ID_CHECKBOX1;
		static const wxWindowID ID_BUTTON_Upload;
		//*)



    
        std::list<FPP*> instances;
        OutputManager* _outputManager;

	private:

		//(*Handlers(FPPConnectDialog)
		void OnButton_UploadClick(wxCommandEvent& event);
		void OnClose(wxCloseEvent& event);
		void SequenceListPopup(wxTreeListEvent& event);
		void OnAddFPPButtonClick(wxCommandEvent& event);
        void OnFPPReDiscoverClick(wxCommandEvent& event);
		void OnChoiceFolderSelect(wxCommandEvent& event);
		void OnChoiceFilterSelect(wxCommandEvent& event);
		void HostSortMenu(wxContextMenuEvent& event);
        void IPSortMenu(wxContextMenuEvent& event);
		void OnHostSortClick(wxCommandEvent& event);
        void OnIPSortClick(wxCommandEvent& event);
		void UploadPopupMenu(wxContextMenuEvent& event);
		void OnUploadPopupClick(wxCommandEvent& event);
		void CapePopupMenu(wxContextMenuEvent& event);
		void OnCapePopupClick(wxCommandEvent& event);
        void MediaPopupMenu(wxContextMenuEvent& event);
        void OnMediaPopupClick(wxCommandEvent& event);
		//*)

        void LoadSequencesFromFolder(wxString dir) const;
        void LoadSequences();
        void PopulateFPPInstanceList(wxProgressDialog *prgs = nullptr);
        void AddInstanceRow(const FPP &inst);
        wxPanel *AddInstanceHeader(const std::string &h, const std::string &tt = std::string());

        void GetFolderList(const wxString& folder);
    
        void OnPopup(wxCommandEvent &event);
    
        bool GetCheckValue(const std::string &col);
        std::string GetChoiceValue(const std::string &col);
        int GetChoiceValueIndex(const std::string &col);
    
        void SetChoiceValueIndex(const std::string &col, int i);
        void SetCheckValue(const std::string &col, bool b);

		void DisplayDateModified(const wxString& filePath, wxTreeListItem &index) const;

		void UpdateSeqCount();
        uint32_t GetSelectedSeqCount();
        void OnSequenceListToggled(wxDataViewEvent& event);
    
        void doUpload(FPPUploadProgressDialog *prgs, std::vector<bool> doUpload);
        std::vector<int> SplitIP(const wxString& ip) const; 

        void SelectBatchRenderSeq();
        void SelectIPsWithSubnet();

		DECLARE_EVENT_TABLE()
};

#endif

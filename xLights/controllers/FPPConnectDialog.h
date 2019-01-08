#ifndef FPPCONNECTDIALOG_H
#define FPPCONNECTDIALOG_H

#include <wx/progdlg.h>
#include <list>

//(*Headers(FPPConnectDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
//*)

#include <wx/dataview.h>
#include "FPP.h"

class OutputManager;


class FPPConnectDialog: public wxDialog
{
    void SaveSettings();
    void ApplySavedHostSettings();

	public:

		FPPConnectDialog(wxWindow* parent, OutputManager* outputManager, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~FPPConnectDialog();

		//(*Declarations(FPPConnectDialog)
		wxButton* AddFPPButton;
		wxButton* Button_Upload;
		wxListView* CheckListBox_Sequences;
		//*)

        wxDataViewListCtrl *FPPInstanceList;

	protected:

		//(*Identifiers(FPPConnectDialog)
		static const long ID_LISTVIEW_Sequences;
		static const long ID_BUTTON1;
		static const long ID_BUTTON_Upload;
		//*)

        static const long ID_MNU_SELECTALL;
        static const long ID_MNU_SELECTNONE;
        static const long ID_FPP_INSTANCE_LIST;

    
        std::list<FPP> instances;
        OutputManager* _outputManager;

	private:

		//(*Handlers(FPPConnectDialog)
		void OnButton_UploadClick(wxCommandEvent& event);
		void OnClose(wxCloseEvent& event);
		void OnFPPInstanceItemValueChanged(wxDataViewEvent& event);
		void SequenceListPopup(wxListEvent& event);
		void OnAddFPPButtonClick(wxCommandEvent& event);
		//*)

        void CreateDriveList();
        void LoadSequencesFromFolder(wxString dir) const;
        void LoadSequences();
        void PopulateFPPInstanceList();

    
        void OnPopup(wxCommandEvent &event);

		DECLARE_EVENT_TABLE()
};

#endif

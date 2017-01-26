#ifndef OUTPUTPROCESSINGDIALOG_H
#define OUTPUTPROCESSINGDIALOG_H

//(*Headers(OutputProcessingDialog)
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include <list>

class OutputProcess;

class OutputProcessingDialog: public wxDialog
{
    std::list<OutputProcess*>* _op;
    void ValidateWindow();
    void DeleteSelectedItem();
    bool EditSelectedItem();

	public:

		OutputProcessingDialog(wxWindow* parent, std::list<OutputProcess*>* op, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~OutputProcessingDialog();

		//(*Declarations(OutputProcessingDialog)
		wxButton* Button_Dim;
		wxButton* Button_Ok;
		wxListView* ListView_Processes;
		wxButton* Button_DimWhite;
		wxButton* Button_Delete;
		wxButton* Button_Cancel;
		wxButton* Button_Edit;
		wxButton* Button_AddSet;
		wxButton* Button_AddRemap;
		//*)

	protected:

		//(*Identifiers(OutputProcessingDialog)
		static const long ID_LISTVIEW1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON6;
		static const long ID_BUTTON7;
		static const long ID_BUTTON8;
		static const long ID_BUTTON5;
		static const long ID_BUTTON4;
		//*)

	private:

		//(*Handlers(OutputProcessingDialog)
		void OnListView_ProcessesBeginDrag(wxListEvent& event);
		void OnListView_ProcessesItemSelect(wxListEvent& event);
		void OnListView_ProcessesItemActivated(wxListEvent& event);
		void OnListView_ProcessesKeyDown(wxListEvent& event);
		void OnButton_EditClick(wxCommandEvent& event);
		void OnButton_DeleteClick(wxCommandEvent& event);
		void OnButton_AddRemapClick(wxCommandEvent& event);
		void OnButton_AddSetClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_DimWhiteClick(wxCommandEvent& event);
		void OnButton_AddDimClick(wxCommandEvent& event);
		void OnButton_AddDimWhiteClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

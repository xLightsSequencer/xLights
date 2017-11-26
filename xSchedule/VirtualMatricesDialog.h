#ifndef VIRTUALMATRICESDIALOG_H
#define VIRTUALMATRICESDIALOG_H

//(*Headers(VirtualMatricesDialog)
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include <list>

class VirtualMatrix;
class OutputManager;

class VirtualMatricesDialog: public wxDialog
{
    std::list<VirtualMatrix*>* _vmatrices;
    OutputManager* _outputManager;
    void ValidateWindow();
    void DoAdd();
    void DoDelete();
    void DoEdit();
    void PopulateList();

	public:

		VirtualMatricesDialog(wxWindow* parent, OutputManager* outputManager, std::list<VirtualMatrix*>* vmatrices, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~VirtualMatricesDialog();

		//(*Declarations(VirtualMatricesDialog)
		wxButton* Button_Ok;
		wxButton* Button_Delete;
		wxButton* Button_Cancel;
		wxListView* ListView1;
		wxButton* Button_Edit;
		wxButton* Button_Add;
		//*)

	protected:

		//(*Identifiers(VirtualMatricesDialog)
		static const long ID_LISTVIEW1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON5;
		//*)

	private:

		//(*Handlers(VirtualMatricesDialog)
		void OnListView1ItemSelect(wxListEvent& event);
		void OnListView1ItemActivated(wxListEvent& event);
		void OnListView1KeyDown(wxListEvent& event);
		void OnButton_AddClick(wxCommandEvent& event);
		void OnButton_EditClick(wxCommandEvent& event);
		void OnButton_DeleteClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

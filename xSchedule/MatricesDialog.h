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

 //(*Headers(MatricesDialog)
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include <list>

class MatrixMapper;
class OutputManager;

class MatricesDialog: public wxDialog
{
    OutputManager* _outputManager;
    std::list<MatrixMapper*>* _matrices;
    void ValidateWindow();
    void DoAdd();
    void DoDelete();
    void DoEdit();
    void PopulateList();

	public:

		MatricesDialog(wxWindow* parent, OutputManager* outputManager, std::list<MatrixMapper*>* matrices, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MatricesDialog();

		//(*Declarations(MatricesDialog)
		wxButton* Button_Ok;
		wxButton* Button_Delete;
		wxButton* Button_Cancel;
		wxListView* ListView1;
		wxButton* Button_Edit;
		wxButton* Button_Add;
		//*)

	protected:

		//(*Identifiers(MatricesDialog)
		static const long ID_LISTVIEW1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON5;
		//*)

	private:

		//(*Handlers(MatricesDialog)
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

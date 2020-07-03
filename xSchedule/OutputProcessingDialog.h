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

 //(*Headers(OutputProcessingDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
//*)

#include <list>

class OutputProcess;
class OutputManager;

class OutputProcessingDialog: public wxDialog
{
    OutputManager* _outputManager;
    std::list<OutputProcess*>* _op;
    void ValidateWindow();
    void DeleteSelectedItem();
    bool EditSelectedItem();
    bool _dragging;
    void OnDragEnd(wxMouseEvent& event);
    void OnDragQuit(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);

	public:

		OutputProcessingDialog(wxWindow* parent, OutputManager* outputManager, std::list<OutputProcess*>* op, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~OutputProcessingDialog();

		//(*Declarations(OutputProcessingDialog)
		wxButton* Button_3to4;
		wxButton* Button_AddDeadChannel;
		wxButton* Button_AddRemap;
		wxButton* Button_AddSet;
		wxButton* Button_Cancel;
		wxButton* Button_ColourOrder;
		wxButton* Button_Delete;
		wxButton* Button_Dim;
		wxButton* Button_DimWhite;
		wxButton* Button_Edit;
		wxButton* Button_Gamma;
		wxButton* Button_Ok;
		wxButton* Button_Reverse;
		wxButton* Button_Sustain;
		wxListView* ListView_Processes;
		//*)

	protected:

		//(*Identifiers(OutputProcessingDialog)
		static const long ID_LISTVIEW1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON6;
		static const long ID_BUTTON12;
		static const long ID_BUTTON7;
		static const long ID_BUTTON8;
		static const long ID_BUTTON9;
		static const long ID_BUTTON10;
		static const long ID_BUTTON11;
		static const long ID_BUTTON13;
		static const long ID_BUTTON14;
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
		void OnButton_ColourOrderClick(wxCommandEvent& event);
		void OnButton_ReverseClick(wxCommandEvent& event);
		void OnButton_GammaClick(wxCommandEvent& event);
		void OnButton_AddDeadChannelClick(wxCommandEvent& event);
		void OnListView_ProcessesItemRClick(wxListEvent& event);
		void OnButton_3to4Click(wxCommandEvent& event);
		void OnButton_SustainClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};


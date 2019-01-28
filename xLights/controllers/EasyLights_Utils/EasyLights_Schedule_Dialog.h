#ifndef EASYLIGHTS_SCHEDULE_DIALOG_H
#define EASYLIGHTS_SCHEDULE_DIALOG_H

//(*Headers(EasyLights_Schedule_Dialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class EasyLights_Schedule_Dialog: public wxDialog
{
	public:

		EasyLights_Schedule_Dialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EasyLights_Schedule_Dialog();

		//(*Declarations(EasyLights_Schedule_Dialog)
		wxButton* Button1;
		wxButton* Button2;
		wxButton* Button3;
		wxButton* Button4;
		wxButton* Button5;
		wxButton* Button6;
		wxButton* Button7;
		wxGrid* Grid1;
		wxListCtrl* ListCtrl1;
		wxListCtrl* ListCtrl2;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxTextCtrl* TextCtrl2;
		//*)

		void Display_PlayLists();
		void Display_DOW();
		void Load_Schedule_to_Work_on();
		void Save_Work_on_Schedule();

		int SchedWork_Active;
		int Grid_Left_Row;
		int Grid_Left_Col;

		
		std::string Sched_Work;
		std::string Selected_List1_PlayList;

	protected:

		//(*Identifiers(EasyLights_Schedule_Dialog)
		static const long ID_STATICTEXT1;
		static const long ID_LISTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_LISTCTRL2;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL2;
		static const long ID_GRID1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON5;
		static const long ID_BUTTON6;
		static const long ID_BUTTON7;
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT4;
		static const long ID_STATICTEXT6;
		static const long ID_STATICTEXT7;
		//*)

	private:

		//(*Handlers(EasyLights_Schedule_Dialog)
		void InitDialog(wxInitDialogEvent &event);
		void OnButtonCloseClick(wxCommandEvent& event);
		void OnButton_Edit_PlayList(wxCommandEvent& event);
		void OnButton_NoSchedule(wxCommandEvent& event);
		void OnButton_Copy_Schedule(wxCommandEvent& event);
		void OnButton_Remove_Schedule(wxCommandEvent& event);
		void OnButton_Save_Schedule(wxCommandEvent& event);
		void OnButton_Add_End_Schedule(wxCommandEvent& event);
		void OnButton_Sanity_Check(wxCommandEvent& event);

		void OnList1_Seq_Selected(wxListEvent& event);
		void OnGridCellLeftClick(wxGridEvent &event);
		void OnGridCellRightClick(wxGridEvent &event);

		//*)

	protected:

		void BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size);

		DECLARE_EVENT_TABLE()
};

#endif

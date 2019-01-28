#ifndef EASYLIGHTS_PLAYLIST_DIALOG_H
#define EASYLIGHTS_PLAYLIST_DIALOG_H

//(*Headers(EasyLights_PlayList_Dialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)


class EasyLights_PlayList_Dialog: public wxDialog
{
	public:

		EasyLights_PlayList_Dialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EasyLights_PlayList_Dialog();

		//(*Declarations(EasyLights_PlayList_Dialog)
		wxButton* Button1;
		wxButton* Button2;
		wxButton* Button3;
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
		wxTextCtrl* TextCtrl1;
		wxTextCtrl* TextCtrl2;
		//*)

		void Display_Sequences();
		void Display_PlayLists();
		//bool TransferDataFromWindow();
		std::vector<std::string> Seq_Array;


		std::string PLwork;
		std::string Selected_List1_Sequence;

		int PLWork_Active;
		int Grid_Left_Row;
		int Grid_Left_Col;

		void Load_PlayList_to_Work_on();
		void Save_Work_on_Play_List();
		void Add_PlayList_to_Display_if_New(std::string PL);


	protected:

		//(*Identifiers(EasyLights_PlayList_Dialog)
		static const long ID_STATICTEXT1;
		static const long ID_LISTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_LISTCTRL2;
		static const long ID_STATICTEXT4;
		static const long ID_BUTTON7;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL2;
		static const long ID_GRID1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON5;
		static const long ID_BUTTON6;
		static const long ID_STATICTEXT5;
		//*)

	private:

		//(*Handlers(EasyLights_PlayList_Dialog)
		void InitDialog(wxInitDialogEvent &event);
		void OnButtonCloseClick(wxCommandEvent& event);
		void OnButton_New_PlayList(wxCommandEvent& event);
		void OnButton_Edit_PlayList(wxCommandEvent& event);
		void OnButton_Delete_PlayList(wxCommandEvent& event);
		void OnButton_Remove_PlayList(wxCommandEvent& event);
		void OnButton_Save_PlayList(wxCommandEvent& event);
		void OnButton_Add_End_PlayList(wxCommandEvent& event);
		void OnList1_Seq_Selected(wxListEvent& event);
		void OnGridCellLeftClick(wxGridEvent &event);
		void OnGridCellRightClick(wxGridEvent &event);

		
		

		//*)

	protected:

		void BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size);

		DECLARE_EVENT_TABLE()
};

#endif

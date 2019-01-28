#ifndef EASYLIGHTS_FSEQ_DIALOG_H
#define EASYLIGHTS_FSEQ_DIALOG_H

//(*Headers(EasyLights_FSEQ_Dialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class EasyLights_EndPoint;

class EL_FSEQ_Info_Xlights
{
public:
	std::string FN_long;
	std::string FN_short;
	wxDateTime FN_DT;
	wxLongLong FN_DT_Value;
	int Version;
};

class EL_FSEQ_Info
{
public:
	EL_FSEQ_Info_Xlights *IX;
	EasyLights_EndPoint *EP;
	std::string ELC_IP;
	std::string ELC_Name;
	std::string ELC_FN_long;
	std::string ELC_FN_short;
	wxDateTime ELC_FN_DT;
	wxLongLong ELC_FN_DT_Value;
	int Process;
	int Grid_Row;
};


class EasyLights_FSEQ_Dialog: public wxDialog
{
	public:

		EasyLights_FSEQ_Dialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EasyLights_FSEQ_Dialog();

		//(*Declarations(EasyLights_FSEQ_Dialog)
		wxButton* Button1;
		wxButton* Button2;
		wxButton* Button3;
		wxButton* Button4;
		wxGrid* Grid1;
		wxListCtrl* ListCtrl1;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl1;
		//*)

	protected:

		//(*Identifiers(EasyLights_FSEQ_Dialog)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_GRID1;
		static const long ID_LISTCTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(EasyLights_FSEQ_Dialog)
		void OnInit(wxInitDialogEvent& event);
		//*)

		void OnGridCellLeftClick(wxGridEvent &event); 
		void InitDialog(wxInitDialogEvent &event);

	protected:

		void BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size);

		DECLARE_EVENT_TABLE()




public:

	void Get_All_Xlights_FSEQ_Files();

	std::vector< EL_FSEQ_Info*> FSEQ_Array;
	std::vector< EL_FSEQ_Info_Xlights*> FSEQ_Array_Xlights;

	void Write_Msg_to_Display(char *S);
	void Write_Msg_to_Display(std::string &S);
	void Write_Msg_to_Display(wxString &S);




	void OnButtonCloseClick(wxCommandEvent& event);
	void OnButton_Fseq_Selected(wxCommandEvent& event);
	void OnButton_Fseq_All(wxCommandEvent& event);
	void OnButton_Fseq_Required(wxCommandEvent& event);

	void Set_Busy_Status(int Busy);

	int Quit_Now;
	int FSEQ_Busy;
	int Button_Active;
	int Process_Selected_Active;

};

#endif

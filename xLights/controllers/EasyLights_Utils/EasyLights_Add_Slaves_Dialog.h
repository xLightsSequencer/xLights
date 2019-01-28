#ifndef EASYLIGHTS_ADD_SLAVES_DIALOG_H
#define EASYLIGHTS_ADD_SLAVES_DIALOG_H

//(*Headers(EasyLights_Add_Slaves_Dialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class EasyLights_EndPoint;
class EL_Controller_Info;


class EasyLights_Add_Slaves_Dialog: public wxDialog
{
	public:

		EasyLights_Add_Slaves_Dialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EasyLights_Add_Slaves_Dialog();

		//(*Declarations(EasyLights_Add_Slaves_Dialog)
		wxButton* Button1;
		wxButton* Button2;
		wxButton* Button3;
		wxButton* Button4;
		wxListCtrl* ListCtrl1;
		wxStaticText* StaticText10;
		wxStaticText* StaticText11;
		wxStaticText* StaticText12;
		wxStaticText* StaticText13;
		wxStaticText* StaticText14;
		wxStaticText* StaticText15;
		wxStaticText* StaticText16;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		wxTextCtrl* TextCtrl10;
		wxTextCtrl* TextCtrl11;
		wxTextCtrl* TextCtrl12;
		wxTextCtrl* TextCtrl1;
		wxTextCtrl* TextCtrl2;
		wxTextCtrl* TextCtrl3;
		wxTextCtrl* TextCtrl4;
		wxTextCtrl* TextCtrl5;
		wxTextCtrl* TextCtrl6;
		wxTextCtrl* TextCtrl7;
		wxTextCtrl* TextCtrl8;
		wxTextCtrl* TextCtrl9;
		//*)

	protected:

		//(*Identifiers(EasyLights_Add_Slaves_Dialog)
		static const long ID_STATICTEXT1;
		static const long ID_LISTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT6;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT7;
		static const long ID_TEXTCTRL5;
		static const long ID_STATICTEXT16;
		static const long ID_TEXTCTRL12;
		static const long ID_STATICTEXT8;
		static const long ID_STATICTEXT9;
		static const long ID_TEXTCTRL6;
		static const long ID_STATICTEXT12;
		static const long ID_TEXTCTRL8;
		static const long ID_STATICTEXT14;
		static const long ID_TEXTCTRL10;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_STATICTEXT10;
		static const long ID_STATICTEXT11;
		static const long ID_TEXTCTRL7;
		static const long ID_STATICTEXT13;
		static const long ID_TEXTCTRL9;
		static const long ID_STATICTEXT15;
		static const long ID_TEXTCTRL11;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		//*)

	private:

		//(*Handlers(EasyLights_Add_Slaves_Dialog)

		void InitDialog(wxInitDialogEvent &event);
		//*)

	protected:

		void BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size);

		DECLARE_EVENT_TABLE()

public:

	bool TransferDataToWindow();
	bool TransferDataFromWindow();

	void OnButton_S1_Add(wxCommandEvent& event);
	void OnButton_S1_Remove(wxCommandEvent& event);
	void OnButton_S2_Add(wxCommandEvent& event);
	void OnButton_S2_Remove(wxCommandEvent& event);
	void OnButtonCloseClick(wxCommandEvent& event);


	EasyLights_EndPoint *Selected_EndPoint;
	EasyLights_EndPoint *EL_EndPoint;
	EasyLights_EndPoint *S1_EndPoint;
	EasyLights_EndPoint *S2_EndPoint;
	//int Selected_EndPoint_Index;
	int S1_Selected_EndPoint_Index;
	int S2_Selected_EndPoint_Index;
	int Dirty;


	int Channels_Avail_To_Slave;

	void Load_S_List(int Ignore_Slaves_Attached);

};

#endif

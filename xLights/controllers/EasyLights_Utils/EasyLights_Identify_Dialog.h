#ifndef EASYLIGHTS_IDENTIFY_DIALOG_H
#define EASYLIGHTS_IDENTIFY_DIALOG_H

//(*Headers(EasyLights_Identify_Dialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class EasyLights_EndPoint;
class EL_Controller_Info;

class EasyLights_Identify_Dialog: public wxDialog
{
	public:

		EasyLights_Identify_Dialog(wxWindow* parent, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EasyLights_Identify_Dialog();

		//(*Declarations(EasyLights_Identify_Dialog)
		wxButton* Button1;
		wxButton* Button2;
		wxButton* Button3;
		wxCheckBox* CheckBox1;
		wxCheckBox* CheckBox2;
		wxChoice* Choice2;
		wxListCtrl* ListCtrl1;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText5;
		wxTextCtrl* TextCtrl1;
		wxTextCtrl* TextCtrl2;
		//*)

	protected:

		//(*Identifiers(EasyLights_Identify_Dialog)
		static const long ID_STATICTEXT1;
		static const long ID_LISTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE2;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		//*)

	private:

		//(*Handlers(EasyLights_Identify_Dialog)

		void OnButton_Cancel(wxCommandEvent& event);
		void OnButton_Edit(wxCommandEvent& event);
		void OnButton_Apply(wxCommandEvent& event);
		bool TransferDataToWindow();
		bool TransferDataFromWindow();
		void InitDialog(wxInitDialogEvent &event);

		//*)

	protected:

		void BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size);

		DECLARE_EVENT_TABLE()

public:

	void Load_Combo();
	void Load_I_List();

	EasyLights_EndPoint *Selected_EndPoint;


	int Selected_EndPoint_Index;
	int Return_Model_Selection_Index(std::string &M);
	void Set_EndPoint_Model_String_From_Choice_Index(int I);
	void Update_Selected_List_Item();

	int Dirty;

};

#endif

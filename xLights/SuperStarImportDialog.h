#ifndef SUPERSTARIMPORTDIALOG_H
#define SUPERSTARIMPORTDIALOG_H

//(*Headers(SuperStarImportDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/dialog.h>
//*)

class SuperStarImportDialog: public wxDialog
{
	public:

		SuperStarImportDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SuperStarImportDialog();

		//(*Declarations(SuperStarImportDialog)
		wxCheckBox* CheckBox_SS_FlipY;
		wxTextCtrl* TextCtrl_SS_Y_Size;
		wxStaticText* StaticText37;
		wxStaticText* StaticText40;
		wxStaticText* StaticText42;
		wxTextCtrl* TextCtrl_SS_X_Size;
		wxStaticText* StaticText44;
		wxStaticText* StaticText39;
		wxChoice* ChoiceSuperStarImportModel;
		wxTextCtrl* TextCtrl_SS_X_Offset;
		wxStaticText* StaticText41;
		wxTextCtrl* TextCtrl_SS_Y_Offset;
		//*)

	protected:

		//(*Identifiers(SuperStarImportDialog)
		static const long ID_CHOICE_SuperStarImportModel;
		static const long ID_STATICTEXT31;
		static const long ID_STATICTEXT39;
		static const long ID_TEXTCTRL_SS_X_Offset;
		static const long ID_STATICTEXT24;
		static const long ID_STATICTEXT40;
		static const long ID_TEXTCTRL_SS_Y_Offset;
		static const long ID_STATICTEXT42;
		static const long ID_TEXTCTRL_SS_X_Size;
		static const long ID_STATICTEXT41;
		static const long ID_TEXTCTRL_SS_Y_Size;
		static const long ID_CHECKBOX_SS_FlipY;
		//*)

	private:

		//(*Handlers(SuperStarImportDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

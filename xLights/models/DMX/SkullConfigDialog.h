#ifndef SKULLCONFIGDIALOG_H
#define SKULLCONFIGDIALOG_H

//(*Headers(SkullConfigDialog)
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
//*)

class SkullConfigDialog: public wxDialog
{
	public:

		SkullConfigDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SkullConfigDialog();

		//(*Declarations(SkullConfigDialog)
		wxCheckBox* CheckBox_16bits;
		wxCheckBox* CheckBox_Color;
		wxCheckBox* CheckBox_EyeLR;
		wxCheckBox* CheckBox_EyeUD;
		wxCheckBox* CheckBox_Jaw;
		wxCheckBox* CheckBox_Nod;
		wxCheckBox* CheckBox_Pan;
		wxCheckBox* CheckBox_Skulltronix;
		wxCheckBox* CheckBox_Tilt;
		//*)

	protected:

		//(*Identifiers(SkullConfigDialog)
		static const long ID_CHECKBOX_Jaw;
		static const long ID_CHECKBOX_Pan;
		static const long ID_CHECKBOX_Tilt;
		static const long ID_CHECKBOX_Nod;
		static const long ID_CHECKBOX_EyeUD;
		static const long ID_CHECKBOX_EyeLR;
		static const long ID_CHECKBOX_Color;
		static const long ID_CHECKBOX_16bits;
		static const long ID_CHECKBOX_Skulltronix;
		//*)

	private:

		//(*Handlers(SkullConfigDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

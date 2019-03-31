#ifndef ALIGNMENTDIALOG_H
#define ALIGNMENTDIALOG_H

//(*Headers(AlignmentDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
//*)

class AlignmentDialog: public wxDialog
{
	public:

        enum class Alignment
        {
            TOP, MIDDLE, BOTTOM,
            LEFT, CENTRE, RIGHT
        };

		AlignmentDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~AlignmentDialog();
        Alignment GetX() const;
        Alignment GetY() const;

		//(*Declarations(AlignmentDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxRadioButton* RadioButton_BC;
		wxRadioButton* RadioButton_BL;
		wxRadioButton* RadioButton_BR;
		wxRadioButton* RadioButton_MC;
		wxRadioButton* RadioButton_ML;
		wxRadioButton* RadioButton_MR;
		wxRadioButton* RadioButton_TC;
		wxRadioButton* RadioButton_TL;
		wxRadioButton* RadioButton_TR;
		//*)

	protected:

		//(*Identifiers(AlignmentDialog)
		static const long ID_RADIOBUTTON1;
		static const long ID_RADIOBUTTON2;
		static const long ID_RADIOBUTTON3;
		static const long ID_RADIOBUTTON4;
		static const long ID_RADIOBUTTON5;
		static const long ID_RADIOBUTTON6;
		static const long ID_RADIOBUTTON7;
		static const long ID_RADIOBUTTON8;
		static const long ID_RADIOBUTTON9;
		static const long ID_BUTTON2;
		static const long ID_BUTTON1;
		//*)

	private:

		//(*Handlers(AlignmentDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

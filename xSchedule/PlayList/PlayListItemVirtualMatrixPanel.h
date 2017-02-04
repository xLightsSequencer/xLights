#ifndef PLAYLISTITEMVirtualMatrixPANEL_H
#define PLAYLISTITEMVirtualMatrixPANEL_H

//(*Headers(PlayListItemVirtualMatrixPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/button.h>
//*)

class PlayListItemVirtualMatrix;
class VirtualMatrixFilePickerCtrl;

class PlayListItemVirtualMatrixPanel: public wxPanel
{
    PlayListItemVirtualMatrix* _VirtualMatrix;

	public:

		PlayListItemVirtualMatrixPanel(wxWindow* parent, PlayListItemVirtualMatrix* VirtualMatrix,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemVirtualMatrixPanel();

		//(*Declarations(PlayListItemVirtualMatrixPanel)
		wxTextCtrl* TextCtrl_Name;
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxSpinCtrl* SpinCtrl_Height;
		wxCheckBox* CheckBox_Topmost;
		wxButton* Button_PositionWindow;
		wxStaticText* StaticText4;
		wxSpinCtrl* SpinCtrl_StartChannel;
		wxSpinCtrl* SpinCtrl_Width;
		//*)

	protected:

		//(*Identifiers(PlayListItemVirtualMatrixPanel)
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL3;
		static const long ID_BUTTON1;
		static const long ID_CHECKBOX1;
		//*)

	private:

		//(*Handlers(PlayListItemVirtualMatrixPanel)
		void OnButton_PositionWindowClick(wxCommandEvent& event);
		void OnTextCtrl_NameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

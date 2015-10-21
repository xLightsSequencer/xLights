#ifndef RESIZEIMAGEDIALOG_H
#define RESIZEIMAGEDIALOG_H

//(*Headers(ResizeImageDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/dialog.h>
//*)

class ResizeImageDialog: public wxDialog
{
	public:

		ResizeImageDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ResizeImageDialog();

		//(*Declarations(ResizeImageDialog)
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxSpinCtrl* WidthSpinCtrl;
		wxChoice* ResizeChoice;
		wxSpinCtrl* HeightSpinCtrl;
		//*)

	protected:

		//(*Identifiers(ResizeImageDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE1;
		//*)

	private:

		//(*Handlers(ResizeImageDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

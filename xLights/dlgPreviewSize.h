#ifndef DLGPREVIEWSIZE_H
#define DLGPREVIEWSIZE_H

//(*Headers(dlgPreviewSize)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/dialog.h>
//*)

class dlgPreviewSize: public wxDialog
{
	public:

		dlgPreviewSize(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~dlgPreviewSize();

		//(*Declarations(dlgPreviewSize)
		wxTextCtrl* TextCtrl_PreviewWidth;
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		wxTextCtrl* TextCtrl_PreviewHeight;
		//*)



	protected:

		//(*Identifiers(dlgPreviewSize)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL_PREVIEW_WIDTH;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL_PREVIEW_HEIGHT;
		//*)

	private:

		//(*Handlers(dlgPreviewSize)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

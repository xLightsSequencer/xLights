#ifndef SPLASHDIALOG_H
#define SPLASHDIALOG_H

//(*Headers(SplashDialog)
#include <wx/dialog.h>
//*)

#include <wx/bitmap.h>

class SplashDialog: public wxDialog
{
	public:

		SplashDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SplashDialog();

		//(*Declarations(SplashDialog)
		//*)

	protected:

		//(*Identifiers(SplashDialog)
		//*)

	private:

		//(*Handlers(SplashDialog)
		void OnPaint(wxPaintEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

        wxBitmap _image;
};

#endif

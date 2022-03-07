#ifndef SKETCHPATHDIALOG_H
#define SKETCHPATHDIALOG_H

//(*Headers(SketchPathDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
//*)

class SketchPathDialog: public wxDialog
{
	public:

		SketchPathDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SketchPathDialog();

		//(*Declarations(SketchPathDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxStaticBox* StaticBox1;
		wxStaticText* StaticText1;
		//*)

	protected:

		//(*Identifiers(SketchPathDialog)
		static const long ID_STATICBOX1;
		static const long ID_STATICTEXT1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(SketchPathDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

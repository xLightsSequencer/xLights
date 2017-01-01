#ifndef VIDEOWINDOWPOSITIONDIALOG_H
#define VIDEOWINDOWPOSITIONDIALOG_H

//(*Headers(VideoWindowPositionDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class VideoWindowPositionDialog: public wxDialog
{
	public:

		VideoWindowPositionDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~VideoWindowPositionDialog();

		//(*Declarations(VideoWindowPositionDialog)
		wxButton* Button_Ok;
		wxStaticText* StaticText1;
		//*)

	protected:

		//(*Identifiers(VideoWindowPositionDialog)
		static const long ID_STATICTEXT1;
		static const long ID_BUTTON1;
		//*)

	private:

		//(*Handlers(VideoWindowPositionDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

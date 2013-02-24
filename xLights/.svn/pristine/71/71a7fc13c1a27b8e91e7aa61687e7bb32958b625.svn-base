#ifndef PLAYBACKOPTIONSDIALOG_H
#define PLAYBACKOPTIONSDIALOG_H

//(*Headers(PlaybackOptionsDialog)
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
//*)

class PlaybackOptionsDialog: public wxDialog
{
	public:

		PlaybackOptionsDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlaybackOptionsDialog();

		//(*Declarations(PlaybackOptionsDialog)
		wxCheckBox* CheckBoxFirstItem;
		wxCheckBox* CheckBoxRepeat;
		wxCheckBox* CheckBoxLastItem;
		wxCheckBox* CheckBoxRandom;
		//*)

	protected:

		//(*Identifiers(PlaybackOptionsDialog)
		static const long ID_CHECKBOX_REPEAT;
		static const long ID_CHECKBOX_FIRSTITEM;
		static const long ID_CHECKBOX_LASTITEM;
		static const long ID_CHECKBOX_RANDOM;
		//*)

	private:

		//(*Handlers(PlaybackOptionsDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

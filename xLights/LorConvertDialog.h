#ifndef LORCONVERTDIALOG_H
#define LORCONVERTDIALOG_H

//(*Headers(LorConvertDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
//*)

class LorConvertDialog: public wxDialog
{
	public:

		LorConvertDialog(wxWindow* parent);
		virtual ~LorConvertDialog();

		//(*Declarations(LorConvertDialog)
		wxStaticText* StaticText20;
		wxStaticText* StaticText31;
		wxCheckBox* CheckBoxMapEmptyChannels;
		wxCheckBox* MapLORChannelsWithNoNetwork;
		wxStaticText* StaticText17;
		wxCheckBox* CheckBoxOffAtEnd;
		//*)

	protected:

		//(*Identifiers(LorConvertDialog)
		static const long ID_STATICTEXT17;
		static const long ID_CHECKBOX_OFF_AT_END;
		static const long ID_STATICTEXT20;
		static const long ID_CHECKBOX_MAP_EMPTY_CHANNELS;
		static const long ID_STATICTEXT33;
		static const long ID_CHECKBOX_LOR_WITH_NO_CHANNELS;
		//*)

	private:

		//(*Handlers(LorConvertDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

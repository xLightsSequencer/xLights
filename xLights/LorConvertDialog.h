#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(LorConvertDialog)
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class LorConvertDialog: public wxDialog
{
	public:

		LorConvertDialog(wxWindow* parent);
		virtual ~LorConvertDialog();

		//(*Declarations(LorConvertDialog)
		wxStaticText* StaticText17;
		wxStaticText* StaticText20;
		wxCheckBox* CheckBoxMapEmptyChannels;
		wxCheckBox* CheckBoxOffAtEnd;
		wxCheckBox* MapLORChannelsWithNoNetwork;
		wxStaticText* StaticText31;
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

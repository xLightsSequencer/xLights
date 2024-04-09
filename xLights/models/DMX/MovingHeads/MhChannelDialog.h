#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifndef MHCHANNELDIALOG_H
#define MHCHANNELDIALOG_H

//(*Headers(MhChannelDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

#include "tmGridCell.h"

class MhFeature;

class MhChannelDialog: public wxDialog
{
	public:

		MhChannelDialog(std::unique_ptr<MhFeature>& _feature, wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MhChannelDialog();

		//(*Declarations(MhChannelDialog)
		wxButton* Button_AddChannel;
		wxFlexGridSizer* FlexGridSizer2;
		wxFlexGridSizer* FlexGridSizerMain;
		wxStaticText* StaticTextFeatureLabel;
		//*)
    
        tmGrid* Grid_Channels;

        void SetFeatureName(const std::string& text);

	protected:

		//(*Identifiers(MhChannelDialog)
		static const long ID_STATICTEXT_FeatureLabel;
		static const long ID_BUTTON_AddChannel;
		//*)

        static const long ID_GRID_CHANNELS;

	private:

        void OnButton_FeatureClick(wxCommandEvent& event);
        void On_CellChanged(wxCommandEvent& event);
    void AddChannel(int row, const std::string& name);

		//(*Handlers(MhChannelDialog)
		void OnButton_AddChannelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
    
        std::unique_ptr<MhFeature>& feature;
        std::vector<std::pair<std::string, std::string>> channel_map;
};

#endif

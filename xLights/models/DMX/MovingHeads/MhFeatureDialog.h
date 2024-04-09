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

#ifndef MHFEATUREDIALOG_H
#define MHFEATUREDIALOG_H

//(*Headers(MhFeatureDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
//*)
#include <wx/xml/xml.h>
#include "tmGridCell.h"

class MhFeature;

class MhFeatureDialog: public wxDialog
{
	public:

		MhFeatureDialog(std::vector<std::unique_ptr<MhFeature>>& _features, wxXmlNode* _node_xml, wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MhFeatureDialog();

		//(*Declarations(MhFeatureDialog)
		wxButton* Button_AddFeature;
		wxFlexGridSizer* FlexGridSizer2;
		wxFlexGridSizer* FlexGridSizerMain;
		//*)

        tmGrid* Grid_Features;

	protected:

		//(*Identifiers(MhFeatureDialog)
		static const long ID_BUTTON_AddFeature;
		//*)

        static const long ID_GRID_FEATURES;

	private:

        void OnButton_FeatureClick(wxCommandEvent& event);
        void OnButton_RenameFeatureClick(wxCommandEvent& event);

		//(*Handlers(MhFeatureDialog)
		void OnButton_AddFeatureClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

        wxXmlNode* node_xml;
        std::vector<std::unique_ptr<MhFeature>>& features;
};

#endif

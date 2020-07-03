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

//(*Headers(StrandNodeNamesDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class Model;

class StrandNodeNamesDialog: public wxDialog
{
    void ValidateWindow();

	public:

		StrandNodeNamesDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~StrandNodeNamesDialog();

		//(*Declarations(StrandNodeNamesDialog)
		wxButton* ButtonCancel;
		wxButton* ButtonOk;
		wxGrid* NodesGrid;
		wxGrid* StrandsGrid;
		//*)

        void Setup(const Model *md, const std::string &nodeNames, const std::string &strandNames);
    
        std::string GetNodeNames();
        std::string GetStrandNames();
	protected:

		//(*Identifiers(StrandNodeNamesDialog)
		static const long ID_GRID2;
		static const long ID_GRID1;
		static const long ID_BUTTONOK;
		static const long ID_BUTTONCANCEL;
		//*)

	private:

		//(*Handlers(StrandNodeNamesDialog)
		void OnButtonCancelClick(wxCommandEvent& event);
		void OnButtonOkClick(wxCommandEvent& event);
		void OnStrandsGridCellChanged(wxGridEvent& event);
		void OnNodesGridCellChanged(wxGridEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

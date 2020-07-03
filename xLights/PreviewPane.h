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

//(*Headers(PreviewPane)
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/dialog.h>
//*)

class ModelPreview;
class LayoutGroup;

class PreviewPane: public wxDialog
{
	public:

		PreviewPane(wxWindow* parent, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PreviewPane();

		wxPanel* GetPreviewPanel() {return PreviewPanel;}
		wxFlexGridSizer* GetPreviewPanelSizer() {return PreviewPanelSizer;}

		void SetLayoutGroup( LayoutGroup* grp );
		bool GetActive();

		//(*Declarations(PreviewPane)
		wxFlexGridSizer* PreviewPanelSizer;
		wxPanel* PreviewPanel;
		//*)

	protected:

		//(*Identifiers(PreviewPane)
		static const long ID_PANEL_PREVIEW;
		//*)

	private:
        LayoutGroup* layout_grp;
        PreviewPane* mPreviewPane;

		//(*Handlers(PreviewPane)
		void OnClose(wxCloseEvent& event);
		void OnResize(wxSizeEvent& event);
		//*)
        void OnMoved(wxMoveEvent& event);

		DECLARE_EVENT_TABLE()
};

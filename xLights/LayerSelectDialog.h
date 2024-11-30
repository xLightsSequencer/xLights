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

//(*Headers(LayerSelectDialog)
#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
//*)

class LayerSelectDialog: public wxDialog
{
    int _start;
    bool _blending;
	std::vector<int> _layerWithEffect;
    void ValidateWindow();

	public:

		LayerSelectDialog(wxWindow* parent, int startLayer, int endLayer, bool blendLayer, const std::string &layersSelected, const std::vector<int> &layerWithEffect, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~LayerSelectDialog();
        void SelectLayer(wxString layer);
        std::string GetSelectedLayers() const;
        void SelectAllLayers(bool incBlendingLayer = false);

		//(*Declarations(LayerSelectDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxCheckListBox* CheckListBox_Layers;
		//*)

	protected:

		//(*Identifiers(LayerSelectDialog)
		static const long ID_CHECKLISTBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

		static const long ID_MCU_SELECTALL;
		static const long ID_MCU_SELECTNONE;
		static const long ID_MCU_SELECTPOPULATED;

	private:

		//(*Handlers(LayerSelectDialog)
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnCheckListBox_LayersToggled(wxCommandEvent& event);
		//*)

		void OnListRClick(wxContextMenuEvent& event);
		void OnPopup(wxCommandEvent &event);

		void DeselectAllLayers();
		void SelectLayersWithEffects();

		DECLARE_EVENT_TABLE()
};

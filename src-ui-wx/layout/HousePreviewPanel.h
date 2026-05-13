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

//(*Headers(HousePreviewPanel)
#include <wx/bmpbuttn.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
//*)

#include <wx/button.h>
#include <wx/timer.h>

class xLightsFrame;
class ModelPreview;
class Model;
class LayoutGroup;

class HousePreviewPanel: public wxPanel
{
	public:

		HousePreviewPanel(wxWindow* parent,
                            xLightsFrame* frame,
                            bool showToolbars,
                            std::vector<Model*> &models,
                            bool allowSelected,
                            int style = 0,
                            bool allowPreviewChange = false,
                            wxWindowID id=wxID_ANY, 
                            const wxPoint& pos=wxDefaultPosition,
                            const wxSize& size=wxDefaultSize);
		virtual ~HousePreviewPanel();
        ModelPreview* GetModelPreview() const { return _modelPreview; }
        void EnablePlayControls(const std::string& control, bool enable);
        void SetToolbar(bool toolbar) { _showToolbar = toolbar; }
        void SetDurationFrames(int frames);
        void SetPositionFrames(int frames);
        void Set3d(bool is3d);
        bool Is3d() const;

		//(*Declarations(HousePreviewPanel)
		wxBitmapButton* FastForward10Button;
		wxBitmapButton* PauseButton;
		wxBitmapButton* PlayButton;
		wxBitmapButton* Rewind10Button;
		wxBitmapButton* RewindButton;
		wxBitmapButton* StopButton;
		wxFlexGridSizer* ButtonSizer;
		wxPanel* Panel1;
		wxSlider* SliderPosition;
		wxStaticText* StaticText_Time;
		//*)

	protected:

		//(*Identifiers(HousePreviewPanel)
		static const long ID_BITMAPBUTTON1;
		static const long ID_BITMAPBUTTON2;
		static const long ID_BITMAPBUTTON3;
		static const long ID_BITMAPBUTTON4;
		static const long ID_BITMAPBUTTON6;
		static const long ID_BITMAPBUTTON7;
		static const long ID_SLIDER1;
		static const long ID_STATICTEXT1;
		static const long ID_PANEL1;
		//*)

	private:

		//(*Handlers(HousePreviewPanel)
		void OnPlayButtonClick(wxCommandEvent& event);
		void OnPauseButtonClick(wxCommandEvent& event);
		void OnStopButtonClick(wxCommandEvent& event);
		void OnRewindButtonClick(wxCommandEvent& event);
		void OnRewind10ButtonClick(wxCommandEvent& event);
		void OnFastForward10ButtonClick(wxCommandEvent& event);
		void OnEndButtonClick(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnSliderPositionCmdSliderUpdated(wxScrollEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

        xLightsFrame* _xLights;
        bool _showToolbar;
        ModelPreview* _modelPreview;

        // Persistent volume + speed controls living in their own panel
        // (_mediaPanel) added as a third row below the existing transport
        // bar (Panel1). Reliable across 2D / 3D / fullscreen views (unlike
        // a hover overlay on the GL / Metal canvas, which mouse-enter
        // events miss when the canvas is grabbing input for 3D rotate /
        // drag).
        wxPanel*    _mediaPanel = nullptr;
        wxSlider*   _hoverVolumeSlider = nullptr;
        wxButton*   _hoverSpeedDown = nullptr;
        wxStaticText* _hoverSpeedLabel = nullptr;
        wxButton*   _hoverSpeedUp = nullptr;

        void OnHoverVolumeSlider(wxCommandEvent& event);
        void OnHoverSpeedDown(wxCommandEvent& event);
        void OnHoverSpeedUp(wxCommandEvent& event);

        void ValidateWindow(const wxSize& size);
};

#ifndef HOUSEPREVIEWPANEL_H
#define HOUSEPREVIEWPANEL_H

//(*Headers(HousePreviewPanel)
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/bmpbuttn.h>
//*)

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
                            std::vector<LayoutGroup *> &groups,
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
        void SetDurationFrames(long frames);
        void SetPositionFrames(long frames);

		//(*Declarations(HousePreviewPanel)
		wxFlexGridSizer* ButtonSizer;
		wxPanel* Panel1;
		wxBitmapButton* FastForward10Button;
		wxBitmapButton* PauseButton;
		wxBitmapButton* Rewind10Button;
		wxBitmapButton* PlayButton;
		wxBitmapButton* StopButton;
		wxBitmapButton* RewindButton;
		wxSlider* SliderPosition;
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

        void ValidateWindow(const wxSize& size);
};

#endif

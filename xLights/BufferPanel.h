#ifndef BUFFERPANEL_H
#define BUFFERPANEL_H

//(*Headers(BufferPanel)
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
//*)

class Model;

class BufferPanel: public wxPanel
{
	public:

		BufferPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~BufferPanel();

        wxString GetBufferString();
        void SetDefaultControls(const Model *model);

    
		//(*Declarations(BufferPanel)
		wxSlider* Slider_ZoomCycles;
		wxBitmapButton* BitmapButton_ZoomCycles;
		wxSlider* Slider_ZoomRotation;
		wxBitmapButton* BitmapButton_ZoomRotation;
		wxFlexGridSizer* Sizer2;
		wxTextCtrl* TextCtrl_ZoomRotation;
		wxBitmapButton* BitmapButton_ZoomInOut;
		wxTextCtrl* TextCtrl_ZoomInOut;
		wxChoice* BufferStyleChoice;
		wxBitmapButton* BitmapButton_EffectBlur;
		wxPanel* Panel_Sizer;
		wxTextCtrl* TextCtrl_ZoomCycles;
		wxFlexGridSizer* RotoSizer;
		wxStaticText* StaticText8;
		wxStaticText* StaticText7;
		wxChoice* BufferTransform;
		wxCheckBox* CheckBox_RotoZoom;
		wxTextCtrl* txtCtlEffectBlur;
		wxBitmapButton* BitmapButton_RotoZoom;
		wxSlider* Slider_ZoomInOut;
		wxScrolledWindow* BufferScrollWindow;
		wxCheckBox* CheckBox_OverlayBkg;
		wxStaticText* StaticText5;
		wxSlider* Slider_EffectBlur;
		wxStaticText* StaticText6;
		wxBitmapButton* BitmapButton_OverlayBkg;
		//*)

	protected:

		//(*Identifiers(BufferPanel)
		static const long ID_CHOICE_BufferStyle;
		static const long ID_BITMAPBUTTON_CHOICE_BufferStyle;
		static const long ID_CHOICE_BufferTransform;
		static const long ID_BITMAPBUTTON_CHOICE_BufferTransform;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_EffectBlur;
		static const long IDD_TEXTCTRL_EffectBlur;
		static const long ID_BITMAPBUTTON_SLIDER_EffectBlur;
		static const long ID_CHECKBOX_OverlayBkg;
		static const long ID_BITMAPBUTTON_OverlayBkg;
		static const long ID_CHECKBOX_RotoZoom;
		static const long ID_BITMAPBUTTON_CHECKBOX_RotoZoom;
		static const long ID_STATICTEXT4;
		static const long ID_SLIDER_ZoomCycles;
		static const long IDD_TEXTCTRL_ZoomCycles;
		static const long ID_BITMAPBUTTON_SLIDER_ZoomCycles;
		static const long ID_STATICTEXT5;
		static const long ID_SLIDER_ZoomRotation;
		static const long IDD_TEXTCTRL_ZoomRotation;
		static const long ID_BITMAPBUTTON_SLIDER_ZoomRotation;
		static const long ID_STATICTEXT6;
		static const long ID_SLIDER_ZoomInOut;
		static const long IDD_TEXTCTRL_ZoomInOut;
		static const long ID_BITMAPBUTTON_SLIDER_ZoomInOut;
		static const long ID_SCROLLED_ColorScroll;
		static const long ID_PANEL1;
		//*)

	private:

		//(*Handlers(BufferPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnCheckBox_RotoZoomClick(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

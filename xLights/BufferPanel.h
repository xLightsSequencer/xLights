#ifndef BUFFERPANEL_H
#define BUFFERPANEL_H

//(*Headers(BufferPanel)
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
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
		wxScrolledWindow* BufferScrollWindow;
		wxBitmapButton* BitmapButton_ZoomCycles;
		wxBitmapButton* BitmapButton_EffectBlur;
		wxBitmapButton* BitmapButton_RotoZoom;
		wxTextCtrl* TextCtrl_ZoomCycles;
		wxTextCtrl* TextCtrl_ZoomRotation;
		wxStaticText* StaticText6;
		wxChoice* BufferStyleChoice;
		wxFlexGridSizer* Sizer2;
		wxBitmapButton* BitmapButton_ZoomRotation;
		wxPanel* Panel_Sizer;
		wxSlider* Slider_ZoomInOut;
		wxStaticText* StaticText8;
		wxFlexGridSizer* RotoSizer;
		wxBitmapButton* BitmapButton_OverlayBkg;
		wxCheckBox* CheckBox_RotoZoom;
		wxChoice* BufferTransform;
		wxTextCtrl* TextCtrl_ZoomInOut;
		wxSlider* Slider_ZoomRotation;
		wxSlider* Slider_ZoomCycles;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxTextCtrl* TextCtrl_EffectBlur;
		wxBitmapButton* BitmapButton_ZoomInOut;
		wxCheckBox* CheckBox_OverlayBkg;
		wxSlider* Slider_EffectBlur;
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

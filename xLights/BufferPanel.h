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
#include <wx/button.h>
//*)

#include "ValueCurve.h"
#include "ValueCurveButton.h"
#include <string>
#include "RotoZoom.h"

class Model;
class SubBufferPanel;

class RotoZoomButton :
    public wxButton
{
    RotoZoomParms* _parms;
public:
    RotoZoomButton(wxWindow *parent,
        wxWindowID id,
        const wxString& label = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxBU_AUTODRAW,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxButtonNameStr) : wxButton(parent, id, label, pos, size, style, validator, name)
    {
        _parms = new RotoZoomParms(name.ToStdString());
    }
    ~RotoZoomButton()
    {
        if (_parms != NULL)
        {
            delete _parms;
        }
    }
    virtual void SetValue(const std::string &val)
    {
        SetValue(wxString(val.c_str()));
    }
    virtual void SetValue(const wxString& value)
    {
        _parms->Deserialise(value.ToStdString());
    }
    RotoZoomParms* GetValue()
    {
        return _parms;
    }
};

class BufferPanel: public wxPanel
{
    void ValidateWindow();
    void OnVCChanged(wxCommandEvent& event);

    int _zooms;
    int _rotations;
    int _zoommaximum;
    int _xcenter;
    int _ycenter;
public:

		BufferPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~BufferPanel();

        wxString GetBufferString();
        void SetDefaultControls(const Model *model);

    
		//(*Declarations(BufferPanel)
		wxScrolledWindow* BufferScrollWindow;
		wxBitmapButton* BitmapButton_EffectBlur;
		wxChoice* BufferStyleChoice;
		wxFlexGridSizer* Sizer2;
		wxPanel* Panel_Sizer;
		wxFlexGridSizer* SubBufferPanelSizer;
		wxBitmapButton* BitmapButton_OverlayBkg;
		wxChoice* BufferTransform;
		wxStaticText* StaticText5;
		RotoZoomButton* Button_Properties;
		wxTextCtrl* TextCtrl_EffectBlur;
		ValueCurveButton* BitmapButton_Blur;
		wxCheckBox* CheckBox_OverlayBkg;
		wxSlider* Slider_EffectBlur;
		//*)

        SubBufferPanel *subBufferPanel;
	protected:

		//(*Identifiers(BufferPanel)
		static const long ID_CHOICE_BufferStyle;
		static const long ID_BITMAPBUTTON_CHOICE_BufferStyle;
		static const long ID_CHOICE_BufferTransform;
		static const long ID_CUSTOM_RotoZoom;
		static const long ID_BITMAPBUTTON_CHOICE_BufferTransform;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_EffectBlur;
		static const long ID_VALUECURVE_Blur;
		static const long IDD_TEXTCTRL_EffectBlur;
		static const long ID_BITMAPBUTTON_SLIDER_EffectBlur;
		static const long ID_CHECKBOX_OverlayBkg;
		static const long ID_BITMAPBUTTON_OverlayBkg;
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
		void OnResize(wxSizeEvent& event);
		void OnBitmapButton_BlurClick(wxCommandEvent& event);
		void OnSlider_EffectBlurCmdSliderUpdated(wxScrollEvent& event);
		void OnBufferTransformSelect(wxCommandEvent& event);
		void OnButton_PropertiesClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

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

#include "xlGLCanvas.h"
#include "BulkEditControls.h"

class KaleidoscopePreview : public xlGLCanvas
{
public:
   KaleidoscopePreview(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
               const wxSize &size=wxDefaultSize,
               long style=0,
               const wxString &name=wxPanelNameStr,
               bool coreProfile = true);
   virtual ~KaleidoscopePreview();

protected:
   void InitializeGLCanvas() override;
};
//(*Headers(KaleidoscopePanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

class KaleidoscopePanel: public wxPanel
{
	public:

		KaleidoscopePanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~KaleidoscopePanel();

		//(*Declarations(KaleidoscopePanel)
		BulkEditSlider* Slider_Kaleidoscope_Rotation;
		BulkEditSlider* Slider_Kaleidoscope_Size;
		BulkEditSlider* Slider_Kaleidoscope_X;
		BulkEditSlider* Slider_Kaleidoscope_Y;
		BulkEditTextCtrl* TextCtrl_Kaleidoscope_Rotation;
		BulkEditTextCtrl* TextCtrl_Kaleidoscope_Size;
		BulkEditTextCtrl* TextCtrl_Kaleidoscope_X;
		BulkEditTextCtrl* TextCtrl_Kaleidoscope_Y;
		BulkEditValueCurveButton* BitmapButton_Kaleidoscope_Rotation;
		BulkEditValueCurveButton* BitmapButton_Kaleidoscope_Size;
		BulkEditValueCurveButton* BitmapButton_Kaleidoscope_X;
		BulkEditValueCurveButton* BitmapButton_Kaleidoscope_Y;
		wxChoice* Choice_Kaleidoscope_Type;
		wxStaticText* StaticText10;
		wxStaticText* StaticText11;
		wxStaticText* StaticText12;
		wxStaticText* StaticText13;
		wxStaticText* StaticText8;
		wxTextCtrl* TextCtrl1;
		//*)

		void ValidateWindow();

	protected:

		//(*Identifiers(KaleidoscopePanel)
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE_Kaleidoscope_Type;
		static const long ID_STATICTEXT10;
		static const long ID_SLIDER_Kaleidoscope_X;
		static const long ID_VALUECURVE_Kaleidoscope_X;
		static const long IDD_TEXTCTRL_Kaleidoscope_X;
		static const long ID_STATICTEXT11;
		static const long ID_SLIDER_Kaleidoscope_Y;
		static const long ID_VALUECURVE_Kaleidoscope_Y;
		static const long IDD_TEXTCTRL_Kaleidoscope_Y;
		static const long ID_STATICTEXT12;
		static const long ID_SLIDER_Kaleidoscope_Size;
		static const long ID_VALUECURVE_Kaleidoscope_Size;
		static const long IDD_TEXTCTRL_Kaleidoscope_Size;
		static const long ID_STATICTEXT13;
		static const long ID_SLIDER_Kaleidoscope_Rotation;
		static const long ID_VALUECURVE_Kaleidoscope_Rotation;
		static const long IDD_TEXTCTRL_Kaleidoscope_Rotation;
		//*)

	private:

		//(*Handlers(KaleidoscopePanel)
		void OnVCButtonClick(wxCommandEvent& event);
		void OnVCChanged(wxCommandEvent& event);
		void OnChoice_Kaleidoscope_TypeSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

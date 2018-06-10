#ifndef WARPPANEL_H
#define WARPPANEL_H

#include "xlGLCanvas.h"
#include "BulkEditControls.h"

class WarpPreview : public xlGLCanvas
{
public:
   WarpPreview(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
               const wxSize &size=wxDefaultSize,
               long style=0,
               const wxString &name=wxPanelNameStr,
               bool coreProfile = true);
   virtual ~WarpPreview();

protected:
   void InitializeGLCanvas() override;
};
//(*Headers(WarpPanel)
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class WarpPanel: public wxPanel
{
	public:

		WarpPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~WarpPanel();

		//(*Declarations(WarpPanel)
		BulkEditSlider* Slider_Warp_Frequency;
		BulkEditSlider* Slider_Warp_Speed;
		BulkEditSlider* Slider_Warp_X;
		BulkEditSlider* Slider_Warp_Y;
		BulkEditTextCtrl* TextCtrl_Warp_Frequency;
		BulkEditTextCtrl* TextCtrl_Warp_Speed;
		BulkEditTextCtrl* TextCtrl_Warp_X;
		BulkEditTextCtrl* TextCtrl_Warp_Y;
		wxChoice* Choice_Warp_Treatment;
		wxChoice* Choice_Warp_Type;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		//*)

		void ValidateWindow();

	protected:

		//(*Identifiers(WarpPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_Warp_Type;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE_Warp_Treatment;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER_Warp_X;
		static const long IDD_TEXTCTRL_Warp_X;
		static const long ID_STATICTEXT4;
		static const long ID_SLIDER_Warp_Y;
		static const long IDD_TEXTCTRL_Warp_Y;
		static const long ID_STATICTEXT6;
		static const long ID_SLIDER_Warp_Speed;
		static const long IDD_TEXTCTRL_Warp_Speed;
		static const long ID_STATICTEXT5;
		static const long ID_SLIDER_Warp_Frequency;
		static const long IDD_TEXTCTRL_Warp_Frequency;
		//*)

		WarpPreview * _preview;
		static const long ID_CANVAS;
		friend class WarpEffect;

	private:

		//(*Handlers(WarpPanel)
		void OnChoice_Warp_TypeSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

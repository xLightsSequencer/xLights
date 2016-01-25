#ifndef PICTURESASSISTPANEL_H
#define PICTURESASSISTPANEL_H

//(*Headers(PicturesAssistPanel)
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/button.h>
//*)

#include "../../FlickerFreeBitmapButton.h"
#include "xlGridCanvasPictures.h"
#include "../../sequencer/Effect.h"

class xlColorPicker;
class xLightsFrame;

class PicturesAssistPanel: public wxPanel
{
	public:

		PicturesAssistPanel(wxWindow* parent, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PicturesAssistPanel();

        void ForceRefresh();
        void SetGridCanvas(xlGridCanvasPictures* canvas) { mGridCanvas = canvas; }
        void SetxLightsFrame(xLightsFrame* xlights_parent) { mxLightsParent = xlights_parent; }

		//(*Declarations(PicturesAssistPanel)
		wxButton* Button_LoadImage;
		wxStaticText* StaticText_ImageSize;
		wxButton* Button_NewImage;
		wxPanel* Panel_Sizer;
		wxScrolledWindow* ScrolledWindowEffectAssist;
		wxButton* Button_Resize;
		wxStaticText* StaticText_CurrentImage;
		wxButton* Button_SaveAs;
		wxFlexGridSizer* FlexGridSizer_RightSide;
		wxStaticText* StaticText_ModelSize;
		wxFlexGridSizer* FlexGridSizer1;
		wxPanel* Panel_RightSide;
		wxButton* Button_SaveImage;
		wxFlexGridSizer* FlexGridSizer_Container;
		//*)

	protected:
		static const long ID_PANEL_EffectGrid;
        static const long ID_BITMAPBUTTON_Paint_Pencil;
        static const long ID_BITMAPBUTTON_Paint_Eraser;
        static const long ID_BITMAPBUTTON_Paint_Eyedropper;
        static const long ID_BITMAPBUTTON_Paint_Selectcopy;

		FlickerFreeBitmapButton* BitmapButton_Paint_Pencil;
		FlickerFreeBitmapButton* BitmapButton_Paint_Eraser;
		FlickerFreeBitmapButton* BitmapButton_Paint_Eyedropper;
		FlickerFreeBitmapButton* BitmapButton_Paint_Selectcopy;


		//(*Identifiers(PicturesAssistPanel)
		static const long ID_BUTTON_NewImage;
		static const long ID_BUTTON_LoadImage;
		static const long ID_BUTTON_SaveImage;
		static const long ID_BUTTON_SaveAs;
		static const long ID_BUTTON1;
		static const long ID_STATICTEXT_CurrentImage;
		static const long ID_STATICTEXT_ImageSize;
		static const long ID_STATICTEXT_ModelSize;
		static const long ID_PANEL_RightSide;
		static const long ID_SCROLLED_EffectAssist;
		static const long ID_PANEL1;
		//*)

	private:

		//(*Handlers(PicturesAssistPanel)
		void OnResize(wxSizeEvent& event);
		void OnButton_SaveImageClick(wxCommandEvent& event);
		void OnButton_NewImageClick(wxCommandEvent& event);
		void OnButton_LoadImageClick(wxCommandEvent& event);
		void OnButton_SaveAsClick(wxCommandEvent& event);
		void OnButton_ResizeClick(wxCommandEvent& event);
		//*)

        void CreatePaintTools(wxWindow* parent, wxFlexGridSizer* container);
		void OnBitmapButton_Paint_PencilClick(wxCommandEvent& event);
		void OnBitmapButton_Paint_EraserClick(wxCommandEvent& event);
		void OnBitmapButton_Paint_EyedropperClick(wxCommandEvent& event);
		void OnBitmapButton_Paint_SelectcopyClick(wxCommandEvent& event);
		void OnColorChange(wxCommandEvent& event);
		void OnImageFileSelected(wxCommandEvent& event);
		void OnImageSize(wxCommandEvent& event);
		void OnEyedropperColor(wxCommandEvent& event);
        void OnWindowScrolled(wxScrollWinEvent &event);

        xLightsFrame* mxLightsParent;
        xlGridCanvasPictures* mGridCanvas;
        xlColorPicker* mColorPicker;
        Effect* mEffect;
        xlGridCanvasPictures::PaintMode mPaintMode;
        xlColor mPaintColor;
        wxPanel* mPaintTools;
        wxBitmap paint_pencil;
        wxBitmap paint_pencil_selected;
        wxBitmap paint_eraser;
        wxBitmap paint_eraser_selected;
        wxBitmap paint_eyedropper;
        wxBitmap paint_eyedropper_selected;
        wxBitmap paint_selectcopy;
        wxBitmap paint_selectcopy_selected;

		DECLARE_EVENT_TABLE()
};

#endif

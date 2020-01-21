#ifndef COLORMANAGERSETTINGSPANEL_H
#define COLORMANAGERSETTINGSPANEL_H

//(*Headers(ColorManagerSettingsPanel)
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/sizer.h>
//*)

#include "Color.h"

class xLightsFrame;
class wxBitmapButton;
class ColorManagerSettingsPanel: public wxPanel
{
	public:

		ColorManagerSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ColorManagerSettingsPanel();

		//(*Declarations(ColorManagerSettingsPanel)
		wxButton* ButtonExport;
		wxButton* ButtonImport;
		wxButton* Button_Reset;
		wxFlexGridSizer* Sizer_Effect_Grid;
		wxFlexGridSizer* Sizer_Layout_Tab;
		wxFlexGridSizer* Sizer_Timing_Tracks;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(ColorManagerSettingsPanel)
		static const long ID_BUTTON_IMPORT;
		static const long ID_BUTTON_EXPORT;
		static const long ID_BUTTON_RESET;
		//*)

	private:
        xLightsFrame *frame;
    
        void UpdateButtonColors();
        void AddButtonsToDialog();
        void RefreshColors();
        void SetButtonColor(wxBitmapButton* btn, const wxColour &c);
        void SetButtonColor(wxBitmapButton* btn, const xlColor &color);
    
		//(*Handlers(ColorManagerSettingsPanel)
		void OnButtonImportClick(wxCommandEvent& event);
		void OnButtonExportClick(wxCommandEvent& event);
		void OnButton_ResetClick(wxCommandEvent& event);
        void ColorButtonSelected(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

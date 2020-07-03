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

#include <list>

//(*Headers(GenerateLyricsDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class GenerateLyricsDialog: public wxDialog
{
	public:

		GenerateLyricsDialog(wxWindow* parent, long channels, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~GenerateLyricsDialog();
        std::list<long> GetChannels(const std::string& phenome) const;
        std::string GetLyricName() const;

		//(*Declarations(GenerateLyricsDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Load;
		wxButton* Button_Ok;
		wxButton* Button_Save;
		wxGrid* SingleNodeGrid;
		wxPanel* Panel1;
		wxSpinCtrl* SpinCtrl_FirstChannel;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Name;
		//*)

	protected:

		//(*Identifiers(GenerateLyricsDialog)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL1;
		static const long ID_PANEL1;
		static const long ID_GRID_COROFACES;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(GenerateLyricsDialog)
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnSingleNodeGridCellLeftClick(wxGridEvent& event);
		void OnSingleNodeGridCellLeftDClick(wxGridEvent& event);
		void OnSingleNodeGridCellChange(wxGridEvent& event);
		void OnSingleNodeGridCellSelect(wxGridEvent& event);
		void OnSpinCtrl_FirstChannelChange(wxSpinEvent& event);
		void OnTextCtrl_NameText(wxCommandEvent& event);
		void Paint(wxPaintEvent& event);
		void OnButton_SaveClick(wxCommandEvent& event);
		void OnButton_LoadClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

        void ValidateWindow();
        void PaintFace(wxDC& dc, int x, int y, const char* xpm[]);
};

#ifndef GENERATELYRICSDIALOG_H
#define GENERATELYRICSDIALOG_H

#include <list>

//(*Headers(GenerateLyricsDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/grid.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class GenerateLyricsDialog: public wxDialog
{
	public:

		GenerateLyricsDialog(wxWindow* parent, long channels, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~GenerateLyricsDialog();
        std::list<long> GetChannels(const std::string& phenome) const;
        std::string GetLyricName() const;

		//(*Declarations(GenerateLyricsDialog)
		wxButton* Button_Ok;
		wxTextCtrl* TextCtrl_Name;
		wxStaticText* StaticText2;
		wxSpinCtrl* SpinCtrl_FirstChannel;
		wxGrid* SingleNodeGrid;
		wxPanel* Panel1;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxButton* Button_Cancel;
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
		//*)

		DECLARE_EVENT_TABLE()

        void ValidateWindow();
        void PaintFace(wxDC& dc, int x, int y, const char* xpm[]);
};

#endif

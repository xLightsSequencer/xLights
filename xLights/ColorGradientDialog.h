#ifndef COLORGRADIENTDIALOG_H
#define COLORGRADIENTDIALOG_H

//(*Headers(ColorGradientDialog)
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/grid.h>
#include <wx/choice.h>
#include <wx/dialog.h>
//*)

class ColorGradientDialog: public wxDialog
{
	public:

		ColorGradientDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ColorGradientDialog();

		//(*Declarations(ColorGradientDialog)
		wxPanel* Panel1;
		wxGrid* GridColours;
		wxChoice* Choice1;
		//*)

	protected:

		//(*Identifiers(ColorGradientDialog)
		static const long ID_PANEL1;
		static const long ID_GRID1;
		static const long ID_CHOICE1;
		//*)

	private:

		//(*Handlers(ColorGradientDialog)
		void OnGridColoursCellChange(wxGridEvent& event);
		void OnGridColoursCellLeftClick(wxGridEvent& event);
		void OnGridColoursCellLeftDClick(wxGridEvent& event);
		void OnGridColoursCellRightClick(wxGridEvent& event);
		void OnGridColoursLabelRightClick(wxGridEvent& event);
		void OnChoice1Select(wxCommandEvent& event);
		void OnGridColoursCellSelect(wxGridEvent& event);
		void OnPanel1Paint(wxPaintEvent& event);
		void OnPanel1LeftDown(wxMouseEvent& event);
		void OnPanel1LeftUp(wxMouseEvent& event);
		void OnPanel1MouseMove(wxMouseEvent& event);
		void OnPanel1MouseEnter(wxMouseEvent& event);
		void OnPanel1MouseLeave(wxMouseEvent& event);
		void OnResize(wxSizeEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

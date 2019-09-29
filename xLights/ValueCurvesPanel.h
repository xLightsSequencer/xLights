#pragma once

//(*Headers(ValueCurvesPanel)
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
//*)

#include <wx/dir.h>
#include "DragValueCurveBitmapButton.h"

class ValueCurvesPanel: public wxPanel
{
    int ProcessPresetDir(wxDir& directory, bool subdirs);

	public:

		ValueCurvesPanel(
                        wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ValueCurvesPanel();
		void UpdateValueCurveButtons();

		//(*Declarations(ValueCurvesPanel)
		wxFlexGridSizer* FlexGridSizer1;
		wxFlexGridSizer* FlexGridSizer2;
		wxGridSizer* GridSizer1;
		wxPanel* Panel_Sizer;
		wxScrolledWindow* ScrolledWindow1;
		//*)

	protected:

		//(*Identifiers(ValueCurvesPanel)
		static const long ID_SCROLLEDWINDOW1;
		static const long ID_PANEL1;
		//*)

	private:
		//(*Handlers(ValueCurvesPanel)
		void OnResize(wxSizeEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};


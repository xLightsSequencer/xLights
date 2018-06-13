#ifndef ASSISTPANEL_H
#define ASSISTPANEL_H

//(*Headers(AssistPanel)
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
//*)

#include "../../xlGridCanvas.h"

class xLightsFrame;
class Model;

class AssistPanel: public wxPanel
{
	public:

		AssistPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~AssistPanel();

        void AdjustSize(wxSize& s);
        wxWindow* GetCanvasParent() { return ScrolledWindowAssist; }
        void SetGridCanvas(xlGridCanvas* canvas);
        void SetEffectInfo(Effect* effect_, xLightsFrame* xlights_parent);
        void RefreshEffect();
        void AddPanel(wxPanel* panel);

		//(*Declarations(AssistPanel)
		wxFlexGridSizer* FlexGridSizer1;
		wxFlexGridSizer* FlexGridSizer2;
		wxScrolledWindow* ScrolledWindowAssist;
		//*)

	protected:
        xlGridCanvas* mGridCanvas;
        wxPanel* mPanel;
        Effect* mEffect;
        Model *mModel;

		//(*Identifiers(AssistPanel)
		static const long ID_SCROLLEDWINDOW_Assist;
		//*)

	private:
        void SetHandlers(wxWindow *);
		void OnChar(wxKeyEvent& event);
		void OnCharHook(wxKeyEvent& event);
		void OnKeyDown(wxKeyEvent& event);

		//(*Handlers(AssistPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

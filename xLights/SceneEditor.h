#ifndef SCENEEDITOR_H
#define SCENEEDITOR_H

//(*Headers(SceneEditor)
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/panel.h>
//*)

#include "xlGridCanvas.h"
#include "Effect.h"

class xLightsFrame;

class SceneEditor: public wxPanel
{
	public:

		SceneEditor(wxWindow* parent, xLightsFrame* xlights_parent);
		virtual ~SceneEditor();

		//(*Declarations(SceneEditor)
		wxScrolledWindow* ScrolledWindowSceneEditor;
		wxPanel* Panel_Sizer;
		xlGridCanvas* PanelSceneEditor;
		//*)

        void SetEffect(Effect* effect_);
        Effect* GetEffect() {return mEffect;}
        void ForceRefresh();

	protected:

		//(*Identifiers(SceneEditor)
		static const long ID_PANEL_Scene_Editor;
		static const long ID_SCROLLED_Scene_Editor;
		static const long ID_PANEL1;
		//*)

	private:

		//(*Handlers(SceneEditor)
		void OnResize(wxSizeEvent& event);
		//*)

        xLightsFrame* mxLightsParent;
        Effect* mEffect;

		DECLARE_EVENT_TABLE()
};

#endif

#ifndef WARPPANEL_H
#define WARPPANEL_H

#include "xlGLCanvas.h"

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
#include <wx/combobox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class WarpPanel: public wxPanel
{
	public:

		WarpPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~WarpPanel();

		//(*Declarations(WarpPanel)
		wxComboBox* Choice_Warp_Effect;
		wxComboBox* Choice_Warp_Type;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		//*)

	protected:

		//(*Identifiers(WarpPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_Warp_Effect;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE_Warp_Type;
		//*)

		WarpPreview * _preview;
		static const long ID_CANVAS;
		friend class WarpEffect;

	private:

		//(*Handlers(WarpPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

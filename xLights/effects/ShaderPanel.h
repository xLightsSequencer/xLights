#ifndef SHADERPANEL_H
#define SHADERPANEL_H

#include "xlGLCanvas.h"

class ShaderPreview : public xlGLCanvas
{
public:
   ShaderPreview(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                 const wxSize &size=wxDefaultSize,
                 long style=0,
                 const wxString &name=wxPanelNameStr,
                 bool coreProfile = true);
   virtual ~ShaderPreview();

protected:
   void InitializeGLContext() override;
};

//(*Headers(ShaderPanel)
#include <wx/panel.h>
//*)

class ShaderPanel: public wxPanel
{
	public:

		ShaderPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ShaderPanel();

		//(*Declarations(ShaderPanel)
		//*)

	protected:

		//(*Identifiers(ShaderPanel)
		//*)

		ShaderPreview *   _preview;
		static const long ID_CANVAS;
      friend class ShaderEffect;

	private:

		//(*Handlers(ShaderPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

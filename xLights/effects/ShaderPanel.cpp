#include "ShaderPanel.h"

//(*InternalHeaders(ShaderPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ShaderPanel)
//*)

ShaderPreview::ShaderPreview( wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style, const wxString &name, bool coreProfile)
   : xlGLCanvas( parent, id, pos, size, style, name, coreProfile )
{

}

ShaderPreview::~ShaderPreview()
{

}

void ShaderPreview::InitializeGLContext()
{
   // should just be doing init stuff?
   SetCurrentGLContext();
}

const long ShaderPanel::ID_CANVAS = wxNewId();

BEGIN_EVENT_TABLE(ShaderPanel,wxPanel)
	//(*EventTable(ShaderPanel)
	//*)
END_EVENT_TABLE()

ShaderPanel::ShaderPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ShaderPanel)
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	//*)

	_preview = new ShaderPreview( this, ID_CANVAS );
}

ShaderPanel::~ShaderPanel()
{
	//(*Destroy(ShaderPanel)
	//*)
}


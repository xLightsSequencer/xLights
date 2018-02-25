#include "SequenceVideoPanel.h"
#include "VideoReader.h"

//(*InternalHeaders(SequenceVideoPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(SequenceVideoPanel)
//*)

BEGIN_EVENT_TABLE(SequenceVideoPanel,wxPanel)
	//(*EventTable(SequenceVideoPanel)
	//*)
END_EVENT_TABLE()

SequenceVideoPanel::SequenceVideoPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
   : _Path(), _VideoReader(), _IsValidVideo(false)
{
	//(*Initialize(SequenceVideoPanel)
	Create(parent, id, wxDefaultPosition, wxSize(268,203), wxTAB_TRAVERSAL, _T("id"));
	//*)
}

SequenceVideoPanel::~SequenceVideoPanel()
{
	//(*Destroy(SequenceVideoPanel)
	//*)
}

void SequenceVideoPanel::SetMediaPath( const std::string& path )
{
   _VideoReader.reset( new VideoReader( std::string( path ), 0, 0, false, true ) );
   _IsValidVideo = _VideoReader->IsValid();
}

void SequenceVideoPanel::UpdateVideo( int ms )
{
   if ( !_IsValidVideo || !IsShown() )
      return;
}


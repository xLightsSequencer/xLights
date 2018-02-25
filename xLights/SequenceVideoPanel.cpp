#include "SequenceVideoPanel.h"
#include "VideoReader.h"

#include <algorithm>

#include "SequenceVideoPreview.h"

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
   : _Path(), _VideoReader(), _IsValidVideo(false), _VideoWidth(0), _VideoHeight(0), _VideoLength(0), _VideoPreview(nullptr)
{
	//(*Initialize(SequenceVideoPanel)
	Create(parent, id, wxDefaultPosition, wxSize(268,203), wxTAB_TRAVERSAL, _T("id"));
	//*)

   _VideoPreview = new SequenceVideoPreview( this );
}

SequenceVideoPanel::~SequenceVideoPanel()
{
	//(*Destroy(SequenceVideoPanel)
	//*)
}

void SequenceVideoPanel::SetMediaPath( const std::string& path )
{
   _VideoReader.reset( new VideoReader( std::string( path ), 0, 0, false, true ) );
   if ( _VideoReader->IsValid() )
   {
      _IsValidVideo = true;
      _VideoWidth   = _VideoReader->GetWidth();
      _VideoHeight  = _VideoReader->GetHeight();
      _VideoLength  = _VideoReader->GetLengthMS();
   }
}

void SequenceVideoPanel::UpdateVideo( int ms )
{
   if ( !_IsValidVideo || !IsShown() )
      return;

   int clampedTime = std::min( ms, _VideoLength );

   AVFrame *frame = _VideoReader->GetNextFrame( clampedTime );
   if ( frame != nullptr )
   _VideoPreview->Render( frame );
}


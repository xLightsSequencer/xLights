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
   EVT_SIZE( SequenceVideoPanel::Resized )
END_EVENT_TABLE()

SequenceVideoPanel::SequenceVideoPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
   : _path(), _videoReader(), _isValidVideo(false), _videoWidth(0), _videoHeight(0), _videoLength(0), _videoPreview(nullptr)
{
	//(*Initialize(SequenceVideoPanel)
	Create(parent, id, wxDefaultPosition, wxDLG_UNIT(parent,wxSize(160,90)), wxTAB_TRAVERSAL, _T("id"));
	//*)

   _videoPreview = new SequenceVideoPreview( this );
}

SequenceVideoPanel::~SequenceVideoPanel()
{
	//(*Destroy(SequenceVideoPanel)
	//*)
}

void SequenceVideoPanel::SetMediaPath(const std::string& path)
{
    if (!VideoReader::IsVideoFile(path))
    {
        _videoReader.reset();
        _videoWidth = 0;
        _videoHeight = 0;
        _videoLength = 0;
        _isValidVideo = false;
        _videoPreview->Clear();
    }
    else
    {
        _videoReader.reset(new VideoReader(path, 0, 0, false, true, false));
        if (_videoReader->IsValid())
        {
            _isValidVideo = true;
            _videoWidth = _videoReader->GetWidth();
            _videoHeight = _videoReader->GetHeight();
            _videoLength = _videoReader->GetLengthMS();
        }
    }
}

void SequenceVideoPanel::UpdateVideo( int ms )
{
   if ( !_isValidVideo || !IsShown() )
      return;

   int clampedTime = std::min( ms, _videoLength );

   AVFrame *frame = _videoReader->GetNextFrame( clampedTime );
   if ( frame != nullptr )
   _videoPreview->Render( frame );
}

void SequenceVideoPanel::Resized( wxSizeEvent& evt )
{
   if ( _videoPreview )
   {
      _videoPreview->Move( 0, 0 );
      _videoPreview->SetSize( evt.GetSize() );
   }
}


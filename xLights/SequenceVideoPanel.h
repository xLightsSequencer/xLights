#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <memory>
#include <string>

class SequenceVideoPreview;
class VideoReader;

//(*Headers(SequenceVideoPanel)
#include <wx/panel.h>
//*)

class SequenceVideoPanel: public wxPanel
{
	public:

		SequenceVideoPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SequenceVideoPanel();

		//(*Declarations(SequenceVideoPanel)
		//*)

      void SetMediaPath( const std::string& path );
      void UpdateVideo( int currentTime );

	protected:

		//(*Identifiers(SequenceVideoPanel)
		//*)

	private:

		//(*Handlers(SequenceVideoPanel)
		//*)

		DECLARE_EVENT_TABLE()

      void Resized( wxSizeEvent& evt );

      std::string                   _path;
      std::unique_ptr<VideoReader>  _videoReader;
      bool                          _isValidVideo = false;
      int                           _videoWidth = 0;
      int                           _videoHeight = 0;
      int                           _videoLength = 0;
      SequenceVideoPreview *        _videoPreview = nullptr;
};

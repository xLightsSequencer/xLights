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

//(*Headers(PlayerFrame)
#include <wx/frame.h>
//*)

class wxMediaCtrl;

class PlayerFrame: public wxFrame
{
public:

    PlayerFrame(wxWindow* parent, bool topmost, wxWindowID id=wxID_ANY, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize);
    virtual ~PlayerFrame();
    bool Load(const wxString& filename);

    //(*Declarations(PlayerFrame)
    //*)

    void Stop();
    void Pause();
    void Seek(int ms);
    int Tell();
    void Play();
    int GetState();
    
protected:

    //(*Identifiers(PlayerFrame)
    //*)

private:

    //(*Handlers(PlayerFrame)
    void OnClose(wxCloseEvent& event);
    //*)

    bool InitMediaPlayer();
    long wxID_MEDIACTRL;

    wxMediaCtrl* _mediaCtrl = nullptr;
    bool _dragging = false;
    wxPoint _startDragPos;
    wxPoint _startMousePos;

    //void OnMouseLeftUp(wxMouseEvent& event);
    //void OnMouseMove(wxMouseEvent& event);
    //void OnMouseLeftDown(wxMouseEvent& event);

    DECLARE_EVENT_TABLE()
};

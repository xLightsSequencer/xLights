#ifndef PLAYERFRAME_H
#define PLAYERFRAME_H

//(*Headers(PlayerFrame)
#include <wx/frame.h>
//*)

#include <wx/mediactrl.h>   //for wxMediaCtrl

class PlayerFrame: public wxFrame
{
public:

    PlayerFrame(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~PlayerFrame();
    bool Load(const wxString& filename, bool play=false);
    bool Play(const wxString& filename);

    //(*Declarations(PlayerFrame)
    //*)

    void Stop();
    void Pause();
    void Seek(int ms);
    int Tell();
    void Play();
    int GetState();
    void SetPlaybackRate(double playSpeed);
    
    bool ShowPlayerControls(wxMediaCtrlPlayerControls flags = wxMEDIACTRLPLAYERCONTROLS_DEFAULT);
protected:

    //(*Identifiers(PlayerFrame)
    //*)

private:

    //(*Handlers(PlayerFrame)
    void OnClose(wxCloseEvent& event);
    //*)

    void InitMediaPlayer();
    static const long wxID_MEDIACTRL;
    bool PlayAfterLoad;
    void OnMediaLoaded(wxMediaEvent& event);

    wxMediaCtrl* MediaCtrl;
    double playbackSpeed;
    
    DECLARE_EVENT_TABLE()
};

#endif

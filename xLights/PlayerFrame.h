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
    wxMediaCtrl* MediaCtrl;
    bool Load(const wxString& filename);
    bool Play(const wxString& filename);

    //(*Declarations(PlayerFrame)
    //*)

protected:

    //(*Identifiers(PlayerFrame)
    //*)

private:

    //(*Handlers(PlayerFrame)
    void OnClose(wxCloseEvent& event);
    //*)

    static const long wxID_MEDIACTRL;
    bool PlayAfterLoad;
    void OnMediaLoaded(wxMediaEvent& event);

    DECLARE_EVENT_TABLE()
};

#endif

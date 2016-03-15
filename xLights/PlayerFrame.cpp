#include "PlayerFrame.h"
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/icon.h>
#include "../include/xLights.xpm"

// as of wxWidgets 2.8.11, wxMediaCtrl events are messed up on Vista and Win7
// so just avoid them altogether


//(*InternalHeaders(PlayerFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayerFrame)
//*)

const long PlayerFrame::wxID_MEDIACTRL = wxNewId();

BEGIN_EVENT_TABLE(PlayerFrame,wxFrame)
    //(*EventTable(PlayerFrame)
    //*)
END_EVENT_TABLE()

PlayerFrame::PlayerFrame(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    //(*Initialize(PlayerFrame)
    Create(parent, id, _("xPlayer"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);

    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&PlayerFrame::OnClose);
    //*)

    SetIcon(wxIcon(xlights_xpm));

    //
    //  Create and attach the sizer
    //
    wxFlexGridSizer* sizer = new wxFlexGridSizer(1);
    this->SetSizer(sizer);
    this->SetAutoLayout(true);
    sizer->AddGrowableRow(0);
    sizer->AddGrowableCol(0);

    //
    //  Create and attach the media control
    //
    MediaCtrl = new wxMediaCtrl();
    wxString MediaBackend;

#ifdef __WXMSW__
    // this causes Windows to use latest installed Windows Media Player version
    // On XP, users were getting WMP 6.4 without this
    MediaBackend = wxMEDIABACKEND_WMP10;
#endif
    
    playbackSpeed = 1.0;

    //  Make sure creation was successful
    bool bOK = MediaCtrl->Create(this, wxID_MEDIACTRL, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize, wxBORDER_NONE, MediaBackend);

    wxASSERT_MSG(bOK, "Could not create media control!");
    wxUnusedVar(bOK);

    sizer->Add(MediaCtrl, 0, wxALL|wxEXPAND, 0);
    PlayAfterLoad=false;
    Connect(wxID_MEDIACTRL, wxEVT_MEDIA_LOADED,
            wxMediaEventHandler(PlayerFrame::OnMediaLoaded));
}


PlayerFrame::~PlayerFrame()
{
    //(*Destroy(PlayerFrame)
    //*)
}

bool PlayerFrame::Load(const wxString& filename, bool play)
{
    PlayAfterLoad=play;
    return MediaCtrl->Load(filename);
}

bool PlayerFrame::Play(const wxString& filename)
{
    bool result = MediaCtrl->Load(filename);
    if (result) PlayAfterLoad=true;
    return result;
}

void PlayerFrame::OnMediaLoaded(wxMediaEvent& WXUNUSED(evt))
{
    if (PlayAfterLoad) Play();
    PlayAfterLoad=false;
}

void PlayerFrame::OnClose(wxCloseEvent& event)
{
    MediaCtrl->Stop();
    this->Show(false);
}


void PlayerFrame::Stop() {
    MediaCtrl->Stop();
}
void PlayerFrame::Pause() {
    MediaCtrl->Pause();
}
void PlayerFrame::Seek(int ms) {
    MediaCtrl->Seek(ms);
}
int PlayerFrame::Tell() {
    return MediaCtrl->Tell();
}
void PlayerFrame::Play() {
    MediaCtrl->Play();
    MediaCtrl->SetPlaybackRate(playbackSpeed);
}
int PlayerFrame::GetState() {
    return MediaCtrl->GetState();
}

void PlayerFrame::SetPlaybackRate(double playSpeed) {
    this->playbackSpeed = playSpeed;
    if (MediaCtrl->GetState() == wxMEDIASTATE_PLAYING) {
        MediaCtrl->SetPlaybackRate(playSpeed);
    }
}

bool PlayerFrame::ShowPlayerControls(wxMediaCtrlPlayerControls f) {
    return MediaCtrl->ShowPlayerControls(f);
}

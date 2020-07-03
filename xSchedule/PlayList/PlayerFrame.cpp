/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/icon.h>
#include <wx/mediactrl.h>   //for wxMediaCtrl

#include "PlayerFrame.h"

#include "../include/xLights.xpm"

// as of wxWidgets 2.8.11, wxMediaCtrl events are messed up on Vista and Win7
// so just avoid them altogether

//(*InternalHeaders(PlayerFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayerFrame)
//*)

BEGIN_EVENT_TABLE(PlayerFrame,wxFrame)
    //(*EventTable(PlayerFrame)
    //*)
END_EVENT_TABLE()

PlayerFrame::PlayerFrame(wxWindow* parent, bool topmost, wxWindowID id, const wxPoint& pos, const wxSize& size) : _mediaCtrl(nullptr)
{
    wxID_MEDIACTRL = wxNewId();

    //Create(parent, id, _("xPlayer"), wxDefaultPosition, wxDefaultSize, style, _T("id"));

    //(*Initialize(PlayerFrame)
    Create(parent, id, _("xPlayer"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);

    Connect(wxID_ANY, wxEVT_CLOSE_WINDOW, (wxObjectEventFunction)&PlayerFrame::OnClose);
    //*)

    //Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&PlayerFrame::OnMouseLeftDown, 0, this);
    //Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&PlayerFrame::OnMouseLeftUp, 0, this);
    //Connect(wxEVT_MOTION, (wxObjectEventFunction)&PlayerFrame::OnMouseMove, 0, this);

    SetSize(size);
    SetPosition(pos);
    if (topmost) {
        SetWindowStyle(wxNO_BORDER | wxSTAY_ON_TOP);
    }

    SetIcon(wxIcon(xlights_xpm));
}

bool PlayerFrame::InitMediaPlayer() {
    if (_mediaCtrl != nullptr) return true;

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
    _mediaCtrl = new wxMediaCtrl();
    if (_mediaCtrl == nullptr) return false;

    wxString MediaBackend;    
#ifdef __WXMSW__
    // this causes Windows to use latest installed Windows Media Player version
    // On XP, users were getting WMP 6.4 without this
    //MediaBackend = wxMEDIABACKEND_WMP10;
    MediaBackend = wxMEDIABACKEND_DIRECTSHOW;
#endif
    
    //  Make sure creation was successful
    bool bOK = _mediaCtrl->Create(this, wxID_MEDIACTRL, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize, wxBORDER_NONE, MediaBackend);

    if (!bOK) return false;

    sizer->Add(_mediaCtrl, 0, wxALL|wxEXPAND, 0);
    Layout();

    //_mediaCtrl->Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&PlayerFrame::OnMouseLeftDown, 0, this);
    //_mediaCtrl->Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&PlayerFrame::OnMouseLeftUp, 0, this);
    //_mediaCtrl->Connect(wxEVT_MOTION, (wxObjectEventFunction)&PlayerFrame::OnMouseMove, 0, this);

    return true;
}

PlayerFrame::~PlayerFrame()
{
    //(*Destroy(PlayerFrame)
    //*)
}

bool PlayerFrame::Load(const wxString& filename)
{
    if (!InitMediaPlayer()) return false;
    return _mediaCtrl->Load(filename);
}

void PlayerFrame::OnClose(wxCloseEvent& event)
{
    Hide();
    if (!InitMediaPlayer()) return;
    _mediaCtrl->Stop();
}

void PlayerFrame::Stop() {
    if (!InitMediaPlayer()) return;
    _mediaCtrl->Stop();
}

void PlayerFrame::Pause() {
    if (!InitMediaPlayer()) return;
    _mediaCtrl->Pause();
}

void PlayerFrame::Seek(int ms) {
    if (!InitMediaPlayer()) return;
    _mediaCtrl->Seek(ms);
}

int PlayerFrame::Tell() {
    if (!InitMediaPlayer()) return 0;
    return _mediaCtrl->Tell();
}

void PlayerFrame::Play() {
    if (!InitMediaPlayer()) return;
    _mediaCtrl->Play();
    _mediaCtrl->SetVolume(0);
    _mediaCtrl->SetPlaybackRate(1);
}

int PlayerFrame::GetState() {
    if (!InitMediaPlayer()) return wxMEDIASTATE_STOPPED;
    return _mediaCtrl->GetState();
}

//void PlayerFrame::OnMouseLeftUp(wxMouseEvent& event)
//{
//    if (_dragging)
//    {
//        int cwpx, cwpy;
//        GetPosition(&cwpx, &cwpy);
//        wxPoint currentWindowPos(cwpx, cwpy);
//
//        int x = currentWindowPos.x + event.GetPosition().x - _startMousePos.x;
//        int y = currentWindowPos.y + event.GetPosition().y - _startMousePos.y;
//
//        Move(_startDragPos.x + x, _startDragPos.y + y);
//        _dragging = false;
//    }
//}

//void PlayerFrame::OnMouseLeftDown(wxMouseEvent& event)
//{
//    _dragging = true;
//    int x, y;
//    GetPosition(&x, &y);
//    _startDragPos = wxPoint(x, y);
//    _startMousePos = event.GetPosition() + _startDragPos;
//}

//void PlayerFrame::OnMouseMove(wxMouseEvent& event)
//{
//    if (_dragging)
//    {
//        int cwpx, cwpy;
//        GetPosition(&cwpx, &cwpy);
//        wxPoint currentWindowPos(cwpx, cwpy);
//
//        int x = currentWindowPos.x + event.GetPosition().x - _startMousePos.x;
//        int y = currentWindowPos.y + event.GetPosition().y - _startMousePos.y;
//
//        Move(_startDragPos.x + x, _startDragPos.y + y);
//    }
//}

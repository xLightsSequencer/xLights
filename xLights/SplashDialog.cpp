#include "SplashDialog.h"

//(*InternalHeaders(SplashDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <SplashImage.h>
#include <wx/dcclient.h>
#include "xLightsVersion.h"

//(*IdInit(SplashDialog)
//*)

BEGIN_EVENT_TABLE(SplashDialog,wxDialog)
	//(*EventTable(SplashDialog)
	//*)
END_EVENT_TABLE()

SplashDialog::SplashDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(SplashDialog)
	Create(parent, id, _("xLights"), wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP|wxSIMPLE_BORDER|wxSTATIC_BORDER|wxNO_BORDER|wxFULL_REPAINT_ON_RESIZE, _T("id"));
	SetClientSize(wxSize(773,247));
	Move(wxDefaultPosition);
	Center();

	Connect(wxEVT_PAINT,(wxObjectEventFunction)&SplashDialog::OnPaint);
	//*)

    _image = wxBITMAP_PNG_FROM_DATA(xl_xsmall);

    int w = 773;
    int h = 247;
    SetSize(w, h);
}

SplashDialog::~SplashDialog()
{
	//(*Destroy(SplashDialog)
	//*)
}

void SplashDialog::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);

    dc.DrawBitmap(_image, 0, 0);

    wxSize size = GetSize();

    dc.DrawText(xlights_version_string + " " + GetBitness(), wxPoint(size.GetWidth() - 100, 10));
    dc.DrawText("www.xlights.org", wxPoint(10, size.GetHeight() - 25));
    dc.DrawText("videos.xlights.org", wxPoint(170, size.GetHeight() - 25));
    dc.DrawText("https://www.facebook.com/groups/628061113896314/", wxPoint(450, size.GetHeight() - 25));

    wxDialog::OnPaint(event);
}

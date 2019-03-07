#include "SplashDialog.h"

//(*InternalHeaders(SplashDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dcclient.h>
#include "xLightsVersion.h"
#include "UtilFunctions.h"
#include <log4cpp/Category.hh>

#include "wx/artprov.h"

//(*IdInit(SplashDialog)
//*)

BEGIN_EVENT_TABLE(SplashDialog,wxDialog)
	//(*EventTable(SplashDialog)
	//*)
END_EVENT_TABLE()

SplashDialog::SplashDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(SplashDialog)
	Create(parent, id, _("xLights"), wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER|wxSTATIC_BORDER|wxNO_BORDER|wxFULL_REPAINT_ON_RESIZE, _T("id"));
	SetClientSize(wxSize(773,247));
	Move(wxDefaultPosition);
	Center();

	Connect(wxEVT_PAINT,(wxObjectEventFunction)&SplashDialog::OnPaint);
	//*)

    SetIcons(wxArtProvider::GetIconBundle("xlART_xLights_Icons", wxART_FRAME_ICON));

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Loading splash image.");
    _image = wxArtProvider::GetBitmap("xlART_xLights_SlashImage", wxART_OTHER);
    logger_base.debug("Splash loaded. IsOk %s, %dx%d", _image.IsOk() ? "TRUE" : "FALSE", _image.GetWidth(), _image.GetHeight());

    wxSize sz = _image.GetScaledSize();
    int w = sz.GetWidth();
    int h = sz.GetHeight();
    SetSize(w, h);
    Center();
}

SplashDialog::~SplashDialog()
{
	//(*Destroy(SplashDialog)
	//*)
}

void SplashDialog::OnPaint(wxPaintEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Splash painting.");

    wxPaintDC dc(this);
    dc.SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    dc.DrawBitmap(_image, 0, 0);

    wxSize size = GetSize();
    
    int scl = 1;
#ifndef __WXOSX__
    if (GetSystemContentScaleFactor() > 1.5) {
        scl = 2;
    }
#endif
    wxString ver = GetDisplayVersionString();
    int w, h, descent;
    
    dc.GetTextExtent(ver, &w, &h, &descent);
    dc.DrawText(ver, wxPoint(size.GetWidth() - w - 10*scl, h - descent));
    dc.DrawText("www.xlights.org", wxPoint(10*scl, size.GetHeight() - 10 - h ));
    dc.DrawText("videos.xlights.org", wxPoint(170*scl, size.GetHeight() - 10 - h));
    dc.DrawText("www.xlights.org/facebook", wxPoint(450*scl, size.GetHeight() - 10 - h));
}

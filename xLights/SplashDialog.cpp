#include "SplashDialog.h"

//(*InternalHeaders(SplashDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <splashimage.h>
#include <wx/dcclient.h>
#include "xLightsVersion.h"
#include <log4cpp/Category.hh>

#include "../include/xLights.xpm"
#include "../include/xLights-16.xpm"
#include "../include/xLights-32.xpm"
#include "../include/xLights-64.xpm"
#include "../include/xLights-128.xpm"

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

    wxIconBundle icons;
    icons.AddIcon(wxIcon(xlights_16_xpm));
    icons.AddIcon(wxIcon(xlights_32_xpm));
    icons.AddIcon(wxIcon(xlights_64_xpm));
    icons.AddIcon(wxIcon(xlights_128_xpm));
    icons.AddIcon(wxIcon(xlights_xpm));

    SetIcons(icons);

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Loading splash image.");
    _image = wxBITMAP_PNG_FROM_DATA(xl_xsmall);
    logger_base.debug("Splash loaded. IsOk %s, %dx%d", _image.IsOk() ? "TRUE" : "FALSE", _image.GetWidth(), _image.GetHeight());

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
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Splash painting.");

    wxPaintDC dc(this);
    dc.SetFont(wxFont(wxSize(0, 12), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    dc.DrawBitmap(_image, 0, 0);

    wxSize size = GetSize();

    dc.DrawText(xlights_version_string + " " + GetBitness(), wxPoint(size.GetWidth() - 100, 10));
    dc.DrawText("www.xlights.org", wxPoint(10, size.GetHeight() - 25));
    dc.DrawText("videos.xlights.org", wxPoint(170, size.GetHeight() - 25));
    dc.DrawText("https://www.facebook.com/groups/628061113896314/", wxPoint(450, size.GetHeight() - 25));
}

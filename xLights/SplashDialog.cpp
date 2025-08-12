/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SplashDialog.h"

//(*InternalHeaders(SplashDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dcclient.h>
#include "xLightsVersion.h"
#include "UtilFunctions.h"
#include "./utils/spdlog_macros.h"

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

    

    LOG_DEBUG("Loading splash image.");
    _image = wxArtProvider::GetBitmap("xlART_xLights_SlashImage", wxART_OTHER);
    wxSize sz = _image.GetPreferredBitmapSizeFor(this);

    LOG_DEBUG("Splash loaded. IsOk %s, %dx%d", _image.IsOk() ? "TRUE" : "FALSE", sz.GetWidth(), sz.GetHeight());

    int w = FromPhys(sz.GetWidth());
    int h = FromPhys(sz.GetHeight());
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
    
    LOG_DEBUG("Splash painting.");

    wxPaintDC dc(this);
    dc.SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    wxBitmap bmp = _image.GetBitmapFor(this);
    dc.DrawBitmap(bmp, 0, 0);
    
    wxSize size = GetSize();
    
    wxString ver = GetDisplayVersionString();
    int w, h, descent;
    
    dc.GetTextExtent(ver, &w, &h, &descent);
    dc.DrawText(ver, wxPoint(size.GetWidth() - w - FromDIP(10), h - descent));
    dc.DrawText("www.xlights.org", wxPoint(FromDIP(10), size.GetHeight() - 10 - h));
    dc.DrawText("videos.xlights.org", wxPoint(FromDIP(170), size.GetHeight() - 10 - h));

    wxString fb = "www.xlights.org/facebook"; 
    dc.GetTextExtent(fb, &w, &h, &descent);
    dc.DrawText(fb, wxPoint(size.GetWidth() - w - FromDIP(10), size.GetHeight() - 10 - h));
}

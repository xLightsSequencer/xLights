/////////////////////////////////////////////////////////////////////////////
// Name:        led.cpp
// Purpose:     wxLed implementation
// Author:      Thomas Monjalon
// Created:     09/06/2005
// Revision:    09/06/2005
// Licence:     wxWidgets
/////////////////////////////////////////////////////////////////////////////

#include "wxLED.h"

#include <string.h>

BEGIN_EVENT_TABLE (wxLed, wxWindow)
    EVT_PAINT (wxLed::OnPaint)
END_EVENT_TABLE ()

wxLed::wxLed (wxWindow * parent, wxWindowID id, const char * disabledColor, const wxPoint & pos, const wxSize & size)
:
	wxWindow (parent, id, pos, size),
	m_bitmap (nullptr)
{
	this->m_isEnabled = false ;
	strncpy (this->m_enabledColor, "FFFFFF", 7) ;
	strncpy (this->m_disabledColor, disabledColor, 7) ;
	* this->m_color = '\0' ;
	this->SetBitmap(this->m_disabledColor) ;
}

wxLed::~wxLed ()
{
	delete this->m_bitmap ;
}

bool wxLed::Enable (bool enable)
{
	bool status = wxWindow::Enable (enable) ;
	this->SetBitmap (this->m_isEnabled ? this->m_enabledColor : this->m_disabledColor) ;
	return status ;
}

void wxLed::SetColor (const char * rgb)
{
	if (strlen (rgb) == 6 && strncmp (this->m_enabledColor, rgb, 6))
	{
		strncpy (this->m_enabledColor, rgb, 7) ;
		if (this->m_isEnabled)
			this->SetBitmap (this->m_enabledColor) ;
	}
}

void wxLed::OnPaint (wxPaintEvent & WXUNUSED (event))
{
	wxPaintDC dc (this) ;
	this->m_mutex.Lock () ;
	dc.DrawBitmap (* this->m_bitmap, 0, 0, true) ;
	this->m_mutex.Unlock () ;
}

#define WX_LED_WIDTH    17
#define WX_LED_HEIGHT   17
#define WX_LED_COLORS    5
#define WX_LED_XPM_COLS    (WX_LED_WIDTH + 1)
#define WX_LED_XPM_LINES   (1 + WX_LED_COLORS + WX_LED_HEIGHT)

void wxLed::SetBitmap (const char * color)
{
	if (strncmp (this->m_color, color, 6))
	{
		char ** xpm = NULL ;
		char * xpmData = NULL ;
		xpm = new char * [WX_LED_XPM_LINES] ;
		if (xpm == NULL)
			goto end ;
		xpmData = new char [WX_LED_XPM_LINES * WX_LED_XPM_COLS] ;
		if (xpmData == NULL)
			goto end ;
		for (int i = 0 ; i < WX_LED_XPM_LINES ; i ++)
			xpm [i] = xpmData + i * WX_LED_XPM_COLS ;
		// width height num_colors chars_per_pixel
		sprintf (xpm [0], "%d %d %d 1", WX_LED_WIDTH, WX_LED_HEIGHT, WX_LED_COLORS/*, WX_LED_XPM_COLS*/) ;
		// colors
		strncpy (xpm [1], "  c None", WX_LED_XPM_COLS) ;
		strncpy (xpm [2], "- c #C0C0C0", WX_LED_XPM_COLS) ;
		strncpy (xpm [3], "_ c #F8F8F8", WX_LED_XPM_COLS) ;
		strncpy (xpm [4], "* c #FFFFFF", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS], "X c #", WX_LED_XPM_COLS) ;
		strncpy ((xpm [WX_LED_COLORS]) + 5, color, 7) ;
		// pixels
		strncpy (xpm [WX_LED_COLORS +  1], "      -----      ", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS +  2], "    ---------    ", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS +  3], "   -----------   ", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS +  4], "  -----XXX----_  ", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS +  5], " ----XX**XXX-___ ", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS +  6], " ---X***XXXXX___ ", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS +  7], "----X**XXXXXX____", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS +  8], "---X**XXXXXXXX___", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS +  9], "---XXXXXXXXXXX___", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS + 10], "---XXXXXXXXXXX___", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS + 11], "----XXXXXXXXX____", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS + 12], " ---XXXXXXXXX___ ", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS + 13], " ---_XXXXXXX____ ", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS + 14], "  _____XXX_____  ", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS + 15], "   ___________   ", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS + 16], "    _________    ", WX_LED_XPM_COLS) ;
		strncpy (xpm [WX_LED_COLORS + 17], "      _____      ", WX_LED_XPM_COLS) ;
		this->m_mutex.Lock () ;
		delete this->m_bitmap ;
		this->m_bitmap = new wxBitmap (xpm) ;
		if (this->m_bitmap == NULL)
		{
			this->m_mutex.Unlock () ;
			goto end ;
		}
		this->SetSize (wxSize (this->m_bitmap->GetWidth (), this->m_bitmap->GetHeight ())) ;
		this->m_mutex.Unlock () ;
		this->Refresh () ;
		strncpy (this->m_color, color, 7) ;
	end :
		delete [] xpm ;
		delete [] xpmData ;
	}
}

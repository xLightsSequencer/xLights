/***************************************************************
 * Name:      xLightsApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-11-03
 * Copyright: Matt Brown ()
 * License:
 **************************************************************/

#include "xLightsApp.h"

//(*AppHeaders
#include "xLightsMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(xLightsApp)

bool xLightsApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	xLightsFrame* Frame = new xLightsFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}

//re-added global keyboard handler: -DJ
//couldn't get PanelSequence2 keyboard handler to work, so just use a global filter as suggested in
// http://wiki.wxwidgets.org/Catching_key_events_globally
//also, there are some issues with controls in other panels
#if 1
int xLightsApp::FilterEvent(wxEvent& event)
{
    if (event.GetEventType() == wxEVT_KEY_DOWN)
//        switch ((int)(((wxKeyEvent&)event).GetKeyCode()))
        {
//        case WXK_F5:
            xLightsFrame* Frame = (xLightsFrame*)GetTopWindow();
//            wxMessageBox(wxString::Format(wxT("tab %d, You pressed 0x%x = 0x%x vs 0x%x"), Frame->Notebook1->GetSelection(), ((wxKeyEvent&)event).GetUnicodeKey(), ((wxKeyEvent&)event).GetKeyCode(), WXK_F5));
//            if (Frame->Notebook1->GetSelection() == SEQUENCETAB) //Nutcracker tab
//            {
//                Frame->OnPanelSequence2KeyDown((wxKeyEvent&)event);
                Frame->OnPanelSequence2Char((wxKeyEvent&)event);
                return true;
//            }
//            break;

//add more cases here:
//        case WXK_F1:
        }
    return -1;
}
#endif

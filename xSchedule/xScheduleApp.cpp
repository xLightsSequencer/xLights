/***************************************************************
 * Name:      xScheduleApp.cpp
 * Purpose:   Code for Application Class
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#include "xScheduleApp.h"

//(*AppHeaders
#include "xScheduleMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(xScheduleApp)

bool xScheduleApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	xScheduleFrame* Frame = new xScheduleFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}

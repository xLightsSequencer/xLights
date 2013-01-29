/***************************************************************
 * Name:      nutcrackerApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Sean Meighan (sean.meighan@oracle.com)
 * Created:   2013-01-28
 * Copyright: Sean Meighan (nutcracker123.com/nutcracker)
 * License:
 **************************************************************/

#include "wx_pch.h"
#include "nutcrackerApp.h"

//(*AppHeaders
#include "nutcrackerMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(nutcrackerApp);

bool nutcrackerApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	nutcrackerFrame* Frame = new nutcrackerFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}

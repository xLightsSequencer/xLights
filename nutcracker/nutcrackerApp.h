/***************************************************************
 * Name:      nutcrackerApp.h
 * Purpose:   Defines Application Class
 * Author:    Sean Meighan (sean.meighan@oracle.com)
 * Created:   2013-01-28
 * Copyright: Sean Meighan (nutcracker123.com/nutcracker)
 * License:
 **************************************************************/

#ifndef NUTCRACKERAPP_H
#define NUTCRACKERAPP_H

#include <wx/app.h>

class nutcrackerApp : public wxApp
{
    public:
        virtual bool OnInit();
};

#endif // NUTCRACKERAPP_H

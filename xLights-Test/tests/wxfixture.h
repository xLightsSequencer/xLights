#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "gtest/gtest.h"

#include "wx/app.h"

struct IP_Host_Tests : public ::testing::Test, wxApp 
{
    wxInitializer init;
    IP_Host_Tests() {
        wxApp::SetInstance(new wxApp);
        if ( !init.IsOk() ) {
            printf("Failed to initialize wxWidgets.\n");
        }
    }
};
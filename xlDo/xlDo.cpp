/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/app.h>

#include "../xLights/automation/automation.h"

int main(int argc, char **argv)
{
    wxApp::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "program");

    #ifdef __WXMSW__
    {
        // Turn on coloured console handling
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) {
            fprintf(stderr, "Error setting console mode.\n");
            return 1;
        }

        DWORD dwMode = 0;
        if (!GetConsoleMode(hOut, &dwMode)) {
            fprintf(stderr, "Error setting console mode.\n");
            return 1;
        }

        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if (!SetConsoleMode(hOut, dwMode)) {
            fprintf(stderr, "Error setting console mode.\n");
            return 1;
        }
    }
    #endif
    
    wxInitializer initializer;
    if (!initializer) {
        fprintf(stderr, "\u001b[31;1mFailed to initialize the wxWidgets library, aborting.\u001b[0m\n");
        return 1;
    }

    return DoXLDoCommands(argc, argv);
}

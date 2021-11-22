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
    
    return DoXLDoCommands(argc, argv)
}

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "pch.h"

#include <wx/string.h>
#include "../xLights/xLightsVersion.h"

#ifdef _MSC_VER
#ifdef _DEBUG
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "ud.lib")
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "ud_net.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_core.lib")
    #pragma comment(lib, "wxscintillad.lib")
    #pragma comment(lib, "wxregexud.lib")
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "ud_xml.lib")
    #pragma comment(lib, "wxtiffd.lib")
    #pragma comment(lib, "wxjpegd.lib")
    #pragma comment(lib, "wxpngd.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_aui.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_gl.lib")
    #pragma comment(lib, "wxzlibd.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_qa.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_html.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_propgrid.lib")
    #pragma comment(lib, "wxexpatd.lib")
    #pragma comment(lib, "log4cppLIBd.lib")
#else
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "u.lib")
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "u_net.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_core.lib")
    #pragma comment(lib, "wxscintilla.lib")
    #pragma comment(lib, "wxregexu.lib")
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "u_xml.lib")
    #pragma comment(lib, "wxtiff.lib")
    #pragma comment(lib, "wxjpeg.lib")
    #pragma comment(lib, "wxpng.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_aui.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_gl.lib")
    #pragma comment(lib, "wxzlib.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_qa.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_html.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_propgrid.lib")
    #pragma comment(lib, "wxexpat.lib")
    #pragma comment(lib, "log4cppLIB.lib")

#endif
#pragma comment(lib, "libcurl.dll.a")
#endif


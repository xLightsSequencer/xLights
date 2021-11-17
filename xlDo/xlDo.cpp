// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/app.h>
#include <wx/cmdline.h>
#include <wx/file.h>
#include <wx/sckaddr.h>
#include <wx/socket.h>
#include "../xSchedule/wxJSON/jsonreader.h"

static const wxCmdLineEntryDesc cmdLineDesc[] =
{
    { wxCMD_LINE_SWITCH, "h", "help", "show this help message",
        wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
    { wxCMD_LINE_SWITCH, "v", "verbose", "Verbose messages to stderr.",
      wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_SWITCH, "A", "xLightsA", "Connect to xLights listening on the A port.",
      wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_SWITCH, "B", "xLightsB", "Connect to xLights listening on the B port.",
      wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },

    { wxCMD_LINE_OPTION, "i", "IPAddress", "IP address of the machine running xLights.",
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },

    { wxCMD_LINE_OPTION, "t", "TemplateFile", "File containing the template for the request.",
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },

    { wxCMD_LINE_OPTION, "c", "Command", "The command to send.",
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },

    { wxCMD_LINE_OPTION, "p1", "Parameter1", "First template substitution parameter.",
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },
    { wxCMD_LINE_OPTION, "p2", "Parameter2", "Second template substitution parameter.",
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },
    { wxCMD_LINE_OPTION, "p3", "Parameter3", "Third template substitution parameter.",
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },
    { wxCMD_LINE_OPTION, "p4", "Parameter4", "Fourth template substitution parameter.",
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },
    { wxCMD_LINE_OPTION, "p5", "Parameter5", "Fifth template substitution parameter.",
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },
    { wxCMD_LINE_OPTION, "p6", "Parameter6", "Sixth template substitution parameter.",
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },
    { wxCMD_LINE_OPTION, "p7", "Parameter7", "Seventh template substitution parameter.",
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },
    { wxCMD_LINE_OPTION, "p8", "Parameter8", "Eighth template substitution parameter.",
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },
    { wxCMD_LINE_OPTION, "p9", "Parameter9", "Ninth template substitution parameter.",
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },

    wxCMD_LINE_DESC_END
};

int GetxFadePort(int xfp)
{
    if (xfp == 0)
        return 0;
    return xfp + 49912;
}

wxString xLightsRequest(int xFadePort, wxString message, wxString ipAddress, bool verbose)
{
    wxSocketClient socket;
    wxIPV4address addr;
    addr.Hostname(ipAddress);
    addr.Service(GetxFadePort(xFadePort + 1));

    if (socket.Connect(addr)) {

        if (verbose) {
            fprintf(stderr, "\u001b[36;1mConnected to xLights.\u001b[0m\n");
        }

        socket.WriteMsg(message.c_str(), message.size() + 1);
        uint8_t buffer[4096];
        memset(buffer, 0x00, sizeof(buffer));
        int read = 0;
        while (read == 0 && socket.IsConnected()) {
            socket.ReadMsg(buffer, sizeof(buffer) - 1);
            read = socket.LastReadCount();
            if (read == 0)
                wxMilliSleep(2);
        }
        wxString msg;
        if (socket.IsConnected()) {
            msg = wxString((char*)buffer);
            if (verbose) {
                fprintf(stderr, "\u001b[32;1mResponse: %s\u001b[0m\n", (const char*)msg.c_str());
            }
        } else {
            fprintf(stderr, "\u001b[31;1mFailed to get response from xLights.\u001b[0m\n");
            return "";
        }
        return msg;
    } else {
        fprintf(stderr, "\u001b[31;1mCannot connect to xLights.\u001b[0m\n");
        return "";
    }
}

int main(int argc, char **argv)
{
    wxApp::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "program");

    bool verbose = false;
    wxString ip = "127.0.0.1";
    int ab = 0; // A
    wxString templateFile;
    wxString command;
    std::vector<wxString> parameters;
    parameters.resize(9);

    wxInitializer initializer;
    if ( !initializer )
    {
        fprintf(stderr, "\u001b[31;1mFailed to initialize the wxWidgets library, aborting.\u001b[0m\n");
        return 1;
    }

    wxCmdLineParser parser(cmdLineDesc, argc, argv);
    switch ( parser.Parse() )
    {
        case -1:
            // help was given, terminating
            break;

        case 0:
            // everything is ok; proceed
            if (parser.Found("v"))
            {
                verbose = true;
            }
            if (parser.Found("A") && parser.Found("B")) {
                fprintf(stderr, "\u001b[31;1mCannot specify both -A and -B.\u001b[0m\n");
                return 1;
            }

            if (parser.Found("A")) {
                ab = 0;
                if (verbose) {
                    fprintf(stderr, "\u001b[36;1mConnecting to xLights A.\u001b[0m\n");
                }
            }
            else if (parser.Found("B")) {
                ab = 1;
                if (verbose) {
                    fprintf(stderr, "\u001b[36;1mConnecting to xLights B.\u001b[0m\n");
                }
            }

            if (parser.Found("i", &ip)) {
                if (verbose) {
                    fprintf(stderr, "\u001b[36;1mConnecting to xLights on ip: %s.\u001b[0m\n", (const char*)ip.c_str());
                }
            }

            if (parser.Found("t") && parser.Found("c")) {
                fprintf(stderr, "\u001b[31;1mCannot specify both -t and -c.\u001b[0m\n");
            }

            if (parser.Found("t", &templateFile)) {
                if (verbose) {
                    fprintf(stderr, "\u001b[36;1mTemplate file: %s.\u001b[0m\n", (const char*)templateFile.c_str());
                }
            }
            else if (parser.Found("c", &command)) {
                if (verbose) {
                    fprintf(stderr, "\u001b[36;1mCommand: %s.\u001b[0m\n", (const char*)command.c_str());
                }
            } else {
                if (verbose) {
                    fprintf(stderr, "\u001b[36;1mReading command from stdin.\u001b[0m\n");
                }
            }

            for (uint8_t i = 0; i < 9; i++) {
                if (parser.Found(wxString::Format("p%d", i + 1), &parameters[i])) {
                    if (verbose) {
                        fprintf(stderr, "\u001b[36;1mParameter %d: %s.\u001b[0m\n", i + 1, (const char*)parameters[i].c_str());
                    }
                }
            }

            break;

        default:
            break;
    }

    if (templateFile != "") {
    
        if (!wxFile::Exists(templateFile)) {
            fprintf(stderr, "\u001b[31;1mTemplate file %s not found.\u001b[0m\n", (const char*)templateFile.c_str());
            return 1;
        }
        
        wxFile f;
        if (f.Open(templateFile)) {
            f.ReadAll(&command);
            if (verbose) {
                fprintf(stderr, "\u001b[36;1mCommand read from template file: %s.\u001b[0m\n", (const char*)command.c_str());
            }
        } else {
            fprintf(stderr, "\u001b[31;1mUnable to open template file %s.\u001b[0m\n", (const char*)templateFile.c_str());
            return 1;
        }
    } else if (command == "") {
        char s[4096];
        memset(s, 0x00, sizeof(s));
        fscanf(stdin, "%s", s);
        command = wxString(s);
        if (verbose) {
            fprintf(stderr, "\u001b[36;1mCommand read from stdin: %s.\u001b[0m\n", (const char*)command.c_str());
        }
    }

    if (command == "") {
        fprintf(stderr, "\u001b[31;1mNo command found to execute.\u001b[0m\n");
        return 1;
    }

    for (uint8_t i = 0; i < 9; i++) {
        command.Replace(wxString::Format("%%P%d", i + 1), parameters[i], true);
    }

    if (verbose) {
        fprintf(stderr, "\u001b[32;1mCommand after parameter replacement: %s.\u001b[0m\n", (const char*)command.c_str());
    }

    wxString resp = xLightsRequest(ab, command, ip, verbose);

    if (resp == "") {
        return 1;
    } else {
        wxJSONValue val;
        wxJSONReader reader;
        if (reader.Parse(resp, &val) == 0) {
            if (!val.HasMember("res")) {
                fprintf(stderr, "\u001b[31;1mxLights response missing result code: %s.\u001b[0m\n", (const char*)resp.c_str());
                return 1;
            } else {
                auto res = val["res"].AsLong();
                if (res != 200) {
                    fprintf(stderr, "\u001b[31;1mxLights response has error code: %d.\u001b[0m\n", res);
                    return 2;
                }
            }
        } else {
            fprintf(stderr, "\u001b[31;1mError parsing xLights response.\u001b[0m\n");
            return 1;
        }
    }

    wxPrintf(resp);
    return 0;
}

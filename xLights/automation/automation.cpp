/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "automation.h"

#include "wx/wx.h"
#include <wx/cmdline.h>

#include <wx/file.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include "../../xSchedule/wxJSON/jsonreader.h"
#include "../utils/Curl.h"
#include <wx/sckaddr.h>
#include "../UtilFunctions.h"

#ifdef xlDO
static int GetxFadePort(int xfp)
{
    if (xfp == 0)
        return 0;
    return xfp + 49912;
}
#endif

void xlDo_Output(const std::string& script, const std::string& resp, bool verbose, bool isJson)
{
    if (script != "") {
        wxFile f(script, wxFile::write);

        if (f.IsOpened()) {
            if (isJson) {
                wxJSONValue val;
                wxJSONReader reader;
                if (reader.Parse(resp, &val) == 0) {
                    for (const auto& it : val.GetMemberNames()) {
                        std::string set = "@set";
                        #ifndef __WXMSW__
                            set = "EXPORT";
                        #endif
                        wxString c;
                        if (val[it].IsString()) {
                            c = wxString::Format("%s %s=%s\n", set, it, val[it].AsString());
                        } else if (val[it].IsLong()) {
                            c = wxString::Format("%s %s=%ld\n", set, it, val[it].AsLong());
                        }
                        f.Write(c);
                        if (verbose)
                            fprintf(stderr, "\u001b[36;1m%s\u001b[0m", (const char*)c.c_str());
                    }
                } else {
                    fprintf(stderr, "\u001b[31;1mFailed to parse response %s.\u001b[0m\n", (const char*)resp.c_str());
                }
            } else {
                f.Write(resp);
            }
        } else {
            fprintf(stderr, "\u001b[31;1mFailed to write to script file %s.\u001b[0m\n", (const char*)script.c_str());
        }
    }

    wxPrintf("%s\n", resp.c_str());
}

int Automation(bool verbose, const std::string& ip, int ab, const std::string& templateFile, const std::string& cmd, const std::vector<wxString>& parameters, const std::string& script)
{
    wxString command(cmd);

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
        fgets(s, sizeof(s) - 1, stdin);
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
        if (i >= parameters.size()) {
            break;
        }
        command.Replace(wxString::Format("%%P%d", i + 1), parameters[i], true);
    }

    if (verbose) {
        fprintf(stderr, "\u001b[32;1mCommand after parameter replacement: %s.\u001b[0m\n", (const char*)command.c_str());
    }

    {
        wxJSONValue val;
        wxJSONReader reader;
        if (reader.Parse(command, &val) == 0) {
            if (val["cmd"].AsString() == "startxLights") {
                std::string params = " -a";
                if (ab == 1)
                    params = " -b";

                std::string url = "http://" + ip + ":" + std::to_string(::GetxFadePort(ab + 1)) + "/getVersion";
                if (val["ifNotRunning"].AsString() == "true") {
                    std::string resp = Curl::HTTPSGet(url, command.ToStdString());
                    if (resp != "") {
                        xlDo_Output(script, "{\"res\":200,\"msg\":\"xLights was already running.\"}", verbose, false);
                        return 0;
                    }
                }

#ifdef LINUX
                wxFileName f(wxStandardPaths::Get().GetExecutablePath());
                wxString appPath(f.GetPath());
                wxString cmdline(appPath + wxT("/xLights") + params + " &");
                
                if (system((const char *)cmdline.c_str()) < 0) {
                    fprintf(stderr, "\u001b[31;1mUnable to start xLights.\u001b[0m\n");
                    return 1;
                }
#elif defined(__WXOSX__)
                std::string p = ToUTF8(wxStandardPaths::Get().GetExecutablePath());
                int idx = p.find_last_of('/');
                p = p.substr(0, idx);
                idx = p.find_last_of('/');
                p = p.substr(0, idx);
                idx = p.find_last_of('/');
                p = p.substr(0, idx);
                wxString s = "open -a " + p + " --args " + params;
                if (system(s.c_str()) < 0) {
                    fprintf(stderr, "\u001b[31;1mUnable to start xLights.\u001b[0m\n");
                    return 1;
                }
#else
                long pid = wxExecute("xLights.exe" + params);
                if (pid == 0) {
                    fprintf(stderr, "\u001b[31;1mUnable to start xLights.\u001b[0m\n");
                    return 1;
                }
#endif


                int loop = 0;
                std::string resp = Curl::HTTPSGet(url, command.ToStdString());
                while (resp == "") {
                    // dont wait more than a minute
                    if (loop++ > 60)
                        break;

                    wxSleep(1);
                    resp = Curl::HTTPSGet(url, command.ToStdString());
                }

                if (loop > 60) {
                    fprintf(stderr, "\u001b[31;1mTimeout waiting for xLights to start.\u001b[0m\n");
                    return 1;
                }

                xlDo_Output(script, "{\"res\":200,\"msg\":\"xLights started.\"}", verbose, true);

                wxSleep(5); // sleep briefly to give xLights a chance to start properly so any subsequent commands work ok

                return 0;
            }
        }
    }
    std::string url = "http://" + ip + ":" + std::to_string(::GetxFadePort(ab + 1)) + "/";
    std::string mime = "text/plain";
    bool isJsonResp = false;
    if (command[0] == '{') {
        url += "xlDoAutomation";
        mime = "application/json";
        isJsonResp = true;
    } else {
        url += ToUTF8(command);
        command = "";
    }
    //30 minute timeout?  Some of the automations like batchRender may take a LONG time
    int responseCode = 0;
    std::string resp = command == "" ? Curl::HTTPSGet(url, "", "", 30*60, {}, &responseCode)
        : Curl::HTTPSPost(url, command, "", "", mime, 30*60, {}, &responseCode);


    if (resp == "") {
        return 1;
    } else if (isJsonResp) {
        wxJSONValue val;
        wxJSONReader reader;
        if (reader.Parse(resp, &val) == 0) {
            if (!val.HasMember("res") && responseCode != 0) {
                val["res"] = (long)responseCode;
                resp = "{\"res\":" + std::to_string(responseCode) + "," + resp.substr(1);
            }
            if (!val.HasMember("res")) {
                fprintf(stderr, "\u001b[31;1mxLights response missing result code: %s.\u001b[0m\n", (const char*)resp.c_str());
                return 1;
            } else {
                auto res = val["res"].AsLong();
                if (res != 200) {
                    fprintf(stderr, "\u001b[31;1mxLights response has error code: %d.\u001b[0m %s\n", (int)res, (const char*)resp.c_str());
                    return 2;
                }
            }
        } else {
            fprintf(stderr, "\u001b[31;1mError parsing xLights response.\u001b[0m\n");
            return 1;
        }
    }
    xlDo_Output(script, resp, verbose, isJsonResp);


    return 0;
}



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

    { wxCMD_LINE_OPTION, "s", "script", "Output a script file to access values.",
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



int DoXLDoCommands(int argc, char **argv) {
    bool verbose = false;
    wxString ip = "127.0.0.1";
    int ab = 0; // A
    wxString templateFile;
    wxString command;
    std::vector<wxString> parameters;
    parameters.resize(9);
    wxString script;
    
    wxMessageOutput::Set(new wxMessageOutputStderr);
    wxCmdLineParser parser(cmdLineDesc, argc, argv);
    switch ( parser.Parse() )
    {
        case -1:
            // help was given, terminating
            return 0;

        case 0:
            // everything is ok; proceed
            if (parser.Found("v")) {
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

            if (parser.Found("s", &script)) {
                if (verbose) {
                    fprintf(stderr, "\u001b[36;1mCreating script: %s.\u001b[0m\n", (const char*)script.c_str());
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

    return Automation(verbose, ip.ToStdString(), ab, templateFile.ToStdString(), command.ToStdString(), parameters, script.ToStdString());
}

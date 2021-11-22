#include "automation.h"

#include "wx/wx.h"

#include <wx/file.h>
#include <wx/sckaddr.h>
#include <wx/socket.h>
#include "../xSchedule/wxJSON/jsonreader.h"

#ifdef xlDO
int GetxFadePort(int xfp)
{
    if (xfp == 0)
        return 0;
    return xfp + 49912;
}
#endif

wxString xlDo_xLightsRequest(int xFadePort, wxString message, wxString ipAddress, bool verbose)
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

void xlDo_Output(const std::string& script, const std::string& resp, bool verbose)
{
    if (script != "") {
        wxFile f(script, wxFile::write);

        if (f.IsOpened()) {

            wxJSONValue val;
            wxJSONReader reader;
            if (reader.Parse(resp, &val) == 0) {
                for (const auto& it : val.GetMemberNames()) {
                    std::string set = "set";
                    #ifndef __WXMSW__
                        set = "EXPORT";
                    #endif
                    std::string c;
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
            fprintf(stderr, "\u001b[31;1mFailed to write to script file %s.\u001b[0m\n", (const char*)script.c_str());
        }
    }

    wxPrintf(resp.c_str());
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

                if (val["ifNotRunning"].AsString() == "true") {
                    if (xlDo_xLightsRequest(ab, "{\"cmd\":\"getVersion\"}", ip, verbose) != "") {
                        xlDo_Output(script, "{\"res\":200,\"msg\":\"xLights was already running.\"}", verbose);
                        return 0;
                    }
                }

                long pid = 0;
#ifdef LINUX
                // Handle xschedule not in path
                wxFileName f(wxStandardPaths::Get().GetExecutablePath());
                wxString appPath(f.GetPath());
                wxString cmdline(appPath + wxT("/xLights") + params);
                pid = wxExecute(cmdline, wxEXEC_ASYNC, NULL, NULL);
#else
                pid = wxExecute("xLights.exe" + params);
#endif

                if (pid == 0) {
                    fprintf(stderr, "\u001b[31;1mUnable to start xLights.\u001b[0m\n");
                    return 1;
                }

                int loop = 0;
                while (xlDo_xLightsRequest(ab, "{\"cmd\":\"getVersion\"}", ip, verbose) == "") {
                    // dont wait more than a minute
                    if (loop++ > 60)
                        break;

                    wxSleep(1);
                }

                if (loop > 60) {
                    fprintf(stderr, "\u001b[31;1mTimeout waiting for xLights to start.\u001b[0m\n");
                    return 1;
                }

                xlDo_Output(script, "{\"res\":200,\"msg\":\"xLights started.\"}", verbose);
                return 0;
            }
        }
    }

    wxString resp = xlDo_xLightsRequest(ab, command, ip, verbose);

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

    xlDo_Output(script, resp, verbose);

    return 0;
}

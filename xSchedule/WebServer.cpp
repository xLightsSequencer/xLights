/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/dir.h> // Linux needs this
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/uri.h>

#include "WebServer.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"
#include "xScheduleApp.h"
#include "ScheduleOptions.h"
#include "../xLights/utils/UtilFunctions.h"
#include "../xLights/ui/wxUtilities.h"
#include "md5.h"

#include <log.h>
#include <nlohmann/json.hpp>

#undef WXUSINGDLL


//#define DETAILED_LOGGING

bool __apiOnly = false;
std::string __password = "";
std::list<std::string> __Loggedin;
int __loginTimeout = 30;
std::string __validPass = "";
std::string __defaultPage = "index.html";

void WebServer::GeneratePass()
{
    wxString newPass = "";
    for (int i = 0; i < 16; ++i) {
        int r = rand01() * 36;
        if (r < 10) {
            newPass += char('0' + r);
        }
        else {
            newPass += char('A' + (r - 10));
        }
    }
    __validPass = newPass;
}

void RemoveFromValid(HttpConnection& connection)
{ 
    // remove any existing entry for this machine ... one logged in entry per machine
    for (auto it = __Loggedin.begin(); it != __Loggedin.end(); ++it) {
        wxArrayString li = wxSplit(*it, '|');
        if (li[0] == connection.Address().IPAddress()) {
            spdlog::debug("Security: Removing ip {}.", li[0].ToStdString());
            __Loggedin.remove(*it);
            break;
        }
    }
}

void UpdateValid(HttpConnection& connection)
{
    if (__password == "") return; // no password ... always logged in

    for (auto it = __Loggedin.begin(); it != __Loggedin.end(); ++it) {
        wxArrayString li = wxSplit(*it, '|');

        if (li[0] == connection.Address().IPAddress()) {
            li[1] = wxDateTime::Now().FormatISOCombined();
            *it = li[0] + "|" + li[1];
            return;
        }
    }
}

void AddToValid(HttpConnection& connection)
{
    // remove any existing entry for this machine ... one logged in entry per machine
    for (auto it = __Loggedin.begin(); it != __Loggedin.end(); ++it) {
        wxArrayString li = wxSplit(*it, '|');
        if (li[0] == connection.Address().IPAddress()) {
            spdlog::debug("Security: Removing ip {}.", li[0].ToStdString());
            __Loggedin.remove(*it);
            break;
        }
    }

    wxString security = connection.Address().IPAddress() + "|" + wxDateTime::Now().FormatISOCombined();

    spdlog::debug("Security: Adding record {}.", security.ToStdString());
    __Loggedin.push_back(security);
}

bool IsValidPass(const std::string& pass)
{
    return __validPass != "" && pass == __validPass;
}

bool CheckLoggedIn(HttpConnection& connection, const std::string& pass)
{
    if (__password == "") return true; // no password ... always logged in

    if (IsValidPass(pass)) return true; // pages which have a valid pass can call APIs

    // remove old logins
    std::list<wxString> toremove;
    for (const auto& it : __Loggedin) {
        wxArrayString li = wxSplit(it, '|');
        wxDateTime lastused;
        lastused.ParseISOCombined(li[1]);
        if (wxDateTime::Now() - lastused > __loginTimeout * 60000) {
            spdlog::debug("Security: Removing ip {} due to timout.", li[0].ToStdString());
            toremove.push_back(it);
        }
    }
    for (const auto& it : toremove) {
        __Loggedin.remove(it);
    }

    for (auto it = __Loggedin.begin(); it != __Loggedin.end(); ++it) {
        wxArrayString li = wxSplit(*it, '|');

        if (li[0] == connection.Address().IPAddress()) {
            li[1] = wxDateTime::Now().FormatISOCombined();
            *it = li[0] + "|" + li[1];
            return true;
        }
    }

    return false;
}

std::map<wxString, wxString> ParseURI(wxString uri)
{
    std::map<wxString, wxString> res;

    wxString s(uri);
    wxString q = s.AfterFirst('?');

    if (q != "") {
        wxArrayString p = wxSplit(q, '&');

        for (auto it = p.begin(); it != p.end(); ++it) {
            wxArrayString x = wxSplit(*it, '=');
            if (x.Count() >= 2) {
                wxString key = x[0];

                res[key] = "";
                for (auto it2 = x.begin(); it2 != x.end(); ++it2) {
                    if (it2 == x.begin()) {
                        // ignore the key
                    }
                    else {
                        if (res[key] != "") {
                            res[key] += "=";
                        }
                        res[key] += *it2;
                    }
                }
            }
        }
    }

    return res;
}

wxString ProcessCommand(HttpConnection& connection, const wxString& command, const wxString& parameters, const wxString& data, const wxString& reference, const std::string& pass)
{
    wxStopWatch sw;

    if (!CheckLoggedIn(connection, pass)) {
        return "{\"result\":\"not logged in\",\"command\":\"" +
            command + "\",\"reference\":\"" +
            reference + "\",\"ip\":\"" +
            connection.Address().IPAddress() + "\"}";
    }

#ifdef DETAILED_LOGGING
    spdlog::info("xScheduleCommand received command = '{}' parameters = '{}'", command.c_str(), parameters.c_str());
#endif

    wxString result;
    size_t rate = 0;
    wxString msg = "";
    if (xScheduleFrame::GetScheduleManager()->Action(command, parameters, data, nullptr, nullptr, nullptr, rate, msg)) {
        wxCommandEvent event(EVT_FRAMEMS);
        event.SetInt(rate);
        wxPostEvent(wxGetApp().GetTopWindow(), event);

        result = "{\"result\":\"ok\",\"reference\":\"" +
            reference + "\",\"command\":\"" +
            command + "\"}";

#ifdef DETAILED_LOGGING
        spdlog::info("    Time {}.", sw.Time());
#endif
    }
    else {
        result = "{\"result\":\"failed\",\"command\":\"" +
            command + "\",\"reference\":\"" +
            reference + "\",\"message\":\"" +
            msg + "\"}";
        spdlog::info("Command command={} parameters={} result='{}'. Time {}.", command.ToStdString(), parameters.ToStdString(), msg.ToStdString(), sw.Time());
    }

    return result;
}

wxString ProcessPluginRequest(HttpConnection& connection, const wxString& plugin, const wxString& command, const wxString& parameters, const wxString& data, const wxString& reference, const std::string& pass)
{
    wxStopWatch sw;

    if (!CheckLoggedIn(connection, pass)) {
        return "{\"result\":\"not logged in\",\"command\":\"" +
            command + "\",\"ip\":\"" +
            connection.Address().IPAddress() + "\"}";
    }

#ifdef DETAILED_LOGGING
    spdlog::info("xSchedule received plugin request command = '{}' parameters = '{}'", command.c_str(), parameters.c_str());
#endif

    return ((xScheduleFrame*)wxTheApp->GetTopWindow())->ProcessPluginRequest(plugin, command, parameters, data, reference);
}

wxString ProcessQuery(HttpConnection& connection, const wxString& query, const wxString& parameters, const wxString& reference, const std::string& pass)
{
    wxStopWatch sw;

    if (!CheckLoggedIn(connection, pass)) {
        return "{\"result\":\"not logged in\",\"query\":\"" +
            query + "\",\"reference\":\"" +
            reference + "\",\"ip\":\"" +
            connection.Address().IPAddress() + "\"}";
    }

    // log everything but playing status
#ifndef DETAILED_LOGGING
    if (query != "GetPlayingStatus")
#endif
        spdlog::info("xScheduleQuery received query = '{}' parameters = '{}'", query.ToStdString(), parameters.ToStdString());

    wxString result = "";
    wxString msg;
    if (xScheduleFrame::GetScheduleManager()->Query(query, parameters, result, msg, connection.Address().IPAddress(), reference)) {
#ifndef DETAILED_LOGGING
        if (query != "GetPlayingStatus")
#endif
            spdlog::info("    data = '{}'. Time = {}.", result.ToStdString(), sw.Time());
    }
    else {
        result = "{\"result\":\"failed\",\"query\":\"" +
            query + "\",\"reference\":\"" +
            reference + "\",\"message\":\"" +
            msg + "\"}";
        spdlog::info("    data = '' : '{}'. Time = {}.", result.ToStdString(), sw.Time());
    }

    return result;
}

wxString ProcessXyzzy(HttpConnection& connection, const wxString& command, const wxString& parameters, const wxString& reference, const std::string& pass)
{
    wxStopWatch sw;

    wxString result;
    wxString msg;
    if (xScheduleFrame::GetScheduleManager()->DoXyzzy(command, parameters, msg, reference)) {
        result = msg;
#ifdef DETAILED_LOGGING
        spdlog::info("xyzzy command={} parameters={} result='{}'. Time {}.", command.c_str(), parameters.c_str(), msg.c_str(), sw.Time());
#endif
    }
    else {
        result = "{\"result\":\"failed\",\"xyzzy\":\"" +
            command + "\",\"reference\":\"" +
            reference + "\",\"message\":\"" +
            msg + "\"}";
        spdlog::info("xyzzy command={} parameters={} result='{}'. Time {}.", command.ToStdString(), parameters.ToStdString(), msg.ToStdString(), sw.Time());
    }

    return result;
}

wxString ProcessLogin(HttpConnection& connection, const wxString& credential, const wxString& reference)
{
    wxStopWatch sw;
    wxString result;
    if (__password != "") {
        wxString cred = connection.Address().IPAddress() + __password;

        // calculate md5 hash
        wxString hash = md5(cred);

        if (hash == credential) {
            // this is a valid login
            AddToValid(connection);
            result = "{\"result\":\"ok\",\"reference\":\"" + reference + "\",\"command\":\"login\"}";

            // THIS SHOULD BE REMOVED
            //logger_base.debug("Security: Login %s success.", (const char *)credential.c_str());

            spdlog::debug("Security: Login success {}. Time {}.", connection.Address().IPAddress().ToStdString(), sw.Time());
        }
        else {
            // not a valid login
            // THIS SHOULD BE REMOVED
            //logger_base.debug("Security: Login failed - credential was %s for %s when it should have been %s.", (const char *)credential.c_str(), (const char *)cred.c_str(), (const char *)hash.c_str());

            RemoveFromValid(connection);

            result = "{\"result\":\"failed\",\"command\":\"login\",\"message\":\"Login failed.\",\"reference\":\"" + reference + "\",\"ip\":\"" + connection.Address().IPAddress() + "\"}";
            spdlog::debug("Security: Login failed. data = '{}'. Time = {}.", result.ToStdString(), sw.Time());
        }
    }
    else {
        result = "{\"result\":\"ok\",\"command\":\"login\",\"reference\":\"" + reference + "\"}";
    }

    return result;
}

wxString ProcessStash(HttpConnection& connection, const wxString& command, const wxString& key, wxString& data, const wxString& reference, const std::string& pass)
{
    wxStopWatch sw;

    if (!CheckLoggedIn(connection, pass)) {
        return "{\"result\":\"not logged in\",\"stash\":\"" +
            command + "\",\"reference\":\"" +
            reference + "\",\"ip\":\"" +
            connection.Address().IPAddress() + "\"}";
    }

    spdlog::info("xScheduleStash received command = '{}' key = '{}'", command.ToStdString(), key.ToStdString());

    wxString result;
    if (wxString(command).Lower() == "store") {
        spdlog::info("    data = '{}'", data.ToStdString());

        if (key == "GetModels") {
            result = "{\"result\":\"failed\",\"stash\":\"" +
                command + "\",\"reference\":\"" +
                reference + "\",\"message\":\"Unable to store under key 'GetModels'. This is a reserved key.\"}";
            spdlog::info("    data = '{}'. Time = {}.", data.ToStdString(), sw.Time());
        }
        else {
            // now store it in a file
            wxString msg = "";
            if (xScheduleFrame::GetScheduleManager()->StoreData(key, data, msg)) {
#ifdef DETAILED_LOGGING
                spdlog::info("    Time {}.", sw.Time());
#endif
                result = "{\"result\":\"ok\",\"reference\":\"" + reference + "\",\"stash\":\"" + command + "\"}";
            }
            else {
                result = "{\"result\":\"failed\",\"stash\":\"" +
                    command + "\",\"reference\":\"" +
                    reference + "\",\"message\":\"" +
                    msg + "\"}";
                spdlog::info("    data = '{}'. Time = {}.", data.ToStdString(), sw.Time());
            }
        }
    }
    else if (wxString(command).Lower() == "retrieve") {
        wxString msg = "";
        if (xScheduleFrame::GetScheduleManager()->RetrieveData(key, data, msg)) {
            spdlog::info("    data = '{}'. Time = {}.", data.ToStdString(), sw.Time());
            result = "";
        }
        else {
            result = "{\"result\":\"failed\",\"stash\":\"" +
                command + "\",\"reference\":\"" +
                reference + "\",\"message\":\"" +
                msg + "\"}";
            spdlog::info("    data = '' : '{}'. Time = {}.", data.ToStdString(), sw.Time());
        }
    }
    else if (wxString(command).Lower() == "retrievejson") {
        wxString msg = "";
        if (xScheduleFrame::GetScheduleManager()->RetrieveData(key, data, msg)) {
            spdlog::info("    data = '{}'. Time = {}.", data.ToStdString(), sw.Time());
            result = "{\"reference\":\"" + reference +
                "\",\"key\":\"" + key + "\",\"value\":[" + data + "]}";
        }
        else {
            result = "{\"result\":\"failed\",\"stash\":\"" +
                command + "\",\"reference\":\"" +
                reference + "\",\"message\":\"" +
                msg + "\"}";
            spdlog::info("    data = '' : '{}'. Time = {}.", data.ToStdString(), sw.Time());
        }
    }
    else {
        result = "{\"result\":\"failed\",\"stash\":\"" +
            command + "\",\"reference\":\"" +
            reference + "\",\"message\":\"Unknown stash command.\"}";
        spdlog::info("    '{}'. Time = {}.", data.ToStdString(), sw.Time());
    }
    return result;
}

wxString GetPluginRequest(const wxString& request)
{
    if (request == "") return "";

    auto plugin = request.AfterFirst('/').BeforeFirst('?');

    if (plugin == "") return "";

    return ((xScheduleFrame*)wxTheApp->GetTopWindow())->GetWebPluginRequest(plugin.ToStdString());
}

bool MyRequestHandler(HttpConnection& connection, HttpRequest& request)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written

    bool res = false;
    std::string plugin;

    spdlog::debug("Web request {}.", request.URI().ToStdString());

    xScheduleFrame::GetScheduleManager()->WebRequestReceived();

    wxString wwwroot = xScheduleFrame::GetScheduleManager()->GetOptions()->GetWWWRoot();
    if (request.URI().Lower().StartsWith("/xschedulecommand")) {
        wxURI url(request.URI());

        std::map<wxString, wxString> parms = ParseURI(url.BuildUnescapedURI());
        wxString command = parms["Command"];
        wxString parameters = parms["Parameters"];
        wxString reference = parms["Reference"];

        wxString result = ProcessCommand(connection, command, parameters, request.Data(), reference, "");

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText(result, "application/json");
        connection.SendResponse(response);

        res = true;
    }
    else if (request.URI().Lower().StartsWith("/xyzzy2")) {
        wxURI url(request.URI().Lower());
        std::map<wxString, wxString> parms = ParseURI(url.BuildUnescapedURI());
        wxString command = parms["c"];
        wxString parameters = parms["p"];
        wxString reference = parms["r"];

        wxString result = ProcessXyzzy(connection, command + "2", parameters, reference, "");

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText(result, "application/json");
        connection.SendResponse(response);

        res = true;
    }
    else if (request.URI().Lower().StartsWith("/xyzzy")) {
        wxURI url(request.URI().Lower());
        std::map<wxString, wxString> parms = ParseURI(url.BuildUnescapedURI());
        wxString command = parms["c"];
        wxString parameters = parms["p"];
        wxString reference = parms["r"];

        wxString result = ProcessXyzzy(connection, command, parameters, reference, "");

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText(result, "application/json");
        connection.SendResponse(response);

        res = true;
    }
    else if (request.URI().Lower().StartsWith("/xschedulelogin")) {
        wxURI url(request.URI());

        std::map<wxString, wxString> parms = ParseURI(url.BuildUnescapedURI());
        wxString credential = parms["Credential"];
        wxString reference = parms["Reference"];

        wxString result = ProcessLogin(connection, credential, reference);

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText(result, "application/json");
        connection.SendResponse(response);

        res = true;
    }
    else if (request.URI().Lower().StartsWith("/xschedulestash")) {
        wxURI url(request.URI());

        std::map<wxString, wxString> parms = ParseURI(url.BuildUnescapedURI());
        wxString command = parms["Command"];
        wxString key = parms["Key"];
        wxString reference = parms["Reference"];
        wxString data = request.Data();

        wxString result = ProcessStash(connection, command, key, data, reference, "");

        HttpResponse response(connection, request, HttpStatus::OK);
        if (result == "") {
            response.MakeFromText(data, "text/plain");
        }
        else {
            response.MakeFromText(result, "application/json");
        }
        connection.SendResponse(response);

        res = true;
    }
    else if (request.URI().Lower().StartsWith("/xschedulequery")) {
        wxURI url(request.URI());

        std::map<wxString, wxString> parms = ParseURI(url.BuildUnescapedURI());
        wxString query = parms["Query"];
        wxString parameters = parms["Parameters"];
        wxString reference = parms["Reference"];

        wxString result = ProcessQuery(connection, query, parameters, reference, "");

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText(result, "application/json");
        connection.SendResponse(response);

        res = true;
    }
    else {
        plugin = GetPluginRequest(request.URI().Lower());
        if (plugin != "") {
            wxURI url(request.URI());
            std::map<wxString, wxString> parms = ParseURI(url.BuildUnescapedURI());
            wxString command = parms["Command"];
            wxString parameters = parms["Parameters"];
            wxString reference = parms["Reference"];
            wxString data = request.Data();

            wxString result = ProcessPluginRequest(connection, plugin, command, parameters, data, reference, "");

            HttpResponse response(connection, request, HttpStatus::OK);
            response.MakeFromText(result, "application/json");
            connection.SendResponse(response);
        }
        else if (wwwroot != "" && !__apiOnly && (request.URI() == "" || request.URI() == "/" || request.URI() == "/" + wwwroot || request.URI() == "/" + wwwroot + "/")) {
            // Chris if you need this line to be this way on linux then use a #ifdef as the other works on windows
            //int port = connection.Server()->Context().Port;
            //wxString url = "http://" + request.Host() + ":" + wxString::Format(wxT("%i"), port) + "/" + wwwroot + "/index.html";
            wxString url = "http://" + request.Host() + "/" + wwwroot + "/" + __defaultPage;

            spdlog::info("Redirecting to '{}'", url.ToStdString());

            HttpResponse response(connection, request, HttpStatus::PermanentRedirect);
            response.AddHeader("Location", url);
            //response.AddHeader("Connection", "Close");
            response.MakeFromText("Redirected to " + url + "\n", "text/plain");

            connection.SendResponse(response);

            res = true; // disable default processing
        }
        else if (wwwroot != "") {
            wxString uri = wxURI(request.URI()).BuildUnescapedURI();

            if (!__apiOnly && request.URI().StartsWith("/" + wwwroot)) {
#ifdef __WXMSW__
                wxString d = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
#elif __LINUX__
                wxString d = wxStandardPaths::Get().GetDataDir();
                if (!wxDir::Exists(d)) {
                    d = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
                }
#else
                wxString d = wxStandardPaths::Get().GetResourcesDir();
#endif

                wxString file = d;
                if (uri.Contains("?")) {
                    file += uri.BeforeFirst('?');
                }
                else {
                    file += uri;
                }

                spdlog::info("File request received = '{}' : '{}'", file.ToStdString(), uri.ToStdString());

                if (!wxFile::Exists(file)) {
                    spdlog::error("    404: file not found.");
                }

                HttpResponse response(connection, request, HttpStatus::OK);

                //response.AddHeader("Cache-Control", "max-age=14400");

                wxFileName fn(file);
                if (__validPass != "" && (fn.GetExt().Lower().StartsWith("htm") || fn.GetExt().Lower() == "js")) {
                    // we should replace any tokens with our token values
                    wxFile f;
                    if (f.Open(file, wxFile::OpenMode::read)) {
                        wxString data;
                        if (f.ReadAll(&data)) {
                            data.Replace("!PASS!", __validPass);
                            wxString mime = "text/html";
                            if (fn.GetExt().Lower() == "js") {
                                mime = "text/javascript";
                            }
                            response.MakeFromText(data, mime);
                        }
                        else {
                            response.MakeFromFile(file);
                        }
                    }
                    else {
                        response.MakeFromFile(file);
                    }
                }
                else {
                    response.MakeFromFile(file);
                }

                connection.SendResponse(response);

                res = true; // disable default processing
            }
        }
    }

    if (res) {
        spdlog::debug("Web request handled");
    }
    else {
        spdlog::debug("Web request NOT handled");
    }

    return res; // lets the library's default processing
}

void MyMessageHandler(HttpConnection& connection, WebSocketMessage& message)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written

    xScheduleFrame::GetScheduleManager()->WebRequestReceived();

    if (message.Type() == WebSocketMessage::Text) {
        wxString result;
        wxString text((char*)message.Content().GetData(), message.Content().GetDataLen());

        if (!text.empty()) {
            spdlog::info("Received {}", text.ToStdString());

            try {
                nlohmann::json root = nlohmann::json::parse(text.ToStdString());

                // extract the type of request
                wxString const type = wxString::FromUTF8(root.value("Type", "").c_str()).Lower();

                if (type == "command") {
                    wxString const c = wxString::FromUTF8(root.value("Command", "").c_str());
                    wxString const p = wxString::FromUTF8(root.value("Parameters", "").c_str());
                    wxString const d = wxString::FromUTF8(root.value("Data", "").c_str());
                    wxString const r = wxString::FromUTF8(root.value("Reference", "").c_str());
                    wxString const pass = wxString::FromUTF8(root.value("Pass", "").c_str());
                    result = ProcessCommand(connection, c, p, d, r, pass.ToStdString());
                } else if (type == "query") {
                    wxString const q = wxString::FromUTF8(root.value("Query", "").c_str());
                    wxString const p = wxString::FromUTF8(root.value("Parameters", "").c_str());
                    wxString const r = wxString::FromUTF8(root.value("Reference", "").c_str());
                    wxString const pass = wxString::FromUTF8(root.value("Pass", "").c_str());
                    result = ProcessQuery(connection, q, p, r, pass.ToStdString());
                } else if (type == "xyzzy2") {
                    wxString const c = wxString::FromUTF8(root.value("c", "").c_str());
                    wxString const p = wxString::FromUTF8(root.value("p", "").c_str());
                    wxString const r = wxString::FromUTF8(root.value("r", "").c_str());
                    result = ProcessXyzzy(connection, c + "2", p, r, "");
                } else if (type == "xyzzy") {
                    wxString const c = wxString::FromUTF8(root.value("c", "").c_str());
                    wxString const p = wxString::FromUTF8(root.value("p", "").c_str());
                    wxString const r = wxString::FromUTF8(root.value("r", "").c_str());
                    result = ProcessXyzzy(connection, c, p, r, "");
                } else if (type == "login") {
                    wxString const c = wxString::FromUTF8(root.value("Credential", "").c_str());
                    wxString const r = wxString::FromUTF8(root.value("Reference", "").c_str());
                    result = ProcessLogin(connection, c, r);
                } else if (type == "stash") {
                    wxString const c = wxString::FromUTF8(root.value("Command", "").c_str());
                    wxString const k = wxString::FromUTF8(root.value("Key", "").c_str());
                    wxString d = wxString::FromUTF8(root.value("Data", "").c_str());
                    wxString const r = wxString::FromUTF8(root.value("Reference", "").c_str());
                    wxString const pass = wxString::FromUTF8(root.value("Pass", "").c_str());
                    result = ProcessStash(connection, c, k, d, r, pass.ToStdString());
                    if (result.IsEmpty()) {
                        result = d;
                    }
                } else {
                    std::string const plugin = ((xScheduleFrame*)wxTheApp->GetTopWindow())->GetWebPluginRequest(type.ToStdString());
                    if (!plugin.empty()) {
                        wxString const c = wxString::FromUTF8(root.value("Command", "").c_str());
                        wxString const p = wxString::FromUTF8(root.value("Parameters", "").c_str());
                        wxString const d = wxString::FromUTF8(root.value("Data", "").c_str());
                        wxString const r = wxString::FromUTF8(root.value("Reference", "").c_str());
                        wxString const pass = wxString::FromUTF8(root.value("Pass", "").c_str());
                        result = ProcessPluginRequest(connection, plugin, c, p, d, r, pass.ToStdString());
                    } else {
                        wxString r = wxString::FromUTF8(root.value("Reference", "").c_str());
                        WebSocketMessage wsm("{\"result\":\"failed\",\"reference\":\"" + r + "\",\"message\":\"Unknown request type or plugin not running.\"}");
                        connection.SendMessage(wsm);
                        return;
                    }
                }
            } catch (const nlohmann::json::parse_error& e) {
                spdlog::error("The JSON document is not well-formed: {}", text.ToStdString());
                spdlog::error("message: {}", e.what());
                spdlog::error("exception id: {}", e.id);
                spdlog::error("byte position of error: {}", e.byte);

                WebSocketMessage wsm("{\"result\":\"failed\",\"message\":\"JSON message not well formed.\"}");
                connection.SendMessage(wsm);
                return;

            } catch (std::exception& ex) {
                spdlog::error("The JSON document is not well-formed: {}", text.ToStdString());
                spdlog::error("    {}", ex.what());
                WebSocketMessage wsm("{\"result\":\"failed\",\"message\":\"JSON message not well formed.\"}");
                connection.SendMessage(wsm);
                return;
            }
        } else {
            result = "{\"result\":\"failed\",\"reference\":\"\",\"message\":\"Empty request.\"}";
        }

        WebSocketMessage wsm(result);
        connection.SendMessage(wsm);
    }
    else if (message.Type() == WebSocketMessage::Binary) {
        spdlog::info("Received <binary>");
        WebSocketMessage wsm("{\"result\":\"failed\",\"message\":\"Binary web sockets not supported.\"}");
        connection.SendMessage(wsm);
    }
}

void WebServer::SendMessageToAllWebSockets(const wxString& message)
{
    static bool reentry = false;
    if (reentry) {
        return;
    }
    reentry = true;

    for (const auto& it : _connections) {
        if (it.second->IsWebSocket()) {
            WebSocketMessage wsm(message);
            if (it.second->SendMessage(wsm)) {
                UpdateValid(*it.second);
            }
            else {
                RemoveFromValid(*it.second);
            }
        }
    }

    reentry = false;
}

bool WebServer::IsSomeoneListening() const
{
    for (auto it : _connections) {
        if (it.second->IsWebSocket()) {
            return true;
        }
    }
    return false;
}

void WebServer::SetAllowUnauthenticatedPagesToBypassLogin(bool allowUnauthPages)
{
    if (!allowUnauthPages) {
        __validPass = "";
    }
    else if (__validPass == "") {
        GeneratePass();
    }
}

WebServer::WebServer(int port, bool apionly, const wxString& password, int mins, bool allowUnauthPages, const std::string& defaultPage)
{
    __apiOnly = apionly; // put this in a global.
    __password = password;
    __loginTimeout = mins;
    if (defaultPage != "") __defaultPage = defaultPage;
    SetAllowUnauthenticatedPagesToBypassLogin(allowUnauthPages);

    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written

    HttpContext context;
    context.Port = port;
    context.RequestHandler = MyRequestHandler;
    context.MessageHandler = MyMessageHandler;

    if (!Start(context)) {
        spdlog::error("Error starting web server.");
        wxMessageBox("Error starting web server. You may already have a program listening on port " + wxString::Format(wxT("%i"), port));
    }
}

WebServer::~WebServer()
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    Stop();
}

void WebServer::SetAPIOnly(bool apiOnly)
{
    __apiOnly = apiOnly;
}

void WebServer::SetPasswordTimeout(int mins)
{
    __loginTimeout = mins;
}

void WebServer::SetPassword(const wxString& password)
{
    __password = password;
}

void WebServer::SetDefaultPage(const std::string& defaultPage)
{
    if (defaultPage != "") __defaultPage = defaultPage;
}

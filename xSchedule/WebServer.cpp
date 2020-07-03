/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
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
#include "md5.h"

#include <log4cpp/Category.hh>

#undef WXUSINGDLL
#include "wxJSON/jsonreader.h"

//#define DETAILED_LOGGING

bool __apiOnly = false;
std::string __password = "";
std::list<std::string> __Loggedin;
int __loginTimeout = 30;

void RemoveFromValid(HttpConnection& connection)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // remove any existing entry for this machine ... one logged in entry per machine
    for (auto it = __Loggedin.begin(); it != __Loggedin.end(); ++it)
    {
        wxArrayString li = wxSplit(*it, '|');
        if (li[0] == connection.Address().IPAddress())
        {
            logger_base.debug("Security: Removing ip %s.", (const char *)li[0].c_str());
            __Loggedin.remove(*it);
            break;
        }
    }
}

void UpdateValid(HttpConnection& connection)
{
    if (__password == "") return; // no password ... always logged in

    for (auto it = __Loggedin.begin(); it != __Loggedin.end(); ++it)
    {
        wxArrayString li = wxSplit(*it, '|');

        if (li[0] == connection.Address().IPAddress())
        {
            li[1] = wxDateTime::Now().FormatISOCombined();
            *it = li[0] + "|" + li[1];
            return;
        }
    }
}

void AddToValid(HttpConnection& connection)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // remove any existing entry for this machine ... one logged in entry per machine
    for (auto it = __Loggedin.begin(); it != __Loggedin.end(); ++it)
    {
        wxArrayString li = wxSplit(*it, '|');
        if (li[0] == connection.Address().IPAddress())
        {
            logger_base.debug("Security: Removing ip %s.", (const char *)li[0].c_str());
            __Loggedin.remove(*it);
            break;
        }
    }

    wxString security = connection.Address().IPAddress() + "|" + wxDateTime::Now().FormatISOCombined();

    logger_base.debug("Security: Adding record %s.", (const char *)security.c_str());
    __Loggedin.push_back(security);
}

bool CheckLoggedIn(HttpConnection& connection)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (__password == "") return true; // no password ... always logged in

    // remove old logins
    std::list<wxString> toremove;
    for (auto it = __Loggedin.begin(); it != __Loggedin.end(); ++it)
    {
        wxArrayString li = wxSplit(*it, '|');
        wxDateTime lastused;
        lastused.ParseISOCombined(li[1]);
        if (wxDateTime::Now() - lastused > __loginTimeout * 60000)
        {
            logger_base.debug("Security: Removing ip %s due to timout.", (const char *)li[0].c_str());
            toremove.push_back(*it);
        }
    }
    for (auto it = toremove.begin(); it != toremove.end(); ++it)
    {
        __Loggedin.remove(*it);
    }

    for (auto it = __Loggedin.begin(); it != __Loggedin.end(); ++it)
    {
        wxArrayString li = wxSplit(*it, '|');

        if (li[0] == connection.Address().IPAddress())
        {
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

    if (q != "")
    {
        wxArrayString p = wxSplit(q, '&');

        for (auto it = p.begin(); it != p.end(); ++it)
        {
            wxArrayString x = wxSplit(*it, '=');
            if (x.Count() >= 2)
            {
                wxString key = x[0];

                res[key] = "";
                for (auto it2 = x.begin(); it2 != x.end(); ++it2)
                {
                    if (it2 == x.begin())
                    {
                        // ignore the key
                    }
                    else
                    {
                        if (res[key] != "")
                        {
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

wxString ProcessCommand(HttpConnection &connection, const wxString& command, const wxString& parameters, const wxString& data, const wxString& reference)
{
    wxStopWatch sw;
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!CheckLoggedIn(connection))
    {
        return "{\"result\":\"not logged in\",\"command\":\""+
         command + "\",\"reference\":\"" +
            reference + "\",\"ip\":\"" +
                connection.Address().IPAddress() + "\"}";
    }

#ifdef DETAILED_LOGGING
    logger_base.info("xScheduleCommand received command = '%s' parameters = '%s'.", (const char *)command.c_str(), (const char *)parameters.c_str());
#endif

    wxString result;
    size_t rate = 0;
    wxString msg = "";
    if (xScheduleFrame::GetScheduleManager()->Action(command, parameters, data, nullptr, nullptr, nullptr, rate, msg))
    {
        wxCommandEvent event(EVT_FRAMEMS);
        event.SetInt(rate);
        wxPostEvent(wxGetApp().GetTopWindow(), event);

        result = "{\"result\":\"ok\",\"reference\":\""+
            reference+"\",\"command\":\""+
            command+"\"}";

#ifdef DETAILED_LOGGING
        logger_base.info("    Time %ld.", sw.Time());
#endif
    }
    else
    {
        result = "{\"result\":\"failed\",\"command\":\""+
            command + "\",\"reference\":\"" +
            reference + "\",\"message\":\"" +
                   msg + "\"}";
        logger_base.info("Command command=%s parameters=%s result='%s'. Time %ld.", (const char *)command.c_str(), (const char*)parameters.c_str(), (const char *)msg.c_str(), sw.Time());
    }

    return result;
}

wxString ProcessPluginRequest(HttpConnection& connection, const wxString& plugin, const wxString& command, const wxString& parameters, const wxString& data, const wxString& reference)
{
    wxStopWatch sw;
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!CheckLoggedIn(connection))
    {
        return "{\"result\":\"not logged in\",\"command\":\"" +
            command + "\",\"ip\":\"" +
            connection.Address().IPAddress() + "\"}";
    }

#ifdef DETAILED_LOGGING
    logger_base.info("xSchedule received plugin request command = '%s' parameters = '%s'.", (const char*)command.c_str(), (const char*)parameters.c_str());
#endif

    return ((xScheduleFrame*)wxTheApp->GetTopWindow())->ProcessPluginRequest(plugin, command, parameters, data, reference);
}

wxString ProcessQuery(HttpConnection &connection, const wxString& query, const wxString& parameters, const wxString& reference)
{
    wxStopWatch sw;
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!CheckLoggedIn(connection))
    {
        return "{\"result\":\"not logged in\",\"query\":\""+
            query + "\",\"reference\":\"" +
            reference + "\",\"ip\":\"" +
            connection.Address().IPAddress() + "\"}";
    }

    // log everything but playing status
#ifndef DETAILED_LOGGING
    if (query != "GetPlayingStatus")
#endif
        logger_base.info("xScheduleQuery received query = '%s' parameters = '%s'.", (const char *)query.c_str(), (const char *)parameters.c_str());

    wxString result = "";
    wxString msg;
    if (xScheduleFrame::GetScheduleManager()->Query(query, parameters, result, msg, connection.Address().IPAddress(), reference))
    {
#ifndef DETAILED_LOGGING
        if (query != "GetPlayingStatus")
#endif
            logger_base.info("    data = '%s'. Time = %ld.", (const char *)result.c_str(), sw.Time());
    }
    else
    {
        result = "{\"result\":\"failed\",\"query\":\""+
            query + "\",\"reference\":\"" +
            reference + "\",\"message\":\"" +
            msg + "\"}";
        logger_base.info("    data = '' : '%s'. Time = %ld.", (const char *)result.c_str(), sw.Time());
    }

    return result;
}

wxString ProcessXyzzy(HttpConnection &connection, const wxString& command, const wxString& parameters, const wxString& reference)
{
    wxStopWatch sw;
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxString result;
    wxString msg;
    if (xScheduleFrame::GetScheduleManager()->DoXyzzy(command, parameters, msg, reference))
    {
        result = msg;
#ifdef DETAILED_LOGGING
        logger_base.info("xyzzy command=%s parameters=%s result='%s'. Time %ld.", (const char *)command.c_str(), (const char*)parameters.c_str(), (const char *)msg.c_str(), sw.Time());
#endif
    }
    else
    {
        result = "{\"result\":\"failed\",\"xyzzy\":\""+
            command + "\",\"reference\":\"" +
            reference + "\",\"message\":\"" +
            msg + "\"}";
        logger_base.info("xyzzy command=%s parameters=%s result='%s'. Time %ld.", (const char *)command.c_str(), (const char*)parameters.c_str(), (const char *)msg.c_str(), sw.Time());
    }

    return result;
}

wxString ProcessLogin(HttpConnection &connection, const wxString& credential, const wxString& reference)
{
    wxStopWatch sw;
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    
    wxString result;
    if (__password != "")
    {
        wxString cred = connection.Address().IPAddress() + __password;

        // calculate md5 hash
        wxString hash = md5(cred);

        if (hash == credential)
        {
            // this is a valid login
            AddToValid(connection);
            result = "{\"result\":\"ok\",\"reference\":\""+reference+"\",\"command\":\"login\"}";

            // THIS SHOULD BE REMOVED
            //logger_base.debug("Security: Login %s success.", (const char *)credential.c_str());

            logger_base.debug("Security: Login success %s. Time %ld.", (const char *)connection.Address().IPAddress().c_str(), sw.Time());
        }
        else
        {
            // not a valid login
            // THIS SHOULD BE REMOVED
            //logger_base.debug("Security: Login failed - credential was %s for %s when it should have been %s.", (const char *)credential.c_str(), (const char *)cred.c_str(), (const char *)hash.c_str());

            RemoveFromValid(connection);

            result = "{\"result\":\"failed\",\"command\":\"login\",\"message\":\"Login failed.\",\"reference\":\""+reference+"\",\"ip\":\"" + connection.Address().IPAddress() + "\"}";
            logger_base.debug("Security: Login failed. data = '%s'. Time = %ld.", (const char *)result.c_str(), sw.Time());
        }
    }
    else
    {
        result = "{\"result\":\"ok\",\"command\":\"login\",\"reference\":\""+reference+"\"}";
    }

    return result;
}

wxString ProcessStash(HttpConnection &connection, const wxString& command, const wxString& key, wxString& data, const wxString& reference)
{
    wxStopWatch sw;
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!CheckLoggedIn(connection))
    {
        return "{\"result\":\"not logged in\",\"stash\":\""+
            command + "\",\"reference\":\"" +
            reference + "\",\"ip\":\"" +
            connection.Address().IPAddress() + "\"}";
    }

    logger_base.info("xScheduleStash received command = '%s' key = '%s'.", (const char *)command.c_str(), (const char *)key.c_str());

    wxString result;
    if (wxString(command).Lower() == "store")
    {
        logger_base.info("    data = '%s'.", (const char *)data.c_str());

        if (key == "GetModels")
        {
            result = "{\"result\":\"failed\",\"stash\":\"" +
                command + "\",\"reference\":\"" +
                reference + "\",\"message\":\"Unable to store under key 'GetModels'. This is a reserved key.\"}";
            logger_base.info("    data = '%s'. Time = %ld.", (const char *)data.c_str(), sw.Time());
        }
        else
        {
            // now store it in a file
            wxString msg = "";
            if (xScheduleFrame::GetScheduleManager()->StoreData(key, data, msg))
            {
#ifdef DETAILED_LOGGING
                logger_base.info("    Time %ld.", sw.Time());
#endif
                result = "{\"result\":\"ok\",\"reference\":\"" + reference + "\",\"stash\":\"" + command + "\"}";
            }
            else
            {
                result = "{\"result\":\"failed\",\"stash\":\"" +
                    command + "\",\"reference\":\"" +
                    reference + "\",\"message\":\"" +
                    msg + "\"}";
                logger_base.info("    data = '%s'. Time = %ld.", (const char *)data.c_str(), sw.Time());
            }
        }
    }
    else if (wxString(command).Lower() == "retrieve")
    {
        wxString msg = "";
        if (xScheduleFrame::GetScheduleManager()->RetrieveData(key, data, msg))
        {
            logger_base.info("    data = '%s'. Time = %ld.", (const char *)data.c_str(), sw.Time());
            result = "";
        }
        else
        {
            result = "{\"result\":\"failed\",\"stash\":\""+
                command + "\",\"reference\":\"" +
                reference+"\",\"message\":\"" +
                msg + "\"}";
            logger_base.info("    data = '' : '%s'. Time = %ld.", (const char *)data.c_str(), sw.Time());
        }
    }
    else if (wxString(command).Lower() == "retrievejson")
    {
        wxString msg = "";
        if (xScheduleFrame::GetScheduleManager()->RetrieveData(key, data, msg))
        {
            logger_base.info("    data = '%s'. Time = %ld.", (const char *)data.c_str(), sw.Time());
            result = "{\"reference\":\"" + reference +
                     "\",\"key\":\"" + key + "\",\"value\":[" + data + "]}";
        }
        else
        {
            result = "{\"result\":\"failed\",\"stash\":\"" +
                command + "\",\"reference\":\"" +
                reference + "\",\"message\":\"" +
                msg + "\"}";
            logger_base.info("    data = '' : '%s'. Time = %ld.", (const char *)data.c_str(), sw.Time());
        }
    }
    else
    {
        result = "{\"result\":\"failed\",\"stash\":\""+
            command+"\",\"reference\":\""+
            reference+"\",\"message\":\"Unknown stash command.\"}";
        logger_base.info("    '%s'. Time = %ld.", (const char *)data.c_str(), sw.Time());
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

bool MyRequestHandler(HttpConnection &connection, HttpRequest &request)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool res = false;
    std::string plugin;

    logger_base.debug("Web request %s.", (const char *)request.URI().c_str());

    xScheduleFrame::GetScheduleManager()->WebRequestReceived();

    wxString wwwroot = xScheduleFrame::GetScheduleManager()->GetOptions()->GetWWWRoot();
    if (request.URI().Lower().StartsWith("/xschedulecommand"))
    {
        wxURI url(request.URI());

        std::map<wxString, wxString> parms = ParseURI(url.BuildUnescapedURI());
        wxString command = parms["Command"];
        wxString parameters = parms["Parameters"];
        wxString reference = parms["Reference"];

        wxString result = ProcessCommand(connection, command, parameters, request.Data(), reference);

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText(result, "application/json");
        connection.SendResponse(response);

        res = true;
    }
    else if (request.URI().Lower().StartsWith("/xyzzy2"))
    {
        wxURI url(request.URI().Lower());
        std::map<wxString, wxString> parms = ParseURI(url.BuildUnescapedURI());
        wxString command = parms["c"];
        wxString parameters = parms["p"];
        wxString reference = parms["r"];

        wxString result = ProcessXyzzy(connection, command + "2", parameters, reference);

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText(result, "application/json");
        connection.SendResponse(response);

        res = true;
    }
    else if (request.URI().Lower().StartsWith("/xyzzy"))
    {
        wxURI url(request.URI().Lower());
        std::map<wxString, wxString> parms = ParseURI(url.BuildUnescapedURI());
        wxString command = parms["c"];
        wxString parameters = parms["p"];
        wxString reference = parms["r"];

        wxString result = ProcessXyzzy(connection, command, parameters, reference);

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText(result, "application/json");
        connection.SendResponse(response);

        res = true;
    }
    else if (request.URI().Lower().StartsWith("/xschedulelogin"))
    {
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
    else if (request.URI().Lower().StartsWith("/xschedulestash"))
    {
        wxURI url(request.URI());

        std::map<wxString, wxString> parms = ParseURI(url.BuildUnescapedURI());
        wxString command = parms["Command"];
        wxString key = parms["Key"];
        wxString reference = parms["Reference"];
        wxString data = request.Data();

        wxString result = ProcessStash(connection, command, key, data, reference);

        HttpResponse response(connection, request, HttpStatus::OK);
        if (result == "")
        {
            response.MakeFromText(data, "text/plain");
        }
        else
        {
            response.MakeFromText(result, "application/json");
        }
        connection.SendResponse(response);

        res = true;
    }
    else if (request.URI().Lower().StartsWith("/xschedulequery"))
    {
        wxURI url(request.URI());

        std::map<wxString, wxString> parms = ParseURI(url.BuildUnescapedURI());
        wxString query = parms["Query"];
        wxString parameters = parms["Parameters"];
        wxString reference = parms["Reference"];

        wxString result = ProcessQuery(connection, query, parameters, reference);

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText(result, "application/json");
        connection.SendResponse(response);

        res = true;
    }
    else
    {
        plugin = GetPluginRequest(request.URI().Lower());
        if (plugin != "")
        {
            wxURI url(request.URI());
            std::map<wxString, wxString> parms = ParseURI(url.BuildUnescapedURI());
            wxString command = parms["Command"];
            wxString parameters = parms["Parameters"];
            wxString reference = parms["Reference"];
            wxString data = request.Data();

            wxString result = ProcessPluginRequest(connection, plugin, command, parameters, data, reference);

            HttpResponse response(connection, request, HttpStatus::OK);
            response.MakeFromText(result, "application/json");
            connection.SendResponse(response);
        }
        else if (wwwroot != "" && !__apiOnly && (request.URI() == "" || request.URI() == "/" || request.URI() == "/" + wwwroot || request.URI() == "/" + wwwroot + "/"))
        {
            // Chris if you need this line to be this way on linux then use a #ifdef as the other works on windows
            //int port = connection.Server()->Context().Port;
            //wxString url = "http://" + request.Host() + ":" + wxString::Format(wxT("%i"), port) + "/" + wwwroot + "/index.html";
            wxString url = "http://" + request.Host() + "/" + wwwroot + "/index.html";

            logger_base.info("Redirecting to '%s'.", (const char*)url.c_str());

            HttpResponse response(connection, request, HttpStatus::PermanentRedirect);
            response.AddHeader("Location", url);
            //response.AddHeader("Connection", "Close");
            response.MakeFromText("Redirected to " + url + "\n", "text/plain");

            connection.SendResponse(response);

            res = true; // disable default processing
        }
        else if (wwwroot != "")
        {
            wxString uri = wxURI(request.URI()).BuildUnescapedURI();

            if (!__apiOnly && request.URI().StartsWith("/" + wwwroot))
            {
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
                if (uri.Contains("?"))
                {
                    file += uri.BeforeFirst('?');
                }
                else
                {
                    file += uri;
                }

                logger_base.info("File request received = '%s' : '%s'.", (const char*)file.c_str(), (const char*)uri.c_str());

                if (!wxFile::Exists(file))
                {
                    logger_base.error("    404: file not found.");
                }

                HttpResponse response(connection, request, HttpStatus::OK);

                //response.AddHeader("Cache-Control", "max-age=14400");

                response.MakeFromFile(file);

                connection.SendResponse(response);

                res = true; // disable default processing
            }
        }
    }

    if (res)
    {
        logger_base.debug("Web request handled");
    }
    else
    {
        logger_base.debug("Web request NOT handled");
    }

    return res; // lets the library's default processing
}

void MyMessageHandler(HttpConnection& connection, WebSocketMessage& message)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    xScheduleFrame::GetScheduleManager()->WebRequestReceived();

    if (message.Type() == WebSocketMessage::Text)
    {
        wxString result;
        wxString text((char*)message.Content().GetData(), message.Content().GetDataLen());

        if (text != "")
        {
            logger_base.info("Received " + text);

            // construct the JSON root object
            wxJSONValue  root;

            // construct a JSON parser
            wxJSONReader reader;

            // now read the JSON text and store it in the 'root' structure
            // check for errors before retreiving values...
            int numErrors = reader.Parse(text, &root);
            if (numErrors > 0) {
                logger_base.error("The JSON document is not well-formed: " + text);
                const wxArrayString& errors = reader.GetErrors();
                for (auto it = errors.begin(); it != errors.end(); ++it)
                {
                    logger_base.error("    " + std::string(it->c_str()));
                }
                WebSocketMessage wsm("{\"result\":\"failed\",\"message\":\"JSON message not well formed.\"}");
                connection.SendMessage(wsm);
                return;
            }

            wxJSONValue defaultValue = wxString("");
            std::string plugin;

            // extract the type of request
            wxString type = root.Get("Type", defaultValue).AsString().Lower();

            if (type == "command")
            {
                wxString c = root.Get("Command", defaultValue).AsString();
                wxString p = root.Get("Parameters", defaultValue).AsString();
                wxString d = root.Get("Data", defaultValue).AsString();
                wxString r = root.Get("Reference", defaultValue).AsString();
                result = ProcessCommand(connection, c, p, d, r);
            }
            else if (type == "query")
            {
                wxString q = root.Get("Query", defaultValue).AsString();
                wxString p = root.Get("Parameters", defaultValue).AsString();
                wxString r = root.Get("Reference", defaultValue).AsString();
                result = ProcessQuery(connection, q, p, r);
            }
            else if (type == "xyzzy2")
            {
                wxString c = root.Get("c", defaultValue).AsString();
                wxString p = root.Get("p", defaultValue).AsString();
                wxString r = root.Get("r", defaultValue).AsString();
                result = ProcessXyzzy(connection, c + "2", p, r);
            }
            else if (type == "xyzzy")
            {
                wxString c = root.Get("c", defaultValue).AsString();
                wxString p = root.Get("p", defaultValue).AsString();
                wxString r = root.Get("r", defaultValue).AsString();
                result = ProcessXyzzy(connection, c, p, r);
            }
            else if (type == "login")
            {
                wxString c = root.Get("Credential", defaultValue).AsString();
                wxString r = root.Get("Reference", defaultValue).AsString();
                result = ProcessLogin(connection, c, r);
            }
            else if (type == "stash")
            {
                wxString c = root.Get("Command", defaultValue).AsString();
                wxString k = root.Get("Key", defaultValue).AsString();
                wxString d = root.Get("Data", defaultValue).AsString();
                wxString r = root.Get("Reference", defaultValue).AsString();
                result = ProcessStash(connection, c, k, d, r);
                if (result == "")
                {
                    result = d;
                }
            }
            else
            {
                plugin = ((xScheduleFrame*)wxTheApp->GetTopWindow())->GetWebPluginRequest(type);
                if (plugin != "")
                {
                    wxString c = root.Get("Command", defaultValue).AsString();
                    wxString p = root.Get("Parameters", defaultValue).AsString();
                    wxString d = root.Get("Data", defaultValue).AsString();
                    wxString r = root.Get("Reference", defaultValue).AsString();
                    result = ProcessPluginRequest(connection, plugin, c, p, d, r);
                }
                else
                {
                    wxString r = root.Get("Reference", defaultValue).AsString();
                    WebSocketMessage wsm("{\"result\":\"failed\",\"reference\":\"" + r + "\",\"message\":\"Unknown request type or plugin not running.\"}");
                    connection.SendMessage(wsm);
                    return;
                }
            }
        }
        else
        {
            result = "{\"result\":\"failed\",\"reference\":\"\",\"message\":\"Empty request.\"}";
        }

        WebSocketMessage wsm(result);
        connection.SendMessage(wsm);
    }
    else if (message.Type() == WebSocketMessage::Binary)
    {
        logger_base.info("Received <binary>");
        WebSocketMessage wsm("{\"result\":\"failed\",\"message\":\"Binary web sockets not supported.\"}");
        connection.SendMessage(wsm);
    }
}

void WebServer::SendMessageToAllWebSockets(const wxString& message)
{
    static bool reentry = false;
    if (reentry)
    {
        return;
    }
    reentry = true;

    for (auto it = _connections.begin(); it != _connections.end(); ++it)
    {
        if ((*it).second->IsWebSocket())
        {
            WebSocketMessage wsm(message);
            if (it->second->SendMessage(wsm))
            {
                UpdateValid(*it->second);
            }
            else
            {
                RemoveFromValid(*it->second);
            }
        }
    }

    reentry = false;
}

bool WebServer::IsSomeoneListening() const
{
    for (auto it : _connections)
    {
        if (it.second->IsWebSocket())
        {
            return true;
        }
    }
    return false;
}

WebServer::WebServer(int port, bool apionly, const wxString& password, int mins)
{
    __apiOnly = apionly; // put this in a global.
    __password = password;
    __loginTimeout = mins;

    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    HttpContext context;
    context.Port = port;
    context.RequestHandler = MyRequestHandler;
    context.MessageHandler = MyMessageHandler;

    if (!Start(context))
    {
        logger_base.error("Error starting web server.");
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

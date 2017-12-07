#include "WebServer.h"

#include <wx/wx.h>
#include <log4cpp/Category.hh>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include "xScheduleMain.h"
#include "ScheduleManager.h"
#include <wx/uri.h>
#include "xScheduleApp.h"
#include "ScheduleOptions.h"
#include "md5.h"

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

    std::string security = connection.Address().IPAddress().ToStdString() + "|" + wxDateTime::Now().FormatISOCombined().ToStdString();

    logger_base.debug("Security: Adding record %s.", (const char *)security.c_str());
    __Loggedin.push_back(security);
}

bool CheckLoggedIn(HttpConnection& connection)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (__password == "") return true; // no password ... always logged in

    // remove old logins
    std::list<std::string> toremove;
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

std::map<std::string, std::string> ParseURI(std::string uri)
{
    std::map<std::string, std::string> res;

    wxString s(uri);
    wxArrayString q = wxSplit(s, '?');

    if (q.Count() > 1)
    {
        wxArrayString p = wxSplit(q[1], '&');

        for (auto it = p.begin(); it != p.end(); ++it)
        {
            wxArrayString x = wxSplit(*it, '=');
            if (x.Count() >= 2)
            {
                std::string key = x[0].ToStdString();

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
                        res[key] += it2->ToStdString();
                    }
                }
            }
        }
    }

    return res;
}

std::string ProcessCommand(HttpConnection &connection, const std::string& command, const std::string& parameters, const std::string& data, const std::string& reference)
{
    wxStopWatch sw;
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!CheckLoggedIn(connection))
    {
        return "{\"result\":\"not logged in\",\"command\":\""+
         command + "\",\"reference\":\"" +
            reference + "\",\"ip\":\"" +
                connection.Address().IPAddress().ToStdString() + "\"}";
    }

#ifdef DETAILED_LOGGING
    logger_base.info("xScheduleCommand received command = '%s' parameters = '%s'.", (const char *)command.c_str(), (const char *)parameters.c_str());
#endif

    std::string result = "";
    size_t rate = 0;
    std::string msg = "";
    if (xScheduleFrame::GetScheduleManager()->Action(command, parameters, data, nullptr, nullptr, rate, msg))
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

std::string ProcessQuery(HttpConnection &connection, const std::string& query, const std::string& parameters, const std::string& reference)
{
    wxStopWatch sw;
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!CheckLoggedIn(connection))
    {
        return "{\"result\":\"not logged in\",\"query\":\""+
            query + "\",\"reference\":\"" +
            reference + "\",\"ip\":\"" +
            connection.Address().IPAddress().ToStdString() + "\"}";
    }

    // log everything but playing status
#ifndef DETAILED_LOGGING
    if (query != "GetPlayingStatus")
#endif
        logger_base.info("xScheduleQuery received query = '%s' parameters = '%s'.", (const char *)query.c_str(), (const char *)parameters.c_str());

    std::string result = "";
    std::string msg;
    if (xScheduleFrame::GetScheduleManager()->Query(query, parameters, result, msg, connection.Address().IPAddress().ToStdString(), reference))
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

std::string ProcessXyzzy(HttpConnection &connection, const std::string& command, const std::string& parameters, const std::string& reference)
{
    wxStopWatch sw;
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string result = "";
    std::string msg;
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

std::string ProcessLogin(HttpConnection &connection, const std::string& credential, const std::string& reference)
{
    wxStopWatch sw;
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    
    std::string result = "";
    if (__password != "")
    {
        std::string cred = connection.Address().IPAddress().ToStdString() + __password;

        // calculate md5 hash
        std::string hash = md5(cred);

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

            result = "{\"result\":\"failed\",\"command\":\"login\",\"message\":\"Login failed.\",\"reference\":\""+reference+"\",\"ip\":\"" + connection.Address().IPAddress().ToStdString() + "\"}";
            logger_base.debug("Security: Login failed. data = '%s'. Time = %ld.", (const char *)result.c_str(), sw.Time());
        }
    }
    else
    {
        result = "{\"result\":\"ok\",\"command\":\"login\",\"reference\":\""+reference+"\"}";
    }

    return result;
}

std::string ProcessStash(HttpConnection &connection, const std::string& command, const std::string& key, std::string& data, const std::string& reference)
{
    wxStopWatch sw;
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!CheckLoggedIn(connection))
    {
        return "{\"result\":\"not logged in\",\"stash\":\""+
            command + "\",\"reference\":\"" +
            reference + "\",\"ip\":\"" +
            connection.Address().IPAddress().ToStdString() + "\"}";
        data = "";
    }

    logger_base.info("xScheduleStash received command = '%s' key = '%s'.", (const char *)command.c_str(), (const char *)key.c_str());

    std::string result = "";
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
            std::string msg = "";
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
        std::string msg = "";
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
        std::string msg = "";
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

bool MyRequestHandler(HttpConnection &connection, HttpRequest &request)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string wwwroot = xScheduleFrame::GetScheduleManager()->GetOptions()->GetWWWRoot();
    if (request.URI().Lower().StartsWith("/xschedulecommand"))
    {
        wxURI url(request.URI());

        std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());
        std::string command = parms["Command"];
        std::string parameters = parms["Parameters"];
        std::string reference = parms["Reference"];

        std::string result = ProcessCommand(connection, command, parameters, request.Data().ToStdString(), reference);

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText(result, "application/json");
        connection.SendResponse(response);

        return true;
    }
    else if (request.URI().Lower().StartsWith("/xyzzy"))
    {
        wxURI url(request.URI().Lower());
        std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());
        std::string command = parms["c"];
        std::string parameters = parms["p"];
        std::string reference = parms["r"];

        std::string result = ProcessXyzzy(connection, command, parameters, reference);

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText(result, "application/json");
        connection.SendResponse(response);

        return true;
    }
    else if (request.URI().Lower().StartsWith("/xschedulelogin"))
    {
        wxURI url(request.URI());

        std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());
        std::string credential = parms["Credential"];
        std::string reference = parms["Reference"];

        std::string result = ProcessLogin(connection, credential, reference);

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText(result, "application/json");
        connection.SendResponse(response);

        return true;
    }
    else if (request.URI().Lower().StartsWith("/xschedulestash"))
    {
        wxURI url(request.URI());

        std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());
        std::string command = parms["Command"];
        std::string key = parms["Key"];
        std::string reference = parms["Reference"];
        std::string data = request.Data().ToStdString();

        std::string result = ProcessStash(connection, command, key, data, reference);

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

        return true;
    }
    else if (request.URI().Lower().StartsWith("/xschedulequery"))
    {
        wxURI url(request.URI());

        std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());
        std::string query = parms["Query"];
        std::string parameters = parms["Parameters"];
        std::string reference = parms["Reference"];

        std::string result = ProcessQuery(connection, query, parameters, reference);

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText(result, "application/json");
        connection.SendResponse(response);

        return true;
    }
    else if (!__apiOnly && (request.URI() == "" || request.URI() == "/" || request.URI() == "/" + wwwroot || request.URI() == "/" + wwwroot + "/"))
    {
        if (wwwroot == "") return false;


        // Chris if you need this line to be this way on linux then use a #ifdef as the other works on windows
        //int port = connection.Server()->Context().Port;
        //wxString url = "http://" + request.Host() + ":" + wxString::Format(wxT("%i"), port) + "/" + wwwroot + "/index.html";
        wxString url = "http://" + request.Host() + "/" + wwwroot + "/index.html";

        logger_base.info("Redirecting to '%s'.", (const char *)url.c_str());

        HttpResponse response(connection, request, HttpStatus::PermanentRedirect);
        response.AddHeader("Location", url);
        //response.AddHeader("Connection", "Close");
        response.MakeFromText("Redirected to " + url + "\n", "text/plain");

        connection.SendResponse(response);

        return true; // disable default processing
    }
    else
    {
        if (wwwroot == "") return false;

        if (!__apiOnly && request.URI().StartsWith("/" + wwwroot))
        {
            wxString d;
#ifdef __WXMSW__
            d = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
#elif __LINUX__
            d = wxStandardPaths::Get().GetDataDir();
            if (!wxDir::Exists(d)) {
                d = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
            }
#else
            d = wxStandardPaths::Get().GetResourcesDir();
#endif

            wxString file = d + wxURI(request.URI()).GetPath();

            logger_base.info("File request received = '%s' : '%s'.", (const char *)file.c_str(), (const char *)request.URI().c_str());

            HttpResponse response(connection, request, HttpStatus::OK);

            //response.AddHeader("Cache-Control", "max-age=14400");

            response.MakeFromFile(file);

            connection.SendResponse(response);

            return true; // disable default processing
        }
    }

    return false; // lets the library's default processing
}

void MyMessageHandler(HttpConnection &connection, WebSocketMessage &message)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (message.Type() == WebSocketMessage::Text)
    {
        wxString text((char *)message.Content().GetData(), message.Content().GetDataLen());
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

        // extract the type of request
        wxString type = root.Get("Type", defaultValue).AsString().Lower();

        std::string result = "";
        if (type == "command")
        {
            wxString c = root.Get("Command", defaultValue).AsString();
            wxString p = root.Get("Parameters", defaultValue).AsString();
            wxString d = root.Get("Data", defaultValue).AsString();
            wxString r = root.Get("Reference", defaultValue).AsString();
            result = ProcessCommand(connection, c.ToStdString(), p.ToStdString(), d.ToStdString(), r.ToStdString());
        }
        else if (type == "query")
        {
            wxString q = root.Get("Query", defaultValue).AsString();
            wxString p = root.Get("Parameters", defaultValue).AsString();
            wxString r = root.Get("Reference", defaultValue).AsString();
            result = ProcessQuery(connection, q.ToStdString(), p.ToStdString(), r.ToStdString());
        }
        else if (type == "xyzzy")
        {
            wxString c = root.Get("c", defaultValue).AsString();
            wxString p = root.Get("p", defaultValue).AsString();
            wxString r = root.Get("r", defaultValue).AsString();
            result = ProcessXyzzy(connection, c.ToStdString(), p.ToStdString(), r.ToStdString());
        }
        else if (type == "login")
        {
            wxString c = root.Get("Credential", defaultValue).AsString();
            wxString r = root.Get("Reference", defaultValue).AsString();
            result = ProcessLogin(connection, c.ToStdString(), r.ToStdString());
        }
        else if (type == "stash")
        {
            wxString c = root.Get("Command", defaultValue).AsString();
            wxString k = root.Get("Key", defaultValue).AsString();
            std::string d = root.Get("Data", defaultValue).AsString().ToStdString();
            wxString r = root.Get("Reference", defaultValue).AsString();
            result = ProcessStash(connection, c.ToStdString(), k.ToStdString(), d, r.ToStdString());
            if (result == "")
            {
                result = d;
            }
        }
        else
        {
            wxString r = root.Get("Reference", defaultValue).AsString();
            WebSocketMessage wsm("{\"result\":\"failed\",\"reference\":\""+r+"\",\"message\":\"Unknown request type.\"}");
            connection.SendMessage(wsm);
            return;
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

void WebServer::SendMessageToAllWebSockets(const std::string& message)
{
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
}

WebServer::WebServer(int port, bool apionly, const std::string& password, int mins)
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

void WebServer::SetPassword(const std::string& password)
{
    __password = password;
}

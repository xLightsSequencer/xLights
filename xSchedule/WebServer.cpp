#include "WebServer.h"

#include <wx/wx.h>
#include <log4cpp/Category.hh>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include "xScheduleMain.h"
#include "ScheduleManager.h"
#include <wx/uri.h>
#include "xScheduleApp.h"
#include "ScheduleOptions.h"
#include "md5.h"

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

void AddToValid(HttpConnection& connection, const HttpHeaders& headers)
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

    std::string security = connection.Address().IPAddress().ToStdString() + "|" + headers["User-Agent"].ToStdString() + "|" + wxDateTime::Now().FormatISOCombined().ToStdString();

    logger_base.debug("Security: Adding record %s.", (const char *)security.c_str());
    __Loggedin.push_back(security);
}

bool CheckLoggedIn(HttpConnection& connection, HttpRequest &request)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (__password == "") return true; // no password ... always logged in

    // remove old logins
    std::list<std::string> toremove;
    for (auto it = __Loggedin.begin(); it != __Loggedin.end(); ++it)
    {
        wxArrayString li = wxSplit(*it, '|');
        wxDateTime lastused;
        lastused.ParseISOCombined(li[2]);
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
            if (li[1] == request.Headers()["User-Agent"])
            {
                li[2] = wxDateTime::Now().FormatISOCombined();
                *it = li[0] + "|" + li[1] + "|" + li[2];
                return true;
            }
        }
    }

    HttpResponse response(connection, request, HttpStatus::OK);
    std::string data = "{\"result\":\"not logged in\",\"ip\":\""+ connection.Address().IPAddress().ToStdString() +"\"}";
    response.MakeFromText(data, "application/json");
    connection.SendResponse(response);

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
            if (x.Count() == 2)
            {
                res[x[0].ToStdString()] = x[1].ToStdString();
            }
        }
    }

    return res;
}

bool MyRequestHandler(HttpConnection &connection, HttpRequest &request)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxStopWatch sw;

    std::string wwwroot = xScheduleFrame::GetScheduleManager()->GetOptions()->GetWWWRoot();
    if (request.URI().Lower().StartsWith("/xschedulecommand"))
    {
        if (!CheckLoggedIn(connection, request)) return true;

        wxURI url(request.URI());

        std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());

        std::string command = parms["Command"];
        std::string parameters = parms["Parameters"];

#ifdef DETAILED_LOGGING
        logger_base.info("xScheduleCommand received command = '%s' parameters = '%s'.", (const char *)command.c_str(), (const char *)parameters.c_str());
#endif

        std::string data = request.Data().ToStdString();
        size_t rate = 0;
        std::string msg = "";
        HttpResponse response(connection, request, HttpStatus::OK);
        if (xScheduleFrame::GetScheduleManager()->Action(command, parameters, data, nullptr, nullptr, rate, msg))
        {
            wxCommandEvent event(EVT_FRAMEMS);
            event.SetInt(rate);
            wxPostEvent(wxGetApp().GetTopWindow(), event);

            response.MakeFromText("{\"result\":\"ok\"}", "application/json");

#ifdef DETAILED_LOGGING
            logger_base.info("    Time %ld.", sw.Time());
#endif
        }
        else
        {
            std::string respdata = "{\"result\":\"failed\",\"message\":\"" + msg + "\"}";
            logger_base.info("    data = '%s'. Time %ld.", (const char *)respdata.c_str(), sw.Time());
            response.MakeFromText(respdata, "application/json");
        }
        connection.SendResponse(response);
        return true;
    }
    else if (request.URI().Lower().StartsWith("/xyzzy"))
    {
        wxURI url(request.URI().Lower());
        std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());
        std::string command = parms["c"];
        std::string parameters = parms["p"];

        std::string msg;
        HttpResponse response(connection, request, HttpStatus::OK);
        if (xScheduleFrame::GetScheduleManager()->DoXyzzy(command, parameters, msg))
        {
#ifdef DETAILED_LOGGING
            logger_base.info("xyzzy command=%s parameters=%s result='%s'. Time %ld.", (const char *)command.c_str(), (const char*)parameters.c_str(), (const char *)msg.c_str(), sw.Time());
#endif
            response.MakeFromText(msg, "application/json");
        }
        else
        {
            std::string data = "{\"result\":\"failed\",\"message\":\"" + msg + "\"}";
            logger_base.info("xyzzy command=%s parameters=%s result='%s'. Time %ld.", (const char *)command.c_str(), (const char*)parameters.c_str(), (const char *)data.c_str(), sw.Time());
            response.MakeFromText(data, "application/json");
        }
        connection.SendResponse(response);
        return true;
    }
    else if (request.URI().Lower().StartsWith("/xschedulelogin"))
    {
        if (__password != "")
        {
            wxURI url(request.URI());

            std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());

            std::string credential = parms["Credential"];

            std::string cred = connection.Address().IPAddress().ToStdString() + __password;

            // calculate md5 hash
            std::string hash = md5(cred);

            HttpResponse response(connection, request, HttpStatus::OK);
            if (hash == credential)
            {
                // this is a valid login
                AddToValid(connection, request.Headers());
                response.MakeFromText("{\"result\":\"ok\"}", "application/json");
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
                std::string data = "{\"result\":\"failed\",\"message\":\"Login failed.\",\"ip\":\""+ connection.Address().IPAddress().ToStdString() +"\"}";
                response.MakeFromText(data, "application/json");
                logger_base.debug("Security: Login failed. data = '%s'. Time = %ld.", (const char *)data.c_str(), sw.Time());
            }
            connection.SendResponse(response);
            return true;
        }
        else
        {
            HttpResponse response(connection, request, HttpStatus::OK);
            response.MakeFromText("{\"result\":\"ok\"}", "application/json");
            connection.SendResponse(response);
            return true;
        }
    }
    else if (request.URI().Lower().StartsWith("/xschedulestash"))
    {
        if (!CheckLoggedIn(connection, request)) return true;

        wxURI url(request.URI());

        std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());

        std::string command = parms["Command"];
        std::string key = parms["Key"];

        logger_base.info("xScheduleStash received command = '%s' key = '%s'.", (const char *)command.c_str(), (const char *)key.c_str());

        if (wxString(command).Lower() == "store")
        {
            std::string data = request.Data().ToStdString();

            logger_base.info("    data = '%s'.", (const char *)data.c_str());

            // now store it in a file
            std::string msg = "";
            HttpResponse response(connection, request, HttpStatus::OK);
            if (xScheduleFrame::GetScheduleManager()->StoreData(key, data, msg))
            {
#ifdef DETAILED_LOGGING
                logger_base.info("    Time %ld.", sw.Time());
#endif
                response.MakeFromText("{\"result\":\"ok\"}", "application/json");
            }
            else
            {
                data = "{\"result\":\"failed\",\"message\":\"" + msg + "\"}";
                logger_base.info("    data = '%s'. Time = %ld.", (const char *)data.c_str(), sw.Time());
                response.MakeFromText(data, "application/json");
            }
            connection.SendResponse(response);
        }
        else if (wxString(command).Lower() == "retrieve")
        {
            std::string data = "";
            std::string msg = "";
            HttpResponse response(connection, request, HttpStatus::OK);
            if (xScheduleFrame::GetScheduleManager()->RetrieveData(key, data, msg))
            {
                logger_base.info("    data = '%s'. Time = %ld.", (const char *)data.c_str(), sw.Time());

                response.MakeFromText(data, "text/plain");
            }
            else
            {
                data = "{\"result\":\"failed\",\"message\":\"" + msg + "\"}";
                logger_base.info("    data = '' : '%s'. Time = %ld.", (const char *)data.c_str(), sw.Time());
                response.MakeFromText("", "application/json");
            }
            connection.SendResponse(response);
        }
        else
        {
            HttpResponse response(connection, request, HttpStatus::OK);
            std::string data = "{\"result\":\"failed\",\"message\":\"Unknown stash command.\"}";
            logger_base.info("    '%s'. Time = %ld.", (const char *)data.c_str(), sw.Time());
            response.MakeFromText(data, "application/json");
            connection.SendResponse(response);
        }
        return true;
    }
    else if (request.URI().Lower().StartsWith("/xschedulequery"))
    {
        if (!CheckLoggedIn(connection, request)) return true;

        wxURI url(request.URI());

        std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());

        std::string query = parms["Query"];
        std::string parameters = parms["Parameters"];

        // log everything but playing status
#ifndef DETAILED_LOGGING
        if (query != "GetPlayingStatus")
#endif
            logger_base.info("xScheduleQuery received query = '%s' parameters = '%s'.", (const char *)query.c_str(), (const char *)parameters.c_str());

        std::string data = "";
        std::string msg;
        HttpResponse response(connection, request, HttpStatus::OK);
        if (xScheduleFrame::GetScheduleManager()->Query(query, parameters, data, msg, connection.Address().IPAddress().ToStdString()))
        {
#ifndef DETAILED_LOGGING
            if (query != "GetPlayingStatus")
#endif
                logger_base.info("    data = '%s'. Time = %ld.", (const char *)data.c_str(), sw.Time());

            response.MakeFromText(data, "application/json");
        }
        else
        {
            data = "{\"result\":\"failed\",\"message\":\"" + msg + "\"}";
            logger_base.info("    data = '' : '%s'. Time = %ld.", (const char *)data.c_str(), sw.Time());
            response.MakeFromText(data, "application/json");
        }

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
        WebSocketMessage wsm(text);
        connection.SendMessage(wsm); // resend text
    }
    else if (message.Type() == WebSocketMessage::Binary)
    {
        //...
        logger_base.info("Received <binary>");
        WebSocketMessage wsm("received binary data");
        connection.SendMessage(wsm);
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

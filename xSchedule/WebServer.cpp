#include "WebServer.h"

#include <wx/wx.h>
#include <log4cpp/Category.hh>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include "xScheduleMain.h"
#include "ScheduleManager.h"
#include <wx/uri.h>
#include <wx/dir.h>
#include "xScheduleApp.h"
#include "ScheduleOptions.h"

bool __apiOnly = false;

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

    std::string wwwroot = xScheduleFrame::GetScheduleManager()->GetOptions()->GetWWWRoot();

    if (request.URI().StartsWith("/xScheduleCommand"))
    {
        wxURI url(request.URI());

        std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());

        std::string command = parms["Command"];
        std::string parameters = parms["Parameters"];

        logger_base.info("xScheduleCommand received command = '%s' parameters = '%s'.", (const char *)command.c_str(), (const char *)parameters.c_str());

        size_t rate = 0;
        std::string msg = "";
        HttpResponse response(connection, request, HttpStatus::OK);
        if (xScheduleFrame::GetScheduleManager()->Action(command, parameters, nullptr, nullptr, rate, msg))
        {
            wxCommandEvent event(EVT_FRAMEMS);
            event.SetInt(rate);
            wxPostEvent(wxGetApp().GetTopWindow(), event);

            response.MakeFromText("{\"result\":\"ok\"}", "application/json");
        }
        else
        {
            std::string data = "{\"result\":\"failed\",\"message\":\"" + msg + "\"}";
            logger_base.info("    data = '%s'.", (const char *)data.c_str());
            response.MakeFromText(data, "application/json");
        }
        connection.SendResponse(response);
        return true;
    }
    else if (request.URI().StartsWith("/xScheduleStash"))
    {
        wxURI url(request.URI());

        std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());

        std::string command = parms["Command"];
        std::string key = parms["Key"];

        logger_base.info("xScheduleStash received command = '%s' key = '%s'.", (const char *)command.c_str(), (const char *)key.c_str());

        if (command == "Store")
        {
            std::string data = request.Data().ToStdString();

            logger_base.info("    data = '%s'.", (const char *)data.c_str());

            // now store it in a file
            std::string msg = "";
            HttpResponse response(connection, request, HttpStatus::OK);
            if (xScheduleFrame::GetScheduleManager()->StoreData(key, data, msg))
            {
                response.MakeFromText("{\"result\":\"ok\"}", "application/json");
            }
            else
            {
                data = "{\"result\":\"failed\",\"message\":\"" + msg + "\"}";
                logger_base.info("    data = '%s'.", (const char *)data.c_str());
                response.MakeFromText(data, "application/json");
            }
            connection.SendResponse(response);
        }
        else if (command == "Retrieve")
        {
            std::string data = "";
            std::string msg = "";
            HttpResponse response(connection, request, HttpStatus::OK);
            if (xScheduleFrame::GetScheduleManager()->RetrieveData(key, data, msg))
            {
                logger_base.info("    data = '%s'.", (const char *)data.c_str());

                response.MakeFromText(data, "text/plain");
            }
            else
            {
                data = "{\"result\":\"failed\",\"message\":\"" + msg + "\"}";
                logger_base.info("    data = '' : '%s'.", (const char *)data.c_str());
                response.MakeFromText("", "application/json");
            }
            connection.SendResponse(response);
        }
        else
        {
            HttpResponse response(connection, request, HttpStatus::OK);
            std::string data = "{\"result\":\"failed\",\"message\":\"Unknown stash command.\"}";
            logger_base.info("    '%s'.", (const char *)data.c_str());
            response.MakeFromText(data, "application/json");
            connection.SendResponse(response);
        }
        return true;
    }
    else if (request.URI().StartsWith("/xScheduleQuery"))
    {
        wxURI url(request.URI());

        std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());

        std::string query = parms["Query"];
        std::string parameters = parms["Parameters"];

        // log everything but playing status
        if (query != "GetPlayingStatus")
            logger_base.info("xScheduleQuery received query = '%s' parameters = '%s'.", (const char *)query.c_str(), (const char *)parameters.c_str());

        std::string data = "";
        std::string msg;
        HttpResponse response(connection, request, HttpStatus::OK);
        if (xScheduleFrame::GetScheduleManager()->Query(query, parameters, data, msg))
        {
            logger_base.info("    data = '%s'.", (const char *)data.c_str());

            response.MakeFromText(data, "application/json");
        }
        else
        {
            data = "{\"result\":\"failed\",\"message\":\"" + msg + "\"}";
            logger_base.info("    data = '' : '%s'.", (const char *)data.c_str());
            response.MakeFromText("", "application/json");
        }

        connection.SendResponse(response);
        return true;
    }
    else if (!__apiOnly && request.URI() == "" || request.URI() == "/" || request.URI() == "/" + wwwroot || request.URI() == "/" + wwwroot + "/")
    {
        int port = connection.Server()->Context().Port;

        // Chris if you need this line to be this way on linux then use a #ifdef as the other works on windows
        //wxString url = "http://" + request.Host() + ":" + wxString::Format(wxT("%i"), port) + "/" + wwwroot + "/index.html";
        wxString url = "http://" + request.Host() + "/" + wwwroot + "/index.html";

        logger_base.info("Redirecting to '%s'.", (const char *)url.c_str());

        HttpResponse response(connection, request, HttpStatus::PermanentRedirect);
        response.AddHeader("location", url);

        connection.SendResponse(response);

        return true; // disable default processing
    }
    else
    {
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
            d = wxStandardPaths::Get().GetExecutablePath();
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

WebServer::WebServer(int port, bool apionly)
{
    __apiOnly = apionly; // put this in a global.

    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    HttpContext context;
    context.Port = port;
    context.RequestHandler = MyRequestHandler;
    context.MessageHandler = MyMessageHandler;

    if (!Start(context))
    {
        logger_base.error("Error starting web server.");
    }
}

WebServer::~WebServer()
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    Stop();
}

void WebServer::SetAPIOnly(bool apiOnly)
{
    __apiOnly = apiOnly;
}

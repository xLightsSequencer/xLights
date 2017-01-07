#include "WebServer.h"

#include <wx/wx.h>
#include <log4cpp/Category.hh>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include "xScheduleMain.h"
#include "ScheduleManager.h"
#include <wx/uri.h>
#include "xScheduleApp.h"

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
    if (request.URI().StartsWith("/xScheduleCommand"))
    {
        wxURI url(request.URI());

        std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());

        std::string command = parms["Command"];
        std::string parameters = parms["Parameters"];

        size_t rate = 0;
        xScheduleFrame::GetScheduleManager()->Action(command, parameters, nullptr, rate);

        wxCommandEvent event(EVT_FRAMEMS);
        event.SetInt(rate);
        wxPostEvent(wxGetApp().GetTopWindow(), event);

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText("{“result”:”ok”}", "application/json");
        connection.SendResponse(response);
    }
    if (request.URI().StartsWith("/xScheduleQuery"))
    {
        wxURI url(request.URI());

        std::map<std::string, std::string> parms = ParseURI(url.BuildUnescapedURI().ToStdString());

        std::string query = parms["Query"];
        std::string parameters = parms["Parameters"];

        std::string result = xScheduleFrame::GetScheduleManager()->Query(query, parameters);

        HttpResponse response(connection, request, HttpStatus::OK);
        response.MakeFromText(result, "application/json");
        connection.SendResponse(response);
    }
    else if (request.URI().StartsWith("/xSchedule"))
    {
        wxString d;
#ifndef __WXMSW__
        d = wxStandardPaths::Get().GetExecutablePath();
#else
        d = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
#endif

        wxString file = d + request.URI();

        HttpResponse response(connection, request, HttpStatus::OK);

        response.MakeFromFile(file);

        connection.SendResponse(response);

        return true; // disable default processing
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

WebServer::WebServer(int port)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    HttpContext context;
    context.Port = 80; // port;
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


// Copyright (c) 2014 framerik <framerik@gmail.com>
// All rights reserved
//
// This library is dual-licensed: you can redistribute it and/or modify it under the terms
// of the GNU General Public License version 2 as published by the Free Software Foundation.
// For the terms of this license, see <http://www.gnu.org/licenses/>.
//
// You are free to use this library under the terms of the GNU General Public License, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// If you find this project useful and wish to donate you can do with an
// Amazon Gift Cards - E-mail Delivery https://www.amazon.it/gp/product/B005VG4G3U/gcrnsts

#include "wxhttpserver.h"
#include <log4cpp/Category.hh>

//#define DETAILED_LOGGING

#define SERVER_ID	100
#define SOCKET_ID	101

#include <wx/arrimpl.cpp>
//WX_DEFINE_EXPORTED_OBJARRAY(HeadersCollection);
WX_DEFINE_OBJARRAY(HeadersCollection)

BEGIN_EVENT_TABLE(HttpServer, wxEvtHandler)
	EVT_SOCKET(SERVER_ID, HttpServer::OnServerEvent)
	EVT_SOCKET(SOCKET_ID, HttpServer::OnSocketEvent)
END_EVENT_TABLE()

HttpServer::HttpServer() :
	_server(NULL)
{
}

HttpServer::~HttpServer()
{
    Stop();
}

bool HttpServer::Start(const HttpContext &context)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxASSERT(!_server || (_server && _server->IsClosed()));

	if (_server && _server->IsConnected())
		Stop();

	_context = context;

	// Create the address - defaults to localhost:0 initially
	_address.Service(_context.Port);

	wxLogMessage(_("starting server on %s:%u..."), _address.IPAddress(), _address.Service());
    logger_base.info("starting server on %s:%u...", (const char *)_address.IPAddress().c_str(), _address.Service());

	// Create the socket
	_server = new wxSocketServer(_address, wxSOCKET_REUSEADDR);

	// We use IsOk() here to see if the server is really listening
    if (!_server->IsOk())
    {
        wxLogError(_("unable to start the server on the specified port"));
        logger_base.error(_("unable to start the server on the specified port"));
    }
	else
	{
		IPaddress address;
        if (!_server->GetLocal(address))
        {
            wxLogError(_("unable to retrieve the address to which you are connected"));
            logger_base.error(_("unable to retrieve the address to which you are connected"));
        }
        else
        {
            wxLogMessage(_("server running on %s:%u"), address.IPAddress(), address.Service());
            logger_base.info("server running on %s:%u", (const char *)address.IPAddress().c_str(), address.Service());
        }

		// Setup the event handler and subscribe to connection events
		_server->SetEventHandler(*this, SERVER_ID);
		_server->SetNotify(wxSOCKET_CONNECTION_FLAG);
		_server->Notify(true);
	}

	return _server->IsOk();
}

bool HttpServer::Stop()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    
    if (!_server) return false;

    // close all open connections
    for (auto it = _connections.begin(); it != _connections.end(); ++it)
    {
        it->second->Close();
    }

    _server->Notify(false);
	_server->Close();
	delete _server;
    _server = nullptr;

	wxLogMessage(_("closed server on %s:%u"), _address.IPAddress(), _address.Service());
    logger_base.debug("closed server on %s:%u", (const char*)_address.IPAddress().c_str(), _address.Service());

	return true;
}

void HttpServer::OnServerEvent(wxSocketEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxStopWatch sw;

    switch (event.GetSocketEvent())
    {
    case wxSOCKET_CONNECTION:
#ifdef DETAILED_LOGGING
        logger_base.info("OnServerEvent: wxSOCKET_CONNECTION");
#endif
        break;
    default:
#ifdef DETAILED_LOGGING
        logger_base.info("OnServerEvent: unexpected event %d", event.GetSocketEvent());
#endif
        break;
    }

    // Accept new connection if there is one in the pending connections queue, else exit. We use Accept(false) for
    // non-blocking accept (although if we got here, there should ALWAYS be a pending connection).
    wxSocketBase *socket = _server->Accept(false);

    if (socket)
    {
#ifdef DETAILED_LOGGING
        logger_base.info("created socket client (socket %d)", socket->GetSocket());
#endif

        HttpConnection *connection = nullptr;

        if (event.GetSocketEvent() == wxSOCKET_CONNECTION)
        {
            connection = new HttpConnection(this, socket);
            _connections[socket] = connection;
        }
        else
            connection = _connections[socket];

        wxASSERT(connection != nullptr);

        socket->SetEventHandler(*this, SOCKET_ID);
        socket->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
        socket->Notify(true);
    }
    else
    {
#ifdef DETAILED_LOGGING
        logger_base.info("error, impossible to accept a new connection");
#endif
    }

#ifdef DETAILED_LOGGING
    logger_base.info("OnServerEvent Time %ld.", sw.Time());
#endif
}

void HttpServer::OnSocketEvent(wxSocketEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxStopWatch sw;

	switch(event.GetSocketEvent())
	{
	case wxSOCKET_INPUT:
#ifdef DETAILED_LOGGING
        logger_base.info("OnSocketEvent: wxSOCKET_INPUT");
#endif
		break;
	case wxSOCKET_LOST:
#ifdef DETAILED_LOGGING
        logger_base.info("OnSocketEvent: wxSOCKET_LOST");
#endif
		break;
	default:
#ifdef DETAILED_LOGGING
        logger_base.info("OnServerEvent: unexpected event %d", event.GetSocketEvent());
#endif
		break;
	}

	wxSocketBase *socket = event.GetSocket();

	HttpConnection *connection = _connections[socket];
	wxASSERT(connection != NULL);

	// Now we process the event
	switch(event.GetSocketEvent())
	{
	case wxSOCKET_INPUT:
		{
			// We disable input events, so that the test doesn't trigger wxSocketEvent again.
			socket->SetNotify(wxSOCKET_LOST_FLAG);
			socket->SetFlags(wxSOCKET_NOWAIT);

			connection->HandleRequest();

			// Enable input events again.
			socket->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
			break;
		}
	case wxSOCKET_LOST:
		{
			_connections.erase(_connections.find(socket));
			delete connection;

			// Destroy() should be used instead of delete wherever possible, due to the fact that wxSocket uses 'delayed events'
			// (see the documentation for wxPostEvent) and we don't want an event to arrive to the event handler (the frame,
			// here) after the socket has been deleted. Also, we might be doing some other thing with the socket at the same
			// time; for example, we might be in the middle of a test or something. Destroy() takes care of all this for us.
#ifdef DETAILED_LOGGING
            logger_base.info("deleted socket client (socket %d)", socket->GetSocket());
#endif
			socket->Destroy();
			break;
		}
    default:
        break;
	}

#ifdef DETAILED_LOGGING
    logger_base.info("OnSocketEvent Time %ld.", sw.Time());
#endif
}

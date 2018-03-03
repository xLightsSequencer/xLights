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
#include "sha1.h"

#include <wx/buffer.h>
#include <wx/base64.h>
#include <wx/filename.h>

HttpConnection::HttpConnection(HttpServer *server, wxSocketBase *socket) :
	_server(server),
	_socket(socket),
	_isWebSocket(false),
	_message(NULL)
{
	if (!_socket->GetPeer(_address))
		wxLogMessage(_("accepted a new connection from <unknown> (socket %d)"), socket->GetSocket());
	else
		wxLogMessage(_("accepted a new connection from %s:%u (socket %d)"), _address.IPAddress(), _address.Service(), socket->GetSocket());
}

HttpConnection::~HttpConnection()
{
	wxLogMessage(_("connection closed (socket %d)"), _socket->GetSocket());
}

bool HttpConnection::HandleRequest()
{
	_socket->SetFlags(wxSOCKET_NOWAIT);

	wxMemoryBuffer input;
	char           buffer[1024];

	do
	{
		_socket->Read(buffer, sizeof(buffer));
		input.AppendData(buffer, _socket->LastReadCount());
	} while (_socket->LastReadCount() > 0);

	if (_isWebSocket)
	{
		return ParseFrame(input);
	}
	else
	{
		HttpRequest request(*this, wxString((char *)input.GetData(), input.GetDataLen()));

		if (request.Method() == "GET")
		{
			if (request["Upgrade"].CmpNoCase("websocket") == 0)
			{
				if (!request["Host"].IsEmpty() && !request["Connection"].IsEmpty() &&
					!request["Sec-WebSocket-Key"].IsEmpty() && !request["Sec-WebSocket-Version"].IsEmpty())
					return WebSocketHandshake(request);
				else
                {
                    HttpResponse hr(*this, request, HttpStatus::BadRequest);
					return SendResponse(hr);
                }
			}
			else
			{
				if (_server->_context.RequestHandler)
				{
					if (_server->_context.RequestHandler(*this, request))
						return true;
				}

				wxString fileName(_server->_context.DefaultDirectory);
				fileName += wxFILE_SEP_PATH;

				if (request.URI() == "/")
				{
					for (size_t i = 0; i < _server->_context.DefaultDocuments.Count(); i++)
					{
						if (wxFileName::FileExists(fileName + _server->_context.DefaultDocuments[i]))
						{
							fileName += _server->_context.DefaultDocuments[i];
							break;
						}
					}
				}
				else
					fileName += request.URI().Mid(1);

				HttpResponse response(*this, request, fileName);

				return SendResponse(response);
			}
		}
		else
		{
			// all others requests are routed to custom implementations
			if (_server->_context.RequestHandler)
			{
				if (_server->_context.RequestHandler(*this, request))
					return true;
			}
		}
	}

	return false;
}

bool HttpConnection::SendResponse(HttpResponse &response)
{
	wxString row = wxString::Format("%s %d %s\r\n", response.Version(), response.Status().Code(), response.Status().Description());
	_socket->Write(row.ToAscii(), row.Length());
	if (_socket->Error())
		return false;

	for (size_t i = 0; i < response.Headers().Count(); i++)
	{
		wxString header = response[i];
		_socket->Write(header.ToAscii(), header.Length());
		if (_socket->Error())
			return false;
	}

	_socket->Write("\r\n", 2);
	if (_socket->Error())
		return false;

	if (!response._content.IsEmpty())
	{
		_socket->Write(response._content.GetData(), response._content.GetDataLen());
		if (_socket->Error())
			return false;
	}

	return true;
}

bool HttpConnection::SendMessage(WebSocketMessage &message)
{
	wxMemoryBuffer header;

	header.AppendByte((wxUint8)0x80 | message._type); // final + type

	if (message._content.GetDataLen() > 0xFFFF)
	{
		wxUint64 tmp = message._content.GetDataLen();
		header.AppendByte(127);
		for (int i = 7; i >= 0; i--)
		{
			header.AppendByte((wxUint8)(tmp & 0x0FF));
			tmp = tmp / 0x100;
		}
	}
	else if (message._content.GetDataLen() > 125)
	{
		header.AppendByte(126);
		header.AppendByte((wxUint8)((message._content.GetDataLen() & 0xFF00) / 0x100));
		header.AppendByte((wxUint8)(message._content.GetDataLen() & 0xFF));
	}
	else
	{
		header.AppendByte((wxUint8) message._content.GetDataLen());
	}

	_socket->Write(header.GetData(), header.GetDataLen());
	if (_socket->Error())
		return false;

	if (!message._content.IsEmpty())
		_socket->Write(message._content.GetData(), message._content.GetDataLen());

	return !_socket->Error();
}

bool HttpConnection::Close()
{
	wxASSERT(_socket);

	bool result = _socket->Close();
	_socket = NULL;

	return result;
}

bool HttpConnection::ParseFrame(wxMemoryBuffer &buffer)
{
	if (buffer.GetDataLen() < 2)
		return false;

    bool done = false;
    int pos = 0;

    while (!done)
    {
        bool final = buffer[pos + 0] & 0x80 ? true : false;

        if (((buffer[pos + 0] & 0x40) != 0) || ((buffer[pos + 0] & 0x20) != 0) || ((buffer[pos + 0] & 0x10) != 0))
        {
            wxLogWarning("reserved bits must be 0");
            return false; // Invalid ws message
        }

        WebSocketMessage::Opcode type = (WebSocketMessage::Opcode)(buffer[pos + 0] & 0x0F);

        if (!_message)
            _message = new WebSocketMessage(type);

        //ws_conn->close  = ((buffer[0] & 0x0F) == 0x08);
        //ws_conn->ping   = ((buffer[0] & 0x0F) == 0x09);
        //ws_conn->pong   = ((buffer[0] & 0x0F) == 0x0A);

        if (type == WebSocketMessage::Ping)
            wxLogMessage("received a PING message");

        if (type == WebSocketMessage::Pong)
            wxLogMessage("received a PONG message");

        int length = buffer[pos + 1] & 0x7F;
        int     start = 2;
        wxUint8 mask[4] = { 0 };

        if (length == 126)
        {
            length = ((((int)buffer[pos + 2]) << 8) + buffer[pos + 3]);
            start = 4;
        }
        else if (length == 127)
        {
            length = 0;
            for (int i = 0; i < 8; i++)
                length = (wxUint64)length * 0x100 + buffer[pos + 2 + i];
            start = 10;
        }

        if (buffer[pos + 1] & 128)
        {
            memcpy(mask, &(buffer[pos + start]), 4);
            start += 4;
        }

        for (int i = 0; i < length; i++)
            _message->_content.AppendByte(buffer[pos + start + i] ^ mask[i % 4]);

        if (final && _server->_context.MessageHandler)
        {
            _server->_context.MessageHandler(*this, *_message);

            switch (_message->_type)
            {
            case WebSocketMessage::Ping:
            {
                WebSocketMessage wsm(WebSocketMessage::Pong);
                SendMessage(wsm);
            }
            break;
            case WebSocketMessage::Close:
            {
                WebSocketMessage wsm(WebSocketMessage::Close);
                SendMessage(wsm);
            }
            break;
            default:
            break;
            }

            delete _message;
            _message = nullptr;
        }

        pos += start + length;

        if (pos >= buffer.GetDataLen())
        {
            done = true;
        }
    }

	return true;
}

bool HttpConnection::WebSocketHandshake(HttpRequest &request)
{
	static const char *magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	if (request["Sec-WebSocket-Version"] != "13")
    {
        HttpResponse hr(*this, request, HttpStatus::UpgradeRequired);
		return SendResponse(hr);
    }

	wxString handshake = wxString::Format("%s%s", request["Sec-WebSocket-Key"], magic);

	SHA1Context ctx = { 0 };
	uint8_t     sha[20] = { 0 };

	SHA1Reset(&ctx);
	SHA1Input(&ctx, (unsigned char *)(const char *)handshake.c_str(), handshake.Length());
	SHA1Result(&ctx, sha);

	HttpResponse response(*this, request, HttpStatus::SwitchingProtocols);
	response.AddHeader("Upgrade", "websocket");
	response.AddHeader("Connection", "Upgrade");
	if (!request["Sec-WebSocket-Protocol"].IsEmpty())
		response.AddHeader("Sec-WebSocket-Protocol", request["Sec-WebSocket-Protocol"]);
	response.AddHeader("Sec-WebSocket-Accept", wxBase64Encode(sha, 20));

	SendResponse(response);

	_isWebSocket = true;

	return true;
}

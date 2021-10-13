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

#ifndef __HTTP_H__
#define __HTTP_H__

#include <wx/wxprec.h> // For compilers that support precompilation, includes "wx/wx.h".
#ifndef WX_PRECOMP // for all others, include the necessary headers
	#include <wx/wx.h>
#endif // WX_PRECOMP

#include <wx/socket.h>
#include <wx/dynarray.h>
#include <wx/hash.h>

#define SERVER_NAME    "xLights Web Server"
#define SERVER_VERSION "1.0"

#undef wxUSE_IPV6

// server can be extended to allow using IPv6 sockets
#if wxUSE_IPV6
	typedef wxIPV6address IPaddress;
#else // wxUSE_IPV6
	typedef wxIPV4address IPaddress;
#endif // wxUSE_IPV6

WX_DECLARE_STRING_HASH_MAP(wxString, wxHashString);

// Forward declarations
class HttpStatus;
class HttpHeader;
class HttpContext;
class HttpServer;
class HttpConnection;
class HttpRequest;
class HttpResponse;
class WebSocketMessage;

typedef bool (*RequestHandlerPtr)(HttpConnection &connection, HttpRequest &request);
typedef void (*MessageHandlerPtr)(HttpConnection &connection, WebSocketMessage &message);

// Class for global HTTP server settings
class /*WXDLLIMPEXP_BASE*/ HttpContext
{
public:
	HttpContext();

	// listening port
	int           Port;
	// default server directory
	wxString      DefaultDirectory;
	// list of predefined documents
	wxArrayString DefaultDocuments;

	// overridables
	RequestHandlerPtr RequestHandler;
	MessageHandlerPtr MessageHandler;

	// default error pages content
	const char *ErrorPage400;
	const char *ErrorPage404;
};

// Status code for HTTP response
class /* WXDLLIMPEXP_BASE */ HttpStatus
{
public:
	enum HttpStatusCode
	{
		Continue = 100,
		SwitchingProtocols = 101,

		OK = 200,
		Created = 201,
		Accepted = 202,
		NonAuthoritativeInformation = 203,
		NoContent = 204,
		ResetContent = 205,
		PartialContent = 206,
		MultiStatus = 207,

		MultipleChoices = 300,
		MovedPermanently = 301,
		Found = 302,
		SeeOther = 303,
		NotModified = 304,
		UseProxy = 305,
		SwitchProxy = 306,
		TemporaryRedirect = 307,
		PermanentRedirect = 308,

		BadRequest = 400,
		Unauthorized = 401,
		PaymentRequired = 402,
		Forbidden = 403,
		NotFound = 404,
		MethodNotAllowed = 405,
		NotAcceptable = 406,
		ProxyAuthenticationRequired = 407,
		RequestTimeout = 408,
		Conflict = 409,
		Gone = 410,
		LengthRequired = 411,
		PreconditionFailed = 412,
		RequestEntityTooLarge = 413,
		RequestURITooLong = 414,
		UnsupportedMediaType = 415,
		RequestedRangeNotSatisfiable = 416,
		ExpectationFailed = 417,
		//I'm a teapot = 418,
		UpgradeRequired = 426,
		RetryWith = 449,
		UnavailableForLegalReasons = 451,

		InternalServerError = 500,
		NotImplemented = 501,
		BadGateway = 502,
		ServiceUnavailable = 503,
		GatewayTimeout = 504,
		HTTPVersionNotSupported = 505,
		BandwidthLimitExceeded = 509,
	};

	HttpStatus(HttpStatusCode code = HttpStatus::OK);

	// Property Code
	inline HttpStatusCode Code() { return _code; }
	inline void SetCode(HttpStatusCode value) { _code = value; }

	// Property Description
	wxString Description();

protected:
	HttpStatusCode _code;
};

// Http header
class /* WXDLLIMPEXP_BASE */ HttpHeader
{
public:
	HttpHeader() { }
	HttpHeader(const wxString &key, const wxString &value) :
		_key(key),
		_value(value)
	{ }

	inline operator wxString() { return wxString::Format("%s: %s\r\n", _key, _value); }

protected:
	wxString _key;
	wxString _value;

	friend class HttpHeaders;
};

//WX_DECLARE_EXPORTED_OBJARRAY(HttpHeader, HeadersCollection);
WX_DECLARE_OBJARRAY(HttpHeader, HeadersCollection);

// Http headers collection
class WXDLLIMPEXP_BASE HttpHeaders : public HeadersCollection
{
public:
	inline void Add(const wxString &key, const wxString &value)
	{
		HeadersCollection::Add(HttpHeader(key, value));
	}

	inline wxString operator[](int index) const
	{ return Item(index); }

	wxString operator[](const wxString &key) const
	{
		for (size_t i = 0; i < Count(); i++)
		{
			HttpHeader &header = Item(i);
			if (header._key == key)
				return header._value;
		}

		return wxEmptyString;
	}

};

// Connection class
class /* WXDLLIMPEXP_BASE */ HttpConnection
{
public:
	HttpConnection(HttpServer *server, wxSocketBase *socket);
	virtual ~HttpConnection();

	virtual bool HandleRequest();
	virtual bool SendResponse(HttpResponse &response);
	virtual bool SendMessage(WebSocketMessage &message);
	virtual bool Close();

	// properties
	inline bool IsOpen() { return (_socket && _socket->IsOk()); }
	inline const HttpServer *Server() const { return _server; }
	inline const wxSocketBase *Socket() const { return _socket; }
	inline const IPaddress &Address() { return _address; }
	inline bool IsWebSocket() { return _isWebSocket; }

protected:
	void ParseRequest(const wxString &content);
	bool ParseFrame(wxMemoryBuffer &buffer);
	bool WebSocketHandshake(HttpRequest &request);

protected:
	HttpServer       *_server;
	wxSocketBase     *_socket;
	IPaddress         _address;
	bool              _isWebSocket;
	WebSocketMessage *_message;
};

//WX_DECLARE_EXPORTED_HASH_MAP(wxSocketBase *, HttpConnection *, wxPointerHash, wxPointerEqual, ConnectionMap);
WX_DECLARE_HASH_MAP(wxSocketBase *, HttpConnection *, wxPointerHash, wxPointerEqual, ConnectionMap);

// HTTP request
class /* WXDLLIMPEXP_BASE */ HttpRequest
{
public:
	HttpRequest(HttpConnection &connection);
	HttpRequest(HttpConnection &connection, const wxString &input);

	virtual void Parse(const wxString &input);

	// properties

	// valid methods are GET, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT
	inline const wxString &Method() const { return _method; }
	// URI of requested document or resource
	inline const wxString &URI() const { return _uri; }
	// HTTP version, default HTTP/1.1
	inline const wxString &Version() const { return _version; }
	// http headers
	inline const HttpHeaders &Headers() const { return _headers; }
    inline const wxString &Data() const { return _data; }

	// server host name for requested URI
	inline wxString Host() const { return _headers["Host"]; }
	// client type identification: browser, producer, version...
    inline wxString UserAgent() const { return _headers["User-Agent"]; }

	inline wxString operator[](int index) const { return _headers[index]; }
	inline wxString operator[](const wxString &key) const { return _headers[key]; }

protected:
	HttpConnection &_connection;
	wxString        _method;
	wxString        _uri;
	wxString        _version;
    wxString        _data;
	HttpHeaders     _headers;
};

// Generic HTTP response
class /* WXDLLIMPEXP_BASE */ HttpResponse
{
public:
	HttpResponse(HttpConnection &connection, HttpRequest &request, HttpStatus::HttpStatusCode code = HttpStatus::OK);
	HttpResponse(HttpConnection &connection, HttpRequest &request, const wxString &fileName, const wxString &charset = wxEmptyString);

	inline void AddHeader(const wxString &key, const wxString &value)
	{ _headers.Add(key, value); }

	// make a response from a server file
	virtual void MakeFromFile(const wxString &fileName, const wxString &charset = wxEmptyString);
	// make a response from textual content
	virtual void MakeFromText(const wxString &text, const wxString &mimeType = "text/html");
	// make a response from status code (useful for server errors)
	virtual void MakeFromCode(HttpStatus::HttpStatusCode code = HttpStatus::NotFound);

	// properties

	// status code HTTP for reponse
	inline HttpStatus &Status() { return _status; }

	// HTTP version, default HTTP/1.1
	inline const wxString &Version() const { return _version; }
	inline void SetVersion(const wxString &value) { _version = value; }

	// content of request
	inline wxMemoryBuffer &Content() { return _content; }

	// http headers
	inline HttpHeaders &Headers() { return _headers; }

	inline wxString operator[](int index) const { return _headers[index]; }
	inline wxString operator[](const wxString &key) const { return _headers[key]; }

protected:
	HttpConnection &_connection;
	HttpRequest    &_request;
	HttpStatus      _status;
	wxString        _version;
	wxMemoryBuffer  _content;
	HttpHeaders     _headers;

	friend class HttpConnection;
};

// Server main class
class /* WXDLLIMPEXP_BASE */ HttpServer : public wxEvtHandler
{
public:
	HttpServer();
	virtual ~HttpServer();

	bool Start(const HttpContext &context);
	bool Stop();

	// properties

	inline const HttpContext &Context() const { return _context; }

protected:
	// event handlers (these functions should _not_ be virtual)
	void OnServerEvent(wxSocketEvent &event);
	void OnSocketEvent(wxSocketEvent &event);
    ConnectionMap   _connections;

private:
	wxSocketServer *_server;
	HttpContext     _context;
	IPaddress       _address;

	DECLARE_EVENT_TABLE()

	friend class HttpConnection;
};

// Complete WebSocket message (framing is managed by server)
class /* WXDLLIMPEXP_BASE */ WebSocketMessage
{
public:
	enum Opcode
	{
		Continuation = 0x0, // denotes a continuation frame
		Text = 0x1, // denotes a text frame
		Binary = 0x2, // denotes a binary frame
		// 0x3-7 are reserved for further non-control frames
		Close = 0x8, // denotes a connection close
		Ping = 0x9, // denotes a ping
		Pong = 0xA, // denotes a pong
		// 0xB-0xF are reserved for further control frames
	};

	WebSocketMessage(Opcode type = Text);
	WebSocketMessage(const wxString &text);
	WebSocketMessage(const wxMemoryBuffer &buffer);

	// properties

	// messagge type
	inline Opcode Type() { return _type; }
	// messagge content
	inline const wxMemoryBuffer &Content() const { return _content; }

protected:
	Opcode         _type;
	wxMemoryBuffer _content;

	friend class HttpConnection;
};

#endif // __HTTP_H__

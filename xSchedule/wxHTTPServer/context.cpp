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

#include <wx/filename.h>

extern const char *PAGESTD;
extern const char *PAGE400;
extern const char *PAGE404;

HttpContext::HttpContext() :
	RequestHandler(NULL),
	MessageHandler(NULL)
{
	// default HTTP port
	Port = 80;

	// default directory is cwd
	DefaultDirectory = wxFileName::GetCwd();

	// default documents
	DefaultDocuments.Add("index.htm");
	DefaultDocuments.Add("index.html");
	DefaultDocuments.Add("default.htm");
	DefaultDocuments.Add("default.html");

	ErrorPage400 = PAGE400;
	ErrorPage404 = PAGE404;
}

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

HttpRequest::HttpRequest(HttpConnection &connection) :
	_connection(connection)
{
}

HttpRequest::HttpRequest(HttpConnection &connection, const wxString &input) :
	_connection(connection)
{
	Parse(input);
}

void HttpRequest::Parse(const wxString &input)
{
	wxArrayString rows = wxSplit(input, '\n');
    _data = "";

	if (rows.Count() > 0)
	{
		wxArrayString tokens = wxSplit(rows[0], ' ');

		if (tokens.Count() > 0)
			_method = tokens[0].Trim(true).Trim(false);
		if (tokens.Count() > 1)
			_uri = tokens[1].Trim(true).Trim(false);
		if (tokens.Count() > 2)
			_version = tokens[2].Trim(true).Trim(false);

		wxLogMessage("new request: Method '%s', URI '%s', Version '%s'", _method, _uri, _version);

        int state = 0; // processing headers

		for (size_t i = 1; i < rows.Count(); i++)
		{
            if (state == 0)
            {
                if (rows[i] == "" || rows[i] == "\r") state = 1;

                int index = rows[i].Find(':');

                if (index != -1)
                {
                    wxString key = rows[i].Mid(0, index);
                    key.Trim(true).Trim(false);

                    wxString value = rows[i].Mid(index + 1);
                    value.Trim(true).Trim(false);

                    _headers.Add(key, value);
                }
            }
            else if (state == 1)
            {
                _data += rows[i];

                // dont add a carriage return to the last line
                if (i < rows.Count() - 1) _data += "\n";
            }
		}
	}
}

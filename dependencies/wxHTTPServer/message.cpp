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

WebSocketMessage::WebSocketMessage(Opcode type /*= Text*/) :
	_type(type)
{
}

WebSocketMessage::WebSocketMessage(const wxString &text)
{
	_type = WebSocketMessage::Text;

	wxScopedCharBuffer buffer = text.ToUTF8();
	_content.AppendData(buffer.data(), buffer.length());
}

WebSocketMessage::WebSocketMessage(const wxMemoryBuffer &buffer)
{
	_type = WebSocketMessage::Binary;

	_content.AppendData(buffer.GetData(), buffer.GetDataLen());
}

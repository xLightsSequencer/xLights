/***************************************************************
 * Name:      heartbeat.h
 * Purpose:
    Defines heartbeat API to allow crash monitoring
 *
 * Author:    Don Julien (djulien@github.com)
 * Created:   2014-02-07
 * Copyright: 2014, part of xLights by Matt Brown
 * License:
     This file is part of xLights, created by Matt Brown (dowdybrown@yahoo.com)

    xLights is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xLights is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xLights.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************/
#ifndef HEARTBEAT_H
#define HEARTBEAT_H


void heartbeat(const char* msg, bool always);

#endif // HEARTBEAT_H

/////////////////////////////////////////////////////////////////////////////
// Name:        serial.cpp
// Purpose:
// Author:      Joachim Buermann (adapted for xLights by Matt Brown)
// Copyright:   (c) 2010 Joachim Buermann
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <sstream>
#include <wx/utils.h>

#include "serial.h"

void SerialPort::SetCallback(int cb)
{
    callback = cb;
}

int SerialPort::GetCallback()
{
    return callback;
}

#ifdef __WXMSW__
# include "serial_win32.cpp"
#elif defined __WXOSX__
# include "serial_osx.cpp"
#else
# include "serial_posix.cpp"
#endif

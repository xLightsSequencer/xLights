// Author:      Joachim Buermann (adapted for xLights by Matt Brown)
// Copyright:   (c) 2010 Joachim Buermann

#include "serial.h"

#pragma region Callback 
void SerialPort::SetCallback(int cb)
{
    _callback = cb;
}

int SerialPort::GetCallback()
{
    return _callback;
}
#pragma endregion Callback 

#ifdef __WXMSW__
# include "serial_win32.cpp"
#elif defined __WXOSX__
# include "serial_osx.cpp"
#else
# include "serial_posix.cpp"
#endif

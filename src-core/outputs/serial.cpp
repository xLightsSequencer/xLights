// Author:      Joachim Buermann (adapted for xLights)
// Copyright:   (c) 2010 Joachim Buermann

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "serial.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#include <log.h>

#ifdef _WIN32
# include "serial_win32.cpp"
#elif defined __APPLE__ && !TARGET_OS_IPHONE
# include "serial_osx.cpp"
#elif !defined(__APPLE__) || !TARGET_OS_IPHONE
# include "serial_posix.cpp"
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
// Stub implementations for iOS — serial ports not available
SerialPort::SerialPort() : _fd(-1), _callback(0) {}
SerialPort::~SerialPort() {}
int SerialPort::AvailableToRead() { return 0; }
int SerialPort::WaitingToWrite() { return 0; }
void SerialPort::SetRTS(bool) {}
void SerialPort::SetDTR(bool) {}
int SerialPort::Close() { return 0; }
int SerialPort::Open(const std::string&, int, const char*) { return -1; }
bool SerialPort::IsOpen() { return false; }
int SerialPort::Purge() { return 0; }
int SerialPort::Read(char*, size_t) { return 0; }
int SerialPort::Write(char*, size_t) { return 0; }
int SerialPort::SendBreak() { return 0; }
speed_t SerialPort::AdaptBaudrate(int) { return 0; }
#endif

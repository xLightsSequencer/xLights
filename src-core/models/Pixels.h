#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <vector>
#include <string>

// A controller can host several LED panel matrices at once (FPP allows 5) and they
// need not all be driven the same way - a cape can shift out its own panels while
// ColorLight receivers hang off the network at the same time.  The protocol names
// the driver family so the upload can tell them apart; the port names which matrix.
constexpr const char* PROTOCOL_LED_PANEL_MATRIX = "LED Panel Matrix";
constexpr const char* PROTOCOL_LED_PANEL_MATRIX_CAPE = "LED Panel Matrix - Hat/Cap/Cape";
constexpr const char* PROTOCOL_LED_PANEL_MATRIX_COLORLIGHT = "LED Panel Matrix - ColorLight";

std::string ChooseBestControllerPixel(const std::vector<std::string>& controllerPixels, const std::string& selectedPixel);
std::vector<std::string> GetAllPixelTypes(bool includeSerial = false, bool includeArtificial = true, bool includeMatrices = true);
std::vector<std::string> GetAllPixelTypes(const std::vector<std::string>& controllerPixels, bool includeSerial, bool includeArtificial, bool includeMatrices);
std::vector<std::string> GetAllSerialTypes();
std::vector<std::string> GetAllSerialTypes(const std::vector<std::string>& controllerSerial);
std::string ChooseBestControllerSerial(const std::vector<std::string>& controllerSerial, const std::string& selectedSerial);
bool IsArtificialPixelType(const std::string& p);
bool IsPixelProtocol(const std::string& p);
bool IsSerialProtocol(const std::string& p);

// LOR Enhanced (LOR's 0x8E pixel command, e.g. for Pixies): each LOR unit ID
// takes a block of 510 channels (170 pixels) in its serial port's channels,
// so a model's DMX channel on the port says which unit and start pixel it
// goes to. A model longer than the rest of its unit carries on into the next
// unit ID, as a Pixie port set for 340 pixels does.
constexpr int LOR_ENHANCED_PIXELS_PER_UNIT = 170;
constexpr int LOR_ENHANCED_CHANNELS_PER_UNIT = LOR_ENHANCED_PIXELS_PER_UNIT * 3;
constexpr int LOR_ENHANCED_MAX_UNIT = 240;
constexpr int LOR_ENHANCED_MAX_CHANNELS = LOR_ENHANCED_MAX_UNIT * LOR_ENHANCED_CHANNELS_PER_UNIT;
bool IsLOREnhancedProtocol(const std::string& p);
inline int LOREnhancedUnitFromChannel(int ch) { return ((ch < 1 ? 1 : ch) - 1) / LOR_ENHANCED_CHANNELS_PER_UNIT + 1; }
inline int LOREnhancedPixelFromChannel(int ch) { return (((ch < 1 ? 1 : ch) - 1) % LOR_ENHANCED_CHANNELS_PER_UNIT) / 3 + 1; }
inline int LOREnhancedChannel(int unit, int pixel) { return (unit - 1) * LOR_ENHANCED_CHANNELS_PER_UNIT + (pixel - 1) * 3 + 1; }
bool IsMatrixProtocol(const std::string& p1);
bool IsPWMProtocol(const std::string& p1);
bool IsLEDPanelMatrixProtocol(const std::string& p1);
std::vector<std::string> GetAllLEDPanelMatrixProtocols();
bool IsVirtualMatrixProtocol(const std::string& p1);
int GetChannelsPerPixel(const std::string& p);

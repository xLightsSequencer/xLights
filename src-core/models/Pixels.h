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
bool IsMatrixProtocol(const std::string& p1);
bool IsPWMProtocol(const std::string& p1);
bool IsLEDPanelMatrixProtocol(const std::string& p1);
std::vector<std::string> GetAllLEDPanelMatrixProtocols();
bool IsVirtualMatrixProtocol(const std::string& p1);
int GetChannelsPerPixel(const std::string& p);

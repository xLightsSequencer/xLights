#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <vector>
#include <string>

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
bool IsLEDPanelMatrixProtocol(const std::string& p1);
bool IsVirtualMatrixProtocol(const std::string& p1);

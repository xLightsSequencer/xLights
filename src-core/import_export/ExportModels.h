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

#include <string>

class ModelManager;
class OutputManager;

// Writes the "Models" report workbook (Models / Groups / Controllers /
// Totals sheets) to the given .xlsx path using libxlsxwriter. wx-free
// port of xLightsFrame::ExportModels — usable from both the desktop and
// the iPad bridge. Returns true on success.
bool ExportModels(const std::string& filename, ModelManager& allModels, OutputManager& outputManager);

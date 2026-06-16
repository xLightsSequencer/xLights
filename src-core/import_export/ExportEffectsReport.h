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

class SequenceElements;
class ModelManager;

// Writes the Effects usage report CSV to the given path — wx-free port
// of xLightsFrame::ExportEffects / ExportElement / ExportNodes.
// Format matches the desktop CSV exactly (same header, same columns).
// Returns true on success.
bool ExportEffects(const std::string& filename,
                   SequenceElements& elements,
                   ModelManager& allModels);

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

// Re-export the wx-free SETTINGS enum
#include "../../controllers/ExportSettings.h"

// wx-dependent function to show the export settings dialog
class wxWindow;

namespace ExportSettings {

    SETTINGS GetSettings(wxWindow* parent);

};

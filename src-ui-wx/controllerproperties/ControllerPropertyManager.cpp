
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControllerPropertyManager.h"
#include "ControllerPropertyAdapter.h"
#include "ControllerEthernetPropertyAdapter.h"
#include "ControllerSerialPropertyAdapter.h"
#include "ControllerNullPropertyAdapter.h"
#include "outputs/Controller.h"

std::unique_ptr<ControllerPropertyAdapter> ControllerPropertyManager::CreateAdapter(Controller& controller) {
    auto type = controller.GetType();
    if (type == "Ethernet") {
        return std::make_unique<ControllerEthernetPropertyAdapter>(controller);
    } else if (type == "Serial") {
        return std::make_unique<ControllerSerialPropertyAdapter>(controller);
    } else if (type == "Null") {
        return std::make_unique<ControllerNullPropertyAdapter>(controller);
    }
    return std::make_unique<ControllerPropertyAdapter>(controller);
}

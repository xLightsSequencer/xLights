#include "ControllerRegistry.h"
#include "FPP.h"
#include "Falcon.h"
#include "ESPixelStick.h"
#include "SanDevices.h"
#include "J1Sys.h"
//#include "EasyLights.h"
#include "Pixlite16.h"
#include "AlphaPix.h"

std::map<std::string, const ControllerRules*> ControllerRegistry::controllers;
std::map<std::string, std::string> ControllerRegistry::controllersByDescription;

ControllerRegistry::ControllerRegistry()
{
    //ctor
}

ControllerRegistry::~ControllerRegistry()
{
    //dtor
}

void ControllerRegistry::AddController(const ControllerRules *rules) {
    if (rules && rules->GetControllerId() != "") {
        controllers[rules->GetControllerId()] = rules;
        controllersByDescription[rules->GetControllerDescription()] = rules->GetControllerId();
    }
}

std::vector<std::string> ControllerRegistry::GetControllerIds(bool sortByDescription) {
    if (controllers.empty()) {
        loadControllers();
    }
    std::vector<std::string> keys;
    keys.reserve(controllers.size());
    if (sortByDescription) {
        for (const auto &c : controllersByDescription) {
            keys.push_back(c.second);
        }
    } else {
        for (const auto &c : controllers) {
            keys.push_back(c.first);
        }
    }
    return keys;
}

const ControllerRules *ControllerRegistry::GetRulesForController(const std::string &id) {
    if (controllers.empty()) {
        loadControllers();
    }
    return controllers[id];
}

void ControllerRegistry::loadControllers() {
    Falcon::RegisterControllers();
    FPP::RegisterCapes();
    ESPixelStick::RegisterControllers();
    SanDevices::RegisterControllers();
    J1Sys::RegisterControllers();
    Pixlite16::RegisterControllers();
    //EasyLights::RegisterControllers();
    AlphaPix::RegisterControllers();
}

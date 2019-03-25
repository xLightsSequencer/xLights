
#include "ControllerRegistry.h"
#include "FPP.h"


std::map<std::string, const ControllerRules*> ControllerRegistry::controllers;


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
    }
}

std::vector<std::string> ControllerRegistry::GetControllerIds() {
    if (controllers.empty()) {
        loadControllers();
    }
    std::vector<std::string> keys;
    keys.reserve(controllers.size());
    for (auto &c : controllers) {
        keys.push_back(c.first);
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
    FPP::RegisterCapes();
}

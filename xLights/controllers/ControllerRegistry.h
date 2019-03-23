#ifndef CONTROLLERREGISTRY_H
#define CONTROLLERREGISTRY_H

#include <vector>
#include <map>

#include "ControllerUploadData.h"


class ControllerRegistry
{
    public:
    
        static std::vector<std::string> GetControllerIds();
        static const ControllerRules *GetRulesForController(const std::string &id);
        static void AddController(const ControllerRules *rules);
    
    protected:

    private:
        ControllerRegistry();
        ~ControllerRegistry();
        static void loadControllers();

        static std::map<std::string, const ControllerRules*> controllers;
};

#endif // CONTROLLERREGISTRY_H

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <regex>
#include <cctype>

#include "ObjectManager.h"

ObjectManager::ObjectManager()
{
    //ctor
}

ObjectManager::~ObjectManager()
{
    //dtor
}

// generate the next similar object name to the candidateName we are given
std::string ObjectManager::GenerateObjectName(const std::string& candidateName) const
{
    // if it is already unique return it
    if (GetObject(candidateName) == nullptr) return candidateName;

    std::string base = candidateName;
    char sep = '-';

    static std::regex dashRegex("-[0-9]+$");
    static std::regex underscoreRegex("_[0-9]+$");
    static std::regex spaceRegex(" [0-9]+$");
    static std::regex nilRegex("[A-Za-z][0-9]+$");
    if (std::regex_search(candidateName, dashRegex)) {
        base = candidateName.substr(0, candidateName.rfind('-'));
    } else if (std::regex_search(candidateName, underscoreRegex)) {
        base = candidateName.substr(0, candidateName.rfind('_'));
        sep = '_';
    } else if (std::regex_search(candidateName, spaceRegex)) {
        base = candidateName.substr(0, candidateName.rfind(' '));
        sep = ' ';
    } else if (std::regex_search(candidateName, nilRegex)) {
        while (base != "" && std::isdigit(base[base.size() - 1])) {
            base = base.substr(0, base.size() - 1);
        }
        sep = 'x';
    }

    // We start at 2 assuming if we are adding multiple then the user will typically rename the first one number one.
    int seq = 2;

    for (;;) {
        std::string tryName = base;

        if (sep == 'x') {
            tryName += std::to_string(seq++);
        } else {
            tryName += sep + std::to_string(seq++);
        }

        if (GetObject(tryName) == nullptr) return tryName;
    }
}

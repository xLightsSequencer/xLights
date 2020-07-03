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

#include <string>

class wxXmlNode;
class BaseObject;

class ObjectManager
{
public:
    ObjectManager();
    virtual ~ObjectManager();

    std::string GenerateObjectName(const std::string& candidateName) const;

    virtual BaseObject *GetObject(const std::string &name) const = 0;

protected:
    wxXmlNode *modelNode;
    wxXmlNode *groupNode;

private:

};


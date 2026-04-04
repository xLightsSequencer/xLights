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

#include "BaseSerializingVisitor.h"
#include <pugixml.hpp>
#include <vector>

class XmlSerializingVisitor : public BaseSerializingVisitor {
public:
    explicit XmlSerializingVisitor(pugi::xml_document* doc, bool exporting = false);
    explicit XmlSerializingVisitor(pugi::xml_node parentNode, bool exporting = false);

    void WriteOpenTag(const std::string& name, const AttrCollector& attrs,
                      bool selfClose = false) override;
    void WriteOpenTag(const std::string& name) override {
        AttrCollector attr;
        WriteOpenTag(name, attr);
    }

    void WriteCloseTag() override;
    void WriteBodyText(const std::string& txt) override;

private:
    std::vector<pugi::xml_node> nodeStack;
    pugi::xml_document *doc = nullptr;
};

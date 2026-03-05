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
#include <wx/xml/xml.h>
#include <vector>

class XmlSerializingVisitor : public BaseSerializingVisitor {
public:
    XmlSerializingVisitor(wxXmlDocument* doc, bool exporting = false);
    XmlSerializingVisitor(wxXmlNode* parentNode, bool exporting = false);

    void WriteOpenTag(const std::string& name, const AttrCollector& attrs,
                      bool selfClose = false) override;
    void WriteOpenTag(const std::string& name) override {
        AttrCollector attr;
        WriteOpenTag(name, attr);
    }

    void WriteCloseTag() override;
    void WriteBodyText(const std::string& txt) override;

private:
    std::vector<wxXmlNode*> nodeStack;
    wxXmlDocument *doc = nullptr;
};

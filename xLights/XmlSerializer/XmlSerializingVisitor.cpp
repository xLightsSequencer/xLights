/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "XmlSerializingVisitor.h"

XmlSerializingVisitor::XmlSerializingVisitor(wxXmlNode* parentNode, bool exporting)
    : BaseSerializingVisitor(exporting) {
    nodeStack.push_back(parentNode);
}
XmlSerializingVisitor::XmlSerializingVisitor(wxXmlDocument* parent, bool exporting)
    : BaseSerializingVisitor(exporting), doc(parent) {
}

void XmlSerializingVisitor::WriteOpenTag(const std::string& name,
                                          const AttrCollector& attrs, bool selfClose) {
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, name);
    for (const auto& [key, val] : attrs.attrs) {
        node->AddAttribute(key, val);
    }
    if (nodeStack.empty()) {
        doc->SetRoot(node);
    } else {
        nodeStack.back()->AddChild(node);
    }
    if (!selfClose) {
        nodeStack.push_back(node);
    }
}

void XmlSerializingVisitor::WriteCloseTag(const std::string& /*name*/) {
    wxASSERT(!nodeStack.empty());
    nodeStack.pop_back();
}

void XmlSerializingVisitor::WriteBodyText(const std::string& txt) {
    nodeStack.back()->SetContent(txt);
}

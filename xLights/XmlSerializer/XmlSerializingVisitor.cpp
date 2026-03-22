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

#include <cassert>

XmlSerializingVisitor::XmlSerializingVisitor(pugi::xml_node parentNode, bool exporting)
    : BaseSerializingVisitor(exporting) {
    nodeStack.push_back(parentNode);
}
XmlSerializingVisitor::XmlSerializingVisitor(pugi::xml_document* parent, bool exporting)
    : BaseSerializingVisitor(exporting), doc(parent) {
}

void XmlSerializingVisitor::WriteOpenTag(const std::string& name,
                                          const AttrCollector& attrs, bool selfClose) {
    pugi::xml_node node;
    if (nodeStack.empty()) {
        node = doc->append_child(name);
    } else {
        node = nodeStack.back().append_child(name);
    }
    for (const auto& [key, val] : attrs.attrs) {
        node.append_attribute(key) = val;
    }
    if (!selfClose) {
        nodeStack.push_back(node);
    }
}

void XmlSerializingVisitor::WriteCloseTag() {
    assert(!nodeStack.empty());
    nodeStack.pop_back();
}

void XmlSerializingVisitor::WriteBodyText(const std::string& txt) {
    nodeStack.back().text().set(txt);
}

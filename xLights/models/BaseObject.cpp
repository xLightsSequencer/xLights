#include <wx/xml/xml.h>

#include "BaseObject.h"

BaseObject::BaseObject()
: ModelXml(nullptr), changeCount(0)
{
    //ctor
}

BaseObject::~BaseObject()
{
    //dtor
}

wxXmlNode* BaseObject::GetModelXml() const {
    return this->ModelXml;
}

void BaseObject::SetLayoutGroup(const std::string &grp) {
    layout_group = grp;
    ModelXml->DeleteAttribute("LayoutGroup");
    ModelXml->AddAttribute("LayoutGroup", grp);
    IncrementChangeCount();
}


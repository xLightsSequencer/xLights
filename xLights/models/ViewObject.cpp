#include <wx/xml/xml.h>

#include "ViewObject.h"

ViewObject::ViewObject(const ObjectManager &manger)
: only_3d(true)
{
    //ctor
}

ViewObject::~ViewObject()
{
    //dtor
}

void ViewObject::AddSizeLocationProperties(wxPropertyGridInterface *grid) {
    GetObjectScreenLocation().AddSizeLocationProperties(grid);
}

void ViewObject::SetFromXml(wxXmlNode* ObjectNode) {

    ModelXml=ObjectNode;

    name=ObjectNode->GetAttribute("name").ToStdString();
    DisplayAs=ObjectNode->GetAttribute("DisplayAs").ToStdString();
    layout_group = ObjectNode->GetAttribute("LayoutGroup","Unassigned");

    GetObjectScreenLocation().Read(ObjectNode);

    InitModel();

    IncrementChangeCount();
}

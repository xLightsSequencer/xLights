#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

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

void ViewObject::AddProperties(wxPropertyGridInterface *grid) {

    //LAYOUT_GROUPS = Model::GetLayoutGroups(modelManager);

    wxPGProperty *sp;

    wxPGProperty *p = grid->Append(new wxPropertyCategory(DisplayAs, "ModelType"));

    AddTypeProperties(grid);

    //int layout_group_number = 0;
    //for( int grp=0; grp < LAYOUT_GROUPS.Count(); grp++)
    //{
    //    if( LAYOUT_GROUPS[grp] == layout_group )
    //    {
    //        layout_group_number = grp;
    //        break;
    //    }
    //}

    //grid->Append(new wxStringProperty("Description", "Description", description));
    //grid->Append(new wxEnumProperty("Preview", "ModelLayoutGroup", LAYOUT_GROUPS, wxArrayInt(), layout_group_number));

}

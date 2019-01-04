#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "ViewObject.h"

ViewObject::ViewObject(const ObjectManager &manger)
: only_3d(true), active(true)
{
}

ViewObject::~ViewObject()
{
}

void ViewObject::AddSizeLocationProperties(wxPropertyGridInterface *grid) {
    GetObjectScreenLocation().AddSizeLocationProperties(grid);
}

void ViewObject::SetFromXml(wxXmlNode* ObjectNode, bool zeroBased) {

    ModelXml=ObjectNode;

    name=ObjectNode->GetAttribute("name").ToStdString();
    DisplayAs=ObjectNode->GetAttribute("DisplayAs").ToStdString();
    layout_group = ObjectNode->GetAttribute("LayoutGroup","Unassigned");
    active = ObjectNode->GetAttribute("Active", "1") == "1";

    GetObjectScreenLocation().Read(ObjectNode);

    InitModel();

    IncrementChangeCount();
}

void ViewObject::AddProperties(wxPropertyGridInterface *grid) {

    //LAYOUT_GROUPS = Model::GetLayoutGroups(modelManager);

    wxPGProperty *p = grid->Append(new wxPropertyCategory(DisplayAs, "ModelType"));
    p = grid->Append(new wxBoolProperty("Active", "Active", active));
    p->SetAttribute("UseCheckbox", true);

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

int ViewObject::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {

    if (event.GetPropertyName() == "Active") {
        ModelXml->DeleteAttribute("Active");
        active = event.GetValue().GetBool();
        if (active) {
            ModelXml->AddAttribute("Active", "1");
        }
        IncrementChangeCount();
        return 3 | 0x0008;
    }

    int i = GetObjectScreenLocation().OnPropertyGridChange(grid, event);

    if (i & 0x2) {
        GetObjectScreenLocation().Write(ModelXml);
        SetFromXml(ModelXml);
        IncrementChangeCount();
    }

    return i;
}

void ViewObject::UpdateXmlWithScale() {
    GetObjectScreenLocation().Write(ModelXml);
    if (ModelXml->HasAttribute("versionNumber"))
        ModelXml->DeleteAttribute("versionNumber");
    ModelXml->AddAttribute("versionNumber", "3");
}

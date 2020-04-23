/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "ViewObject.h"
#include "Model.h"

ViewObject::ViewObject(const ObjectManager &manger)
: only_3d(true)
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
    layout_group = "Default"; // objects in 3d can only belong to default as only default is 3d
    _active = ObjectNode->GetAttribute("Active", "1") == "1";

    GetObjectScreenLocation().Read(ObjectNode);

    InitModel();

    IncrementChangeCount();
}

void ViewObject::AddProperties(wxPropertyGridInterface *grid, OutputManager* outputManager) {

    //LAYOUT_GROUPS = Model::GetLayoutGroups(modelManager);

    wxPGProperty *p = grid->Append(new wxPropertyCategory(DisplayAs, "ModelType"));
    p = grid->Append(new wxBoolProperty("Active", "Active", IsActive()));
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
        SetActive(event.GetValue().GetBool());
        return 0;
    }

    int i = GetObjectScreenLocation().OnPropertyGridChange(grid, event);
    GetObjectScreenLocation().Write(ModelXml);
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ViewObject::OnPropertyGridChange");
    
    return i;
}

void ViewObject::UpdateXmlWithScale() {
    GetObjectScreenLocation().Write(ModelXml);
    if (ModelXml->HasAttribute("versionNumber"))
        ModelXml->DeleteAttribute("versionNumber");
    ModelXml->AddAttribute("versionNumber", CUR_MODEL_POS_VER);
}

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
#include <wx/sstream.h>

#include "MhFeature.h"
#include "MhChannel.h"
#include "../BaseObject.h"
#include <glm/gtc/type_ptr.hpp>

MhFeature::MhFeature(wxXmlNode* node, wxString _name)
    : SerializedObject("MhFeature_" + _name), node_xml(node), base_name(_name), xml_name("MhFeature_" + _name)
{
}

MhFeature::~MhFeature()
{
}

void MhFeature::Init(BaseObject* base) {
    this->base = base;
}

void MhFeature::AddTypeProperties(wxPropertyGridInterface *grid) {
    grid->Append(new wxPropertyCategory(base_name, base_name + "Properties"));

    for (auto it = channels.begin(); it != channels.end(); ++it) {
        (*it)->AddTypeProperties(grid);
    }

    grid->Collapse(base_name + "Properties");
}

int MhFeature::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked) {

    int return_val = -1;
    for (auto it = channels.begin(); it != channels.end(); ++it) {
        int result = (*it)->OnPropertyGridChange(grid, event, base, locked);
        if( result >= 0 ) {
            return_val = result;
            break;
        }
    }
    return return_val;
}

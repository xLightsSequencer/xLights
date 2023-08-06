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
#include "../../BaseObject.h"
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

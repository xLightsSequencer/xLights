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

#include <wx/xml/xml.h>

#include "BaseObject.h"
#include "BaseObjectVisitor.h"
#include "DMX/DmxMovingHeadAdv.h"

namespace XmlNodeKeys
{
    constexpr auto ModelsNodeName = "models";

    constexpr auto DisplayAsAttribute = "DisplayAs";
    constexpr auto StartSideAttribute = "StartSide";
    constexpr auto DirAttribute       = "Dir";

    constexpr auto DmxMovingHeadAdvNodeName = "DmxMovingHeadAdv";

    //constexpr auto DocumentNodeName = "PaintDocument";
    //constexpr auto VersionAttribute = "version";
    //constexpr auto VersionValue = "1.2";
};

struct XmlSerializingVisitor : BaseObjectVisitor
{
    XmlSerializingVisitor(wxXmlNode *parentNode) : parentNode(parentNode)
    {
    }

    wxXmlNode *parentNode;

    void AddCommonModelAttributes(const Model &model, wxXmlNode *node)
    {
        node->AddAttribute(XmlNodeKeys::DisplayAsAttribute, XmlNodeKeys::DmxMovingHeadAdvNodeName);
        node->AddAttribute(XmlNodeKeys::StartSideAttribute, model.GetStartSide());
        node->AddAttribute(XmlNodeKeys::DirAttribute, model.GetStartSide());
    }

     void Visit(const DmxMovingHeadAdv &moving_head) override
    {
        wxXmlNode *mhNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::DmxMovingHeadAdvNodeName);
        AddCommonModelAttributes(moving_head, mhNode);
        parentNode->AddChild(mhNode);
    }
};

struct XmlSerializer
{
    XmlSerializer()
    {
    }

    void SerializeAndSaveModel(const BaseObject &object)
    {
        wxString name = object.GetModelXml()->GetAttribute("name");
        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (filename.IsEmpty())
            return;
        wxXmlDocument doc = SerializeModel(object);
        doc.Save(filename);
    }

    wxXmlDocument SerializeModel(const BaseObject &object)
    {
        wxXmlDocument doc;
        
        wxXmlNode *docNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelsNodeName);
        
        XmlSerializingVisitor visitor{docNode};
        
        object.Accept(visitor);
        
        doc.SetRoot(docNode);
        
        return doc;
    }
};

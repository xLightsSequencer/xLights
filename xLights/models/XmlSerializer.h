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

    // Common BaseObject Attributes
    constexpr auto NameAttribute        = "name";
    constexpr auto DisplayAsAttribute   = "DisplayAs";
    constexpr auto LayoutGroupAttribute = "LayoutGroup";

    // Common Model Attributes
    constexpr auto StartSideAttribute     = "StartSide";
    constexpr auto DirAttribute           = "Dir";
    constexpr auto Parm1Attribute         = "parm1";
    constexpr auto Parm2Attribute         = "parm2";
    constexpr auto Parm3Attribute         = "parm3";
    constexpr auto AntialiasAttribute     = "Antialias";
    constexpr auto PixelSizeAttribute     = "PixelSize";
    constexpr auto StringTypeAttribute    = "StringType";
    constexpr auto TransparencyAttribute  = "Transparency";
    constexpr auto StartChannelAttribute  = "StartChannel";
    constexpr auto versionNumberAttribute = "versionNumber";

    constexpr auto DmxMovingHeadAdvNodeName = "DmxMovingHeadAdv";
};

struct XmlSerializingVisitor : BaseObjectVisitor
{
    XmlSerializingVisitor(wxXmlNode *parentNode) : parentNode(parentNode)
    {
    }

    wxXmlNode *parentNode;

    void AddBaseObjectAttributes(const BaseObject &base, wxXmlNode *node)
    {
        node->AddAttribute(XmlNodeKeys::NameAttribute, base.GetName());
        node->AddAttribute(XmlNodeKeys::DisplayAsAttribute, base.GetDisplayAs());
        node->AddAttribute(XmlNodeKeys::LayoutGroupAttribute, base.GetLayoutGroup());
    }

    void AddCommonModelAttributes(const Model &model, wxXmlNode *node)
    {
        node->AddAttribute(XmlNodeKeys::StartSideAttribute, model.GetStartSide());
        node->AddAttribute(XmlNodeKeys::DirAttribute, model.GetDirection());
        node->AddAttribute(XmlNodeKeys::Parm1Attribute, std::to_string(model.GetParm1()));
        node->AddAttribute(XmlNodeKeys::Parm2Attribute, std::to_string(model.GetParm2()));
        node->AddAttribute(XmlNodeKeys::Parm3Attribute, std::to_string(model.GetParm3()));
        node->AddAttribute(XmlNodeKeys::AntialiasAttribute, std::to_string((long)model.GetPixelStyle()));
        node->AddAttribute(XmlNodeKeys::PixelSizeAttribute, std::to_string(model.GetPixelSize()));
        node->AddAttribute(XmlNodeKeys::StringTypeAttribute, model.GetStringType());
        node->AddAttribute(XmlNodeKeys::TransparencyAttribute, std::to_string(model.GetTransparency()));
        node->AddAttribute(XmlNodeKeys::StartChannelAttribute, model.GetModelStartChannel());
        node->AddAttribute(XmlNodeKeys::versionNumberAttribute, CUR_MODEL_POS_VER);
    }

    void Visit(const DmxMovingHeadAdv &moving_head) override
    {
        wxXmlNode *mhNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::DmxMovingHeadAdvNodeName);
        AddBaseObjectAttributes(moving_head, mhNode);
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

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
#include "DMX/DmxColorAbilityCMY.h"
#include "DMX/DmxColorAbilityRGB.h"
#include "DMX/DmxColorAbilityWheel.h"
#include "DMX/DmxMovingHeadAdv.h"
#include "DMX/Mesh.h"


namespace XmlNodeKeys
{
// Model Node Names
constexpr auto ModelsNodeName = "models";
constexpr auto ModelNodeName = "model";
constexpr auto TypeAttribute = "type";
constexpr auto ExportedAttribute = "exported";

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
constexpr auto NodeNamesAttribute     = "NodeNames";
constexpr auto StrandNamesAttribute   = "StrandNames";
constexpr auto ControllerAttribute   = "Controller";
constexpr auto versionNumberAttribute = "versionNumber";

// Size/Position Attributes
constexpr auto WorldPosXAttribute = "WorldPosX";
constexpr auto WorldPosYAttribute = "WorldPosY";
constexpr auto WorldPosZAttribute = "WorldPosZ";
constexpr auto ScaleXAttribute    = "ScaleX";
constexpr auto ScaleYAttribute    = "ScaleY";
constexpr auto ScaleZAttribute    = "ScaleZ";
constexpr auto RotateXAttribute   = "RotateX";
constexpr auto RotateYAttribute   = "RotateY";
constexpr auto RotateZAttribute   = "RotateZ";
constexpr auto WidthAttribute     = "Width";
constexpr auto HeightAttribute    = "Height";
constexpr auto DepthAttribute     = "Depth";
constexpr auto OffsetXAttribute   = "OffsetX";
constexpr auto OffsetYAttribute   = "OffsetY";
constexpr auto OffsetZAttribute   = "OffsetZ";

// DmxColorAbilityRGB Attributes
constexpr auto DmxRedChannelAttribute   = "DmxRedChannel";
constexpr auto DmxGreenChannelAttribute = "DmxGreenChannel";
constexpr auto DmxBlueChannelAttribute  = "DmxBlueChannel";
constexpr auto DmxWhiteChannelAttribute = "DmxWhiteChannel";

// DmxColorAbilityWheel Attributes
constexpr auto DmxColorWheelChannelAttribute = "DmxColorWheelChannel";
constexpr auto DmxDimmerChannelAttribute     = "DmxDimmerChannel";
constexpr auto DmxColorWheelColorAttribute   = "DmxColorWheelColor";
constexpr auto DmxColorWheelDMXAttribute     = "DmxColorWheelDMX";

// DmxColorAbilityCMY Attributes
constexpr auto DmxCyanChannelAttribute    = "DmxCyanChannel";
constexpr auto DmxMagentaChannelAttribute = "DmxMagentaChannel";
constexpr auto DmxYellowChannelAttribute  = "DmxYellowChannel";

// DmxMovingHeadComm Attributes
constexpr auto DmxFixturelAttribute = "DmxFixture";

// DMX Moving Head Attributes
constexpr auto DmxColorTypeAttribute = "DmxColorType";
constexpr auto DmxBeamYOffsetAttribute = "DmxBeamYOffset";
constexpr auto DmxBeamLengthAttribute = "DmxBeamLength";

// DmxMotor Attributes
constexpr auto ChannelCoarseAttribute = "ChannelCoarse";
constexpr auto ChannelFineAttribute   = "ChannelFine";
constexpr auto MinLimitAttribute      = "MinLimit";
constexpr auto MaxLimitAttribute      = "MaxLimit";
constexpr auto RangeOfMotionAttribute = "RangeOfMotion";
constexpr auto OrientZeroAttribute    = "OrientZero";
constexpr auto OrientHomeAttribute    = "OrientHome";
constexpr auto SlewLimitAttribute     = "SlewLimit";
constexpr auto ReverseAttribute       = "Reverse";
constexpr auto UpsideDownAttribute    = "UpsideDown";

// Mesh Attributes
constexpr auto ObjFileAttribute    = "ObjFile";
constexpr auto MeshOnlyAttribute   = "MeshOnly";
constexpr auto BrightnessAttribute = "Brightness";

// Model Types
constexpr auto DmxMovingHeadAdvType    = "DmxMovingHeadAdv";

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
        node->AddAttribute(XmlNodeKeys::NodeNamesAttribute, model.GetNodeNames());
        node->AddAttribute(XmlNodeKeys::StrandNamesAttribute, model.GetStrandNames());
        node->AddAttribute(XmlNodeKeys::ControllerAttribute, model.GetControllerName());
        node->AddAttribute(XmlNodeKeys::versionNumberAttribute, CUR_MODEL_POS_VER);
    }

    void AddModelScreenLocationAttributes(const BaseObject &base, wxXmlNode *node)
    {
        glm::vec3 loc = base.GetBaseObjectScreenLocation().GetWorldPosition();
        node->AddAttribute(XmlNodeKeys::WorldPosXAttribute, std::to_string(loc.x));
        node->AddAttribute(XmlNodeKeys::WorldPosYAttribute, std::to_string(loc.y));
        node->AddAttribute(XmlNodeKeys::WorldPosZAttribute, std::to_string(loc.z));
        glm::vec3 scale = base.GetBaseObjectScreenLocation().GetScaleMatrix();
        node->AddAttribute(XmlNodeKeys::ScaleXAttribute, std::to_string(scale.x));
        node->AddAttribute(XmlNodeKeys::ScaleYAttribute, std::to_string(scale.y));
        node->AddAttribute(XmlNodeKeys::ScaleZAttribute, std::to_string(scale.z));
        glm::vec3 rotate = base.GetBaseObjectScreenLocation().GetRotation();
        node->AddAttribute(XmlNodeKeys::RotateXAttribute, std::to_string(rotate.x));
        node->AddAttribute(XmlNodeKeys::RotateYAttribute, std::to_string(rotate.y));
        node->AddAttribute(XmlNodeKeys::RotateZAttribute, std::to_string(rotate.z));
    }

    void AddColorAbilityRGBAttributes(const DmxColorAbilityRGB *colors, wxXmlNode *node)
    {
        node->AddAttribute(XmlNodeKeys::DmxRedChannelAttribute, std::to_string(colors->GetRedChannel()));
        node->AddAttribute(XmlNodeKeys::DmxGreenChannelAttribute, std::to_string(colors->GetGreenChannel()));
        node->AddAttribute(XmlNodeKeys::DmxBlueChannelAttribute, std::to_string(colors->GetBlueChannel()));
        node->AddAttribute(XmlNodeKeys::DmxWhiteChannelAttribute, std::to_string(colors->GetWhiteChannel()));
    }

    void AddColorWheelAttributes(const DmxColorAbilityWheel *colors, wxXmlNode *node)
    {
        node->AddAttribute(XmlNodeKeys::DmxColorWheelChannelAttribute, std::to_string(colors->GetWheelChannel()));
        node->AddAttribute(XmlNodeKeys::DmxDimmerChannelAttribute, std::to_string(colors->GetDimmerChannel()));
        std::vector<WheelColor> settings = colors->GetWheelColorSettings();
        int index = 0;
        for (const auto& it : settings)
        {
            node->AddAttribute(XmlNodeKeys::DmxColorWheelColorAttribute + std::to_string(index), (std::string)it.color);
            node->AddAttribute(XmlNodeKeys::DmxColorWheelDMXAttribute + std::to_string(index), std::to_string(it.dmxValue));
            ++index;
        }
    }

    void AddColorAbilityCMYAttributes(const DmxColorAbilityCMY *colors, wxXmlNode *node)
    {
        node->AddAttribute(XmlNodeKeys::DmxCyanChannelAttribute, std::to_string(colors->GetCyanChannel()));
        node->AddAttribute(XmlNodeKeys::DmxMagentaChannelAttribute, std::to_string(colors->GetMagentaChannel()));
        node->AddAttribute(XmlNodeKeys::DmxYellowChannelAttribute, std::to_string(colors->GetYellowChannel()));
        node->AddAttribute(XmlNodeKeys::DmxWhiteChannelAttribute, std::to_string(colors->GetWhiteChannel()));
    }

    void AddColorAttributes(const DmxModel &dmx_model, wxXmlNode *node) {
        if (dmx_model.HasColorAbility()) {
            DmxColorAbility* color_ability = dmx_model.GetColorAbility();
            std::string color_type = color_ability->GetTypeName();
            node->AddAttribute(XmlNodeKeys::DmxColorTypeAttribute, std::to_string(dmx_model.DmxColorTypetoID(color_type)));
            if (color_type == "RGBW" ) {
                AddColorAbilityRGBAttributes( reinterpret_cast<DmxColorAbilityRGB*>(color_ability), node);
            } else if (color_type == "ColorWheel" ) {
                AddColorWheelAttributes( reinterpret_cast<DmxColorAbilityWheel*>(color_ability), node);
            } else if (color_type == "CMYW" ) {
                AddColorAbilityCMYAttributes( reinterpret_cast<DmxColorAbilityCMY*>(color_ability), node);
            }
        }
    }

    void AddDmxMotorAttributes(const DmxMotor *motor, wxXmlNode *node) {
        wxXmlNode *motor_node = new wxXmlNode(wxXML_ELEMENT_NODE, motor->GetName());
        motor_node->AddAttribute(XmlNodeKeys::ChannelCoarseAttribute, std::to_string(motor->GetChannelCoarse()));
        motor_node->AddAttribute(XmlNodeKeys::ChannelFineAttribute, std::to_string(motor->GetChannelFine()));
        motor_node->AddAttribute(XmlNodeKeys::MinLimitAttribute, std::to_string(motor->GetMinLimit()));
        motor_node->AddAttribute(XmlNodeKeys::MaxLimitAttribute, std::to_string(motor->GetMaxLimit()));
        motor_node->AddAttribute(XmlNodeKeys::RangeOfMotionAttribute, std::to_string(motor->GetRangeOfMotion()));
        motor_node->AddAttribute(XmlNodeKeys::OrientZeroAttribute, std::to_string(motor->GetOrientZero()));
        motor_node->AddAttribute(XmlNodeKeys::OrientHomeAttribute, std::to_string(motor->GetOrientHome()));
        motor_node->AddAttribute(XmlNodeKeys::SlewLimitAttribute, std::to_string(motor->GetSlewLimit()));
        motor_node->AddAttribute(XmlNodeKeys::ReverseAttribute, std::to_string(motor->GetReverse()));
        motor_node->AddAttribute(XmlNodeKeys::UpsideDownAttribute, std::to_string(motor->GetUpsideDown()));
        node->AddChild(motor_node);
    }

    void AddMeshAttributes(const Mesh *mesh, wxXmlNode *node) {
        wxXmlNode *mesh_node = new wxXmlNode(wxXML_ELEMENT_NODE, mesh->GetName());
        mesh_node->AddAttribute(XmlNodeKeys::ObjFileAttribute, mesh->GetObjFile());
        mesh_node->AddAttribute(XmlNodeKeys::MeshOnlyAttribute, std::to_string(mesh->GetMeshOnly()));
        mesh_node->AddAttribute(XmlNodeKeys::BrightnessAttribute, std::to_string(mesh->GetBrightness()));
        mesh_node->AddAttribute(XmlNodeKeys::WidthAttribute, std::to_string(mesh->GetWidth()));
        mesh_node->AddAttribute(XmlNodeKeys::HeightAttribute, std::to_string(mesh->GetHeight()));
        mesh_node->AddAttribute(XmlNodeKeys::DepthAttribute, std::to_string(mesh->GetDepth()));
        mesh_node->AddAttribute(XmlNodeKeys::ScaleXAttribute, std::to_string(mesh->GetScaleX()));
        mesh_node->AddAttribute(XmlNodeKeys::ScaleYAttribute, std::to_string(mesh->GetScaleY()));
        mesh_node->AddAttribute(XmlNodeKeys::ScaleZAttribute, std::to_string(mesh->GetScaleZ()));
        mesh_node->AddAttribute(XmlNodeKeys::RotateXAttribute, std::to_string(mesh->GetRotateX()));
        mesh_node->AddAttribute(XmlNodeKeys::RotateYAttribute, std::to_string(mesh->GetRotateY()));
        mesh_node->AddAttribute(XmlNodeKeys::RotateZAttribute, std::to_string(mesh->GetRotateZ()));
        mesh_node->AddAttribute(XmlNodeKeys::OffsetXAttribute, std::to_string(mesh->GetOffsetX()));
        mesh_node->AddAttribute(XmlNodeKeys::OffsetYAttribute, std::to_string(mesh->GetOffsetY()));
        mesh_node->AddAttribute(XmlNodeKeys::OffsetZAttribute, std::to_string(mesh->GetOffsetZ()));
        node->AddChild(mesh_node);
    }

    void Visit(const DmxMovingHeadAdv &moving_head) override
    {
        wxXmlNode *mhNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(moving_head, mhNode);
        AddCommonModelAttributes(moving_head, mhNode);
        AddModelScreenLocationAttributes(moving_head, mhNode);
        AddColorAttributes(moving_head, mhNode);
        mhNode->AddAttribute(XmlNodeKeys::DmxFixturelAttribute, moving_head.GetFixture());
        mhNode->AddAttribute(XmlNodeKeys::DmxBeamYOffsetAttribute, std::to_string(moving_head.GetBeamYOffset()));
        mhNode->AddAttribute(XmlNodeKeys::DmxBeamLengthAttribute, std::to_string(moving_head.GetBeamLength()));
        AddDmxMotorAttributes(reinterpret_cast<DmxMotor*>(moving_head.GetPanMotor()), mhNode);
        AddDmxMotorAttributes(reinterpret_cast<DmxMotor*>(moving_head.GetTiltMotor()), mhNode);
        AddMeshAttributes(reinterpret_cast<Mesh*>(moving_head.GetBaseMesh()), mhNode);
        AddMeshAttributes(reinterpret_cast<Mesh*>(moving_head.GetYokeMesh()), mhNode);
        AddMeshAttributes(reinterpret_cast<Mesh*>(moving_head.GetHeadMesh()), mhNode);
        parentNode->AddChild(mhNode);
    }
};

struct XmlDeserializingObjectFactory
{
    Model* Deserialize(wxXmlNode *node, const ModelManager &manager)
    {
        auto type = node->GetAttribute(XmlNodeKeys::DisplayAsAttribute);

        if (type == XmlNodeKeys::DmxMovingHeadAdvType)
        {
            return DeserializeDmxMovingHeadAdv(new wxXmlNode(*node), manager);
        }

        throw std::runtime_error("Unknown object type: " + type);
    }

private:
    //void RetrieveBaseObjectAttributes(BaseObject &base, wxXmlNode *node)
    //{
    //    base.name = node->GetAttribute(XmlNodeKeys::NameAttribute);
    //    base.SetDisplayAs(node->GetAttribute(XmlNodeKeys::DisplayAsAttribute));
    //    node->AddAttribute(XmlNodeKeys::LayoutGroupAttribute, base.GetLayoutGroup());
    //}

    Model* DeserializeDmxMovingHeadAdv(wxXmlNode *node, const ModelManager &manager)
    {
        Model *model;
        model = new DmxMovingHeadAdv(node, manager, false);

        //RetrieveBaseObjectAttributes(object, node);

        return model;
    }
};

struct XmlSerializer
{
    XmlSerializer()
    {
    }

    // Serializes and Saves a single model into an XML document
    void SerializeAndSaveModel(const BaseObject &object)
    {
        wxString name = object.GetModelXml()->GetAttribute("name");
        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (filename.IsEmpty())
            return;
        wxXmlDocument doc = SerializeModel(object);
        doc.Save(filename);
    }

    // Serialize a single model into an XML document
    wxXmlDocument SerializeModel(const BaseObject &object)
    {
        wxXmlDocument doc;
        
        wxXmlNode *docNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelsNodeName);
        docNode->AddAttribute(XmlNodeKeys::TypeAttribute, XmlNodeKeys::ExportedAttribute);

        XmlSerializingVisitor visitor{docNode};
        
        object.Accept(visitor);
        
        doc.SetRoot(docNode);
        
        return doc;
    }

    // Deserialize a single model from an XML document
    Model* DeserializeModel(const wxXmlDocument &doc, const ModelManager &manager)
    {
        wxXmlNode *root = doc.GetRoot();

        wxXmlNode *model_node = root->GetChildren();

        XmlDeserializingObjectFactory factory{};

        return factory.Deserialize(model_node, manager);
    }

};

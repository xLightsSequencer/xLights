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

#include "ArchesModel.h"
#include "BaseObject.h"
#include "BaseObjectVisitor.h"
#include "DMX/DmxColorAbilityCMY.h"
#include "DMX/DmxColorAbilityRGB.h"
#include "DMX/DmxColorAbilityWheel.h"
#include "DMX/DmxPresetAbility.h"
#include "DMX/DmxDimmerAbility.h"
#include "DMX/DmxShutterAbility.h"
#include "DMX/DmxMovingHeadAdv.h"
#include "DMX/DmxMovingHead.h"
#include "DMX/Mesh.h"
#include "ThreePointScreenLocation.h"

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

// Model Screen Location Attributes
constexpr auto LockedAttribute    = "Locked";

// TwoPoint Screen Location Attributes
constexpr auto X2Attribute    = "X2";
constexpr auto Y2Attribute    = "Y2";
constexpr auto Z2Attribute    = "Z2";

// ThreePoint Screen Location Attributes
constexpr auto AngleAttribute    = "Angle";
constexpr auto ShearAttribute    = "Shear";

// DmxColorAbilityRGB Attributes
constexpr auto DmxRedChannelAttribute   = "DmxRedChannel";
constexpr auto DmxGreenChannelAttribute = "DmxGreenChannel";
constexpr auto DmxBlueChannelAttribute  = "DmxBlueChannel";
constexpr auto DmxWhiteChannelAttribute = "DmxWhiteChannel";

// DmxColorAbilityWheel Attributes
constexpr auto DmxColorWheelChannelAttribute = "DmxColorWheelChannel";
constexpr auto DmxDimmerChannelAttribute     = "DmxDimmerChannel";
constexpr auto DmxColorWheelDelayAttribute   = "DmxColorWheelDelay";
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
constexpr auto DmxBeamWidthAttribute = "DmxBeamWidth";//old MH model
constexpr auto DmxStyleAttribute = "DmxStyle";        // old MH model
constexpr auto HideBodyAttribute = "HideBody";        // old MH model

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

// DmxPresetAbility Attributes
constexpr auto DmxPresetChannelAttribute = "DmxPresetChannel";
constexpr auto DmxPresetValueAttribute = "DmxPresetValue";
constexpr auto DmxPresetDescAttribute = "DmxPresetDesc";

// DmxShutterAbility Attributes
constexpr auto DmxShutterChannelAttribute = "DmxShutterChannel";
constexpr auto DmxShutterOpenAttribute = "DmxShutterOpen";
constexpr auto DmxShutterOnValueAttribute = "DmxShutterOnValue";

// DmxDimmerAbility Attributes
constexpr auto MhDimmerChannelAttribute = "MhDimmerChannel";

// Arch Attributes
constexpr auto ZigZagAttribute = "ZigZag";
constexpr auto HollowAttribute = "Hollow";
constexpr auto GapAttribute    = "Gap";

// Mesh Attributes
constexpr auto ObjFileAttribute    = "ObjFile";
constexpr auto MeshOnlyAttribute   = "MeshOnly";
constexpr auto BrightnessAttribute = "Brightness";

// Model Types
constexpr auto ArchesType           = "Arches";
constexpr auto DmxMovingHeadAdvType = "DmxMovingHeadAdv";
constexpr auto DmxMovingHeadType = "DmxMovingHead";

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
        bool locked = base.GetBaseObjectScreenLocation().IsLocked();
        node->AddAttribute(XmlNodeKeys::LockedAttribute, std::to_string(locked));
    }

    void AddThreePointScreenLocationAttributes(const BaseObject &base, wxXmlNode *node)
    {
        const ThreePointScreenLocation& screenLoc = dynamic_cast<const ThreePointScreenLocation&>(base.GetBaseObjectScreenLocation());
        float x2 = screenLoc.GetX2();
        float y2 = screenLoc.GetY2();
        float z2 = screenLoc.GetZ2();
        node->AddAttribute(XmlNodeKeys::X2Attribute, std::to_string(x2));
        node->AddAttribute(XmlNodeKeys::Y2Attribute, std::to_string(y2));
        node->AddAttribute(XmlNodeKeys::Z2Attribute, std::to_string(z2));
        int angle = screenLoc.GetAngle();
        node->AddAttribute(XmlNodeKeys::AngleAttribute, std::to_string(angle));
        float shear = screenLoc.GetYShear();
        node->AddAttribute(XmlNodeKeys::ShearAttribute, std::to_string(shear));
        node->AddAttribute(XmlNodeKeys::HeightAttribute, std::to_string(base.GetHeight()));
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
        node->AddAttribute(XmlNodeKeys::DmxColorWheelDelayAttribute, std::to_string(colors->GetWheelDelay()));
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

    void AddPresetAttributes(const DmxPresetAbility* preset_channels, wxXmlNode* node) {
        if (!preset_channels) {
            return;
        }
        auto const& settings = preset_channels->GetPresetSettings();
        int index { 0 };
        for (auto const& it : settings) {
            node->AddAttribute(XmlNodeKeys::DmxPresetChannelAttribute + std::to_string(index), std::to_string(it.DMXChannel));
            node->AddAttribute(XmlNodeKeys::DmxPresetValueAttribute + std::to_string(index), std::to_string(it.DMXValue));
            node->AddAttribute(XmlNodeKeys::DmxPresetDescAttribute + std::to_string(index), it.Description);
            ++index;
        }
    }

    void AddShutterAbilityAttributes(const DmxShutterAbility shutter, wxXmlNode* node) {
        node->AddAttribute(XmlNodeKeys::DmxShutterChannelAttribute, std::to_string(shutter.GetShutterChannel()));
        node->AddAttribute(XmlNodeKeys::DmxShutterOpenAttribute, std::to_string(shutter.GetShutterThreshold()));
        node->AddAttribute(XmlNodeKeys::DmxShutterOnValueAttribute, std::to_string(shutter.GetShutterOnValue()));
    }

    void AddDimmerAbilityAttributes(const DmxDimmerAbility dimmer, wxXmlNode* node) {
        node->AddAttribute(XmlNodeKeys::MhDimmerChannelAttribute, std::to_string(dimmer.GetDimmerChannel()));
    }

    void Visit(const ArchesModel &arch) override
    {
        wxXmlNode *archNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(arch, archNode);
        AddCommonModelAttributes(arch, archNode);
        AddModelScreenLocationAttributes(arch, archNode);
        AddThreePointScreenLocationAttributes(arch, archNode);
        archNode->AddAttribute(XmlNodeKeys::ZigZagAttribute, std::to_string(arch.GetZigZag()));
        archNode->AddAttribute(XmlNodeKeys::HollowAttribute, std::to_string(arch.GetHollow()));
        archNode->AddAttribute(XmlNodeKeys::GapAttribute, std::to_string(arch.GetGap()));
        parentNode->AddChild(archNode);
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
        AddDimmerAbilityAttributes(moving_head, mhNode);
        AddShutterAbilityAttributes(moving_head, mhNode);
        AddPresetAttributes(moving_head.GetPresetAbility(), mhNode);
        parentNode->AddChild(mhNode);
    }

    void Visit(const DmxMovingHead& moving_head) override {
        wxXmlNode* mhNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(moving_head, mhNode);
        AddCommonModelAttributes(moving_head, mhNode);
        AddModelScreenLocationAttributes(moving_head, mhNode);
        AddColorAttributes(moving_head, mhNode);
        mhNode->AddAttribute(XmlNodeKeys::DmxFixturelAttribute, moving_head.GetFixture());
        mhNode->AddAttribute(XmlNodeKeys::DmxStyleAttribute, moving_head.GetDMXStyle());
        mhNode->AddAttribute(XmlNodeKeys::DmxBeamLengthAttribute, std::to_string(moving_head.GetBeamLength()));
        mhNode->AddAttribute(XmlNodeKeys::DmxBeamWidthAttribute, std::to_string(moving_head.GetBeamWidth()));
        mhNode->AddAttribute(XmlNodeKeys::HideBodyAttribute, std::to_string(moving_head.GetHideBody()));
        AddDmxMotorAttributes(reinterpret_cast<DmxMotor*>(moving_head.GetPanMotor()), mhNode);
        AddDmxMotorAttributes(reinterpret_cast<DmxMotor*>(moving_head.GetTiltMotor()), mhNode);
        AddDimmerAbilityAttributes(moving_head, mhNode);
        AddShutterAbilityAttributes(moving_head, mhNode);
        AddPresetAttributes(moving_head.GetPresetAbility(), mhNode);
        parentNode->AddChild(mhNode);
    }
};

struct XmlDeserializingObjectFactory
{
    Model* Deserialize(wxXmlNode *node, xLightsFrame* xlights)
    {
        auto type = node->GetAttribute(XmlNodeKeys::DisplayAsAttribute);

        if (type == XmlNodeKeys::ArchesType) {
            return DeserializeArches(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::DmxMovingHeadAdvType) {
            return DeserializeDmxMovingHeadAdv(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::DmxMovingHeadType) {
            return DeserializeDmxMovingHead(new wxXmlNode(*node), xlights);
        }

        throw std::runtime_error("Unknown object type: " + type);
    }

private:

    Model* DeserializeArches(wxXmlNode *node, xLightsFrame* xlights)
    {
        Model *model;
        model = new ArchesModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeDmxMovingHeadAdv(wxXmlNode *node, xLightsFrame* xlights)
    {
        Model *model = new DmxMovingHeadAdv(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        // TODO: I'd like to get rid of this whole ImportModelChildren call but left it in the flow for now
        float min_x = (float)(model->GetBaseObjectScreenLocation().GetLeft());
        float max_x = (float)(model->GetBaseObjectScreenLocation().GetRight());
        float min_y = (float)(model->GetBaseObjectScreenLocation().GetBottom());
        float max_y = (float)(model->GetBaseObjectScreenLocation().GetTop());
        model->ImportModelChildren(node, xlights, newname, min_x, max_x, min_y, max_y);

        return model;
    }

    Model* DeserializeDmxMovingHead(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model = new DmxMovingHead(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        // TODO: I'd like to get rid of this whole ImportModelChildren call but left it in the flow for now
        float min_x = (float)(model->GetBaseObjectScreenLocation().GetLeft());
        float max_x = (float)(model->GetBaseObjectScreenLocation().GetRight());
        float min_y = (float)(model->GetBaseObjectScreenLocation().GetBottom());
        float max_y = (float)(model->GetBaseObjectScreenLocation().GetTop());
        model->ImportModelChildren(node, xlights, newname, min_x, max_x, min_y, max_y);

        return model;
    }
};

struct XmlSerializer
{
    XmlSerializer()
    {
    }

    static bool IsXmlSerializerFormat(const wxXmlNode *node)
    {
        if (node->GetAttribute(XmlNodeKeys::TypeAttribute, "") == XmlNodeKeys::ExportedAttribute) {
            return true;
        }
        return false;
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
    Model* DeserializeModel(const wxXmlDocument &doc, xLightsFrame* xlights)
    {
        wxXmlNode *root = doc.GetRoot();
        wxXmlNode *model_node = root->GetChildren();
        return DeserializeModel(model_node, xlights);
    }

    // Deserialize a single model XML node
    Model* DeserializeModel(wxXmlNode *model_node, xLightsFrame* xlights)
    {
        XmlDeserializingObjectFactory factory{};
        Model* model = factory.Deserialize(model_node, xlights);

        // TODO: I'd like to get rid of this whole ImportModelChildren call but left it in the flow for now
        float min_x = (float)(model->GetBaseObjectScreenLocation().GetLeft());
        float max_x = (float)(model->GetBaseObjectScreenLocation().GetRight());
        float min_y = (float)(model->GetBaseObjectScreenLocation().GetBottom());
        float max_y = (float)(model->GetBaseObjectScreenLocation().GetTop());
        model->ImportModelChildren(model->GetModelXml(), xlights, model->GetName(), min_x, max_x, min_y, max_y);
        
        return model;
    }

};

#pragma once
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
#include "CandyCaneModel.h"
#include "ChannelBlockModel.h"
#include "CircleModel.h"
#include "CubeModel.h"
#include "CustomModel.h"
#include "IciclesModel.h"
#include "ImageModel.h"
#include "MatrixModel.h"
#include "Model.h"
#include "ModelGroup.h"
#include "PolyLineModel.h"
#include "SingleLineModel.h"
#include "SphereModel.h"
#include "SpinnerModel.h"
#include "StarModel.h"
#include "SubModel.h"
#include "ThreePointScreenLocation.h"
#include "TreeModel.h"
#include "WindowFrameModel.h"
#include "WreathModel.h"
#include "DMX/DmxColorAbilityCMY.h"
#include "DMX/DmxColorAbilityRGB.h"
#include "DMX/DmxColorAbilityWheel.h"
#include "DMX/DmxMovingHeadAdv.h"
#include "DMX/Mesh.h"

namespace XmlNodeKeys {
    // Model Node Names
    constexpr auto ModelsNodeName    = "models";
    constexpr auto ModelNodeName     = "model";
    constexpr auto ExportedAttribute = "exported";

    // Common BaseObject Attributes
    constexpr auto NameAttribute        = "name";
    constexpr auto StateNameAttribute   = "Name";  //should fix this
    constexpr auto FaceNameAttribute    = "Name";  //should fix this
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
    constexpr auto BTransparencyAttribute = "BlackTransparency";
    constexpr auto StartChannelAttribute  = "StartChannel";
    constexpr auto NodeNamesAttribute     = "NodeNames";
    constexpr auto StrandNamesAttribute   = "StrandNames";
    constexpr auto ControllerAttribute    = "Controller";
    constexpr auto versionNumberAttribute = "versionNumber";
    constexpr auto ActiveAttribute        = "Active";
    constexpr auto FromBaaseAttribute     = "FromBase";
    constexpr auto DescriptionAttribute   = "Description";
    constexpr auto CustomStringsAttribute = "String";
    constexpr auto TagColourAttribute     = "TagColour";

    // Common SubModel Attributes
    constexpr auto SubModelNodeName        = "subModel";
    constexpr auto LayoutAttribute         = "layout";
    constexpr auto SMTypeAttribute         = "type";
    constexpr auto BufferStyleAttribute    = "bufferstyle";
    constexpr auto SubBufferStyleAttribute = "subBuffer";
    constexpr auto LineAttribute           = "line";

    // ModelGroup
    constexpr auto GroupNodeName            = "modelGroup";
    constexpr auto mgSelectedAttribute      = "selected";
    constexpr auto mgLayoutAttribute        = "layout";
    constexpr auto mgGridSizeAttribute      = "GridSize";
    constexpr auto mgLayoutGroupAttribute   = "LayoutGroup";
    constexpr auto mgNameAttribute          = "name";
    constexpr auto mgCentreMinxAttribute    = "centreMinx";
    constexpr auto mgCentreMinyAttribute    = "centreMiny";
    constexpr auto mgCentreMaxxAttribute    = "centreMaxx";
    constexpr auto mgCentreMaxyAttribute    = "centreMaxy";
    constexpr auto mgModelsAttribute        = "models";
    constexpr auto mgCentrexAttribute       = "centrex";
    constexpr auto mgCentreyAttribute       = "centrey";
    constexpr auto mgCentreDefinedAttribute = "centreDefined";
    constexpr auto mgxCentreOffsetAttribute = "XCentreOffset";
    constexpr auto mgyCentreOffsetAttribute = "YCentreOffset";
    constexpr auto mgDefaultCameraAttribute = "DefaultCamera";
    constexpr auto mgTagColourAttribute     = "TagColour";

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

    // Dimentions Attributes
    constexpr auto DimUnitsAttribute  = "units";
    constexpr auto DimWidthAttribute  = "width";
    constexpr auto DimHeightAttribute = "height";
    constexpr auto DimDepthAttribute  = "depth";

    // Model Screen Location Attributes
    constexpr auto LockedAttribute = "Locked";

    // TwoPoint Screen Location Attributes
    constexpr auto X2Attribute = "X2";
    constexpr auto Y2Attribute = "Y2";
    constexpr auto Z2Attribute = "Z2";

    // ThreePoint Screen Location Attributes
    constexpr auto AngleAttribute = "Angle";
    constexpr auto ShearAttribute = "Shear";

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
    constexpr auto DmxColorTypeAttribute   = "DmxColorType";
    constexpr auto DmxBeamYOffsetAttribute = "DmxBeamYOffset";
    constexpr auto DmxBeamLengthAttribute  = "DmxBeamLength";

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

    // Servo Model
    //  TBC

    // Controller
    constexpr auto ConnectionAttribute = "Connection";
    constexpr auto ProtocolAttribute   = "Protocol";
    constexpr auto PortAttribute       = "Port";
    constexpr auto StartNullAttribute  = "startNull";
    constexpr auto EndNullAttribute    = "endNull";

    // Mesh Attributes
    constexpr auto ObjFileAttribute    = "ObjFile";
    constexpr auto MeshOnlyAttribute   = "MeshOnly";
    constexpr auto BrightnessAttribute = "Brightness";

    // Shared (by some) Attributes
    constexpr auto ArcAttribute            = "Arc";
    constexpr auto arcAttribute            = "arc";    //Arches is lowercase - maybe should be fixed
    constexpr auto AlternateNodesAttribute = "AlternateNodes";
    constexpr auto LayerSizesAttribute     = "LayerSizes";
    constexpr auto ZigZagAttribute         = "ZigZag";
    constexpr auto CustomColorsAttribute   = "CustomColors";
    constexpr auto TypeAttribute           = "type";

    // Arch Attributes
    constexpr auto HollowAttribute = "Hollow";
    constexpr auto GapAttribute    = "Gap";

    // Candy Canes
    constexpr auto CCHeightAttribute  = "CandyCaneHeight";
    constexpr auto CCReverseAttribute = "CandyCaneReverse";
    constexpr auto CCSticksAttribute  = "CandyCaneSticks";

    // Channel Block Model
    // Since this is dynamic it all gets done by the visitor

    // Circle Model
    constexpr auto InsideOutAttribute = "InsideOut";

    // Cube
    constexpr auto StyleAttribute          = "Style";
    constexpr auto CubeStartAttribute      = "Start";
    constexpr auto StrandPerLineAttribute  = "StrandPerLine";
    constexpr auto StrandPerLayerAttribute = "StrandPerLayer";

    // Custom Model
    constexpr auto CustomModelAttribute  = "CustomModel";
    constexpr auto CMBrightnessAttribute = "Brightness";
    constexpr auto StrandsAttribute      = "Strands";
    constexpr auto NodesAttribute        = "Nodes";
    constexpr auto PixelCountAttribute   = "PixelCount";
    constexpr auto PixelTypeAttribute    = "PixelType";
    constexpr auto PixelSpacingAttribute = "PixelSpacing";
    constexpr auto PixelAttribute        = "Pixel";
    constexpr auto BkgLightnessAttribute = "CustomBkgLightness";
    constexpr auto BkgImageAttribute     = "CustomBkgImage";
    constexpr auto BkgAttribute          = "Bkg";
    constexpr auto CMDepthAttribute      = "Depth";

    // Image Model
    constexpr auto ImageAttribute = "Image";
    constexpr auto BlackAttribute = "Black";
    constexpr auto ChainAttribute = "Chain";

    // Icicles Model
    constexpr auto DropPatternAttribute = "DropPattern";

    // Matrix
    constexpr auto LowDefinitionAttribute = "LowDefinition";
    constexpr auto NoZigZagAttribute      = "NoZig";

    // Poly Line Model
    constexpr auto NumPointsAttribute    = "NumPoints";
    constexpr auto PointDataAttribute    = "PointData";
    constexpr auto cPointDataAttribute   = "cPointData";
    constexpr auto IndivSegAttribute     = "IndivSegs";
    constexpr auto SegsExpandedAttribute = "SegsExpanded";
    constexpr auto ModelHeightAttribute  = "ModelHeight";

    // Sphere
    constexpr auto DegreesAttribute  = "Degrees";
    constexpr auto StartLatAttribute = "StartLatitude";
    constexpr auto EndLatAttribute   = "EndLatitude";

    // Spinner Model
    constexpr auto StartAngleAttribute = "StartAngle";
    constexpr auto HallowAttribute     = "Hollow";
    constexpr auto ArcAngleAttribute   = "Arc";
    constexpr auto AlternateAttribute  = "Alternate";

    // Star Model
    constexpr auto StarStartLocationAttribute = "StarStartLocation";
    constexpr auto StarRatioAttribute         = "starRatio";
    constexpr auto StarCenterPercentAttribute = "starCenterPercent";

    // Tree Model
    constexpr auto BottomTopRatioAttribute  = "TreeBottomTopRatio";
    constexpr auto PerspectiveAttribute     = "TreePerspective";
    constexpr auto SpiralRotationsAttribute = "TreeSpirals";
    constexpr auto TreeRotationAttribute    = "TreeSpiralRotations";
    
    // Window Frame Model
    constexpr auto RotationAttribute = "Rotation";

    // States
    constexpr auto StateNodeName = "stateInfo";
    
    // Faces
    constexpr auto FaceNodeName = "faceInfo";

    // WIP
    // constexpr auto Attribute = "";

    // View_Object

    // Effect Version

    // Effect Group

    // View

    // Group - already done

    // Perspectives

    // Settings

    // ViewPoint

    // Model Types
    constexpr auto ArchesType           = "Arches";
    constexpr auto CandyCaneType        = "Candy Canes";
    constexpr auto ChannelBlockType     = "Channel Block";
    constexpr auto CircleType           = "Circle";
    constexpr auto CubeType             = "Cube";
    constexpr auto CustomType           = "Custom";
    constexpr auto DmxMovingHeadAdvType = "DmxMovingHeadAdv";
    constexpr auto IciclesType          = "Icicles";
    constexpr auto ImageType            = "Image";
    constexpr auto MatrixType           = "Matrix";
    constexpr auto SingleLineType       = "Single Line";
    constexpr auto PolyLineType         = "Poly Line";
    constexpr auto SphereType           = "Sphere";
    constexpr auto SpinnerType          = "Spinner";
    constexpr auto StarType             = "Star";
    constexpr auto TreeType             = "Tree";
    constexpr auto WindowType           = "Window Frame";
    constexpr auto WreathType           = "Wreath";
};

struct XmlSerializingVisitor : BaseObjectVisitor {
    XmlSerializingVisitor(wxXmlNode* parentNode) :
        parentNode(parentNode) {
    }

    wxXmlNode* parentNode;

    void AddBaseObjectAttributes(const BaseObject& base, wxXmlNode* node) {
        node->AddAttribute(XmlNodeKeys::NameAttribute, base.GetName());
        node->AddAttribute(XmlNodeKeys::DisplayAsAttribute, base.GetDisplayAs());
        node->AddAttribute(XmlNodeKeys::LayoutGroupAttribute, base.GetLayoutGroup());
    }

    void AddCommonModelAttributes(const Model& model, wxXmlNode* node) {
        node->AddAttribute(XmlNodeKeys::StartSideAttribute, model.GetStartSide());
        node->AddAttribute(XmlNodeKeys::DirAttribute, model.GetDirection());
        node->AddAttribute(XmlNodeKeys::Parm1Attribute, std::to_string(model.GetParm1()));
        node->AddAttribute(XmlNodeKeys::Parm2Attribute, std::to_string(model.GetParm2()));
        node->AddAttribute(XmlNodeKeys::Parm3Attribute, std::to_string(model.GetParm3()));
        node->AddAttribute(XmlNodeKeys::AntialiasAttribute, std::to_string((long)model.GetPixelStyle()));
        node->AddAttribute(XmlNodeKeys::PixelSizeAttribute, std::to_string(model.GetPixelSize()));
        node->AddAttribute(XmlNodeKeys::StringTypeAttribute, model.GetStringType());
        node->AddAttribute(XmlNodeKeys::TransparencyAttribute, std::to_string(model.GetTransparency()));
        node->AddAttribute(XmlNodeKeys::BTransparencyAttribute, std::to_string(model.GetBlackTransparency()));
        node->AddAttribute(XmlNodeKeys::DescriptionAttribute, model.GetDescription());
        node->AddAttribute(XmlNodeKeys::TagColourAttribute, model.GetTagColour().GetAsString(wxC2S_HTML_SYNTAX));
        node->AddAttribute(XmlNodeKeys::StartChannelAttribute, model.GetModelStartChannel());
        node->AddAttribute(XmlNodeKeys::NodeNamesAttribute, model.GetNodeNames());
        node->AddAttribute(XmlNodeKeys::StrandNamesAttribute, model.GetStrandNames());
        node->AddAttribute(XmlNodeKeys::ControllerAttribute, model.GetControllerName());
        node->AddAttribute(XmlNodeKeys::versionNumberAttribute, CUR_MODEL_POS_VER);
    }

    void AddModelScreenLocationAttributes(const BaseObject& base, wxXmlNode* node) {
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

    void AddThreePointScreenLocationAttributes(const BaseObject& base, wxXmlNode* node) {
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

    void AddColorAbilityRGBAttributes(const DmxColorAbilityRGB* colors, wxXmlNode* node) {
        node->AddAttribute(XmlNodeKeys::DmxRedChannelAttribute, std::to_string(colors->GetRedChannel()));
        node->AddAttribute(XmlNodeKeys::DmxGreenChannelAttribute, std::to_string(colors->GetGreenChannel()));
        node->AddAttribute(XmlNodeKeys::DmxBlueChannelAttribute, std::to_string(colors->GetBlueChannel()));
        node->AddAttribute(XmlNodeKeys::DmxWhiteChannelAttribute, std::to_string(colors->GetWhiteChannel()));
    }

    void AddColorWheelAttributes(const DmxColorAbilityWheel* colors, wxXmlNode* node) {
        node->AddAttribute(XmlNodeKeys::DmxColorWheelChannelAttribute, std::to_string(colors->GetWheelChannel()));
        node->AddAttribute(XmlNodeKeys::DmxDimmerChannelAttribute, std::to_string(colors->GetDimmerChannel()));
        std::vector<WheelColor> settings = colors->GetWheelColorSettings();
        int index = 0;
        for (const auto& it : settings) {
            node->AddAttribute(XmlNodeKeys::DmxColorWheelColorAttribute + std::to_string(index), (std::string)it.color);
            node->AddAttribute(XmlNodeKeys::DmxColorWheelDMXAttribute + std::to_string(index), std::to_string(it.dmxValue));
            ++index;
        }
    }

    void AddColorAbilityCMYAttributes(const DmxColorAbilityCMY* colors, wxXmlNode* node) {
        node->AddAttribute(XmlNodeKeys::DmxCyanChannelAttribute, std::to_string(colors->GetCyanChannel()));
        node->AddAttribute(XmlNodeKeys::DmxMagentaChannelAttribute, std::to_string(colors->GetMagentaChannel()));
        node->AddAttribute(XmlNodeKeys::DmxYellowChannelAttribute, std::to_string(colors->GetYellowChannel()));
        node->AddAttribute(XmlNodeKeys::DmxWhiteChannelAttribute, std::to_string(colors->GetWhiteChannel()));
    }

    void AddColorAttributes(const DmxModel& dmx_model, wxXmlNode* node) {
        if (dmx_model.HasColorAbility()) {
            DmxColorAbility* color_ability = dmx_model.GetColorAbility();
            std::string color_type = color_ability->GetTypeName();
            node->AddAttribute(XmlNodeKeys::DmxColorTypeAttribute, std::to_string(dmx_model.DmxColorTypetoID(color_type)));
            if (color_type == "RGBW") {
                AddColorAbilityRGBAttributes(reinterpret_cast<DmxColorAbilityRGB*>(color_ability), node);
            } else if (color_type == "ColorWheel") {
                AddColorWheelAttributes(reinterpret_cast<DmxColorAbilityWheel*>(color_ability), node);
            } else if (color_type == "CMYW") {
                AddColorAbilityCMYAttributes(reinterpret_cast<DmxColorAbilityCMY*>(color_ability), node);
            }
        }
    }

    void AddDmxMotorAttributes(const DmxMotor* motor, wxXmlNode* node) {
        wxXmlNode* motor_node = new wxXmlNode(wxXML_ELEMENT_NODE, motor->GetName());
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

    void AddMeshAttributes(const Mesh* mesh, wxXmlNode* node) {
        wxXmlNode* mesh_node = new wxXmlNode(wxXML_ELEMENT_NODE, mesh->GetName());
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

    wxString vectorToString(const std::vector<int>& v, const std::string& separator = ",") {
        wxString oss;
        for (size_t i = 0; i < v.size(); ++i) {
            oss << v[i];
            if (i < v.size() - 1) {
                oss << separator;
            }
        }
        return oss;
    }

    void SortAttributes(wxXmlNode* input) {
        const wxString attributeToPrioritize = "name";
        std::vector<std::pair<std::string, std::string>> attributes;
        for (wxXmlAttribute* attr = input->GetAttributes(); attr != nullptr; attr = attr->GetNext()) {
            attributes.push_back({ std::string(attr->GetName()), std::string(attr->GetValue()) });
        }
        /*std::sort(attributes.begin(), attributes.end(), [](const std::pair<std::string, std::string>& a, const std::pair<std::string, std::string>& b) {
            return Lower(a.first) < Lower(b.first);
        });*/
        auto custom_comparator = [&attributeToPrioritize](const std::pair<wxString, wxString>& a, const std::pair<wxString, wxString>& b) {
            if (a.first == attributeToPrioritize) return true;
            if (b.first == attributeToPrioritize) return false;
            return Lower(a.first) < Lower(b.first);
        };
        std::sort(attributes.begin(), attributes.end(), custom_comparator);
        wxXmlAttribute* currentAttr = input->GetAttributes();
        while (currentAttr != nullptr) {
            wxXmlAttribute* toRemove = currentAttr;
            currentAttr = currentAttr->GetNext();
            input->DeleteAttribute(toRemove->GetName());
        }
        for (const auto& attr : attributes) {
            input->AddAttribute(attr.first, attr.second);
        }
    }

    void AddFacesandStates(wxXmlNode* node, const Model* m) {

        FaceStateData faces = m->GetFaceInfo();
        for (const auto& f : faces) {
            wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::FaceNodeName);
            xmlNode->AddAttribute(XmlNodeKeys::FaceNameAttribute, f.first);
            for (const auto& f2 : f.second) {
                /*if (f2.first == "CustomColors") {
                    xmlNode->AddAttribute(XmlNodeKeys::CustomColorsAttribute, f2.second);
                } else if (f2.first == "Type") {
                    xmlNode->AddAttribute(XmlNodeKeys::TypeAttribute, f2.second);
                } else if (f2.first != "") {
                    xmlNode->AddAttribute(f2.first, f2.second);
                }*/
                if (f2.first != "") xmlNode->AddAttribute(f2.first, f2.second);
            }
            SortAttributes(xmlNode);
            node->AddChild(xmlNode);
        }

        FaceStateData states = m->GetStateInfo();
        for (const auto& s : states) {
            wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::StateNodeName);
            xmlNode->AddAttribute(XmlNodeKeys::StateNameAttribute, s.first);
            for (const auto& s2 : s.second) {
                /*if (s2.first == "CustomColors") {
                    xmlNode->AddAttribute(XmlNodeKeys::CustomColorsAttribute, s2.second);
                } else if (s2.first == "Type") {
                    xmlNode->AddAttribute(XmlNodeKeys::TypeAttribute, s2.second);
                } else if (s2.first != "") {
                    xmlNode->AddAttribute(s2.first, s2.second);
                }*/
                if (s2.first != "") xmlNode->AddAttribute(s2.first, s2.second);
            }
            SortAttributes(xmlNode);
            node->AddChild(xmlNode);
        }
    }

    void AddAliases(wxXmlNode* node, const std::list<std::string> aliases) {
        wxXmlNode* aliashdr = new wxXmlNode(wxXML_ELEMENT_NODE, "Aliases");

        if (aliases.size() > 0) {
            for (const auto& a : aliases) {
                wxXmlNode* alias = new wxXmlNode(wxXML_ELEMENT_NODE, "alias");
                alias->AddAttribute("name", a);
                aliashdr->AddChild(alias);
            }
            node->AddChild(aliashdr);
        }
    }

    void AddSubmodels(wxXmlNode* node, const Model* m) {
        const std::vector<Model*>& submodelList = m->GetSubModels();
       
        for (Model* s : submodelList) {
            wxXmlNode* submodels = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::SubModelNodeName);
            const SubModel* submodel = dynamic_cast<const SubModel*>(s);
            if (submodel == nullptr) return;
            submodels->AddAttribute(XmlNodeKeys::NameAttribute, s->GetName());
            submodels->AddAttribute(XmlNodeKeys::LayoutAttribute, submodel->GetSubModelLayout());
            submodels->AddAttribute(XmlNodeKeys::SMTypeAttribute, submodel->GetSubModelType());
            submodels->AddAttribute(XmlNodeKeys::BufferStyleAttribute, submodel->GetSubModelBufferStyle());
            const std::string submodelType = submodel->GetSubModelType();
            if (submodelType == "subbuffer") {
                submodels->AddAttribute(XmlNodeKeys::SubBufferStyleAttribute, submodel->GetSubModelNodeRanges());
            } else {
                wxArrayString nodeInfo = wxSplit(submodel->GetSubModelNodeRanges(), ',');
                for (auto i = 0; i < nodeInfo.size(); i++) {
                    submodels->AddAttribute("line" + std::to_string(i), nodeInfo[i]);
                }
            }
            SortAttributes(submodels);
            AddAliases(submodels, s->GetAliases());
            node->AddChild(submodels);
        }
    }

    void AddGroups(wxXmlNode* node, const Model* m) {
        const ModelManager& mgr = m->GetModelManager();
        std::vector<Model*> mg = mgr.GetModelGroups(m);

        for (const Model* g : mg) {
            const ModelGroup* mg1 = dynamic_cast<const ModelGroup*>(g);
            if (mg1 == nullptr) return;
            wxXmlNode* groups = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::GroupNodeName);
            groups->AddAttribute(XmlNodeKeys::mgNameAttribute, g->GetName());
            groups->AddAttribute(XmlNodeKeys::mgModelsAttribute, m->GetName());
            groups->AddAttribute(XmlNodeKeys::mgLayoutGroupAttribute, g->GetLayoutGroup());
            groups->AddAttribute(XmlNodeKeys::mgSelectedAttribute, std::to_string(mg1->IsSelected()));
            groups->AddAttribute(XmlNodeKeys::mgLayoutAttribute, mg1->GetLayout());
            groups->AddAttribute(XmlNodeKeys::mgGridSizeAttribute, std::to_string(mg1->GetGridSize()));
            groups->AddAttribute(XmlNodeKeys::mgTagColourAttribute, mg1->GetTagColour().GetAsString(wxC2S_HTML_SYNTAX));
            groups->AddAttribute(XmlNodeKeys::mgCentreMinxAttribute, std::to_string(mg1->GetCentreMinx()));
            groups->AddAttribute(XmlNodeKeys::mgCentreMinyAttribute, std::to_string(mg1->GetCentreMiny()));
            groups->AddAttribute(XmlNodeKeys::mgCentreMaxxAttribute, std::to_string(mg1->GetCentreMaxx()));
            groups->AddAttribute(XmlNodeKeys::mgCentreMaxyAttribute, std::to_string(mg1->GetCentreMaxy()));
            groups->AddAttribute(XmlNodeKeys::mgCentrexAttribute, std::to_string(mg1->GetCentreX()));
            groups->AddAttribute(XmlNodeKeys::mgCentreyAttribute, std::to_string(mg1->GetCentreY()));
            groups->AddAttribute(XmlNodeKeys::mgCentreDefinedAttribute, std::to_string(mg1->GetCentreDefined()));
            groups->AddAttribute(XmlNodeKeys::mgDefaultCameraAttribute, mg1->GetDefaultCamera());
            groups->AddAttribute(XmlNodeKeys::mgxCentreOffsetAttribute, std::to_string(mg1->GetXCentreOffset()));
            groups->AddAttribute(XmlNodeKeys::mgyCentreOffsetAttribute, std::to_string(mg1->GetYCentreOffset()));
            const std::list<std::string>& aliases = mg1->GetAliases();
            AddAliases(groups,aliases);
            SortAttributes(groups);
            node->AddChild(groups);
        }
    }

    //xmlNode->AddAttribute(XmlNodeKeys::#, std::to_string(#()));

    void Visit(const ArchesModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::ZigZagAttribute, model.GetZigZag() ? "true": "false");
        xmlNode->AddAttribute(XmlNodeKeys::HollowAttribute, std::to_string(model.GetHollow()));
        xmlNode->AddAttribute(XmlNodeKeys::GapAttribute, std::to_string(model.GetGap()));
        xmlNode->AddAttribute(XmlNodeKeys::arcAttribute, std::to_string(model.GetArc()));
        xmlNode->AddAttribute(XmlNodeKeys::LayerSizesAttribute, vectorToString(model.GetLayerSizes()));
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        SortAttributes(xmlNode);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const CandyCaneModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::CCHeightAttribute, std::to_string(model.GetCandyCaneHeight()));
        xmlNode->AddAttribute(XmlNodeKeys::CCReverseAttribute, model.IsReverse() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::CCSticksAttribute, model.IsSticks() ? "true": "false");
        xmlNode->AddAttribute(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const CircleModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::InsideOutAttribute, model.IsInsideOut() ? "1" : "0");
        xmlNode->AddAttribute(XmlNodeKeys::LayerSizesAttribute, vectorToString(model.GetLayerSizes()));
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);        
    }

    void Visit(const ChannelBlockModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        std::vector<std::string> cp = model.GetChannelProperies();
        for (auto i = 0; i < cp.size();  i++) {
            xmlNode->AddAttribute("ChannelProperties.ChannelColor" + std::to_string(i+1), cp[i]);
        }
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const CubeModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::StyleAttribute, model.GetStrandStyle());
        xmlNode->AddAttribute(XmlNodeKeys::CubeStartAttribute, model.GetStrandStart());
        xmlNode->AddAttribute(XmlNodeKeys::StrandPerLineAttribute, model.GetStrandPerLine());
        xmlNode->AddAttribute(XmlNodeKeys::StrandPerLayerAttribute, model.GetStrandPerLayer());
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const CustomModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::CMDepthAttribute, std::to_string(model.GetCustomDepth()));
        xmlNode->AddAttribute(XmlNodeKeys::CustomModelAttribute, model.GetCustomData());
        xmlNode->AddAttribute(XmlNodeKeys::BkgImageAttribute, model.GetCustomBackground());
        xmlNode->AddAttribute(XmlNodeKeys::BkgLightnessAttribute, std::to_string(model.GetCustomLightness()));
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        AddFacesandStates(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }
    void Visit(const IciclesModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::DropPatternAttribute, model.GetDropPattern());
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const ImageModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::ImageAttribute, model.GetImageFile());
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const MatrixModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->DeleteAttribute(XmlNodeKeys::DisplayAsAttribute);
        if (model.isVerticalMatrix()) {
            xmlNode->AddAttribute(XmlNodeKeys::DisplayAsAttribute, "Vert Matrix");
        } else {
            xmlNode->AddAttribute(XmlNodeKeys::DisplayAsAttribute, "Horiz Matrix");
        }
        xmlNode->AddAttribute(XmlNodeKeys::LowDefinitionAttribute, std::to_string(model.GetLowDefFactor()));
        xmlNode->AddAttribute(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::NoZigZagAttribute, model.IsNoZigZag() ? "true" : "false");
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const SingleLineModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const PolyLineModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::IndivSegAttribute, model.HasIndivSegs() ? "1" : "0");
        xmlNode->AddAttribute(XmlNodeKeys::DropPatternAttribute, model.GetDropPattern());
        xmlNode->AddAttribute(XmlNodeKeys::NumPointsAttribute, model.GetNumPoints());
        std::vector<int> segSize = model.GetSegmentsSizes();
        for (auto i = 0; i < segSize.size(); i++) {
            xmlNode->AddAttribute("Seg" + std::to_string(i + 1), std::to_string(segSize[i] / (segSize.size()+1)));
        }
        std::vector<std::string> cSize = model.GetCorners();
        for (auto i = 0; i < cSize.size(); i++) {
            xmlNode->AddAttribute("Corner" + std::to_string(i + 1), cSize[i]);
        }
        xmlNode->AddAttribute(XmlNodeKeys::PointDataAttribute, model.GetPointData());
        xmlNode->AddAttribute(XmlNodeKeys::cPointDataAttribute, model.GetcPointData());
        xmlNode->AddAttribute(XmlNodeKeys::SegsExpandedAttribute, model.AreSegsExpanded() ? "TRUE" : "FALSE");
        xmlNode->AddAttribute(XmlNodeKeys::ModelHeightAttribute, std::to_string(model.GetHeight()));
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const SphereModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::DegreesAttribute, std::to_string(model.GetSphereDegrees()));
        xmlNode->AddAttribute(XmlNodeKeys::StartLatAttribute, std::to_string(model.GetStartLatitude()));
        xmlNode->AddAttribute(XmlNodeKeys::EndLatAttribute, std::to_string(model.GetEndLatitude()));
        xmlNode->AddAttribute(XmlNodeKeys::LowDefinitionAttribute, std::to_string(model.GetLowDefFactor()));
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const SpinnerModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::AlternateAttribute, model.HasAlternateNodes() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::ZigZagAttribute, model.HasZigZag() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::HallowAttribute, std::to_string(model.GetHollowPercent()));
        xmlNode->AddAttribute(XmlNodeKeys::ArcAngleAttribute, std::to_string(model.GetArcAngle()));
        xmlNode->AddAttribute(XmlNodeKeys::StartAngleAttribute, std::to_string(model.GetStartAngle()));
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const StarModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::LayerSizesAttribute, vectorToString(model.GetLayerSizes()));
        xmlNode->AddAttribute(XmlNodeKeys::StarStartLocationAttribute, model.GetStartLocation());
        xmlNode->AddAttribute(XmlNodeKeys::StarRatioAttribute, std::to_string(model.GetStarRatio()));
        xmlNode->AddAttribute(XmlNodeKeys::StarCenterPercentAttribute, std::to_string(model.GetInnerPercent()));
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const TreeModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->DeleteAttribute(XmlNodeKeys::DisplayAsAttribute);
        xmlNode->AddAttribute(XmlNodeKeys::DisplayAsAttribute, model.GetTreeDescription());
        xmlNode->AddAttribute(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::NoZigZagAttribute, model.IsNoZigZag() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::BottomTopRatioAttribute, std::to_string(model.GetBottomTopRatio()));
        xmlNode->AddAttribute(XmlNodeKeys::PerspectiveAttribute, std::to_string(model.GetTreePerspective()));
        xmlNode->AddAttribute(XmlNodeKeys::SpiralRotationsAttribute, std::to_string(model.GetSpiralRotations()));
        xmlNode->AddAttribute(XmlNodeKeys::TreeRotationAttribute, std::to_string(model.GetTreeRotation()));
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const WindowFrameModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::RotationAttribute, model.GetRotation() ? "Counter Clockwise" : "Clockwise");
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const WreathModel& model) override {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        //AddThreePointScreenLocationAttributes(model, xmlNode);
        AddAliases(xmlNode, model.GetAliases());
        const Model* m = dynamic_cast<const Model*>(&model);
        if (m == nullptr) return;
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }

    void Visit(const DmxMovingHeadAdv& moving_head) override {
        wxXmlNode* mhNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
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
        const Model* m = dynamic_cast<const Model*>(&moving_head);
        if (m == nullptr)
            return;
        AddSubmodels(mhNode, m);
        AddGroups(mhNode, m);
        parentNode->AddChild(mhNode);
    }
};

struct XmlDeserializingObjectFactory {
    Model* Deserialize(wxXmlNode* node, xLightsFrame* xlights) {
        auto type = node->GetAttribute(XmlNodeKeys::DisplayAsAttribute);

        if (type == XmlNodeKeys::ArchesType) {
            return DeserializeArches(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::CandyCaneType) {
            return DeserializeCandyCane(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::ChannelBlockType) {
            return DeserializeChannelBlock(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::CircleType) {
            return DeserializeCircle(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::CubeType) {
            return DeserializeCube(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::CustomType) {
            return DeserializeCustom(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::DmxMovingHeadAdvType) {
            return DeserializeDmxMovingHeadAdv(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::IciclesType) {
            return DeserializeIcicles(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::ImageType) {
            return DeserializeImage(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::MatrixType) {
            return DeserializeMatrix(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::SingleLineType) {
            return DeserializeSingleLine(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::PolyLineType) {
            return DeserializePolyLine(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::SphereType) {
            return DeserializeSphere(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::SpinnerType) {
            return DeserializeSpinner(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::StarType) {
            return DeserializeStar(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::TreeType) {
            return DeserializeTree(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::WindowType) {
            return DeserializeWindow(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::WreathType) {
            return DeserializeWreath(new wxXmlNode(*node), xlights);
        }

        throw std::runtime_error("Unknown object type: " + type);
    }

private:
    Model* DeserializeArches(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new ArchesModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeCandyCane(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new CandyCaneModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

     Model* DeserializeChannelBlock(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new ChannelBlockModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeCircle(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new CircleModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeCube(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new CubeModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeCustom(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new CustomModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeDmxMovingHeadAdv(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new DmxMovingHeadAdv(node, xlights->AllModels, false);

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

    Model* DeserializeIcicles(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new IciclesModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeImage(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new ImageModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeMatrix(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new MatrixModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeSingleLine(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new SingleLineModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializePolyLine(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new PolyLineModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeSphere(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new SphereModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeSpinner(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new SpinnerModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeStar(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new StarModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeTree(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new TreeModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeWindow(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new WindowFrameModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }

    Model* DeserializeWreath(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new WreathModel(node, xlights->AllModels, false);

        std::string name = node->GetAttribute("name");
        wxString newname = xlights->AllModels.GenerateModelName(name);
        model->SetProperty("name", newname, true);

        return model;
    }
};

struct XmlSerializer {
    XmlSerializer() {
    }

    // Serializes and Saves a single model into an XML document
    void SerializeAndSaveModel(const BaseObject& object) {
        wxString name = object.GetModelXml()->GetAttribute("name");
        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (filename.IsEmpty())
            return;
        wxXmlDocument doc = SerializeModel(object);
        doc.Save(filename);
    }

    // Serialize a single model into an XML document
    wxXmlDocument SerializeModel(const BaseObject& object) {
        wxXmlDocument doc;

        wxXmlNode* docNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelsNodeName);
        docNode->AddAttribute(XmlNodeKeys::TypeAttribute, XmlNodeKeys::ExportedAttribute);

        XmlSerializingVisitor visitor{ docNode };

        object.Accept(visitor);

        doc.SetRoot(docNode);

        return doc;
    }

    // Deserialize a single model from an XML document
    Model* DeserializeModel(const wxXmlDocument& doc, xLightsFrame* xlights) {
        wxXmlNode* root = doc.GetRoot();
        wxXmlNode* model_node = root->GetChildren();
        return DeserializeModel(model_node, xlights);
    }

    // Deserialize a single model XML node
    Model* DeserializeModel(wxXmlNode* model_node, xLightsFrame* xlights) {
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

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
    constexpr auto ModelsNodeName = "models";
    constexpr auto ModelNodeName = "model";
    constexpr auto TypeAttribute = "type";
    constexpr auto ExportedAttribute = "exported";

    // Common BaseObject Attributes
    constexpr auto NameAttribute = "name";
    constexpr auto DisplayAsAttribute = "DisplayAs";
    constexpr auto LayoutGroupAttribute = "LayoutGroup";

    // Common Model Attributes
    constexpr auto StartSideAttribute = "StartSide";
    constexpr auto DirAttribute = "Dir";
    constexpr auto Parm1Attribute = "parm1";
    constexpr auto Parm2Attribute = "parm2";
    constexpr auto Parm3Attribute = "parm3";
    constexpr auto AntialiasAttribute = "Antialias";
    constexpr auto PixelSizeAttribute = "PixelSize";
    constexpr auto StringTypeAttribute = "StringType";
    constexpr auto TransparencyAttribute = "Transparency";
    constexpr auto BTransparencyAttribute = "BlackTransparency";
    constexpr auto StartChannelAttribute = "StartChannel";
    constexpr auto NodeNamesAttribute = "NodeNames";
    constexpr auto StrandNamesAttribute = "StrandNames";
    constexpr auto ControllerAttribute = "Controller";
    constexpr auto versionNumberAttribute = "versionNumber";
    constexpr auto ActiveAttribute = "Active";
    constexpr auto FromBaaseAttribute = "FromBase";
    constexpr auto DescriptionAttribute = "Description";
    constexpr auto CustomStringsAttribute = "String";

    // Common SubModel Attributes
    constexpr auto SubModelNodeName = "subModel";
    constexpr auto SubModelNameAttribute = "name";
    constexpr auto LayoutAttribute = "layout";
    constexpr auto SMTypeAttribute = "type";
    constexpr auto BufferStyleAttribute = "bufferstyle";
    constexpr auto SubBufferStyleAttribute = "subBuffer";
    constexpr auto LineAttribute = "line";

    // ModelGroup
    constexpr auto GroupNodeName = "modelGroup";
    constexpr auto mgSelectedAttribute = "selected";
    constexpr auto mgLayoutAttribute = "layout";
    constexpr auto mgGridSizeAttribute = "GridSize";
    constexpr auto mgLayoutGroupAttribute = "LayoutGroup";
    constexpr auto mgNameAttribute = "name";
    constexpr auto mgCentreMinxAttribute = "centreMinx";
    constexpr auto mgCentreMinyAttribute = "centreMiny";
    constexpr auto mgCentreMaxxAttribute = "centreMaxx";
    constexpr auto mgCentreMaxyAttribute = "centreMaxy";
    constexpr auto mgModelsAttribute = "models";
    constexpr auto mgCentrexAttribute = "centrex";
    constexpr auto mgCentreyAttribute = "centrey";
    constexpr auto mgCentreDefinedAttribute = "centreDefined";
    constexpr auto mgxCentreOffsetAttribute = "XCentreOffset";
    constexpr auto mgyCentreOffsetAttribute = "YCentreOffset";
    constexpr auto mgDefaultCameraAttribute = "DefaultCamera";
    constexpr auto mgTagColourAttribute = "TagColour";

    // Size/Position Attributes
    constexpr auto WorldPosXAttribute = "WorldPosX";
    constexpr auto WorldPosYAttribute = "WorldPosY";
    constexpr auto WorldPosZAttribute = "WorldPosZ";
    constexpr auto ScaleXAttribute = "ScaleX";
    constexpr auto ScaleYAttribute = "ScaleY";
    constexpr auto ScaleZAttribute = "ScaleZ";
    constexpr auto RotateXAttribute = "RotateX";
    constexpr auto RotateYAttribute = "RotateY";
    constexpr auto RotateZAttribute = "RotateZ";
    constexpr auto WidthAttribute = "Width";
    constexpr auto HeightAttribute = "Height";
    constexpr auto DepthAttribute = "Depth";
    constexpr auto OffsetXAttribute = "OffsetX";
    constexpr auto OffsetYAttribute = "OffsetY";
    constexpr auto OffsetZAttribute = "OffsetZ";

    // Dimentions Attributes
    constexpr auto DimUnitsAttribute = "units";
    constexpr auto DimWidthAttribute = "width";
    constexpr auto DimHeightAttribute = "height";
    constexpr auto DimDepthAttribute = "depth";

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
    constexpr auto DmxRedChannelAttribute = "DmxRedChannel";
    constexpr auto DmxGreenChannelAttribute = "DmxGreenChannel";
    constexpr auto DmxBlueChannelAttribute = "DmxBlueChannel";
    constexpr auto DmxWhiteChannelAttribute = "DmxWhiteChannel";

    // DmxColorAbilityWheel Attributes
    constexpr auto DmxColorWheelChannelAttribute = "DmxColorWheelChannel";
    constexpr auto DmxDimmerChannelAttribute = "DmxDimmerChannel";
    constexpr auto DmxColorWheelColorAttribute = "DmxColorWheelColor";
    constexpr auto DmxColorWheelDMXAttribute = "DmxColorWheelDMX";

    // DmxColorAbilityCMY Attributes
    constexpr auto DmxCyanChannelAttribute = "DmxCyanChannel";
    constexpr auto DmxMagentaChannelAttribute = "DmxMagentaChannel";
    constexpr auto DmxYellowChannelAttribute = "DmxYellowChannel";

    // DmxMovingHeadComm Attributes
    constexpr auto DmxFixturelAttribute = "DmxFixture";

    // DMX Moving Head Attributes
    constexpr auto DmxColorTypeAttribute = "DmxColorType";
    constexpr auto DmxBeamYOffsetAttribute = "DmxBeamYOffset";
    constexpr auto DmxBeamLengthAttribute = "DmxBeamLength";

    // DmxMotor Attributes
    constexpr auto ChannelCoarseAttribute = "ChannelCoarse";
    constexpr auto ChannelFineAttribute = "ChannelFine";
    constexpr auto MinLimitAttribute = "MinLimit";
    constexpr auto MaxLimitAttribute = "MaxLimit";
    constexpr auto RangeOfMotionAttribute = "RangeOfMotion";
    constexpr auto OrientZeroAttribute = "OrientZero";
    constexpr auto OrientHomeAttribute = "OrientHome";
    constexpr auto SlewLimitAttribute = "SlewLimit";
    constexpr auto ReverseAttribute = "Reverse";
    constexpr auto UpsideDownAttribute = "UpsideDown";

    // Servo Model
    //  TBC

    // Controller
    constexpr auto ConnectionAttribute = "Connection";
    constexpr auto ProtocolAttribute = "Protocol";
    constexpr auto PortAttribute = "Port";
    constexpr auto StartNullAttribute = "startNull";
    constexpr auto EndNullAttribute = "endNull";

    // Arch Attributes
    constexpr auto ZigZagAttribute = "ZigZag";
    constexpr auto HollowAttribute = "Hollow";
    constexpr auto GapAttribute = "Gap";

    // Mesh Attributes
    constexpr auto ObjFileAttribute = "ObjFile";
    constexpr auto MeshOnlyAttribute = "MeshOnly";
    constexpr auto BrightnessAttribute = "Brightness";

    // Arch, CandyCane, Icicles, Single Line Models

    // Channel Block Model
    constexpr auto ChannelPropertiesCC1Attribute = "ChannelProperties.ChannelColor1";
    constexpr auto ChannelPropertiesCC2Attribute = "ChannelProperties.ChannelColor2";
    constexpr auto ChannelPropertiesCC3Attribute = "ChannelProperties.ChannelColor3";
    constexpr auto ChannelPropertiesCC4Attribute = "ChannelProperties.ChannelColor4";
    constexpr auto ChannelPropertiesCC5Attribute = "ChannelProperties.ChannelColor5";
    constexpr auto ChannelPropertiesCC6Attribute = "ChannelProperties.ChannelColor6";
    constexpr auto ChannelPropertiesCC7Attribute = "ChannelProperties.ChannelColor7";
    constexpr auto ChannelPropertiesCC8Attribute = "ChannelProperties.ChannelColor8";
    constexpr auto ChannelPropertiesCC9Attribute = "ChannelProperties.ChannelColor9";
    constexpr auto ChannelPropertiesCC10Attribute = "ChannelProperties.ChannelColor10";
    constexpr auto ChannelPropertiesCC11Attribute = "ChannelProperties.ChannelColor11";
    constexpr auto ChannelPropertiesCC12Attribute = "ChannelProperties.ChannelColor12";
    constexpr auto ChannelPropertiesCC13Attribute = "ChannelProperties.ChannelColor13";
    constexpr auto ChannelPropertiesCC14Attribute = "ChannelProperties.ChannelColor14";
    constexpr auto ChannelPropertiesCC15Attribute = "ChannelProperties.ChannelColor15";
    constexpr auto ChannelPropertiesCC16Attribute = "ChannelProperties.ChannelColor16";

    // Circle Model
    constexpr auto InsideOutAttribute = "InsideOut";

    // Cube
    constexpr auto StyleAttribute = "Style";
    constexpr auto StrandPerLineAttribute = "SrtrandPerLine";

    // Custom Model
    constexpr auto CustomModel = "Custom";
    constexpr auto CMBrightnessAttribute = "Brightness";
    constexpr auto StrandsAttribute = "Strands";
    constexpr auto NodesAttribute = "Nodes";
    constexpr auto PixelCountAttribute = "PixelCount";
    constexpr auto PixelTypeAttribute = "PixelType";
    constexpr auto PixelSpacingAttribute = "PixelSpacing";
    constexpr auto PixelAttribute = "Pixel";
    constexpr auto BkgLightnessAttribute = "BkgLightness";
    constexpr auto BkgAttribute = "Bkg";

    // Image Model
    constexpr auto ImageAttribute = "Image";
    constexpr auto BlackAttribute = "Black";
    constexpr auto ChainAttribute = "Chain";

    // Icicles Model
    constexpr auto DropPatternAttribute = "DropPattern";

    // Poly Line Model
    constexpr auto NumPointsAttribute = "NumPoints";
    constexpr auto PointDataAttribute = "PointData";
    constexpr auto cPointDataAttribute = "cPointData";
    constexpr auto IndivegAttribute = "Indiveg";
    constexpr auto SegExpandedAttribute = "SegExpanded";
    constexpr auto SegAttribute = "Seg";       // needs fix Seg1, Seg2, Seg3
    constexpr auto CornerAttribute = "Corner"; // needs fix Corner1, Corner2, Corner3

    // Spinner Model
    constexpr auto ArcAttribute = "Arc";
    constexpr auto StringAttribute = "String";
    constexpr auto AlternateAttribute = "Alternate";
    constexpr auto StartAttribute = "Start";

    // Star Model
    constexpr auto StarStartLocationAttribute = "StarStartLocation";
    constexpr auto LayerSizesAttribute = "LayerSizes";
    constexpr auto TagColourAttribute = "TagColour";

    // Tree Model
    constexpr auto TreeBottomTopRatioAttribute = "TreeBottomTopRatio";
    constexpr auto TreePerspectiveAttribute = "TreePerspective";
    constexpr auto TreeAttribute = "Tree";
    constexpr auto TreeSpiralsAttribute = "TreeSpirals";

    // Window Frame Model
    constexpr auto RotationAttribute = "Rotation";

    // States
    constexpr auto StateAttribute = "stateInfo";
    constexpr auto ColorsAttribute = "Colors";
    constexpr auto sAttribute = "s";           // needs enumeration s1, s2, s3 etc
    constexpr auto sColorAttribute = "sColor"; // needs enumeration s1-Color s2-Color, s3-Color etc
    constexpr auto sNameAttribute = "sName";   // needs enumeration s1-Name s2-Name, s3-Name etc

    // Faces
    constexpr auto EyesClosedAttribute = "Eyes - Closed";
    constexpr auto EyesClosedColorAttribute = "Eyes - Closed - Color";
    constexpr auto EyesClosed2Attribute = "Eyes - Closed2";
    constexpr auto EyesClosed2ColorAttribute = "Eyes - Closed2 - Color";
    constexpr auto EyesClosed3Attribute = "Eyes - Closed3";
    constexpr auto EyesClosed3ColorAttribute = "Eyes - Closed3 - Color";
    constexpr auto EyesOpenAttribute = "Eyes - Open";
    constexpr auto EyesOpenColorAttribute = "Eyes - Open - Color";
    constexpr auto EyesOpen2Attribute = "Eyes - Open2";
    constexpr auto EyesOpen2ColorAttribute = "Eyes - Open2 - Color";
    constexpr auto EyesOpen3Attribute = "Eyes - Open3";
    constexpr auto EyesOpen3ColorAttribute = "Eyes - Open3 - Color";
    constexpr auto FaceOutlineAttribute = "FaceOutline";
    constexpr auto FaceOutlineColorAttribute = "FaceOutline - Color";
    constexpr auto FaceOutline2Attribute = "FaceOutline2";
    constexpr auto FaceOutline2ColorAttribute = "FaceOutline2 - Color";
    constexpr auto MouthAIAttribute = "Mouth - AI";
    constexpr auto MouthEAttribute = "Mouth - E";
    constexpr auto MouthFVAttribute = "Mouth - FV";
    constexpr auto MouthLAttribute = "Mouth - L";
    constexpr auto MouthMBPAttribute = "Mouth - MBP";
    constexpr auto MouthOAttribute = "Mouth - O";
    constexpr auto MouthUAttribute = "Mouth - U";
    constexpr auto MouthWQAttribute = "Mouth - WQ";
    constexpr auto MouthetcAttribute = "Mouth - etc";
    constexpr auto MouthretAttribute = "Mouth - rest";
    constexpr auto MouthAIColorAttribute = "Mouth - AI-Color";
    constexpr auto MouthEColorAttribute = "Mouth - E-Color";
    constexpr auto MouthFVColorAttribute = "Mouth - FV-Color";
    constexpr auto MouthLColorAttribute = "Mouth - L-Color";
    constexpr auto MouthMBPColorAttribute = "Mouth - MBP-Color";
    constexpr auto MouthOColorAttribute = "Mouth - O-Color";
    constexpr auto MouthUColorAttribute = "Mouth - U-Color";
    constexpr auto MouthWQColorAttribute = "Mouth - WQ-Color";
    constexpr auto MouthetcColorAttribute = "Mouth - etc-Color";
    constexpr auto MouthrestColorAttribute = "Mouth - rest-Color";
    constexpr auto MouthAI2Attribute = "Mouth - AI2";
    constexpr auto MouthE2Attribute = "Mouth - E2";
    constexpr auto MouthFV2Attribute = "Mouth - FV2";
    constexpr auto MouthL2Attribute = "Mouth - L2";
    constexpr auto MouthMBP2Attribute = "Mouth - MBP2";
    constexpr auto MouthO2Attribute = "Mouth - O2";
    constexpr auto MouthU2Attribute = "Mouth - U2";
    constexpr auto MouthWQ2Attribute = "Mouth - WQ2";
    constexpr auto Mouthetc2Attribute = "Mouth - etc2";
    constexpr auto Mouthret2Attribute = "Mouth - rest2";
    constexpr auto MouthAIColor2Attribute = "Mouth - AI2-Color";
    constexpr auto MouthEColor2Attribute = "Mouth - E2-Color";
    constexpr auto MouthFVColor2Attribute = "Mouth - FV2-Color";
    constexpr auto MouthLColor2Attribute = "Mouth - L2-Color";
    constexpr auto MouthMBPColor2Attribute = "Mouth - MBP2-Color";
    constexpr auto MouthOColor2Attribute = "Mouth - O2-Color";
    constexpr auto MouthUColor2Attribute = "Mouth - U2-Color";
    constexpr auto MouthWQColor2Attribute = "Mouth - WQ2-Color";
    constexpr auto MouthetcColor2Attribute = "Mouth - etc2-Color";
    constexpr auto MouthrestColor2Attribute = "Mouth - rest2-Color";

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
    constexpr auto ArchesType = "Arches";
    constexpr auto CandyCaneType = "Candy Canes";
    constexpr auto ChannelBlockType = "Channel Block";
    constexpr auto CircleType = "Circle";
    constexpr auto CubeType = "Cube";
    constexpr auto CustomType = "Custom";
    constexpr auto DmxMovingHeadAdvType = "DmxMovingHeadAdv";
    constexpr auto IciclesType = "Icicles";
    constexpr auto ImageType = "Image";
    constexpr auto MatrixType = "Matrix";
    constexpr auto SingleLineType = "Single Line";
    constexpr auto PolyLineType = "Poly Line";
    constexpr auto SphereType = "Sphere";
    constexpr auto SpinnerType = "Spinner";
    constexpr auto StarType = "Star";
    constexpr auto Tree360Type = "Tree 360";
    constexpr auto TreeFlatType = "Tree Flat";
    constexpr auto TreeRibbonType = "Tree Ribbon";
    constexpr auto WindowType = "Window Frame";
    constexpr auto WreathType = "Wreath";
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

    void AddAliases(wxXmlNode* node, const std::list<std::string> aliases) {
        wxXmlNode* aliashdr = new wxXmlNode(wxXML_ELEMENT_NODE, "Aliases");

        for (const auto& a : aliases) {
            wxXmlNode* alias = new wxXmlNode(wxXML_ELEMENT_NODE, "alias");
            alias->AddAttribute("name", a);
            aliashdr->AddChild(alias);
        }
        node->AddChild(aliashdr);
    }

    void AddSubmodels(wxXmlNode* node, const std::string name, const Model* sm) {
        const SubModel* sm0 = dynamic_cast<const SubModel*>(sm);
        wxXmlNode* submodels = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::SubModelNodeName);
        submodels->AddAttribute(XmlNodeKeys::SubModelNameAttribute, sm->GetName());
        const std::list<std::string>& aliases = sm->GetAliases();

        submodels->AddAttribute(XmlNodeKeys::LayoutAttribute, sm0->GetSubModelLayout());
        submodels->AddAttribute(XmlNodeKeys::SMTypeAttribute, sm0->GetSubModelType());
        
        const std::string submodelBufferStyle = sm0->GetSubModelBufferStyle();
        if (submodelBufferStyle == "bufferstyle") {
            submodels->AddAttribute(XmlNodeKeys::BufferStyleAttribute, submodelBufferStyle);
        } else {
            wxArrayString nodeInfo = wxSplit(sm0->GetSubModelNodeRanges(), ',');
            for (auto i = 0; i < nodeInfo.size(); i++) {
                submodels->AddAttribute("line" + std::to_string(i), nodeInfo[i]);
            }
        }
        AddAliases(submodels, aliases);
        node->AddChild(submodels);
    }

    void AddGroups(wxXmlNode* node, const std::string name, const Model* m) {
        const ModelManager& mgr = m->GetModelManager();
        std::vector<Model*> mg = mgr.GetModelGroups(m);

        for (const Model* g : mg) {
            const ModelGroup* mg1 = dynamic_cast<const ModelGroup*>(g);
            wxXmlNode* groups = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::GroupNodeName);
            groups->AddAttribute(XmlNodeKeys::mgNameAttribute, g->GetName());
            groups->AddAttribute(XmlNodeKeys::mgModelsAttribute, name);
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
            // AddAliases(groups,aliases);
            node->AddChild(groups);
        }
    }

    void Visit(const ArchesModel& arch) override {
        wxXmlNode* archNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);

        const std::vector<Model*>& submodels = arch.GetSubModels();
        const std::list<std::string>& aliases = arch.GetAliases();
        const std::string name = arch.GetName();

        const Model* m = dynamic_cast<const Model*>(&arch);

        AddBaseObjectAttributes(arch, archNode);
        AddCommonModelAttributes(arch, archNode);
        AddModelScreenLocationAttributes(arch, archNode);
        AddThreePointScreenLocationAttributes(arch, archNode);
        archNode->AddAttribute(XmlNodeKeys::ZigZagAttribute, std::to_string(arch.GetZigZag()));
        archNode->AddAttribute(XmlNodeKeys::HollowAttribute, std::to_string(arch.GetHollow()));
        archNode->AddAttribute(XmlNodeKeys::GapAttribute, std::to_string(arch.GetGap()));
        archNode->AddAttribute(XmlNodeKeys::LayerSizesAttribute, vectorToString(arch.GetLayerSizes()));
        AddAliases(archNode, aliases);
        for (Model* submodel : submodels) {
            Model* sm = arch.GetSubModel(submodel->GetName());
            AddSubmodels(archNode, name, sm);
        }
        AddGroups(archNode, name, m);
        parentNode->AddChild(archNode);
    }

    void Visit(const CandyCaneModel& cc) override {
        wxXmlNode* ccNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);

        const std::vector<Model*>& submodels = cc.GetSubModels();
        const std::list<std::string>& aliases = cc.GetAliases();
        const std::string name = cc.GetName();

        const Model* m = dynamic_cast<const Model*>(&cc);

        AddBaseObjectAttributes(cc, ccNode);
        AddCommonModelAttributes(cc, ccNode);
        AddModelScreenLocationAttributes(cc, ccNode);
        AddThreePointScreenLocationAttributes(cc, ccNode);
        AddAliases(ccNode, aliases);
        for (Model* submodel : submodels) {
            Model* sm = cc.GetSubModel(submodel->GetName());
            AddSubmodels(ccNode, name, sm);
        }
        AddGroups(ccNode, name, m);
        parentNode->AddChild(ccNode);
    }

    void Visit(const CircleModel& circle) override {
        wxXmlNode* circleNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);

        const std::vector<Model*>& submodels = circle.GetSubModels();
        const std::list<std::string>& aliases = circle.GetAliases();
        const std::string name = circle.GetName();

        const Model* m = dynamic_cast<const Model*>(&circle);

        AddBaseObjectAttributes(circle, circleNode);
        AddCommonModelAttributes(circle, circleNode);
        AddModelScreenLocationAttributes(circle, circleNode);
        // AddThreePointScreenLocationAttributes(circle, circleNode);  //export crashes, most likely an attribute that doesn't exist - need to look into
        AddAliases(circleNode, aliases);
        for (Model* submodel : submodels) {
            Model* sm = circle.GetSubModel(submodel->GetName());
            AddSubmodels(circleNode, name, sm);
        }
        AddGroups(circleNode, name, m);
        parentNode->AddChild(circleNode);
    }

    void Visit(const ChannelBlockModel& channelblock) override {
    }
    void Visit(const CubeModel& cube) override {
    }
    void Visit(const CustomModel& custom) override {
    }
    void Visit(const IciclesModel& icicles) override {
    }
    void Visit(const ImageModel& image) override {
    }
    void Visit(const MatrixModel& matricx) override {
    }
    void Visit(const SingleLineModel& singleline) override {
    }
    void Visit(const PolyLineModel& polyline) override {
    }
    void Visit(const SphereModel& sphere) override {
    }
    void Visit(const SpinnerModel& spinner) override {
    }
    void Visit(const StarModel& star) override {
    }
    void Visit(const TreeModel& tree) override {
    }
    // void Visit(const TreeModel &treeflat) override {}
    // void Visit(const TreeModel &treeribbon) override {}
    void Visit(const WindowFrameModel& window) override {
    }
    void Visit(const WreathModel& wreath) override {
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
        } else if (type == XmlNodeKeys::CircleType) {
            return DeserializeCircle(new wxXmlNode(*node), xlights);
        } else if (type == XmlNodeKeys::DmxMovingHeadAdvType) {
            return DeserializeDmxMovingHeadAdv(new wxXmlNode(*node), xlights);
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

    Model* DeserializeCircle(wxXmlNode* node, xLightsFrame* xlights) {
        Model* model;
        model = new CircleModel(node, xlights->AllModels, false);

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

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
#include "Color.h"
#include "CubeModel.h"
#include "CustomModel.h"
#include "DimmingCurve.h"
#include "GridlinesObject.h"
#include "IciclesModel.h"
#include "ImageModel.h"
#include "LayoutGroup.h"
#include "MatrixModel.h"
#include "MeshObject.h"
#include "ModelManager.h"
#include "Model.h"
#include "ModelGroup.h"
#include "PolyLineModel.h"
#include "RulerObject.h"
#include "SequenceViewManager.h"
#include "SingleLineModel.h"
#include "SphereModel.h"
#include "SpinnerModel.h"
#include "StarModel.h"
#include "SubModel.h"
#include "TerrianObject.h"
#include "ThreePointScreenLocation.h"
#include "TreeModel.h"
#include "ViewObjectManager.h"
#include "WindowFrameModel.h"
#include "WreathModel.h"
#include "xLightsVersion.h"
#include "xLightsMain.h"
#include "../Pixels.h"
#include "DMX/DmxColorAbilityCMY.h"
#include "DMX/DmxColorAbilityRGB.h"
#include "DMX/DmxColorAbilityWheel.h"
#include "DMX/DmxPresetAbility.h"
#include "DMX/DmxDimmerAbility.h"
#include "DMX/DmxShutterAbility.h"
#include "DMX/DmxMovingHeadAdv.h"
#include "DMX/DmxMovingHead.h"
#include "DMX/Mesh.h"

namespace XmlNodeKeys {
    // Model Node Names
    constexpr auto ModelsNodeName    = "models";
    constexpr auto ModelNodeName     = "model";
    constexpr auto ExportedAttribute = "exported";
    constexpr auto DimmingCurveName  = "dimmingCurve";

    // Common BaseObject Attributes
    constexpr auto NameAttribute        = "name";
    constexpr auto StateNameAttribute   = "Name";
    constexpr auto DisplayAsAttribute   = "DisplayAs";
    constexpr auto LayoutGroupAttribute = "LayoutGroup";
    constexpr auto FromBaseAttribute    = "FromBase";

    // Shared (by some) Attributes
    constexpr auto ArcAttribute            = "Arc";
    constexpr auto AlternateNodesAttribute = "AlternateNodes";
    constexpr auto BrightnessAttribute     = "Brightness";      //should fix
    constexpr auto DCBrightnessAttribute   = "brightness";      //should fix
    constexpr auto LayerSizesAttribute     = "LayerSizes";
    constexpr auto ZigZagAttribute         = "ZigZag";      //fix it
    constexpr auto CZigZagAttribute        = "zigZag";      //fix it
    constexpr auto CustomColorsAttribute   = "CustomColors";
    constexpr auto TypeAttribute           = "type";
    constexpr auto StateTypeAttribute      = "Type";
    constexpr auto CReverseAttribute       = "reverse";     //fix it
    constexpr auto ReverseAttribute        = "Reverse";     //fix it
    constexpr auto PointDataAttribute      = "PointData";

    // Common Model Attributes
    constexpr auto StartSideAttribute       = "StartSide";
    constexpr auto DirAttribute             = "Dir";
    constexpr auto Parm1Attribute           = "parm1";
    constexpr auto Parm2Attribute           = "parm2";
    constexpr auto Parm3Attribute           = "parm3";
    constexpr auto AdvancedAttribute        = "Advanced";
    constexpr auto AntialiasAttribute       = "Antialias";
    constexpr auto PixelSizeAttribute       = "PixelSize";
    constexpr auto StringTypeAttribute      = "StringType";
    constexpr auto RGBWHandleAttribute      = "RGBWHandling";
    constexpr auto TransparencyAttribute    = "Transparency";
    constexpr auto BTransparencyAttribute   = "BlackTransparency";
    constexpr auto StartChannelAttribute    = "StartChannel";
    constexpr auto NodeNamesAttribute       = "NodeNames";
    constexpr auto StrandNamesAttribute     = "StrandNames";
    constexpr auto ShadowModelAttribute     = "ShadowModelFor";
    constexpr auto ControllerAttribute      = "Controller";
    constexpr auto xlightsVersionAttr       = "SourceVersion";
    constexpr auto versionNumberAttribute   = "versionNumber";
    constexpr auto ActiveAttribute          = "Active";
    constexpr auto FromBaaseAttribute       = "FromBase";
    constexpr auto DescriptionAttribute     = "Description";
    constexpr auto CustomStringsAttribute   = "String";
    constexpr auto TagColourAttribute       = "TagColour";
    constexpr auto XLVersionAttribute       = "xLightsVersion";
    constexpr auto SettingsAttribute        = "settings";
    constexpr auto SerializeAttribute       = "Serialize";
    constexpr auto CustomColorAttribute     = "CustomColor";
    constexpr auto ModelBrightnessAttribute = "ModelBrightness";
    constexpr auto LowDefinitionAttribute   = "LowDefinition";
    constexpr auto ModelChainAttribute      = "ModelChain";
    constexpr auto AliasesAttribute         = "Aliases";
    constexpr auto AliasNodeName            = "alias";


    // Common SubModel Attributes
    constexpr auto SubModelNodeName        = "subModel";
    constexpr auto LayoutAttribute         = "layout";
    constexpr auto SMTypeAttribute         = "type";
    constexpr auto BufferStyleAttribute    = "bufferstyle";
    constexpr auto SubBufferAttribute      = "subBuffer";
    constexpr auto LineAttribute           = "line";

    // ModelGroup
    constexpr auto GroupNodeName            = "modelGroup";
    constexpr auto mgSelectedAttribute      = "selected";
    constexpr auto mgGridSizeAttribute      = "GridSize";
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

    // Dimensions Attributes
    constexpr auto DimNodeName        = "dimensions";
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

    // DMX Moving Head Attributes
    constexpr auto DmxColorTypeAttribute   = "DmxColorType";
    constexpr auto DmxBeamYOffsetAttribute = "DmxBeamYOffset";
    constexpr auto DmxBeamLengthAttribute  = "DmxBeamLength";
    constexpr auto DmxBeamWidthAttribute   = "DmxBeamWidth";    //old MH model
    constexpr auto DmxStyleAttribute       = "DmxStyle";        // old MH model
    constexpr auto HideBodyAttribute       = "HideBody";        // old MH model

    // DmxColorAbilityRGB Attributes
    constexpr auto DmxRedChannelAttribute   = "DmxRedChannel";
    constexpr auto DmxGreenChannelAttribute = "DmxGreenChannel";
    constexpr auto DmxBlueChannelAttribute  = "DmxBlueChannel";
    constexpr auto DmxWhiteChannelAttribute = "DmxWhiteChannel";

    // DmxColorAbilityWheel Attributes
    constexpr auto DmxColorWheelChannelAttribute = "DmxColorWheelChannel";
    constexpr auto DmxDimmerChannelAttribute     = "DmxDimmerChannel";
    constexpr auto DmxColorWheelColorAttribute   = "DmxColorWheelColor";
    constexpr auto DmxColorWheelDelayAttribute   = "DmxColorWheelDelay";
    constexpr auto DmxColorWheelDMXAttribute     = "DmxColorWheelDMX";

    // DmxPresetAbility Attributes
    constexpr auto DmxPresetChannelAttribute = "DmxPresetChannel";
    constexpr auto DmxPresetValueAttribute   = "DmxPresetValue";
    constexpr auto DmxPresetDescAttribute    = "DmxPresetDesc";

    // DmxShutterAbility Attributes
    constexpr auto DmxShutterChannelAttribute = "DmxShutterChannel";
    constexpr auto DmxShutterOpenAttribute    = "DmxShutterOpen";
    constexpr auto DmxShutterOnValueAttribute = "DmxShutterOnValue";

    // DmxDimmerAbility Attributes
    constexpr auto MhDimmerChannelAttribute = "MhDimmerChannel";

    // DmxColorAbilityCMY Attributes
    constexpr auto DmxCyanChannelAttribute    = "DmxCyanChannel";
    constexpr auto DmxMagentaChannelAttribute = "DmxMagentaChannel";
    constexpr auto DmxYellowChannelAttribute  = "DmxYellowChannel";

    // DmxMovingHeadComm Attributes
    constexpr auto DmxFixturelAttribute = "DmxFixture";

    // DmxMotor Attributes
    constexpr auto ChannelCoarseAttribute = "ChannelCoarse";
    constexpr auto ChannelFineAttribute   = "ChannelFine";
    constexpr auto MinLimitAttribute      = "MinLimit";
    constexpr auto MaxLimitAttribute      = "MaxLimit";
    constexpr auto RangeOfMotionAttribute = "RangeOfMotion";
    constexpr auto OrientZeroAttribute    = "OrientZero";
    constexpr auto OrientHomeAttribute    = "OrientHome";
    constexpr auto SlewLimitAttribute     = "SlewLimit";
    constexpr auto UpsideDownAttribute    = "UpsideDown";

    // Servo Model
    //  TBC

    // Controller Attributes
    constexpr auto CtrlConnectionName       = "ControllerConnection";
    constexpr auto ProtocolAttribute        = "Protocol";
    constexpr auto ProtocolSpeedAttribute   = "Speed";
    constexpr auto PortAttribute            = "Port";
    constexpr auto StartNullAttribute       = "nullNodes";
    constexpr auto EndNullAttribute         = "endNullNodes";
    constexpr auto GammaAttribute           = "gamma";
    constexpr auto ColorOrderAttribute      = "colorOrder";
    constexpr auto GroupCountAttribute      = "groupCount";

    // Smart Remote Attributes
    constexpr auto SmartRemoteAttribute     = "SmartRemote";
    constexpr auto SmartRemoteTypeAttribute = "SmartRemoteType";
    constexpr auto SRCascadeOnPortAttribute = "SRCascadeOnPort";
    constexpr auto SRMaxCascadeAttribute    = "SRMaxCascade";
    constexpr auto SmartRemoteTsAttribute   = "ts";

    // Mesh Attributes
    constexpr auto ObjFileAttribute    = "ObjFile";
    constexpr auto MeshOnlyAttribute   = "MeshOnly";

    // Arch Attributes
    constexpr auto HollowAttribute = "Hollow";
    constexpr auto GapAttribute    = "Gap";
    constexpr auto ArchesSkewAttribute = "ArchesSkew";

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
    constexpr auto StrandsAttribute      = "Strands";
    constexpr auto NodesAttribute        = "Nodes";
    constexpr auto PixelTypeAttribute    = "PixelType";
    constexpr auto PixelSpacingAttribute = "PixelSpacing";
    constexpr auto PixelAttribute        = "Pixel";
    constexpr auto BkgLightnessAttribute = "CustomBkgLightness";
    constexpr auto BkgImageAttribute     = "CustomBkgImage";
    constexpr auto BkgAttribute          = "Bkg";
    constexpr auto CMDepthAttribute      = "Depth";

    // Dimming Curves
    constexpr auto DimmingNodeName     = "dimmingCurve";

    // Image Model
    constexpr auto ImageAttribute = "Image";
    constexpr auto BlackAttribute = "Black";
    constexpr auto ChainAttribute = "Chain";

    // Icicles Model
    constexpr auto DropPatternAttribute = "DropPattern";

    // Matrix
    constexpr auto NoZigZagAttribute      = "NoZig";

    // Poly Line Model
    constexpr auto NumPointsAttribute    = "NumPoints";
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

    //Layout Groups
    constexpr auto BackgroundImageAttribute      = "backgroundImage";
    constexpr auto BackgroundBrightnessAttribute = "backgroundBrightness";
    constexpr auto BackgroundAlphaAttribute      = "backgroundAlpha";
    constexpr auto ScaleImageAttribute           = "scaleImage";
     
    // View_Object
    constexpr auto GridLineSpacingAttribute = "GridLineSpacing";
    constexpr auto GridWidthAttribute       = "GridWidth";
    constexpr auto GridHeightAttribute      = "GridHeight";
    constexpr auto UnitsAttribute           = "Units";
    constexpr auto LengthAttribute          = "Length";
    constexpr auto TerrainLineAttribute     = "TerrianLineSpacing";     //fix spelling in v8
    constexpr auto TerrainWidthAttribute    = "TerrianWidth";           //fix spelling in v8
    constexpr auto TerrainDepthAttribute    = "TerrianDepth";           //fix spelling in v8
    constexpr auto TerrainBrushAttribute    = "TerrianBrushSize";       //fix spelling in v8
    constexpr auto GridColorAttribute       = "gridColor";
    constexpr auto HideGridAttribute        = "HideGrid";
    constexpr auto HideImageAttribute       = "HideImage";
    constexpr auto ObjAttribute             = "ObjFile";

    //Settings
    constexpr auto ValueAttribute = "value";

    //Colors
    //Already does it in ColorManager.cpp ColorManager::Save()
    constexpr auto RedAttribute   = "Red";
    constexpr auto GreenAttribute = "Green";
    constexpr auto BlueAttribute  = "Blue";

    // Effect Version
    constexpr auto VersionAttribute = "version";

    // Previews
    constexpr auto ModelsAttribute = "models";

    // Group - already done

    // Perspectives
    constexpr auto CurrentAttribute  = "current";

    // ViewPoint
    constexpr auto posXAttribute     = "posX";
    constexpr auto posYAttribute     = "posY";
    constexpr auto posZAttribute     = "posZ";
    constexpr auto angleXAttribute   = "angleX";
    constexpr auto angleYAttribute   = "angleY";
    constexpr auto angleZAttribute   = "angleZ";
    constexpr auto distanceAttribute = "distance";
    constexpr auto zoomAttribute     = "zoom";
    constexpr auto panXAttribute     = "panx";
    constexpr auto panYAttribute     = "pany";
    constexpr auto panZAttribute     = "panz";
    constexpr auto zoomXAttribute    = "zoom_corrx";
    constexpr auto zoomYAttribute    = "zoom_corry";
    constexpr auto is3DAttribute     = "is_ed";

    // Model Types
    constexpr auto ArchesType           = "Arches";
    constexpr auto CandyCaneType        = "Candy Canes";
    constexpr auto ChannelBlockType     = "Channel Block";
    constexpr auto CircleType           = "Circle";
    constexpr auto CubeType             = "Cube";
    constexpr auto CustomType           = "Custom";
    constexpr auto DmxMovingHeadType    = "DmxMovingHead";
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

    //Extra Types
    constexpr auto ViewObjectsType  = "view_objects";
    constexpr auto EffectsType      = "effects";
    constexpr auto ViewsType        = "views";
    constexpr auto PalettesType     = "palettes";
    constexpr auto LayoutGroupsType = "layoutGroups";
    constexpr auto PerspectivesType = "perspectives";
    constexpr auto SettingsType     = "settings";
    constexpr auto ColorsType       = "colors";
    constexpr auto ViewPointsType   = "Viewpoints";
}; // end namespace XmlNodeKeys

namespace XmlSerialize {

static void DeserializeFaceInfo(wxXmlNode* f, FaceStateData & faceInfo) {
    std::string name = f->GetAttribute(XmlNodeKeys::StateNameAttribute, "SingleNode").ToStdString();
    std::string type = f->GetAttribute(XmlNodeKeys::StateTypeAttribute, "SingleNode").ToStdString();
    if (name == xlEMPTY_STRING) {
        name = type;
    }
    if (!(type == "SingleNode" || type == "NodeRange" || type == "Matrix")) {
        if (type == "Coro") {
            type = "SingleNode";
        } else {
            type = "Matrix";
        }
    }
    wxXmlAttribute* att = f->GetAttributes();
    while (att != nullptr) {
        if (att->GetName() != XmlNodeKeys::StateNameAttribute) {
            if (att->GetName().Left(5) == "Mouth" || att->GetName().Left(4) == "Eyes") {
                if (type == XmlNodeKeys::MatrixType) {
                    faceInfo[name][att->GetName().ToStdString()] = FixFile("", att->GetValue());
                    if (att->GetValue() != faceInfo[name][att->GetName().ToStdString()])
                        att->SetValue(faceInfo[name][att->GetName().ToStdString()]);
                } else {
                    faceInfo[name][att->GetName().ToStdString()] = att->GetValue();
                }
            } else {
                faceInfo[name][att->GetName().ToStdString()] = att->GetValue();
            }
        }
        att = att->GetNext();
    }
}

static void DeserializeStateInfo(wxXmlNode* f, FaceStateData & stateInfo) {
    std::string name = f->GetAttribute(XmlNodeKeys::StateNameAttribute, "SingleNode").ToStdString();
    std::string type = f->GetAttribute(XmlNodeKeys::StateTypeAttribute, "SingleNode").ToStdString();
    if (name == "") {
        name = type;
    }
    if (!(type == "SingleNode" || type == "NodeRange")) {
        if (type == "Coro") {
            type = "SingleNode";
        }
    }
    wxXmlAttribute* att = f->GetAttributes();
    while (att != nullptr) {
        if (att->GetName() != "Name") {
            if (att->GetValue() != "") { // we only save non default values to keep xml file small
                std::string key = att->GetName().ToStdString();
                std::string value = att->GetValue().ToStdString();
                std::string storedKey = key;
                if (key.find('s') == 0) { // Handle all keys starting with 's'
                    size_t sPos = key.find('s');
                    size_t dashPos = key.find('-');
                    size_t endPos = (dashPos != std::string::npos) ? dashPos : key.length();
                    if (sPos == 0 && sPos + 1 < endPos) {
                        std::string numStr = key.substr(sPos + 1, endPos - sPos - 1);
                        int num = std::stoi(numStr);
                        std::string paddedNum = wxString::Format("%03d", num).ToStdString();
                        storedKey = "s" + paddedNum + (dashPos != std::string::npos ? key.substr(dashPos) : "");
                    }
                }
                stateInfo[name][storedKey] = value;
            }
        }
        att = att->GetNext();
    }
}
} // end namespace XmlSerialize

struct XmlSerializingVisitor : BaseObjectVisitor {
    XmlSerializingVisitor(wxXmlNode* parentNode) :
        parentNode(parentNode) {
    }

    wxXmlNode* parentNode;

    void AddBaseObjectAttributes(const BaseObject& base, wxXmlNode* node) {
        node->AddAttribute(XmlNodeKeys::NameAttribute, base.GetName());
        node->AddAttribute(XmlNodeKeys::DisplayAsAttribute, base.GetDisplayAs());
        node->AddAttribute(XmlNodeKeys::LayoutGroupAttribute, base.GetLayoutGroup());
        node->AddAttribute(XmlNodeKeys::ActiveAttribute, std::to_string(base.IsActive()));
    }

    void AddCommonModelAttributes(const Model& model, wxXmlNode* node) {
        node->AddAttribute(XmlNodeKeys::StartSideAttribute, model.GetStartSide());
        node->AddAttribute(XmlNodeKeys::DirAttribute, model.GetDirection());
        node->AddAttribute(XmlNodeKeys::Parm1Attribute, std::to_string(model.GetParm1()));
        node->AddAttribute(XmlNodeKeys::Parm2Attribute, std::to_string(model.GetParm2()));
        node->AddAttribute(XmlNodeKeys::Parm3Attribute, std::to_string(model.GetParm3()));
        node->AddAttribute(XmlNodeKeys::AntialiasAttribute, std::to_string((long)model.GetPixelStyle()));
        node->AddAttribute(XmlNodeKeys::PixelSizeAttribute, std::to_string(model.GetPixelSize()));
        node->AddAttribute(XmlNodeKeys::RGBWHandleAttribute, model.GetRGBWHandling());
        node->AddAttribute(XmlNodeKeys::StringTypeAttribute, model.GetStringType());
        node->AddAttribute(XmlNodeKeys::TransparencyAttribute, std::to_string(model.GetTransparency()));
        node->AddAttribute(XmlNodeKeys::BTransparencyAttribute, std::to_string(model.GetBlackTransparency()));
        node->AddAttribute(XmlNodeKeys::DescriptionAttribute, model.GetDescription());
        node->AddAttribute(XmlNodeKeys::TagColourAttribute, model.GetTagColourAsString());
        node->AddAttribute(XmlNodeKeys::StartChannelAttribute, model.GetModelStartChannel());
        node->AddAttribute(XmlNodeKeys::NodeNamesAttribute, model.GetNodeNames());
        node->AddAttribute(XmlNodeKeys::StrandNamesAttribute, model.GetStrandNames());
        node->AddAttribute(XmlNodeKeys::ControllerAttribute, model.GetControllerName());
        node->AddAttribute(XmlNodeKeys::versionNumberAttribute, CUR_MODEL_POS_VER);
        node->AddAttribute(XmlNodeKeys::xlightsVersionAttr, xlights_version_string);
        node->AddAttribute(XmlNodeKeys::SerializeAttribute, "1");
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

    void AddTwoPointScreenLocationAttributes(const BaseObject& base, wxXmlNode* node) {
        const TwoPointScreenLocation& screenLoc = dynamic_cast<const TwoPointScreenLocation&>(base.GetBaseObjectScreenLocation());
        float x2 = screenLoc.GetX2();
        float y2 = screenLoc.GetY2();
        float z2 = screenLoc.GetZ2();
        node->AddAttribute(XmlNodeKeys::X2Attribute, std::to_string(x2));
        node->AddAttribute(XmlNodeKeys::Y2Attribute, std::to_string(y2));
        node->AddAttribute(XmlNodeKeys::Z2Attribute, std::to_string(z2));
    }

    void AddThreePointScreenLocationAttributes(const BaseObject& base, wxXmlNode* node) {
        AddTwoPointScreenLocationAttributes( base, node );
        const ThreePointScreenLocation& screenLoc = dynamic_cast<const ThreePointScreenLocation&>(base.GetBaseObjectScreenLocation());
        int angle = screenLoc.GetAngle();
        node->AddAttribute(XmlNodeKeys::AngleAttribute, std::to_string(angle));
        float shear = screenLoc.GetYShear();
        node->AddAttribute(XmlNodeKeys::ShearAttribute, std::to_string(shear));
        float height = screenLoc.GetMHeight();
        node->AddAttribute(XmlNodeKeys::HeightAttribute, std::to_string(height));
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
        node->AddAttribute(XmlNodeKeys::DmxColorWheelDelayAttribute, std::to_string(colors->GetWheelDelay()));
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

    void SortAttributes(wxXmlNode* input) {
        const wxString attributeToPrioritize = "name";
        const wxString attributeToDePrioritize = "CustomModel";
        std::vector<std::pair<std::string, std::string>> attributes;
        for (wxXmlAttribute* attr = input->GetAttributes(); attr != nullptr; attr = attr->GetNext()) {
            attributes.push_back({ std::string(attr->GetName()), std::string(attr->GetValue()) });
        }
        auto custom_comparator = [&attributeToPrioritize, &attributeToDePrioritize](const std::pair<wxString, wxString>& a, const std::pair<wxString, wxString>& b) {
            if (a.first == b.first) return false;
            if (a.first == attributeToPrioritize) return true;
            if (b.first == attributeToPrioritize) return false;
            if (a.first.Contains(attributeToDePrioritize)) return false;
            if (b.first.Contains(attributeToDePrioritize)) return true;
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
            xmlNode->AddAttribute(XmlNodeKeys::StateNameAttribute, f.first);
            for (const auto& f2 : f.second) {
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
                if (s2.first != "") xmlNode->AddAttribute(s2.first, s2.second);
            }
            SortAttributes(xmlNode);
            node->AddChild(xmlNode);
        }
    }

    void AddAliases(wxXmlNode* node, const std::list<std::string>& aliases) {
        wxXmlNode* aliashdr = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::AliasesAttribute);

        if (aliases.size() > 0) {
            for (const auto& a : aliases) {
                wxXmlNode* alias = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::AliasNodeName);
                alias->AddAttribute(XmlNodeKeys::NameAttribute, a);
                aliashdr->AddChild(alias);
            }
            node->AddChild(aliashdr);
        }
    }

    void AddDimmingCurve(wxXmlNode* node, const Model* m) {
        std::map<std::string, std::map<std::string, std::string>> dcInfo = m->GetDimmingInfo();
        if (dcInfo.size() != 0) {
            wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::DimmingNodeName);

            for (const auto& d1 : dcInfo) {
                wxXmlNode* dc = new wxXmlNode(wxXML_ELEMENT_NODE, d1.first);
                for (const auto& d2 : d1.second) {
                    dc->AddAttribute(d2.first, d2.second);
                    xmlNode->AddChild(dc);
                }
            }
            node->AddChild(xmlNode);
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
            
            // FIXME: Just want to get it compiling
            /*if (submodelType == "subbuffer") {
             submodels->AddAttribute(XmlNodeKeys::SubBufferStyleAttribute, submodel->GetSubModelNodeRanges());
             } else {
             wxArrayString nodeInfo = wxSplit(submodel->GetSubModelNodeRanges(), ',');
             for (auto i = 0; i < nodeInfo.size(); i++) {
             submodels->AddAttribute("line" + std::to_string(i), nodeInfo[i]);
             }
             }*/
            SortAttributes(submodels);
            AddAliases(submodels, s->GetAliases());
            node->AddChild(submodels);
        }
        /*wxXmlNode * root = m->GetModelXml();
         wxXmlNode * child = root->GetChildren();
         std::vector<std::pair<wxString, wxString>> submodelAliases;
         while (child != nullptr) {
         if (child->GetName() == "subModel") {
         for (auto node = child->GetChildren(); node != nullptr; node = node->GetNext()) {
         if (node->GetName() == "Aliases") {
         for (auto a = node->GetChildren(); a != nullptr; a = a->GetNext()) {
         submodelAliases.push_back(std::make_pair(child->GetAttribute("name"), a->GetAttribute("name")));
         }
         }
         }
         wxXmlNode *n = child;
         child = child->GetNext();
         root->RemoveChild(n);
         delete n;
         } else {
         child = child->GetNext();
         }
         }
         
         for (auto a = _subModels.begin(); a != _subModels.end(); ++a) {
         child = new wxXmlNode(wxXML_ELEMENT_NODE, "subModel");
         child->AddAttribute("name", (*a)->name);
         child->AddAttribute("layout", (*a)->vertical ? "vertical" : "horizontal");
         child->AddAttribute("type", (*a)->isRanges ? "ranges" : "subbuffer");
         child->AddAttribute("bufferstyle", (*a)->bufferStyle);
         
         auto aliases = new wxXmlNode(wxXmlNodeType::wxXML_ELEMENT_NODE, "Aliases");
         for (const auto& entry : submodelAliases) {
         if (entry.first == (*a)->name) {
         auto n = new wxXmlNode(wxXmlNodeType::wxXML_ELEMENT_NODE, "alias");
         n->AddAttribute("name", entry.second.Lower());
         aliases->AddChild(n);
         }
         }
         child->AddChild(aliases);
         
         // If the submodel name has changed ... we need to rename the model
         if ((*a)->oldName != (*a)->name)
         {
         xlights->RenameModel(m->GetName() + std::string("/") + (*a)->oldName.ToStdString(), m->GetName() + std::string("/") + (*a)->name.ToStdString());
         }
         
         if ((*a)->isRanges) {
         for (int x = 0; x < (*a)->strands.size(); x++) {
         child->AddAttribute(wxString::Format("line%d", x), (*a)->strands[x]);
         }
         } else {
         child->AddAttribute("subBuffer", (*a)->subBuffer);
         }
         root->AddChild(child);
         }
         
         std::vector<std::string> submodelOrder;
         for (auto it = _subModels.begin(); it != _subModels.end(); ++it)
         {
         submodelOrder.push_back((*it)->name);
         }
         
         xlights->EnsureSequenceElementsAreOrderedCorrectly(m->GetName(), submodelOrder);
         }*/
    }

    void AddGroups(wxXmlNode* node, const Model* m) {
        const ModelManager& mgr = m->GetModelManager();
        std::vector<Model*> mg = mgr.GetModelGroups(m);

        for (const Model* g : mg) {
            const ModelGroup* mg1 = dynamic_cast<const ModelGroup*>(g);
            if (mg1 == nullptr) return;
            wxXmlNode* groups = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::GroupNodeName);
            groups->AddAttribute(XmlNodeKeys::NameAttribute, g->GetName());
            groups->AddAttribute(XmlNodeKeys::mgModelsAttribute, m->GetName());
            groups->AddAttribute(XmlNodeKeys::LayoutGroupAttribute, g->GetLayoutGroup());
            groups->AddAttribute(XmlNodeKeys::mgSelectedAttribute, std::to_string(mg1->IsSelected()));
            groups->AddAttribute(XmlNodeKeys::LayoutAttribute, mg1->GetLayout());
            groups->AddAttribute(XmlNodeKeys::mgGridSizeAttribute, std::to_string(mg1->GetGridSize()));
            groups->AddAttribute(XmlNodeKeys::TagColourAttribute, mg1->GetTagColourAsString());
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

    void AddControllerConnection(wxXmlNode* node, const Model* m) {
        auto const& cc = m->GetConstCtrlConn();
        int p = cc.GetPort();
        if (p != 0) {
            wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::CtrlConnectionName);
            xmlNode->AddAttribute(XmlNodeKeys::ControllerAttribute, m->GetControllerName());
            xmlNode->AddAttribute(XmlNodeKeys::ProtocolAttribute, m->GetControllerProtocol());
            xmlNode->AddAttribute(XmlNodeKeys::ProtocolSpeedAttribute, std::to_string(m->GetControllerProtocolSpeed()));

            // Save all the property checkbox active states
            if (cc.IsPropertySet(CtrlProps::USE_SMART_REMOTE)) xmlNode->AddAttribute(XmlNodeKeys::SmartRemoteAttribute, std::to_string(cc.GetSmartRemote()));
            if (cc.IsPropertySet(CtrlProps::START_NULLS_ACTIVE)) xmlNode->AddAttribute(XmlNodeKeys::StartNullAttribute, std::to_string(cc.GetStartNulls()));
            if (cc.IsPropertySet(CtrlProps::END_NULLS_ACTIVE)) xmlNode->AddAttribute(XmlNodeKeys::EndNullAttribute, std::to_string(cc.GetEndNulls()));
            if (cc.IsPropertySet(CtrlProps::BRIGHTNESS_ACTIVE)) xmlNode->AddAttribute(XmlNodeKeys::BrightnessAttribute, std::to_string(cc.GetBrightness()));
            if (cc.IsPropertySet(CtrlProps::GAMMA_ACTIVE)) xmlNode->AddAttribute(XmlNodeKeys::GammaAttribute, std::to_string(cc.GetGamma()));
            if (cc.IsPropertySet(CtrlProps::COLOR_ORDER_ACTIVE)) xmlNode->AddAttribute(XmlNodeKeys::ColorOrderAttribute, cc.GetColorOrder());
            if (cc.IsPropertySet(CtrlProps::REVERSE_ACTIVE)) xmlNode->AddAttribute(XmlNodeKeys::CReverseAttribute, std::to_string(cc.GetReverse()));
            if (cc.IsPropertySet(CtrlProps::GROUP_COUNT_ACTIVE)) xmlNode->AddAttribute(XmlNodeKeys::GroupCountAttribute, std::to_string(cc.GetGroupCount()));
            if (cc.IsPropertySet(CtrlProps::ZIG_ZAG_ACTIVE)) xmlNode->AddAttribute(XmlNodeKeys::CZigZagAttribute, std::to_string(cc.GetZigZag()));
            if (cc.IsPropertySet(CtrlProps::TS_ACTIVE)) xmlNode->AddAttribute(XmlNodeKeys::SmartRemoteTsAttribute, std::to_string(cc.GetSmartTs()));

            // Set all the Smart Remote values
            xmlNode->AddAttribute(XmlNodeKeys::SRMaxCascadeAttribute, std::to_string(m->GetSRMaxCascade()));
            xmlNode->AddAttribute(XmlNodeKeys::SRCascadeOnPortAttribute, std::to_string(m->GetSRMaxCascade()));
            xmlNode->AddAttribute(XmlNodeKeys::SmartRemoteTsAttribute, std::to_string(m->GetSmartTs()));
            xmlNode->AddAttribute(XmlNodeKeys::SmartRemoteTypeAttribute, m->GetSmartRemoteType());

            node->AddChild(xmlNode);
        }
    }

    void AddDimensions(wxXmlNode* node, const Model* m) {
        std::string rdu = m->GetRulerDim();
        if (rdu != "") {
            wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::DimNodeName);
            xmlNode->AddAttribute(XmlNodeKeys::DimDepthAttribute, std::to_string(m->GetModelScreenLocation().GetRealDepth()));
            xmlNode->AddAttribute(XmlNodeKeys::DimHeightAttribute, std::to_string(m->GetModelScreenLocation().GetRealHeight()));
            xmlNode->AddAttribute(XmlNodeKeys::DimUnitsAttribute, rdu);
            xmlNode->AddAttribute(XmlNodeKeys::DimWidthAttribute, std::to_string(m->GetModelScreenLocation().GetRealWidth()));
            node->AddChild(xmlNode);
        }
    }

    void AddOtherElements(wxXmlNode* xmlNode, const Model* m)
    {
        SortAttributes(xmlNode);
        AddAliases(xmlNode, m->GetAliases());
        AddDimmingCurve(xmlNode,m);
        AddFacesandStates(xmlNode, m);
        AddSubmodels(xmlNode, m);
        AddGroups(xmlNode, m);
        AddControllerConnection(xmlNode, m);
        AddDimensions(xmlNode, m);
        parentNode->AddChild(xmlNode);
    }
    
    void AddCustomModel(wxXmlNode* xmlNode, const CustomModel& m) {
        /*std::vector<std::vector<std::vector<int>>> locations = m.GetLocations();
        std::vector<std::vector<int>> customModel;
        std::string type1 = "";
        std::string type2 = "";
        bool hasInfo = false;
        uint16_t maxNode = 0;
        auto custom_comparator = [](const std::vector<int>& a, const std::vector<int>& b) { return a[2] < b[2]; };
        for (auto l = 0; l < locations.size(); l++) {
            type2 += (l > 0 ? "|" : "");
            hasInfo = false;
            bool newRow = true;
            for (auto h = 0; h < locations[l].size(); h++) {
                for (auto w = 0; w < locations[l][h].size(); w++) {
                    if (locations[l][h][w] != -1) {
                        type2 +=  (hasInfo && !newRow ? "," : "") + std::to_string(h) + "," + std::to_string(w) + "," + std::to_string(locations[l][h][w]);
                        if (locations[l][h][w] > maxNode) maxNode = locations[l][h][w];
                        newRow = false;
                        hasInfo = true;
                        if (m.IsAllNodesUnique()) {
                            std::vector<int> row;
                            row.push_back(h);
                            row.push_back(w);
                            row.push_back(locations[l][h][w]);
                            customModel.push_back(row);
                        }
                    }
                }
            }
        }
        std::string stats = std::to_string(maxNode) + "," + std::to_string(m.GetCustomWidth()) + "," + std::to_string(m.GetCustomHeight()) + "," + std::to_string(m.GetCustomDepth());
        if (m.IsAllNodesUnique() && locations.size() == 1) {
            std::sort(customModel.begin(), customModel.end(), custom_comparator);
            if (customModel.back()[2] == customModel.size()) {
                for (auto r = 0; r < customModel.size(); r++)
                    type1 += (r > 0 ? "," : "") + std::to_string(customModel[r][0]) + "," + std::to_string(customModel[r][1]);
                xmlNode->AddAttribute("CustomModel2.0", "1," + stats + ",0,0|" + type1);
            } else {
                xmlNode->AddAttribute("CustomModel2.0", "2," + stats + ",0,0|" + type2);
            }
        } else {
            xmlNode->AddAttribute("CustomModel2.0", "2," + stats + ",0,0|" + type2);
        }*/
    }

    void SerializeSuperStrings(Model const& model, wxXmlNode* node) {
        int num_colors = model.GetNumSuperStringColours();
        if (num_colors == 0) return;
        for (int i = 0; i < num_colors; ++i) {
            std::string key = std::format("SuperStringColour{}",i);
            node->AddAttribute(key, model.GetSuperStringColour(i));
        }
    }

    void SerializeViewsObject(wxXmlNode* node, xLightsFrame* xlights) {
        wxXmlNode* viewsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "views");
        SequenceViewManager* seqViewMgr = xlights->GetViewsManager();
        std::list<SequenceView*> views = seqViewMgr->GetViews();
        for (SequenceView* view : views) {
            std::string name = view->GetName();
            if (name != "Master View") {
                wxXmlNode* viewChild = new wxXmlNode(wxXML_ELEMENT_NODE, "view");
                viewChild->AddAttribute("name", name);
                viewChild->AddAttribute(XmlNodeKeys::ModelsAttribute, view->GetModelsString());
                viewsNode->AddChild(viewChild);
            }
        }
        node->AddChild(viewsNode);
    }

    void SerializeColorsObject(wxXmlNode* node, xLightsFrame* xlights) {
        wxXmlNode* colorsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "colors");
        ColorManager* colorMgr = new ColorManager(xlights);
        std::map<std::string, xlColor> colors = colorMgr->GetColors();
        for (const auto& c : colors) {
            wxXmlNode* colorChild = new wxXmlNode(wxXML_ELEMENT_NODE, c.first);
            colorChild->AddAttribute(XmlNodeKeys::RedAttribute, std::to_string(c.second.red));
            colorChild->AddAttribute(XmlNodeKeys::GreenAttribute, std::to_string(c.second.green));
            colorChild->AddAttribute(XmlNodeKeys::BlueAttribute, std::to_string(c.second.blue));
            colorsNode->AddChild(colorChild);
        }
        node->AddChild(colorsNode);
    }

    void SerializeLayoutGroupsObject(wxXmlNode* node, xLightsFrame* xlights) {
        std::vector<LayoutGroup*> layoutGroups = xlights->LayoutGroups;
        wxXmlNode* lgNode = new wxXmlNode(wxXML_ELEMENT_NODE, "layoutGroups");
        for (LayoutGroup* lg : layoutGroups) {
            wxXmlNode* lgChild = new wxXmlNode(wxXML_ELEMENT_NODE, "layoutGroup");
            lgChild->AddAttribute("name", lg->GetName());
            lgChild->AddAttribute(XmlNodeKeys::BackgroundImageAttribute, lg->GetBackgroundImage());
            lgChild->AddAttribute(XmlNodeKeys::BackgroundBrightnessAttribute, std::to_string(lg->GetBackgroundBrightness()));
            lgChild->AddAttribute(XmlNodeKeys::BackgroundAlphaAttribute, std::to_string(lg->GetBackgroundAlpha()));
            lgChild->AddAttribute(XmlNodeKeys::ScaleImageAttribute, std::to_string(lg->GetBackgroundScaled()));
            lgNode->AddChild(lgChild);
        }
        node->AddChild(lgNode);
    }

    void SerializePerspectivesObject(wxXmlNode* node, xLightsFrame* xlights) {
        std::list<std::string> perspectives = xlights->GetPerspectives();
        wxXmlNode* perspectivesNode = new wxXmlNode(wxXML_ELEMENT_NODE, "perspectives");
        for (std::string p : perspectives) {
            wxXmlNode* pChild = new wxXmlNode(wxXML_ELEMENT_NODE, "perspective");
            pChild->AddAttribute("name", p);
            perspectivesNode->AddChild(pChild);
        }
        node->AddChild(perspectivesNode);
    }

    void SerializeSettingsObject(wxXmlNode* node, xLightsFrame* xlights) {
        wxXmlNode* settings = new wxXmlNode(wxXML_ELEMENT_NODE, "settings");
        wxXmlNode* scaleimage = new wxXmlNode(wxXML_ELEMENT_NODE, "scaleImage");
        scaleimage->AddAttribute("value", std::to_string(xlights->GetDefaultPreviewBackgroundScaled()));
        settings->AddChild(scaleimage);
        wxXmlNode* bkgimage = new wxXmlNode(wxXML_ELEMENT_NODE, "backgroundImage");
        bkgimage->AddAttribute("value", "tbd");
        settings->AddChild(bkgimage);
        wxXmlNode* bkgbright = new wxXmlNode(wxXML_ELEMENT_NODE, "backgroundBrightness");
        bkgbright->AddAttribute("value", std::to_string(xlights->GetDefaultPreviewBackgroundBrightness()));
        settings->AddChild(bkgbright);
        wxXmlNode* bkgalpha = new wxXmlNode(wxXML_ELEMENT_NODE, "backgroundAlpha");
        bkgalpha->AddAttribute("value", std::to_string(xlights->GetDefaultPreviewBackgroundAlpha()));
        settings->AddChild(bkgalpha);
        wxXmlNode* boundbox = new wxXmlNode(wxXML_ELEMENT_NODE, "Display2DBoundingBox");
        boundbox->AddAttribute("value", std::to_string(xlights->GetDisplay2DBoundingBox()));
        settings->AddChild(boundbox);
        wxXmlNode* grid = new wxXmlNode(wxXML_ELEMENT_NODE, "Display2DGrid");
        grid->AddAttribute("value", std::to_string(xlights->GetDisplay2DGrid()));
        settings->AddChild(grid);
        wxXmlNode* gridspace = new wxXmlNode(wxXML_ELEMENT_NODE, "Display2DGridSpacing");
        gridspace->AddAttribute("value", std::to_string(xlights->GetDisplay2DGridSpacing()));
        settings->AddChild(gridspace);
        wxXmlNode* center0 = new wxXmlNode(wxXML_ELEMENT_NODE, "Display2DCenter0");
        center0->AddAttribute("value", std::to_string(xlights->GetDisplay2DCenter0()));
        settings->AddChild(center0);
        wxXmlNode* laygrp = new wxXmlNode(wxXML_ELEMENT_NODE, "storedLayourGroup");
        laygrp->AddAttribute("value", xlights->GetStoredLayoutGroup());
        settings->AddChild(laygrp);
        wxXmlNode* layout3d = new wxXmlNode(wxXML_ELEMENT_NODE, "LayoutMode3D");
        layout3d->AddAttribute("value", "tbd");
        settings->AddChild(layout3d);
        wxXmlNode* previewW = new wxXmlNode(wxXML_ELEMENT_NODE, "previewWidth");
        previewW->AddAttribute("value", std::to_string(0));
        settings->AddChild(previewW);
        wxXmlNode* previewH = new wxXmlNode(wxXML_ELEMENT_NODE, "previewHeight");
        previewH->AddAttribute("value", std::to_string(0));
        settings->AddChild(previewH);
        node->AddChild(settings);
    }

    void Serialize3dObjects(wxXmlNode* node, xLightsFrame* xlights){
        wxXmlNode* settings = new wxXmlNode(wxXML_ELEMENT_NODE, "view_objects");
        wxXmlNode* terrain = new wxXmlNode(wxXML_ELEMENT_NODE, "view_object");
        const ViewObjectManager* om = new ViewObjectManager(xlights);
        //std::map<std::string, ViewObject*> vo = om->GetViewObjects();
        //ViewObject* t0 = om->GetViewObject("Terrain");
        //BaseObject* b0 = om->GetObject("Mesh");
        TerrianObject* t = new TerrianObject(node, *om);
        //OutputManager* om0 = new OutputManager();
        //ModelManager* mm = new ModelManager(om0, xlights);
        //std::map<std::string, Model*> gm = mm->GetModels();

        terrain->AddAttribute(XmlNodeKeys::ImageAttribute, t->GetAttribute("Image"));
        terrain->AddAttribute(XmlNodeKeys::BrightnessAttribute, t->GetAttribute("Brightness"));
        terrain->AddAttribute(XmlNodeKeys::TransparencyAttribute, std::to_string(t->getTransparency()));
        terrain->AddAttribute(XmlNodeKeys::TerrainLineAttribute, std::to_string(t->getSpacing()));
        terrain->AddAttribute(XmlNodeKeys::TerrainWidthAttribute, std::to_string(t->getWidth()));
        terrain->AddAttribute(XmlNodeKeys::TerrainDepthAttribute, std::to_string(t->getDepth()));
        terrain->AddAttribute(XmlNodeKeys::TerrainBrushAttribute, t->GetAttribute("TerrianBrushSize"));
        terrain->AddAttribute(XmlNodeKeys::GridColorAttribute, t->getGridColor());
        terrain->AddAttribute(XmlNodeKeys::HideGridAttribute, std::to_string(t->isHideGrid()));
        terrain->AddAttribute(XmlNodeKeys::HideImageAttribute, std::to_string(t->isHideImage()));
        terrain->AddAttribute(XmlNodeKeys::PointDataAttribute, t->getPointData());

        settings->AddChild(terrain);

        node->AddChild(settings);
    };

    [[nodiscard]] wxXmlNode* CommonVisitSteps(const Model& model) {
        wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
        AddBaseObjectAttributes(model, xmlNode);
        AddCommonModelAttributes(model, xmlNode);
        AddModelScreenLocationAttributes(model, xmlNode);
        SerializeSuperStrings(model, xmlNode);
        return xmlNode;
    }

    void Visit(const ArchesModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::ZigZagAttribute, model.GetZigZag() ? "true": "false");
        xmlNode->AddAttribute(XmlNodeKeys::HollowAttribute, std::to_string(model.GetHollow()));
        xmlNode->AddAttribute(XmlNodeKeys::GapAttribute, std::to_string(model.GetGap()));
        xmlNode->AddAttribute(XmlNodeKeys::ArcAttribute, std::to_string(model.GetArc()));
        xmlNode->AddAttribute(XmlNodeKeys::LayerSizesAttribute, model.SerialiseLayerSizes());
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const CandyCaneModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::CCHeightAttribute, std::to_string(model.GetCandyCaneHeight()));
        xmlNode->AddAttribute(XmlNodeKeys::CCReverseAttribute, model.IsReverse() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::CCSticksAttribute, model.IsSticks() ? "true": "false");
        xmlNode->AddAttribute(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const CircleModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        xmlNode->AddAttribute(XmlNodeKeys::InsideOutAttribute, model.IsInsideOut() ? "1" : "0");
        xmlNode->AddAttribute(XmlNodeKeys::LayerSizesAttribute,  model.SerialiseLayerSizes());
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);     
    }

    void Visit(const ChannelBlockModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        AddTwoPointScreenLocationAttributes(model, xmlNode);
        std::vector<std::string> cp = model.GetChannelProperies();
        for (auto i = 0; i < cp.size();  i++) {
            xmlNode->AddAttribute("ChannelProperties.ChannelColor" + std::to_string(i+1), cp[i]);
        }
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const CubeModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        xmlNode->AddAttribute(XmlNodeKeys::StyleAttribute, model.GetStrandStyle());
        xmlNode->AddAttribute(XmlNodeKeys::CubeStartAttribute, model.GetStrandStart());
        xmlNode->AddAttribute(XmlNodeKeys::StrandPerLineAttribute, model.GetStrandPerLine());
        xmlNode->AddAttribute(XmlNodeKeys::StrandPerLayerAttribute, model.GetStrandPerLayer());
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const CustomModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        xmlNode->AddAttribute(XmlNodeKeys::CMDepthAttribute, std::to_string(model.GetCustomDepth()));
        //xmlNode->AddAttribute(XmlNodeKeys::CustomModelAttribute, model.GetCustomData());
        xmlNode->AddAttribute(XmlNodeKeys::BkgImageAttribute, model.GetCustomBackground());
        xmlNode->AddAttribute(XmlNodeKeys::BkgLightnessAttribute, std::to_string(model.GetCustomLightness()));
        const Model* m = dynamic_cast<const Model*>(&model);
        AddCustomModel(xmlNode, model);
        AddOtherElements(xmlNode, m);
    }
    void Visit(const IciclesModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        AddThreePointScreenLocationAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::DropPatternAttribute, model.GetDropPattern());
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const ImageModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        xmlNode->AddAttribute(XmlNodeKeys::ImageAttribute, model.GetImageFile());
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const MatrixModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        xmlNode->DeleteAttribute(XmlNodeKeys::DisplayAsAttribute);
        if (model.isVerticalMatrix()) {
            xmlNode->AddAttribute(XmlNodeKeys::DisplayAsAttribute, "Vert Matrix");
        } else {
            xmlNode->AddAttribute(XmlNodeKeys::DisplayAsAttribute, "Horiz Matrix");
        }
        xmlNode->AddAttribute(XmlNodeKeys::LowDefinitionAttribute, std::to_string(model.GetLowDefFactor()));
        xmlNode->AddAttribute(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::NoZigZagAttribute, model.IsNoZigZag() ? "true" : "false");
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const SingleLineModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        AddTwoPointScreenLocationAttributes(model, xmlNode);
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const PolyLineModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
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
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const SphereModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        xmlNode->AddAttribute(XmlNodeKeys::DegreesAttribute, std::to_string(model.GetSphereDegrees()));
        xmlNode->AddAttribute(XmlNodeKeys::StartLatAttribute, std::to_string(model.GetStartLatitude()));
        xmlNode->AddAttribute(XmlNodeKeys::EndLatAttribute, std::to_string(model.GetEndLatitude()));
        xmlNode->AddAttribute(XmlNodeKeys::LowDefinitionAttribute, std::to_string(model.GetLowDefFactor()));
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const SpinnerModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        xmlNode->AddAttribute(XmlNodeKeys::AlternateAttribute, model.HasAlternateNodes() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::ZigZagAttribute, model.HasZigZag() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::HallowAttribute, std::to_string(model.GetHollowPercent()));
        xmlNode->AddAttribute(XmlNodeKeys::ArcAngleAttribute, std::to_string(model.GetArcAngle()));
        xmlNode->AddAttribute(XmlNodeKeys::StartAngleAttribute, std::to_string(model.GetStartAngle()));
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const StarModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        xmlNode->AddAttribute(XmlNodeKeys::LayerSizesAttribute, model.SerialiseLayerSizes());
        xmlNode->AddAttribute(XmlNodeKeys::StarStartLocationAttribute, model.GetStartLocation());
        xmlNode->AddAttribute(XmlNodeKeys::StarRatioAttribute, std::to_string(model.GetStarRatio()));
        xmlNode->AddAttribute(XmlNodeKeys::StarCenterPercentAttribute, std::to_string(model.GetInnerPercent()));
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const TreeModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        xmlNode->DeleteAttribute(XmlNodeKeys::DisplayAsAttribute);
        xmlNode->AddAttribute(XmlNodeKeys::DisplayAsAttribute, model.GetTreeDescription());
        xmlNode->AddAttribute(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::NoZigZagAttribute, model.IsNoZigZag() ? "true" : "false");
        xmlNode->AddAttribute(XmlNodeKeys::BottomTopRatioAttribute, std::to_string(model.GetBottomTopRatio()));
        xmlNode->AddAttribute(XmlNodeKeys::PerspectiveAttribute, std::to_string(model.GetTreePerspective()));
        xmlNode->AddAttribute(XmlNodeKeys::SpiralRotationsAttribute, std::to_string(model.GetSpiralRotations()));
        xmlNode->AddAttribute(XmlNodeKeys::TreeRotationAttribute, std::to_string(model.GetTreeRotation()));
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const WindowFrameModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        xmlNode->AddAttribute(XmlNodeKeys::RotationAttribute, model.GetRotation() ? "Counter Clockwise" : "Clockwise");
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const WreathModel& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const DmxMovingHeadAdv& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        AddColorAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::DmxFixturelAttribute, model.GetFixture());
        xmlNode->AddAttribute(XmlNodeKeys::DmxBeamYOffsetAttribute, std::to_string(model.GetBeamYOffset()));
        xmlNode->AddAttribute(XmlNodeKeys::DmxBeamLengthAttribute, std::to_string(model.GetBeamLength()));
        AddDmxMotorAttributes(reinterpret_cast<DmxMotor*>(model.GetPanMotor()), xmlNode);
        AddDmxMotorAttributes(reinterpret_cast<DmxMotor*>(model.GetTiltMotor()), xmlNode);
        AddMeshAttributes(reinterpret_cast<Mesh*>(model.GetBaseMesh()), xmlNode);
        AddMeshAttributes(reinterpret_cast<Mesh*>(model.GetYokeMesh()), xmlNode);
        AddMeshAttributes(reinterpret_cast<Mesh*>(model.GetHeadMesh()), xmlNode);
        AddDimmerAbilityAttributes(model, xmlNode);
        AddShutterAbilityAttributes(model, xmlNode);
        AddPresetAttributes(model.GetPresetAbility(), xmlNode);
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }

    void Visit(const DmxMovingHead& model) override {
        wxXmlNode* xmlNode = CommonVisitSteps(model);
        AddColorAttributes(model, xmlNode);
        xmlNode->AddAttribute(XmlNodeKeys::DmxFixturelAttribute, model.GetFixture());
        xmlNode->AddAttribute(XmlNodeKeys::DmxStyleAttribute, model.GetDMXStyle());
        xmlNode->AddAttribute(XmlNodeKeys::DmxBeamLengthAttribute, std::to_string(model.GetBeamLength()));
        xmlNode->AddAttribute(XmlNodeKeys::DmxBeamWidthAttribute, std::to_string(model.GetBeamWidth()));
        xmlNode->AddAttribute(XmlNodeKeys::HideBodyAttribute, std::to_string(model.GetHideBody()));
        AddDmxMotorAttributes(reinterpret_cast<DmxMotor*>(model.GetPanMotor()), xmlNode);
        AddDmxMotorAttributes(reinterpret_cast<DmxMotor*>(model.GetTiltMotor()), xmlNode);
        AddDimmerAbilityAttributes(model, xmlNode);
        AddShutterAbilityAttributes(model, xmlNode);
        AddPresetAttributes(model.GetPresetAbility(), xmlNode);
        const Model* m = dynamic_cast<const Model*>(&model);
        AddOtherElements(xmlNode, m);
    }
};

struct XmlDeserializingObjectFactory {
    Model* Deserialize(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        auto type = node->GetAttribute(XmlNodeKeys::DisplayAsAttribute);

        if (type == XmlNodeKeys::ArchesType) {
            return DeserializeArches(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::CandyCaneType) {
            return DeserializeCandyCane(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::ChannelBlockType) {
            return DeserializeChannelBlock(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::CircleType) {
            return DeserializeCircle(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::CubeType) {
            return DeserializeCube(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::CustomType) {
            return DeserializeCustom(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::DmxMovingHeadType) {
            return DeserializeDmxMovingHead(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::DmxMovingHeadAdvType) {
            return DeserializeDmxMovingHeadAdv(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::IciclesType) {
            return DeserializeIcicles(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::ImageType) {
            return DeserializeImage(new wxXmlNode(*node), xlights, importing);
        } else if (type.Contains(XmlNodeKeys::MatrixType)) {
            return DeserializeMatrix(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::SingleLineType) {
            return DeserializeSingleLine(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::PolyLineType) {
            return DeserializePolyLine(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::SphereType) {
            return DeserializeSphere(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::SpinnerType) {
            return DeserializeSpinner(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::StarType) {
            return DeserializeStar(new wxXmlNode(*node), xlights, importing);
        } else if (type.Contains(XmlNodeKeys::TreeType)) {
            return DeserializeTree(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::WindowType) {
            return DeserializeWindow(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::WreathType) {
            return DeserializeWreath(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::ViewObjectsType) {
            return DeserializeEffects(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::EffectsType) {
            return DeserializeViews(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::ViewsType) {
            return DeserializePalettes(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::PalettesType) {
            return DeserializeGroups(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::LayoutGroupsType) {
            return DeserializePerspectives(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::PerspectivesType) {
            return DeserializeSettings(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::SettingsType) {
            return DeserializeColors(new wxXmlNode(*node), xlights, importing);
        } else if (type == XmlNodeKeys::ColorsType) {
            return DeserializeViewPoints(new wxXmlNode(*node), xlights, importing);
        }

        throw std::runtime_error("Unknown object type: " + type);
    }

private:
    void CommonDeserializeSteps(Model* model, wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        if (model->GetModelScreenLocation().CheckUpgrade(node) == ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_EXEC_READ) {
            model->GetModelScreenLocation().Read(node);
        }

        DeserializeBaseObjectAttributes(model, node, xlights, importing);
        DeserializeCommonModelAttributes(model, node, importing);
        DeserializeModelScreenLocationAttributes(model, node, importing);
        DeserializeSuperStrings(model, node);
    }

    void DeserializeControllerConnection(Model* model, wxXmlNode* node) {
        for (wxXmlNode* p = node->GetChildren(); p != nullptr; p = p->GetNext()) {
            if (p->GetName() == "ControllerConnection") {
                auto& cc = model->GetCtrlConn();
                cc.SetName(p->GetAttribute(XmlNodeKeys::ControllerAttribute, xlEMPTY_STRING).Trim(true).Trim(false).ToStdString());
                cc.SetProtocol(p->GetAttribute(XmlNodeKeys::ProtocolAttribute, xlEMPTY_STRING).ToStdString());
                cc.SetSerialProtocolSpeed(std::stoi(p->GetAttribute(XmlNodeKeys::ProtocolSpeedAttribute, std::to_string(CtrlDefs::DEFAULT_PROTOCOL_SPEED)).ToStdString()));
                cc.SetPort(std::stoi(p->GetAttribute(XmlNodeKeys::PortAttribute, std::to_string(CtrlDefs::DEFAULT_PORT)).ToStdString()));
                cc.SetBrightness(std::stoi(p->GetAttribute(XmlNodeKeys::BrightnessAttribute, std::to_string(CtrlDefs::DEFAULT_BRIGHTNESS)).ToStdString()));
                cc.SetStartNulls(std::stoi(p->GetAttribute(XmlNodeKeys::StartNullAttribute, std::to_string(CtrlDefs::DEFAULT_NULLS)).ToStdString()));
                cc.SetEndNulls(std::stoi(p->GetAttribute(XmlNodeKeys::EndNullAttribute, std::to_string(CtrlDefs::DEFAULT_NULLS)).ToStdString()));
                cc.SetColorOrder(p->GetAttribute(XmlNodeKeys::ColorOrderAttribute, CtrlDefs::DEFAULT_COLOR_ORDER).ToStdString());
                cc.SetGroupCount(std::stoi(p->GetAttribute(XmlNodeKeys::GroupCountAttribute, std::to_string(CtrlDefs::DEFAULT_GROUP_COUNT)).ToStdString()));
                cc.SetGamma(std::stof(p->GetAttribute(XmlNodeKeys::GammaAttribute, std::to_string(CtrlDefs::DEFAULT_GAMMA)).ToStdString()));
                cc.SetReverse(std::stoi(p->GetAttribute(XmlNodeKeys::CReverseAttribute, std::to_string(CtrlDefs::DEFAULT_REVERSE)).ToStdString()));
                cc.SetZigZag(std::stoi(p->GetAttribute(XmlNodeKeys::CZigZagAttribute, std::to_string(CtrlDefs::DEFAULT_ZIGZAG)).ToStdString()));
                
                // Set all the property checkbox active states
                cc.UpdateProperty(CtrlProps::USE_SMART_REMOTE,   p->HasAttribute(XmlNodeKeys::SmartRemoteAttribute));
                cc.UpdateProperty(CtrlProps::START_NULLS_ACTIVE, p->HasAttribute(XmlNodeKeys::StartNullAttribute));
                cc.UpdateProperty(CtrlProps::END_NULLS_ACTIVE,   p->HasAttribute(XmlNodeKeys::EndNullAttribute));
                cc.UpdateProperty(CtrlProps::BRIGHTNESS_ACTIVE,  p->HasAttribute(XmlNodeKeys::BrightnessAttribute));
                cc.UpdateProperty(CtrlProps::GAMMA_ACTIVE,       p->HasAttribute(XmlNodeKeys::GammaAttribute));
                cc.UpdateProperty(CtrlProps::COLOR_ORDER_ACTIVE, p->HasAttribute(XmlNodeKeys::ColorOrderAttribute));
                cc.UpdateProperty(CtrlProps::REVERSE_ACTIVE,     p->HasAttribute(XmlNodeKeys::CReverseAttribute));
                cc.UpdateProperty(CtrlProps::GROUP_COUNT_ACTIVE, p->HasAttribute(XmlNodeKeys::GroupCountAttribute));
                cc.UpdateProperty(CtrlProps::ZIG_ZAG_ACTIVE,     p->HasAttribute(XmlNodeKeys::CZigZagAttribute));
                cc.UpdateProperty(CtrlProps::TS_ACTIVE,          p->HasAttribute(XmlNodeKeys::SmartRemoteTsAttribute));

                // Set all the Smart Remote values
                cc.SetSmartRemote(std::stoi(p->GetAttribute(XmlNodeKeys::SmartRemoteAttribute, "0").ToStdString()));
                cc.SetSRMaxCascade(std::stoi(p->GetAttribute(XmlNodeKeys::SRMaxCascadeAttribute, "1").ToStdString()));
                cc.SetSRCascadeOnPort(p->GetAttribute(XmlNodeKeys::SRCascadeOnPortAttribute, "FALSE").ToStdString() == "TRUE");
                cc.SetSmartRemoteTs(std::stoi(p->GetAttribute(XmlNodeKeys::SmartRemoteTsAttribute, "0").ToStdString()));
                cc.SetSmartRemoteType(p->GetAttribute(XmlNodeKeys::SmartRemoteTypeAttribute, ""));
            }
        }
    }

    void DeserializeBaseObjectAttributes(Model* model, wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        std::string name = node->GetAttribute("name").Trim(true).Trim(false).ToStdString();
        if (importing)
        {
            name = xlights->AllModels.GenerateModelName(name);
            model->SetLayoutGroup("Unassigned");
        } else {
            model->SetLayoutGroup(node->GetAttribute(XmlNodeKeys::LayoutGroupAttribute, "Unassigned").ToStdString());
        }
        model->SetName(name);
        model->SetDisplayAs(node->GetAttribute(XmlNodeKeys::DisplayAsAttribute).ToStdString());
        model->SetActive(std::stoi(node->GetAttribute(XmlNodeKeys::ActiveAttribute, "1").ToStdString()));
        model->SetFromBase(std::stoi(node->GetAttribute(XmlNodeKeys::FromBaseAttribute, "0").ToStdString()));
    }

    void DeserializeCommonModelAttributes(Model* model, wxXmlNode* node, bool importing) {
        if (node->HasAttribute(XmlNodeKeys::StartSideAttribute)) {
            model->SetStartSide(node->GetAttribute(XmlNodeKeys::StartSideAttribute, "B"));
            model->SetIsBtoT(node->GetAttribute(XmlNodeKeys::StartSideAttribute, "B") == "B");
        } else {
            model->SetIsBtoT(true);
        }
        model->SetDirection(node->GetAttribute(XmlNodeKeys::DirAttribute, "L"));
        model->SetIsLtoR(node->GetAttribute(XmlNodeKeys::DirAttribute, "L") != "R");
        model->SetParm1(std::stol(node->GetAttribute(XmlNodeKeys::Parm1Attribute,"0").ToStdString()));
        model->SetParm2(std::stol(node->GetAttribute(XmlNodeKeys::Parm2Attribute,"0").ToStdString()));
        model->SetParm3(std::stol(node->GetAttribute(XmlNodeKeys::Parm3Attribute,"0").ToStdString()));
        model->SetPixelStyle((Model::PIXEL_STYLE)(std::stol(node->GetAttribute(XmlNodeKeys::AntialiasAttribute, std::to_string((int)Model::PIXEL_STYLE::PIXEL_STYLE_SMOOTH)).ToStdString())));
        model->SetPixelSize(std::stoi(node->GetAttribute(XmlNodeKeys::PixelSizeAttribute, "2").ToStdString()));
        model->SetRGBWHandling((std::string)node->GetAttribute(XmlNodeKeys::RGBWHandleAttribute));
        model->SetStringType(node->GetAttribute(XmlNodeKeys::StringTypeAttribute, "RGB Nodes").ToStdString());
        model->SetLowDefFactor(std::stoi(node->GetAttribute(XmlNodeKeys::LowDefinitionAttribute, "100").ToStdString()));
        model->SetShadowModelFor(node->GetAttribute(XmlNodeKeys::ShadowModelAttribute, "").ToStdString());
        model->SetTransparency(std::stol(node->GetAttribute(XmlNodeKeys::TransparencyAttribute,"0").ToStdString()));
        model->SetBlackTransparency(std::stol(node->GetAttribute(XmlNodeKeys::BTransparencyAttribute,"0").ToStdString()));
        model->SetDescription(UnXmlSafe(node->GetAttribute(XmlNodeKeys::DescriptionAttribute)));
        model->SetTagColourAsString(node->GetAttribute(XmlNodeKeys::TagColourAttribute, "#000000"));
        model->SetStartChannel(node->GetAttribute(XmlNodeKeys::StartChannelAttribute, "1").ToStdString());
        model->SetNodeNames(node->GetAttribute(XmlNodeKeys::NodeNamesAttribute).ToStdString());
        model->SetStrandNames(node->GetAttribute(XmlNodeKeys::StrandNamesAttribute).ToStdString());
        model->SetCustomColor(node->GetAttribute(XmlNodeKeys::CustomColorAttribute, "#000000").ToStdString());
        model->SetModelChain(node->GetAttribute(XmlNodeKeys::ModelChainAttribute,""));
        
        bool hasIndiv = std::stol(node->GetAttribute(XmlNodeKeys::AdvancedAttribute,"0").ToStdString());
        model->SetHasIndividualStartChannels(hasIndiv);
        if (hasIndiv ) {
            model->AddIndividualStartChannel(model->GetModelStartChannel());
        }

        wxXmlNode* f = node->GetChildren();
        while (f != nullptr) {
            if ("faceInfo" == f->GetName()) {
                FaceStateData newFaceInfo;
                XmlSerialize::DeserializeFaceInfo(f, newFaceInfo);
                model->SetFaceInfo(newFaceInfo);
                model->UpdateFaceInfoNodes();
            } else if ("stateInfo" == f->GetName()) {
                FaceStateData newStateInfo;
                XmlSerialize::DeserializeStateInfo(f, newStateInfo);
                model->SetStateInfo(newStateInfo);
                model->UpdateStateInfoNodes();
            } else if (XmlNodeKeys::DimmingCurveName == f->GetName()) {
                model->modelDimmingCurve = DimmingCurve::createFromXML(f);
            } else if ("subModel" == f->GetName()) {
                DeserializeSubModel(model, f);
            } else if ("ControllerConnection" == f->GetName()) {
                if (!importing) {
                    DeserializeControllerConnection(model, node);
                }
            } else if (f->GetName() == XmlNodeKeys::AliasesAttribute) {
                DeserializeAliases(model, f);
            }
            f = f->GetNext();
        }
        
        if (node->HasAttribute(XmlNodeKeys::ModelBrightnessAttribute) && model->modelDimmingCurve == nullptr) {
            int b = std::stoi(node->GetAttribute(XmlNodeKeys::ModelBrightnessAttribute, "0").ToStdString());
            if (b != 0) {
                model->modelDimmingCurve = DimmingCurve::createBrightnessGamma(b, 1.0);
            }
        }
    }
    
    void DeserializeSubModel(Model* model, wxXmlNode* node)
    {
        const std::string name = node->GetAttribute(XmlNodeKeys::NameAttribute).Trim(true).Trim(false).ToStdString();
        const std::string layout = node->GetAttribute(XmlNodeKeys::LayoutAttribute, "vertical").ToStdString();
        const std::string type = node->GetAttribute(XmlNodeKeys::TypeAttribute, "ranges").ToStdString();
        const std::string bufferStyle = node->GetAttribute(XmlNodeKeys::BufferStyleAttribute, "Default").ToStdString();
        SubModel *sm = new SubModel(model, name, layout, type, bufferStyle);
        model->AddSubmodel(sm);

        if (sm->IsRanges()) {
            if (sm->IsXYBufferStyle()) {
                int line = 0;
                std::vector<int> nodeIndexes;
                while (node->HasAttribute(wxString::Format("line%d", line))) {
                    sm->AddRangeXY( node->GetAttribute(wxString::Format("line%d", line)) );
                    line++;
                }
                sm->CheckDuplicates();
                sm->CalcRangeXYBufferSize();
            } else { //default and stacked buffer styles
                int line = 0;
                while (node->HasAttribute(wxString::Format("line%d", line))) {
                    wxString nodes = node->GetAttribute(wxString::Format("line%d", line));
                    sm->AddDefaultBuffer(nodes);
                    line++;
                }
                sm->CheckDuplicates();
            }
        } else {
            sm->AddSubbuffer(node->GetAttribute(XmlNodeKeys::SubBufferAttribute) );
        }
    }

    void DeserializeAliases(Model* model, wxXmlNode* node)
    {
        std::list<std::string> aliases;

        wxXmlNode* f = node->GetChildren();
        while (f != nullptr) {
            if (f->HasAttribute(XmlNodeKeys::NameAttribute)) {
                aliases.push_back(f->GetAttribute(XmlNodeKeys::NameAttribute));
            }
            f = f->GetNext();
        }

        if (aliases.size() > 0) {
            model->SetAliases(aliases);
        }
    }

    void DeserializeSuperStrings(Model* model, wxXmlNode* node)
    {
        bool found = true;
        int index = 0;
        while (found) {
            auto an = std::format("SuperStringColour{}", index);
            if (node->HasAttribute(an)) {
                model->AddSuperStringColour(xlColor(node->GetAttribute(an)));
            } else {
                found = false;
            }
            index++;
        }
    }

    void DeserializeModelScreenLocationAttributes(Model* model, wxXmlNode* node, bool importing) {
        glm::vec3 loc;
        loc.x = std::stof(node->GetAttribute(XmlNodeKeys::WorldPosXAttribute).ToStdString());
        loc.y = std::stof(node->GetAttribute(XmlNodeKeys::WorldPosYAttribute).ToStdString());
        loc.z = std::stof(node->GetAttribute(XmlNodeKeys::WorldPosZAttribute).ToStdString());
        model->GetBaseObjectScreenLocation().SetWorldPosition(loc);
        glm::vec3 scale(1.0f, 1.0f, 1.0f);
        if (node->HasAttribute(XmlNodeKeys::ScaleXAttribute)) { scale.x = std::stof(node->GetAttribute(XmlNodeKeys::ScaleXAttribute).ToStdString()); }
        if (node->HasAttribute(XmlNodeKeys::ScaleYAttribute)) { scale.y = std::stof(node->GetAttribute(XmlNodeKeys::ScaleYAttribute).ToStdString()); }
        if (node->HasAttribute(XmlNodeKeys::ScaleZAttribute)) { scale.z = std::stof(node->GetAttribute(XmlNodeKeys::ScaleZAttribute).ToStdString()); }
        model->GetBaseObjectScreenLocation().SetScaleMatrix(scale);
        glm::vec3 rotate(0.0f, 0.0f, 0.0f);
        if (node->HasAttribute(XmlNodeKeys::RotateXAttribute)) { rotate.x = std::stof(node->GetAttribute(XmlNodeKeys::RotateXAttribute).ToStdString()); }
        if (node->HasAttribute(XmlNodeKeys::RotateYAttribute)) { rotate.y = std::stof(node->GetAttribute(XmlNodeKeys::RotateYAttribute).ToStdString()); }
        if (node->HasAttribute(XmlNodeKeys::RotateZAttribute)) { rotate.z = std::stof(node->GetAttribute(XmlNodeKeys::RotateZAttribute).ToStdString()); }
        model->GetBaseObjectScreenLocation().SetRotation(rotate);
        if( !importing ) {
            if (node->HasAttribute(XmlNodeKeys::LockedAttribute)) {
                bool locked = std::stoi(node->GetAttribute(XmlNodeKeys::LockedAttribute).ToStdString()) > 0;
                model->GetModelScreenLocation().Lock(locked);
            }
        }
    }

    void DeserializeTwoPointScreenLocationAttributes(Model* model, wxXmlNode* node) {
        float x2 = std::stof(node->GetAttribute(XmlNodeKeys::X2Attribute, "0").ToStdString());
        float y2 = std::stof(node->GetAttribute(XmlNodeKeys::Y2Attribute, "0").ToStdString());
        float z2 = std::stof(node->GetAttribute(XmlNodeKeys::Z2Attribute, "0").ToStdString());
        TwoPointScreenLocation& screenLoc = dynamic_cast<TwoPointScreenLocation&>(model->GetBaseObjectScreenLocation());
        screenLoc.SetX2(x2);
        screenLoc.SetY2(y2);
        screenLoc.SetZ2(z2);
    }

    void DeserializeThreePointScreenLocationAttributes(Model* model, wxXmlNode* node) {
        DeserializeTwoPointScreenLocationAttributes(model, node);
        int angle = std::stoi(node->GetAttribute(XmlNodeKeys::AngleAttribute, "0").ToStdString());
        float height = std::stof(node->GetAttribute("Height", "1.0").ToStdString());
        float shear = std::stof(node->GetAttribute("Shear", "0.0").ToStdString());
        ThreePointScreenLocation& screenLoc = dynamic_cast<ThreePointScreenLocation&>(model->GetBaseObjectScreenLocation());
        screenLoc.SetAngle(angle);
        screenLoc.SetMHeight(height);
        screenLoc.SetYShear(shear);
    }

    Model* DeserializeArches(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        ArchesModel* model = new ArchesModel(xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        DeserializeThreePointScreenLocationAttributes(model, node);
        model->SetZigZag(node->GetAttribute(XmlNodeKeys::ZigZagAttribute).ToStdString() == "true");
        if (node->HasAttribute(XmlNodeKeys::HollowAttribute)) { model->SetHollow(std::stoi(node->GetAttribute(XmlNodeKeys::HollowAttribute).ToStdString())); }
        if (node->HasAttribute(XmlNodeKeys::GapAttribute)) { model->SetGap(std::stoi(node->GetAttribute(XmlNodeKeys::GapAttribute).ToStdString())); }
        if( node->HasAttribute("arc")) { // special case for legacy Arch model format
            model->SetArc(std::stoi(node->GetAttribute("arc").ToStdString()));
        } else if( node->HasAttribute("Arc")) {
            model->SetArc(std::stoi(node->GetAttribute(XmlNodeKeys::ArcAttribute).ToStdString()));
        }
        model->DeserialiseLayerSizes(node->GetAttribute(XmlNodeKeys::LayerSizesAttribute).ToStdString(), false);
        if (node->HasAttribute(XmlNodeKeys::ArchesSkewAttribute)) {
            int angle = std::stoi(node->GetAttribute(XmlNodeKeys::ArchesSkewAttribute, "0").ToStdString());
            ThreePointScreenLocation& screenLoc = dynamic_cast<ThreePointScreenLocation&>(model->GetBaseObjectScreenLocation());
            screenLoc.SetAngle(angle);
        }
        model->SetFromXml(nullptr); // hopefully can delete this later
        return model;
    }

    Model* DeserializeCandyCane(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        CandyCaneModel* model = new CandyCaneModel(xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        DeserializeThreePointScreenLocationAttributes(model, node);
        model->SetReverse(node->GetAttribute("CandyCaneReverse", "false") == "true");
        model->SetSticks(node->GetAttribute("CandyCaneSticks", "false") == "true");
        model->SetAlternateNodes(node->GetAttribute("AlternateNodes", "false") == "true");
        if (node->HasAttribute("CandyCaneSkew")) {
            int angle = wxAtoi(node->GetAttribute("CandyCaneSkew", "0"));
            ThreePointScreenLocation& screenLoc = dynamic_cast<ThreePointScreenLocation&>(model->GetBaseObjectScreenLocation());
            screenLoc.SetAngle(angle);
        }
        model->SetCaneHeight(std::stof(node->GetAttribute("CandyCaneHeight", "1.0").ToStdString()));
        model->SetFromXml(nullptr); // hopefully can delete this later
        return model;
    }

     Model* DeserializeChannelBlock(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        ChannelBlockModel* model = new ChannelBlockModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeCircle(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        CircleModel* model = new CircleModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeCube(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        CubeModel* model = new CubeModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeCustom(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        CustomModel* model = new CustomModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeDmxMovingHead(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        DmxMovingHead* model = new DmxMovingHead(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeDmxMovingHeadAdv(wxXmlNode *node, xLightsFrame* xlights, bool importing) {
        DmxMovingHeadAdv *model = new DmxMovingHeadAdv(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeIcicles(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        IciclesModel* model = new IciclesModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeImage(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        ImageModel* model = new ImageModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeMatrix(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        MatrixModel* model = new MatrixModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeSingleLine(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        SingleLineModel* model = new SingleLineModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializePolyLine(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        PolyLineModel* model = new PolyLineModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeSphere(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        SphereModel* model = new SphereModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeSpinner(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        SpinnerModel* model = new SpinnerModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeStar(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        StarModel* model = new StarModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeTree(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        TreeModel* model = new TreeModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeWindow(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        WindowFrameModel* model = new WindowFrameModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeWreath(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        WreathModel* model = new WreathModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeEffects(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        WreathModel* model = new WreathModel(node, xlights->AllModels, false);
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeViews(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        Model* model;
        model = new WreathModel(node, xlights->AllModels, false);        // FIXME: Based on class looks like in progress work
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializePalettes(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        Model* model;
        model = new WreathModel(node, xlights->AllModels, false);        // FIXME: Based on class looks like in progress work
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeGroups(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        Model* model;
        model = new WreathModel(node, xlights->AllModels, false);        // FIXME: Based on class looks like in progress work
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializePerspectives(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        Model* model;
        model = new WreathModel(node, xlights->AllModels, false);        // FIXME: Based on class looks like in progress work
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeSettings(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        Model* model;
        model = new WreathModel(node, xlights->AllModels, false);        // FIXME: Based on class looks like in progress work
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeColors(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        Model* model;
        model = new WreathModel(node, xlights->AllModels, false);        // FIXME: Based on class looks like in progress work
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }

    Model* DeserializeViewPoints(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
        Model* model;
        model = new WreathModel(node, xlights->AllModels, false);        // FIXME: Based on class looks like in progress work
        CommonDeserializeSteps(model, node, xlights, importing);
        return model;
    }
};

struct XmlSerializer {
    XmlSerializer() {
    }

    static bool IsXmlSerializerFormat(const wxXmlNode* node) {
        if (node->GetAttribute(XmlNodeKeys::TypeAttribute, "") == XmlNodeKeys::ExportedAttribute) {
            return true;
        }
        if (node->GetAttribute(XmlNodeKeys::SerializeAttribute, "") == "1") {
            return true;
        }
        return false;
    }

    // Serializes and Saves a single model into an XML document
    void SerializeAndSaveModel(const BaseObject& object, xLightsFrame* xlights) {
        wxString name = object.GetName();

        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (filename.IsEmpty())
            return;
        wxXmlDocument doc = SerializeModel(object, xlights);
        doc.Save(filename);
    }

    // Serialize a single model into an XML document
    wxXmlDocument SerializeModel(const BaseObject& object, xLightsFrame* xlights) {
        wxXmlDocument doc;

        wxXmlNode* docNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelsNodeName);
        docNode->AddAttribute(XmlNodeKeys::TypeAttribute, XmlNodeKeys::ExportedAttribute);

        XmlSerializingVisitor visitor{ docNode };

        object.Accept(visitor);

        doc.SetRoot(docNode);

        return doc;
    }

    // Deserialize a single model from an XML document
    Model* DeserializeModel(const wxXmlDocument& doc, xLightsFrame* xlights, bool importing) {
        wxXmlNode* root = doc.GetRoot();
        wxXmlNode* model_node = root->GetChildren();
        return DeserializeModel(model_node, xlights, importing);
    }

    // Deserialize a single model XML node
    Model* DeserializeModel(wxXmlNode* model_node, xLightsFrame* xlights, bool importing) {
        XmlDeserializingObjectFactory factory{};
        Model* model = factory.Deserialize(model_node, xlights, importing);

        // TODO: I'd like to get rid of this whole ImportModelChildren call but left it in the flow for now
        /*float min_x = (float)(model->GetBaseObjectScreenLocation().GetLeft());
        float max_x = (float)(model->GetBaseObjectScreenLocation().GetRight());
        float min_y = (float)(model->GetBaseObjectScreenLocation().GetBottom());
        float max_y = (float)(model->GetBaseObjectScreenLocation().GetTop());
        float min_z = (float)(model->GetBaseObjectScreenLocation().GetFront());
        float max_z = (float)(model->GetBaseObjectScreenLocation().GetBack());
        model->ImportModelChildren(model->GetModelXml(), xlights, model->GetName(), min_x, max_x, min_y, max_y, min_z, max_z);*/
        return model;
    }

};

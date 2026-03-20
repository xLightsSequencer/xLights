/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// BaseSerializingVisitor.cpp — all shared Visit() implementations and
// attribute/child-element helpers for the XML serialization visitor hierarchy.

#include "BaseSerializingVisitor.h"

#include <charconv>

#include "../models/ArchesModel.h"
#include "../models/BaseObject.h"
#include "../models/BaseObjectVisitor.h"
#include "../models/CandyCaneModel.h"
#include "../models/ChannelBlockModel.h"
#include "../models/CircleModel.h"
#include "../Color.h"
#include "../models/CubeModel.h"
#include "../models/CustomModel.h"
#include "../DimmingCurve.h"
#include "../models/GridlinesObject.h"
#include "../models/IciclesModel.h"
#include "../models/ImageModel.h"
#include "../models/ImageObject.h"
#include "../models/MatrixModel.h"
#include "../models/MultiPointModel.h"
#include "../models/MeshObject.h"
#include "../models/Model.h"
#include "../models/ModelGroup.h"
#include "../models/PolyLineModel.h"
#include "../models/RulerObject.h"
#include "../models/SingleLineModel.h"
#include "../models/SphereModel.h"
#include "../models/SpinnerModel.h"
#include "../models/StarModel.h"
#include "../models/SubModel.h"
#include "../models/TerrainObject.h"
#include "../models/TerrainScreenLocation.h"
#include "../models/ThreePointScreenLocation.h"
#include "../models/TreeModel.h"
#include "../models/WindowFrameModel.h"
#include "../models/WreathModel.h"
#include "../xLightsVersion.h"
#include "../Pixels.h"
#include "../models/DMX/DmxBeamAbility.h"
#include "../models/DMX/DmxColorAbilityCMY.h"
#include "../models/DMX/DmxColorAbilityRGB.h"
#include "../models/DMX/DmxColorAbilityWheel.h"
#include "../models/DMX/DmxPresetAbility.h"
#include "../models/DMX/DmxDimmerAbility.h"
#include "../models/DMX/DmxFloodArea.h"
#include "../models/DMX/DmxFloodlight.h"
#include "../models/DMX/DmxGeneral.h"
#include "../models/DMX/DmxImage.h"
#include "../models/DMX/DmxShutterAbility.h"
#include "../models/DMX/DmxMovingHeadAdv.h"
#include "../models/DMX/DmxMovingHead.h"
#include "../models/DMX/DmxServo.h"
#include "../models/DMX/DmxServo3D.h"
#include "../models/DMX/DmxSkull.h"
#include "../models/DMX/Mesh.h"
#include "../models/DMX/Servo.h"

#include "XmlNodeKeys.h"

#include <algorithm>
#include <format>

// ---------------------------------------------------------------------------
// File-local helpers
// ---------------------------------------------------------------------------

static std::string LowerStr(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// ---------------------------------------------------------------------------
// Static utilities
// ---------------------------------------------------------------------------

// static
std::string BaseSerializingVisitor::FloatToString(float f) {
    char buf[32];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), f, std::chars_format::general, 6);
    return std::string(buf, ptr);
}

// static
void BaseSerializingVisitor::SortAttributes(AttrCollector& collector) {
    const std::string attrToPrioritize       = "name";
    const std::string attrToPrioritizeSecond = "DisplayAs";
    const std::string attrToDePrioritize     = "CustomModel";

    auto custom_comparator = [&](const std::pair<std::string, std::string>& a,
                                  const std::pair<std::string, std::string>& b) {
        if (a.first == b.first) return false;
        if (a.first == attrToPrioritize) return true;
        if (b.first == attrToPrioritize) return false;
        if (a.first == attrToPrioritizeSecond) return true;
        if (b.first == attrToPrioritizeSecond) return false;
        if (a.first.find(attrToDePrioritize) != std::string::npos) return false;
        if (b.first.find(attrToDePrioritize) != std::string::npos) return true;
        return LowerStr(a.first) < LowerStr(b.first);
    };

    std::sort(collector.attrs.begin(), collector.attrs.end(), custom_comparator);
}

// ---------------------------------------------------------------------------
// Attribute helpers
// ---------------------------------------------------------------------------

void BaseSerializingVisitor::AddBaseObjectAttributes(const BaseObject& base, AttrCollector& attrs) {
    attrs.Add(XmlNodeKeys::NameAttribute, base.GetName());
    attrs.Add(XmlNodeKeys::DisplayAsAttribute, DisplayAsTypeToString(base.GetDisplayAs()));
    attrs.Add(XmlNodeKeys::LayoutGroupAttribute, base.GetLayoutGroup());
    if (!base.IsActive()) {
        attrs.Add(XmlNodeKeys::ActiveAttribute, "0");
    }
    if (base.IsFromBase()) {
        attrs.Add(XmlNodeKeys::FromBaseAttribute, "1");
    }
}

void BaseSerializingVisitor::AddCommonModelAttributes(const Model& model, AttrCollector& attrs) {
    attrs.Add(XmlNodeKeys::StartSideAttribute, model.GetStartSide());
    attrs.Add(XmlNodeKeys::DirAttribute, model.GetDirection());
    attrs.Add(XmlNodeKeys::Parm1Attribute, std::to_string(model.GetParm1()));
    attrs.Add(XmlNodeKeys::Parm2Attribute, std::to_string(model.GetParm2()));
    attrs.Add(XmlNodeKeys::Parm3Attribute, std::to_string(model.GetParm3()));
    attrs.Add(XmlNodeKeys::AntialiasAttribute, std::to_string((long)model.GetPixelStyle()));
    attrs.Add(XmlNodeKeys::PixelSizeAttribute, std::to_string(model.GetPixelSize()));
    if (model.GetChanCountPerNode() > 3) {
        attrs.Add(XmlNodeKeys::RGBWHandleAttribute, model.GetRGBWHandling());
    }
    attrs.Add(XmlNodeKeys::StringTypeAttribute, model.GetStringType());
    attrs.Add(XmlNodeKeys::TransparencyAttribute, std::to_string(model.GetTransparency()));
    if (model.GetBlackTransparency() != 0) {
        attrs.Add(XmlNodeKeys::BTransparencyAttribute, std::to_string(model.GetBlackTransparency()));
    }
    if (!model.GetDescription().empty()) {
        attrs.Add(XmlNodeKeys::DescriptionAttribute, model.GetDescription());
    }
    if (model.GetTagColourAsString() != "#000000") {
        attrs.Add(XmlNodeKeys::TagColourAttribute, model.GetTagColourAsString());
    }
    attrs.Add(XmlNodeKeys::StartChannelAttribute, model.GetModelStartChannel());
    if (!model.GetNodeNames().empty()) {
        attrs.Add(XmlNodeKeys::NodeNamesAttribute, model.GetNodeNames());
    }
    if (!model.GetStrandNames().empty()) {
        attrs.Add(XmlNodeKeys::StrandNamesAttribute, model.GetStrandNames());
    }
    if (!model.GetPixelSpacing().empty()) {
        attrs.Add(XmlNodeKeys::PixelSpacingAttribute, model.GetPixelSpacing());
    }
    if (!model.GetPixelCount().empty()) {
        attrs.Add(XmlNodeKeys::PixelCountAttribute, model.GetPixelCount());
    }
    if (!model.GetPixelType().empty()) {
        attrs.Add(XmlNodeKeys::PixelTypeAttribute, model.GetPixelType());
    }
    attrs.Add(XmlNodeKeys::ControllerAttribute, model.GetControllerName());
    attrs.Add(XmlNodeKeys::versionNumberAttribute, CUR_MODEL_POS_VER);
    attrs.Add(XmlNodeKeys::xlightsVersionAttr, xlights_version_string);

    if (!model.GetModelChain().empty()) {
        attrs.Add(XmlNodeKeys::ModelChainAttribute, model.GetModelChain());
    }
    if (model.GetCustomColor() != xlBLACK) {
        attrs.Add(XmlNodeKeys::CustomColorAttribute, model.GetCustomColor());
    }
    if (!model.GetShadowModelFor().empty()) {
        attrs.Add(XmlNodeKeys::ShadowModelAttribute, model.GetShadowModelFor());
    }

    // Individual Start Channels
    if (model.HasIndividualStartChannels()) {
        attrs.Add(XmlNodeKeys::AdvancedAttribute, "1");
        int numStrings = model.GetNumStrings();
        for (int i = 0; i < numStrings; i++) {
            attrs.Add(Model::StartChanAttrName(i), model.GetIndividualStartChannel(i));
        }
    }
}

void BaseSerializingVisitor::AddSuperStrings(const Model& model, AttrCollector& attrs) {
    int num_colors = model.GetNumSuperStringColours();
    if (num_colors == 0) return;
    for (int i = 0; i < num_colors; ++i) {
        std::string key = std::format("SuperStringColour{}", i);
        attrs.Add(key, model.GetSuperStringColour(i));
    }
}

void BaseSerializingVisitor::AddModelScreenLocationAttributes(const BaseObject& base, AttrCollector& attrs) {
    glm::vec3 loc = base.GetBaseObjectScreenLocation().GetWorldPosition();
    attrs.Add(XmlNodeKeys::WorldPosXAttribute, std::to_string(loc.x));
    attrs.Add(XmlNodeKeys::WorldPosYAttribute, std::to_string(loc.y));
    attrs.Add(XmlNodeKeys::WorldPosZAttribute, std::to_string(loc.z));
    if (base.GetBaseObjectScreenLocation().IsLocked()) {
        attrs.Add(XmlNodeKeys::LockedAttribute, "1");
    }
}

void BaseSerializingVisitor::AddBoxedScreenLocationAttributes(const BaseObject& base, AttrCollector& attrs) {
    glm::vec3 scale = base.GetBaseObjectScreenLocation().GetScaleMatrix();
    attrs.Add(XmlNodeKeys::ScaleXAttribute, std::to_string(scale.x));
    attrs.Add(XmlNodeKeys::ScaleYAttribute, std::to_string(scale.y));
    attrs.Add(XmlNodeKeys::ScaleZAttribute, std::to_string(scale.z));
    glm::vec3 rotate = base.GetBaseObjectScreenLocation().GetRotation();
    attrs.Add(XmlNodeKeys::RotateXAttribute, std::to_string(rotate.x));
    attrs.Add(XmlNodeKeys::RotateYAttribute, std::to_string(rotate.y));
    attrs.Add(XmlNodeKeys::RotateZAttribute, std::to_string(rotate.z));
}

void BaseSerializingVisitor::AddTwoPointScreenLocationAttributes(const BaseObject& base, AttrCollector& attrs) {
    const TwoPointScreenLocation& screenLoc = dynamic_cast<const TwoPointScreenLocation&>(base.GetBaseObjectScreenLocation());
    attrs.Add(XmlNodeKeys::X2Attribute, std::to_string(screenLoc.GetX2()));
    attrs.Add(XmlNodeKeys::Y2Attribute, std::to_string(screenLoc.GetY2()));
    attrs.Add(XmlNodeKeys::Z2Attribute, std::to_string(screenLoc.GetZ2()));
}

void BaseSerializingVisitor::AddThreePointScreenLocationAttributes(const BaseObject& base, AttrCollector& attrs) {
    AddTwoPointScreenLocationAttributes(base, attrs);
    glm::vec3 rotate = base.GetBaseObjectScreenLocation().GetRotation();
    attrs.Add(XmlNodeKeys::RotateXAttribute, std::to_string(rotate.x));
    const ThreePointScreenLocation& screenLoc = dynamic_cast<const ThreePointScreenLocation&>(base.GetBaseObjectScreenLocation());
    attrs.Add(XmlNodeKeys::AngleAttribute, std::to_string(screenLoc.GetAngle()));
    attrs.Add(XmlNodeKeys::ShearAttribute, std::to_string(screenLoc.GetYShear()));
    attrs.Add(XmlNodeKeys::HeightAttribute, std::to_string(screenLoc.GetMHeight()));
}

void BaseSerializingVisitor::AddPolyPointScreenLocationAttributes(const BaseObject& base, AttrCollector& attrs) {
    const PolyPointScreenLocation& screenLoc = dynamic_cast<const PolyPointScreenLocation&>(base.GetBaseObjectScreenLocation());
    glm::vec3 scale = base.GetBaseObjectScreenLocation().GetScaleMatrix();
    attrs.Add(XmlNodeKeys::ScaleXAttribute, std::to_string(scale.x));
    attrs.Add(XmlNodeKeys::ScaleYAttribute, std::to_string(scale.y));
    attrs.Add(XmlNodeKeys::ScaleZAttribute, std::to_string(scale.z));
    attrs.Add(XmlNodeKeys::NumPointsAttribute, std::to_string(screenLoc.GetNumPoints()));
    attrs.Add(XmlNodeKeys::PointDataAttribute, screenLoc.GetPointDataAsString());
    attrs.Add(XmlNodeKeys::cPointDataAttribute, screenLoc.GetCurveDataAsString());
}

void BaseSerializingVisitor::AddMultiPointScreenLocationAttributes(const BaseObject& base, AttrCollector& attrs) {
    const PolyPointScreenLocation& screenLoc = dynamic_cast<const PolyPointScreenLocation&>(base.GetBaseObjectScreenLocation());
    attrs.Add(XmlNodeKeys::NumPointsAttribute, std::to_string(screenLoc.GetNumPoints()));
    attrs.Add(XmlNodeKeys::PointDataAttribute, screenLoc.GetPointDataAsString());
}

// ---------------------------------------------------------------------------
// DMX attribute helpers
// ---------------------------------------------------------------------------

void BaseSerializingVisitor::AddColorAbilityRGBAttributes(const DmxColorAbilityRGB* colors, AttrCollector& attrs) {
    attrs.Add(XmlNodeKeys::DmxRedChannelAttribute,   std::to_string(colors->GetRedChannel()));
    attrs.Add(XmlNodeKeys::DmxGreenChannelAttribute, std::to_string(colors->GetGreenChannel()));
    attrs.Add(XmlNodeKeys::DmxBlueChannelAttribute,  std::to_string(colors->GetBlueChannel()));
    attrs.Add(XmlNodeKeys::DmxWhiteChannelAttribute, std::to_string(colors->GetWhiteChannel()));
    if (colors->GetRedBrightness()   != 100) attrs.Add("DmxRedBrightness",   std::to_string(colors->GetRedBrightness()));
    if (colors->GetGreenBrightness() != 100) attrs.Add("DmxGreenBrightness", std::to_string(colors->GetGreenBrightness()));
    if (colors->GetBlueBrightness()  != 100) attrs.Add("DmxBlueBrightness",  std::to_string(colors->GetBlueBrightness()));
    if (colors->GetWhiteBrightness() != 100) attrs.Add("DmxWhiteBrightness", std::to_string(colors->GetWhiteBrightness()));
    if (colors->GetRedGamma()   != 1.0f) attrs.Add("DmxRedGamma",   FloatToString(colors->GetRedGamma()));
    if (colors->GetGreenGamma() != 1.0f) attrs.Add("DmxGreenGamma", FloatToString(colors->GetGreenGamma()));
    if (colors->GetBlueGamma()  != 1.0f) attrs.Add("DmxBlueGamma",  FloatToString(colors->GetBlueGamma()));
    if (colors->GetWhiteGamma() != 1.0f) attrs.Add("DmxWhiteGamma", FloatToString(colors->GetWhiteGamma()));
}

void BaseSerializingVisitor::AddColorWheelAttributes(const DmxColorAbilityWheel* colors, AttrCollector& attrs) {
    attrs.Add(XmlNodeKeys::DmxColorWheelChannelAttribute, std::to_string(colors->GetWheelChannel()));
    attrs.Add(XmlNodeKeys::DmxDimmerChannelAttribute,     std::to_string(colors->GetDimmerChannel()));
    attrs.Add(XmlNodeKeys::DmxColorWheelDelayAttribute,   std::to_string(colors->GetWheelDelay()));
    std::vector<WheelColor> settings = colors->GetWheelColorSettings();
    int index = 0;
    for (const auto& it : settings) {
        attrs.Add(XmlNodeKeys::DmxColorWheelColorAttribute + std::to_string(index), (std::string)it.color);
        attrs.Add(XmlNodeKeys::DmxColorWheelDMXAttribute   + std::to_string(index), std::to_string(it.dmxValue));
        ++index;
    }
}

void BaseSerializingVisitor::AddColorAbilityCMYAttributes(const DmxColorAbilityCMY* colors, AttrCollector& attrs) {
    attrs.Add(XmlNodeKeys::DmxCyanChannelAttribute,    std::to_string(colors->GetCyanChannel()));
    attrs.Add(XmlNodeKeys::DmxMagentaChannelAttribute, std::to_string(colors->GetMagentaChannel()));
    attrs.Add(XmlNodeKeys::DmxYellowChannelAttribute,  std::to_string(colors->GetYellowChannel()));
    attrs.Add(XmlNodeKeys::DmxWhiteChannelAttribute,   std::to_string(colors->GetWhiteChannel()));
}

void BaseSerializingVisitor::AddColorAbilityAttributes(const DmxColorAbility* color_ability, AttrCollector& attrs) {
    auto color_type = color_ability->GetColorType();
    attrs.Add(XmlNodeKeys::DmxColorTypeAttribute, std::to_string((int)color_type));
    if (color_type == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW) {
        const auto* rgb = dynamic_cast<const DmxColorAbilityRGB*>(color_ability);
        if (rgb != nullptr) AddColorAbilityRGBAttributes(rgb, attrs);
    } else if (color_type == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_WHEEL) {
        const auto* wheel = dynamic_cast<const DmxColorAbilityWheel*>(color_ability);
        if (wheel != nullptr) AddColorWheelAttributes(wheel, attrs);
    } else if (color_type == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_CMYW) {
        const auto* cmy = dynamic_cast<const DmxColorAbilityCMY*>(color_ability);
        if (cmy != nullptr) AddColorAbilityCMYAttributes(cmy, attrs);
    }
}

void BaseSerializingVisitor::AddBeamAbilityAttributes(const DmxBeamAbility* beam, AttrCollector& attrs) {
    attrs.Add("DmxBeamLength", std::to_string(beam->GetBeamLength()));
    attrs.Add("DmxBeamWidth",  std::to_string(beam->GetBeamWidth()));
    if (beam->SupportsOrient()) {
        attrs.Add("DmxBeamOrient", std::to_string(beam->GetBeamOrient()));
    }
    if (beam->SupportsYOffset()) {
        attrs.Add("DmxBeamYOffset", std::to_string(beam->GetBeamYOffset()));
    }
}

void BaseSerializingVisitor::AddPresetAbilityAttributes(const DmxPresetAbility* presets, AttrCollector& attrs) {
    auto const& settings = presets->GetPresetSettings();
    int index { 0 };
    for (auto const& it : settings) {
        attrs.Add(XmlNodeKeys::DmxPresetChannelAttribute + std::to_string(index), std::to_string(it.DMXChannel));
        attrs.Add(XmlNodeKeys::DmxPresetValueAttribute   + std::to_string(index), std::to_string(it.DMXValue));
        attrs.Add(XmlNodeKeys::DmxPresetDescAttribute    + std::to_string(index), it.Description);
        ++index;
    }
}

void BaseSerializingVisitor::AddShutterAbilityAttributes(const DmxShutterAbility* shutter, AttrCollector& attrs) {
    attrs.Add(XmlNodeKeys::DmxShutterChannelAttribute, std::to_string(shutter->GetShutterChannel()));
    attrs.Add(XmlNodeKeys::DmxShutterOpenAttribute,    std::to_string(shutter->GetShutterThreshold()));
    attrs.Add(XmlNodeKeys::DmxShutterOnValueAttribute, std::to_string(shutter->GetShutterOnValue()));
}

void BaseSerializingVisitor::AddDimmerAbilityAttributes(const DmxDimmerAbility* dimmer, AttrCollector& attrs) {
    attrs.Add(XmlNodeKeys::MhDimmerChannelAttribute, std::to_string(dimmer->GetDimmerChannel()));
}

void BaseSerializingVisitor::AddDmxModelAttributes(const DmxModel& dmx_model, AttrCollector& attrs) {
    if (dmx_model.HasBeamAbility()) {
        AddBeamAbilityAttributes(dmx_model.GetBeamAbility(), attrs);
    }
    if (dmx_model.HasPresetAbility()) {
        AddPresetAbilityAttributes(dmx_model.GetPresetAbility(), attrs);
    }
    if (dmx_model.HasColorAbility()) {
        AddColorAbilityAttributes(dmx_model.GetColorAbility(), attrs);
    }
    if (dmx_model.HasShutterAbility()) {
        AddShutterAbilityAttributes(dmx_model.GetShutterAbility(), attrs);
    }
    if (dmx_model.HasDimmerAbility()) {
        AddDimmerAbilityAttributes(dmx_model.GetDimmerAbility(), attrs);
    }
    AddBoxedScreenLocationAttributes(dmx_model, attrs);
}

void BaseSerializingVisitor::AddDmxMovingHeadCommAttributes(const DmxMovingHeadComm& model, AttrCollector& attrs) {
    AddDmxModelAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::DmxFixtureAttribute, model.GetFixture());
    // NOTE: Motor child elements are NOT written here. The calling Visit method
    // must write them via WriteDmxMotorElement after calling WriteOpenTag.
}

// ---------------------------------------------------------------------------
// Child-element writers
// ---------------------------------------------------------------------------

void BaseSerializingVisitor::WriteDmxMotorElement(const DmxMotor* motor) {
    AttrCollector attrs;
    attrs.Add(XmlNodeKeys::ChannelCoarseAttribute, std::to_string(motor->GetChannelCoarse()));
    attrs.Add(XmlNodeKeys::ChannelFineAttribute,   std::to_string(motor->GetChannelFine()));
    attrs.Add(XmlNodeKeys::MinLimitAttribute,      std::to_string(motor->GetMinLimit()));
    attrs.Add(XmlNodeKeys::MaxLimitAttribute,      std::to_string(motor->GetMaxLimit()));
    attrs.Add(XmlNodeKeys::RangeOfMotionAttribute, std::to_string(motor->GetRangeOfMotion()));
    attrs.Add(XmlNodeKeys::OrientZeroAttribute,    std::to_string(motor->GetOrientZero()));
    attrs.Add(XmlNodeKeys::OrientHomeAttribute,    std::to_string(motor->GetOrientHome()));
    attrs.Add(XmlNodeKeys::SlewLimitAttribute,     std::to_string(motor->GetSlewLimit()));
    attrs.Add(XmlNodeKeys::ReverseAttribute,       std::to_string(motor->GetReverse()));
    attrs.Add(XmlNodeKeys::UpsideDownAttribute,    std::to_string(motor->GetUpsideDown()));
    WriteOpenTag(motor->GetName(), attrs, true);
}

void BaseSerializingVisitor::WriteMeshElement(const Mesh* mesh) {
    AttrCollector attrs;
    attrs.Add(XmlNodeKeys::ObjFileAttribute,    mesh->GetObjFile());
    attrs.Add(XmlNodeKeys::MeshOnlyAttribute,   std::to_string(mesh->GetMeshOnly()));
    attrs.Add(XmlNodeKeys::BrightnessAttribute, std::to_string(mesh->GetBrightness()));
    attrs.Add(XmlNodeKeys::WidthAttribute,      std::to_string(mesh->GetWidth()));
    attrs.Add(XmlNodeKeys::HeightAttribute,     std::to_string(mesh->GetHeight()));
    attrs.Add(XmlNodeKeys::DepthAttribute,      std::to_string(mesh->GetDepth()));
    attrs.Add(XmlNodeKeys::ScaleXAttribute,     std::to_string(mesh->GetScaleX()));
    attrs.Add(XmlNodeKeys::ScaleYAttribute,     std::to_string(mesh->GetScaleY()));
    attrs.Add(XmlNodeKeys::ScaleZAttribute,     std::to_string(mesh->GetScaleZ()));
    attrs.Add(XmlNodeKeys::RotateXAttribute,    std::to_string(mesh->GetRotateX()));
    attrs.Add(XmlNodeKeys::RotateYAttribute,    std::to_string(mesh->GetRotateY()));
    attrs.Add(XmlNodeKeys::RotateZAttribute,    std::to_string(mesh->GetRotateZ()));
    attrs.Add(XmlNodeKeys::OffsetXAttribute,    std::to_string(mesh->GetOffsetX()));
    attrs.Add(XmlNodeKeys::OffsetYAttribute,    std::to_string(mesh->GetOffsetY()));
    attrs.Add(XmlNodeKeys::OffsetZAttribute,    std::to_string(mesh->GetOffsetZ()));
    WriteOpenTag(mesh->GetName(), attrs, true);
}

void BaseSerializingVisitor::WriteServoElement(const Servo* servo) {
    AttrCollector attrs;
    attrs.Add("Channel",             std::to_string(servo->GetChannel()));
    attrs.Add("MinLimit",            std::to_string(servo->GetMinLimit()));
    attrs.Add("MaxLimit",            std::to_string(servo->GetMaxLimit()));
    attrs.Add("RangeOfMotion",       std::to_string(servo->GetRangeOfMotion()));
    attrs.Add("PivotOffsetX",        std::to_string(servo->GetPivotOffsetX()));
    attrs.Add("PivotOffsetY",        std::to_string(servo->GetPivotOffsetY()));
    attrs.Add("PivotOffsetZ",        std::to_string(servo->GetPivotOffsetZ()));
    attrs.Add("ServoStyle",          servo->GetStyle());
    attrs.Add("ControllerMin",       std::to_string(servo->GetControllerMin()));
    attrs.Add("ControllerMax",       std::to_string(servo->GetControllerMax()));
    attrs.Add("ControllerReverse",   servo->GetControllerReverse() ? "1" : "0");
    attrs.Add("ControllerZeroBehavior", servo->GetControllerZero());
    attrs.Add("ControllerDataType",  servo->GetControllerDataType());
    WriteOpenTag(servo->GetName(), attrs, true);
}

void BaseSerializingVisitor::WriteDmxImageElement(const DmxImage* img) {
    AttrCollector attrs;
    attrs.Add(XmlNodeKeys::ImageAttribute,   img->GetImageFile());
    attrs.Add(XmlNodeKeys::ScaleXAttribute,  std::to_string(img->GetScaleX()));
    attrs.Add(XmlNodeKeys::ScaleYAttribute,  std::to_string(img->GetScaleY()));
    attrs.Add(XmlNodeKeys::ScaleZAttribute,  std::to_string(img->GetScaleZ()));
    attrs.Add(XmlNodeKeys::RotateXAttribute, std::to_string(img->GetRotateX()));
    attrs.Add(XmlNodeKeys::RotateYAttribute, std::to_string(img->GetRotateY()));
    attrs.Add(XmlNodeKeys::RotateZAttribute, std::to_string(img->GetRotateZ()));
    attrs.Add(XmlNodeKeys::OffsetXAttribute, std::to_string(img->GetOffsetX()));
    attrs.Add(XmlNodeKeys::OffsetYAttribute, std::to_string(img->GetOffsetY()));
    attrs.Add(XmlNodeKeys::OffsetZAttribute, std::to_string(img->GetOffsetZ()));
    WriteOpenTag(img->GetName(), attrs, true);
}

void BaseSerializingVisitor::WriteFacesAndStates(const Model* m) {
    FaceStateData faces = m->GetFaceInfo();
    for (const auto& f : faces) {
        AttrCollector attrs;
        attrs.Add(XmlNodeKeys::StateNameAttribute, f.first);
        for (const auto& f2 : f.second) {
            if (!f2.first.empty()) attrs.Add(f2.first, f2.second);
        }
        SortAttributes(attrs);
        WriteOpenTag(XmlNodeKeys::FaceNodeName, attrs, true);
    }

    FaceStateData states = m->GetStateInfo();
    for (const auto& s : states) {
        AttrCollector attrs;
        attrs.Add(XmlNodeKeys::StateNameAttribute, s.first);
        for (const auto& s2 : s.second) {
            if (!s2.first.empty()) attrs.Add(s2.first, s2.second);
        }
        SortAttributes(attrs);
        WriteOpenTag(XmlNodeKeys::StateNodeName, attrs, true);
    }
}

void BaseSerializingVisitor::WriteAliases(const std::list<std::string>& aliases) {
    if (aliases.empty()) return;
    AttrCollector hdrAttrs;
    WriteOpenTag(XmlNodeKeys::AliasesAttribute, hdrAttrs, false);
    for (const auto& a : aliases) {
        AttrCollector attrs;
        attrs.Add(XmlNodeKeys::NameAttribute, a);
        WriteOpenTag(XmlNodeKeys::AliasNodeName, attrs, true);
    }
    WriteCloseTag();
}

void BaseSerializingVisitor::WriteDimmingCurve(const Model* m) {
    std::map<std::string, std::map<std::string, std::string>> dcInfo = m->GetDimmingInfo();
    if (dcInfo.empty()) return;

    AttrCollector hdrAttrs;
    WriteOpenTag(XmlNodeKeys::DimmingNodeName, hdrAttrs, false);
    for (const auto& d1 : dcInfo) {
        AttrCollector attrs;
        for (const auto& d2 : d1.second) {
            attrs.Add(d2.first, d2.second);
        }
        WriteOpenTag(d1.first, attrs, true);
    }
    WriteCloseTag();
}

void BaseSerializingVisitor::WriteSubmodels(const Model* m) {
    const std::vector<Model*>& submodelList = m->GetSubModels();
    for (Model* s : submodelList) {
        const SubModel* submodel = dynamic_cast<const SubModel*>(s);
        if (submodel == nullptr) continue;
        AttrCollector attrs;
        attrs.Add(XmlNodeKeys::NameAttribute,        s->GetName());
        attrs.Add(XmlNodeKeys::LayoutAttribute,      submodel->GetSubModelLayout());
        attrs.Add(XmlNodeKeys::SMTypeAttribute,      submodel->GetSubModelType());
        attrs.Add(XmlNodeKeys::BufferStyleAttribute, submodel->GetSubModelBufferStyle());
        if (submodel->IsRanges()) {
            for (int x = 0; x < submodel->GetNumRanges(); ++x) {
                attrs.Add("line" + std::to_string(x), std::string(submodel->GetRange(x)));
            }
        } else {
            attrs.Add("subBuffer", std::string(submodel->GetSubModelLines()));
        }
        SortAttributes(attrs);

        const auto& subAliases = s->GetAliases();
        if (subAliases.empty()) {
            WriteOpenTag(XmlNodeKeys::SubModelNodeName, attrs, true);
        } else {
            WriteOpenTag(XmlNodeKeys::SubModelNodeName, attrs, false);
            WriteAliases(subAliases);
            WriteCloseTag();
        }
    }
}

void BaseSerializingVisitor::WriteControllerConnectionElement(const Model* m) {
    auto const& cc = m->GetConstCtrlConn();
    int p = cc.GetCtrlPort();
    if (p == 0) return;
    Visit(cc);
}

void BaseSerializingVisitor::WriteOtherElements(const Model* m) {
    WriteFacesAndStates(m);
    WriteControllerConnectionElement(m);
    WriteDimmingCurve(m);
    WriteAliases(m->GetAliases());
    WriteSubmodels(m);
}

void BaseSerializingVisitor::Visit(const ControllerConnection& cc) {
    AttrCollector attrs;
    attrs.Add(XmlNodeKeys::PortAttribute,     std::to_string(cc.GetCtrlPort()));
    attrs.Add(XmlNodeKeys::ProtocolAttribute, cc.GetProtocol());
    if (cc.IsSerialProtocol()) {
        attrs.Add(XmlNodeKeys::ChannelAttribute,       std::to_string(cc.GetDMXChannel()));
        attrs.Add(XmlNodeKeys::ProtocolSpeedAttribute, std::to_string(cc.GetProtocolSpeed()));
    }
    if (cc.GetSmartRemote() && cc.IsPropertySet(CtrlProps::USE_SMART_REMOTE)) {
        attrs.Add(XmlNodeKeys::SmartRemoteAttribute,     std::to_string(cc.GetSmartRemote()));
        attrs.Add(XmlNodeKeys::SRMaxCascadeAttribute,    std::to_string(cc.GetSRMaxCascade()));
        attrs.Add(XmlNodeKeys::SRCascadeOnPortAttribute, cc.GetSRCascadeOnPort() ? "TRUE" : "FALSE");
        attrs.Add(XmlNodeKeys::SmartRemoteTypeAttribute, cc.GetSmartRemoteType());
        if (cc.IsPropertySet(CtrlProps::TS_ACTIVE))
            attrs.Add(XmlNodeKeys::SmartRemoteTsAttribute, std::to_string(cc.GetSmartTs()));
    }
    if (cc.IsPropertySet(CtrlProps::START_NULLS_ACTIVE))
        attrs.Add(XmlNodeKeys::StartNullAttribute, std::to_string(cc.GetStartNulls()));
    if (cc.IsPropertySet(CtrlProps::END_NULLS_ACTIVE))
        attrs.Add(XmlNodeKeys::EndNullAttribute, std::to_string(cc.GetEndNulls()));
    if (cc.IsPropertySet(CtrlProps::BRIGHTNESS_ACTIVE))
        attrs.Add(XmlNodeKeys::DCBrightnessAttribute, std::to_string(cc.GetBrightness()));
    if (cc.IsPropertySet(CtrlProps::GAMMA_ACTIVE))
        attrs.Add(XmlNodeKeys::GammaAttribute, FloatToString(cc.GetGamma()));
    if (cc.IsPropertySet(CtrlProps::COLOR_ORDER_ACTIVE))
        attrs.Add(XmlNodeKeys::ColorOrderAttribute, cc.GetColorOrder());
    if (cc.IsPropertySet(CtrlProps::REVERSE_ACTIVE))
        attrs.Add(XmlNodeKeys::CReverseAttribute, std::to_string(cc.GetReverse()));
    if (cc.IsPropertySet(CtrlProps::GROUP_COUNT_ACTIVE))
        attrs.Add(XmlNodeKeys::GroupCountAttribute, std::to_string(cc.GetGroupCount()));
    if (cc.IsPropertySet(CtrlProps::ZIG_ZAG_ACTIVE))
        attrs.Add(XmlNodeKeys::CZigZagAttribute, std::to_string(cc.GetZigZag()));

    WriteOpenTag(XmlNodeKeys::CtrlConnectionName, attrs, true);
}

// ---------------------------------------------------------------------------
// Common visit preambles
// ---------------------------------------------------------------------------

void BaseSerializingVisitor::CommonVisitSteps(const Model& model, AttrCollector& attrs) {
    AddBaseObjectAttributes(model, attrs);
    AddCommonModelAttributes(model, attrs);
    AddModelScreenLocationAttributes(model, attrs);
    AddSuperStrings(model, attrs);
}

void BaseSerializingVisitor::CommonObjectVisitSteps(const ViewObject& object, AttrCollector& attrs) {
    AddBaseObjectAttributes(object, attrs);
    AddModelScreenLocationAttributes(object, attrs);
}

// ---------------------------------------------------------------------------
// Model Visit() implementations
// ---------------------------------------------------------------------------

void BaseSerializingVisitor::Visit(const ArchesModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddThreePointScreenLocationAttributes(model, attrs);
    if (model.GetZigZag()) {
        attrs.Add(XmlNodeKeys::ZigZagAttribute, "true");
    }
    attrs.Add(XmlNodeKeys::HollowAttribute, std::to_string(model.GetHollow()));
    if (model.GetGap()) {
        attrs.Add(XmlNodeKeys::GapAttribute, std::to_string(model.GetGap()));
    }
    attrs.Add(XmlNodeKeys::CArcAttribute, std::to_string(model.GetArc()));
    attrs.Add(XmlNodeKeys::LayerSizesAttribute, model.SerialiseLayerSizes());
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const CandyCaneModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddThreePointScreenLocationAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::CCHeightAttribute,       std::to_string(model.GetCandyCaneHeight()));
    attrs.Add(XmlNodeKeys::CCReverseAttribute,      model.IsReverse() ? "true" : "false");
    attrs.Add(XmlNodeKeys::CCSticksAttribute,       model.IsSticks() ? "true" : "false");
    attrs.Add(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const CircleModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddBoxedScreenLocationAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::InsideOutAttribute,  model.IsInsideOut() ? "1" : "0");
    attrs.Add(XmlNodeKeys::LayerSizesAttribute, model.SerialiseLayerSizes());
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const ChannelBlockModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddTwoPointScreenLocationAttributes(model, attrs);
    std::vector<std::string> cp = model.GetChannelColors();
    for (auto i = 0; i < (int)cp.size(); i++) {
        attrs.Add(XmlNodeKeys::ChannelColorAttribute + std::to_string(i + 1), cp[i]);
    }
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const CubeModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddBoxedScreenLocationAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::StyleAttribute,          model.GetCubeStyle());
    attrs.Add(XmlNodeKeys::CubeStartAttribute,      model.GetCubeStart());
    attrs.Add(XmlNodeKeys::CubeStringsAttribute,    std::to_string(model.GetCubeStrings()));
    attrs.Add(XmlNodeKeys::StrandPerLineAttribute,  model.GetStrandStyle());
    attrs.Add(XmlNodeKeys::StrandPerLayerAttribute, model.IsStrandPerLayer() ? "TRUE" : "FALSE");
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const CustomModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddBoxedScreenLocationAttributes(model, attrs);
    int depth = model.GetCustomDepth();
    attrs.Add(XmlNodeKeys::CMDepthAttribute, std::to_string(depth));
    std::string custom_data = model.GetCompressedData();
    if (custom_data == xlEMPTY_STRING) {
        custom_data = model.GetCustomData();
        attrs.Add(XmlNodeKeys::CustomModelAttribute, custom_data);
    } else {
        attrs.Add(XmlNodeKeys::CustomModelCmpAttribute, custom_data);
    }
    if (!model.GetCustomBackground().empty()) {
        attrs.Add(XmlNodeKeys::BkgImageAttribute,     model.GetCustomBackground());
        attrs.Add(XmlNodeKeys::BkgLightnessAttribute, std::to_string(model.GetCustomLightness()));
    }
    if (model.HasIndivStartNodes()) {
        int cnt = model.GetIndivStartNodesCount();
        attrs.Add(XmlNodeKeys::CustomStringsAttribute, std::to_string(cnt));
        for (int x = 0; x < cnt; ++x) {
            attrs.Add(std::string(model.StartNodeAttrName(x)), std::to_string(model.GetIndivStartNode(x)));
        }
    }
    if (forExport && RulerObject::GetRuler() != nullptr) {
        float widthmm  = RulerObject::GetRuler()->Convert(RulerObject::GetRuler()->GetUnits(), "mm", RulerObject::GetRuler()->Measure(model.GetModelScreenLocation().GetMWidth()));
        float heightmm = RulerObject::GetRuler()->Convert(RulerObject::GetRuler()->GetUnits(), "mm", RulerObject::GetRuler()->Measure(model.GetModelScreenLocation().GetMHeight()));
        float depthmm  = RulerObject::GetRuler()->Convert(RulerObject::GetRuler()->GetUnits(), "mm", RulerObject::GetRuler()->Measure(model.GetModelScreenLocation().GetMDepth()));
        if (widthmm > 0)               attrs.Add("widthmm",  std::to_string((int)widthmm));
        if (heightmm > 0)              attrs.Add("heightmm", std::to_string((int)heightmm));
        if (depth != 1 && depthmm > 0) attrs.Add("depthmm",  std::to_string((int)depthmm));
    }
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const IciclesModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddThreePointScreenLocationAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
    attrs.Add(XmlNodeKeys::DropPatternAttribute,    model.GetDropPattern());
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const ImageModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddBoxedScreenLocationAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::ImageAttribute,         model.GetImageFile());
    attrs.Add(XmlNodeKeys::WhiteAsAlphaAttribute,  model.IsWhiteAsAlpha() ? "True" : "False");
    attrs.Add(XmlNodeKeys::OffBrightnessAttribute, std::to_string(model.GetOffBrightness()));
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const MatrixModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddBoxedScreenLocationAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::VertMatrixAttribute,    model.isVerticalMatrix() ? "true" : "false");
    attrs.Add(XmlNodeKeys::LowDefinitionAttribute, std::to_string(model.GetLowDefFactor()));
    attrs.Add(XmlNodeKeys::AlternateNodesAttribute,model.HasAlternateNodes() ? "true" : "false");
    attrs.Add(XmlNodeKeys::NoZigZagAttribute,      model.IsNoZigZag() ? "true" : "false");
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const ModelGroup& model) {
    AttrCollector attrs;
    AddBaseObjectAttributes(model, attrs);
    attrs.Add("GridSize",      std::to_string(model.GetGridSize()));
    attrs.Add("layout",        model.GetLayout());
    attrs.Add("DefaultCamera", model.GetDefaultCamera());

    int xOffset = model.GetXCentreOffset();
    int yOffset = model.GetYCentreOffset();
    if (xOffset != 0) attrs.Add("XCentreOffset", std::to_string(xOffset));
    if (yOffset != 0) attrs.Add("YCentreOffset", std::to_string(yOffset));

    std::string modelsStr;
    const auto& modelNames = model.ModelNames();
    for (size_t i = 0; i < modelNames.size(); ++i) {
        if (i > 0) modelsStr += ",";
        modelsStr += modelNames[i];
    }
    attrs.Add("models", modelsStr);

    if (model.GetCentreDefined()) {
        attrs.Add("centreDefined", "1");
        attrs.Add("centrex",       std::to_string(model.GetCentreX()));
        attrs.Add("centrey",       std::to_string(model.GetCentreY()));
        attrs.Add("centreMinx",    std::to_string(model.GetCentreMinx()));
        attrs.Add("centreMiny",    std::to_string(model.GetCentreMiny()));
        attrs.Add("centreMaxx",    std::to_string(model.GetCentreMaxx()));
        attrs.Add("centreMaxy",    std::to_string(model.GetCentreMaxy()));
    }
    if (model.GetTagColourAsString() != "#000000") {
        attrs.Add(XmlNodeKeys::TagColourAttribute, model.GetTagColourAsString());
    }

    SortAttributes(attrs);
    WriteOpenTag("modelGroup", attrs, false);
    WriteAliases(model.GetAliases());
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const MultiPointModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddMultiPointScreenLocationAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::MultiStringsAttribute, std::to_string(model.GetNumStrings()));
    attrs.Add(XmlNodeKeys::ModelHeightAttribute,  std::to_string(model.GetModelHeight()));
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const SingleLineModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddTwoPointScreenLocationAttributes(model, attrs);
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const PolyLineModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddPolyPointScreenLocationAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::PolyStringsAttribute,    std::to_string(model.GetNumStrings()));
    attrs.Add(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
    attrs.Add(XmlNodeKeys::DropPatternAttribute,    model.GetDropPattern());
    std::vector<int> nodeSize = model.GetNodeSizes();
    for (auto i = 0; i < (int)nodeSize.size(); i++) {
        attrs.Add(std::string(model.StartNodeAttrName(i)), std::to_string(nodeSize[i]));
    }
    if (!model.GetAutoDistribute()) {
        std::vector<int> segSize = model.GetSegmentsSizes();
        for (auto i = 0; i < (int)segSize.size(); i++) {
            attrs.Add(std::string(model.SegAttrName(i)), std::to_string(segSize[i]));
        }
    }
    std::vector<std::string> cSize = model.GetCorners();
    for (auto i = 0; i < (int)cSize.size(); i++) {
        attrs.Add(std::string(model.CornerAttrName(i)), cSize[i]);
    }
    attrs.Add(XmlNodeKeys::SegsExpandedAttribute, model.AreSegsExpanded() ? "TRUE" : "FALSE");
    attrs.Add(XmlNodeKeys::ModelHeightAttribute,  std::to_string(model.GetModelHeight()));
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const SphereModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddBoxedScreenLocationAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::DegreesAttribute,        std::to_string(model.GetSphereDegrees()));
    attrs.Add(XmlNodeKeys::StartLatAttribute,       std::to_string(model.GetStartLatitude()));
    attrs.Add(XmlNodeKeys::EndLatAttribute,         std::to_string(model.GetEndLatitude()));
    attrs.Add(XmlNodeKeys::LowDefinitionAttribute,  std::to_string(model.GetLowDefFactor()));
    attrs.Add(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
    attrs.Add(XmlNodeKeys::NoZigZagAttribute,       model.IsNoZigZag() ? "true" : "false");
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const SpinnerModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddBoxedScreenLocationAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::AlternateAttribute,  model.HasAlternateNodes() ? "true" : "false");
    attrs.Add(XmlNodeKeys::ZigZagAttribute,     model.HasZigZag() ? "true" : "false");
    attrs.Add(XmlNodeKeys::HollowAttribute,     std::to_string(model.GetHollowPercent()));
    attrs.Add(XmlNodeKeys::ArcAttribute,        std::to_string(model.GetArcAngle()));
    attrs.Add(XmlNodeKeys::StartAngleAttribute, std::to_string(model.GetStartAngle()));
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const StarModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddBoxedScreenLocationAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::LayerSizesAttribute,        model.SerialiseLayerSizes());
    attrs.Add(XmlNodeKeys::StarStartLocationAttribute, model.GetStartLocation());
    attrs.Add(XmlNodeKeys::StarRatioAttribute,         std::to_string(model.GetStarRatio()));
    attrs.Add(XmlNodeKeys::StarCenterPercentAttribute, std::to_string(model.GetInnerPercent()));
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const TreeModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddBoxedScreenLocationAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::AlternateNodesAttribute,      model.HasAlternateNodes() ? "true" : "false");
    attrs.Add(XmlNodeKeys::NoZigZagAttribute,            model.IsNoZigZag() ? "true" : "false");
    attrs.Add(XmlNodeKeys::StrandDirAttribute,           model.isVerticalMatrix() ? "Vertical" : "Horizontal");
    attrs.Add(XmlNodeKeys::exportFirstStrandAttribute,   std::to_string(model.GetFirstStrand() + 1));
    attrs.Add(XmlNodeKeys::TreeBottomTopRatioAttribute,  std::to_string(model.GetBottomTopRatio()));
    attrs.Add(XmlNodeKeys::TreePerspectiveAttribute,     std::to_string(model.GetTreePerspective()));
    attrs.Add(XmlNodeKeys::TreeSpiralRotationsAttribute, std::to_string(model.GetSpiralRotations()));
    attrs.Add(XmlNodeKeys::TreeRotationAttribute,        std::to_string(model.GetTreeRotation()));
    attrs.Add(XmlNodeKeys::TreeTypeAttribute,            std::to_string(model.GetTreeType()));
    attrs.Add(XmlNodeKeys::TreeDegreesAttribute,         std::to_string(model.GetTreeDegrees()));
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const WindowFrameModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddBoxedScreenLocationAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::RotationAttribute, model.GetRotation() ? "Counter Clockwise" : "Clockwise");
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const WreathModel& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddBoxedScreenLocationAttributes(model, attrs);
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const DmxFloodArea& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddDmxModelAttributes(model, attrs);
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const DmxFloodlight& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddDmxModelAttributes(model, attrs);
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const DmxGeneral& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddDmxModelAttributes(model, attrs);
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const DmxMovingHeadAdv& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddDmxMovingHeadCommAttributes(model, attrs);
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteDmxMotorElement(model.GetPanMotor());
    WriteDmxMotorElement(model.GetTiltMotor());
    WriteMeshElement(model.GetBaseMesh());
    WriteMeshElement(model.GetYokeMesh());
    WriteMeshElement(model.GetHeadMesh());
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const DmxMovingHead& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddDmxMovingHeadCommAttributes(model, attrs);
    attrs.Add(XmlNodeKeys::DmxStyleAttribute, model.GetDMXStyle());
    attrs.Add(XmlNodeKeys::HideBodyAttribute, model.GetHideBody() ? "True" : "False");
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteDmxMotorElement(model.GetPanMotor());
    WriteDmxMotorElement(model.GetTiltMotor());
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const DmxServo& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddDmxModelAttributes(model, attrs);
    attrs.Add("NumServos",                        std::to_string(model.GetNumServos()));
    attrs.Add("Bits16",                           model.Is16Bit() ? "1" : "0");
    attrs.Add(XmlNodeKeys::BrightnessAttribute,   std::to_string(model.GetBrightness()));
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    for (int i = 0; i < model.GetNumServos(); ++i) {
        WriteServoElement(model.GetServo(i));
        WriteDmxImageElement(model.GetStaticImage(i));
        WriteDmxImageElement(model.GetMotionImage(i));
    }
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const DmxServo3d& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddDmxModelAttributes(model, attrs);
    attrs.Add("NumServos", std::to_string(model.GetNumServos()));
    attrs.Add("NumStatic", std::to_string(model.GetNumStatic()));
    attrs.Add("NumMotion", std::to_string(model.GetNumMotion()));
    attrs.Add("Bits16",    model.Is16Bit() ? "1" : "0");
    attrs.Add(XmlNodeKeys::BrightnessAttribute, std::to_string(model.GetBrightness()));
    // Servo/mesh linkage attributes go on the parent model element — collect all
    // before WriteOpenTag because the element cannot be re-opened after writing.
    for (int i = 0; i < model.GetNumServos(); ++i) {
        std::string num = std::to_string(i + 1);
        attrs.Add("Servo" + num + "Linkage", "Mesh " + std::to_string(model.GetServoLink(i) + 1));
        attrs.Add("Mesh"  + num + "Linkage", "Mesh " + std::to_string(model.GetMeshLink(i) + 1));
    }
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    for (int i = 0; i < model.GetNumServos(); ++i) {
        WriteServoElement(model.GetServo(i));
    }
    for (int i = 0; i < model.GetNumStatic(); ++i) {
        WriteMeshElement(model.GetStaticMesh(i));
    }
    for (int i = 0; i < model.GetNumMotion(); ++i) {
        WriteMeshElement(model.GetMotionMesh(i));
    }
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

void BaseSerializingVisitor::Visit(const DmxSkull& model) {
    AttrCollector attrs;
    CommonVisitSteps(model, attrs);
    AddDmxModelAttributes(model, attrs);
    attrs.Add("DmxEyeBrtChannel", std::to_string(model.GetEyeBrightnessChannel()));
    attrs.Add("HasJaw",           model.HasJaw()   ? "1" : "0");
    attrs.Add("HasPan",           model.HasPan()   ? "1" : "0");
    attrs.Add("HasTilt",          model.HasTilt()  ? "1" : "0");
    attrs.Add("HasNod",           model.HasNod()   ? "1" : "0");
    attrs.Add("HasEyeUD",         model.HasEyeUD() ? "1" : "0");
    attrs.Add("HasEyeLR",         model.HasEyeLR() ? "1" : "0");
    attrs.Add("HasColor",         model.HasColor() ? "1" : "0");
    attrs.Add("Bits16",           model.Is16Bit()  ? "1" : "0");
    attrs.Add("MeshOnly",         model.IsMeshOnly() ? "1" : "0");
    attrs.Add("DmxJawOrient",     std::to_string(model.GetJawOrient()));
    attrs.Add("DmxPanOrient",     std::to_string(model.GetPanOrient()));
    attrs.Add("DmxTiltOrient",    std::to_string(model.GetTiltOrient()));
    attrs.Add("DmxNodOrient",     std::to_string(model.GetNodOrient()));
    attrs.Add("DmxEyeUDOrient",   std::to_string(model.GetEyeUDOrient()));
    attrs.Add("DmxEyeLROrient",   std::to_string(model.GetEyeLROrient()));
    SortAttributes(attrs);
    WriteOpenTag(XmlNodeKeys::ModelNodeName, attrs, false);
    WriteServoElement(model.GetJawServo());
    WriteServoElement(model.GetPanServo());
    WriteServoElement(model.GetTiltServo());
    WriteServoElement(model.GetNodServo());
    WriteServoElement(model.GetEyeUDServo());
    WriteServoElement(model.GetEyeLRServo());
    WriteMeshElement(model.GetHeadMesh());
    WriteMeshElement(model.GetJawMesh());
    WriteMeshElement(model.GetEyeLMesh());
    WriteMeshElement(model.GetEyeRMesh());
    WriteOtherElements(dynamic_cast<const Model*>(&model));
    WriteCloseTag();
}

// ---------------------------------------------------------------------------
// ViewObject Visit() implementations
// ---------------------------------------------------------------------------

void BaseSerializingVisitor::Visit(const GridlinesObject& object) {
    AttrCollector attrs;
    CommonObjectVisitSteps(object, attrs);
    AddBoxedScreenLocationAttributes(object, attrs);
    attrs.Add("GridLineSpacing", std::to_string(object.GetGridLineSpacing()));
    attrs.Add("GridWidth",       std::to_string(object.GetGridWidth()));
    attrs.Add("GridHeight",      std::to_string(object.GetGridHeight()));
    attrs.Add("GridColor",       object.GetGridColor());
    attrs.Add("GridAxis",        object.GetHasAxis() ? "1" : "0");
    attrs.Add("PointToFront",    object.GetPointToFront() ? "1" : "0");
    WriteOpenTag(XmlNodeKeys::ViewObjectNodeName, attrs, true);
}

void BaseSerializingVisitor::Visit(const TerrainObject& object) {
    AttrCollector attrs;
    CommonObjectVisitSteps(object, attrs);
    AddBoxedScreenLocationAttributes(object, attrs);
    attrs.Add(XmlNodeKeys::ImageAttribute,        object.GetImageFile());
    attrs.Add(XmlNodeKeys::TransparencyAttribute, std::to_string(object.GetTransparency()));
    attrs.Add(XmlNodeKeys::BrightnessAttribute,   std::to_string(object.GetBrightness()));
    attrs.Add(XmlNodeKeys::TerrainLineAttribute,  std::to_string(object.GetSpacing()));
    attrs.Add(XmlNodeKeys::TerrainWidthAttribute, std::to_string(object.GetWidth()));
    attrs.Add(XmlNodeKeys::TerrainDepthAttribute, std::to_string(object.GetDepth()));
    attrs.Add(XmlNodeKeys::HideGridAttribute,     object.IsHideGrid()  ? "1" : "0");
    attrs.Add(XmlNodeKeys::HideImageAttribute,    object.IsHideImage() ? "1" : "0");
    attrs.Add(XmlNodeKeys::GridColorAttribute,    object.GetGridColor());
    const TerrainScreenLocation& screenLoc = dynamic_cast<const TerrainScreenLocation&>(object.GetBaseObjectScreenLocation());
    attrs.Add(XmlNodeKeys::PointDataAttribute,    screenLoc.GetDataAsString());
    WriteOpenTag(XmlNodeKeys::ViewObjectNodeName, attrs, true);
}

void BaseSerializingVisitor::Visit(const ImageObject& object) {
    AttrCollector attrs;
    CommonObjectVisitSteps(object, attrs);
    AddBoxedScreenLocationAttributes(object, attrs);
    attrs.Add(XmlNodeKeys::ImageAttribute,        object.GetImageFile());
    attrs.Add(XmlNodeKeys::TransparencyAttribute, std::to_string(object.GetTransparency()));
    attrs.Add(XmlNodeKeys::BrightnessAttribute,   std::to_string(object.GetBrightness()));
    WriteOpenTag(XmlNodeKeys::ViewObjectNodeName, attrs, true);
}

void BaseSerializingVisitor::Visit(const MeshObject& object) {
    AttrCollector attrs;
    CommonObjectVisitSteps(object, attrs);
    AddBoxedScreenLocationAttributes(object, attrs);
    attrs.Add(XmlNodeKeys::ObjFileAttribute,    object.GetObjFile());
    attrs.Add(XmlNodeKeys::MeshOnlyAttribute,   object.IsMeshOnly() ? "1" : "0");
    attrs.Add(XmlNodeKeys::BrightnessAttribute, std::to_string(object.GetBrightness()));
    WriteOpenTag(XmlNodeKeys::ViewObjectNodeName, attrs, true);
}

void BaseSerializingVisitor::Visit(const RulerObject& object) {
    AttrCollector attrs;
    CommonObjectVisitSteps(object, attrs);
    AddTwoPointScreenLocationAttributes(object, attrs);
    attrs.Add(XmlNodeKeys::UnitsAttribute,  std::to_string(object.GetUnits()));
    attrs.Add(XmlNodeKeys::LengthAttribute, std::to_string(object.GetLength()));
    WriteOpenTag(XmlNodeKeys::ViewObjectNodeName, attrs, true);
}

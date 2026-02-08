/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "XmlSerializingVisitor.h"

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
#include "../LayoutGroup.h"
#include "../models/MatrixModel.h"
#include "../models/MultiPointModel.h"
#include "../models/MeshObject.h"
#include "../models/ModelManager.h"
#include "../models/Model.h"
#include "../models/ModelGroup.h"
#include "../models/PolyLineModel.h"
#include "../models/RulerObject.h"
#include "../SequenceViewManager.h"
#include "../models/SingleLineModel.h"
#include "../models/SphereModel.h"
#include "../models/SpinnerModel.h"
#include "../models/StarModel.h"
#include "../models/SubModel.h"
#include "../models/TerrainObject.h"
#include "../models/TerrainScreenLocation.h"
#include "../models/ThreePointScreenLocation.h"
#include "../models/TreeModel.h"
#include "../models/ViewObjectManager.h"
#include "../models/WindowFrameModel.h"
#include "../models/WreathModel.h"
#include "../xLightsVersion.h"
#include "../xLightsMain.h"
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

#include <wx/xml/xml.h>

#include "XmlNodeKeys.h"
#include "XmlSerializeFunctions.h"

void XmlSerializingVisitor::AddBaseObjectAttributes(const BaseObject& base, wxXmlNode* node) {
    node->AddAttribute(XmlNodeKeys::NameAttribute, base.GetName());
    node->AddAttribute(XmlNodeKeys::DisplayAsAttribute, base.GetDisplayAs());
    node->AddAttribute(XmlNodeKeys::LayoutGroupAttribute, base.GetLayoutGroup());
    node->AddAttribute(XmlNodeKeys::ActiveAttribute, std::to_string(base.IsActive()));
}

void XmlSerializingVisitor::AddCommonModelAttributes(const Model& model, wxXmlNode* node) {
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
}

void XmlSerializingVisitor::AddModelScreenLocationAttributes(const BaseObject& base, wxXmlNode* node) {
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

void XmlSerializingVisitor::AddTwoPointScreenLocationAttributes(const BaseObject& base, wxXmlNode* node) {
    const TwoPointScreenLocation& screenLoc = dynamic_cast<const TwoPointScreenLocation&>(base.GetBaseObjectScreenLocation());
    float x2 = screenLoc.GetX2();
    float y2 = screenLoc.GetY2();
    float z2 = screenLoc.GetZ2();
    node->AddAttribute(XmlNodeKeys::X2Attribute, std::to_string(x2));
    node->AddAttribute(XmlNodeKeys::Y2Attribute, std::to_string(y2));
    node->AddAttribute(XmlNodeKeys::Z2Attribute, std::to_string(z2));
}

void XmlSerializingVisitor::AddThreePointScreenLocationAttributes(const BaseObject& base, wxXmlNode* node) {
    AddTwoPointScreenLocationAttributes( base, node );
    const ThreePointScreenLocation& screenLoc = dynamic_cast<const ThreePointScreenLocation&>(base.GetBaseObjectScreenLocation());
    int angle = screenLoc.GetAngle();
    node->AddAttribute(XmlNodeKeys::AngleAttribute, std::to_string(angle));
    float shear = screenLoc.GetYShear();
    node->AddAttribute(XmlNodeKeys::ShearAttribute, std::to_string(shear));
    float height = screenLoc.GetMHeight();
    node->AddAttribute(XmlNodeKeys::HeightAttribute, std::to_string(height));
}

void XmlSerializingVisitor::AddPolyPointScreenLocationAttributes(const BaseObject& base, wxXmlNode* node) {
    const PolyPointScreenLocation& screenLoc = dynamic_cast<const PolyPointScreenLocation&>(base.GetBaseObjectScreenLocation());
    node->AddAttribute(XmlNodeKeys::NumPointsAttribute, std::to_string(screenLoc.GetNumPoints()));
    node->AddAttribute(XmlNodeKeys::PointDataAttribute, screenLoc.GetPointDataAsString());
    node->AddAttribute(XmlNodeKeys::cPointDataAttribute, screenLoc.GetCurveDataAsString());
}

void XmlSerializingVisitor::AddMultiPointScreenLocationAttributes(const BaseObject& base, wxXmlNode* node) {
    const PolyPointScreenLocation& screenLoc = dynamic_cast<const PolyPointScreenLocation&>(base.GetBaseObjectScreenLocation());
    node->AddAttribute(XmlNodeKeys::NumPointsAttribute, std::to_string(screenLoc.GetNumPoints()));
    node->AddAttribute(XmlNodeKeys::PointDataAttribute, screenLoc.GetPointDataAsString());
}

void XmlSerializingVisitor::AddDmxMotorAttributes(const DmxMotor* motor, wxXmlNode* node) {
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

void XmlSerializingVisitor::AddMeshAttributes(const Mesh* mesh, wxXmlNode* node) {
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

void XmlSerializingVisitor::AddServoAttributes(const Servo* servo, wxXmlNode* node) {
    wxXmlNode* servo_node = new wxXmlNode(wxXML_ELEMENT_NODE, servo->GetName());
    servo_node->AddAttribute("Channel", std::to_string(servo->GetChannel()));
    servo_node->AddAttribute("MinLimit", std::to_string(servo->GetMinLimit()));
    servo_node->AddAttribute("MaxLimit", std::to_string(servo->GetMaxLimit()));
    servo_node->AddAttribute("RangeOfMotion", std::to_string(servo->GetRangeOfMotion()));
    servo_node->AddAttribute("PivotOffsetX", std::to_string(servo->GetPivotOffsetX()));
    servo_node->AddAttribute("PivotOffsetY", std::to_string(servo->GetPivotOffsetY()));
    servo_node->AddAttribute("PivotOffsetZ", std::to_string(servo->GetPivotOffsetZ()));
    servo_node->AddAttribute("ServoStyle", servo->GetStyle());
    servo_node->AddAttribute("ControllerMin", std::to_string(servo->GetControllerMin()));
    servo_node->AddAttribute("ControllerMax", std::to_string(servo->GetControllerMax()));
    servo_node->AddAttribute("ControllerReverse", std::to_string(servo->GetControllerMax()));
    servo_node->AddAttribute("ControllerReverse", servo->GetControllerReverse() ? "1" : "0");
    servo_node->AddAttribute("ControllerZeroBehavior", servo->GetControllerZero());
    servo_node->AddAttribute("ControllerDataType", servo->GetControllerDataType());
    node->AddChild(servo_node);
}

void XmlSerializingVisitor::AddDmxImageAttributes(const DmxImage* img, wxXmlNode* node) {
    wxXmlNode* img_node = new wxXmlNode(wxXML_ELEMENT_NODE, img->GetName());
    img_node->AddAttribute(XmlNodeKeys::ImageAttribute, img->GetImageFile());
    img_node->AddAttribute(XmlNodeKeys::ScaleXAttribute, std::to_string(img->GetScaleX()));
    img_node->AddAttribute(XmlNodeKeys::ScaleYAttribute, std::to_string(img->GetScaleY()));
    img_node->AddAttribute(XmlNodeKeys::ScaleZAttribute, std::to_string(img->GetScaleZ()));
    img_node->AddAttribute(XmlNodeKeys::RotateXAttribute, std::to_string(img->GetRotateX()));
    img_node->AddAttribute(XmlNodeKeys::RotateYAttribute, std::to_string(img->GetRotateY()));
    img_node->AddAttribute(XmlNodeKeys::RotateZAttribute, std::to_string(img->GetRotateZ()));
    img_node->AddAttribute(XmlNodeKeys::OffsetXAttribute, std::to_string(img->GetOffsetX()));
    img_node->AddAttribute(XmlNodeKeys::OffsetYAttribute, std::to_string(img->GetOffsetY()));
    img_node->AddAttribute(XmlNodeKeys::OffsetZAttribute, std::to_string(img->GetOffsetZ()));
}

void XmlSerializingVisitor::AddDmxMovingHeadCommAttributes(const DmxMovingHeadComm& model, wxXmlNode* node) {
    AddDmxModelAttributes(model, node);
    node->AddAttribute(XmlNodeKeys::DmxFixtureAttribute, model.GetFixture());
    AddDmxMotorAttributes(reinterpret_cast<DmxMotor*>(model.GetPanMotor()), node);
    AddDmxMotorAttributes(reinterpret_cast<DmxMotor*>(model.GetTiltMotor()), node);
}

void XmlSerializingVisitor::AddDmxModelAttributes(const DmxModel& dmx_model, wxXmlNode* node) {
    if (dmx_model.HasBeamAbility()) {
        AddBeamAbilityAttributes(dmx_model.GetBeamAbility(), node);
    }
    if (dmx_model.HasPresetAbility()) {
        AddPresetAbilityAttributes(dmx_model.GetPresetAbility(), node);
    }
    if (dmx_model.HasColorAbility()) {
        AddColorAbilityAttributes(dmx_model.GetColorAbility(), node);
    }
    if (dmx_model.HasShutterAbility()) {
        AddShutterAbilityAttributes(dmx_model.GetShutterAbility(), node);
    }
    if (dmx_model.HasDimmerAbility()) {
        AddDimmerAbilityAttributes(dmx_model.GetDimmerAbility(), node);
    }
}

void XmlSerializingVisitor::AddColorAbilityRGBAttributes(const DmxColorAbilityRGB* colors, wxXmlNode* node) {
    node->AddAttribute(XmlNodeKeys::DmxRedChannelAttribute, std::to_string(colors->GetRedChannel()));
    node->AddAttribute(XmlNodeKeys::DmxGreenChannelAttribute, std::to_string(colors->GetGreenChannel()));
    node->AddAttribute(XmlNodeKeys::DmxBlueChannelAttribute, std::to_string(colors->GetBlueChannel()));
    node->AddAttribute(XmlNodeKeys::DmxWhiteChannelAttribute, std::to_string(colors->GetWhiteChannel()));
}

void XmlSerializingVisitor::AddColorWheelAttributes(const DmxColorAbilityWheel* colors, wxXmlNode* node) {
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

void XmlSerializingVisitor::AddColorAbilityCMYAttributes(const DmxColorAbilityCMY* colors, wxXmlNode* node) {
    node->AddAttribute(XmlNodeKeys::DmxCyanChannelAttribute, std::to_string(colors->GetCyanChannel()));
    node->AddAttribute(XmlNodeKeys::DmxMagentaChannelAttribute, std::to_string(colors->GetMagentaChannel()));
    node->AddAttribute(XmlNodeKeys::DmxYellowChannelAttribute, std::to_string(colors->GetYellowChannel()));
    node->AddAttribute(XmlNodeKeys::DmxWhiteChannelAttribute, std::to_string(colors->GetWhiteChannel()));
}

void XmlSerializingVisitor::AddColorAbilityAttributes(const DmxColorAbility* color_ability, wxXmlNode* node) {
    std::string color_type = color_ability->GetTypeName();
    node->AddAttribute(XmlNodeKeys::DmxColorTypeAttribute, color_type);
    if (color_type == "RGBW") {
        AddColorAbilityRGBAttributes(dynamic_cast<const DmxColorAbilityRGB*>(color_ability), node);
    } else if (color_type == "ColorWheel") {
        AddColorWheelAttributes(dynamic_cast<const DmxColorAbilityWheel*>(color_ability), node);
    } else if (color_type == "CMYW") {
        AddColorAbilityCMYAttributes(dynamic_cast<const DmxColorAbilityCMY*>(color_ability), node);
    }
}

void XmlSerializingVisitor::AddBeamAbilityAttributes(const DmxBeamAbility* beam, wxXmlNode* node) {
    node->AddAttribute("DmxBeamLength", std::to_string(beam->GetBeamLength()));
    node->AddAttribute("DmxBeamWidth", std::to_string(beam->GetBeamWidth()));
    if (beam->SupportsOrient()) {
        node->AddAttribute("DmxBeamOrient", std::to_string(beam->GetBeamOrient()));
    }
    if (beam->SupportsYOffset()) {
        node->AddAttribute("DmxBeamYOffset", std::to_string(beam->GetBeamYOffset()));
    }
}

void XmlSerializingVisitor::AddPresetAbilityAttributes(const DmxPresetAbility* presets, wxXmlNode* node) {
    auto const& settings = presets->GetPresetSettings();
    int index { 0 };
    for (auto const& it : settings) {
        node->AddAttribute(XmlNodeKeys::DmxPresetChannelAttribute + std::to_string(index), std::to_string(it.DMXChannel));
        node->AddAttribute(XmlNodeKeys::DmxPresetValueAttribute + std::to_string(index), std::to_string(it.DMXValue));
        node->AddAttribute(XmlNodeKeys::DmxPresetDescAttribute + std::to_string(index), it.Description);
        ++index;
    }
}

void XmlSerializingVisitor::AddShutterAbilityAttributes(const DmxShutterAbility* shutter, wxXmlNode* node) {
    node->AddAttribute(XmlNodeKeys::DmxShutterChannelAttribute, std::to_string(shutter->GetShutterChannel()));
    node->AddAttribute(XmlNodeKeys::DmxShutterOpenAttribute, std::to_string(shutter->GetShutterThreshold()));
    node->AddAttribute(XmlNodeKeys::DmxShutterOnValueAttribute, std::to_string(shutter->GetShutterOnValue()));
}

void XmlSerializingVisitor::AddDimmerAbilityAttributes(const DmxDimmerAbility* dimmer, wxXmlNode* node) {
    node->AddAttribute(XmlNodeKeys::MhDimmerChannelAttribute, std::to_string(dimmer->GetDimmerChannel()));
}

void XmlSerializingVisitor::SortAttributes(wxXmlNode* input) {
    const wxString attributeToPrioritize = "name";
    const wxString attributeToPrioritizeSecond = "DisplayAs";
    const wxString attributeToDePrioritize = "CustomModel";
    std::vector<std::pair<std::string, std::string>> attributes;
    for (wxXmlAttribute* attr = input->GetAttributes(); attr != nullptr; attr = attr->GetNext()) {
        attributes.push_back({ std::string(attr->GetName()), std::string(attr->GetValue()) });
    }
    auto custom_comparator = [&attributeToPrioritize, &attributeToPrioritizeSecond, &attributeToDePrioritize](const std::pair<wxString, wxString>& a, const std::pair<wxString, wxString>& b) {
        if (a.first == b.first) return false;
        if (a.first == attributeToPrioritize) return true;
        if (b.first == attributeToPrioritize) return false;
        if (a.first == attributeToPrioritizeSecond ) return true;
        if (b.first == attributeToPrioritizeSecond) return false;
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

void XmlSerializingVisitor::AddFacesandStates(wxXmlNode* node, const Model* m) {

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

void XmlSerializingVisitor::AddAliases(wxXmlNode* node, const std::list<std::string>& aliases) {
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

void XmlSerializingVisitor::AddDimmingCurve(wxXmlNode* node, const Model* m) {
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

void XmlSerializingVisitor::AddSubmodels(wxXmlNode* node, const Model* m) {
    const std::vector<Model*>& submodelList = m->GetSubModels();
    
    for (Model* s : submodelList) {
        wxXmlNode* sm_node = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::SubModelNodeName);
        const SubModel* submodel = dynamic_cast<const SubModel*>(s);
        if (submodel == nullptr) return;
        sm_node->AddAttribute(XmlNodeKeys::NameAttribute, s->GetName());
        sm_node->AddAttribute(XmlNodeKeys::LayoutAttribute, submodel->GetSubModelLayout());
        sm_node->AddAttribute(XmlNodeKeys::SMTypeAttribute, submodel->GetSubModelType());
        sm_node->AddAttribute(XmlNodeKeys::BufferStyleAttribute, submodel->GetSubModelBufferStyle());
        if (submodel->IsRanges()) {
            for (int x = 0; x < submodel->GetNumRanges(); ++x) {
                sm_node->AddAttribute(wxString::Format("line%d", x), submodel->GetRange(x));
            }
        } else {
            sm_node->AddAttribute("subBuffer", submodel->GetSubModelLines());
        }
        SortAttributes(sm_node);
        AddAliases(sm_node, s->GetAliases());
        node->AddChild(sm_node);
    }
}

void XmlSerializingVisitor::AddGroups(wxXmlNode* node, const Model* m) {
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

void XmlSerializingVisitor::AddControllerConnection(wxXmlNode* node, const Model* m) {
    auto const& cc = m->GetConstCtrlConn();
    int p = cc.GetCtrlPort();
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

void XmlSerializingVisitor::AddDimensions(wxXmlNode* node, const Model* m) {
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

void XmlSerializingVisitor::AddOtherElements(wxXmlNode* xmlNode, const Model* m)
{
    SortAttributes(xmlNode);
    AddFacesandStates(xmlNode, m);
    AddControllerConnection(xmlNode, m);
    AddDimmingCurve(xmlNode,m);
    AddAliases(xmlNode, m->GetAliases());
    AddSubmodels(xmlNode, m);
    AddGroups(xmlNode, m);
    AddDimensions(xmlNode, m);
}

void XmlSerializingVisitor::AddSuperStrings(Model const& model, wxXmlNode* node) {
    int num_colors = model.GetNumSuperStringColours();
    if (num_colors == 0) return;
    for (int i = 0; i < num_colors; ++i) {
        std::string key = std::format("SuperStringColour{}",i);
        node->AddAttribute(key, model.GetSuperStringColour(i));
    }
}

wxXmlNode* XmlSerializingVisitor::CommonVisitSteps(const Model& model) {
    wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelNodeName);
    AddBaseObjectAttributes(model, xmlNode);
    AddCommonModelAttributes(model, xmlNode);
    AddModelScreenLocationAttributes(model, xmlNode);
    AddSuperStrings(model, xmlNode);
    parentNode->AddChild(xmlNode);
    return xmlNode;
}

void XmlSerializingVisitor::Visit(const ArchesModel& model) {
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

void XmlSerializingVisitor::Visit(const CandyCaneModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    AddThreePointScreenLocationAttributes(model, xmlNode);
    xmlNode->AddAttribute(XmlNodeKeys::CCHeightAttribute, std::to_string(model.GetCandyCaneHeight()));
    xmlNode->AddAttribute(XmlNodeKeys::CCReverseAttribute, model.IsReverse() ? "true" : "false");
    xmlNode->AddAttribute(XmlNodeKeys::CCSticksAttribute, model.IsSticks() ? "true": "false");
    xmlNode->AddAttribute(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const CircleModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    xmlNode->AddAttribute(XmlNodeKeys::InsideOutAttribute, model.IsInsideOut() ? "1" : "0");
    xmlNode->AddAttribute(XmlNodeKeys::LayerSizesAttribute,  model.SerialiseLayerSizes());
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const ChannelBlockModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    AddTwoPointScreenLocationAttributes(model, xmlNode);
    std::vector<std::string> cp = model.GetChannelColors();
    for (auto i = 0; i < cp.size();  i++) {
        xmlNode->AddAttribute(XmlNodeKeys::ChannelColorAttribute + std::to_string(i+1), cp[i]);
    }
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const CubeModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    xmlNode->AddAttribute(XmlNodeKeys::StyleAttribute, model.GetCubeStyle());
    xmlNode->AddAttribute(XmlNodeKeys::CubeStartAttribute, model.GetCubeStart());
    xmlNode->AddAttribute(XmlNodeKeys::CubeStringsAttribute, std::to_string(model.GetCubeStrings()));
    xmlNode->AddAttribute(XmlNodeKeys::StrandPerLineAttribute, model.GetStrandStyle());
    xmlNode->AddAttribute(XmlNodeKeys::StrandPerLayerAttribute, model.IsStrandPerLayer() ? "TRUE" : "FALSE");
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const CustomModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    int depth = model.GetCustomDepth();
    xmlNode->AddAttribute(XmlNodeKeys::CMDepthAttribute, std::to_string(depth));
    std::string custom_data = model.GetCompressedData();
    if (custom_data == xlEMPTY_STRING) {
        custom_data = model.GetCustomData();
        xmlNode->AddAttribute(XmlNodeKeys::CustomModelAttribute, custom_data);
    } else {
        xmlNode->AddAttribute(XmlNodeKeys::CustomModelCmpAttribute, custom_data);
    }
    xmlNode->AddAttribute(XmlNodeKeys::BkgImageAttribute, model.GetCustomBackground());
    xmlNode->AddAttribute(XmlNodeKeys::BkgLightnessAttribute, std::to_string(model.GetCustomLightness()));

    // If we have a ruler then also include the model dimensions so if imported we can bring them in as the right size
    if (RulerObject::GetRuler() != nullptr)
    {
        float widthmm = RulerObject::GetRuler()->Convert(RulerObject::GetRuler()->GetUnits(), "mm", RulerObject::GetRuler()->Measure(model.GetModelScreenLocation().GetMWidth()));
        float heightmm = RulerObject::GetRuler()->Convert(RulerObject::GetRuler()->GetUnits(), "mm", RulerObject::GetRuler()->Measure(model.GetModelScreenLocation().GetMHeight()));
        float depthmm = RulerObject::GetRuler()->Convert(RulerObject::GetRuler()->GetUnits(), "mm", RulerObject::GetRuler()->Measure(model.GetModelScreenLocation().GetMDepth()));
        if (widthmm > 0) { xmlNode->AddAttribute("widthmm", std::to_string((int)widthmm)); }
        if (heightmm > 0) { xmlNode->AddAttribute("heightmm", std::to_string((int)heightmm)); }
        if (depth != 1 && depthmm > 0) { xmlNode->AddAttribute("depthmm", std::to_string((int)depthmm)); }
    }

    const Model* m = dynamic_cast<const Model*>(&model);
    //AddCustomModel(xmlNode, model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const IciclesModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    AddThreePointScreenLocationAttributes(model, xmlNode);
    xmlNode->AddAttribute(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
    xmlNode->AddAttribute(XmlNodeKeys::DropPatternAttribute, model.GetDropPattern());
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const ImageModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    xmlNode->AddAttribute(XmlNodeKeys::ImageAttribute, model.GetImageFile());
    xmlNode->AddAttribute(XmlNodeKeys::WhiteAsAlphaAttribute, model.IsWhiteAsAlpha() ? "True" : "False");
    xmlNode->AddAttribute(XmlNodeKeys::OffBrightnessAttribute, std::to_string(model.GetOffBrightness()));
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const MatrixModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    xmlNode->AddAttribute(XmlNodeKeys::VertMatrixAttribute, model.isVerticalMatrix() ? "true" : "false");
    xmlNode->AddAttribute(XmlNodeKeys::LowDefinitionAttribute, std::to_string(model.GetLowDefFactor()));
    xmlNode->AddAttribute(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
    xmlNode->AddAttribute(XmlNodeKeys::NoZigZagAttribute, model.IsNoZigZag() ? "true" : "false");
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const MultiPointModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    AddMultiPointScreenLocationAttributes(model, xmlNode);
    xmlNode->AddAttribute(XmlNodeKeys::MultiStringsAttribute, std::to_string(model.GetNumStrings()));
    xmlNode->AddAttribute(XmlNodeKeys::ModelHeightAttribute, std::to_string(model.GetModelHeight()));
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const SingleLineModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    AddTwoPointScreenLocationAttributes(model, xmlNode);
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const PolyLineModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    AddPolyPointScreenLocationAttributes(model, xmlNode);
    xmlNode->AddAttribute(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
    xmlNode->AddAttribute(XmlNodeKeys::DropPatternAttribute, model.GetDropPattern());
    std::vector<int> nodeSize = model.GetNodeSizes();
    for (auto i = 0; i < nodeSize.size(); i++) {
        xmlNode->AddAttribute(model.SegAttrName(i), std::to_string(nodeSize[i]));
    }
    std::vector<int> segSize = model.GetSegmentsSizes();
    for (auto i = 0; i < segSize.size(); i++) {
        xmlNode->AddAttribute(model.StartNodeAttrName(i), std::to_string(segSize[i]));
    }
    std::vector<std::string> cSize = model.GetCorners();
    for (auto i = 0; i < cSize.size(); i++) {
        xmlNode->AddAttribute(model.CornerAttrName(i), cSize[i]);
    }
    xmlNode->AddAttribute(XmlNodeKeys::SegsExpandedAttribute, model.AreSegsExpanded() ? "TRUE" : "FALSE");
    xmlNode->AddAttribute(XmlNodeKeys::ModelHeightAttribute, std::to_string(model.GetHeight()));
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const SphereModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    xmlNode->AddAttribute(XmlNodeKeys::DegreesAttribute, std::to_string(model.GetSphereDegrees()));
    xmlNode->AddAttribute(XmlNodeKeys::StartLatAttribute, std::to_string(model.GetStartLatitude()));
    xmlNode->AddAttribute(XmlNodeKeys::EndLatAttribute, std::to_string(model.GetEndLatitude()));
    xmlNode->AddAttribute(XmlNodeKeys::LowDefinitionAttribute, std::to_string(model.GetLowDefFactor()));
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const SpinnerModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    xmlNode->AddAttribute(XmlNodeKeys::AlternateAttribute, model.HasAlternateNodes() ? "true" : "false");
    xmlNode->AddAttribute(XmlNodeKeys::ZigZagAttribute, model.HasZigZag() ? "true" : "false");
    xmlNode->AddAttribute(XmlNodeKeys::HallowAttribute, std::to_string(model.GetHollowPercent()));
    xmlNode->AddAttribute(XmlNodeKeys::ArcAngleAttribute, std::to_string(model.GetArcAngle()));
    xmlNode->AddAttribute(XmlNodeKeys::StartAngleAttribute, std::to_string(model.GetStartAngle()));
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const StarModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    xmlNode->AddAttribute(XmlNodeKeys::LayerSizesAttribute, model.SerialiseLayerSizes());
    xmlNode->AddAttribute(XmlNodeKeys::StarStartLocationAttribute, model.GetStartLocation());
    xmlNode->AddAttribute(XmlNodeKeys::StarRatioAttribute, std::to_string(model.GetStarRatio()));
    xmlNode->AddAttribute(XmlNodeKeys::StarCenterPercentAttribute, std::to_string(model.GetInnerPercent()));
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const TreeModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    xmlNode->AddAttribute(XmlNodeKeys::AlternateNodesAttribute, model.HasAlternateNodes() ? "true" : "false");
    xmlNode->AddAttribute(XmlNodeKeys::NoZigZagAttribute, model.IsNoZigZag() ? "true" : "false");
    xmlNode->AddAttribute(XmlNodeKeys::TreeBottomTopRatioAttribute, std::to_string(model.GetBottomTopRatio()));
    xmlNode->AddAttribute(XmlNodeKeys::TreePerspectiveAttribute, std::to_string(model.GetTreePerspective()));
    xmlNode->AddAttribute(XmlNodeKeys::TreeSpiralRotationsAttribute, std::to_string(model.GetSpiralRotations()));
    xmlNode->AddAttribute(XmlNodeKeys::TreeRotationAttribute, std::to_string(model.GetTreeRotation()));
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const WindowFrameModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    xmlNode->AddAttribute(XmlNodeKeys::RotationAttribute, model.GetRotation() ? "Counter Clockwise" : "Clockwise");
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const WreathModel& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const DmxFloodArea& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    AddDmxModelAttributes(model, xmlNode);
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const DmxFloodlight& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    AddDmxModelAttributes(model, xmlNode);
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const DmxGeneral& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    AddDmxModelAttributes(model, xmlNode);
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const DmxMovingHeadAdv& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    AddDmxMovingHeadCommAttributes(model, xmlNode);
    AddMeshAttributes(reinterpret_cast<Mesh*>(model.GetBaseMesh()), xmlNode);
    AddMeshAttributes(reinterpret_cast<Mesh*>(model.GetYokeMesh()), xmlNode);
    AddMeshAttributes(reinterpret_cast<Mesh*>(model.GetHeadMesh()), xmlNode);
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const DmxMovingHead& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    AddDmxMovingHeadCommAttributes(model, xmlNode);
    xmlNode->AddAttribute(XmlNodeKeys::DmxStyleAttribute, model.GetDMXStyle());
    xmlNode->AddAttribute(XmlNodeKeys::HideBodyAttribute, std::to_string(model.GetHideBody()));
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const DmxServo& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    AddDmxModelAttributes(model, xmlNode);
    xmlNode->AddAttribute("NumServos", std::to_string(model.GetNumServos()));
    xmlNode->AddAttribute("Bits16", model.Is16Bit() ? "1" : "0");
    xmlNode->AddAttribute(XmlNodeKeys::BrightnessAttribute, std::to_string(model.GetBrightness()));
    xmlNode->AddAttribute(XmlNodeKeys::TransparencyAttribute, std::to_string(model.GetTransparency()));
    for (int i = 0; i < model.GetNumServos(); ++i) {
        AddServoAttributes(model.GetServo(i), xmlNode);
        AddDmxImageAttributes(model.GetStaticImage(i), xmlNode);
        AddDmxImageAttributes(model.GetMotionImage(i), xmlNode);
    }
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const DmxServo3d& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    AddDmxModelAttributes(model, xmlNode);
    xmlNode->AddAttribute("NumServos", std::to_string(model.GetNumServos()));
    xmlNode->AddAttribute("NumStatic", std::to_string(model.GetNumStatic()));
    xmlNode->AddAttribute("NumMotion", std::to_string(model.GetNumMotion()));
    xmlNode->AddAttribute("Bits16", model.Is16Bit() ? "1" : "0");
    xmlNode->AddAttribute(XmlNodeKeys::BrightnessAttribute, std::to_string(model.GetBrightness()));
    for (int i = 0; i < model.GetNumServos(); ++i) {
        AddServoAttributes(model.GetServo(i), xmlNode);
        std::string num = std::to_string(i + 1);
        std::string servo_link = "Servo" + num + "Linkage";
        std::string mesh_link = "Mesh" + num + "Linkage";
        xmlNode->AddAttribute(servo_link, std::to_string(model.GetServoLink(i)));
        xmlNode->AddAttribute(mesh_link, std::to_string(model.GetMeshLink(i)));
    }
    for (int i = 0; i < model.GetNumStatic(); ++i) {
        AddMeshAttributes(model.GetStaticMesh(i), xmlNode);
    }
    for (int i = 0; i < model.GetNumMotion(); ++i) {
        AddMeshAttributes(model.GetMotionMesh(i), xmlNode);
    }
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

void XmlSerializingVisitor::Visit(const DmxSkull& model) {
    wxXmlNode* xmlNode = CommonVisitSteps(model);
    AddDmxModelAttributes(model, xmlNode);
    xmlNode->AddAttribute("DmxEyeBrtChannel", std::to_string(model.GetEyeBrightnessChannel()));
    xmlNode->AddAttribute("HasJaw", model.HasJaw() ? "1" : "0");
    xmlNode->AddAttribute("HasPan", model.HasPan() ? "1" : "0");
    xmlNode->AddAttribute("HasTilt", model.HasTilt() ? "1" : "0");
    xmlNode->AddAttribute("HasNod", model.HasNod() ? "1" : "0");
    xmlNode->AddAttribute("HasEyeUD", model.HasEyeUD() ? "1" : "0");
    xmlNode->AddAttribute("HasEyeLR", model.HasEyeLR() ? "1" : "0");
    xmlNode->AddAttribute("HasColor", model.HasColor() ? "1" : "0");
    xmlNode->AddAttribute("Bits16", model.Is16Bit() ? "1" : "0");
    xmlNode->AddAttribute("MeshOnly", model.IsMeshOnly() ? "1" : "0");
    xmlNode->AddAttribute("DmxJawOrient", std::to_string(model.GetJawOrient()));
    xmlNode->AddAttribute("DmxPanOrient", std::to_string(model.GetPanOrient()));
    xmlNode->AddAttribute("DmxTiltOrient", std::to_string(model.GetTiltOrient()));
    xmlNode->AddAttribute("DmxNodOrient", std::to_string(model.GetNodOrient()));
    xmlNode->AddAttribute("DmxEyeUDOrient", std::to_string(model.GetEyeUDOrient()));
    xmlNode->AddAttribute("DmxEyeLROrient", std::to_string(model.GetEyeLROrient()));
    
    AddServoAttributes(model.GetJawServo(), xmlNode);
    AddServoAttributes(model.GetPanServo(), xmlNode);
    AddServoAttributes(model.GetTiltServo(), xmlNode);
    AddServoAttributes(model.GetNodServo(), xmlNode);
    AddServoAttributes(model.GetEyeUDServo(), xmlNode);
    AddServoAttributes(model.GetEyeLRServo(), xmlNode);
    
    AddMeshAttributes(model.GetHeadMesh(), xmlNode);
    AddMeshAttributes(model.GetJawMesh(), xmlNode);
    AddMeshAttributes(model.GetEyeLMesh(), xmlNode);
    AddMeshAttributes(model.GetEyeRMesh(), xmlNode);
    
    const Model* m = dynamic_cast<const Model*>(&model);
    AddOtherElements(xmlNode, m);
}

wxXmlNode* XmlSerializingVisitor::CommonObjectVisitSteps(const ViewObject& object) {
    wxXmlNode* xmlNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ViewObjectNodeName);
    AddBaseObjectAttributes(object, xmlNode);
    AddModelScreenLocationAttributes(object, xmlNode);
    parentNode->AddChild(xmlNode);
    return xmlNode;
}

void XmlSerializingVisitor::Visit(const GridlinesObject& object)
{
    wxXmlNode* xmlNode = CommonObjectVisitSteps(object);
    xmlNode->AddAttribute("GridLineSpacing", std::to_string(object.GetGridLineSpacing()));
    xmlNode->AddAttribute("GridWidth", std::to_string(object.GetGridWidth()));
    xmlNode->AddAttribute("GridHeight", std::to_string(object.GetGridHeight()));
    xmlNode->AddAttribute("GridColor", object.GetGridColor());
    xmlNode->AddAttribute("GridAxis", object.GetHasAxis() ? "1" : "0");
    xmlNode->AddAttribute("PointToFront", object.GetPointToFront() ? "1" : "0");
}

void XmlSerializingVisitor::Visit(const TerrainObject& object)
{
    wxXmlNode* xmlNode = CommonObjectVisitSteps(object);
    xmlNode->AddAttribute(XmlNodeKeys::ImageAttribute, object.GetImageFile());
    xmlNode->AddAttribute(XmlNodeKeys::TransparencyAttribute, std::to_string(object.GetTransparency()));
    xmlNode->AddAttribute(XmlNodeKeys::BrightnessAttribute, std::to_string(object.GetBrightness()));
    xmlNode->AddAttribute(XmlNodeKeys::TerrainLineAttribute, std::to_string(object.GetSpacing()));
    xmlNode->AddAttribute(XmlNodeKeys::TerrainWidthAttribute, std::to_string(object.GetWidth()));
    xmlNode->AddAttribute(XmlNodeKeys::TerrainDepthAttribute, std::to_string(object.GetDepth()));
    xmlNode->AddAttribute(XmlNodeKeys::TerrainDepthAttribute, std::to_string(object.GetDepth()));
    xmlNode->AddAttribute(XmlNodeKeys::HideGridAttribute, object.IsHideGrid() ? "1" : "0");
    xmlNode->AddAttribute(XmlNodeKeys::HideImageAttribute, object.IsHideImage() ? "1" : "0");
    xmlNode->AddAttribute(XmlNodeKeys::ImageAttribute, object.GetGridColor());
    const TerrainScreenLocation& screenLoc = dynamic_cast<const TerrainScreenLocation&>(object.GetBaseObjectScreenLocation());
    xmlNode->AddAttribute(XmlNodeKeys::PointDataAttribute, screenLoc.GetDataAsString());
}

void XmlSerializingVisitor::Visit(const ImageObject& object)
{
    wxXmlNode* xmlNode = CommonObjectVisitSteps(object);
    xmlNode->AddAttribute(XmlNodeKeys::ImageAttribute, object.GetImageFile());
    xmlNode->AddAttribute(XmlNodeKeys::TransparencyAttribute, std::to_string(object.GetTransparency()));
    xmlNode->AddAttribute(XmlNodeKeys::BrightnessAttribute, std::to_string(object.GetBrightness()));
}

void XmlSerializingVisitor::Visit(const MeshObject& object)
{
    wxXmlNode* xmlNode = CommonObjectVisitSteps(object);
    xmlNode->AddAttribute(XmlNodeKeys::ObjFileAttribute, object.GetObjFile());
    xmlNode->AddAttribute(XmlNodeKeys::MeshOnlyAttribute, object.IsMeshOnly() ? "1" : "0");
    xmlNode->AddAttribute(XmlNodeKeys::BrightnessAttribute, std::to_string(object.GetBrightness()));
}

void XmlSerializingVisitor::Visit(const RulerObject& object)
{
    wxXmlNode* xmlNode = CommonObjectVisitSteps(object);
    AddTwoPointScreenLocationAttributes(object, xmlNode);
    xmlNode->AddAttribute(XmlNodeKeys::UnitsAttribute, std::to_string(object.GetUnits()));
    xmlNode->AddAttribute(XmlNodeKeys::LengthAttribute, std::to_string(object.GetLength()));
}

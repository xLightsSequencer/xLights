/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "XmlDeserializingModelFactory.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif
#include "XmlNodeKeys.h"
#include "XmlSerializeFunctions.h"
#include "DimmingCurve.h"
#include "UtilFunctions.h"
#include "../models/ArchesModel.h"
#include "../models/CandyCaneModel.h"
#include "../models/ChannelBlockModel.h"
#include "../models/CircleModel.h"
#include "../models/CubeModel.h"
#include "../models/CustomModel.h"
#include "../models/IciclesModel.h"
#include "../models/ImageModel.h"
#include "../models/LabelModel.h"
#include "../models/MatrixModel.h"
#include "../models/ModelGroup.h"
#include "../models/MultiPointModel.h"
#include "../models/PolyLineModel.h"
#include "../models/RulerObject.h"
#include "../models/SingleLineModel.h"
#include "../models/SphereModel.h"
#include "../models/SpinnerModel.h"
#include "../models/StarModel.h"
#include "../models/SubModel.h"
#include "../models/TreeModel.h"
#include "../models/WindowFrameModel.h"
#include "../models/WreathModel.h"
#include "../models/DMX/DmxBeamAbility.h"
#include "../models/DMX/DmxColorAbilityCMY.h"
#include "../models/DMX/DmxColorAbilityRGB.h"
#include "../models/DMX/DmxColorAbilityWheel.h"
#include "../models/DMX/DmxPresetAbility.h"
#include "../models/DMX/DmxDimmerAbility.h"
#include "../models/DMX/DmxShutterAbility.h"
#include "../models/DMX/DmxFloodArea.h"
#include "../models/DMX/DmxFloodlight.h"
#include "../models/DMX/DmxGeneral.h"
#include "../models/DMX/DmxImage.h"
#include "../models/DMX/DmxMovingHeadAdv.h"
#include "../models/DMX/DmxMovingHead.h"
#include "../models/DMX/DmxServo.h"
#include "../models/DMX/DmxServo3D.h"
#include "../models/DMX/DmxSkull.h"
#include "../models/DMX/Mesh.h"
#include "../models/DMX/Servo.h"
#include "../models/ModelManager.h"
#include "../render/UICallbacks.h"
#include "../utils/string_utils.h"
#include "../utils/FileUtils.h"
#include <cstring>
#include <set>
#include <string_view>

using namespace XmlSerialize;

// Helper to read an attribute, falling back to a legacy attribute name
static std::string GetAttrWithLegacyFallback(pugi::xml_node node, const char* newAttr, const char* legacyAttr, const char* defaultVal = "") {
    if (!node.attribute(newAttr).empty()) {
        return node.attribute(newAttr).as_string(defaultVal);
    }
    return node.attribute(legacyAttr).as_string(defaultVal);
}

// Helper to check if either the new or legacy attribute name exists
static bool HasAttrOrLegacy(pugi::xml_node node, const char* newAttr, const char* legacyAttr) {
    return !node.attribute(newAttr).empty() || !node.attribute(legacyAttr).empty();
}

// Helper to read a new named attribute, falling back to a legacy parm attribute
static long ReadAttrWithParmFallback(pugi::xml_node node, const char* newAttr, const char* parmAttr, const char* defaultVal) {
    if (!node.attribute(newAttr).empty()) {
        return std::strtol(node.attribute(newAttr).as_string(defaultVal), nullptr, 10);
    }
    return std::strtol(node.attribute(parmAttr).as_string(defaultVal), nullptr, 10);
}

// Helper to read an integer attribute, falling back to a legacy attribute name
static int ReadIntAttrWithLegacyFallback(pugi::xml_node node, const char* newAttr, const char* legacyAttr, int defaultVal) {
    if (!node.attribute(newAttr).empty()) {
        return node.attribute(newAttr).as_int(defaultVal);
    }
    return node.attribute(legacyAttr).as_int(defaultVal);
}

Model* XmlDeserializingModelFactory::Deserialize(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    std::string type = node.attribute(XmlNodeKeys::DisplayAsAttribute).as_string("DisplayAs Missing");

    if (type.empty()) {
        throw std::runtime_error("Model has an empty DisplayAs attribute");
    }

    std::string node_name = node.name();  // need this to support importing old models that did not have the DisplayAs attribute

    Model* model = nullptr;
    if (type == XmlNodeKeys::ArchesType || node_name == "archesmodel") {
        model = DeserializeArches(node, modelManager, importing);
    } else if (type == XmlNodeKeys::CandyCaneType) {
        model = DeserializeCandyCane(node, modelManager, importing);
    } else if (type == XmlNodeKeys::ChannelBlockType) {
        model = DeserializeChannelBlock(node, modelManager, importing);
    } else if (type == XmlNodeKeys::CircleType || node_name == "circlemodel") {
        model = DeserializeCircle(node, modelManager, importing);
    } else if (type == XmlNodeKeys::CubeType || node_name == "Cubemodel") {
        model = DeserializeCube(node, modelManager, importing);
    } else if (type == XmlNodeKeys::CustomType || node_name == "custommodel") {
        model = DeserializeCustom(node, modelManager, importing);
    } else if (type == XmlNodeKeys::DmxMovingHeadType) {
        model = DeserializeDmxMovingHead(node, modelManager, importing);
    } else if (type == XmlNodeKeys::DmxMovingHeadAdvType) {
        model = DeserializeDmxMovingHeadAdv(node, modelManager, importing);
    } else if (type == XmlNodeKeys::DmxFloodAreaType) {
        model = DeserializeDmxFloodArea(node, modelManager, importing);
    } else if (type == XmlNodeKeys::DmxFloodlightType) {
        model = DeserializeDmxFloodlight(node, modelManager, importing);
    } else if (type == XmlNodeKeys::DmxGeneralType || node_name == "dmxgeneral") {
        model = DeserializeDmxGeneral(node, modelManager, importing);
    } else if (type == XmlNodeKeys::DmxServoType || node_name == "dmxservo") {
        model = DeserializeDmxServo(node, modelManager, importing);
    } else if (type == XmlNodeKeys::DmxServo3dType || node_name == "dmxservo3d") {
        model = DeserializeDmxServo3d(node, modelManager, importing);
    } else if (type == XmlNodeKeys::DmxSkullType) {
        model = DeserializeDmxSkull(node, modelManager, importing);
    } else if (type == XmlNodeKeys::IciclesType || node_name == "iciclemodel") {
        model = DeserializeIcicles(node, modelManager, importing);
    } else if (type == XmlNodeKeys::ImageType) {
        model = DeserializeImage(node, modelManager, importing);
    } else if (type == XmlNodeKeys::LabelType) {
        model = DeserializeLabel(node, modelManager, importing);
    } else if (type.find(XmlNodeKeys::MatrixType) != std::string::npos || node_name == "matrixmodel") {
        model = DeserializeMatrix(node, modelManager, importing);
    } else if (type == XmlNodeKeys::ModelGroupType || node_name == "modelGroup") {
        model = DeserializeModelGroup(node, modelManager, importing);
    } else if (type.find(XmlNodeKeys::MultiPointType) != std::string::npos || node_name == "multipointmodel") {
        model = DeserializeMultiPoint(node, modelManager, importing);
    } else if (type == XmlNodeKeys::SingleLineType) {
        model = DeserializeSingleLine(node, modelManager, importing);
    } else if (type == XmlNodeKeys::PolyLineType || node_name == "polylinemodel") {
        model = DeserializePolyLine(node, modelManager, importing);
    } else if (type == XmlNodeKeys::SphereType || node_name == "spheremodel") {
        model = DeserializeSphere(node, modelManager, importing);
    } else if (type == XmlNodeKeys::SpinnerType) {
        model = DeserializeSpinner(node, modelManager, importing);
    } else if (type == XmlNodeKeys::StarType || node_name == "starmodel") {
        model = DeserializeStar(node, modelManager, importing);
    } else if (type.find(XmlNodeKeys::TreeType) != std::string::npos || node_name == "treemodel") {
        model = DeserializeTree(node, modelManager, importing);
    } else if (type == XmlNodeKeys::WindowType) {
        model = DeserializeWindow(node, modelManager, importing);
    } else if (type == XmlNodeKeys::WreathType) {
        model = DeserializeWreath(node, modelManager, importing);
    } else {
        throw std::runtime_error("Unknown model type: " + type);
    }

    // Apply real-world dimensions after Setup() has initialized RenderWi/RenderHt.
    // This must happen here rather than in DeserializeCommonModelChildElements because
    // SetMWidth/SetMHeight divide by (RenderWi-1) / (RenderHt-1), and those values are
    // only correct after each Deserialize* function has called model->Setup().
    if (model != nullptr && importing && RulerObject::GetRuler() != nullptr) {
        pugi::xml_node dimNode = node.child(XmlNodeKeys::DimNodeName);
        if (dimNode) {
            std::string units = dimNode.attribute(XmlNodeKeys::DimUnitsAttribute).as_string("mm");
            float width  = dimNode.attribute(XmlNodeKeys::DimWidthAttribute).as_float(0);
            float height = dimNode.attribute(XmlNodeKeys::DimHeightAttribute).as_float(0);
            float depth  = dimNode.attribute(XmlNodeKeys::DimDepthAttribute).as_float(0);
            model->ApplyDimensions(units, width, height, depth);
            modelManager.SetUsedRuler();
        }
    }

    return model;
}

void XmlDeserializingModelFactory::CommonDeserializeSteps(Model* model, pugi::xml_node node, ModelManager& modelManager, bool importing) {
    DeserializeBaseObjectAttributes(model, node, modelManager, importing);
    DeserializeCommonModelAttributes(model, node, modelManager, importing);
    DeserializeModelScreenLocationAttributes(model, node, importing);
    DeserializeSuperStrings(model, node);
    DeserializeCommonModelChildElements(model, node, modelManager, importing);
}

void XmlDeserializingModelFactory::DeserializeControllerConnection(Model* model, pugi::xml_node ccNode) {
    auto& cc = model->GetCtrlConn();
    cc.SetDMXChannel(ccNode.attribute(XmlNodeKeys::ChannelAttribute).as_int(1));
    cc.SetProtocol(ccNode.attribute(XmlNodeKeys::ProtocolAttribute).as_string(""));
    cc.SetSerialProtocolSpeed(ccNode.attribute(XmlNodeKeys::ProtocolSpeedAttribute).as_int(CtrlDefs::DEFAULT_PROTOCOL_SPEED));
    cc.SetCtrlPort(ccNode.attribute(XmlNodeKeys::PortAttribute).as_int(CtrlDefs::DEFAULT_PORT));
    cc.SetBrightness(ReadIntAttrWithLegacyFallback(ccNode, XmlNodeKeys::BrightnessAttribute, XmlNodeKeys::DCBrightnessAttribute, CtrlDefs::DEFAULT_BRIGHTNESS));
    cc.SetStartNulls(ccNode.attribute(XmlNodeKeys::StartNullAttribute).as_int(CtrlDefs::DEFAULT_NULLS));
    cc.SetEndNulls(ccNode.attribute(XmlNodeKeys::EndNullAttribute).as_int(CtrlDefs::DEFAULT_NULLS));
    cc.SetColorOrder(ccNode.attribute(XmlNodeKeys::ColorOrderAttribute).as_string(CtrlDefs::DEFAULT_COLOR_ORDER.data()));
    cc.SetGroupCount(ccNode.attribute(XmlNodeKeys::GroupCountAttribute).as_int(CtrlDefs::DEFAULT_GROUP_COUNT));
    cc.SetGamma(ccNode.attribute(XmlNodeKeys::GammaAttribute).as_float(CtrlDefs::DEFAULT_GAMMA));
    cc.SetReverse(ReadIntAttrWithLegacyFallback(ccNode, XmlNodeKeys::ReverseAttribute, XmlNodeKeys::CReverseAttribute, CtrlDefs::DEFAULT_REVERSE));
    cc.SetZigZag(ReadIntAttrWithLegacyFallback(ccNode, XmlNodeKeys::ZigZagAttribute, XmlNodeKeys::CZigZagAttribute, CtrlDefs::DEFAULT_ZIGZAG));

    // Set all the property checkbox active states
    cc.UpdateProperty(CtrlProps::START_NULLS_ACTIVE, !ccNode.attribute(XmlNodeKeys::StartNullAttribute).empty());
    cc.UpdateProperty(CtrlProps::END_NULLS_ACTIVE,   !ccNode.attribute(XmlNodeKeys::EndNullAttribute).empty());
    cc.UpdateProperty(CtrlProps::BRIGHTNESS_ACTIVE,  HasAttrOrLegacy(ccNode, XmlNodeKeys::BrightnessAttribute, XmlNodeKeys::DCBrightnessAttribute));
    cc.UpdateProperty(CtrlProps::GAMMA_ACTIVE,       !ccNode.attribute(XmlNodeKeys::GammaAttribute).empty());
    cc.UpdateProperty(CtrlProps::COLOR_ORDER_ACTIVE, !ccNode.attribute(XmlNodeKeys::ColorOrderAttribute).empty());
    cc.UpdateProperty(CtrlProps::REVERSE_ACTIVE,     HasAttrOrLegacy(ccNode, XmlNodeKeys::ReverseAttribute, XmlNodeKeys::CReverseAttribute));
    cc.UpdateProperty(CtrlProps::GROUP_COUNT_ACTIVE, !ccNode.attribute(XmlNodeKeys::GroupCountAttribute).empty());
    cc.UpdateProperty(CtrlProps::ZIG_ZAG_ACTIVE,     HasAttrOrLegacy(ccNode, XmlNodeKeys::ZigZagAttribute, XmlNodeKeys::CZigZagAttribute));
    cc.UpdateProperty(CtrlProps::TS_ACTIVE,          !ccNode.attribute(XmlNodeKeys::SmartRemoteTsAttribute).empty());

    // Set all the Smart Remote values
    cc.LoadSmartRemote(ccNode.attribute(XmlNodeKeys::SmartRemoteAttribute).as_int(0));
    cc.SetSRMaxCascade(ccNode.attribute(XmlNodeKeys::SRMaxCascadeAttribute).as_int(1));
    cc.SetSRCascadeOnPort(std::string_view(ccNode.attribute(XmlNodeKeys::SRCascadeOnPortAttribute).as_string("FALSE")) == "TRUE");
    cc.SetSmartRemoteTs(ccNode.attribute(XmlNodeKeys::SmartRemoteTsAttribute).as_int(0));
    cc.SetSmartRemoteType(ccNode.attribute(XmlNodeKeys::SmartRemoteTypeAttribute).as_string(""));

    cc.UpdateProperty(CtrlProps::USE_SMART_REMOTE, cc.GetSmartRemote());
}

void XmlDeserializingModelFactory::DeserializeBaseObjectAttributes(Model* model, pugi::xml_node node, ModelManager& modelManager, bool importing) {
    std::string name = Trim(node.attribute("name").as_string());
    if (importing)
    {
        name = modelManager.GenerateModelName(name);
        model->SetLayoutGroup("Unassigned", true);
    } else {
        model->SetLayoutGroup(node.attribute(XmlNodeKeys::LayoutGroupAttribute).as_string("Unassigned"), true);
    }
    model->SetName(name);
    model->SetActive(node.attribute(XmlNodeKeys::ActiveAttribute).as_int(1));
    model->SetFromBase(node.attribute(XmlNodeKeys::FromBaseAttribute).as_int(0));
}

void XmlDeserializingModelFactory::DeserializeCommonModelAttributes(Model* model, pugi::xml_node node, ModelManager& modelManager, bool importing) {
    if (!node.attribute(XmlNodeKeys::StartSideAttribute).empty()) {
        model->SetStartSide(node.attribute(XmlNodeKeys::StartSideAttribute).as_string("B"));
        model->SetIsBtoT(std::string_view(node.attribute(XmlNodeKeys::StartSideAttribute).as_string("B")) == "B");
    } else {
        model->SetIsBtoT(true);
    }
    model->SetDirection(node.attribute(XmlNodeKeys::DirAttribute).as_string("L"));
    model->SetIsLtoR(std::string_view(node.attribute(XmlNodeKeys::DirAttribute).as_string("L")) != "R");
    // Note: parm1/2/3 are no longer read here.
    // Each model's specific deserializer reads its own named attributes with parm fallback.
    model->SetPixelStyle((Model::PIXEL_STYLE)(node.attribute(XmlNodeKeys::AntialiasAttribute).as_int((int)Model::PIXEL_STYLE::PIXEL_STYLE_SMOOTH)));
    model->SetPixelSize(node.attribute(XmlNodeKeys::PixelSizeAttribute).as_int(2));
    model->SetRGBWHandling(node.attribute(XmlNodeKeys::RGBWHandleAttribute).as_string());
    model->SetStringType(node.attribute(XmlNodeKeys::StringTypeAttribute).as_string("RGB Nodes"));
    model->SetLowDefFactor(node.attribute(XmlNodeKeys::LowDefinitionAttribute).as_int(100));
    model->SetShadowModelFor(node.attribute(XmlNodeKeys::ShadowModelAttribute).as_string(""));
    model->SetTransparency(node.attribute(XmlNodeKeys::TransparencyAttribute).as_int(0));
    model->SetBlackTransparency(node.attribute(XmlNodeKeys::BTransparencyAttribute).as_int(0));
    model->SetDescription(node.attribute(XmlNodeKeys::DescriptionAttribute).as_string());
    model->SetTagColourAsString(node.attribute(XmlNodeKeys::TagColourAttribute).as_string("#000000"));
    model->SetNodeNames(node.attribute(XmlNodeKeys::NodeNamesAttribute).as_string());
    model->SetStrandNames(node.attribute(XmlNodeKeys::StrandNamesAttribute).as_string());
    model->SetCustomColor(node.attribute(XmlNodeKeys::CustomColorAttribute).as_string("#000000"));
    model->SetModelChain(node.attribute(XmlNodeKeys::ModelChainAttribute).as_string(""));
    model->SetPixelSpacing(node.attribute(XmlNodeKeys::PixelSpacingAttribute).as_string(""));
    model->SetPixelCount(node.attribute(XmlNodeKeys::PixelCountAttribute).as_string(""));
    model->SetPixelType(node.attribute(XmlNodeKeys::PixelTypeAttribute).as_string(""));

    if (!importing) {
        model->SetControllerName(Trim(node.attribute(XmlNodeKeys::ControllerAttribute).as_string("")), true);
    }

    // Keep this after SetControllerName because it can clear out the start channel
    model->SetStartChannel(node.attribute(XmlNodeKeys::StartChannelAttribute).as_string("1"));

    // Individual Start Channels
    bool hasIndivChan = node.attribute(XmlNodeKeys::AdvancedAttribute).as_int(0) != 0;
    model->SetHasIndividualStartChannels(hasIndivChan);
    if (hasIndivChan) {
        // Read string count from new attribute names, falling back to parm1 for backward compat
        int num_strings = ReadAttrWithParmFallback(node, XmlNodeKeys::NumStringsAttribute, XmlNodeKeys::Parm1Attribute, "1");
        // Try model-specific attribute names if NumStrings wasn't found
        if (node.attribute(XmlNodeKeys::NumStringsAttribute).empty() && node.attribute(XmlNodeKeys::Parm1Attribute).empty()) {
            if (!node.attribute(XmlNodeKeys::NumArchesAttribute).empty())
                num_strings = node.attribute(XmlNodeKeys::NumArchesAttribute).as_int(1);
            else if (!node.attribute(XmlNodeKeys::NumCanesAttribute).empty())
                num_strings = node.attribute(XmlNodeKeys::NumCanesAttribute).as_int(1);
            else if (!node.attribute(XmlNodeKeys::NumChannelsAttribute).empty())
                num_strings = node.attribute(XmlNodeKeys::NumChannelsAttribute).as_int(1);
            else if (!node.attribute(XmlNodeKeys::DmxChannelCountAttribute).empty())
                num_strings = node.attribute(XmlNodeKeys::DmxChannelCountAttribute).as_int(1);
        }
        model->SetIndivStartChannelCount(num_strings);
        for (auto i = 0; i < num_strings;  i++) {
            model->SetIndividualStartChannel(i, node.attribute(model->StartChanAttrName(i)).as_string(""));
        }
    }

    if (!node.attribute(XmlNodeKeys::ModelBrightnessAttribute).empty() && model->GetDimmingCurve() == nullptr) {
        std::string mb = node.attribute(XmlNodeKeys::ModelBrightnessAttribute).as_string("0");
        if (mb.empty()) {
            mb = "0";
        }
        int b = std::strtol(mb.c_str(), nullptr, 10);
        if (b != 0) {
            std::map<std::string, std::map<std::string, std::string>> dimmingInfo;
            dimmingInfo["all"]["gamma"] = "1.0";
            dimmingInfo["all"]["brightness"] = mb;
            model->SetDimmingInfo(dimmingInfo);
        }
    }
}

void XmlDeserializingModelFactory::DeserializeCommonModelChildElements(Model* model, pugi::xml_node node, ModelManager& modelManager, bool importing) {
    bool importAliases = !importing;
    bool skipImportAliases = false;
    bool merge = false;
    bool showPopup = true;

    // Pre-pass: collect all modelGroup names so user can choose which to import
    std::set<std::string> selectedGroups;
    if (importing) {
        std::vector<std::string> allGroupNames;
        for (pugi::xml_node c = node.first_child(); c; c = c.next_sibling()) {
            if (std::string_view(c.name()) == "modelGroup") {
                std::string gname = c.attribute("name").as_string();
                if (!gname.empty()) {
                    allGroupNames.push_back(gname);
                }
            }
        }
        if (!allGroupNames.empty()) {
            UICallbacks* uiCb = modelManager.GetUICallbacks();
            if (uiCb) {
                std::vector<std::string> chosen = uiCb->ChooseFromList(
                    "Select Groups to Import (Cancel to import no groups)",
                    allGroupNames, allGroupNames);
                for (const auto& s : chosen) {
                    selectedGroups.insert(s);
                }
            } else {
                for (const auto& s : allGroupNames) {
                    selectedGroups.insert(s);
                }
            }
        }
    }

    pugi::xml_node f = node.first_child();
    while (f) {
        std::string_view fname = f.name();
        if ("faceInfo" == fname) {
            FaceStateData newFaceInfo(model->GetFaceInfo());
            XmlSerialize::DeserializeFaceInfo(f, newFaceInfo);
            model->SetFaceInfo(newFaceInfo);
            model->UpdateFaceInfoNodes();
        } else if ("stateInfo" == fname) {
            FaceStateData newStateInfo(model->GetStateInfo());
            XmlSerialize::DeserializeStateInfo(f, newStateInfo);
            model->SetStateInfo(newStateInfo);
            model->UpdateStateInfoNodes();
        } else if (XmlNodeKeys::DimmingCurveName == fname) {
            DeserializeDimmingCurve(model, f);
        } else if ("subModel" == fname) {
            DeserializeSubModel(model, f);
        } else if ("modelGroup" == fname && importing) {
            std::string gname = f.attribute("name").as_string();
            if (selectedGroups.count(gname)) {
                model->AddModelGroups(f, model->GetName(), merge, showPopup);
            }
        } else if (strcasecmp(f.name(), "shadowmodels") == 0 && importing) {
            model->ImportExtraModels(f, modelManager, "Unassigned");
        } else if (strcasecmp(f.name(), "associatedmodels") == 0 && importing) {
            model->ImportExtraModels(f, modelManager, model->GetLayoutGroup());
        } else if ("ControllerConnection" == fname) {
            if (!importing) {
                DeserializeControllerConnection(model, f);
            }
        } else if (fname == XmlNodeKeys::AliasesAttribute) {
            // can't be sure of the order of tags in xml and we don't want to ask twice, so setup breadcrumbs to ensure a single prompt
            if (importAliases == false) {
                if (skipImportAliases != true) {
                    UICallbacks* uiCb = modelManager.GetUICallbacks();
                    if (uiCb && uiCb->PromptYesNo("Should I import aliases from the base model?", "Import Aliases?")) {
                        importAliases = true;
                    } else {
                        skipImportAliases = true;
                    }
                }
            }
            if (importAliases == true) {
                DeserializeAliases(model, f);
            }
        }
        f = f.next_sibling();
    }
}
void XmlDeserializingModelFactory::DeserializeDimmingCurve(Model* model, pugi::xml_node node) {
    std::map<std::string, std::map<std::string, std::string>> dimmingInfo;
    pugi::xml_node child = node.first_child();
    while (child) {
        std::string key = child.name();
        if (!child.attribute("filename").empty()) {
            dimmingInfo[key]["filename"] = child.attribute("filename").as_string();
        } else {
            dimmingInfo[key]["brightness"] = child.attribute("brightness").as_string("100");
            dimmingInfo[key]["gamma"] = child.attribute("gamma").as_string("1.0");
        }
        child = child.next_sibling();
    }
    model->SetDimmingInfo(dimmingInfo);
}

void XmlDeserializingModelFactory::DeserializeSubModel(Model* model, pugi::xml_node node)
{
    // Use the shared parsing function from XmlSerializeFunctions
    auto smData = XmlSerialize::ParseSubModelNode(node);
    if (!smData) {
        return; // Invalid submodel node
    }

    // Create the SubModel object
    SubModel* sm = new SubModel(
        model,
        smData->name,
        smData->vertical,
        smData->isRanges,
        smData->bufferStyle
    );

    model->AddSubmodel(sm);

    // Populate the submodel based on type
    if (sm->IsRanges()) {
        if (sm->IsXYBufferStyle()) {
            // XY buffer style (Keep XY)
            for (const auto& line : smData->strands) {
                sm->AddRangeXY(line);
            }
            sm->CheckDuplicates();
            sm->CalcRangeXYBufferSize();
        } else {
            // Default and stacked buffer styles
            for (const auto& line : smData->strands) {
                sm->AddDefaultBuffer(line);
            }
            sm->CheckDuplicates();
        }
    } else {
        // Subbuffer type
        sm->AddSubbuffer(smData->subBuffer);
    }

    // Load aliases for this submodel (handles both "Aliases" and legacy "aliases")
    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        std::string_view childName = child.name();
        if (childName == XmlNodeKeys::AliasesAttribute || childName == "aliases") {
            DeserializeAliases(sm, child);
            break;
        }
    }
}

void XmlDeserializingModelFactory::DeserializeAliases(Model* model, pugi::xml_node node)
{
    std::list<std::string> aliases;

    pugi::xml_node f = node.first_child();
    while (f) {
        if (!f.attribute(XmlNodeKeys::NameAttribute).empty()) {
            aliases.push_back(f.attribute(XmlNodeKeys::NameAttribute).as_string());
        }
        f = f.next_sibling();
    }

    if (aliases.size() > 0) {
        model->SetAliases(aliases);
    }
}

void XmlDeserializingModelFactory::DeserializeSuperStrings(Model* model, pugi::xml_node node)
{
    bool found = true;
    int index = 0;
    while (found) {
        auto an = fmt::format("SuperStringColour{}", index);
        if (!node.attribute(an).empty()) {
            model->AddSuperStringColour(xlColor(std::string(node.attribute(an).as_string())));
        } else {
            found = false;
        }
        index++;
    }
}

Model* XmlDeserializingModelFactory::DeserializeArches(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    ArchesModel* model = new ArchesModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetNumArches(ReadAttrWithParmFallback(node, XmlNodeKeys::NumArchesAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    model->SetNodesPerArch(ReadAttrWithParmFallback(node, XmlNodeKeys::NodesPerArchAttribute, XmlNodeKeys::Parm2Attribute, "1"));
    model->SetLightsPerNode(ReadAttrWithParmFallback(node, XmlNodeKeys::LightsPerNodeAttribute, XmlNodeKeys::Parm3Attribute, "1"));
    DeserializeThreePointScreenLocationAttributes(model, node);
    model->SetZigZag(std::string_view(node.attribute(XmlNodeKeys::ZigZagAttribute).as_string()) == "true");
    if (!node.attribute(XmlNodeKeys::HollowAttribute).empty()) { model->SetHollow(node.attribute(XmlNodeKeys::HollowAttribute).as_int(0)); }
    if (!node.attribute(XmlNodeKeys::GapAttribute).empty()) { model->SetGap(node.attribute(XmlNodeKeys::GapAttribute).as_int(0)); }
    if (HasAttrOrLegacy(node, XmlNodeKeys::ArcAttribute, XmlNodeKeys::CArcAttribute)) {
        model->SetArc(std::strtol(GetAttrWithLegacyFallback(node, XmlNodeKeys::ArcAttribute, XmlNodeKeys::CArcAttribute).c_str(), nullptr, 10));
    }
    model->DeserializeLayerSizes(node.attribute(XmlNodeKeys::LayerSizesAttribute).as_string(), false);
    if (!node.attribute(XmlNodeKeys::ArchesSkewAttribute).empty()) {
        int angle = node.attribute(XmlNodeKeys::ArchesSkewAttribute).as_int(0);
        ThreePointScreenLocation& screenLoc = dynamic_cast<ThreePointScreenLocation&>(model->GetBaseObjectScreenLocation());
        screenLoc.SetAngle(angle);
    }
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeCandyCane(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    CandyCaneModel* model = new CandyCaneModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetNumCanes(ReadAttrWithParmFallback(node, XmlNodeKeys::NumCanesAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    model->SetNodesPerCane(ReadAttrWithParmFallback(node, XmlNodeKeys::NodesPerCaneAttribute, XmlNodeKeys::Parm2Attribute, "1"));
    model->SetLightsPerNode(ReadAttrWithParmFallback(node, XmlNodeKeys::LightsPerNodeAttribute, XmlNodeKeys::Parm3Attribute, "1"));
    DeserializeThreePointScreenLocationAttributes(model, node);
    model->SetReverse(std::string_view(node.attribute(XmlNodeKeys::CCReverseAttribute).as_string("false")) == "true");
    model->SetSticks(std::string_view(node.attribute(XmlNodeKeys::CCSticksAttribute).as_string("false")) == "true");
    model->SetAlternateNodes(std::string_view(node.attribute(XmlNodeKeys::AlternateNodesAttribute).as_string("false")) == "true");
    if (!node.attribute(XmlNodeKeys::CCSkewAttribute).empty()) {
        int angle = node.attribute(XmlNodeKeys::CCSkewAttribute).as_int(0);
        ThreePointScreenLocation& screenLoc = dynamic_cast<ThreePointScreenLocation&>(model->GetBaseObjectScreenLocation());
        screenLoc.SetAngle(angle);
    }
    model->SetCaneHeight(node.attribute(XmlNodeKeys::CCHeightAttribute).as_float(1.0f));
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeChannelBlock(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    ChannelBlockModel* model = new ChannelBlockModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetNumChannels(ReadAttrWithParmFallback(node, XmlNodeKeys::NumChannelsAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    DeserializeTwoPointScreenLocationAttributes(model, node);
    // Setup the model early to size the vector for number of colors
    model->Setup();
    for (auto i = 0; i < model->GetNumStrands();  i++) {
        std::string color = node.attribute(std::string(XmlNodeKeys::ChannelColorAttribute) + std::to_string(i+1)).as_string("white");
        model->SetChannelColor(i, color);
    }
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeCircle(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    CircleModel* model = new CircleModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetNumCircleStrings(ReadAttrWithParmFallback(node, XmlNodeKeys::NumStringsAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    model->SetCircleNodesPerString(ReadAttrWithParmFallback(node, XmlNodeKeys::NodesPerStringAttribute, XmlNodeKeys::Parm2Attribute, "1"));
    // convert old circle sizes to new Layer sizes setting - this also reverses the order
    std::string layer_sizes;
    std::string circleSizes = node.attribute(XmlNodeKeys::CircleSizesAttribute).as_string("");
    if (!circleSizes.empty()) {
        layer_sizes = ReverseCSV(circleSizes);
    } else {
        layer_sizes = node.attribute(XmlNodeKeys::LayerSizesAttribute).as_string("");
    }
    model->DeserializeLayerSizes(layer_sizes, false);
    if (node.attribute("StartSide").empty()) {
        model->SetIsBtoT(false);
    }
    model->SetCenterPercent(ReadAttrWithParmFallback(node, XmlNodeKeys::CenterPercentAttribute, XmlNodeKeys::Parm3Attribute, "0"));
    model->SetInsideOut(std::string_view(node.attribute(XmlNodeKeys::InsideOutAttribute).as_string("0")) == "1");
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeCube(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    CubeModel* model = new CubeModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetCubeWidth(ReadAttrWithParmFallback(node, XmlNodeKeys::CubeWidthAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    model->SetCubeHeight(ReadAttrWithParmFallback(node, XmlNodeKeys::CubeHeightAttribute, XmlNodeKeys::Parm2Attribute, "1"));
    model->SetCubeDepth(ReadAttrWithParmFallback(node, XmlNodeKeys::CubeDepthAttribute, XmlNodeKeys::Parm3Attribute, "1"));
    model->SetCubeStrings(node.attribute(XmlNodeKeys::CubeStringsAttribute).as_int(1));
    model->SetCubeStart(node.attribute(XmlNodeKeys::CubeStartAttribute).as_string(""));
    model->SetCubeStyle(node.attribute(XmlNodeKeys::StyleAttribute).as_string(""));
    model->SetStrandStyle(node.attribute(XmlNodeKeys::StrandPerLineAttribute).as_string(""));
    model->SetStrandPerLayer(std::string_view(node.attribute(XmlNodeKeys::StrandPerLayerAttribute).as_string("FALSE")) == "TRUE");
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeCustom(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    CustomModel* model = new CustomModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetCustomWidth(ReadAttrWithParmFallback(node, XmlNodeKeys::CustomWidthAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    model->SetCustomHeight(ReadAttrWithParmFallback(node, XmlNodeKeys::CustomHeightAttribute, XmlNodeKeys::Parm2Attribute, "1"));
    model->SetCustomDepth(node.attribute(XmlNodeKeys::CMDepthAttribute).as_int(1));
    int num_strings = node.attribute(XmlNodeKeys::CustomStringsAttribute).as_int(1);
    model->SetNumStrings(num_strings);
    model->SetCustomBackground(node.attribute(XmlNodeKeys::BkgImageAttribute).as_string(""));
    model->SetCustomLightness(node.attribute(XmlNodeKeys::BkgLightnessAttribute).as_int(0));
    model->SetCustomBkgScale(node.attribute(XmlNodeKeys::BkgScaleAttribute).as_int(100));
    model->SetCustomBkgBrightness(node.attribute(XmlNodeKeys::BkgBrightnessAttribute).as_int(20));
    std::vector<std::vector<std::vector<int>>>& locations = model->GetData();
    locations = XmlSerialize::ParseCustomModelDataFromXml(node);

    // Individual Start Nodes
    if (num_strings > 1) {
        model->SetHasIndivStartNodes(true);
        model->SetIndivStartNodesCount(num_strings);
        for (auto i = 0; i < num_strings;  i++) {
            // Try new attribute name first, fall back to old "String" format for backward compatibility
            std::string nodeVal = node.attribute(model->StartNodeAttrName(i)).as_string("");
            if (nodeVal.empty()) {
                // Old format used "String1", "String2", etc. for start nodes (only if not using indiv channels)
                if (!model->HasIndividualStartChannels()) {
                    nodeVal = node.attribute(Model::StartChanAttrName(i)).as_string("0");
                } else {
                    nodeVal = "0";
                }
            }
            model->SetNodeSize(i, std::strtol(nodeVal.c_str(), nullptr, 10));
        }
    }

    // Individual Start Channels - re-read with correct string count
    if (model->HasIndividualStartChannels()) {
        model->SetIndivStartChannelCount(num_strings);
        for (auto i = 0; i < num_strings; i++) {
            model->SetIndividualStartChannel(i, node.attribute(Model::StartChanAttrName(i)).as_string(""));
        }
    }

    model->Setup();

    // Any per-string start nodes that were not present in the XML remain 0.
    // Now that Setup() has built the node list, ComputeStringStartNode() returns
    // accurate values. Set them and re-run Setup() so stringStartChan is consistent.
    if (num_strings > 1) {
        bool anyUpdated = false;
        for (int i = 0; i < num_strings; i++) {
            if (model->GetIndivStartNode(i) == 0) {
                model->SetNodeSize(i, model->ComputeStringStartNode(i));
                anyUpdated = true;
            }
        }
        if (anyUpdated) {
            model->Setup();
        }
    }

    return model;
}

Model* XmlDeserializingModelFactory::DeserializeIcicles(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    IciclesModel* model = new IciclesModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetNumIcicleStrings(ReadAttrWithParmFallback(node, XmlNodeKeys::NumStringsAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    model->SetLightsPerString(ReadAttrWithParmFallback(node, XmlNodeKeys::NodesPerStringAttribute, XmlNodeKeys::Parm2Attribute, "1"));
    DeserializeThreePointScreenLocationAttributes(model, node);
    model->SetDropPattern(node.attribute(XmlNodeKeys::DropPatternAttribute).as_string("3,4,5,4"));
    model->SetAlternateNodes(std::string_view(node.attribute(XmlNodeKeys::AlternateNodesAttribute).as_string("false")) == "true");
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeImage(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    ImageModel* model = new ImageModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetImageFile(node.attribute(XmlNodeKeys::ImageAttribute).as_string(""));
    model->SetWhiteAsAlpha(std::string_view(node.attribute(XmlNodeKeys::WhiteAsAlphaAttribute).as_string("False")) == "True");
    model->SetOffBrightness(node.attribute(XmlNodeKeys::OffBrightnessAttribute).as_int(80));
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeLabel(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    LabelModel* model = new LabelModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetLabelText(node.attribute(XmlNodeKeys::LabelTextAttribute).as_string("Label"));
    model->SetLabelFontSize(node.attribute(XmlNodeKeys::LabelFontSizeAttribute).as_int(14));
    xlColor color;
    color.SetFromString(node.attribute(XmlNodeKeys::LabelTextColorAttribute).as_string("#FFFFFF"));
    model->SetLabelTextColor(color);
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeMatrix(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    MatrixModel* model = new MatrixModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetNumMatrixStrings(ReadAttrWithParmFallback(node, XmlNodeKeys::NumStringsAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    model->SetNodesPerString(ReadAttrWithParmFallback(node, XmlNodeKeys::NodesPerStringAttribute, XmlNodeKeys::Parm2Attribute, "1"));
    model->SetStrandsPerString(ReadAttrWithParmFallback(node, XmlNodeKeys::StrandsPerStringAttribute, XmlNodeKeys::Parm3Attribute, "1"));
    model->SetAlternateNodes(std::string_view(node.attribute(XmlNodeKeys::AlternateNodesAttribute).as_string("false")) == "true");
    model->SetNoZigZag(std::string_view(node.attribute(XmlNodeKeys::NoZigZagAttribute).as_string("false")) == "true");
    std::string type = node.attribute(XmlNodeKeys::DisplayAsAttribute).as_string("Matrix");
    if (type == "Vert Matrix") {
        model->SetVertical(true);
    } else if (std::string_view(node.attribute(XmlNodeKeys::VertMatrixAttribute).as_string("false")) == "true") {
        model->SetVertical(true);
    }
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeMultiPoint(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    MultiPointModel* model = new MultiPointModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    DeserializePolyPointScreenLocationAttributes(model, node);
    int num_strings = node.attribute(XmlNodeKeys::MultiStringsAttribute).as_int(1);
    model->SetNumStrings(num_strings);
    model->SetModelHeight(node.attribute(XmlNodeKeys::ModelHeightAttribute).as_float(1.0f));

    // Individual Start Channels
    if (model->HasIndividualStartChannels()) {
        model->SetIndivStartChannelCount(num_strings);
        for (int i = 0; i < num_strings; i++) {
            model->SetIndividualStartChannel(i, node.attribute(model->StartChanAttrName(i)).as_string(""));
        }
    }

    // Individual Start Nodes
    if (num_strings > 1 && !node.attribute(model->StartNodeAttrName(0)).empty()) {
        model->SetHasIndivStartNodes(true);
        model->SetIndivStartNodesCount(num_strings);
        for (auto i = 0; i < num_strings;  i++) {
            model->SetNodeSize(i, node.attribute(model->StartNodeAttrName(i)).as_int(0));
        }
    }

    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeSingleLine(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    SingleLineModel* model = new SingleLineModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetNumLines(ReadAttrWithParmFallback(node, XmlNodeKeys::NumStringsAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    model->SetNodesPerLine(ReadAttrWithParmFallback(node, XmlNodeKeys::NodesPerStringAttribute, XmlNodeKeys::Parm2Attribute, "50"));
    model->SetLightsPerNode(ReadAttrWithParmFallback(node, XmlNodeKeys::LightsPerNodeAttribute, XmlNodeKeys::Parm3Attribute, "1"));
    DeserializeTwoPointScreenLocationAttributes(model, node);
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializePolyLine(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    PolyLineModel* model = new PolyLineModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetLightsPerNode(ReadAttrWithParmFallback(node, XmlNodeKeys::LightsPerNodeAttribute, XmlNodeKeys::Parm3Attribute, "1"));
    model->SetTotalLightCount(ReadAttrWithParmFallback(node, XmlNodeKeys::NodesPerStringAttribute, XmlNodeKeys::Parm2Attribute, "0"));
    DeserializePolyPointScreenLocationAttributes(model, node);
    int num_strings = node.attribute(XmlNodeKeys::PolyStringsAttribute).as_int(1);
    model->SetNumStrings(num_strings);
    model->SetDropPattern(node.attribute(XmlNodeKeys::DropPatternAttribute).as_string("1"));
    model->SetAlternateNodes(std::string_view(node.attribute(XmlNodeKeys::AlternateNodesAttribute).as_string("false")) == "true");
    model->SetModelHeight(node.attribute(XmlNodeKeys::ModelHeightAttribute).as_float(1.0f));
    PolyPointScreenLocation& screenLoc = dynamic_cast<PolyPointScreenLocation&>(model->GetBaseObjectScreenLocation());

    // Re-read the individual start channels using num_strings count.
    if (model->HasIndividualStartChannels()) {
        model->SetIndivStartChannelCount(num_strings);
        for (int i = 0; i < num_strings; i++) {
            model->SetIndividualStartChannel(i, node.attribute(model->StartChanAttrName(i)).as_string(""));
        }
    }

    // Individual Start Nodes
    if (num_strings > 1 && !node.attribute(model->StartNodeAttrName(0)).empty()) {
        model->SetHasIndivStartNodes(true);
        model->SetIndivStartNodesCount(num_strings);
        for (auto i = 0; i < num_strings;  i++) {
            model->SetNodeSize(i, node.attribute(model->StartNodeAttrName(i)).as_int(0));
        }
    }

    // Individual Segments
    int num_segments = screenLoc.GetNumPoints() - 1;
    model->SetNumSegments(num_segments);
    if (!node.attribute(model->SegAttrName(0)).empty()) {  // old models didn't require segment attributes
        model->SetAutoDistribute(false);
        for (auto i = 0; i < num_segments;  i++) {
            model->SetSegmentSize(i, node.attribute(model->SegAttrName(i)).as_int(0));
        }
    }

    // Corner Settings
    for (int x = 0; x <= num_segments; x++) {
        std::string corner = node.attribute(model->CornerAttrName(x)).as_string("Neither");
        model->SetCornerString(x, corner);
        if( x == 0 ) {
            model->SetLeadOffset(x, corner == "Leading Segment" ? 1.0 : corner == "Trailing Segment" ? 0.0 : 0.5);
        } else if( x == num_segments ) {
            model->SetTrailOffset(x-1, corner == "Leading Segment" ? 0.0 : corner == "Trailing Segment" ? 1.0 : 0.5);
        } else {
            model->SetTrailOffset(x-1, corner == "Leading Segment" ? 0.0 : corner == "Trailing Segment" ? 1.0 : 0.5);
            model->SetLeadOffset(x, corner == "Leading Segment" ? 1.0 : corner == "Trailing Segment" ? 0.0 : 0.5);
        }
    }
    model->ClearPolyLineCreate();
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeSphere(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    SphereModel* model = new SphereModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetNumMatrixStrings(ReadAttrWithParmFallback(node, XmlNodeKeys::NumStringsAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    model->SetNodesPerString(ReadAttrWithParmFallback(node, XmlNodeKeys::NodesPerStringAttribute, XmlNodeKeys::Parm2Attribute, "1"));
    model->SetStrandsPerString(ReadAttrWithParmFallback(node, XmlNodeKeys::StrandsPerStringAttribute, XmlNodeKeys::Parm3Attribute, "1"));
    model->SetStartLatitude(node.attribute(XmlNodeKeys::StartLatAttribute).as_int(-86));
    model->SetEndLatitude(node.attribute(XmlNodeKeys::EndLatAttribute).as_int(86));
    model->SetDegrees(node.attribute(XmlNodeKeys::DegreesAttribute).as_int(360));
    model->SetAlternateNodes(std::string_view(node.attribute(XmlNodeKeys::AlternateNodesAttribute).as_string("false")) == "true");
    model->SetNoZigZag(std::string_view(node.attribute(XmlNodeKeys::NoZigZagAttribute).as_string("false")) == "true");
    std::string version = node.attribute(XmlNodeKeys::versionNumberAttribute).as_string();
    if (version.empty() || std::strtol(version.c_str(), nullptr, 10) < 8) {
        // Sphere scaling was fixed to be truly round. Previously:
        //   Hradius = BufferHt / (1.8 * 1.8 * 2)  (used for X and Z)
        //   Vradius = BufferHt / (1.8 * 2)         (used for Y)
        // Now all axes use: radius = max(BufferHt, BufferWi) / (1.8 * 2)
        //
        // To preserve visual size we need:
        //   scaleX,Z *= BufferHt / (1.8 * max(BufferHt, BufferWi))
        //   scaleY   *= BufferHt / max(BufferHt, BufferWi)
        int nodesPerString = model->GetNodesPerString();
        int strandsPerString = model->GetStrandsPerString();
        if (strandsPerString < 1) strandsPerString = 1;
        if (strandsPerString > nodesPerString) strandsPerString = nodesPerString;
        int pixelsPerStrand = (strandsPerString > 0) ? nodesPerString / strandsPerString : nodesPerString;
        int numStrands = model->GetNumStrings() * strandsPerString;
        int bufferHt = pixelsPerStrand;
        int bufferWi = numStrands;
        int mx = std::max(bufferHt, bufferWi);
        if (mx > 0) {
            float htOverMax = (float)bufferHt / (float)mx;
            auto mtrx = model->GetModelScreenLocation().GetScaleMatrix();
            mtrx.x *= htOverMax / 1.8f;
            mtrx.z *= htOverMax / 1.8f;
            mtrx.y *= htOverMax;
            model->GetModelScreenLocation().SetScaleMatrix(mtrx);
        }
    }

    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeSpinner(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    SpinnerModel* model = new SpinnerModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetNumSpinnerStrings(ReadAttrWithParmFallback(node, XmlNodeKeys::NumStringsAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    model->SetNodesPerArm(ReadAttrWithParmFallback(node, XmlNodeKeys::NodesPerArmAttribute, XmlNodeKeys::Parm2Attribute, "1"));
    model->SetArmsPerString(ReadAttrWithParmFallback(node, XmlNodeKeys::ArmsPerStringAttribute, XmlNodeKeys::Parm3Attribute, "1"));
    model->SetHollow(node.attribute(XmlNodeKeys::HollowAttribute).as_int(20));
    model->SetStartAngle(node.attribute(XmlNodeKeys::StartAngleAttribute).as_int(0));
    model->SetArc(node.attribute(XmlNodeKeys::ArcAttribute).as_int(360));
    model->SetZigZag(std::string_view(node.attribute(XmlNodeKeys::ZigZagAttribute).as_string("false")) == "true");
    model->SetAlternate(std::string_view(node.attribute(XmlNodeKeys::AlternateAttribute).as_string("false")) == "true");
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeStar(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    StarModel* model = new StarModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetNumStarStrings(ReadAttrWithParmFallback(node, XmlNodeKeys::NumStringsAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    model->SetStarNodesPerString(ReadAttrWithParmFallback(node, XmlNodeKeys::NodesPerStringAttribute, XmlNodeKeys::Parm2Attribute, "1"));
    model->SetStarPoints(ReadAttrWithParmFallback(node, XmlNodeKeys::StarPointsAttribute, XmlNodeKeys::Parm3Attribute, "5"));

    // convert old star sizes to new Layer sizes setting
    std::string layer_sizes;
    if (!node.attribute(XmlNodeKeys::StarSizesAttribute).empty()) {
        layer_sizes = node.attribute(XmlNodeKeys::StarSizesAttribute).as_string("");
    } else {
        layer_sizes = node.attribute(XmlNodeKeys::LayerSizesAttribute).as_string("");
    }
    model->DeserializeLayerSizes(layer_sizes, false);

    std::string starStartLocation = node.attribute(XmlNodeKeys::StarStartLocationAttribute).as_string("");
    if (starStartLocation.empty()) {
        starStartLocation = model->ConvertFromDirStartSide();
    }
    model->SetStarStartLocation(starStartLocation);

    model->SetStarRatio(node.attribute(XmlNodeKeys::StarRatioAttribute).as_float(2.618034f));
    model->SetInnerPercent(node.attribute(XmlNodeKeys::StarCenterPercentAttribute).as_int(-1));

    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeTree(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    TreeModel* model = new TreeModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetNumMatrixStrings(ReadAttrWithParmFallback(node, XmlNodeKeys::NumStringsAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    model->SetNodesPerString(ReadAttrWithParmFallback(node, XmlNodeKeys::NodesPerStringAttribute, XmlNodeKeys::Parm2Attribute, "1"));
    model->SetStrandsPerString(ReadAttrWithParmFallback(node, XmlNodeKeys::StrandsPerStringAttribute, XmlNodeKeys::Parm3Attribute, "1"));
    model->SetAlternateNodes(std::string_view(node.attribute(XmlNodeKeys::AlternateNodesAttribute).as_string("false")) == "true");
    model->SetNoZigZag(std::string_view(node.attribute(XmlNodeKeys::NoZigZagAttribute).as_string("false")) == "true");
    model->SetVertical(std::string_view(node.attribute(XmlNodeKeys::StrandDirAttribute).as_string("Vertical")) == "Vertical");
    model->SetFirstStrand(node.attribute(XmlNodeKeys::exportFirstStrandAttribute).as_int(0) - 1);
    model->SetTreeRotation(node.attribute(XmlNodeKeys::TreeRotationAttribute).as_float(3.0f));
    model->SetTreeSpiralRotations(node.attribute(XmlNodeKeys::TreeSpiralRotationsAttribute).as_float(0.0f));
    model->SetTreeBottomTopRatio(node.attribute(XmlNodeKeys::TreeBottomTopRatioAttribute).as_float(6.0f));
    model->SetPerspective(node.attribute(XmlNodeKeys::TreePerspectiveAttribute).as_float(0.2f));
    std::string type = node.attribute(XmlNodeKeys::DisplayAsAttribute).as_string("Tree");
    if (type != "Tree") {  // handle old DisplayAs format
        // Split on space to parse old format like "Tree Flat", "Tree Ribbon", "Tree 360"
        std::string_view sv(type);
        auto spacePos = sv.find(' ');
        if (spacePos != std::string_view::npos) {
            std::string_view token = sv.substr(spacePos + 1);
            if (token == "Flat") {
                model->SetTreeType(1);
                model->SetTreeDegrees(0);
            } else if (token == "Ribbon") {
                model->SetTreeType(2);
                model->SetTreeDegrees(-1);
            } else {
                long degrees = std::strtol(std::string(token).c_str(), nullptr, 10);
                model->SetTreeDegrees(degrees);
            }
        }
    } else {
        model->SetTreeType(node.attribute(XmlNodeKeys::TreeTypeAttribute).as_int(0));
        model->SetTreeDegrees(node.attribute(XmlNodeKeys::TreeDegreesAttribute).as_int(360));
    }
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeWindow(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    WindowFrameModel* model = new WindowFrameModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetTopNodes(ReadAttrWithParmFallback(node, XmlNodeKeys::TopNodesAttribute, XmlNodeKeys::Parm1Attribute, "0"));
    model->SetSideNodes(ReadAttrWithParmFallback(node, XmlNodeKeys::SideNodesAttribute, XmlNodeKeys::Parm2Attribute, "0"));
    model->SetBottomNodes(ReadAttrWithParmFallback(node, XmlNodeKeys::BottomNodesAttribute, XmlNodeKeys::Parm3Attribute, "0"));
    std::string rotation = node.attribute("Rotation").as_string("CW");
    model->SetRotation((rotation == "Clockwise" || rotation == "CW") ? 0 : 1);
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeWreath(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    WreathModel* model = new WreathModel(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    model->SetNumWreathStrings(ReadAttrWithParmFallback(node, XmlNodeKeys::NumStringsAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    model->SetWreathNodesPerString(ReadAttrWithParmFallback(node, XmlNodeKeys::NodesPerStringAttribute, XmlNodeKeys::Parm2Attribute, "50"));
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeModelGroup(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    // Phase 3: Use setters to configure ModelGroup instead of passing XML to constructor
    // This removes the XML dependency from the ModelGroup constructor

    // Create ModelGroup with basic parameters (no XML)
    ModelGroup* model = new ModelGroup(modelManager);

    // Deserialize base object attributes (name, layout group, active state)
    DeserializeBaseObjectAttributes(model, node, modelManager, importing);

    // Deserialize ModelGroup-specific properties using Phase 1 setters
    model->SetGridSize(node.attribute("GridSize").as_int(400));
    model->SetLayout(node.attribute("layout").as_string("minimalGrid"));
    model->SetDefaultCamera(node.attribute("DefaultCamera").as_string("2D"));

    // Centre offset attributes
    if (!node.attribute("XCentreOffset").empty()) {
        model->SetXCentreOffset(node.attribute("XCentreOffset").as_float(0));
    }
    if (!node.attribute("YCentreOffset").empty()) {
        model->SetYCentreOffset(node.attribute("YCentreOffset").as_float(0));
    }

    // Centre position attributes
    if (std::string_view(node.attribute("centreDefined").as_string("0")) != "0") {
        model->SetCentreDefined(true);
        model->SetCentreX(node.attribute("centrex").as_float(0));
        model->SetCentreY(node.attribute("centrey").as_float(0));
        if (!node.attribute("centreMinx").empty()) {
            model->SetCentreMinx(node.attribute("centreMinx").as_int(0));
        }
        if (!node.attribute("centreMiny").empty()) {
            model->SetCentreMiny(node.attribute("centreMiny").as_int(0));
        }
        if (!node.attribute("centreMaxx").empty()) {
            model->SetCentreMaxx(node.attribute("centreMaxx").as_int(0));
        }
        if (!node.attribute("centreMaxy").empty()) {
            model->SetCentreMaxy(node.attribute("centreMaxy").as_int(0));
        }
    }

    // Tag colour
    model->SetTagColourAsString(node.attribute(XmlNodeKeys::TagColourAttribute).as_string("#000000"));

    // Parse and add models to the group
    std::string modelsStr = node.attribute("models").as_string("");
    if (!modelsStr.empty()) {
        std::vector<std::string> modelNames;
        Split(modelsStr, ',', modelNames, true);
        for (const auto& modelName : modelNames) {
            if (!modelName.empty()) {
                model->AddModel(modelName);
            }
        }
    }

    // Deserialize child elements (Aliases)
    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        std::string_view childName = child.name();
        if (childName == XmlNodeKeys::AliasesAttribute || childName == "aliases") {
            DeserializeAliases(model, child);
        }
    }

    // Note: We call RebuildBuffers() to finalize the model group
    // This handles node initialization and buffer setup
    model->RebuildBuffers();

    return model;
}

// ************************************************************************************************************
// **********************                        DMX Section                           ************************
// ************************************************************************************************************
void XmlDeserializingModelFactory::DeserializeDmxModel(DmxModel* dmx_model, pugi::xml_node node) {
    dmx_model->SetDmxChannelCount(ReadAttrWithParmFallback(node, XmlNodeKeys::DmxChannelCountAttribute, XmlNodeKeys::Parm1Attribute, "1"));
    if (dmx_model->HasBeamAbility()) {
        DeserializeBeamAbility(dmx_model, node);
    }
    if (dmx_model->HasPresetAbility()) {
        DeserializePresetAbility(dmx_model, node);
    }
    if (dmx_model->HasColorAbility()) {
        DeserializeColorAbility(dmx_model, node);
    }
    if (dmx_model->HasShutterAbility()) {
        DeserializeShutterAbility(dmx_model, node);
    }
    if (dmx_model->HasDimmerAbility()) {
        DeserializeDimmerAbility(dmx_model, node);
    }
}

void XmlDeserializingModelFactory::DeserializeBeamAbility(DmxModel* model, pugi::xml_node node) {
    DmxBeamAbility* beam_ability = model->GetBeamAbility();
    beam_ability->SetBeamLength(node.attribute("DmxBeamLength").as_float(beam_ability->GetDefaultBeamLength()));
    beam_ability->SetBeamWidth(node.attribute("DmxBeamWidth").as_float(beam_ability->GetDefaultBeamWidth()));
    beam_ability->SetBeamOrient(node.attribute("DmxBeamOrient").as_int(0));
    beam_ability->SetBeamYOffset(node.attribute("DmxBeamYOffset").as_float(beam_ability->GetDefaultBeamYOffset()));
}

void XmlDeserializingModelFactory::DeserializePresetAbility(DmxModel* model, pugi::xml_node node) {
    DmxPresetAbility* preset_ability = model->GetPresetAbility();
    for (int i = 0; i < DmxPresetAbility::MAX_PRESETS; ++i) {
        auto dmxChanKey = fmt::format("DmxPresetChannel{}", i);
        auto dmxValueKey = fmt::format("DmxPresetValue{}", i);
        auto descKey = fmt::format("DmxPresetDesc{}", i);
        if (node.attribute(dmxChanKey).empty() || node.attribute(dmxValueKey).empty()) {
            break;
        }
        uint8_t dmxChan = node.attribute(dmxChanKey).as_int(1);
        uint8_t dmxVal = node.attribute(dmxValueKey).as_int(0);
        std::string dmxDesc = node.attribute(descKey).as_string();
        preset_ability->AddPreset(dmxChan, dmxVal, dmxDesc);
    }
}

void XmlDeserializingModelFactory::DeserializeDynamicColorAbility(DmxModel* model, pugi::xml_node node) {
    int color_type = node.attribute("DmxColorType").as_int(0);
    model->InitColorAbility(color_type);
}

void XmlDeserializingModelFactory::DeserializeColorAbility(DmxModel* model, pugi::xml_node node) {
    DmxColorAbility* color_ability = model->GetColorAbility();
    DmxColorAbility::DMX_COLOR_TYPE color_type = color_ability->GetColorType();
    if (color_type == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW) {
        DeserializeColorAbilityRGBAttributes(dynamic_cast<DmxColorAbilityRGB*>(color_ability), node);
    } else if (color_type == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_WHEEL) {
        DeserializeColorWheelAttributes(dynamic_cast<DmxColorAbilityWheel*>(color_ability), node);
    } else if (color_type == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_CMYW) {
        DeserializeColorAbilityCMYAttributes(dynamic_cast<DmxColorAbilityCMY*>(color_ability), node);
    }
}

void XmlDeserializingModelFactory::DeserializeColorAbilityRGBAttributes(DmxColorAbilityRGB* ability, pugi::xml_node node) {
    ability->SetRedChannel(node.attribute("DmxRedChannel").as_int(1));
    ability->SetGreenChannel(node.attribute("DmxGreenChannel").as_int(2));
    ability->SetBlueChannel(node.attribute("DmxBlueChannel").as_int(3));
    ability->SetWhiteChannel(node.attribute("DmxWhiteChannel").as_int(0));

    ability->SetRedBrightness(node.attribute("DmxRedBrightness").as_int(100));
    ability->SetGreenBrightness(node.attribute("DmxGreenBrightness").as_int(100));
    ability->SetBlueBrightness(node.attribute("DmxBlueBrightness").as_int(100));
    ability->SetWhiteBrightness(node.attribute("DmxWhiteBrightness").as_int(100));

    ability->SetRedGamma(node.attribute("DmxRedGamma").as_float(1.0f));
    ability->SetGreenGamma(node.attribute("DmxGreenGamma").as_float(1.0f));
    ability->SetBlueGamma(node.attribute("DmxBlueGamma").as_float(1.0f));
    ability->SetWhiteGamma(node.attribute("DmxWhiteGamma").as_float(1.0f));
 }

void XmlDeserializingModelFactory::DeserializeColorWheelAttributes(DmxColorAbilityWheel* ability, pugi::xml_node node) {
    ability->SetWheelChannel(node.attribute("DmxColorWheelChannel").as_int(0));
    ability->SetDimmerChannel(node.attribute("DmxDimmerChannel").as_int(0));
    ability->SetWheelDelay(node.attribute("DmxColorWheelDelay").as_int(0));
    ability->ClearColors();
    for (int i = 0; i< DmxColorAbilityWheel::MAX_COLORS; ++i) {
        auto dmxkey = fmt::format("DmxColorWheelDMX{}", i);
        auto colorkey = fmt::format("DmxColorWheelColor{}", i);
        if ( node.attribute(dmxkey).empty() || node.attribute(colorkey).empty() ) {
            break;
        }
        uint8_t dmxVal = node.attribute(dmxkey).as_int(1);
        std::string dmxcolor = node.attribute(colorkey).as_string();
        ability->AddColor(dmxcolor, dmxVal);
    }
}

void XmlDeserializingModelFactory::DeserializeColorAbilityCMYAttributes(DmxColorAbilityCMY* ability, pugi::xml_node node) {
    ability->SetCyanChannel(node.attribute("DmxCyanChannel").as_int(1));
    ability->SetMagentaChannel(node.attribute("DmxMagentaChannel").as_int(2));
    ability->SetYellowChannel(node.attribute("DmxYellowChannel").as_int(3));
    ability->SetWhiteChannel(node.attribute("DmxWhiteChannel").as_int(0));
}

void XmlDeserializingModelFactory::DeserializeShutterAbility(DmxModel* model, pugi::xml_node node) {
    DmxShutterAbility* shutter_ability = model->GetShutterAbility();
    shutter_ability->SetShutterChannel(node.attribute("DmxShutterChannel").as_int(0));
    shutter_ability->SetShutterThreshold(node.attribute("DmxShutterOpen").as_int(1));
    shutter_ability->SetShutterOnValue(node.attribute("DmxShutterOnValue").as_int(0));
}

void XmlDeserializingModelFactory::DeserializeDimmerAbility(DmxModel* model, pugi::xml_node node) {
    DmxDimmerAbility* dimmer_ability = model->GetDimmerAbility();
    dimmer_ability->SetDimmerChannel(node.attribute(XmlNodeKeys::MhDimmerChannelAttribute).as_int(0));
}

void XmlDeserializingModelFactory::DeserializeDmxImage(DmxImage* img, pugi::xml_node node) {
    img->SetImageFile(FileUtils::FixFile(std::string(""), std::string(node.attribute(XmlNodeKeys::ImageAttribute).as_string(""))));
    img->SetScaleX(node.attribute(XmlNodeKeys::ScaleXAttribute).as_float(1.0f));
    img->SetScaleY(node.attribute(XmlNodeKeys::ScaleYAttribute).as_float(1.0f));
    img->SetScaleZ(node.attribute(XmlNodeKeys::ScaleZAttribute).as_float(1.0f));
    img->SetRotateX(node.attribute(XmlNodeKeys::RotateXAttribute).as_float(0.0f));
    img->SetRotateY(node.attribute(XmlNodeKeys::RotateYAttribute).as_float(0.0f));
    img->SetRotateZ(node.attribute(XmlNodeKeys::RotateZAttribute).as_float(0.0f));
    img->SetOffsetX(node.attribute(XmlNodeKeys::OffsetXAttribute).as_float(0.0f));
    img->SetOffsetY(node.attribute(XmlNodeKeys::OffsetYAttribute).as_float(0.0f));
    img->SetOffsetZ(node.attribute(XmlNodeKeys::OffsetZAttribute).as_float(0.0f));
}

void XmlDeserializingModelFactory::DeserializeDmxMotor(DmxMotor* motor, pugi::xml_node node) {
    motor->SetChannelCoarse(node.attribute(XmlNodeKeys::ChannelCoarseAttribute).as_int(0));
    motor->SetChannelFine(node.attribute(XmlNodeKeys::ChannelFineAttribute).as_int(0));
    motor->SetMinLimit(node.attribute(XmlNodeKeys::MinLimitAttribute).as_int(-180));
    motor->SetMaxLimit(node.attribute(XmlNodeKeys::MaxLimitAttribute).as_int(180));
    motor->SetRangeOfMOtion(node.attribute(XmlNodeKeys::RangeOfMotionAttribute).as_float(180.0f));
    motor->SetOrientZero(node.attribute(XmlNodeKeys::OrientZeroAttribute).as_int(0));
    motor->SetOrientHome(node.attribute(XmlNodeKeys::OrientHomeAttribute).as_int(0));
    motor->SetSlewLimit(node.attribute(XmlNodeKeys::SlewLimitAttribute).as_float(0.0f));
    motor->SetReverse(std::string_view(node.attribute(XmlNodeKeys::ReverseAttribute).as_string("0")) == "1");
    motor->SetUpsideDown(std::string_view(node.attribute(XmlNodeKeys::UpsideDownAttribute).as_string("0")) == "1");
}

void XmlDeserializingModelFactory::DeserializeMesh(Mesh* mesh, pugi::xml_node node) {
    mesh->SetObjFile(FileUtils::FixFile(std::string(""), std::string(node.attribute(XmlNodeKeys::ObjFileAttribute).as_string(""))));
    mesh->SetRenderWidth(node.attribute(XmlNodeKeys::WidthAttribute).as_float(1.0f));
    mesh->SetRenderHeight(node.attribute(XmlNodeKeys::HeightAttribute).as_float(1.0f));
    mesh->SetRenderDepth(node.attribute(XmlNodeKeys::DepthAttribute).as_float(1.0f));
    mesh->SetMeshOnly(std::string_view(node.attribute(XmlNodeKeys::MeshOnlyAttribute).as_string("0")) == "1");
    mesh->SetBrightness(node.attribute(XmlNodeKeys::BrightnessAttribute).as_float(100.0f));
    mesh->SetScaleX(node.attribute(XmlNodeKeys::ScaleXAttribute).as_float(1.0f));
    mesh->SetScaleY(node.attribute(XmlNodeKeys::ScaleYAttribute).as_float(1.0f));
    mesh->SetScaleZ(node.attribute(XmlNodeKeys::ScaleZAttribute).as_float(1.0f));
    mesh->SetRotateX(node.attribute(XmlNodeKeys::RotateXAttribute).as_float(0.0f));
    mesh->SetRotateY(node.attribute(XmlNodeKeys::RotateYAttribute).as_float(0.0f));
    mesh->SetRotateZ(node.attribute(XmlNodeKeys::RotateZAttribute).as_float(0.0f));
    mesh->SetOffsetX(node.attribute(XmlNodeKeys::OffsetXAttribute).as_float(0.0f));
    mesh->SetOffsetY(node.attribute(XmlNodeKeys::OffsetYAttribute).as_float(0.0f));
    mesh->SetOffsetZ(node.attribute(XmlNodeKeys::OffsetZAttribute).as_float(0.0f));
}

void XmlDeserializingModelFactory::DeserializeServo(Servo* servo, pugi::xml_node node) {
    servo->SetChannel(node.attribute("Channel").as_int(0));
    servo->SetMinLimit(node.attribute("MinLimit").as_int(1));
    servo->SetMaxLimit(node.attribute("MaxLimit").as_int(65535));
    servo->SetRangeOfMotion(node.attribute("RangeOfMotion").as_float(180.0f));
    servo->SetScaledPivotOffsetX(node.attribute("PivotOffsetX").as_float(0));
    servo->SetScaledPivotOffsetY(node.attribute("PivotOffsetY").as_float(0));
    servo->SetScaledPivotOffsetZ(node.attribute("PivotOffsetZ").as_float(0));
    servo->SetStyle(node.attribute("ServoStyle").as_string("Translate X"));
    servo->SetControllerMin(node.attribute("ControllerMin").as_int(1000));
    servo->SetControllerMax(node.attribute("ControllerMax").as_int(2000));
    servo->SetControllerReverse(node.attribute("ControllerReverse").as_int(0) != 0);
    servo->SetControllerZero(node.attribute("ControllerZeroBehavior").as_string("Hold"));
    servo->SetControllerDataType(node.attribute("ControllerDataType").as_string("Scaled"));
}

void XmlDeserializingModelFactory::DeserializeDmxMovingHeadComm(DmxMovingHeadComm* model, pugi::xml_node node) {
    model->SetDmxFixture(node.attribute(XmlNodeKeys::DmxFixtureAttribute).as_string("MH1"));
    DeserializeDmxModel(model, node);
}

Model* XmlDeserializingModelFactory::DeserializeDmxFloodArea(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    DmxFloodArea* model = new DmxFloodArea(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    DeserializeDmxModel(model, node);
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeDmxFloodlight(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    DmxFloodlight* model = new DmxFloodlight(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    DeserializeDmxModel(model, node);
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeDmxGeneral(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    DmxGeneral* model = new DmxGeneral(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    DeserializeDmxModel(model, node);
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeDmxServo3d(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    DmxServo3d* model = new DmxServo3d(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    DeserializeDmxModel(model, node);
    model->SetNumServos(node.attribute("NumServos").as_int(1));
    model->SetNumStatic(node.attribute("NumStatic").as_int(1));
    model->SetNumMotion(node.attribute("NumMotion").as_int(1));
    model->SetIs16Bit(std::string_view(node.attribute("Bits16").as_string("1")) == "1");
    model->SetBrightness(node.attribute(XmlNodeKeys::BrightnessAttribute).as_int(100));

    pugi::xml_node n = node.first_child();
    while (n) {
        std::string name = n.name();
        std::string::size_type servo_idx = name.find("Servo");
        std::string::size_type static_idx = name.find("StaticMesh");
        std::string::size_type motion_idx = name.find("MotionMesh");

        if ("StaticMesh" == name) { // convert original name that had no number
            Mesh* msh = model->CreateStaticMesh("StaticMesh1", 0);
            DeserializeMesh(msh, n);
        } else if ("MotionMesh" == name) { // convert original name that had no number
            Mesh* msh = model->CreateMotionMesh("MotionMesh1", 0);
            DeserializeMesh(msh, n);
        } else if (static_idx != std::string::npos) {
            std::string num = name.substr(10, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < model->GetNumStatic()) {
                Mesh* msh = model->CreateStaticMesh(name, id);
                DeserializeMesh(msh, n);
            }
        } else if (motion_idx != std::string::npos) {
            std::string num = name.substr(10, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < model->GetNumMotion()) {
                Mesh* msh = model->CreateMotionMesh(name, id);
                DeserializeMesh(msh, n);
            }
        } else if (servo_idx != std::string::npos) {
            std::string num = name.substr(5, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < model->GetNumServos()) {
                Servo* sv = model->CreateServo(name, id);
                DeserializeServo(sv, n);
            }
        }
        n = n.next_sibling();
    }

    // get servo linkages
    for (int i = 0; i < model->GetNumServos(); ++i) {
        std::string num = std::to_string(i + 1);
        std::string this_link = "Servo" + num + "Linkage";
        std::string this_default = "Mesh " + num;
        std::string link = node.attribute(this_link).as_string(this_default.c_str());
        if (link.length() < 5) {
            link = "Mesh 1";
        }
        std::string num2 = link.substr(5, link.length());
        int link_id = atoi(num2.c_str());
        if (link_id < 1) {
            link_id = 1;
        }
        if (link_id != i + 1) {
            model->SetServoLink(i, link_id - 1);
        }
    }

    // get mesh linkages
    for (int i = 0; i < model->GetNumServos(); ++i) {
        std::string num = std::to_string(i + 1);
        std::string this_link = "Mesh" + num + "Linkage";
        std::string this_default = "Mesh " + num;
        std::string link = node.attribute(this_link).as_string(this_default.c_str());
        if (link.length() < 5) {
            link = "Mesh 1";
        }
        std::string num2 = link.substr(5, link.length());
        int link_id = atoi(num2.c_str());
        if (link_id < 1) {
            link_id = 1;
        }
        if (link_id != i + 1) {
            model->SetMeshLink(i, link_id - 1);
        }
    }

    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeDmxServo(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    DmxServo* model = new DmxServo(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    DeserializeDmxModel(model, node);
    model->SetNumServos(node.attribute("NumServos").as_int(1));
    model->SetIs16Bit(std::string_view(node.attribute("Bits16").as_string("1")) == "1");
    model->SetBrightness(node.attribute(XmlNodeKeys::BrightnessAttribute).as_int(100));
    model->SetTransparency(node.attribute(XmlNodeKeys::TransparencyAttribute).as_int(0));

    pugi::xml_node n = node.first_child();
    while (n) {
        std::string name = n.name();
        std::string::size_type servo_idx = name.find("Servo");
        std::string::size_type static_idx = name.find("StaticImage");
        std::string::size_type motion_idx = name.find("MotionImage");

        if ("StaticImage" == name) { // convert original name that had no number
            DmxImage* img = model->CreateStaticImage("StaticImage1", 0);
            DeserializeDmxImage(img, n);
        } else if ("MotionImage" == name) { // convert original name that had no number
            DmxImage* img = model->CreateStaticImage("MotionImage1", 0);
            DeserializeDmxImage(img, n);
        } else if (static_idx != std::string::npos) {
            std::string num = name.substr(11, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < model->GetNumServos()) {
                DmxImage* img = model->CreateStaticImage(name, id);
                DeserializeDmxImage(img, n);
            }
        } else if (motion_idx != std::string::npos) {
            std::string num = name.substr(11, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < model->GetNumServos()) {
                DmxImage* img = model->CreateMotionImage(name, id);
                DeserializeDmxImage(img, n);
            }
        } else if (servo_idx != std::string::npos) {
            std::string num = name.substr(5, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < model->GetNumServos()) {
                Servo* sv = model->CreateServo(name, id);
                DeserializeServo(sv, n);
            }
        }
        n = n.next_sibling();
    }

    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeDmxSkull(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    DmxSkull* model = new DmxSkull(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    DeserializeDmxModel(model, node);
    model->SetEyeBrightnessChannel(node.attribute("DmxEyeBrtChannel").as_int(15));
    model->SetJawOrient(node.attribute("DmxJawOrient").as_int(model->GetDefaultOrient(DmxSkull::SERVO_TYPE::JAW)));
    model->SetPanOrient(node.attribute("DmxPanOrient").as_int(model->GetDefaultOrient(DmxSkull::SERVO_TYPE::PAN)));
    model->SetTiltOrient(node.attribute("DmxTiltOrient").as_int(model->GetDefaultOrient(DmxSkull::SERVO_TYPE::TILT)));
    model->SetNodOrient(node.attribute("DmxNodOrient").as_int(model->GetDefaultOrient(DmxSkull::SERVO_TYPE::NOD)));
    model->SetEyeUDOrient(node.attribute("DmxEyeUDOrient").as_int(model->GetDefaultOrient(DmxSkull::SERVO_TYPE::EYE_UD)));
    model->SetEyeLROrient(node.attribute("DmxEyeLROrient").as_int(model->GetDefaultOrient(DmxSkull::SERVO_TYPE::EYE_LR)));
    model->SetHasJaw(std::string_view(node.attribute("HasJaw").as_string("1")) == "1");
    model->SetHasPan(std::string_view(node.attribute("HasPan").as_string("1")) == "1");
    model->SetHasTilt(std::string_view(node.attribute("HasTilt").as_string("1")) == "1");
    model->SetHasNod(std::string_view(node.attribute("HasNod").as_string("1")) == "1");
    model->SetHasEyeUD(std::string_view(node.attribute("HasEyeUD").as_string("1")) == "1");
    model->SetHasEyeLR(std::string_view(node.attribute("HasEyeLR").as_string("1")) == "1");
    model->SetHasColor(std::string_view(node.attribute("HasColor").as_string("1")) == "1");
    model->SetIs16Bit(std::string_view(node.attribute("Bits16").as_string("1")) == "1");
    model->SetMeshOnly(std::string_view(node.attribute("MeshOnly").as_string("0")) == "1");

    pugi::xml_node n = node.first_child();

    while (n) {
        std::string name = n.name();
        if ("HeadMesh" == name) {
            Mesh* mesh = model->CreateMesh("HeadMesh");
            if (mesh != nullptr) { DeserializeMesh(mesh, n); }
        } else if ("JawMesh" == name) {
            Mesh* mesh = model->CreateMesh("JawMesh");
            if (mesh != nullptr) { DeserializeMesh(mesh, n); }
        } else if ("EyeMeshL" == name) {
            Mesh* mesh = model->CreateMesh("EyeMeshL");
            if (mesh != nullptr) { DeserializeMesh(mesh, n); }
        } else if ("EyeMeshR" == name) {
            Mesh* mesh = model->CreateMesh("EyeMeshR");
            if (mesh != nullptr) { DeserializeMesh(mesh, n); }
        } else if ("JawServo" == name) {
            Servo* servo = model->CreateServo("JawServo");
            if (servo != nullptr) { DeserializeServo(servo, n); }
        } else if ("PanServo" == name) {
            Servo* servo = model->CreateServo("PanServo");
            if (servo != nullptr) { DeserializeServo(servo, n); }
        } else if ("TiltServo" == name) {
            Servo* servo = model->CreateServo("TiltServo");
            if (servo != nullptr) { DeserializeServo(servo, n); }
        } else if ("NodServo" == name) {
            Servo* servo = model->CreateServo("NodServo");
            if (servo != nullptr) { DeserializeServo(servo, n); }
        } else if ("EyeUpDownServo" == name) {
            Servo* servo = model->CreateServo("EyeUpDownServo");
            if (servo != nullptr) { DeserializeServo(servo, n); }
        } else if ("EyeLeftRightServo" == name) {
            Servo* servo = model->CreateServo("EyeLeftRightServo");
            if (servo != nullptr) { DeserializeServo(servo, n); }
        }
        n = n.next_sibling();
    }
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeDmxMovingHead(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    DmxMovingHead* model = new DmxMovingHead(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    DeserializeDynamicColorAbility(model, node);
    DeserializeDmxMovingHeadComm(model, node);
    model->SetHideBody(std::string_view(node.attribute("HideBody").as_string("False")) == "True");
    model->SetDmxStyle(node.attribute("DmxStyle").as_string("Moving Head Top"));

    pugi::xml_node n = node.first_child();
    while (n) {
        std::string name = n.name();
        if ("PanMotor" == name) {
            DmxMotor* motor = model->GetPanMotor();
            DeserializeDmxMotor(motor, n);
        } else if ("TiltMotor" == name) {
            DmxMotor* motor = model->GetTiltMotor();
            DeserializeDmxMotor(motor, n);
        }
        n = n.next_sibling();
    }
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeDmxMovingHeadAdv(pugi::xml_node node, ModelManager& modelManager, bool importing) {
    DmxMovingHeadAdv *model = new DmxMovingHeadAdv(modelManager);
    CommonDeserializeSteps(model, node, modelManager, importing);
    DeserializeDynamicColorAbility(model, node);
    DeserializeDmxMovingHeadComm(model, node);

    pugi::xml_node n = node.first_child();
    while (n) {
        std::string name = n.name();
        if ("PanMotor" == name) {
            DmxMotor* motor = model->GetPanMotor();
            DeserializeDmxMotor(motor, n);
        } else if ("TiltMotor" == name) {
            DmxMotor* motor = model->GetTiltMotor();
            DeserializeDmxMotor(motor, n);
        } else if ("BaseMesh" == name) {
            Mesh* msh = model->CreateBaseMesh(name);
            DeserializeMesh(msh, n);
        } else if ("YokeMesh" == name) {
            Mesh* msh = model->CreateYokeMesh(name);
            DeserializeMesh(msh, n);
        } else if ("HeadMesh" == name) {
            Mesh* msh = model->CreateHeadMesh(name);
            DeserializeMesh(msh, n);
        } else if ("PositionZone" == name) {
            PositionZone zone;
            zone.pan_min = n.attribute("PanMin").as_int(0);
            zone.pan_max = n.attribute("PanMax").as_int(255);
            zone.tilt_min = n.attribute("TiltMin").as_int(0);
            zone.tilt_max = n.attribute("TiltMax").as_int(255);
            zone.channel = std::max(1, n.attribute("Channel").as_int(1));
            zone.value = (uint8_t)n.attribute("Value").as_int(0);
            model->AddPositionZone(zone);
        }
        n = n.next_sibling();
    }
    model->Setup();
    return model;
}

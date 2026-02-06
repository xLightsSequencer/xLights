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
#include "XmlNodeKeys.h"
#include "XmlSerializeFunctions.h"
#include "../DimmingCurve.h"
#include "../models/ArchesModel.h"
#include "../models/CandyCaneModel.h"
#include "../models/ChannelBlockModel.h"
#include "../models/CircleModel.h"
#include "../models/CubeModel.h"
#include "../models/CustomModel.h"
#include "../models/IciclesModel.h"
#include "../models/ImageModel.h"
#include "../models/MatrixModel.h"
#include "../models/MultiPointModel.h"
#include "../models/PolyLineModel.h"
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
#include "../xLightsMain.h"

#include <wx/xml/xml.h>

using namespace XmlSerialize;

Model* XmlDeserializingModelFactory::Deserialize(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    auto type = node->GetAttribute(XmlNodeKeys::DisplayAsAttribute);

    std::string node_name = node->GetName();  // need this to support importing old models that did not have the DisplayAs attribute

    if (type == XmlNodeKeys::ArchesType || node_name == "archesmodel") {
        return DeserializeArches(node, xlights, importing);
    } else if (type == XmlNodeKeys::CandyCaneType) {
        return DeserializeCandyCane(node, xlights, importing);
    } else if (type == XmlNodeKeys::ChannelBlockType) {
        return DeserializeChannelBlock(node, xlights, importing);
    } else if (type == XmlNodeKeys::CircleType) {
        return DeserializeCircle(node, xlights, importing);
    } else if (type == XmlNodeKeys::CubeType) {
        return DeserializeCube(node, xlights, importing);
    } else if (type == XmlNodeKeys::CustomType || node_name == "custommodel") {
        return DeserializeCustom(node, xlights, importing);
    } else if (type == XmlNodeKeys::DmxMovingHeadType) {
        return DeserializeDmxMovingHead(node, xlights, importing);
    } else if (type == XmlNodeKeys::DmxMovingHeadAdvType) {
        return DeserializeDmxMovingHeadAdv(node, xlights, importing);
    } else if (type == XmlNodeKeys::DmxFloodAreaType) {
        return DeserializeDmxFloodArea(node, xlights, importing);
    } else if (type == XmlNodeKeys::DmxFloodlightType) {
        return DeserializeDmxFloodlight(node, xlights, importing);
    } else if (type == XmlNodeKeys::DmxGeneralType) {
        return DeserializeDmxGeneral(node, xlights, importing);
    } else if (type == XmlNodeKeys::DmxServoType) {
        return DeserializeDmxServo(node, xlights, importing);
    } else if (type == XmlNodeKeys::DmxServo3dType) {
        return DeserializeDmxServo3d(node, xlights, importing);
    } else if (type == XmlNodeKeys::DmxSkullType) {
        return DeserializeDmxSkull(node, xlights, importing);
    } else if (type == XmlNodeKeys::IciclesType) {
        return DeserializeIcicles(node, xlights, importing);
    } else if (type == XmlNodeKeys::ImageType) {
        return DeserializeImage(node, xlights, importing);
    } else if (type.Contains(XmlNodeKeys::MatrixType)) {
        return DeserializeMatrix(node, xlights, importing);
    } else if (type.Contains(XmlNodeKeys::MultiPointType)) {
        return DeserializeMultiPoint(node, xlights, importing);
    } else if (type == XmlNodeKeys::SingleLineType) {
        return DeserializeSingleLine(node, xlights, importing);
    } else if (type == XmlNodeKeys::PolyLineType) {
        return DeserializePolyLine(node, xlights, importing);
    } else if (type == XmlNodeKeys::SphereType) {
        return DeserializeSphere(node, xlights, importing);
    } else if (type == XmlNodeKeys::SpinnerType) {
        return DeserializeSpinner(node, xlights, importing);
    } else if (type == XmlNodeKeys::StarType) {
        return DeserializeStar(node, xlights, importing);
    } else if (type.Contains(XmlNodeKeys::TreeType)) {
        return DeserializeTree(node, xlights, importing);
    } else if (type == XmlNodeKeys::WindowType) {
        return DeserializeWindow(node, xlights, importing);
    } else if (type == XmlNodeKeys::WreathType) {
        return DeserializeWreath(node, xlights, importing);
    }
    return nullptr;
}

void XmlDeserializingModelFactory::CommonDeserializeSteps(Model* model, wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    DeserializeBaseObjectAttributes(model, node, xlights, importing);
    DeserializeCommonModelAttributes(model, node, importing);
    DeserializeModelScreenLocationAttributes(model, node, importing);
    DeserializeSuperStrings(model, node);
}

void XmlDeserializingModelFactory::DeserializeControllerConnection(Model* model, wxXmlNode* node) {
    for (wxXmlNode* p = node->GetChildren(); p != nullptr; p = p->GetNext()) {
        if (p->GetName() == "ControllerConnection") {
            auto& cc = model->GetCtrlConn();
            cc.SetProtocol(p->GetAttribute(XmlNodeKeys::ProtocolAttribute, xlEMPTY_STRING).ToStdString());
            cc.SetSerialProtocolSpeed(std::stoi(p->GetAttribute(XmlNodeKeys::ProtocolSpeedAttribute, std::to_string(CtrlDefs::DEFAULT_PROTOCOL_SPEED)).ToStdString()));
            cc.SetCtrlPort(std::stoi(p->GetAttribute(XmlNodeKeys::PortAttribute, std::to_string(CtrlDefs::DEFAULT_PORT)).ToStdString()));
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

void XmlDeserializingModelFactory::DeserializeBaseObjectAttributes(Model* model, wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    std::string name = node->GetAttribute("name").Trim(true).Trim(false).ToStdString();
    if (importing)
    {
        name = xlights->AllModels.GenerateModelName(name);
        model->SetLayoutGroup("Unassigned", true);
    } else {
        model->SetLayoutGroup(node->GetAttribute(XmlNodeKeys::LayoutGroupAttribute, "Unassigned").ToStdString(), true);
    }
    model->SetName(name);
    model->SetDisplayAs(node->GetAttribute(XmlNodeKeys::DisplayAsAttribute).ToStdString());
    model->SetActive(std::stoi(node->GetAttribute(XmlNodeKeys::ActiveAttribute, "1").ToStdString()));
    model->SetFromBase(std::stoi(node->GetAttribute(XmlNodeKeys::FromBaseAttribute, "0").ToStdString()));
}

void XmlDeserializingModelFactory::DeserializeCommonModelAttributes(Model* model, wxXmlNode* node, bool importing) {
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

    if (!importing) {
        model->SetControllerName(node->GetAttribute(XmlNodeKeys::ControllerAttribute, xlEMPTY_STRING).Trim(true).Trim(false).ToStdString(), true);
    }
    
    // Individual Start Channels
    bool hasIndivChan = std::stol(node->GetAttribute(XmlNodeKeys::AdvancedAttribute,"0").ToStdString());
    model->SetHasIndividualStartChannels(hasIndivChan);
    if (hasIndivChan) {
        int num_strings = model->GetParm1();
        model->SetIndivStartChannelCount(num_strings);
        for (auto i = 0; i < num_strings;  i++) {
            model->SetIndividualStartChannel(i, node->GetAttribute(model->StartChanAttrName(i), "").ToStdString());
        }
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
        std::string mb = node->GetAttribute(XmlNodeKeys::ModelBrightnessAttribute, "0").ToStdString();
        if (mb.empty()) {
            mb = "0";
        }
        int b = std::stoi(mb);
        if (b != 0) {
            model->modelDimmingCurve = DimmingCurve::createBrightnessGamma(b, 1.0);
        }
    }
}

void XmlDeserializingModelFactory::DeserializeSubModel(Model* model, wxXmlNode* node)
{
    const std::string name = node->GetAttribute(XmlNodeKeys::NameAttribute).Trim(true).Trim(false).ToStdString();
    const std::string layout = node->GetAttribute(XmlNodeKeys::LayoutAttribute, "vertical").ToStdString();
    const std::string type = node->GetAttribute(XmlNodeKeys::TypeAttribute, "ranges").ToStdString();
    const std::string bufferStyle = node->GetAttribute(XmlNodeKeys::BufferStyleAttribute, "Default").ToStdString();
    SubModel *sm = new SubModel(model, name, layout == "vertical", type == "ranges", bufferStyle);
    model->AddSubmodel(sm);

    if (sm->IsRanges()) {
        if (sm->IsXYBufferStyle()) {
            int line = 0;
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

void XmlDeserializingModelFactory::DeserializeAliases(Model* model, wxXmlNode* node)
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

void XmlDeserializingModelFactory::DeserializeSuperStrings(Model* model, wxXmlNode* node)
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

Model* XmlDeserializingModelFactory::DeserializeArches(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    ArchesModel* model = new ArchesModel(xlights->AllModels);
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
    model->DeserializeLayerSizes(node->GetAttribute(XmlNodeKeys::LayerSizesAttribute).ToStdString(), false);
    if (node->HasAttribute(XmlNodeKeys::ArchesSkewAttribute)) {
        int angle = std::stoi(node->GetAttribute(XmlNodeKeys::ArchesSkewAttribute, "0").ToStdString());
        ThreePointScreenLocation& screenLoc = dynamic_cast<ThreePointScreenLocation&>(model->GetBaseObjectScreenLocation());
        screenLoc.SetAngle(angle);
    }
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeCandyCane(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    CandyCaneModel* model = new CandyCaneModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    DeserializeThreePointScreenLocationAttributes(model, node);
    model->SetReverse(node->GetAttribute(XmlNodeKeys::CCReverseAttribute, "false") == "true");
    model->SetSticks(node->GetAttribute(XmlNodeKeys::CCSticksAttribute, "false") == "true");
    model->SetAlternateNodes(node->GetAttribute(XmlNodeKeys::AlternateNodesAttribute, "false") == "true");
    if (node->HasAttribute(XmlNodeKeys::CCSkewAttribute)) {
        int angle = wxAtoi(node->GetAttribute(XmlNodeKeys::CCSkewAttribute, "0"));
        ThreePointScreenLocation& screenLoc = dynamic_cast<ThreePointScreenLocation&>(model->GetBaseObjectScreenLocation());
        screenLoc.SetAngle(angle);
    }
    model->SetCaneHeight(std::stof(node->GetAttribute(XmlNodeKeys::CCHeightAttribute, "1.0").ToStdString()));
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeChannelBlock(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    ChannelBlockModel* model = new ChannelBlockModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    DeserializeTwoPointScreenLocationAttributes(model, node);
    // Setup the model early to size the vector for number of colors
    model->Setup();
    for (auto i = 0; i < model->GetNumStrands();  i++) {
        std::string color = node->GetAttribute(XmlNodeKeys::ChannelColorAttribute + std::to_string(i+1), "white");
        model->SetChannelColor(i, color);
    }
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeCircle(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    CircleModel* model = new CircleModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    // convert old circle sizes to new Layer sizes setting - this also reverses the order
    std::string layer_sizes = xlEMPTY_STRING;
    if (node->GetAttribute(XmlNodeKeys::CircleSizesAttribute, xlEMPTY_STRING) != xlEMPTY_STRING) {
        layer_sizes = ReverseCSV(node->GetAttribute(XmlNodeKeys::CircleSizesAttribute, xlEMPTY_STRING));
    } else {
        layer_sizes = node->GetAttribute(XmlNodeKeys::LayerSizesAttribute, xlEMPTY_STRING);
    }
    model->DeserializeLayerSizes(layer_sizes, false);
    if (!node->HasAttribute("StartSide")) {
        model->SetIsBtoT(false);
    }
    model->SetInsideOut(node->GetAttribute(XmlNodeKeys::InsideOutAttribute, "false") == "true");
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeCube(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    CubeModel* model = new CubeModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    model->SetCubeStrings(std::stoi(node->GetAttribute(XmlNodeKeys::CubeStringsAttribute, "1").ToStdString()));
    model->SetCubeStart(node->GetAttribute(XmlNodeKeys::CubeStartAttribute, xlEMPTY_STRING).ToStdString());
    model->SetCubeStyle(node->GetAttribute(XmlNodeKeys::StyleAttribute, xlEMPTY_STRING));
    model->SetStrandStyle(node->GetAttribute(XmlNodeKeys::StrandPerLineAttribute, xlEMPTY_STRING));
    model->SetStrandPerLayer(node->GetAttribute(XmlNodeKeys::StrandPerLayerAttribute, "FALSE") == "TRUE");
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeCustom(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    CustomModel* model = new CustomModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    model->SetCustomDepth(std::stol(node->GetAttribute(XmlNodeKeys::CMDepthAttribute, "1").ToStdString()));
    int num_strings = std::stoi(node->GetAttribute(XmlNodeKeys::CustomStringsAttribute, "1").ToStdString());
    model->SetNumStrings(num_strings);
    model->SetCustomBackground(node->GetAttribute(XmlNodeKeys::BkgImageAttribute, xlEMPTY_STRING));
    model->SetCustomLightness(std::stol(node->GetAttribute(XmlNodeKeys::BkgLightnessAttribute, "0").ToStdString()));
    std::vector<std::vector<std::vector<int>>>& locations = model->GetData();
    locations = XmlSerialize::ParseCustomModelDataFromXml(node);

    // Individual Start Nodes
    if (num_strings > 1) {
        model->SetHasIndivStartNodes(true);
        model->SetIndivStartNodesCount(num_strings);
        for (auto i = 0; i < num_strings;  i++) {
            model->SetNodeSize(i, std::stoi(node->GetAttribute(model->StartNodeAttrName(i), "0").ToStdString()));
        }
    }
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeIcicles(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    IciclesModel* model = new IciclesModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    DeserializeThreePointScreenLocationAttributes(model, node);
    model->SetDropPattern(node->GetAttribute(XmlNodeKeys::DropPatternAttribute, "3,4,5,4"));
    model->SetAlternateNodes(node->GetAttribute(XmlNodeKeys::AlternateNodesAttribute, "false") == "true");
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeImage(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    ImageModel* model = new ImageModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    model->SetImageFile(node->GetAttribute(XmlNodeKeys::ImageAttribute, xlEMPTY_STRING));
    model->SetWhiteAsAlpha(node->GetAttribute(XmlNodeKeys::WhiteAsAlphaAttribute, "False") == "True");
    model->SetOffBrightness(std::stoi(node->GetAttribute(XmlNodeKeys::OffBrightnessAttribute, "80").ToStdString()));
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeMatrix(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    MatrixModel* model = new MatrixModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    model->SetAlternateNodes(node->GetAttribute(XmlNodeKeys::AlternateNodesAttribute, "false") == "true");
    model->SetNoZigZag(node->GetAttribute(XmlNodeKeys::NoZigZagAttribute, "false") == "true");
    std::string type = node->GetAttribute(XmlNodeKeys::DisplayAsAttribute, "Matrix");
    if (type == "Vert Matrix") {
        model->SetVertical(true);
    } else if (node->GetAttribute(XmlNodeKeys::VertMatrixAttribute, "false") == "true") {
        model->SetVertical(true);
    }
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeMultiPoint(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    MultiPointModel* model = new MultiPointModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    DeserializePolyPointScreenLocationAttributes(model, node);
    model->SetNumStrings(std::stoi(node->GetAttribute(XmlNodeKeys::MultiStringsAttribute, "1").ToStdString()));
    model->SetModelHeight(std::stof(node->GetAttribute(XmlNodeKeys::ModelHeightAttribute, "1.0").ToStdString()));
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeSingleLine(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    SingleLineModel* model = new SingleLineModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    DeserializeTwoPointScreenLocationAttributes(model, node);
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializePolyLine(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    PolyLineModel* model = new PolyLineModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    DeserializePolyPointScreenLocationAttributes(model, node);
    int num_strings = std::stoi(node->GetAttribute(XmlNodeKeys::PolyStringsAttribute, "1").ToStdString());
    model->SetNumStrings(num_strings);
    model->SetDropPattern(node->GetAttribute(XmlNodeKeys::DropPatternAttribute, "1"));
    model->SetAlternateNodes(node->GetAttribute(XmlNodeKeys::AlternateNodesAttribute, "false") == "true");
    model->SetModelHeight(std::stof(node->GetAttribute(XmlNodeKeys::ModelHeightAttribute, "1.0").ToStdString()));
    PolyPointScreenLocation& screenLoc = dynamic_cast<PolyPointScreenLocation&>(model->GetBaseObjectScreenLocation());

    // Individual Start Nodes
    if (num_strings > 1) {
        model->SetHasIndivStartNodes(true);
        model->SetIndivStartNodesCount(num_strings);
        for (auto i = 0; i < num_strings;  i++) {
            model->SetNodeSize(i, std::stoi(node->GetAttribute(model->StartNodeAttrName(i), "0").ToStdString()));
        }
    }

    // Individual Segments
    int num_segments = screenLoc.GetNumPoints() - 1;
    model->SetNumSegments(num_segments);
    if (node->HasAttribute(model->SegAttrName(0))) {  // old models didn't require segment attributes
        model->SetAutoDistribute(false);
        for (auto i = 0; i < num_segments;  i++) {
            model->SetSegmentSize(i, std::stoi(node->GetAttribute(model->SegAttrName(i), "0").ToStdString()));
        }
    }

    // Corner Settings
    for (int x = 0; x <= num_segments; x++) {
        std::string corner = node->GetAttribute(model->CornerAttrName(x), "Neither");
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

Model* XmlDeserializingModelFactory::DeserializeSphere(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    SphereModel* model = new SphereModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    model->SetStartLatitude(std::stoi(node->GetAttribute(XmlNodeKeys::StartLatAttribute, "-86").ToStdString()));
    model->SetDegrees(std::stoi(node->GetAttribute(XmlNodeKeys::DegreesAttribute, "360").ToStdString()));
    model->SetAlternateNodes(node->GetAttribute(XmlNodeKeys::AlternateNodesAttribute, "false") == "true");
    model->SetNoZigZag(node->GetAttribute(XmlNodeKeys::NoZigZagAttribute, "false") == "true");
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeSpinner(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    SpinnerModel* model = new SpinnerModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    model->SetHollow(std::stoi(node->GetAttribute(XmlNodeKeys::HollowAttribute, "20").ToStdString()));
    model->SetStartAngle(std::stoi(node->GetAttribute(XmlNodeKeys::StartAngleAttribute, "0").ToStdString()));
    model->SetArc(std::stoi(node->GetAttribute(XmlNodeKeys::ArcAttribute, "360").ToStdString()));
    model->SetZigZag(node->GetAttribute(XmlNodeKeys::NoZigZagAttribute, "false") == "true");
    model->SetAlternate(node->GetAttribute(XmlNodeKeys::AlternateAttribute, "false") == "true");
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeStar(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    StarModel* model = new StarModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);

    // convert old star sizes to new Layer sizes setting
    std::string layer_sizes = xlEMPTY_STRING;
    if (node->GetAttribute("starSizes", "") != "") {
        layer_sizes = node->GetAttribute(XmlNodeKeys::StarSizesAttribute, "");
    } else {
        layer_sizes = node->GetAttribute(XmlNodeKeys::LayerSizesAttribute, "");
    }
    model->DeserializeLayerSizes(layer_sizes, false);

    auto starStartLocation = node->GetAttribute(XmlNodeKeys::StarStartLocationAttribute, "");
    if (starStartLocation == "") {
        starStartLocation = model->ConvertFromDirStartSide();
    }
    model->SetStarStartLocation(starStartLocation);

    model->SetStarRatio(wxAtof(node->GetAttribute(XmlNodeKeys::StarRatioAttribute, "2.618034") ));
    model->SetInnerPercent(wxAtoi(node->GetAttribute(XmlNodeKeys::StarCenterPercentAttribute, "-1")));

    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeTree(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    TreeModel* model = new TreeModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    model->SetAlternateNodes(node->GetAttribute(XmlNodeKeys::AlternateNodesAttribute, "false") == "true");
    model->SetNoZigZag(node->GetAttribute(XmlNodeKeys::NoZigZagAttribute, "false") == "true");
    model->SetVertical(node->GetAttribute(XmlNodeKeys::StrandDirAttribute, "Vertical") == "Vertical");
    model->SetFirstStrand(std::stoi(node->GetAttribute(XmlNodeKeys::exportFirstStrandAttribute, "0").ToStdString()) - 1);
    model->SetTreeRotation(std::stof(node->GetAttribute(XmlNodeKeys::TreeRotationAttribute, "3.0").ToStdString()));
    model->SetTreeSpiralRotations(std::stof(node->GetAttribute(XmlNodeKeys::TreeSpiralRotationsAttribute, "0.0").ToStdString()));
    model->SetTreeBottomTopRatio(std::stof(node->GetAttribute(XmlNodeKeys::TreeBottomTopRatioAttribute, "6.0").ToStdString()));
    model->SetPerspective(std::stof(node->GetAttribute(XmlNodeKeys::TreePerspectiveAttribute, "0.2").ToStdString()));
    std::string type = node->GetAttribute(XmlNodeKeys::DisplayAsAttribute, "Tree");
    if (type != "Tree") {  // handle old DiaplsyAs format
        wxStringTokenizer tkz(type, " ");
        wxString token = tkz.GetNextToken();
        token = tkz.GetNextToken();
        if (token == "Flat") {
            model->SetTreeType(1);
            model->SetTreeDegrees(0);
        } else if (token == "Ribbon") {
            model->SetTreeType(2);
            model->SetTreeDegrees(-1);
        } else {
            long degrees = 0;
            token.ToLong(&degrees);
            model->SetTreeDegrees(degrees);
        }
    } else {
        model->SetTreeType(std::stoi(node->GetAttribute(XmlNodeKeys::TreeTypeAttribute, "0").ToStdString()));
        model->SetTreeDegrees(std::stol(node->GetAttribute(XmlNodeKeys::TreeDegreesAttribute, "360").ToStdString()));
    }
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeWindow(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    WindowFrameModel* model = new WindowFrameModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    model->SetRotation(std::stof(node->GetAttribute(XmlNodeKeys::RotationAttribute, "3.0").ToStdString()));
    model->SetRotation((node->GetAttribute("Rotation", "CW") == "Clockwise" || node->GetAttribute("Rotation", "CW") == "CW") ? 0 : 1);
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeWreath(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    WreathModel* model = new WreathModel(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    model->Setup();
    return model;
}

// ************************************************************************************************************
// **********************                        DMX Section                           ************************
// ************************************************************************************************************
void XmlDeserializingModelFactory::DeserializeDmxModel(DmxModel* dmx_model, wxXmlNode* node) {
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

void XmlDeserializingModelFactory::DeserializeBeamAbility(DmxModel* model, wxXmlNode* node) {
    DmxBeamAbility* beam_ability = model->GetBeamAbility();
    beam_ability->SetBeamLength(std::stof(node->GetAttribute("DmxBeamLength", std::to_string(beam_ability->GetDefaultBeamLength())).ToStdString()));
    beam_ability->SetBeamWidth(std::stof(node->GetAttribute("DmxBeamWidth", std::to_string(beam_ability->GetDefaultBeamWidth())).ToStdString()));
    beam_ability->SetBeamOrient(std::stoi(node->GetAttribute("DmxBeamOrient", "0").ToStdString()));
    beam_ability->SetBeamYOffset(std::stof(node->GetAttribute("DmxBeamYOffset", std::to_string(beam_ability->GetDefaultBeamYOffset())).ToStdString()));
}

void XmlDeserializingModelFactory::DeserializePresetAbility(DmxModel* model, wxXmlNode* node) {
    DmxPresetAbility* preset_ability = model->GetPresetAbility();
    for (int i = 0; i < DmxPresetAbility::MAX_PRESETS; ++i) {
        auto dmxChanKey = wxString::Format("DmxPresetChannel%d", i);
        auto dmxValueKey = wxString::Format("DmxPresetValue%d", i);
        auto descKey = wxString::Format("DmxPresetDesc%d", i);
        if (!node->HasAttribute(dmxChanKey) || !node->HasAttribute(dmxValueKey)) {
            break;
        }
        uint8_t dmxChan = wxAtoi(node->GetAttribute(dmxChanKey, "1"));
        uint8_t dmxVal = wxAtoi(node->GetAttribute(dmxValueKey, "0"));
        std::string dmxDesc = node->GetAttribute(descKey);
        preset_ability->AddPreset(dmxChan, dmxVal, dmxDesc);
    }
}

void XmlDeserializingModelFactory::DeserializeDynamicColorAbility(DmxModel* model, wxXmlNode* node) {
    int color_type = std::stoi(node->GetAttribute("DmxColorType", "0").ToStdString());
    model->InitColorAbility(color_type);
}

void XmlDeserializingModelFactory::DeserializeColorAbility(DmxModel* model, wxXmlNode* node) {
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

void XmlDeserializingModelFactory::DeserializeColorAbilityRGBAttributes(DmxColorAbilityRGB* ability, wxXmlNode* node) {
    ability->SetRedChannel(std::stoi(node->GetAttribute("DmxRedChannel", "1").ToStdString()));
    ability->SetGreenChannel(std::stoi(node->GetAttribute("DmxGreenChannel", "2").ToStdString()));
    ability->SetBlueChannel(std::stoi(node->GetAttribute("DmxBlueChannel", "3").ToStdString()));
    ability->SetWhiteChannel(std::stoi(node->GetAttribute("DmxWhiteChannel", "0").ToStdString()));

    ability->SetRedBrightness(std::stoi(node->GetAttribute("DmxRedBrightness", "100").ToStdString()));
    ability->SetGreenBrightness(std::stoi(node->GetAttribute("DmxGreenBrightness", "100").ToStdString()));
    ability->SetBlueBrightness(std::stoi(node->GetAttribute("DmxBlueBrightness", "100").ToStdString()));
    ability->SetWhiteBrightness(std::stoi(node->GetAttribute("DmxWhiteBrightness", "100").ToStdString()));

    ability->SetRedGamma(std::stof(node->GetAttribute("DmxRedGamma", "1.0").ToStdString()));
    ability->SetGreenGamma(std::stof(node->GetAttribute("DmxGreenGamma", "1.0").ToStdString()));
    ability->SetBlueGamma(std::stof(node->GetAttribute("DmxBlueGamma", "1.0").ToStdString()));
    ability->SetWhiteGamma(std::stof(node->GetAttribute("DmxWhiteGamma", "1.0").ToStdString()));
 }

void XmlDeserializingModelFactory::DeserializeColorWheelAttributes(DmxColorAbilityWheel* ability, wxXmlNode* node) {
    ability->SetWheelChannel(std::stoi(node->GetAttribute("DmxColorWheelChannel", "0").ToStdString()));
    ability->SetDimmerChannel(std::stoi(node->GetAttribute("DmxDimmerChannel", "0").ToStdString()));
    ability->SetWheelDelay(std::stoi(node->GetAttribute("DmxColorWheelDelay", "0").ToStdString()));
    for (int i = 0; i< DmxColorAbilityWheel::MAX_COLORS; ++i) {
        auto dmxkey = wxString::Format("DmxColorWheelDMX%d", i);
        auto colorkey = wxString::Format("DmxColorWheelColor%d", i);
        if ( !node->HasAttribute(dmxkey) || !node->HasAttribute(colorkey) ) {
            break;
        }
        uint8_t dmxVal = wxAtoi(node->GetAttribute(dmxkey, "1"));
        wxString dmxcolor = node->GetAttribute(colorkey);
        ability->AddColor(dmxcolor, dmxVal);
    }
}

void XmlDeserializingModelFactory::DeserializeColorAbilityCMYAttributes(DmxColorAbilityCMY* ability, wxXmlNode* node) {
    ability->SetCyanChannel(std::stoi(node->GetAttribute("DmxCyanChannel", "1").ToStdString()));
    ability->SetMagentaChannel(std::stoi(node->GetAttribute("DmxMagentaChannel", "2").ToStdString()));
    ability->SetYellowChannel(std::stoi(node->GetAttribute("DmxYellowChannel", "3").ToStdString()));
    ability->SetWhiteChannel(std::stoi(node->GetAttribute("DmxWhiteChannel", "0").ToStdString()));
}

void XmlDeserializingModelFactory::DeserializeShutterAbility(DmxModel* model, wxXmlNode* node) {
    DmxShutterAbility* shutter_ability = model->GetShutterAbility();
    shutter_ability->SetShutterChannel(std::stoi(node->GetAttribute("DmxShutterChannel", "0").ToStdString()));
    shutter_ability->SetShutterThreshold(std::stoi(node->GetAttribute("DmxShutterOpen", "1").ToStdString()));
    shutter_ability->SetShutterOnValue(std::stoi(node->GetAttribute("DmxShutterOnValue", "0").ToStdString()));
}

void XmlDeserializingModelFactory::DeserializeDimmerAbility(DmxModel* model, wxXmlNode* node) {
    DmxDimmerAbility* dimmer_ability = model->GetDimmerAbility();
    dimmer_ability->SetDimmerChannel(std::stoi(node->GetAttribute("MHDimmerChannel", "0").ToStdString()));
}

void XmlDeserializingModelFactory::DeserializeDmxImage(DmxImage* img, wxXmlNode* node) {
    img->SetImageFile(FixFile("", node->GetAttribute(XmlNodeKeys::ImageAttribute, "")));
    img->SetScaleX(std::stof(node->GetAttribute(XmlNodeKeys::ScaleXAttribute, "1.0f").ToStdString()));
    img->SetScaleY(std::stof(node->GetAttribute(XmlNodeKeys::ScaleYAttribute, "1.0f").ToStdString()));
    img->SetScaleZ(std::stof(node->GetAttribute(XmlNodeKeys::ScaleZAttribute, "1.0f").ToStdString()));
    img->SetRotateX(std::stof(node->GetAttribute(XmlNodeKeys::RotateXAttribute, "0.0f").ToStdString()));
    img->SetRotateY(std::stof(node->GetAttribute(XmlNodeKeys::RotateYAttribute, "0.0f").ToStdString()));
    img->SetRotateZ(std::stof(node->GetAttribute(XmlNodeKeys::RotateZAttribute, "0.0f").ToStdString()));
    img->SetOffsetX(std::stof(node->GetAttribute(XmlNodeKeys::OffsetXAttribute, "0.0f").ToStdString()));
    img->SetOffsetY(std::stof(node->GetAttribute(XmlNodeKeys::OffsetYAttribute, "0.0f").ToStdString()));
    img->SetOffsetZ(std::stof(node->GetAttribute(XmlNodeKeys::OffsetZAttribute, "0.0f").ToStdString()));
}

void XmlDeserializingModelFactory::DeserializeDmxMotor(DmxMotor* motor, wxXmlNode* node) {
    motor->SetChannelCoarse(std::stoi(node->GetAttribute(XmlNodeKeys::ChannelCoarseAttribute, "0").ToStdString()));
    motor->SetChannelFine(std::stoi(node->GetAttribute(XmlNodeKeys::ChannelFineAttribute, "0").ToStdString()));
    motor->SetMinLimit(std::stoi(node->GetAttribute(XmlNodeKeys::MinLimitAttribute, "-180").ToStdString()));
    motor->SetMaxLimit(std::stoi(node->GetAttribute(XmlNodeKeys::MaxLimitAttribute, "180").ToStdString()));
    motor->SetRangeOfMOtion(std::stof(node->GetAttribute(XmlNodeKeys::RangeOfMotionAttribute, "180.0").ToStdString()));
    motor->SetOrientZero(std::stoi(node->GetAttribute(XmlNodeKeys::OrientZeroAttribute, "0").ToStdString()));
    motor->SetOrientHome(std::stoi(node->GetAttribute(XmlNodeKeys::OrientHomeAttribute, "0").ToStdString()));
    motor->SetSlewLimit(std::stof(node->GetAttribute(XmlNodeKeys::SlewLimitAttribute, "0.0").ToStdString()));
    motor->SetReverse(std::stoi(node->GetAttribute(XmlNodeKeys::OrientHomeAttribute, "0").ToStdString()));
    motor->SetReverse(node->GetAttribute(XmlNodeKeys::ReverseAttribute, "0") == "1");
    motor->SetUpsideDown(node->GetAttribute(XmlNodeKeys::UpsideDownAttribute, "0") == "1");
}

void XmlDeserializingModelFactory::DeserializeMesh(Mesh* mesh, wxXmlNode* node) {
    mesh->SetObjFile(FixFile("", node->GetAttribute(XmlNodeKeys::ObjFileAttribute, "")));
    mesh->SetRenderWidth(std::stof(node->GetAttribute(XmlNodeKeys::WidthAttribute, "1.0f").ToStdString()));
    mesh->SetRenderHeight(std::stof(node->GetAttribute(XmlNodeKeys::HeightAttribute, "1.0f").ToStdString()));
    mesh->SetRenderDepth(std::stof(node->GetAttribute(XmlNodeKeys::DepthAttribute, "1.0f").ToStdString()));
    mesh->SetMeshOnly(node->GetAttribute(XmlNodeKeys::MeshOnlyAttribute, "0") == "1");
    mesh->SetBrightness(std::stof(node->GetAttribute(XmlNodeKeys::BrightnessAttribute, "100.0f").ToStdString()));
    mesh->SetScaleX(std::stof(node->GetAttribute(XmlNodeKeys::ScaleXAttribute, "1.0f").ToStdString()));
    mesh->SetScaleY(std::stof(node->GetAttribute(XmlNodeKeys::ScaleYAttribute, "1.0f").ToStdString()));
    mesh->SetScaleZ(std::stof(node->GetAttribute(XmlNodeKeys::ScaleZAttribute, "1.0f").ToStdString()));
    mesh->SetRotateX(std::stof(node->GetAttribute(XmlNodeKeys::RotateXAttribute, "0.0f").ToStdString()));
    mesh->SetRotateY(std::stof(node->GetAttribute(XmlNodeKeys::RotateYAttribute, "0.0f").ToStdString()));
    mesh->SetRotateZ(std::stof(node->GetAttribute(XmlNodeKeys::RotateZAttribute, "0.0f").ToStdString()));
    mesh->SetOffsetX(std::stof(node->GetAttribute(XmlNodeKeys::OffsetXAttribute, "0.0f").ToStdString()));
    mesh->SetOffsetY(std::stof(node->GetAttribute(XmlNodeKeys::OffsetYAttribute, "0.0f").ToStdString()));
    mesh->SetOffsetZ(std::stof(node->GetAttribute(XmlNodeKeys::OffsetZAttribute, "0.0f").ToStdString()));
}

void XmlDeserializingModelFactory::DeserializeServo(Servo* servo, wxXmlNode* node) {
    servo->SetChannel(std::stoi(node->GetAttribute("Channel", "0").ToStdString()));
    servo->SetMinLimit(std::stoi(node->GetAttribute("MinLimit", "1").ToStdString()));
    servo->SetMaxLimit(std::stoi(node->GetAttribute("MaxLimit", "65535").ToStdString()));
    servo->SetRangeOfMotion(std::stof(node->GetAttribute("RangeOfMotion", "180.0f").ToStdString()));
    servo->SetScaledPivotOffsetX(std::stof(node->GetAttribute("PivotOffsetX", "0").ToStdString()));
    servo->SetScaledPivotOffsetY(std::stof(node->GetAttribute("PivotOffsetY", "0").ToStdString()));
    servo->SetScaledPivotOffsetZ(std::stof(node->GetAttribute("PivotOffsetZ", "0").ToStdString()));
    servo->SetStyle(node->GetAttribute("ServoStyle", "Translate X"));
    servo->SetControllerMin(std::stoi(node->GetAttribute("ControllerMin", "1000").ToStdString()));
    servo->SetControllerMax(std::stoi(node->GetAttribute("ControllerMax", "2000").ToStdString()));
    servo->SetControllerReverse(std::stoi(node->GetAttribute("ControllerReverse", "0").ToStdString()) != 0);
    servo->SetControllerZero(node->GetAttribute("ControllerZeroBehavior", "Hold"));
    servo->SetControllerDataType(node->GetAttribute("ControllerDataType", "Scaled"));
}

void XmlDeserializingModelFactory::DeserializeDmxMovingHeadComm(DmxMovingHeadComm* model, wxXmlNode* node) {
    model->SetDmxFixture(node->GetAttribute(XmlNodeKeys::DmxFixtureAttribute, "MH1"));
    DeserializeDmxModel(model, node);
}

Model* XmlDeserializingModelFactory::DeserializeDmxFloodArea(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    DmxFloodArea* model = new DmxFloodArea(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    DeserializeDmxModel(model, node);
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeDmxFloodlight(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    DmxFloodlight* model = new DmxFloodlight(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    DeserializeDmxModel(model, node);
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeDmxGeneral(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    DmxGeneral* model = new DmxGeneral(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    DeserializeDmxModel(model, node);
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeDmxServo3d(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    DmxServo3d* model = new DmxServo3d(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    DeserializeDmxModel(model, node);
    model->SetNumServos(std::stoi(node->GetAttribute("NumServos", "1").ToStdString()));
    model->SetNumStatic(std::stoi(node->GetAttribute("NumStatic", "1").ToStdString()));
    model->SetNumMotion(std::stoi(node->GetAttribute("NumMotion", "1").ToStdString()));
    model->SetIs16Bit(node->GetAttribute("Bits16", "0") == "1");
    model->SetBrightness(std::stoi(node->GetAttribute(XmlNodeKeys::BrightnessAttribute, "100").ToStdString()));

    wxXmlNode* n = node->GetChildren();
    while (n != nullptr) {
        std::string name = n->GetName();
        int servo_idx = name.find("Servo");
        int static_idx = name.find("StaticMesh");
        int motion_idx = name.find("MotionMesh");

        if ("StaticMesh" == name) { // convert original name that had no number
            Mesh* msh = model->CreateStaticMesh("StaticMesh1", 0);
            DeserializeMesh(msh, n);
        } else if ("MotionMesh" == name) { // convert original name that had no number
            Mesh* msh = model->CreateStaticMesh("MotionMesh1", 0);
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
        n = n->GetNext();
    }

    // get servo linkages
    for (int i = 0; i < model->GetNumServos(); ++i) {
        std::string num = std::to_string(i + 1);
        std::string this_link = "Servo" + num + "Linkage";
        std::string this_default = "Mesh " + num;
        std::string link = node->GetAttribute(this_link, this_default);
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
        std::string link = node->GetAttribute(this_link, this_default);
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

Model* XmlDeserializingModelFactory::DeserializeDmxServo(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    DmxServo* model = new DmxServo(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    DeserializeDmxModel(model, node);
    model->SetNumServos(std::stoi(node->GetAttribute("NumServos", "1").ToStdString()));
    model->SetIs16Bit(node->GetAttribute("Bits16", "0") == "1");
    model->SetBrightness(std::stoi(node->GetAttribute(XmlNodeKeys::BrightnessAttribute, "100").ToStdString()));
    model->SetTransparency(std::stoi(node->GetAttribute(XmlNodeKeys::TransparencyAttribute, "0").ToStdString()));

    wxXmlNode* n = node->GetChildren();
    while (n != nullptr) {
        std::string name = n->GetName();
        int servo_idx = name.find("Servo");
        int static_idx = name.find("StaticImage");
        int motion_idx = name.find("MotionImage");

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
        n = n->GetNext();
    }

    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeDmxSkull(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    DmxSkull* model = new DmxSkull(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    DeserializeDmxModel(model, node);
    model->SetEyeBrightnessChannel(std::stoi(node->GetAttribute("DmxEyeBrtChannel", "15").ToStdString()));
    model->SetJawOrient(std::stoi(node->GetAttribute("DmxJawOrient", std::to_string(model->GetDefaultOrient(DmxSkull::SERVO_TYPE::JAW))).ToStdString()));
    model->SetPanOrient(std::stoi(node->GetAttribute("DmxPanOrient", std::to_string(model->GetDefaultOrient(DmxSkull::SERVO_TYPE::PAN))).ToStdString()));
    model->SetTiltOrient(std::stoi(node->GetAttribute("DmxTiltOrient", std::to_string(model->GetDefaultOrient(DmxSkull::SERVO_TYPE::TILT))).ToStdString()));
    model->SetNodOrient(std::stoi(node->GetAttribute("DmxNodOrient", std::to_string(model->GetDefaultOrient(DmxSkull::SERVO_TYPE::NOD))).ToStdString()));
    model->SetEyeUDOrient(std::stoi(node->GetAttribute("DmxEyeUDOrient", std::to_string(model->GetDefaultOrient(DmxSkull::SERVO_TYPE::EYE_UD))).ToStdString()));
    model->SetEyeLROrient(std::stoi(node->GetAttribute("DmxEyeLROrient", std::to_string(model->GetDefaultOrient(DmxSkull::SERVO_TYPE::EYE_LR))).ToStdString()));
    model->SetHasJaw(node->GetAttribute("HasJaw", "1") == "1");
    model->SetHasPan(node->GetAttribute("HasPan", "1") == "1");
    model->SetHasTilt(node->GetAttribute("HasTilt", "1") == "1");
    model->SetHasNod(node->GetAttribute("HasNod", "1") == "1");
    model->SetHasEyeUD(node->GetAttribute("HasEyeUD", "1") == "1");
    model->SetHasEyeLR(node->GetAttribute("HasEyeLR", "1") == "1");
    model->SetHasColor(node->GetAttribute("HasColor", "1") == "1");
    model->SetIs16Bit(node->GetAttribute("Bits16", "1") == "1");
    model->SetMeshOnly(node->GetAttribute("MeshOnly", "0") == "1");

    wxXmlNode* n = node->GetChildren();

    while (n != nullptr) {
        std::string name = n->GetName();
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
        n = n->GetNext();
    }
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeDmxMovingHead(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    DmxMovingHead* model = new DmxMovingHead(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    DeserializeDynamicColorAbility(model, node);
    DeserializeDmxMovingHeadComm(model, node);
    model->SetHideBody(node->GetAttribute("HideBody", "False").ToStdString() == "True");
    model->SetDmxStyle(node->GetAttribute("DmxStyle", "Moving Head Top"));

    wxXmlNode* n = node->GetChildren();
    while (n != nullptr) {
        std::string name = n->GetName();
        if ("PanMotor" == name) {
            DmxMotor* motor = model->CreatePanMotor(name);
            DeserializeDmxMotor(motor, n);
        } else if ("TiltMotor" == name) {
            DmxMotor* motor = model->CreateTiltMotor(name);
            DeserializeDmxMotor(motor, n);
        }
        n = n->GetNext();
    }
    model->Setup();
    return model;
}

Model* XmlDeserializingModelFactory::DeserializeDmxMovingHeadAdv(wxXmlNode *node, xLightsFrame* xlights, bool importing) {
    DmxMovingHeadAdv *model = new DmxMovingHeadAdv(xlights->AllModels);
    CommonDeserializeSteps(model, node, xlights, importing);
    DeserializeDynamicColorAbility(model, node);
    DeserializeDmxMovingHeadComm(model, node);

    wxXmlNode* n = node->GetChildren();
    while (n != nullptr) {
        std::string name = n->GetName();
        if ("PanMotor" == name) {
            DmxMotor* motor = model->CreatePanMotor(name);
            DeserializeDmxMotor(motor, n);
        } else if ("TiltMotor" == name) {
            DmxMotor* motor = model->CreateTiltMotor(name);
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
        } else if ("Features" == name) {
            // process features
          /*  if( features_xml_node == nullptr ) {
                features_xml_node = new wxXmlNode(wxXML_ELEMENT_NODE, "Features");
                node->AddChild(features_xml_node);
            } else {
                n = features_xml_node->GetChildren();
                while (n != nullptr) {
                    std::string node_name = n->GetName();
                    std::string feature_name = n->GetAttribute("Name", node_name);
                    bool feature_found {false};
                    for (auto it = features.begin(); it != features.end(); ++it) {
                        if( (*it)->GetName() == feature_name ) {
                            feature_found = true;
                            (*it)->Init();
                            break;
                        }
                    }
                    if( !feature_found ) {
                        std::unique_ptr<MhFeature> newFeature(new MhFeature(n, node_name, feature_name));
                        newFeature->Init();
                        features.push_back(std::move(newFeature));
                    }
                    n = n->GetNext();
                }
            }*/
        }
        n = n->GetNext();
    }
    model->Setup();
    return model;
}

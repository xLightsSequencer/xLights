/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "GdtfParser.h"
#include "../models/Model.h"
#include "../models/BoxedScreenLocation.h"
#include "../models/DMX/DmxMovingHead.h"
#include "../models/DMX/DmxColorAbility.h"
#include "../models/DMX/DmxColorAbilityCMY.h"
#include "../models/DMX/DmxColorAbilityRGB.h"
#include "../models/DMX/DmxColorAbilityWheel.h"
#include "../models/DMX/DmxShutterAbility.h"
#include "../models/ModelManager.h"
#include "../render/UICallbacks.h"
#include "../utils/TraceLog.h"
#include "UtilFunctions.h"
#include <algorithm>
#include <string_view>
#include <format>

namespace XmlSerialize {
    // Helper function to parse value from GDTF format
    static int ParseDmxValue(const std::string& s, int channels) {
        auto slashPos = s.find('/');
        if (slashPos == std::string::npos) {
            return 0;
        }
        std::string first = s.substr(0, slashPos);
        std::string second = s.substr(slashPos + 1);
        int firstVal = (int)std::strtol(first.c_str(), nullptr, 10);
        if (second == "1") {
            if (channels == 2) {
                return firstVal << 8;
            }
            return firstVal;
        } else if (second == "2") {
            return firstVal;
        }
        return 0;
    }
    // Helper function to parse a DMX channel from XML
    static DMXChannelInfo ParseDmxChannel(pugi::xml_node channelNode) {
        DMXChannelInfo info;
        std::string offset = channelNode.attribute("Offset").as_string();
        std::vector<std::string> os;
        Split(offset, ',', os);
        if (!os.empty()) {
            info.channelStart = (int)std::strtol(os[0].c_str(), nullptr, 10);
            info.channels = os.size();
        }
        for (pugi::xml_node nn = channelNode.first_child(); nn; nn = nn.next_sibling()) {
            if (std::string_view(nn.name()) == "LogicalChannel") {
                info.attribute = nn.attribute("Attribute").as_string();
                for (pugi::xml_node nnn = nn.first_child(); nnn; nnn = nnn.next_sibling()) {
                    if (std::string_view(nnn.name()) == "ChannelFunction") {
                        for (pugi::xml_node nnnn = nnn.first_child(); nnnn; nnnn = nnnn.next_sibling()) {
                            std::string_view nName = nnnn.attribute("Name").as_string();
                            if (std::string_view(nnnn.name()) == "ChannelSet" && !nName.empty()) {
                                DMXChannelValue value;
                                value.description = std::string(nName);
                                value.low = ParseDmxValue(nnnn.attribute("DMXFrom").as_string(), info.channels);
                                pugi::xml_node nextNode = nnnn.next_sibling();
                                if (!nextNode) {
                                    value.high = value.low;
                                } else {
                                    value.high = ParseDmxValue(nextNode.attribute("DMXFrom").as_string(), info.channels) - 1;
                                }
                                info.values.push_back(value);
                            }
                        }
                    }
                }
            }
        }
        return info;
    }
    bool ParseGdtfDescriptionXml(
        pugi::xml_document& gdtf_doc,
        ModelManager& modelManager,
        UICallbacks* uiCallbacks,
        bool& cancelled,
        GdtfModelData& outData
    ) {
        pugi::xml_node root = gdtf_doc.document_element();
        if (!root) {
            return false;
        }
        // Parse all available DMX modes
        std::map<std::string, pugi::xml_node> modes;
        for (pugi::xml_node n = root.first_child(); n; n = n.next_sibling()) {
            if (std::string_view(n.name()) == "FixtureType") {
                for (pugi::xml_node nn = n.first_child(); nn; nn = nn.next_sibling()) {
                    if (std::string_view(nn.name()) == "DMXModes") {
                        for (pugi::xml_node nnn = nn.first_child(); nnn; nnn = nnn.next_sibling()) {
                            if (std::string_view(nnn.name()) == "DMXMode") {
                                modes[nnn.attribute("Name").as_string()] = nnn;
                            }
                        }
                    }
                }
            }
        }
        if (modes.empty()) {
            return false;
        }
        // Select mode (user choice if multiple modes)
        std::string selectedMode = modes.begin()->first;
        if (modes.size() > 1 && uiCallbacks) {
            std::vector<std::string> choices;
            for (const auto& it : modes) {
                choices.push_back(it.first);
            }
            auto selected = uiCallbacks->ChooseFromList("Select the model mode", choices);
            if (selected.empty()) {
                cancelled = true;
                return false;
            }
            selectedMode = selected.front();
        }
        outData.selectedMode = selectedMode;
        // Parse channels for the selected mode
        outData.totalChannels = 0;
        outData.isMovingHead = false;
        for (pugi::xml_node n = modes[selectedMode].first_child(); n; n = n.next_sibling()) {
            if (std::string_view(n.name()) == "DMXChannels") {
                for (pugi::xml_node nn = n.first_child(); nn; nn = nn.next_sibling()) {
                    if (std::string_view(nn.name()) == "DMXChannel") {
                        DMXChannelInfo channelInfo = ParseDmxChannel(nn);
                        outData.channels.push_back(channelInfo);
                        outData.totalChannels = std::max(
                            outData.totalChannels,
                            channelInfo.channelStart + channelInfo.channels - 1
                        );
                        outData.isMovingHead |= channelInfo.IsMovingHead();
                    }
                }
            }
        }
        return true;
    }
    Model* CreateDmxModelFromGdtfData(
        Model* existingModel,
        const GdtfModelData& gdtfData,
        ModelManager& modelManager
    ) {
        // Preserve existing model properties
        std::string startChannel = existingModel->GetModelStartChannel();
        auto x = existingModel->GetHcenterPos();
        auto y = existingModel->GetVcenterPos();
        auto w = ((BoxedScreenLocation&)existingModel->GetModelScreenLocation()).GetScaleX();
        auto h = ((BoxedScreenLocation&)existingModel->GetModelScreenLocation()).GetScaleY();
        auto lg = existingModel->GetLayoutGroup();
        TraceLog::AddTraceMessage("GetXlightsModel converted model to DMX");
        delete existingModel;
        existingModel = nullptr;
        // Create appropriate DMX model type
        Model* model = nullptr;
        DmxMovingHead* mh = nullptr;
        if (gdtfData.isMovingHead) {
            model = modelManager.CreateDefaultModel("DmxMovingHeadAdv", startChannel);
            mh = dynamic_cast<DmxMovingHead*>(model);
        } else {
            model = modelManager.CreateDefaultModel("DmxMovingHead", startChannel);
            mh = dynamic_cast<DmxMovingHead*>(model);
            if (mh != nullptr) {
                mh->SetDmxStyle("Moving Head Bars");
            }
        }
        // Restore position and scale
        model->SetHcenterPos(x);
        model->SetVcenterPos(y);
        // Multiply by 5 because default custom model has width and height set to 5 and DMX model is 1 pixel
        ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(w * 5, h * 5);
        model->SetLayoutGroup(lg);
        // Set number of channels
        if (auto* dmx = dynamic_cast<DmxModel*>(model)) {
            dmx->SetDmxChannelCount(gdtfData.totalChannels);
        }
        // Add mode description
        if (!gdtfData.selectedMode.empty()) {
            model->SetDescription("Mode: " + gdtfData.selectedMode);
        }
        // Setup node names and state info
        std::vector<std::string> nodeNames(gdtfData.totalChannels);
        FaceStateData stateInfo;
        // Process each channel
        for (const auto& channel : gdtfData.channels) {
            // Map standard DMX attributes to model attributes
            if (channel.attribute == "Pan") {
                DmxMotor* motor = mh->GetPanMotor();
                motor->SetChannelCoarse(channel.channelStart);
            } else if (channel.attribute == "Tilt") {
                DmxMotor* motor = mh->GetTiltMotor();
                motor->SetChannelCoarse(channel.channelStart);
            } else if (channel.attribute == "ColorAdd_W") {
                if (mh->HasColorAbility()) {
                    DmxColorAbility* color_ability = mh->GetColorAbility();
                    DmxColorAbility::DMX_COLOR_TYPE color_type = color_ability->GetColorType();
                    if (color_type == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW) {
                        auto ability = dynamic_cast<DmxColorAbilityRGB*>(color_ability);
                        ability->SetWhiteChannel(channel.channelStart);
                    } else if (color_type == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_CMYW) {
                        auto ability = dynamic_cast<DmxColorAbilityCMY*>(color_ability);
                        ability->SetWhiteChannel(channel.channelStart);
                    }
                }
            } else if (channel.attribute == "ColorRGB_Red" || channel.attribute == "ColorAdd_R") {
                if (mh->HasColorAbility()) {
                    DmxColorAbility* color_ability = mh->GetColorAbility();
                    DmxColorAbility::DMX_COLOR_TYPE color_type = color_ability->GetColorType();
                    if (color_type == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW) {
                        auto ability = dynamic_cast<DmxColorAbilityRGB*>(color_ability);
                        ability->SetRedChannel(channel.channelStart);
                    }
                }
            } else if (channel.attribute == "ColorRGB_Green" || channel.attribute == "ColorAdd_G") {
                if (mh->HasColorAbility()) {
                    DmxColorAbility* color_ability = mh->GetColorAbility();
                    DmxColorAbility::DMX_COLOR_TYPE color_type = color_ability->GetColorType();
                    if (color_type == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW) {
                        auto ability = dynamic_cast<DmxColorAbilityRGB*>(color_ability);
                        ability->SetGreenChannel(channel.channelStart);
                    }
                }
            } else if (channel.attribute == "ColorRGB_Blue" || channel.attribute == "ColorAdd_B") {
                if (mh->HasColorAbility()) {
                    DmxColorAbility* color_ability = mh->GetColorAbility();
                    DmxColorAbility::DMX_COLOR_TYPE color_type = color_ability->GetColorType();
                    if (color_type == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW) {
                        auto ability = dynamic_cast<DmxColorAbilityRGB*>(color_ability);
                        ability->SetBlueChannel(channel.channelStart);
                    }
                }
            } else if (channel.attribute == "Shutter" || channel.attribute == "Shutter1") {
                if (mh->HasShutterAbility()) {
                    auto shutter_ability = mh->GetShutterAbility();
                    shutter_ability->SetShutterChannel(channel.channelStart);
                }
            }
            // Setup node names for this channel
            for (int i = 0; i < channel.channels; ++i) {
                int nodeIndex = channel.channelStart + i - 1;
                if (nodeIndex >= 0 && nodeIndex < (int)nodeNames.size()) {
                    if (channel.channels == 1) {
                        nodeNames[nodeIndex] = channel.attribute;
                    } else {
                        nodeNames[nodeIndex] = std::format("{}-{}", channel.attribute, i + 1);
                    }
                }
            }
            // Add state information for channels with values
            if (!channel.values.empty()) {
                std::map<std::string, std::string> states;
                int stateNum = 1;
                for (const auto& value : channel.values) {
                    states[std::format("s{}-Name", stateNum)] = value.description;
                    states[std::format("s{}", stateNum)] = std::to_string(channel.channelStart);
                    states[std::format("s{}-Color", stateNum)] = std::format("#{:02x}{:02x}{:02x}", value.low, value.low, value.low);
                    stateNum++;
                }
                stateInfo[channel.attribute] = states;
            }
        }
        // Set node names
        std::string nodeNamesStr;
        for (const auto& name : nodeNames) {
            if (!nodeNamesStr.empty()) {
                nodeNamesStr += ",";
            }
            nodeNamesStr += name;
        }
        model->SetNodeNames(nodeNamesStr);
        // Set state information using the FaceStateData object
        if (!stateInfo.empty()) {
            model->SetStateInfo(stateInfo);
            model->UpdateStateInfoNodes();
        }
        model->Selected(true);
        return model;
    }
} // namespace XmlSerialize

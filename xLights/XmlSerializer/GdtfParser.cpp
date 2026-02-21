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
#include "../xLightsMain.h"
#include "../UtilFunctions.h"
#include <wx/choicdlg.h>
#include <algorithm>

namespace XmlSerialize {
    // Helper function to parse value from GDTF format
    static int ParseDmxValue(const wxString& s, int channels) {
        wxArrayString ss = wxSplit(s, '/');
        if (ss.size() != 2) {
            return 0;
        }
        if (ss[1] == "1") {
            if (channels == 2) {
                return wxAtoi(ss[0]) << 8;
            }
            return wxAtoi(ss[0]);
        } else if (ss[1] == "2") {
            return wxAtoi(ss[0]);
        }
        return 0;
    }
    // Helper function to parse a DMX channel from XML
    static DMXChannelInfo ParseDmxChannel(wxXmlNode* channelNode) {
        DMXChannelInfo info;
        wxString offset = channelNode->GetAttribute("Offset");
        wxArrayString os = wxSplit(offset, ',');
        if (os.size() > 0) {
            info.channelStart = wxAtoi(os[0]);
            info.channels = os.size();
        }
        for (wxXmlNode* nn = channelNode->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
            if (nn->GetName() == "LogicalChannel") {
                info.attribute = nn->GetAttribute("Attribute").ToStdString();
                for (wxXmlNode* nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext()) {
                    if (nnn->GetName() == "ChannelFunction") {
                        for (wxXmlNode* nnnn = nnn->GetChildren(); nnnn != nullptr; nnnn = nnnn->GetNext()) {
                            if (nnnn->GetName() == "ChannelSet" && !nnnn->GetAttribute("Name").empty()) {
                                DMXChannelValue value;
                                value.description = nnnn->GetAttribute("Name").ToStdString();
                                value.low = ParseDmxValue(nnnn->GetAttribute("DMXFrom"), info.channels);
                                wxXmlNode* nextNode = nnnn->GetNext();
                                if (nextNode == nullptr) {
                                    value.high = value.low;
                                } else {
                                    value.high = ParseDmxValue(nextNode->GetAttribute("DMXFrom"), info.channels) - 1;
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
        wxXmlDocument& gdtf_doc,
        xLightsFrame* xlights,
        bool& cancelled,
        GdtfModelData& outData
    ) {
        if (!gdtf_doc.IsOk()) {
            return false;
        }
        // Parse all available DMX modes
        std::map<std::string, wxXmlNode*> modes;
        for (wxXmlNode* n = gdtf_doc.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext()) {
            if (n->GetName() == "FixtureType") {
                for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                    if (nn->GetName() == "DMXModes") {
                        for (wxXmlNode* nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext()) {
                            if (nnn->GetName() == "DMXMode") {
                                modes[nnn->GetAttribute("Name").ToStdString()] = nnn;
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
        if (modes.size() > 1) {
            wxArrayString choices;
            for (const auto& it : modes) {
                choices.push_back(it.first);
            }
            wxSingleChoiceDialog dlg(xlights, "Select the model mode", "DMX Model Mode", choices);
            if (dlg.ShowModal() != wxID_OK) {
                cancelled = true;
                return false;
            }
            selectedMode = choices[dlg.GetSelection()].ToStdString();
        }
        outData.selectedMode = selectedMode;
        // Parse channels for the selected mode
        outData.totalChannels = 0;
        outData.isMovingHead = false;
        for (wxXmlNode* n = modes[selectedMode]->GetChildren(); n != nullptr; n = n->GetNext()) {
            if (n->GetName() == "DMXChannels") {
                for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                    if (nn->GetName() == "DMXChannel") {
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
        xLightsFrame* xlights
    ) {
        // Preserve existing model properties
        std::string startChannel = existingModel->GetModelStartChannel();
        auto x = existingModel->GetHcenterPos();
        auto y = existingModel->GetVcenterPos();
        auto w = ((BoxedScreenLocation&)existingModel->GetModelScreenLocation()).GetScaleX();
        auto h = ((BoxedScreenLocation&)existingModel->GetModelScreenLocation()).GetScaleY();
        auto lg = existingModel->GetLayoutGroup();
        xlights->AddTraceMessage("GetXlightsModel converted model to DMX");
        delete existingModel;
        existingModel = nullptr;
        // Create appropriate DMX model type
        Model* model = nullptr;
        DmxMovingHead* mh = nullptr;
        if (gdtfData.isMovingHead) {
            model = xlights->AllModels.CreateDefaultModel("DmxMovingHeadAdv", startChannel);
            mh = dynamic_cast<DmxMovingHead*>(model);
        } else {
            model = xlights->AllModels.CreateDefaultModel("DmxMovingHead", startChannel);
            mh = dynamic_cast<DmxMovingHead*>(model);
            if (mh != nullptr) {
                mh->SetDmxStyle("Moving Head Bars");
            }
        }
        // Restore position and scale
        model->SetHcenterPos(x);
        model->SetVcenterPos(y);
        // Multiply by 5 because default custom model has parm1 and parm2 set to 5 and DMX model is 1 pixel
        ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(w * 5, h * 5);
        model->SetLayoutGroup(lg);
        // Set number of channels
        model->SetParm1(gdtfData.totalChannels);
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
                        nodeNames[nodeIndex] = wxString::Format("%s-%d", channel.attribute, i + 1).ToStdString();
                    }
                }
            }
            // Add state information for channels with values
            if (!channel.values.empty()) {
                std::map<std::string, std::string> states;
                int stateNum = 1;
                for (const auto& value : channel.values) {
                    states[wxString::Format("s%d-Name", stateNum).ToStdString()] = value.description;
                    states[wxString::Format("s%d", stateNum).ToStdString()] = wxString::Format("%d", channel.channelStart).ToStdString();
                    states[wxString::Format("s%d-Color", stateNum).ToStdString()] = wxString::Format("#%02x%02x%02x", value.low, value.low, value.low).ToStdString();
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
        model->Selected = true;
        return model;
    }
} // namespace XmlSerialize

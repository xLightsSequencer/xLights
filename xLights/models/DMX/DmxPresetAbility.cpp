/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include "DmxPresetAbility.h"
#include "../BaseObject.h"

#include "../Model.h"
#include "../Node.h"
#include "../../Color.h"

void DmxPresetAbility::AddPreset(uint8_t chan, uint8_t val, const std::string& desc)
{
    _presets.emplace_back(chan, val, desc);
}

void DmxPresetAbility::SetPresetValues(xlColorVector& pixelVector) const
{
    for (auto const& pre : _presets) {
        if (pre.DMXChannel != 0 && pre.DMXValue != 0) {
            if (pixelVector.size() > pre.DMXChannel - 1) {
                xlColor c(pre.DMXValue, pre.DMXValue, pre.DMXValue);
                pixelVector[pre.DMXChannel - 1] = c;
            }
        }
    }
}

void DmxPresetAbility::AddProperties(wxPropertyGridInterface* grid, int num_channels) {
    int fixedChannelSize = _presets.size();
    int maxChannelSize = num_channels;
    if (fixedChannelSize > num_channels) {
         fixedChannelSize = num_channels;
    }

    auto p = grid->Append(new wxUIntProperty("Number of Fixed Channels", "DmxPresetSize", fixedChannelSize));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", maxChannelSize);
    p->SetEditor("SpinCtrl");

    while (maxChannelSize < _presets.size()) {
        _presets.pop_back();
    }

    int index{ 0 };
    for (auto const& pre : _presets) {
        auto sp = grid->AppendIn(p, new wxUIntProperty(wxString::Format("Fixed Channel %d DMX Channel", 1 + index),
                                                       wxString::Format("DmxPresetChannel%d", index), pre.DMXChannel));

        sp->SetAttribute("Min", 1);
        sp->SetAttribute("Max", maxChannelSize);
        sp->SetEditor("SpinCtrl");
        sp = grid->AppendIn(p, new wxUIntProperty(wxString::Format("Fixed Channel %d DMX Value", 1 + index),
                                                  wxString::Format("DmxPresetValue%d", index), pre.DMXValue));
        sp->SetAttribute("Min", 0);
        sp->SetAttribute("Max", 255);
        sp->SetEditor("SpinCtrl");

        grid->AppendIn(p, new wxStringProperty(wxString::Format("Fixed Channel %d Description", 1 + index),
                                               wxString::Format("DmxPresetDesc%d", index), pre.Description));

        ++index;
    }
}

int DmxPresetAbility::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, int num_channels, BaseObject* base)
{
    if ("DmxPresetSize" == event.GetPropertyName()) {
        int presetSize = (int)event.GetPropertyValue().GetInteger();
        // if new presets size is less than number of presets, remove unneeded presets

        int dmxChannelCount = num_channels;
        if (presetSize > dmxChannelCount) { // find the #of channels
            presetSize = dmxChannelCount;
        }
        while (presetSize < _presets.size()) {
            _presets.pop_back();
        }
        // if presets size is greater than  number of presets, add needed presets
        int diff = presetSize - _presets.size();
        for (int i = 0; i < diff; i++) {
            _presets.emplace_back(_presets.size() + 1, 0, "");
        }

        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetSize");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetSize");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetSize");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetSize");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxPresetAbility::OnPropertyGridChange::DmxPresetSize");
        return 0;
    } else if (event.GetPropertyName().StartsWith("DmxPresetSize.DmxPresetChannel")) {
        int dxmVal = (int)event.GetPropertyValue().GetInteger();
        wxString namekey = event.GetPropertyName();
        namekey.Replace("DmxPresetSize.DmxPresetChannel", "");
        int index = wxAtoi(namekey);
        if (index >= 0 && index < _presets.size()) {
            _presets[index].DMXChannel = dxmVal;

            base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetChannel");
            base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetChannel");
            base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetChannel");
            base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetChannel");
        }
    } else if (event.GetPropertyName().StartsWith("DmxPresetSize.DmxPresetValue")) {
        int dxmVal = (int)event.GetPropertyValue().GetInteger();
        wxString namekey = event.GetPropertyName();
        namekey.Replace("DmxPresetSize.DmxPresetValue", "");
        int index = wxAtoi(namekey);
        if (index >= 0 && index < _presets.size()) {
            _presets[index].DMXValue = dxmVal;

            base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetValue");
            base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetValue");
            base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetValue");
            base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetValue");
        }
    } else if (event.GetPropertyName().StartsWith("DmxPresetSize.DmxPresetDesc")) {
        auto dxmDesc = event.GetPropertyValue().GetString();
        wxString namekey = event.GetPropertyName();
        namekey.Replace("DmxPresetSize.DmxPresetDesc", "");
        int index = wxAtoi(namekey);
        if (index >= 0 && index < _presets.size()) {
            _presets[index].Description = dxmDesc;

            base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetDesc");
            base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetDesc");
            base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetDesc");
            base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxPresetAbility::OnColorPropertyGridChange::DmxPresetDesc");
        }
    }
    return -1;
}

[[nodiscard]] std::list<std::string> DmxPresetAbility::CheckModelSettings(Model* m) const
{
    std::list<std::string> res;
    auto nodeCount = m->GetNodeCount();

    for (auto const& pre : _presets) {
        if (pre.DMXChannel > nodeCount) {
            res.push_back(wxString::Format("    ERR: Model %s Fixed channel refers to a channel (%d) not present on the model which only has %d channels.", m->GetName(), pre.DMXChannel, nodeCount));
        }
    }

    return res;
}

[[nodiscard]] bool DmxPresetAbility::IsValidModelSettings(Model* m) const
{
    auto nodeCount = m->GetNodeCount();

    for (auto const& pre : _presets) {
        if (pre.DMXChannel > nodeCount) {
            return false;
        }
    }
    return true;
}

void DmxPresetAbility::SetNodeNames(std::vector<std::string>& names) const
{
    for (auto const& pre : _presets) {

        if (0 != pre.DMXChannel && pre.DMXChannel < names.size() && !pre.Description.empty()) {
            names[pre.DMXChannel - 1] = pre.Description;
        }
    }
}

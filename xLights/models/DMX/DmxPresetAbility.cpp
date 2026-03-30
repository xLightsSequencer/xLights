/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <format>

#include "DmxPresetAbility.h"

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
            if ((int)pixelVector.size() > pre.DMXChannel - 1) {
                xlColor c(pre.DMXValue, pre.DMXValue, pre.DMXValue);
                pixelVector[pre.DMXChannel - 1] = c;
            }
        }
    }
}

[[nodiscard]] std::list<std::string> DmxPresetAbility::CheckModelSettings(Model* m) const
{
    std::list<std::string> res;
    auto nodeCount = m->GetNodeCount();

    for (auto const& pre : _presets) {
        if (pre.DMXChannel > nodeCount) {
            res.push_back(std::format("    ERR: Model {} Fixed channel refers to a channel ({}) not present on the model which only has {} channels.", m->GetName(), pre.DMXChannel, nodeCount));
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

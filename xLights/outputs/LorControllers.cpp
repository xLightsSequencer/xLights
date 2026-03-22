
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LorControllers.h"

#include <numeric>

#pragma region Construtors and Destructors
LorControllers::LorControllers(pugi::xml_node node) {

    pugi::xml_node ctrlr_node = node.first_child();
    for (pugi::xml_node e = ctrlr_node.first_child(); e; e = e.next_sibling()) {
        _controllers.push_back(new LorController(e));
    }
    _dirty = false;
}

LorControllers::LorControllers(const LorControllers& from)
{
    for (const auto& it : from._controllers)
    {
        _controllers.push_back(it->Copy());
    }
    _dirty = true;
}

void LorControllers::Save(pugi::xml_node node) {
    for (const auto& it : _controllers) {
        pugi::xml_node cntrl_node = node.append_child("controller");
        it->Save(cntrl_node);
    }
    _dirty = false;
}
#pragma endregion

#pragma region Getters and Setters
int LorControllers::GetTotalChannels() const
{
    return std::accumulate(begin(_controllers), end(_controllers), 0, [](uint32_t accumulator, auto const c) { return accumulator + c->GetTotalNumChannels(); });
}

bool LorControllers::IsDirty() const
{
    if (_dirty) return true;

    return std::any_of(begin(_controllers), end(_controllers), [](auto c) { return c->IsDirty(); });
}

void LorControllers::ClearDirty()
{
    _dirty = false;
    std::for_each(begin(_controllers), end(_controllers), [](auto c) { c->ClearDirty(); });
}
#pragma endregion

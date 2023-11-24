
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "LorControllers.h"

#include <numeric>

#pragma region Construtors and Destructors
LorControllers::LorControllers(wxXmlNode* node) {

    wxXmlNode* ctrlr_node = node->GetChildren();
    for (wxXmlNode* e = ctrlr_node->GetChildren(); e != nullptr; e = e->GetNext()) {
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

void LorControllers::Save(wxXmlNode* node) {
    for (const auto& it : _controllers) {
        wxXmlNode* cntrl_node = new wxXmlNode(wxXML_ELEMENT_NODE, "controller");
        node->AddChild(cntrl_node);
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
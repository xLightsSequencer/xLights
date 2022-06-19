/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include "DmxPresetAbility.h"
#include "../BaseObject.h"

#include "../Model.h"
#include "../Node.h"
#include "../../Color.h"

constexpr int MAX_PRESETS{ 25 };

void DmxPresetAbility::InitPreset(wxXmlNode* ModelXml)
{
    ReadXMLSettings(ModelXml);
}

void DmxPresetAbility::ReadXMLSettings(wxXmlNode* ModelXml)
{
    for (int i = 0; i < MAX_PRESETS; ++i) {
        auto dmxChanKey = wxString::Format("DmxPresetChannel%d", i);
        auto dmxValueKey = wxString::Format("DmxPresetValue%d", i);
        auto descKey = wxString::Format("DmxPresetDesc%d", i);
        if (!ModelXml->HasAttribute(dmxChanKey) || !ModelXml->HasAttribute(dmxValueKey)) {
            break;
        }
        uint8_t dmxChan = wxAtoi(ModelXml->GetAttribute(dmxChanKey, "1"));
        uint8_t dmxVal = wxAtoi(ModelXml->GetAttribute(dmxValueKey, "0"));
        std::string dmxDesc = ModelXml->GetAttribute(descKey);
        presets.emplace_back(dmxChan, dmxVal, dmxDesc);
    }
}

void DmxPresetAbility::SetPresetValues(xlColorVector& pixelVector) const
{
    for (auto const& pre : presets) {
        if (pre.DMXChannel != 0 && pre.DMXValue != 0) {
            if (pixelVector.size() > pre.DMXChannel - 1) {
                xlColor c(pre.DMXValue, pre.DMXValue, pre.DMXValue);
                pixelVector[pre.DMXChannel - 1] = c;
            }
        }
    }
}

void DmxPresetAbility::AddProperties(wxPropertyGridInterface* grid) const
{
    auto p = grid->Append(new wxUIntProperty("Number of Presets", "DmxPresetSize", presets.size()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", MAX_PRESETS);
    p->SetEditor("SpinCtrl");

    int index{ 0 };
    for (auto const& pre : presets) {
        auto sp = grid->AppendIn(p, new wxUIntProperty(wxString::Format("Preset %d DMX Channel", 1 + index),
                                                       wxString::Format("DmxPresetChannel%d", index), pre.DMXChannel));

        sp->SetAttribute("Min", 1);
        sp->SetAttribute("Max", 255);
        sp->SetEditor("SpinCtrl");
        sp = grid->AppendIn(p, new wxUIntProperty(wxString::Format("Preset %d DMX Value", 1 + index),
                                                  wxString::Format("DmxPresetValue%d", index), pre.DMXValue));
        sp->SetAttribute("Min", 0);
        sp->SetAttribute("Max", 255);
        sp->SetEditor("SpinCtrl");

        sp = grid->AppendIn(p, new wxStringProperty(wxString::Format("Preset %d Description", 1 + index),
                                                  wxString::Format("DmxPresetDesc%d", index), pre.Description));

        ++index;
    }
}

int DmxPresetAbility::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base)
{
    if ("DmxPresetSize" == event.GetPropertyName()) {
        int presetSize = (int)event.GetPropertyValue().GetInteger();
        // if new presets size is less than number of presets, remove unneeded presets
        while (presetSize < presets.size()) {
            presets.pop_back();
        }
        // if presets size is greater than  number of presets, add needed presets
        int diff = presetSize - presets.size();
        for (int i = 0; i < diff; i++) {
            presets.emplace_back(presets.size() + 1, 0, "");
        }
        WriteXMLSettings(ModelXml);

        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetSize");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetSize");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetSize");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetSize");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxColorAbility::OnPropertyGridChange::DmxPresetSize");
        return 0;
    } else if (event.GetPropertyName().StartsWith("DmxPresetSize.DmxPresetChannel")) {
        int dxmVal = (int)event.GetPropertyValue().GetInteger();
        wxString namekey = event.GetPropertyName();
        namekey.Replace("DmxPresetSize.DmxPresetChannel", "");
        int index = wxAtoi(namekey);
        if (index >= 0 && index < presets.size()) {
            presets[index].DMXChannel = dxmVal;
            WriteXMLSettings(ModelXml);

            base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetChannel");
            base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetChannel");
            base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetChannel");
            base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetChannel");
        }
    } else if (event.GetPropertyName().StartsWith("DmxPresetSize.DmxPresetValue")) {
        int dxmVal = (int)event.GetPropertyValue().GetInteger();
        wxString namekey = event.GetPropertyName();
        namekey.Replace("DmxPresetSize.DmxPresetValue", "");
        int index = wxAtoi(namekey);
        if (index >= 0 && index < presets.size()) {
            presets[index].DMXValue = dxmVal;
            WriteXMLSettings(ModelXml);

            base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetValue");
            base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetValue");
            base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetValue");
            base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetValue");
        }
    } else if (event.GetPropertyName().StartsWith("DmxPresetSize.DmxPresetDesc")) {
        auto dxmDesc = event.GetPropertyValue().GetString();
        wxString namekey = event.GetPropertyName();
        namekey.Replace("DmxPresetSize.DmxPresetDesc", "");
        int index = wxAtoi(namekey);
        if (index >= 0 && index < presets.size()) {
            presets[index].Description = dxmDesc;
            WriteXMLSettings(ModelXml);

            base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetDesc");
            base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetDesc");
            base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetDesc");
            base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DmxPresetDesc");
        }
    }
    return -1;
}

void DmxPresetAbility::WriteXMLSettings(wxXmlNode* ModelXml) const
{
    for (int i = 0; i < MAX_PRESETS; ++i) {
        auto dmxChanKey = wxString::Format("DmxPresetChannel%d", i);
        auto dmxValueKey = wxString::Format("DmxPresetValue%d", i);
        auto descKey = wxString::Format("DmxPresetDesc%d", i);
        if (ModelXml->HasAttribute(dmxChanKey)) {
            ModelXml->DeleteAttribute(dmxChanKey);
        }
        if (ModelXml->HasAttribute(dmxValueKey)) {
            ModelXml->DeleteAttribute(dmxValueKey);
        }
        if (ModelXml->HasAttribute(descKey)) {
            ModelXml->DeleteAttribute(descKey);
        }
    }
    int index{ 0 };
    for (auto const& pre : presets) {
        auto dmxChanKey = wxString::Format("DmxPresetChannel%d", index);
        auto dmxValueKey = wxString::Format("DmxPresetValue%d", index);
        auto descKey = wxString::Format("DmxPresetDesc%d", index);
        ModelXml->DeleteAttribute(dmxChanKey);
        ModelXml->AddAttribute(dmxChanKey, wxString::Format("%d", pre.DMXChannel));
        ModelXml->DeleteAttribute(dmxValueKey);
        ModelXml->AddAttribute(dmxValueKey, wxString::Format("%d", pre.DMXValue));
        ModelXml->DeleteAttribute(descKey);
        ModelXml->AddAttribute(descKey, pre.Description);
        ++index;
    }
}

[[nodiscard]] std::list<std::string> DmxPresetAbility::CheckModelSettings(Model* m) const
{
    std::list<std::string> res;
    auto nodeCount = m->GetNodeCount();

    for (auto const& pre : presets) {
        if (pre.DMXChannel > nodeCount) {
            res.push_back(wxString::Format("    ERR: Model %s Preset channel refers to a channel (%d) not present on the model which only has %d channels.", m->GetName(), pre.DMXChannel, nodeCount));
        }
    }

    return res;
}

[[nodiscard]] bool DmxPresetAbility::IsValidModelSettings(Model* m) const
{
    auto nodeCount = m->GetNodeCount();

    for (auto const& pre : presets) {
        if (pre.DMXChannel > nodeCount) {
            return false;
        }
    }
    return true;
}

void DmxPresetAbility::ExportParameters(wxFile& f, wxXmlNode* ModelXml) const
{
    for (int i = 0; i < MAX_PRESETS; ++i) {
        auto dmxChanKey = wxString::Format("DmxPresetChannel%d", i);
        auto dmxValueKey = wxString::Format("DmxPresetValue%d", i);
        auto descKey = wxString::Format("DmxPresetDesc%d", i);
        if (!ModelXml->HasAttribute(dmxChanKey) || !ModelXml->HasAttribute(dmxValueKey)) {
            break;
        }

        wxString dmxChan = ModelXml->GetAttribute(dmxChanKey, "0");
        wxString dmxValue = ModelXml->GetAttribute(dmxValueKey, "0");
        wxString dmxDesc = ModelXml->GetAttribute(descKey);
        f.Write(wxString::Format("%s=\"%s\" ", dmxChanKey, dmxChan));
        f.Write(wxString::Format("%s=\"%s\" ", dmxValueKey, dmxValue));
        f.Write(wxString::Format("%s=\"%s\" ", descKey, dmxDesc));
    }
}

void DmxPresetAbility::ImportParameters(wxXmlNode* ImportXml, Model* m) const
{
    for (int i = 0; i < MAX_PRESETS; ++i) {
        auto dmxChanKey = wxString::Format("DmxPresetChannel%d", i);
        auto dmxValueKey = wxString::Format("DmxPresetValue%d", i);
        auto descKey = wxString::Format("DmxPresetDesc%d", i);
        if (!ImportXml->HasAttribute(dmxChanKey) || !ImportXml->HasAttribute(dmxValueKey)) {
            break;
        }

        wxString dmxChan = ImportXml->GetAttribute(dmxChanKey, "0");
        wxString dmxValue = ImportXml->GetAttribute(dmxValueKey, "0");
        wxString dmxDesc = ImportXml->GetAttribute(descKey);
        m->SetProperty(dmxChanKey, dmxChan);
        m->SetProperty(dmxValueKey, dmxValue);
        m->SetProperty(descKey, dmxDesc);
    }
}

void DmxPresetAbility::SetNodeNames(std::vector<std::string>& names) const
{
    for (auto const& pre : presets) {

        if (0 != pre.DMXChannel && pre.DMXChannel < names.size() && !pre.Description.empty()) {
            names[pre.DMXChannel - 1] = pre.Description;
        }
    }
}
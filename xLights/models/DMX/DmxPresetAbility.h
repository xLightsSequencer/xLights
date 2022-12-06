#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "../Node.h"

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;
class Model;
class xlColor;
class wxFile;

struct PresetSetting {
    PresetSetting(uint8_t channel, uint8_t value, std::string description) :
        DMXChannel(channel), DMXValue(value), Description(description)
    {}
    uint8_t DMXChannel;
    uint8_t DMXValue;
    std::string Description;
};

class DmxPresetAbility
{
public:
    DmxPresetAbility(wxXmlNode* ModelXml)
    {
        InitPreset(ModelXml);
    };
    virtual ~DmxPresetAbility() = default;

    virtual void InitPreset(wxXmlNode* ModelXml);
    virtual void SetPresetValues(xlColorVector& pixelVector) const;

    virtual void AddProperties(wxPropertyGridInterface* grid) const;
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base);
    [[nodiscard]] virtual std::list<std::string> CheckModelSettings(Model* m) const;
    [[nodiscard]] virtual bool IsValidModelSettings(Model* m) const;
    virtual void ExportParameters(wxFile& f, wxXmlNode* ModelXml) const;
    virtual void ImportParameters(wxXmlNode* ImportXml, Model* m) const;
    virtual void SetNodeNames(std::vector<std::string> & names) const;

private:
    std::vector<PresetSetting> presets;
    void ReadXMLSettings(wxXmlNode* ModelXml);
    void WriteXMLSettings(wxXmlNode* ModelXml) const;
};

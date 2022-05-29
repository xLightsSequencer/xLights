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

#include "DmxColorAbility.h"
#include <optional>

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

struct WheelColor
{
    WheelColor(xlColor col, uint8_t value) :
        color(std::move(col)), dmxValue(value)
    { }
    xlColor color;
    uint8_t dmxValue;
};

class DmxColorAbilityWheel : public DmxColorAbility
{
    public:
    DmxColorAbilityWheel(wxXmlNode* ModelXml) :
        DmxColorAbility()
    {
        InitColor(ModelXml);
    };
        void InitColor( wxXmlNode* ModelXml) override;
        bool IsColorChannel(uint32_t channel)const override;
        void SetColorPixels(const xlColor& color, xlColorVector & pixelVector ) const override;
        void AddColorTypeProperties(wxPropertyGridInterface *grid)const override;
        int OnColorPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base) override;
        std::list<std::string> CheckModelSettings(Model *m) const override;
        bool IsValidModelSettings(Model* m) const override;
        xlColor GetBeamColor(const std::vector<NodeBaseClassPtr>& Nodes) const override;
        void GetColor(xlColor &color, int transparency, int blackTransparency,
                      bool allowSelected, const xlColor *c, const std::vector<NodeBaseClassPtr> &Nodes) const override;
        [[nodiscard]] xlColor GetColorPixels(xlColorVector const& pixelVector ) const override;
        bool ApplyChannelTransparency(xlColor& color,int transparency, uint32_t channel) const override;
        std::string GetTypeName() const override{ return "ColorWheel" ;};
        void ExportParameters(wxFile& f, wxXmlNode* ModelXml) const override;
        void ImportParameters(wxXmlNode* ImportXml, Model* m) const override;
        void SetNodeNames(std::vector<std::string> & names) const override;

    private:
        int wheel_channel;
        int dimmer_channel;
        std::vector<WheelColor> colors;

        std::optional<xlColor> GetDMXWheelValue(xlColor const & color) const;
        std::optional<xlColor> GetWheelColorFromDMXValue(xlColor const& dmx) const;
        void ReadColorSettings(wxXmlNode* ModelXml);
        void WriteColorSettings(wxXmlNode* ModelXml) const;
};

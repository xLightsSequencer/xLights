#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "DmxColorAbility.h"
#include "../../Color.h"
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
        void AddColorTypeProperties(wxPropertyGridInterface *grid, bool pwm)const override;
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
        void SetNodeNames(std::vector<std::string> & names, const std::string &pfx = "") const override;
        int GetNumChannels() const override;
        [[nodiscard]] uint32_t GetWheelChannel() const { return wheel_channel; }
        [[nodiscard]] uint32_t GetDimmerChannel() const { return dimmer_channel; }
        [[nodiscard]] uint32_t GetWheelDelay() const { return wheel_delay; }
        [[nodiscard]] xlColorVector GetColors() const override;
        [[nodiscard]] std::vector<WheelColor> const& GetWheelColorSettings() const { return colors; };
        [[nodiscard]] size_t GetColorWheelColorSize() const { return colors.size(); };
        [[nodiscard]] int GetDMXWheelIndex(xlColor const& color) const;

        virtual void GetPWMOutputs(std::map<uint32_t, PWMOutput> &map) const override;

    private:
        uint32_t wheel_channel;
        uint32_t dimmer_channel;
        uint32_t wheel_delay;
        std::vector<WheelColor> colors;

        std::optional<xlColor> GetDMXWheelValue(xlColor const & color) const;
        std::optional<xlColor> GetWheelColorFromDMXValue(xlColor const& dmx) const;
        void ReadColorSettings(wxXmlNode* ModelXml);
        void WriteColorSettings(wxXmlNode* ModelXml) const;
};

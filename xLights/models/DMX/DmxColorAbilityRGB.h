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

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

class DmxColorAbilityRGB : public DmxColorAbility
{
    public:
    DmxColorAbilityRGB();

    void InitColor() override;
    bool IsColorChannel(uint32_t channel)const override;
    void SetColorPixels(const xlColor& color, xlColorVector & pixelVector ) const override;
    void AddColorTypeProperties(wxPropertyGridInterface *grid, bool pwm) const override;
    int OnColorPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base) override;
    [[nodiscard]] std::list<std::string> CheckModelSettings(Model *m) const override;
    [[nodiscard]] bool IsValidModelSettings(Model* m) const override;
    [[nodiscard]] xlColor GetBeamColor( const std::vector<NodeBaseClassPtr>& Nodes) const override;

    void GetColor(xlColor &color, int transparency, int blackTransparency,
                  bool allowSelected, const xlColor *c, const std::vector<NodeBaseClassPtr> &Nodes) const override;
    [[nodiscard]] xlColor GetColorPixels(xlColorVector const& pixelVector ) const override;
    bool ApplyChannelTransparency( xlColor &color, int transparency, uint32_t channel) const override;

    [[nodiscard]] std::string GetTypeName() const override{ return "RGBW"; }
    void SetNodeNames(std::vector<std::string> & names, const std::string &pfx = "") const override;
    int GetNumChannels() const override;

    [[nodiscard]] uint32_t GetRedChannel() const { return red_channel; }
    [[nodiscard]] uint32_t GetGreenChannel() const { return green_channel; }
    [[nodiscard]] uint32_t GetBlueChannel() const { return blue_channel; }
    [[nodiscard]] uint32_t GetWhiteChannel() const { return white_channel; }

    void SetRedChannel(uint32_t chan) { red_channel = chan; }
    void SetGreenChannel(uint32_t chan) { green_channel = chan; }
    void SetBlueChannel(uint32_t chan) { blue_channel = chan; }
    void SetWhiteChannel(uint32_t chan) { white_channel = chan; }

    void SetRedBrightness(int brightness) { red_brightness = brightness; }
    void SetGreenBrightness(int brightness) { green_brightness = brightness; }
    void SetBlueBrightness(int brightness) { blue_brightness = brightness; }
    void SetWhiteBrightness(int brightness) { white_brightness = brightness; }

    void SetRedGamma(float gamma) { red_gamma = gamma; }
    void SetGreenGamma(float gamma) { green_gamma = gamma; }
    void SetBlueGamma(float gamma) { blue_gamma = gamma; }
    void SetWhiteGamma(float gamma) { white_gamma = gamma; }

    virtual void GetPWMOutputs(std::map<uint32_t, PWMOutput> &map) const override;

private:
    uint32_t red_channel;
    uint32_t green_channel;
    uint32_t blue_channel;
    uint32_t white_channel;
    
    float red_gamma = 1.0;
    float green_gamma = 1.0;
    float blue_gamma = 1.0;
    float white_gamma = 1.0;
    
    int red_brightness = 100;
    int green_brightness = 100;
    int blue_brightness = 100;
    int white_brightness = 100;
};

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
    DmxColorAbilityRGB(wxXmlNode* ModelXml) :
        DmxColorAbility()
    {
        InitColor(ModelXml);
    };
    void InitColor( wxXmlNode* ModelXml) override;
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

    [[nodiscard]] std::string GetTypeName() const override{ return "RGBW" ;};
    void ExportParameters(wxFile& f, wxXmlNode* ModelXml) const override;
    void ImportParameters(wxXmlNode* ImportXml, Model* m) const override;
    void SetNodeNames(std::vector<std::string> & names, const std::string &pfx = "") const override;
    int GetNumChannels() const override;

    [[nodiscard]] uint32_t GetRedChannel() const { return red_channel; }
    [[nodiscard]] uint32_t GetGreenChannel() const { return green_channel; }
    [[nodiscard]] uint32_t GetBlueChannel() const { return blue_channel; }
    [[nodiscard]] uint32_t GetWhiteChannel() const { return white_channel; }

    void SetRedChannel( wxXmlNode* ModelXml, int chan );
    void SetGreenChannel( wxXmlNode* ModelXml, int chan );
    void SetBlueChannel( wxXmlNode* ModelXml, int chan );
    void SetWhiteChannel( wxXmlNode* ModelXml, int chan );

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

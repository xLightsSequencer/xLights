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

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

class DmxColorAbilityCMY : public DmxColorAbility
{
public:
    DmxColorAbilityCMY();

    void InitColor() override;
    bool IsColorChannel(uint32_t channel) const override;
    void SetColorPixels(const xlColor& color, xlColorVector& pixelVector) const override;
    void AddColorTypeProperties(wxPropertyGridInterface* grid, bool pwm) const override;
    int OnColorPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, BaseObject* base) override;
    [[nodiscard]] std::list<std::string> CheckModelSettings(Model* m) const override;
    [[nodiscard]] bool IsValidModelSettings(Model* m) const override;
    [[nodiscard]] xlColor GetBeamColor(const std::vector<NodeBaseClassPtr>& Nodes) const override;

    void GetColor(xlColor& color, int transparency, int blackTransparency,
                  bool allowSelected, const xlColor* c, const std::vector<NodeBaseClassPtr>& Nodes) const override;
    [[nodiscard]] xlColor GetColorPixels(xlColorVector const& pixelVector) const override;
    bool ApplyChannelTransparency(xlColor& color, int transparency, uint32_t channel) const override;

    [[nodiscard]] std::string GetTypeName() const override { return "CMYW";}

    void SetNodeNames(std::vector<std::string>& names, const std::string &pfx = "") const override;
    int GetNumChannels() const override;

    [[nodiscard]] uint32_t GetCyanChannel() const { return cyan_channel; }
    [[nodiscard]] uint32_t GetMagentaChannel() const { return magenta_channel; }
    [[nodiscard]] uint32_t GetYellowChannel() const { return yellow_channel; }
    [[nodiscard]] uint32_t GetWhiteChannel() const { return white_channel; }

    void SetCyanChannel(int chan) { cyan_channel = chan; }
    void SetMagentaChannel(int chan) { magenta_channel = chan; }
    void SetYellowChannel(int chan) { yellow_channel = chan; }
    void SetWhiteChannel(int chan) { white_channel = chan; }

    [[nodiscard]] uint8_t GetC(xlColor c) const;
    [[nodiscard]] uint8_t GetM(xlColor c) const;
    [[nodiscard]] uint8_t GetY(xlColor c) const;
    [[nodiscard]] float GetK(xlColor c) const;
    [[nodiscard]] xlColor GetRGB(uint8_t cyan, uint8_t magenta, uint8_t yellow) const;
    
    virtual void GetPWMOutputs(std::map<uint32_t, PWMOutput> &map) const override;

private:
    uint32_t cyan_channel;
    uint32_t magenta_channel;
    uint32_t yellow_channel;
    uint32_t white_channel;
};

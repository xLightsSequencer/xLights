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

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

class DmxColorAbilityCMY : public DmxColorAbility
{
public:
    DmxColorAbilityCMY(wxXmlNode* ModelXml) :
        DmxColorAbility()
    {
        InitColor(ModelXml);
    };
    void InitColor(wxXmlNode* ModelXml) override;
    bool IsColorChannel(uint32_t channel) const override;
    void SetColorPixels(const xlColor& color, xlColorVector& pixelVector) const override;
    void AddColorTypeProperties(wxPropertyGridInterface* grid) const override;
    int OnColorPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base) override;
    [[nodiscard]] std::list<std::string> CheckModelSettings(Model* m) const override;
    [[nodiscard]] bool IsValidModelSettings(Model* m) const override;
    [[nodiscard]] xlColor GetBeamColor(const std::vector<NodeBaseClassPtr>& Nodes) const override;

    void GetColor(xlColor& color, int transparency, int blackTransparency,
                  bool allowSelected, const xlColor* c, const std::vector<NodeBaseClassPtr>& Nodes) const override;
    [[nodiscard]] xlColor GetColorPixels(xlColorVector const& pixelVector) const override;
    bool ApplyChannelTransparency(xlColor& color, int transparency, uint32_t channel) const override;

    [[nodiscard]] std::string GetTypeName() const override
    {
        return "CMYW";
    };
    void ExportParameters(wxFile& f, wxXmlNode* ModelXml) const override;
    void ImportParameters(wxXmlNode* ImportXml, Model* m) const override;
    void SetNodeNames(std::vector<std::string>& names) const override;

    [[nodiscard]] uint32_t GetCyanChannel() const
    {
        return cyan_channel;
    }
    [[nodiscard]] uint32_t GetMagentaChannel() const
    {
        return magenta_channel;
    }
    [[nodiscard]] uint32_t GetYellowChannel() const
    {
        return yellow_channel;
    }
    [[nodiscard]] uint32_t GetWhiteChannel() const
    {
        return white_channel;
    }
    [[nodiscard]] uint8_t GetC(xlColor c) const;
    [[nodiscard]] uint8_t GetM(xlColor c) const;
    [[nodiscard]] uint8_t GetY(xlColor c) const;
    [[nodiscard]] float GetK(xlColor c) const;
    [[nodiscard]] xlColor GetRGB(uint8_t cyan, uint8_t magenta, uint8_t yellow) const;

        private:
    uint32_t cyan_channel;
    uint32_t magenta_channel;
    uint32_t yellow_channel;
    uint32_t white_channel;
};

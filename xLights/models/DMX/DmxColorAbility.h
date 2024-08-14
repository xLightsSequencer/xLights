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

#include <map>

#include "../Node.h"
#include "../PWMOutput.h"

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;
class Model;
class xlColor;
class wxFile;

class DmxColorAbility
{
    public:

        virtual ~DmxColorAbility() = default;

        virtual void InitColor( wxXmlNode* ModelXml) = 0;
        virtual bool IsColorChannel(uint32_t channel) const = 0;
        virtual void SetColorPixels(const xlColor& color, xlColorVector & pixelVector ) const = 0;

        virtual void AddColorTypeProperties(wxPropertyGridInterface *grid, bool pwm) const = 0;
        virtual int OnColorPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base) = 0;
        virtual void GetColor(xlColor& color, int transparency, int blackTransparency,
                              bool allowSelected, const xlColor* c, const std::vector<NodeBaseClassPtr>& Nodes) const = 0;
        [[nodiscard]] virtual xlColor GetColorPixels(xlColorVector const& pixelVector ) const = 0;
        [[nodiscard]] virtual std::list<std::string> CheckModelSettings(Model *m) const = 0;
        [[nodiscard]] virtual bool IsValidModelSettings(Model* m) const = 0;

        [[nodiscard]] virtual xlColor GetBeamColor( const std::vector<NodeBaseClassPtr>& Nodes) const = 0;
        virtual bool ApplyChannelTransparency(xlColor& color, int transparency, uint32_t channel) const = 0;
        [[nodiscard]] virtual std::string GetTypeName() const = 0;
        virtual void ExportParameters(wxFile& f, wxXmlNode* ModelXml) const = 0;
        virtual void ImportParameters(wxXmlNode* ImportXml, Model* m) const = 0;
        virtual void SetNodeNames(std::vector<std::string> & names, const std::string &pfx = "") const = 0;
        virtual int GetNumChannels() const = 0;
        [[nodiscard]] virtual xlColorVector GetColors() const { return xlColorVector(); }

        virtual void GetPWMOutputs(std::map<uint32_t, PWMOutput> &map) const = 0;
    protected:
        DmxColorAbility() = default;

        [[nodiscard]] virtual bool CheckChannel(uint32_t chan, size_t vecSize) const
        {
            return chan > 0 && vecSize >= chan;
        };
    private:
};

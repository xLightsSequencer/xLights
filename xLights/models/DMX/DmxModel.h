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

#include "../Model.h"

#include <memory>
class DmxColorAbility;
class DmxPresetAbility;
class wxFile;

enum DMX_COLOR_TYPES {
    DMX_COLOR_TYPE_RGBW,
    DMX_COLOR_TYPE_WHEEL,
    DMX_COLOR_TYPE_CMYW
};

static const char* DMX_COLOR_TYPES_VALUES[] = {
    "RGBW",
    "ColorWheel",
    "CMYW"
};

static wxPGChoices DMX_COLOR_TYPES(wxArrayString(3, DMX_COLOR_TYPES_VALUES));

class DmxModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        DmxModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxModel();

        static void DrawInvalid(xlGraphicsProgram* pg, ModelScreenLocation* msl, bool is_3d, bool applyTransform);

        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
                                   int &BufferWi, int &BufferHi, int stagger) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                           std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi, int stagger, bool deep = false) const override;

        virtual void AddDimensionProperties(wxPropertyGridInterface* grid) override {}
        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual void DisableUnusedProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual std::string GetDimension() const override { return ""; }

        [[nodiscard]] bool HasColorAbility() const { return nullptr != color_ability ; }
        [[nodiscard]] DmxColorAbility* GetColorAbility() const { return color_ability.get(); }
        [[nodiscard]] bool HasPresetAbility() const { return nullptr != preset_ability ; }
        [[nodiscard]] DmxPresetAbility* GetPresetAbility() const { return preset_ability.get(); }
        virtual void EnableFixedChannels(xlColorVector& pixelVector) const;
        virtual bool SupportsXlightsModel() override { return true; }
        virtual bool SupportsExportAsCustom() const override { return false; }
        virtual bool SupportsWiringView() const override { return false; }
        virtual void ExportXlightsModel() override {}
        [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y, float& min_z, float& max_z) override = 0;
        virtual int GetNumPhysicalStrings() const override { return 1; }
        virtual bool IsDMXModel() const override { return true; }
        virtual std::list<std::string> CheckModelSettings() override;

        [[nodiscard]] virtual std::vector<std::string> GenerateNodeNames() const;

        static int DmxColorTypetoID(std::string const& color_type) {
            if (color_type == "RGBW") {
                return DMX_COLOR_TYPE_RGBW;
            }
            if (color_type == "ColorWheel") {
                return DMX_COLOR_TYPE_WHEEL;
            }
            if (color_type == "CMYW") {
                return DMX_COLOR_TYPE_CMYW;
            }
            return DMX_COLOR_TYPE_RGBW;
        }

        virtual std::vector<PWMOutput> GetPWMOutputs() const override;
        virtual void GetPWMOutputs(std::map<uint32_t, PWMOutput> &channels) const;
    protected:
        virtual void InitModel() override;
        void ExportBaseParameters(wxFile& f);
        bool ImportBaseParameters(wxXmlNode* root);
        void UpdateChannelCount(int num_channels, bool do_work);

        virtual int GetChannelValue( int channel, bool bits16);
        int GetChannelValue(int channel_coarse, int channel_fine);
       void SetNodeNames(const std::string& default_names, bool force = false);

        std::unique_ptr<DmxColorAbility> color_ability{ nullptr };
        std::unique_ptr<DmxPresetAbility> preset_ability{ nullptr };

    private:
};


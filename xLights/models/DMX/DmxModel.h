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
class DmxBeamAbility;
class DmxColorAbility;
class DmxDimmerAbility;
class DmxPresetAbility;
class DmxShutterAbility;
class wxFile;

class DmxModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        DmxModel(const ModelManager &manager);
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

        [[nodiscard]] bool HasBeamAbility() const { return nullptr != beam_ability ; }
        [[nodiscard]] DmxBeamAbility* GetBeamAbility() const { return beam_ability.get(); }
        [[nodiscard]] bool HasColorAbility() const { return nullptr != color_ability ; }
        [[nodiscard]] DmxColorAbility* GetColorAbility() const { return color_ability.get(); }
        [[nodiscard]] bool HasPresetAbility() const { return nullptr != preset_ability ; }
        [[nodiscard]] DmxPresetAbility* GetPresetAbility() const { return preset_ability.get(); }
        [[nodiscard]] bool HasShutterAbility() const { return nullptr != shutter_ability ; }
        [[nodiscard]] DmxShutterAbility* GetShutterAbility() const { return shutter_ability.get(); }
        [[nodiscard]] bool HasDimmerAbility() const { return nullptr != dimmer_ability ; }
        [[nodiscard]] DmxDimmerAbility* GetDimmerAbility() const { return dimmer_ability.get(); }
        void InitColorAbility(int type);

        virtual void EnableFixedChannels(xlColorVector& pixelVector) const;
        virtual bool SupportsXlightsModel() override { return true; }
        virtual bool SupportsExportAsCustom() const override { return false; }
        virtual bool SupportsWiringView() const override { return false; }
        virtual int GetNumPhysicalStrings() const override { return 1; }
        virtual bool IsDMXModel() const override { return true; }
        virtual std::list<std::string> CheckModelSettings() override;

        [[nodiscard]] virtual std::vector<std::string> GenerateNodeNames() const;

        virtual std::vector<PWMOutput> GetPWMOutputs() const override;
        virtual void GetPWMOutputs(std::map<uint32_t, PWMOutput> &channels) const;
    protected:
        virtual void InitModel() override;
        void UpdateChannelCount(int num_channels, bool do_work);

        virtual int GetChannelValue( int channel, bool bits16);
        int GetChannelValue(int channel_coarse, int channel_fine);
        void SetNodeNames(const std::string& default_names, bool force = false);

        std::unique_ptr<DmxBeamAbility> beam_ability{ nullptr };
        std::unique_ptr<DmxColorAbility> color_ability{ nullptr };
        std::unique_ptr<DmxPresetAbility> preset_ability{ nullptr };
        std::unique_ptr<DmxShutterAbility> shutter_ability{ nullptr };
        std::unique_ptr<DmxDimmerAbility> dimmer_ability{ nullptr };

    private:
};


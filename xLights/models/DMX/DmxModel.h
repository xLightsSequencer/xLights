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

#include "../Model.h"
class DmxColorAbility;
class wxFile;


class DmxModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        DmxModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxModel();

        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
                                   int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                           std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;

        virtual void AddDimensionProperties(wxPropertyGridInterface* grid) override {}
        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual void DisableUnusedProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual std::string GetDimension() const override { return ""; }

        virtual bool HasColorAbility() { return false; }
        DmxColorAbility* GetColorAbility() { return color_ability; }
        virtual bool SupportsXlightsModel() override { return true; }
        virtual bool SupportsExportAsCustom() const override { return false; }
        virtual bool SupportsWiringView() const override { return false; }
        virtual void ExportXlightsModel() override = 0;
        virtual void ImportXlightsModel(std::string const& filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override = 0;
        virtual int GetNumPhysicalStrings() const override { return 1; }
        virtual bool IsDMXModel() const override { return true; }

    protected:
        virtual void InitModel() override;
        void ExportBaseParameters(wxFile& f);
        void ImportBaseParameters(wxXmlNode* root);
        void UpdateChannelCount(int num_channels, bool do_work);

        virtual int GetChannelValue( int channel, bool bits16);
        void SetNodeNames(const std::string& default_names, bool force = false);

        DmxColorAbility* color_ability;

    private:
};


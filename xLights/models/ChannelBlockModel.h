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

#include "Model.h"
#include <vector>

class ChannelBlockModel : public ModelWithScreenLocation<TwoPointScreenLocation>
{
    public:
        ChannelBlockModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~ChannelBlockModel();
    
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
                                   int &BufferWi, int &BufferHi, int stagger) const override;
        virtual void InitRenderBufferNodes(const std::string& type, const std::string& camera, const std::string& transform,
                                           std::vector<NodeBaseClassPtr>& Nodes, int& BufferWi, int& BufferHi, int stagger, bool deep = false) const override;
        virtual void AddDimensionProperties(wxPropertyGridInterface* grid) override {}
        virtual std::string GetDimension() const override { return ""; }
        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void DisableUnusedProperties(wxPropertyGridInterface *grid) override;
        virtual int GetNumPhysicalStrings() const override { return 1; }
        virtual bool SupportsExportAsCustom() const override { return false; }
        virtual bool SupportsWiringView() const override { return false; }

    protected:
        virtual void InitModel() override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int GetNumStrands() const override;
        virtual int CalcCannelsPerString() override;

    private:
		void InitChannelBlock();
        static std::vector<std::string> LINE_BUFFER_STYLES;
        static std::string ChanColorAttrName(int idx)
        {
            return wxString::Format(wxT("ChannelColor%i"), idx + 1).ToStdString();  // a space between "String" and "%i" breaks the start channels listed in Indiv Start Chans
        }
        void AdjustChannelProperties(wxPropertyGridInterface *grid, int newNum);
};

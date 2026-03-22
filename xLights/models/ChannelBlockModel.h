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

#include "Model.h"
#include <vector>

class ChannelBlockModel : public ModelWithScreenLocation<TwoPointScreenLocation>
{
    public:
        ChannelBlockModel(const ModelManager &manager);
        virtual ~ChannelBlockModel();
    
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
                                   int &BufferWi, int &BufferHi, int stagger) const override;
        virtual void InitRenderBufferNodes(const std::string& type, const std::string& camera, const std::string& transform,
                                           std::vector<NodeBaseClassPtr>& Nodes, int& BufferWi, int& BufferHi, int stagger, bool deep = false) const override;
        virtual std::string GetDimension() const override { return ""; }
        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual int GetNumPhysicalStrings() const override { return 1; }
        virtual bool SupportsExportAsCustom() const override { return false; }
        virtual bool SupportsWiringView() const override { return false; }
        std::vector<std::string> const& GetChannelColors() const { return _channelColors; }
        virtual int GetNumStrands() const override;

        void SetChannelColor(int idx, const std::string & color) { _channelColors[idx] = color; }
        virtual int GetNumStrings() const override { return _numChannels; }

        [[nodiscard]] int GetNumChannels() const { return _numChannels; }
        void SetNumChannels(int val) { _numChannels = val; }

        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

    protected:
        virtual void InitModel() override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int CalcChannelsPerString() override;

    private:
        int _numChannels = 1;
        std::vector<std::string> _channelColors;
		void InitChannelBlock();
        static std::vector<std::string> LINE_BUFFER_STYLES;
        static std::string ChanColorAttrName(int idx)
        {
            return "ChannelColor" + std::to_string(idx + 1);  // a space between "String" and "%i" breaks the start channels listed in Indiv Start Chans
        }
};

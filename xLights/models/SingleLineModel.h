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

class SingleLineModel : public ModelWithScreenLocation<TwoPointScreenLocation>
{
    public:
        SingleLineModel(const ModelManager &manager);
        //SingleLineModel(int lights, const Model &base, int strand, int node = -1);
        virtual ~SingleLineModel();

        void InitLine();
    
        void Reset(int lights, const Model &base, int strand, int node = -1, bool forceDirection = false);
        virtual const std::vector<std::string> &GetBufferStyles() const override;
        
        virtual int GetLightsPerNode() const override { return _lightsPerNode; }
        virtual int GetNumStrings() const override { return _numStrings; }
        virtual int NodesPerString() const override;
        using Model::NodesPerString; // bring NodesPerString(int) into scope
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return false; }

        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

        [[nodiscard]] int GetNumLines() const { return _numStrings; }
        [[nodiscard]] int GetNodesPerString() const { return _nodesPerString; }
        void SetNumLines(int val) { _numStrings = val; }
        void SetNodesPerLine(int val) { _nodesPerString = val; }
        void SetLightsPerNode(int val) { _lightsPerNode = val; }

        const Model *GetParent() { return parent; }
        virtual bool SupportsModelScreenLocation() const override { return validLocation; }

    protected:
        static std::vector<std::string> LINE_BUFFER_STYLES;
        virtual void InitModel() override;
        virtual bool IsNodeFirst(int n) const override;

    private:
        int _numStrings = 1;
        int _nodesPerString = 1;
        int _lightsPerNode = 1;
        const Model *parent = nullptr;
        bool validLocation = true;
};

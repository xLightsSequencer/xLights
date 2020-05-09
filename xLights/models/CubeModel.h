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

class CubeModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
    
        CubeModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~CubeModel();

        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi) const override;
        virtual int GetNumPhysicalStrings() const override { return GetStrings(); }
        virtual bool SupportsXlightsModel() override { return true; }
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return false; }
        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
            std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;
        virtual int NodeRenderOrder() override { return 1; }
        virtual int GetStrandLength(int strand) const override { return _strandLength; }
        virtual int GetNumStrands() const override { return _strands; };
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual void ExportAsCustomXModel() const override;
        virtual int NodesPerString() const override;

        virtual std::string ChannelLayoutHtml(OutputManager * outputManager) override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    protected:
        int GetStartIndex() const;
        int GetStyleIndex() const;
        int GetStrandStyleIndex() const;
        std::tuple<int, int, int>& FlipX(std::tuple<int, int, int>& pt, int width) const;
        std::tuple<int, int, int>& RotateY90Degrees(std::tuple<int, int, int>& pt, int by, int width, int depth) const;
        std::tuple<int, int, int>& RotateZ90Degrees(std::tuple<int, int, int>& pt, int by, int width, int height) const;
        std::tuple<int, int, int>& RotateX90Degrees(std::tuple<int, int, int>& pt, int by, int height, int depth) const;
        int CalcTransformationIndex() const;
        std::vector<std::tuple<int, int, int>> BuildCube() const;
        bool IsStrandPerLayer() const;
        virtual std::string GetStartLocation() const override;
        int GetStrings() const;
        void DumpNodes(std::vector<std::tuple<int, int, int>> nodes,int width, int height, int depth) const;
        int FindNodeIndex(std::vector<std::tuple<int, int, int>> nodes, int x, int y, int z) const;
        void DumpNode(const std::string desc, const std::tuple<int, int, int>& node, int width, int height, int depth) const;

        CubeModel(const ModelManager &manager);
        virtual void InitModel() override;
        
    private:
        unsigned long _lastChangeCount = -999;
        int _strandLength = 1;
        int _strands = 1;
};

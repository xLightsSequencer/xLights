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

class CubeModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
    
        CubeModel(const ModelManager &manager);
        virtual ~CubeModel();

        [[nodiscard]] virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi, int stagger) const override;
        [[nodiscard]] virtual int GetNumPhysicalStrings() const override;
        [[nodiscard]] virtual bool SupportsWiringView() const override { return false; }
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
            std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi, int stagger, bool deep = false) const override;
        [[nodiscard]] virtual int NodeRenderOrder() override { return 1; }
        [[nodiscard]] virtual int GetStrandLength(int strand) const override { return _strandLength; }
        [[nodiscard]] virtual int GetNumStrands() const override { return _strands; };
        [[nodiscard]] virtual int MapToNodeIndex(int strand, int node) const override;
        virtual void ExportAsCustomXModel3D() const override;
        [[nodiscard]] virtual bool SupportsExportAsCustom3D() const override { return true; }
        [[nodiscard]] virtual bool SupportsExportAsCustom() const override { return false; }
        [[nodiscard]] virtual int NodesPerString() const override;

        [[nodiscard]] virtual std::string ChannelLayoutHtml(OutputManager * outputManager) override;

        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        [[nodiscard]] virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

        [[nodiscard]] std::string GetCubeStyle() const;
        [[nodiscard]] std::string GetStrandStyle() const;
        [[nodiscard]] int GetCubeStrings() const { return _cubeStrings; }
        [[nodiscard]] std::string GetCubeStart() const;
        [[nodiscard]] bool IsStrandPerLayer() const { return _strandPerLayer; }

        void SetCubeStyle(const std::string & style);
        void SetStrandStyle(const std::string & style);
        void SetCubeStrings(int strings) { _cubeStrings = strings; }
        void SetCubeStart(const std::string & start);
        void SetStrandPerLayer(bool val) { _strandPerLayer = val; }

    protected:
        void FlipX(std::tuple<int, int, int>& pt, int width) const;
        void RotateX90Degrees(std::tuple<int, int, int>& pt, int by, int height, int depth) const;
        void RotateY90Degrees(std::tuple<int, int, int>& pt, int by, int width, int depth) const;
        void RotateZ90Degrees(std::tuple<int, int, int>& pt, int by, int width, int height) const;
        [[nodiscard]] int CalcTransformationIndex() const;
        [[nodiscard]] std::vector<std::tuple<int, int, int>> BuildCube() const;
        [[nodiscard]] virtual std::string GetStartLocation() const override;
        void DumpNodes(std::vector<std::tuple<int, int, int>> nodes,int width, int height, int depth) const;
        [[nodiscard]] int FindNodeIndex(std::vector<std::tuple<int, int, int>> nodes, int x, int y, int z) const;
        void DumpNode(const std::string desc, const std::tuple<int, int, int>& node, int width, int height, int depth) const;

        virtual void InitModel() override;
        
    private:
        int _strandLength = 1;
        int _strands = 1;
        int _cubeStart = 0;
        int _cubeStrings = 1;
        int _cubeStyle = 0;
        int _strandStyle = 0;
        bool _strandPerLayer = FALSE;
};

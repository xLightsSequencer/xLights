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

#include <set>

#include "Model.h"

class SubModel : public Model {
public:
    SubModel(Model *p, wxXmlNode* node);  // TODO:  delete this
    SubModel(Model *p, const std::string _name, const std::string layout, const std::string type, const std::string bufferStyle);
    virtual ~SubModel() {}

    static const std::vector<std::string> BUFFER_STYLES;

    virtual std::string GetFullName() const override { return parent->GetFullName() + "/" + name;}

    virtual const ModelScreenLocation &GetModelScreenLocation() const override { return parent->GetModelScreenLocation(); }
    virtual ModelScreenLocation &GetModelScreenLocation() override { return parent->GetModelScreenLocation(); };

    virtual const ModelScreenLocation &GetBaseObjectScreenLocation() const override { return parent->GetModelScreenLocation(); }
    virtual ModelScreenLocation &GetBaseObjectScreenLocation() override { return parent->GetModelScreenLocation(); };
    virtual glm::vec3 MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override {
        return glm::vec3(0, 0, 0);
    }
    virtual glm::vec3 MoveHandle3D(float scale, int handle, glm::vec3 &rot, glm::vec3 &mov) override {
        return glm::vec3(0, 0, 0);
    }


    virtual const std::string &GetLayoutGroup() const override { return parent->GetLayoutGroup(); }

    virtual void AddProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual void UpdateProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override {}
    virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override {}
    virtual bool SupportsExportAsCustom() const override { return false; }
    virtual bool SupportsWiringView() const override { return false; }
    bool IsNodesAllValid() const { return _nodesAllValid; }

    Model* GetParent() const { return parent; }

    static const std::vector<std::string> GetBufferStyleList() {
        return BUFFER_STYLES;
    }
    
    virtual const std::vector<std::string>& GetBufferStyles() const override;
    virtual const std::string AdjustBufferStyle(const std::string &style) const override;

    virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi, int stagger) const override;
    virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
        std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi, int stagger, bool deep = false) const override;

    std::string GetSameLineDuplicates() const {
        return _sameLineDuplicates;
    }
    std::string GetCrossLineDuplicates() const {
        return _crossLineDuplicates;
    }

    [[nodiscard]] FaceStateData const& GetFaceInfo() const override { return parent->faceInfo; };
    [[nodiscard]] FaceStateNodes const& GetFaceInfoNodes() const override { return parent->faceInfoNodes; };

    std::string GetSubModelLayout() const { return _layout; }
    std::string GetSubModelType() const { return _type; }
    std::string GetSubModelBufferStyle() const { return _bufferStyle; }
    std::string GetSubModelLines() const { return _propertyGridDisplay; }

    // Functions for adding the different buffer types
    void AddDefaultBuffer( wxString const& nodes );
    void AddRangeXY( wxString const& nodes );
    void AddSubbuffer(std::string const& range );
    
    void CheckDuplicates();
    void CalcRangeXYBufferSize();

    [[nodiscard]] bool IsRanges() const { return _isRanges; }
    [[nodiscard]] bool IsVertical() const { return _vert; }
    [[nodiscard]] bool IsXYBufferStyle();

    private:
    Model *parent = nullptr;
    bool _nodesAllValid = false;
    bool _vert = false;
    bool _isRanges = true;
    const std::string _layout;
    const std::string _type;
    const std::string _bufferStyle;
    std::string _propertyGridDisplay;
    std::string _sameLineDuplicates;
    std::string _crossLineDuplicates;
    std::vector<int> _nodeIndexes;
    std::set<int> _nodeIdx;
    unsigned int _startChannel = UINT32_MAX;

    static std::vector<std::string> SUBMODEL_BUFFER_STYLES;

    // variables only used for default buffer
    int _row = 0;
    int _col = 0;
    int _maxRow = 0;
    int _maxCol = 0;
    std::map<int, int> _nodeIndexMap;

};


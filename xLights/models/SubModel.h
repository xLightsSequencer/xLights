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

#include <set>

#include "Model.h"

class SubModel : public Model {
public:
    SubModel(Model *p, wxXmlNode *n);
    virtual ~SubModel() {}

    static const std::vector<std::string> BUFFER_STYLES;

    virtual std::string GetFullName() const override { return parent->GetFullName() + "/" + name;}

    virtual const ModelScreenLocation &GetModelScreenLocation() const override { return parent->GetModelScreenLocation(); }
    virtual ModelScreenLocation &GetModelScreenLocation() override { return parent->GetModelScreenLocation(); };

    virtual const ModelScreenLocation &GetBaseObjectScreenLocation() const override { return parent->GetModelScreenLocation(); }
    virtual ModelScreenLocation &GetBaseObjectScreenLocation() override { return parent->GetModelScreenLocation(); };
    virtual glm::vec3 MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override { return glm::vec3(0, 0, 0); }

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
    std::string GetDuplicateNodes() const { return _duplicateNodes; }

private:
    void CheckDuplicates(const std::vector<int>& nodeIndexes);

    Model *parent = nullptr;
    bool _nodesAllValid = false;
    const std::string _layout;
    const std::string _type;
    const std::string _bufferStyle;
    std::string _properyGridDisplay;
    std::string _duplicateNodes;
};


/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "TreePropertyAdapter.h"
#include "../../../models/TreeModel.h"
#include "../../../OutputModelManager.h"

static const char* TREE_STYLES_VALUES[] = {
    "Round",
    "Flat",
    "Ribbon"
};
static wxPGChoices TREE_STYLES(wxArrayString(3, TREE_STYLES_VALUES));

static const char* TREE_DIRECTION_VALUES[] = {
    "Horizontal",
    "Vertical"
};
static wxPGChoices TREE_DIRECTIONS(wxArrayString(2, TREE_DIRECTION_VALUES));

TreePropertyAdapter::TreePropertyAdapter(Model& model)
    : MatrixPropertyAdapter(model), _tree(static_cast<TreeModel&>(model)) {}

void TreePropertyAdapter::AddStyleProperties(wxPropertyGridInterface* grid) {
    grid->Append(new wxEnumProperty("Type", "TreeStyle", TREE_STYLES, _tree.GetTreeType()));

    wxPGProperty* p = grid->Append(new wxUIntProperty("Degrees", "TreeDegrees", _tree.GetTreeType() == 0 ? _tree.GetTreeDegrees() : 180));
    p->SetAttribute("Min", "1");
    p->SetAttribute("Max", "360");
    p->SetEditor("SpinCtrl");
    p->Enable(_tree.GetTreeType() == 0);

    p = grid->Append(new wxFloatProperty("Rotation", "TreeRotation", _tree.GetTreeType() == 0 ? _tree.GetTreeRotation() : 3));
    p->SetAttribute("Min", "-360");
    p->SetAttribute("Max", "360");
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");
    p->Enable(_tree.GetTreeType() == 0);

    p = grid->Append(new wxFloatProperty("Spiral Wraps", "TreeSpiralRotations", _tree.GetTreeType() == 0 ? _tree.GetSpiralRotations() : 0.0));
    p->SetAttribute("Min", "-200");
    p->SetAttribute("Max", "200");
    p->SetAttribute("Precision", 2);
    p->SetEditor("SpinCtrl");
    p->Enable(_tree.GetTreeType() == 0);

    p = grid->Append(new wxFloatProperty("Bottom/Top Ratio", "TreeBottomTopRatio", _tree.GetTreeType() == 0 ? _tree.GetBottomTopRatio() : 6.0));
    p->SetAttribute("Min", "-50");
    p->SetAttribute("Max", "50");
    p->SetAttribute("Step", 0.5);
    p->SetAttribute("Precision", 2);
    p->SetEditor("SpinCtrl");
    p->Enable(_tree.GetTreeType() == 0);

    p = grid->Append(new wxFloatProperty("Perspective", "TreePerspective", _tree.GetTreeType() == 0 ? _tree.GetTreePerspective() * 10 : 2));
    p->SetAttribute("Min", "0");
    p->SetAttribute("Max", "10");
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");
    p->Enable(_tree.GetTreeType() == 0);

    p = grid->Append(new wxBoolProperty("Alternate Nodes", "AlternateNodes", _tree.HasAlternateNodes()));
    p->SetEditor("CheckBox");
    p->Enable(_tree.IsNoZigZag() == false);

    p = grid->Append(new wxBoolProperty("Don't Zig Zag", "NoZig", _tree.IsNoZigZag()));
    p->SetEditor("CheckBox");
    p->Enable(_tree.HasAlternateNodes() == false);

    grid->Append(new wxEnumProperty("Strand Direction", "StrandDir", TREE_DIRECTIONS, _tree.isVerticalMatrix() ? 1 : 0));
}

int TreePropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if (event.GetPropertyName() == "TreeStyle") {
        _tree.SetTreeType(event.GetPropertyValue().GetLong());
        wxPGProperty* p = grid->GetPropertyByName("TreeDegrees");
        if (p != nullptr) {
            p->Enable(_tree.GetTreeType() == 0);
        }
        p = grid->GetPropertyByName("TreeRotation");
        if (p != nullptr) {
            p->Enable(_tree.GetTreeType() == 0);
        }
        _tree.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "TreePropertyAdapter::OnPropertyGridChange::TreeStyle");
        return 0;
    } else if (event.GetPropertyName() == "TreeDegrees") {
        _tree.SetTreeDegrees(event.GetPropertyValue().GetLong());
        _tree.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "TreePropertyAdapter::OnPropertyGridChange::TreeDegrees");
        return 0;
    } else if (event.GetPropertyName() == "TreeRotation") {
        _tree.SetTreeRotation(static_cast<float>(event.GetPropertyValue().GetDouble()));
        _tree.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "TreePropertyAdapter::OnPropertyGridChange::TreeRotation");
        return 0;
    } else if (event.GetPropertyName() == "TreeSpiralRotations") {
        _tree.SetTreeSpiralRotations(static_cast<float>(event.GetPropertyValue().GetDouble()));
        _tree.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "TreePropertyAdapter::OnPropertyGridChange::TreeSpiralRotations");
        return 0;
    } else if (event.GetPropertyName() == "TreeBottomTopRatio") {
        _tree.SetTreeBottomTopRatio(static_cast<float>(event.GetPropertyValue().GetDouble()));
        _tree.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "TreePropertyAdapter::OnPropertyGridChange::TreeBottomTopRatio");
        return 0;
    } else if (event.GetPropertyName() == "TreePerspective") {
        _tree.SetPerspective(static_cast<float>(event.GetPropertyValue().GetDouble() / 10.0));
        _tree.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "TreePropertyAdapter::OnPropertyGridChange::TreePerspective");
        return 0;
    } else if ("StrandDir" == event.GetPropertyName()) {
        _tree.SetVertical(event.GetPropertyValue().GetBool());
        _tree.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "TreePropertyAdapter::OnPropertyGridChange::StrandDir");
        return 0;
    }

    return MatrixPropertyAdapter::OnPropertyGridChange(grid, event);
}

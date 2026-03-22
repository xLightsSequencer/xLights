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

#include "SubModelPropertyAdapter.h"
#include "../ScreenLocationPropertyHelper.h"
#include "../../../models/SubModel.h"
#include "../../../models/ModelManager.h"

SubModelPropertyAdapter::SubModelPropertyAdapter(SubModel& subModel)
    : ModelPropertyAdapter(subModel), _subModel(subModel) {}

void SubModelPropertyAdapter::AddProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    // SubModel has a simplified property grid - just type properties, no controller/channel stuff
    AddTypeProperties(grid, outputManager);
}

void SubModelPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxStringProperty("SubModel Type", "SMT", _subModel.GetSubModelType()));
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);

    p = grid->Append(new wxStringProperty("SubModel Layout", "SML", _subModel.GetSubModelLayout()));
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);

    p = grid->Append(new wxStringProperty("SubModel Buffer Style", "SMBS", _subModel.GetSubModelBufferStyle()));
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);

    p = grid->Append(new wxStringProperty("SubModel", "SMN", _subModel.GetSubModelLines()));
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);

    auto modelGroups = _subModel.GetParent()->GetModelManager().GetGroupsContainingModel(&_subModel);
    if (modelGroups.size() > 0) {
        std::string mgs;
        std::string mgscr;
        for (const auto& it : modelGroups) {
            if (mgs != "") {
                mgs += ", ";
                mgscr += "\n";
            }
            mgs += it;
            mgscr += it;
        }
        p = grid->Append(new wxStringProperty("In Model Groups", "MGS", mgs));
        p->SetHelpString(mgscr);
        p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
        p->ChangeFlag(wxPGFlags::ReadOnly, true);
    }
    auto smaliases = _subModel.GetParent()->GetSubModel(_subModel.GetName())->GetAliases();
    if (smaliases.size() > 0) {
        std::string sma;
        std::string smacr;
        for (const auto& it : smaliases) {
            if (sma != "") {
                sma += ", ";
                smacr += "\n";
            }
            sma += it;
            smacr += it;
        }
        p = grid->Append(new wxStringProperty("SubModel Aliases", "SMA", sma));
        p->SetHelpString(smacr);
        p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
        p->ChangeFlag(wxPGFlags::ReadOnly, true);
    }
}

int SubModelPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    // SubModel properties are all read-only, so nothing to handle
    // Fall through to screen location handler
    int i = ScreenLocationPropertyHelper::OnPropertyGridChange(_subModel.GetModelScreenLocation(), grid, event);
    return i;
}

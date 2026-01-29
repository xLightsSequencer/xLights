/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "IciclesModel.h"
#include "../OutputModelManager.h"
#include "../xLightsVersion.h"
#include "../xLightsMain.h"

IciclesModel::IciclesModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    screenLocation.SetMHeight(-0.5);
    screenLocation.SetSupportsShear(true);
}

IciclesModel::~IciclesModel()
{
}

void IciclesModel::InitModel()
{
    int numStrings = parm1;
    int lightsPerString = parm2;

    SetNodeCount(numStrings, lightsPerString, rgbOrder);
    ParseDropSizes();

    int width = -1;
    size_t curNode = 0;
    size_t curCoord = 0;
    for (size_t x = 0; x < numStrings; x++) {
        int lights = lightsPerString;
        size_t y = 0;
        size_t curDrop = 0;
        size_t nodesInDrop = _dropSizes[curDrop];
        width++;
        while (lights > 0) {
            if (curCoord >= Nodes[curNode]->Coords.size()) {
                curNode++;
                curCoord = 0;
            }
            while (y >= _dropSizes[curDrop]) {
                width++;
                y = 0;
                curDrop++;
                if (curDrop >= _dropSizes.size()) {
                    curDrop = 0;
                }
                nodesInDrop = _dropSizes[curDrop];
            }
            Nodes[curNode]->ActChan = stringStartChan[0] + curNode * GetNodeChannelCount(StringType);
            Nodes[curNode]->StringNum = x;
            Nodes[curNode]->Coords[curCoord].bufX = width;
            if (_alternateNodes) {
                if (y + 1 <= (nodesInDrop + 1) / 2) {
                    Nodes[curNode]->Coords[curCoord].bufY = _maxH - 1 - (2 * y);
                    Nodes[curNode]->Coords[curCoord].screenY = (2 * y);
                }
                else {
                    Nodes[curNode]->Coords[curCoord].bufY = _maxH - 1 - ((nodesInDrop - (y + 1)) * 2 + 1);
                    Nodes[curNode]->Coords[curCoord].screenY = ((nodesInDrop - (y + 1)) * 2 + 1);
                }
            }
            else {
                Nodes[curNode]->Coords[curCoord].bufY = _maxH - y - 1;
                Nodes[curNode]->Coords[curCoord].screenY = y;
            }
            Nodes[curNode]->Coords[curCoord].screenX = width;
            lights--;
            y++;
            curCoord++;
        }
    }
    if (!IsLtoR) {
        for (size_t n = 0; n < Nodes.size(); n++) {
            for (auto& cd : Nodes[n]->Coords) {
                cd.bufX = width - cd.bufX;
                cd.screenX = width - cd.screenX;
            }
        }
    }
    SetBufferSize(_maxH, width + 1);

    //single icicle move to the center
    if (width == 0) {
        for (auto& n : Nodes) {
            for (auto& c : n->Coords) {
                c.screenX = 0.5;
            }
        }
        width++;
    }
    screenLocation.SetRenderSize(width, _maxH);
}

static const char* LEFT_RIGHT_VALUES[] = { "Green Square", "Blue Square" };
static wxPGChoices LEFT_RIGHT(wxArrayString(2, LEFT_RIGHT_VALUES));

void IciclesModel::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{

    wxPGProperty *p = grid->Append(new wxUIntProperty("# Strings", "IciclesStrings", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("Lights/String", "IciclesLights", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 2000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "IciclesLights", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 2000);
        p->SetEditor("SpinCtrl");
        p->SetHelpString("This is typically the total number of pixels per #String.");
    }

    p = grid->Append(new wxBoolProperty("Alternate Nodes", "AlternateNodes", _alternateNodes));
    p->SetEditor("CheckBox");

    grid->Append(new wxDropPatternProperty("Drop Pattern", "IciclesDrops", _dropPatternString));

    grid->Append(new wxEnumProperty("Starting Location", "IciclesStart", LEFT_RIGHT, IsLtoR ? 0 : 1));
}

int IciclesModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("IciclesStrings" == event.GetPropertyName()) {
        parm1 = static_cast<int>(event.GetPropertyValue().GetLong());
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        return 0;
    } else if ("IciclesLights" == event.GetPropertyName()) {
        parm2 = static_cast<int>(event.GetPropertyValue().GetLong());
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        return 0;
    } else if ("IciclesDrops" == event.GetPropertyName()) {
        SetDropPattern(event.GetPropertyValue().GetString());
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::IciclesDrops");
        return 0;
    } else if ("IciclesStart" == event.GetPropertyName()) {
        _dir = event.GetValue().GetLong() == 0 ? "L" : "R";
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::IciclesStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::IciclesStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::IciclesStart");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::IciclesStart");
        return 0;
    } else if ("AlternateNodes" == event.GetPropertyName()) {
        _alternateNodes = event.GetPropertyValue().GetBool();
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::AlternateNodes");
        return 0;
    }
    return Model::OnPropertyGridChange(grid, event);
}

void IciclesModel::SetDropPattern(const std::string & pattern)
{
    _dropPatternString = pattern;
    ParseDropSizes();
}

void IciclesModel::ParseDropSizes()
{
    _dropSizes.clear();
    wxArrayString pat = wxSplit(_dropPatternString, ',');
    _maxH = 0;
    for (int x = 0; x < pat.size(); x++) {
        int d = wxAtoi(pat[x]);
        if (d >= 0) { // we dont handle drops of less than zero
            _dropSizes.push_back(wxAtoi(pat[x]));
            _maxH = std::max(_maxH, (size_t)d);
        }
    }
    if (_dropSizes.size() == 0) {
        _dropSizes.push_back(5);
    }
}

std::string IciclesModel::GetDimension() const
{
    // the height does not make sense for icicles
    return static_cast<TwoPointScreenLocation>(screenLocation).GetDimension(1.0);
}

void IciclesModel::AddDimensionProperties(wxPropertyGridInterface* grid)
{
    // the height does not make sense for icicles
    static_cast<TwoPointScreenLocation>(screenLocation).AddDimensionProperties(grid, 1.0);
}

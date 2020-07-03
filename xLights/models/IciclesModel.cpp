/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "IciclesModel.h"
#include "../OutputModelManager.h"

IciclesModel::IciclesModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    screenLocation.SetHeight(-0.5);
    screenLocation.SetSupportsShear(true);
    SetFromXml(node, zeroBased);
}

IciclesModel::~IciclesModel()
{
}

void IciclesModel::InitModel() {
    wxString dropPattern = GetModelXml()->GetAttribute("DropPattern", "3,4,5,4");
    _alternateNodes = (ModelXml->GetAttribute("AlternateNodes", "false") == "true");
    wxArrayString pat = wxSplit(dropPattern, ',');
    int numStrings = parm1;
    int lightsPerString = parm2;

    SetNodeCount(numStrings, lightsPerString, rgbOrder);

    std::vector<unsigned int> dropSizes;
    unsigned int maxH = 0;
    for (int x = 0; x < pat.size(); x++) {
        dropSizes.push_back(wxAtoi(pat[x]));
        maxH = std::max(maxH, dropSizes[x]);
    }
    if (dropSizes.size() == 0) {
        dropSizes.push_back(5);
    }

    int width = -1;
    int curNode = 0;
    int curCoord = 0;
    for (int x = 0; x < numStrings; x++) {
        int lights = lightsPerString;
        int y = 0;
        int curDrop = 0;
        int nodesInDrop = dropSizes[curDrop];
        width++;
        while (lights) {
            if (curCoord >= Nodes[curNode]->Coords.size()) {
                curNode++;
                curCoord = 0;
            }
            while (y >= dropSizes[curDrop]) {
                width++;
                y = 0;
                curDrop++;
                if (curDrop >= dropSizes.size()) {
                    curDrop = 0;
                }
                nodesInDrop = dropSizes[curDrop];
            }
            Nodes[curNode]->ActChan = stringStartChan[0] + curNode*GetNodeChannelCount(StringType);
            Nodes[curNode]->StringNum=x;
            Nodes[curNode]->Coords[curCoord].bufX = width;
            if (_alternateNodes) {
                if (y + 1 <= (nodesInDrop + 1) / 2)
                {
                    Nodes[curNode]->Coords[curCoord].bufY = maxH - 1 - (2 * y);
                    Nodes[curNode]->Coords[curCoord].screenY = (2 * y);
                }
                else
                {
                    Nodes[curNode]->Coords[curCoord].bufY = maxH - 1 - ((nodesInDrop - (y + 1)) * 2 + 1);
                    Nodes[curNode]->Coords[curCoord].screenY = ((nodesInDrop - (y + 1)) * 2 + 1);
                }
            }
            else {
                Nodes[curNode]->Coords[curCoord].bufY = maxH - y - 1;
                Nodes[curNode]->Coords[curCoord].screenY = y;
            }
            Nodes[curNode]->Coords[curCoord].screenX = width;
            lights--;
            y++;
            curCoord++;
        }
    }
    if (!IsLtoR) {
        for(size_t n=0; n < Nodes.size(); n++) {
            for (auto cd = Nodes[n]->Coords.begin(); cd != Nodes[n]->Coords.end(); cd++) {
                cd->bufX = width - cd->bufX;
                cd->screenX = width - cd->screenX;
            }
        }
    }
    SetBufferSize(maxH, width+1);
    screenLocation.SetRenderSize(width, maxH);
}

static const char* LEFT_RIGHT_VALUES[] = { "Green Square", "Blue Square" };
static wxPGChoices LEFT_RIGHT(wxArrayString(2, LEFT_RIGHT_VALUES));

void IciclesModel::AddTypeProperties(wxPropertyGridInterface *grid) {

    wxPGProperty *p = grid->Append(new wxUIntProperty("# Strings", "IciclesStrings", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("Lights/String", "IciclesLights", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 1000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "IciclesLights", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 1000);
        p->SetEditor("SpinCtrl");
    }

    p = grid->Append(new wxBoolProperty("Alternate Nodes", "AlternateNodes", _alternateNodes));
    p->SetEditor("CheckBox");

    grid->Append(new wxStringProperty("Drop Pattern", "IciclesDrops", GetModelXml()->GetAttribute("DropPattern", "3,4,5,4")));

    grid->Append(new wxEnumProperty("Starting Location", "IciclesStart", LEFT_RIGHT, IsLtoR ? 0 : 1));
}

int IciclesModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("IciclesStrings" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        return 0;
    } else if ("IciclesLights" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        return 0;
    } else if ("IciclesDrops" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DropPattern");
        ModelXml->AddAttribute("DropPattern", event.GetPropertyValue().GetString());
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::IciclesDrops");
        return 0;
    } else if ("IciclesStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 ? "L" : "R");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::IciclesStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::IciclesStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::IciclesStart");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::IciclesStart");
        return 0;
    } else if ("AlternateNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("AlternateNodes");
        ModelXml->AddAttribute("AlternateNodes", event.GetPropertyValue().GetBool() ? "true" : "false");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::AlternateNodes");
        return 0;
    }
    return Model::OnPropertyGridChange(grid, event);
}
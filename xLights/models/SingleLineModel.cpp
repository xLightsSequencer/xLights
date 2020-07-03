/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SingleLineModel.h"
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include "ModelScreenLocation.h"

std::vector<std::string> SingleLineModel::LINE_BUFFER_STYLES;


SingleLineModel::SingleLineModel(const ModelManager &manager) : ModelWithScreenLocation(manager) {
    parm1 = parm2 = parm3 = 0;
}
SingleLineModel::SingleLineModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    SetFromXml(node, zeroBased);
}
SingleLineModel::SingleLineModel(int lights, const Model &pbc, int strand, int node) : ModelWithScreenLocation(pbc.GetModelManager())
{
    Reset(lights, pbc, strand, node);
}
void SingleLineModel::Reset(int lights, const Model &pbc, int strand, int node, bool forceDirection)
{
    Nodes.clear();
    parm1 = lights;
    parm2 = 1;
    parm3 = 1;

    StringType = pbc.GetStringType();
    rgbOrder = pbc.rgbOrder;
    SingleNode = pbc.SingleNode;
    SingleChannel = pbc.SingleChannel;
    IsLtoR = pbc.GetIsLtoR();
    customColor = pbc.customColor;

    bool flip = false;
    if (forceDirection) {
        int sn = pbc.GetNodeStringNumber(pbc.MapToNodeIndex(strand, 0));
        for (int s = strand - 1; s >= 0; s--) {
            if (pbc.GetNodeStringNumber(pbc.MapToNodeIndex(s, 0)) == sn) {
                flip = !flip;
            } else {
                break;
            }
        }
    }
    stringStartChan.resize(lights);
    if (node == -1) {
        for (int x = 0; x < lights; x++) {
            stringStartChan[x] = pbc.NodeStartChannel(pbc.MapToNodeIndex(strand, x));
        }
    } else {
        stringStartChan[0] = pbc.NodeStartChannel(pbc.MapToNodeIndex(strand, node));
    }
    InitModel();
    for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
        (*it)->model = &pbc;
        xlColor c = (*it)->model->GetNodeMaskColor(strand);
        (*it)->SetMaskColor(c);
    }
    if (flip) {
        int l = 0;
        int r = Nodes.size() - 1;
        while (l < r) {
            Nodes[l].swap(Nodes[r]);
            l++;
            r--;
        }
    }
}

SingleLineModel::~SingleLineModel()
{
    //dtor
}

const std::vector<std::string> &SingleLineModel::GetBufferStyles() const {
    struct Initializer {
        Initializer() {
            LINE_BUFFER_STYLES = Model::DEFAULT_BUFFER_STYLES;
            auto it = std::find(LINE_BUFFER_STYLES.begin(), LINE_BUFFER_STYLES.end(), "Single Line");
            if (it != LINE_BUFFER_STYLES.end()) {
                LINE_BUFFER_STYLES.erase(it);
            }
        }
    };
    
    static Initializer ListInitializationGuard;
    return LINE_BUFFER_STYLES;
}


void SingleLineModel::InitModel() {
    InitLine();

    if ( Nodes.size() > 0 && (BufferWi > 1 || Nodes.front()->Coords.size() > 1))
    {
        int lightcount = BufferWi * Nodes.front()->Coords.size();
        float bulbOffset = BufferWi / (lightcount - 1.0);
        float currentX = 0;
        for (auto& n : Nodes)
        {
            for (auto& c : n->Coords)
            {
                c.screenY = 0;
                c.screenX = currentX;
                currentX += bulbOffset;
            }
        }
    }
    else if (Nodes.size() > 0)
    {
        // 1 node 1 light
        Nodes.front()->Coords.front().screenY = 0.0;
        Nodes.front()->Coords.front().screenX = 0.5;
    }
    screenLocation.SetRenderSize(BufferWi, 1);
}


// initialize buffer coordinates
// parm1=Number of Strings/Arches/Canes
// parm2=Pixels Per String/Arch/Cane
void SingleLineModel::InitLine() {
    int numLights = parm1 * parm2;
    SetNodeCount(parm1,parm2,rgbOrder);
    SetBufferSize(1,SingleNode?parm1:numLights);
    int LastStringNum=-1;
    int chan = 0;
    int ChanIncr = GetNodeChannelCount(StringType);
    size_t NodeCount=GetNodeCount();
    if (!IsLtoR) {
        ChanIncr = -ChanIncr;
    }

    int idx = 0;
    for(size_t n=0; n<NodeCount; n++) {
        if (Nodes[n]->StringNum != LastStringNum) {
            LastStringNum=Nodes[n]->StringNum;
            chan=stringStartChan[LastStringNum];
            if (!IsLtoR) {
                chan += NodesPerString(LastStringNum) * GetNodeChannelCount(StringType);
                chan += ChanIncr;
            }
        }
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        Nodes[n]->Coords.resize(SingleNode?parm2:parm3);
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            Nodes[n]->Coords[c].bufX=idx;
            Nodes[n]->Coords[c].bufY=0;
        }
        idx++;
    }
}

static const char* LEFT_RIGHT_VALUES[] = { "Green Square", "Blue Square" };
static wxPGChoices LEFT_RIGHT(wxArrayString(2, LEFT_RIGHT_VALUES));

void SingleLineModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    wxPGProperty *p = grid->Append(new wxUIntProperty("# Strings", "SingleLineCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("Lights/String", "SingleLineNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "SingleLineNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxUIntProperty("Lights/Node", "SingleLineLights", parm3));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 300);
        p->SetEditor("SpinCtrl");
    }

    grid->Append(new wxEnumProperty("Starting Location", "SingleLineStart", LEFT_RIGHT, IsLtoR ? 0 : 1));
}

int SingleLineModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("SingleLineCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SingleLineModel::OnPropertyGridChange::SingleLineCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SingleLineModel::OnPropertyGridChange::SingleLineCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SingleLineModel::OnPropertyGridChange::SingleLineCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SingleLineModel::OnPropertyGridChange::SingleLineCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "SingleLineModel::OnPropertyGridChange::SingleLineCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SingleLineModel::OnPropertyGridChange::SingleLineCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "SingleLineModel::OnPropertyGridChange::SingleLineCount");
        return 0;
    } else if ("SingleLineNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SingleLineModel::OnPropertyGridChange::SingleLineNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SingleLineModel::OnPropertyGridChange::SingleLineNodes");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SingleLineModel::OnPropertyGridChange::SingleLineNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "SingleLineModel::OnPropertyGridChange::SingleLineNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SingleLineModel::OnPropertyGridChange::SingleLineNodes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SingleLineModel::OnPropertyGridChange::SingleLineNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "SingleLineModel::OnPropertyGridChange::SingleLineNodes");
        return 0;
    } else if ("SingleLineLights" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SingleLineModel::OnPropertyGridChange::SingleLineLights");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SingleLineModel::OnPropertyGridChange::SingleLineLights");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SingleLineModel::OnPropertyGridChange::SingleLineLights");
        return 0;
    } else if ("SingleLineStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 ? "L" : "R");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SingleLineModel::OnPropertyGridChange::SingleLineStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SingleLineModel::OnPropertyGridChange::SingleLineStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SingleLineModel::OnPropertyGridChange::SingleLineStart");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SingleLineModel::OnPropertyGridChange::SingleLineStart");
        return 0;
    }

    return Model::OnPropertyGridChange(grid, event);
}


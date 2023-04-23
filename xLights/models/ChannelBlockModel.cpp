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

#include "ChannelBlockModel.h"
#include "ModelScreenLocation.h"
#include "../OutputModelManager.h"

std::vector<std::string> ChannelBlockModel::LINE_BUFFER_STYLES;

ChannelBlockModel::ChannelBlockModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager) 
{
    SetFromXml(node, zeroBased);
}

ChannelBlockModel::~ChannelBlockModel()
{
}

const std::vector<std::string> &ChannelBlockModel::GetBufferStyles() const {
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

void ChannelBlockModel::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    wxPGProperty *p = grid->Append(new wxUIntProperty("# Channels", "ChannelBlockCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 64);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("Indiv Colors", "ChannelProperties", true));
    //p->SetAttribute("UseCheckbox", true);
    p->Enable(false);

    for (int x = 0; x < parm1; ++x) {
        wxString nm = ChanColorAttrName(x);
        std::string val = ModelXml->GetAttribute("ChannelProperties." + nm).ToStdString();
        if (val == "") {
            val = "white";
            ModelXml->DeleteAttribute("ChannelProperties." + nm);
            ModelXml->AddAttribute("ChannelProperties." + nm, val);
        }
        grid->AppendIn(p, new wxColourProperty(nm, nm, wxColor(val)));
    }
}

int ChannelBlockModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("ChannelBlockCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        //AdjustChannelProperties(grid, event.GetPropertyValue().GetLong());
        //AdjustStringProperties(grid, event.GetPropertyValue().GetLong());
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ChannelBlockModel::OnPropertyGridChange::ChannelBlockCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ChannelBlockModel::OnPropertyGridChange::ChannelBlockCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ChannelBlockModel::OnPropertyGridChange::ChannelBlockCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ChannelBlockModel::OnPropertyGridChange::ChannelBlockCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ChannelBlockModel::OnPropertyGridChange::ChannelBlockCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ChannelBlockModel::OnPropertyGridChange::ChannelBlockCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ChannelBlockModel::OnPropertyGridChange::ChannelBlockCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ChannelBlockModel::OnPropertyGridChange::ChannelBlockCount");
        return 0;
    } else if (event.GetPropertyName().StartsWith("ChannelProperties.")) {
        wxColor c;
        c << event.GetProperty()->GetValue();
        xlColor xc = c;
        ModelXml->DeleteAttribute(event.GetPropertyName());
        ModelXml->AddAttribute(event.GetPropertyName(), xc);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ChannelBlockModel::OnPropertyGridChange::ChannelProperties");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ChannelBlockModel::OnPropertyGridChange::ChannelProperties");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ChannelBlockModel::OnPropertyGridChange::ChannelProperties");
        return 0;
    }

    return Model::OnPropertyGridChange(grid, event);
}

void ChannelBlockModel::AdjustChannelProperties(wxPropertyGridInterface *grid, int newNum) {
    wxPropertyGrid *pg = (wxPropertyGrid*)grid;
    wxPGProperty *p = grid->GetPropertyByName("ChannelProperties");
    if (p != nullptr) {
        pg->Freeze();
        int count = p->GetChildCount();
        while (count > newNum) {
            count--;
            wxString nm = ChanColorAttrName(count);
            wxPGProperty *sp = grid->GetPropertyByName(wxS("ChannelProperties." + nm));
            if (sp != nullptr) {
                grid->DeleteProperty(sp);
            }
            ModelXml->DeleteAttribute("ChannelProperties." + nm);
        }

        while (count < newNum) {
            wxString nm = ChanColorAttrName(count);
            std::string val = ModelXml->GetAttribute("ChannelProperties." + nm).ToStdString();
            if (val == "") {
                ModelXml->DeleteAttribute("ChannelProperties." + nm);
                ModelXml->AddAttribute("ChannelProperties." + nm, "white");
            }
            grid->AppendIn(p, new wxColourProperty(nm, nm, wxColor(val)));
            count++;
        }

        pg->Thaw();
        pg->RefreshGrid();
    }
}

void ChannelBlockModel::GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi, int stagger) const {
    BufferHi = 1;
    BufferWi = GetNodeCount();
}

void ChannelBlockModel::InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                        std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi, int stagger, bool deep) const {
    BufferHi = 1;
    BufferWi = GetNodeCount();
        
    int NumChannels=parm1;
    int cur = 0;
    for (int y=0; y < NumChannels; ++y) {
		int idx = y;
		newNodes.push_back(NodeBaseClassPtr(Nodes[idx]->clone()));
		for(size_t c=0; c < newNodes[cur]->Coords.size(); ++c) {
			newNodes[cur]->Coords[c].bufX=cur;
			newNodes[cur]->Coords[c].bufY=0;
		}
		cur++;
    }
}

void ChannelBlockModel::DisableUnusedProperties(wxPropertyGridInterface *grid)
{
    // disable string type properties.  Only Single Color White allowed.
    wxPGProperty *p = grid->GetPropertyByName("ModelStringType");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelStringColor");
    if (p != nullptr) {
        p->Enable(false);
    }

    // Dont remove faces as they could be used by someone who is mapping in dumb channels

    // Don't remove ModelStates ... these can be used for DMX devices that use a value range to set a colour or behaviour
}

void ChannelBlockModel::InitModel() {
    StringType = "Single Color Custom";
    customColor = xlWHITE;
    SingleNode = true;

    InitChannelBlock();

    for (auto node = Nodes.begin(); node != Nodes.end(); ++node) {
        int count = 0;
        int num = node->get()->Coords.size();
        float offset = 0.0;
        if (num == 1) {
            offset = 0.5;
        }
        else {
            offset = (float)1 / (float)num / 2.0;
        }
        for (auto coord = node->get()->Coords.begin(); coord != node->get()->Coords.end(); ++coord) {
            coord->screenY = 0;
            if (num > 1) {
                coord->screenX = (float)coord->bufX + (float)count / (float)num + offset;
                count++;
            }
            else {
                coord->screenX = coord->bufX + offset;
            }
        }
    }
    screenLocation.SetRenderSize(BufferWi, 1);
}

void ChannelBlockModel::InitChannelBlock() {
    SetNodeCount(parm1, 1, rgbOrder);
    SetBufferSize(1, parm1);
    int LastStringNum = -1;
    int chan = 0;
    int ChanIncr = 1;
    size_t NodeCount = GetNodeCount();

    int idx = 0;
    for (size_t n = 0; n<NodeCount; ++n) {
        if (Nodes[n]->StringNum != LastStringNum) {
            LastStringNum = Nodes[n]->StringNum;
            chan = stringStartChan[LastStringNum];
        }
        Nodes[n]->ActChan = chan;
        chan += ChanIncr;
        Nodes[n]->Coords.resize(1);
        size_t CoordCount = GetCoordCount(n);
        for (size_t c = 0; c < CoordCount; ++c) {
            Nodes[n]->Coords[c].bufX = idx;
            Nodes[n]->Coords[c].bufY = 0;
        }
        idx++;
        wxString nm = ChanColorAttrName(n);
        std::string val = ModelXml->GetAttribute("ChannelProperties." + nm).ToStdString();
        xlColor c = xlColor(val);
        Nodes[n]->SetMaskColor(c);
        NodeClassCustom* ncc = dynamic_cast<NodeClassCustom*>(Nodes[n].get());
        ncc->SetCustomColor(c);
    }
}

int ChannelBlockModel::MapToNodeIndex(int strand, int node) const {
    return strand;
}
int ChannelBlockModel::GetNumStrands() const {
     return parm1;
}
int ChannelBlockModel::CalcCannelsPerString() {
	return GetNodeChannelCount(StringType);
}

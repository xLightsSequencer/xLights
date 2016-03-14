#include "SingleLineModel.h"
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

std::vector<std::string> SingleLineModel::LINE_BUFFER_STYLES;


SingleLineModel::SingleLineModel() {
    parm1 = parm2 = parm3 = 0;
}
SingleLineModel::SingleLineModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased)
{
    SetFromXml(node, netInfo, zeroBased);
}
SingleLineModel::SingleLineModel(int lights, const Model &pbc, int strand, int node)
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
    for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
        (*it)->model = &pbc;
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
    if (SingleNode || parm3 <= 1) {
        CopyBufCoord2ScreenCoord();
    } else {
        float xoffset=BufferWi/2;
        for (auto node = Nodes.begin(); node != Nodes.end(); node++) {
            float x = node->get()->Coords[0].bufX;
            node->get()->Coords.resize(parm3);
            for(size_t c=0; c < parm3; c++) {
                node->get()->Coords[c].screenY = 0;
                node->get()->Coords[c].screenX = x - 0.5 + ((float)c / (float)parm3) - xoffset;
            }
        }
        SetRenderSize(BufferHt,BufferWi);
    }
}


// initialize buffer coordinates
// parm1=Number of Strings/Arches
// parm2=Pixels Per String/Arch
void SingleLineModel::InitLine() {
    int numLights = parm1 * parm2;
    SetNodeCount(parm1,parm2,rgbOrder);
    SetBufferSize(1,numLights);
    int LastStringNum=-1;
    int chan = 0,idx;
    int ChanIncr=SingleChannel ?  1 : 3;
    size_t NodeCount=GetNodeCount();
    
    idx = 0;
    for(size_t n=0; n<NodeCount; n++) {
        if (Nodes[n]->StringNum != LastStringNum) {
            LastStringNum=Nodes[n]->StringNum;
            chan=stringStartChan[LastStringNum];
        }
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            Nodes[n]->Coords[c].bufX=IsLtoR ? idx : numLights-idx-1;
            Nodes[n]->Coords[c].bufY=0;
            idx++;
        }
    }
}

static wxPGChoices LEFT_RIGHT;

void SingleLineModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    if (LEFT_RIGHT.GetCount() == 0) {
        LEFT_RIGHT.Add("Left");
        LEFT_RIGHT.Add("Right");
    }
    wxPGProperty *p = grid->Append(new wxUIntProperty("# Strings", "SingleLineCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("Lights/String", "SingleLineNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 250);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "SingleLineNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 250);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxUIntProperty("Lights/Node", "SingleLineLights", parm3));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 250);
        p->SetEditor("SpinCtrl");
    }
    
    p = grid->Append(new wxEnumProperty("Starting Location", "SingleLineStart", LEFT_RIGHT, IsLtoR ? 0 : 1));
}
int SingleLineModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("SingleLineCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, *ModelNetInfo, zeroBased);
        return 3;
    } else if ("SingleLineNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, *ModelNetInfo, zeroBased);
        return 3;
    } else if ("SingleLineLights" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, *ModelNetInfo, zeroBased);
        return 3;
    } else if ("SingleLineStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 ? "L" : "R");
        SetFromXml(ModelXml, *ModelNetInfo, zeroBased);
        return 3;
    }
    
    return Model::OnPropertyGridChange(grid, event);
}


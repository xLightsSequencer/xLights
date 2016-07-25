#include "PolyLineModel.h"
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>


#include "ModelScreenLocation.h"

std::vector<std::string> PolyLineModel::LINE_BUFFER_STYLES;


PolyLineModel::PolyLineModel(const ModelManager &manager) : ModelWithScreenLocation(manager) {
    parm1 = parm2 = parm3 = 0;
}
PolyLineModel::PolyLineModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    SetFromXml(node, zeroBased);
}
PolyLineModel::PolyLineModel(int lights, const Model &pbc, int strand, int node) : ModelWithScreenLocation(pbc.GetModelManager())
{
    Reset(lights, pbc, strand, node);
}
void PolyLineModel::Reset(int lights, const Model &pbc, int strand, int node, bool forceDirection)
{
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

PolyLineModel::~PolyLineModel()
{
    //dtor
}

const std::vector<std::string> &PolyLineModel::GetBufferStyles() const {
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


void PolyLineModel::InitModel() {
    Nodes.clear();
    InitLine();

    for (auto node = Nodes.begin(); node != Nodes.end(); node++) {
        int count = 0;
        int num = node->get()->Coords.size();
        float offset = 0.0;
        if (num == 1) {
            offset = 0.5;
        } else {
            offset = (float)1 / (float)num / 2.0;
        }
        for (auto coord = node->get()->Coords.begin(); coord != node->get()->Coords.end(); coord++) {
            coord->screenY = coord->bufY;
            if (num > 1) {
                coord->screenX = coord->bufX + (float)count / (float)num + offset ;
                count++;
            } else {
                coord->screenX = coord->bufX + offset ;
            }
        }
    }
    screenLocation.SetRenderSize(BufferWi, BufferHt);
}


// initialize buffer coordinates
// parm1=Number of Strings/Arches/Canes
// parm2=Pixels Per String/Arch/Cane
void PolyLineModel::InitLine() {
    int numLights = parm1 * parm2;
	num_segments = wxAtoi(ModelXml->GetAttribute("NumPoints", "2")) - 1;
    SetNodeCount(parm1,parm2,rgbOrder);
    size_t NodeCount=GetNodeCount();
    int nodes_per_segment = NodeCount/num_segments;
    int remainder = NodeCount - nodes_per_segment*num_segments;
    int nodes_this_segment = nodes_per_segment + ((remainder-- > 0) ? 1 : 0);
    SetBufferSize(num_segments,(SingleNode?parm1:nodes_this_segment));
    int LastStringNum=-1;
    int chan = 0,idx;
    int ChanIncr=SingleChannel ?  1 : 3;

    idx = 0;
    for(size_t m=0; m<num_segments; m++) {
        int seg_idx = 0;
        int end_node = idx + nodes_this_segment;
        for(size_t n=idx; n<end_node; n++) {
            if (Nodes[idx]->StringNum != LastStringNum) {
                LastStringNum=Nodes[idx]->StringNum;
                chan=stringStartChan[LastStringNum];
            }
            Nodes[idx]->ActChan=chan;
            chan+=ChanIncr;
            Nodes[idx]->Coords.resize(SingleNode?parm2:parm3);
            size_t CoordCount=GetCoordCount(idx);
            for(size_t c=0; c < CoordCount; c++) {
                Nodes[idx]->Coords[c].bufX=IsLtoR ? seg_idx : (SingleNode ? seg_idx : numLights-seg_idx-1);
                Nodes[idx]->Coords[c].bufY=m;
            }
            idx++;
            seg_idx++;
        }
        nodes_this_segment = nodes_per_segment + ((remainder-- > 0) ? 1 : 0);
    }
}

static wxPGChoices LEFT_RIGHT;

void PolyLineModel::AddTypeProperties(wxPropertyGridInterface *grid) {
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

    p = grid->Append(new wxEnumProperty("Starting Location", "SingleLineStart", LEFT_RIGHT, IsLtoR ? 0 : 1));
    p = grid->Append(new wxStringProperty("Layer Sizes", "PolyLayerSizes", ModelXml->GetAttribute("polyLineSizes")));
}
int PolyLineModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("SingleLineCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("SingleLineNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("SingleLineLights" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("SingleLineStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 ? "L" : "R");
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("PolyLayerSizes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("polyLineSizes");
        ModelXml->AddAttribute("polyLineSizes", event.GetValue().GetString());
        SetFromXml(ModelXml, zeroBased);
        return 3;
    }

    return Model::OnPropertyGridChange(grid, event);
}


/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SingleLineModel.h"

#include "ModelScreenLocation.h"
#include "../XmlSerializer/XmlNodeKeys.h"

std::vector<std::string> SingleLineModel::LINE_BUFFER_STYLES;

SingleLineModel::SingleLineModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::SingleLine;
    _numStrings = 0;
    _nodesPerString = 0;
    _lightsPerNode = 0;
}

/*SingleLineModel::SingleLineModel(int lights, const Model &pbc, int strand, int node) : ModelWithScreenLocation(pbc.GetModelManager())
{
    Reset(lights, pbc, strand, node);
}*/

bool SingleLineModel::IsNodeFirst(int n) const 
{
    return (GetIsLtoR() && n == 0) || (!GetIsLtoR() && n == Nodes.size() - 1);
}

void SingleLineModel::Reset(int lights, const Model &pbc, int strand, int node, bool forceDirection)
{
    // If this method is called, we're creating a SingleLineModel to represent raw data and will not have a proper screen location
    validLocation = false;
    parent = &pbc;
    Nodes.clear();
    _numStrings = lights;
    _nodesPerString = 1;
    _lightsPerNode = 1;

    StringType = pbc.GetStringType();
    rgbOrder = pbc.rgbOrder;
    SingleNode = pbc.SingleNode;
    SingleChannel = pbc.SingleChannel;
    IsLtoR = pbc.GetIsLtoR();
    SetCustomColor(pbc.GetCustomColor());

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
        name = pbc.GetFullName() + "-" + pbc.GetStrandName(strand, true);
        for (int x = 0; x < lights; x++) {
            stringStartChan[x] = pbc.NodeStartChannel(pbc.MapToNodeIndex(strand, x));
        }
    } else {
        int idx = pbc.MapToNodeIndex(strand, node);
        name = pbc.GetFullName() + "-" + pbc.GetNodeName(idx, true);
        stringStartChan[0] = pbc.NodeStartChannel(idx);
    }
    InitModel();
    for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
        (*it)->model = &pbc;
        MaskedNodeBaseClass *mn = dynamic_cast<MaskedNodeBaseClass*>(it->get());
        if (mn) {
            xlColor c = (*it)->model->GetNodeMaskColor(strand);
            mn->SetMaskColor(c);
        }
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


int SingleLineModel::NodesPerString() const
{
    if (SingleNode) {
        return 1;
    }
    int ts = GetSmartTs();
    if (ts <= 1) {
        return _nodesPerString;
    }
    return _nodesPerString * ts;
}

// initialize buffer coordinates
// _numStrings=Number of Strings/Arches/Canes
// _nodesPerString=Pixels Per String/Arch/Cane
void SingleLineModel::InitLine() {
    int numLights = _numStrings * _nodesPerString;
    SetNodeCount(_numStrings,_nodesPerString,rgbOrder);
    SetBufferSize(1,SingleNode?_numStrings:numLights);
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
        Nodes[n]->Coords.resize(SingleNode?_nodesPerString:_lightsPerNode);
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            Nodes[n]->Coords[c].bufX=idx;
            Nodes[n]->Coords[c].bufY=0;
        }
        idx++;
    }
}



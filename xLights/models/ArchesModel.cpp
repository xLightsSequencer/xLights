#include <wx/xml/xml.h>

#include "ArchesModel.h"


std::vector<std::string> ArchesModel::ARCHES_BUFFER_STYLES {
    "Default", "Per Preview", "Rotate CC 90",
    "Rotate CW 90", "Rotate 180", "Flip Vertical", "Flip Horizontal",
    "Single Line"
};


ArchesModel::ArchesModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased)
{
    SetFromXml(node, netInfo, zeroBased);
}

ArchesModel::~ArchesModel()
{
}

void ArchesModel::GetBufferSize(const std::string &type, int &BufferWi, int &BufferHi) const {
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = this->BufferWi * this->BufferHt;
    } else {
        Model::GetBufferSize(type, BufferWi, BufferHi);
    }
}
void ArchesModel::InitRenderBufferNodes(const std::string &type, std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi) const {
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = GetNodeCount();
        
        int NumArches=parm1;
        int SegmentsPerArch=parm2;
        int cur = 0;
        for (int y=0; y < NumArches; y++) {
            for(int x=0; x<SegmentsPerArch; x++) {
                int idx = y * SegmentsPerArch + x;
                newNodes.push_back(NodeBaseClassPtr(Nodes[idx]->clone()));
                for(size_t c=0; c < newNodes[cur]->Coords.size(); c++) {
                    newNodes[cur]->Coords[c].bufX=cur;
                    newNodes[cur]->Coords[c].bufY=0;
                }
                cur++;
            }
        }
    } else {
        Model::InitRenderBufferNodes(type, newNodes, BufferWi, BufferHi);
    }
}

void ArchesModel::InitModel() {
    int NumArches=parm1;
    int SegmentsPerArch=parm2;
    
    SetBufferSize(NumArches,SegmentsPerArch);
    if (SingleNode) {
        SetNodeCount(NumArches * SegmentsPerArch, parm3,rgbOrder);
    } else {
        SetNodeCount(NumArches, SegmentsPerArch, rgbOrder);
        if (parm3 > 1) {
            for (int x = 0; x < Nodes.size(); x++) {
                Nodes[x]->Coords.resize(parm3);
            }
        }
    }
    SetRenderSize(NumArches,SegmentsPerArch);
    
    for (int y=0; y < NumArches; y++) {
        for(int x=0; x<SegmentsPerArch; x++) {
            int idx = y * SegmentsPerArch + x;
            Nodes[idx]->ActChan = stringStartChan[y] + x*GetNodeChannelCount(StringType);
            Nodes[idx]->StringNum=y;
            for(size_t c=0; c < GetCoordCount(idx); c++) {
                Nodes[idx]->Coords[c].bufX=IsLtoR ? x : SegmentsPerArch-x-1;
                Nodes[idx]->Coords[c].bufY=isBotToTop ? y : NumArches-y-1;
            }
        }
    }
    SetArchCoord();
}
int ArchesModel::MapToNodeIndex(int strand, int node) const {
    return strand * parm2 + node;
}
int ArchesModel::GetNumStrands() const {
     return parm1;
}
int ArchesModel::CalcCannelsPerString() {
    SingleChannel = false;
    return GetNodeChannelCount(StringType) * parm2;
}

void ArchesModel::SetArchCoord() {
    double xoffset,x,y;
    int numlights=parm1*parm2*parm3;
    size_t NodeCount=GetNodeCount();
    SetRenderSize(parm2*parm3,numlights*2);
    double midpt=parm2*parm3;
    midpt -= 1.0;
    midpt /= 2.0;
    for(size_t n=0; n<NodeCount; n++) {
        xoffset=Nodes[n]->StringNum*parm2*parm3*2 - numlights;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            double angle=-M_PI/2.0 + M_PI * ((double)(Nodes[n]->Coords[c].bufX * parm3 + c))/midpt/2.0;
            x=xoffset + midpt*sin(angle)*2.0+parm2*parm3;
            y=(parm2*parm3)*cos(angle);
            Nodes[n]->Coords[c].screenX=x;
            Nodes[n]->Coords[c].screenY=y-(RenderHt/2);
        }
    }
}

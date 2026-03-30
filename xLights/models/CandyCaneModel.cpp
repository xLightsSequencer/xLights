/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "../OutputModelManager.h"
#include "CandyCaneModel.h"
#include "ModelScreenLocation.h"
#include "../XmlSerializer/XmlNodeKeys.h"

#include <log.h>

CandyCaneModel::CandyCaneModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::CandyCanes;
    screenLocation.SetModelHandleHeight(true);
    screenLocation.SetSupportsAngle(true);
    screenLocation.SetPreferredSelectionPlane(ModelScreenLocation::MSLPLANE::GROUND);
}

CandyCaneModel::~CandyCaneModel()
{
}


bool CandyCaneModel::IsNodeFirst(int n) const
{
    return (GetIsLtoR() && n == 0) || (!GetIsLtoR() && n == (int)Nodes.size() - _nodesPerCane);
}

// Canes are 3 high per width of each indivudual cane, then multiply by 2 because standard ThreePointLocation applies a / 2 for heights
std::string CandyCaneModel::GetDimension() const
{
    if (_numCanes != 0) {
        return GetModelScreenLocation().GetDimension(6.0 / _numCanes);
    }
    return GetModelScreenLocation().GetDimension(6.0);
}

void CandyCaneModel::GetBufferSize(const std::string &tp, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi, int stagger) const {
    std::string type = tp.starts_with("Per Model ") ? tp.substr(10) : tp;
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = this->BufferWi * this->BufferHt;
        AdjustForTransform(transform, BufferWi, BufferHi);
    } else {
        Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi, stagger);
    }
}

void CandyCaneModel::InitRenderBufferNodes(const std::string &tp, const std::string &camera,  const std::string &transform,
                                        std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi, int stagger, bool deep) const {
    std::string type = tp.starts_with("Per Model ") ? tp.substr(10) : tp;
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = GetNodeCount();

        int NumCanes=_numCanes;
        int SegmentsPerCane=_nodesPerCane;
        int cur = 0;
        for (int y=0; y < NumCanes; y++) {
            for(int x=0; x<SegmentsPerCane; x++) {
                int idx = y * SegmentsPerCane + x;
                newNodes.push_back(NodeBaseClassPtr(Nodes[idx]->clone()));
                for(size_t c=0; c < newNodes[cur]->Coords.size(); c++) {
                    newNodes[cur]->Coords[c].bufX=cur;
                    newNodes[cur]->Coords[c].bufY=0;
                }
                cur++;
            }
        }
        ApplyTransform(transform, newNodes, BufferWi, BufferHi);
    } else {
        Model::InitRenderBufferNodes(type, camera, transform, newNodes, BufferWi, BufferHi, stagger);
    }
}

void CandyCaneModel::InitModel() {
    int NumCanes = _numCanes;
    int SegmentsPerCane = _nodesPerCane;

    // When a SingleNode model is saved, _nodesPerCane is stored as 1 and _lightsPerNode holds lights per cane.
    // On reload, restore _nodesPerCane from _lightsPerNode so SetNodeCount gets the correct count.
    if (SingleNode && _nodesPerCane <= 1 && _lightsPerNode > 1) {
        SegmentsPerCane = _lightsPerNode;
        _nodesPerCane = _lightsPerNode;
    }

    SetNodeCount(NumCanes, SegmentsPerCane, rgbOrder);
    if (SingleNode) {
        SegmentsPerCane = 1;
        _lightsPerNode = _nodesPerCane;
        _nodesPerCane = 1;
    } else {
        if (_lightsPerNode > 1)
		{
            for (size_t x = 0; x < Nodes.size(); x++)
			{
                Nodes[x]->Coords.resize(_lightsPerNode);
            }
        }
    }
    SetBufferSize(SegmentsPerCane, NumCanes);

    if (!IsLtoR) {
        for (int y = 0; y < (NumCanes / 2); y++) {
            int i = stringStartChan[y];
            stringStartChan[y] = stringStartChan[NumCanes - 1 - y];
            stringStartChan[NumCanes - 1 - y] = i;
        }
    }

    for (int y=0; y < NumCanes; y++) {
        for(int x=0; x<SegmentsPerCane; x++) {
            int idx = y * SegmentsPerCane + x;
            Nodes[idx]->ActChan = stringStartChan[y] + x*GetNodeChannelCount(StringType);
            Nodes[idx]->StringNum=y;
            for(size_t c=0; c < GetCoordCount(idx); c++) {
                Nodes[idx]->Coords[c].bufX=y;
                if (HasAlternateNodes())
                {
                    if (x + 1 <= (SegmentsPerCane + 1) / 2)
                    {
                        Nodes[idx]->Coords[c].bufY = 2 * x;
                    }
                    else
                    {
                        Nodes[idx]->Coords[c].bufY = (SegmentsPerCane - (x + 1)) * 2 + 1;
                    }
                }
                else
                {
                    Nodes[idx]->Coords[c].bufY = x;
                }
            }
        }
    }
    SetCaneCoord();
}

int CandyCaneModel::MapToNodeIndex(int strand, int node) const {
    return strand * _nodesPerCane + node;
}

int CandyCaneModel::GetNumStrands() const {
     return _numCanes;
}

int CandyCaneModel::CalcChannelsPerString() {
    if (SingleNode) {
        return GetNodeChannelCount(StringType);
    }
    return GetNodeChannelCount(StringType) * _nodesPerCane;
}

int CandyCaneModel::NodesPerString() const {
    if (SingleNode) {
        return 1;
    }
    int ts = GetSmartTs();
    if (ts <= 1) {
        return _nodesPerCane;
    }
    return _nodesPerCane * ts;
}

static void rotate_point(float cx,float cy, float angle, float &x, float &y)
{
    float s = sin(angle);
    float c = cos(angle);

    // translate point back to origin:
    x -= cx;
    y -= cy;

    // rotate point
    float xnew = x * c - y * s;
    float ynew = x * s + y * c;

    // translate point back:
    x = xnew + cx;
    y = ynew + cy;
}

void CandyCaneModel::SetCaneCoord() {

    

    int NumCanes = _numCanes;
    size_t SegmentsPerCane = _nodesPerCane;
    int LightsPerNode = _lightsPerNode;

    int lightspercane = SegmentsPerCane * LightsPerNode;
    float angle = toRadians(screenLocation.GetAngle());

    double height;
    double width;


    double caneGap = 2.0;
    int upright = SegmentsPerCane * 6.0 / 9.0;
    upright *= _lightsPerNode;
    if (SingleNode) {
        upright = _lightsPerNode * 6.0 / 9.0;
    }
    double widthPerCane = double(lightspercane)*3.0/9.0;
    width = (double)NumCanes*widthPerCane + (NumCanes - 1) * caneGap;
    height = lightspercane - widthPerCane/2.0;

    if (_sticks) {
        height = lightspercane * _caneheight;
        for (int i = 0; i < NumCanes; i++){
            int y = 0;
            double x = i*(widthPerCane + caneGap) + widthPerCane / 2.0;
            for (size_t n = 0; n < SegmentsPerCane; n++) {
                size_t CoordCount = GetCoordCount(n);
                if (SingleNode)
                {
                    auto node = n + i * SegmentsPerCane;
                    for (size_t c = 0; c < CoordCount; c++) {
                        Nodes[node]->Coords[c].screenX = x;
                        Nodes[node]->Coords[c].screenY = _caneheight * (float)y * screenLocation.GetMHeight();
                        rotate_point(x, 0, angle,
                            Nodes[node]->Coords[c].screenX,
                            Nodes[node]->Coords[c].screenY);
                        y++;
                    }
                }
                else
                {
                    auto node = FindNodeAtXY(i, y / _lightsPerNode);
                    for (size_t c = 0; c < CoordCount; c++) {
                        if (node == -1)
                        {
                            spdlog::error("Candy Cane buffer x,y {}, {} not found.", i, y);
                        }
                        else
                        {
                            Nodes[node]->Coords[c].screenX = x;
                            Nodes[node]->Coords[c].screenY = _caneheight * (float)y * screenLocation.GetMHeight();
                            rotate_point(x, 0, angle,
                                Nodes[node]->Coords[c].screenX,
                                Nodes[node]->Coords[c].screenY);
                        }
                        y++;
                    }
                }
            }
        }
    } else {
        int arclights = lightspercane - upright;
        for (int i = 0; i < NumCanes; i++) {
            // draw the uprights
            double x = i*(widthPerCane + caneGap);
            if (_reverse) {
                x += widthPerCane;
            }
            double y = 0;
            int curLight = 0;
            int curNode = 0;
            double cx = x;
            if (_reverse) {
                cx -= widthPerCane/2 * screenLocation.GetMHeight();
            } else {
                cx += widthPerCane/2 * screenLocation.GetMHeight();
            }

            double ox = x;
            while (curLight < upright) {
                size_t CoordCount = GetCoordCount(curNode);
                if (SingleNode)
                {
                    CoordCount = upright;
                    int node = curNode + i * SegmentsPerCane;
                    for (size_t c = 0; c < CoordCount; c++) {
                        Nodes[node]->Coords[c].screenX = x;
                        Nodes[node]->Coords[c].screenY = _caneheight * (float)y * screenLocation.GetMHeight();
                        rotate_point(x, 0, angle,
                            Nodes[node]->Coords[c].screenX,
                            Nodes[node]->Coords[c].screenY);
                        y++;
                        curLight++;
                    }
                }
                else
                {
                    auto node = FindNodeAtXY(i, y / _lightsPerNode);
                    for (size_t c = 0; c < CoordCount; c++) {
                        if (node == -1)
                        {
                            spdlog::error("Candy Cane buffer x,y {}, {} not found.", i, y);
                        }
                        else
                        {
                            Nodes[node]->Coords[c].screenX = x;
                            Nodes[node]->Coords[c].screenY = _caneheight * (float)y * screenLocation.GetMHeight();
                            rotate_point(x, 0, angle,
                                Nodes[node]->Coords[c].screenX,
                                Nodes[node]->Coords[c].screenY);
                        }
                        y++;
                        curLight++;
                    }
                    curNode++;
                }
            }
            y--;
            x = cx;
            while (curLight < lightspercane) {
                size_t CoordCount = GetCoordCount(curNode);
                size_t c = 0;
                if (SingleNode)
                {
                    CoordCount = lightspercane;
                    c = curLight;
                    int node = curNode + i * SegmentsPerCane;
                    for (; c < CoordCount; c++)
                    {
                        // drawing left to right
                        double aangle = M_PI - M_PI * (curLight - upright + 1) / arclights;
                        double y2 = sin(aangle) * widthPerCane / 2 * screenLocation.GetMHeight();
                        double x2 = cos(aangle) * widthPerCane / 2 * screenLocation.GetMHeight();
                        if (_reverse) {
                            Nodes[node]->Coords[c].screenX = x - x2;
                        }
                        else {
                            Nodes[node]->Coords[c].screenX = x + x2;
                        }
                        Nodes[node]->Coords[c].screenY = _caneheight * (float)(y * screenLocation.GetMHeight() + y2);
                        rotate_point(ox, 0, angle,
                            Nodes[node]->Coords[c].screenX,
                            Nodes[node]->Coords[c].screenY);
                        curLight++;
                    }
                }
                else
                {
                    auto node = FindNodeAtXY(i, curLight / _lightsPerNode);
                    for (; c < CoordCount; c++)
                    {
                        // drawing left to right
                        double aangle = M_PI - M_PI * (curLight - upright + 1) / arclights;
                        double y2 = sin(aangle) * widthPerCane / 2 * screenLocation.GetMHeight();
                        double x2 = cos(aangle) * widthPerCane / 2 * screenLocation.GetMHeight();
                        if (node == -1)
                        {
                            spdlog::error("Candy Cane buffer x,y {}, {} not found.", i, curLight);
                        }
                        else
                        {
                            if (_reverse) {
                                Nodes[node]->Coords[c].screenX = x - x2;
                            }
                            else {
                                Nodes[node]->Coords[c].screenX = x + x2;
                            }
                            Nodes[node]->Coords[c].screenY = _caneheight * (float)(y * screenLocation.GetMHeight() + y2);
                            rotate_point(ox, 0, angle,
                                Nodes[node]->Coords[c].screenX,
                                Nodes[node]->Coords[c].screenY);
                        }
                        curLight++;
                    }
                }
                curNode++;
            }
        }
    }
    float min = 99999;
    float max = -9999;
    for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
        for (auto & it2 : (*it)->Coords) {
            min = std::min(min, it2.screenY);
            max = std::max(max, it2.screenY);
        }
    }
    screenLocation.SetRenderSize(width, height);
}

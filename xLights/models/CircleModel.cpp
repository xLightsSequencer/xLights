/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/


#include "CircleModel.h"
#include "ModelScreenLocation.h"
#include "../OutputModelManager.h"
#include "../XmlSerializer/XmlNodeKeys.h"

CircleModel::CircleModel(const ModelManager& manager) : ModelWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::Circle;
}

CircleModel::~CircleModel()
{
}

int CircleModel::GetStrandLength(int strand) const {
    return SingleNode ? 1 : GetLayerSize(GetLayerSizeCount() - strand - 1);
}

int CircleModel::MapToNodeIndex(int strand, int node) const {
    int idx = 0;
    for (int x = 0; x < strand; x++) {
        idx += GetStrandLength(x);
    }
    idx += node;
    return idx;
}

int CircleModel::GetNumStrands() const {
    return GetLayerSizeCount();
}

bool CircleModel::AllNodesAllocated() const
{
    int allocated = 0;
    for (auto it : GetLayerSizes()) {
        allocated += it;
    }

    return (allocated == GetNodeCount());
}

// _centerPercent controls inner circle size
// top left=top ccw, top right=top cw, bottom left=bottom cw, bottom right=bottom ccw
int CircleModel::NodesPerString() const
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

void CircleModel::InitModel()
{
    if (GetLayerSizeCount() == 0) {
        SetLayerSizeCount(1);
    }
    if (GetLayerSizeCount() == 1) {
        SetLayerSize(0, _numStrings * _nodesPerString);
    }

    InitCircle();
    SetCircleCoord();
    screenLocation.RenderDp = 10.0f;  // give the bounding box a little depth
}

int CircleModel::maxSize() {
    int maxLights = 0;
    for (const auto x : GetLayerSizes()) {
        if (x > maxLights) {
            maxLights = x;
        }
    }
    return maxLights;
}

void CircleModel::InitCircle()
{
    int maxLights = 0;
    int numLights = _numStrings * _nodesPerString;
    int cnt = 0;

    if (GetLayerSizeCount() == 0) {
        SetLayerSizeCount(1);
        SetLayerSize(0, numLights);
    }

    for (int x = 0; x < GetLayerSizeCount(); x++) {
        if ((cnt + GetLayerSize(x)) > numLights) {
            if (cnt > numLights) {
                SetLayerSize(x, 0);
            }
            else {
                SetLayerSize(x, numLights - cnt);
            }
        }
        cnt += GetLayerSize(x);
        if (GetLayerSize(x) > maxLights) {
            maxLights = GetLayerSize(x);
        }
    }

    SetNodeCount(_numStrings, _nodesPerString, rgbOrder);
    SetBufferSize(GetLayerSizeCount(), maxLights);
    int LastStringNum = -1;
    int chan = 0;
    int ChanIncr = GetNodeChannelCount(StringType);
    size_t NodeCount = GetNodeCount();

    size_t node = 0;
    int nodesToMap = NodeCount;
    for (int circle = 0; circle < GetLayerSizeCount(); circle++) {
        int idx = 0;
        auto strandLen = GetStrandLength(circle);
        int loop_count = std::min(nodesToMap, strandLen);
        // if the number of nodes in this layer is exactly divisible into the maximum loops then we dont fudge ... but if it isnt then we want to fudge things slightly 
        // so that the largest layer goes to the end but any layer not divisible does not use the last x value
        double fudge = -1 * maxLights / strandLen + 1;
        for (size_t n = 0; n < loop_count; n++) {
            if (Nodes[node]->StringNum != LastStringNum) {
                LastStringNum = Nodes[node]->StringNum;
                chan = stringStartChan[LastStringNum];
            }
            Nodes[node]->ActChan = chan;
            chan += ChanIncr;
            double pct = (loop_count == 1) ? (double)n : (double)n / (double)(loop_count - 1);
            size_t CoordCount = GetCoordCount(node);
            for (size_t c = 0; c < CoordCount; c++) {
                if (loop_count == 1) {
                    Nodes[node]->Coords[c].bufX = idx;
                    Nodes[node]->Coords[c].bufY = _insideOut ? GetLayerSizeCount() - circle - 1 : circle;
                }
                else {
                    int x_pos = (GetStrandLength(circle) == maxLights) ? idx : std::floor(pct * ((double)maxLights - 1.0 + fudge));
                    Nodes[node]->Coords[c].bufX = x_pos;
                    Nodes[node]->Coords[c].bufY = _insideOut ? GetLayerSizeCount() - circle - 1 : circle;
                    idx++;
                }
            }
            node++;
        }
        nodesToMap -= loop_count;
    }
}

// Set screen coordinates for circles
void CircleModel::SetCircleCoord()
{
    size_t NodeCount = GetNodeCount();
    int maxLights = maxSize();
    screenLocation.SetRenderSize(maxLights, maxLights);
    int nodesToMap = NodeCount;
    int node = 0;
    double maxRadius = maxLights / 2.0;
    double minRadius = (double)_centerPercent / 100.0 * maxRadius;
    for (int c2 = 0; c2 < GetLayerSizeCount(); c2++) {
        int circle = c2;
        int loop_count = std::min(nodesToMap, GetStrandLength(circle));
        double radius = (GetLayerSizeCount() == 1) ? maxRadius : _insideOut ? (double)minRadius + (maxRadius - minRadius) * (1.0 - (double)(GetLayerSizeCount() - circle - 1) / (double)(GetLayerSizeCount() - 1)) : (double)minRadius + (maxRadius - minRadius) * (1.0 - (double)circle / (double)(GetLayerSizeCount() - 1));
        for (size_t n = 0; n < loop_count; n++) {
            size_t CoordCount = GetCoordCount(node);
            for (size_t c = 0; c < CoordCount; c++) {
                double angle = (isBotToTop ? -M_PI : 0) + M_PI * ((loop_count == 1) ? (double)c / (double)CoordCount : (double)n / (double)loop_count) * 2.0;
                if (!IsLtoR) {
                    angle *= -1;
                }
                double x = sin(angle) * radius;
                double y = cos(angle) * radius;
                Nodes[node]->Coords[c].screenX = x;
                Nodes[node]->Coords[c].screenY = y;
            }
            node++;
        }
        nodesToMap -= loop_count;
    }
}

void CircleModel::OnLayerSizesChange(bool countChanged)
{
    // if string count is 1 then adjust nodes per string to match sum of nodes
    if (_numStrings == 1 && GetLayerSizeCount() > 0) {
        _nodesPerString = (int)GetLayerSizesTotalNodes();
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "CircleModel::OnLayerSizesChange");
    }
}

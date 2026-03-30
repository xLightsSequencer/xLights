/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ArchesModel.h"
#include "ModelScreenLocation.h"
#include "xLightsVersion.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"
#include "../ModelPreview.h"
#include "../ModelPreview.h"
#include "../XmlSerializer/XmlNodeKeys.h"

ArchesModel::ArchesModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::Arches;
    screenLocation.SetModelHandleHeight(true);
    screenLocation.SetSupportsAngle(true);
    screenLocation.SetPreferredSelectionPlane(ModelScreenLocation::MSLPLANE::GROUND);
}

ArchesModel::~ArchesModel()
{
}



void ArchesModel::GetBufferSize(const std::string &tp, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi, int stagger) const {
    std::string type = tp.starts_with("Per Model ") ? tp.substr(10) : tp;
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = this->BufferWi * this->BufferHt;
        AdjustForTransform(transform, BufferWi, BufferHi);
    } else {
        Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi, stagger);
    }
}
void ArchesModel::InitRenderBufferNodes(const std::string& tp, const std::string& camera, const std::string& transform,
    std::vector<NodeBaseClassPtr>& newNodes, int& BufferWi, int& BufferHi, int stagger, bool deep) const
{
    std::string type = tp.starts_with("Per Model ") ? tp.substr(10) : tp;
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = GetNodeCount();

        int cur = 0;

        for (int x = 0; x < (int)Nodes.size(); x++) {
            newNodes.push_back(NodeBaseClassPtr(Nodes[x]->clone()));
            for (size_t c = 0; c < newNodes[cur]->Coords.size(); c++) {
                newNodes[cur]->Coords[c].bufX = cur;
                newNodes[cur]->Coords[c].bufY = 0;
            }
            cur++;
        }
        ApplyTransform(transform, newNodes, BufferWi, BufferHi);
    } else {
        Model::InitRenderBufferNodes(type, camera, transform, newNodes, BufferWi, BufferHi, stagger);
    }
}

bool ArchesModel::IsNodeFirst(int n) const 
{
    if (GetLayerSizeCount() == 0) {
        return (GetIsLtoR() && n == 0) || (!GetIsLtoR() && n == (int)Nodes.size() - 1);
    } else {
        return n == 0;
    }
}

void ArchesModel::InitModel()
{
    if (GetLayerSizeCount() == 0) {
        int NumArches = _numArches;
        int SegmentsPerArch = _nodesPerArch;

        SetBufferSize(NumArches, SegmentsPerArch);
        if (SingleNode) {
            SetNodeCount(NumArches * SegmentsPerArch, _lightsPerNode, rgbOrder);
        } else {
            SetNodeCount(NumArches, SegmentsPerArch, rgbOrder);
            if (_lightsPerNode > 1) {
                for (size_t x = 0; x < Nodes.size(); x++) {
                    Nodes[x]->Coords.resize(_lightsPerNode);
                }
            }
        }
        screenLocation.SetRenderSize(SegmentsPerArch, NumArches);

        for (int y = 0; y < NumArches; y++) {
            for (int x = 0; x < SegmentsPerArch; x++) {
                int idx = y * SegmentsPerArch + x;
                int startChan = stringStartChan[y] + x * GetNodeChannelCount(StringType);
                if (!IsLtoR) {
                    startChan = stringStartChan[NumArches - y - 1] + (SegmentsPerArch - x - 1) * GetNodeChannelCount(StringType);
                }

                Nodes[idx]->ActChan = startChan;
                Nodes[idx]->StringNum = y;

                for (size_t c = 0; c < GetCoordCount(idx); c++) {
                    Nodes[idx]->Coords[c].bufX = x;
                    Nodes[idx]->Coords[c].bufY = y;
                }
            }
        }
        SetArchCoord();
    } else {
        int maxLen = 0;
        int lcount = 0;
        int sumNodes = 0;
        for (const auto& it : GetLayerSizes()) {
            maxLen = std::max(maxLen, it);
            lcount++;
            sumNodes += it;
        }
        SetBufferSize(lcount, maxLen);
        if (SingleNode) {
            SetNodeCount(_nodesPerArch, _lightsPerNode, rgbOrder);
        } else {
            SetNodeCount(1, _nodesPerArch, rgbOrder);
            if (_lightsPerNode > 1) {
                for (size_t x = 0; x < Nodes.size(); x++) {
                    Nodes[x]->Coords.resize(_lightsPerNode);
                }
            }
        }
        screenLocation.SetRenderSize(maxLen, 1);

        int idx = 0;
        bool dir = IsLtoR;
        bool in_out = isBotToTop;
        float y = 0;
        for (int layer = 0; layer < (int)GetLayerSizeCount(); layer++) {
            int yy = layer;
            if (in_out) yy = GetLayerSizeCount() - layer - 1;
            int it = GetLayerSizes()[yy];
            if (idx < (int)Nodes.size()) {
                if (it == 1) {
                    int startChan = stringStartChan[0] + idx * GetNodeChannelCount(StringType);
                    Nodes[idx]->ActChan = startChan;
                    Nodes[idx]->StringNum = yy;
                    for (size_t c = 0; c < GetCoordCount(idx); c++) {
                        Nodes[idx]->Coords[c].bufX = maxLen / 2;
                        Nodes[idx]->Coords[c].bufY = yy;
                    }
                    idx++;
                } else {
                    float gap = (float)(maxLen - 1) / (float)(it - 1);
                    for (int x = 0; x < it; x++) {
                        if (idx < (int)Nodes.size()) {
                            int xx = std::round((float)x * gap);
                            if (!dir) {
                                xx = maxLen - 1 - xx;
                            }
                            int startChan = stringStartChan[0] + idx * GetNodeChannelCount(StringType);

                            Nodes[idx]->ActChan = startChan;
                            Nodes[idx]->StringNum = 0;

                            for (size_t c = 0; c < GetCoordCount(idx); c++) {
                                Nodes[idx]->Coords[c].bufX = xx;
                                Nodes[idx]->Coords[c].bufY = yy;
                            }
                        }
                        idx++;
                    }
                }
                y = y + 1;
                if (_zigzag) dir = !dir;
            }
        }
        SetLayerdArchCoord(lcount, maxLen);
    }
}

int ArchesModel::MapToNodeIndex(int strand, int node) const {
    if (GetLayerSizeCount() != 0) {
        int idx = 0;
        for (int x = GetLayerSizeCount() - 1; x > strand; x--) {
            idx += GetStrandLength(x);
        }
        idx += node;
        return idx;
    }
    return strand * _nodesPerArch + node;
}
int ArchesModel::GetNumStrands() const {
    if (GetLayerSizeCount() != 0) {
        return GetLayerSizeCount();
    }
    return _numArches;
}

int ArchesModel::NodesPerString() const
{
    if (SingleNode) {
        return 1;
    }
    int ts = GetSmartTs();
    if (ts <= 1) {
        return _nodesPerArch;
    }
    return _nodesPerArch * ts;
}

int ArchesModel::GetStrandLength(int strand) const
{
    if (GetLayerSizeCount() == 0) {
        return Model::GetStrandLength(strand);
    } else {
        return GetLayerSize(strand);
    }
}

int ArchesModel::GetMappedStrand(int strand) const
{
    if (GetLayerSizeCount() != 0) {
        return GetLayerSizeCount() - strand - 1;
    }
    return strand;
}

int ArchesModel::CalcChannelsPerString()
{
    SingleChannel = false;
    return GetNodeChannelCount(StringType) * _nodesPerArch;
}

static void rotate_point(float cx, float cy, float angle, float& x, float& y)
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

void ArchesModel::SetLayerdArchCoord(int arches, int maxLen)
{
    double x;
    size_t NodeCount = GetNodeCount();
    double midpt = maxLen * _lightsPerNode;
    midpt -= 1.0;
    midpt /= 2.0;
    double total = toRadians(_arc);
    double start = (M_PI - total) / 2.0;
    float skew_angle = toRadians(screenLocation.GetAngle());

    double angle = -M_PI / 2.0 + start;
    x = midpt * sin(angle) * 2.0 + maxLen * _lightsPerNode;
    double width = maxLen * _lightsPerNode * 2 - x;

    double archgap = 0;
    if (arches > 1) {
        archgap = (double)(1.0 - _hollow / 100.0) / (double)(arches - 1);
    }

    double minY = 999999;
    for (size_t n = 0; n < NodeCount; n++) {
        size_t CoordCount = GetCoordCount(n);
        double adj = 1.0 - archgap * (arches - 1 - Nodes[n]->Coords[0].bufY);
        for (size_t c = 0; c < CoordCount; c++) {
            double angle2 = -M_PI / 2.0 + start + total * ((double)(Nodes[n]->Coords[c].bufX * _lightsPerNode + c)) / midpt / 2.0;
            x = midpt * sin(angle2) * 2.0 * adj + maxLen * _lightsPerNode;
            double y = (maxLen * _lightsPerNode) * cos(angle2);
            Nodes[n]->Coords[c].screenX = x;
            Nodes[n]->Coords[c].screenY = y * screenLocation.GetMHeight() * adj;
            rotate_point(x, 0, skew_angle,
                Nodes[n]->Coords[c].screenX,
                Nodes[n]->Coords[c].screenY);
            minY = std::min(minY, y);
        }
    }
    float renderHt = _nodesPerArch * _lightsPerNode;
    if (minY > 1) {
        renderHt -= minY;
        for (const auto& it : Nodes) {
            for (auto& coord : it->Coords) {
                coord.screenY -= minY;
            }
        }
    }
    screenLocation.SetRenderSize(width, renderHt);
}

void ArchesModel::SetArchCoord()
{
    double x;
    size_t NodeCount = GetNodeCount();
    double midpt = _nodesPerArch * _lightsPerNode;
    midpt -= 1.0;
    midpt /= 2.0;
    double total = toRadians(_arc);
    double start = (M_PI - total) / 2.0;
    float skew_angle = toRadians(screenLocation.GetAngle());

    double angle = -M_PI / 2.0 + start;
    x = midpt * sin(angle) * 2.0 + _nodesPerArch * _lightsPerNode;
    double width = _nodesPerArch * _lightsPerNode * 2 - x;

    double minY = 999999;
    int gaps = 0;
    for (size_t n = 0; n < NodeCount; n++) {
        double xoffset = Nodes[n]->StringNum * width;
        size_t CoordCount = GetCoordCount(n);
        for (size_t c = 0; c < CoordCount; c++) {
            double angle2 = -M_PI / 2.0 + start + total * ((double)(Nodes[n]->Coords[c].bufX * _lightsPerNode + c)) / midpt / 2.0;
            x = xoffset + midpt * sin(angle2) * 2.0 + _nodesPerArch * _lightsPerNode + gaps * _gap;
            double y = (_nodesPerArch * _lightsPerNode) * cos(angle2);
            Nodes[n]->Coords[c].screenX = x;
            Nodes[n]->Coords[c].screenY = y * screenLocation.GetMHeight();
            rotate_point(x, 0, skew_angle,
                Nodes[n]->Coords[c].screenX,
                Nodes[n]->Coords[c].screenY);
            minY = std::min(minY, y);
        }
        if ((n + 1) % _nodesPerArch == 0) {
            gaps++;
        }
    }
    float renderHt = _nodesPerArch * _lightsPerNode;
    if (minY > 1) {
        renderHt -= minY;
        for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
            for (auto &coord : (*it)->Coords) {
                coord.screenY -= minY;
            }
        }
    }
    screenLocation.SetRenderSize(width * _numArches + (_numArches - 1) * _gap, renderHt);
}

std::string ArchesModel::GetDimension() const
{
    if (GetLayerSizeCount() == 0 && _numArches != 0) {
        return GetModelScreenLocation().GetDimension(1.0 / _numArches);
    }
    return GetModelScreenLocation().GetDimension(1.0);
}

void ArchesModel::OnLayerSizesChange(bool countChanged)
{
    // if string count is 1 then adjust nodes per string to match sum of nodes
    if (_numArches == 1 && GetLayerSizeCount() > 0) {
        _nodesPerArch = (int)GetLayerSizesTotalNodes();
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ArchesModel::OnLayerSizesChange");
    }
}

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//
// TODO
//
// Need to get RGB Dumb working ... 1 arm per string, multi arms per string and all one dumb rgb or single colour


#include <cassert>
#include "SpinnerModel.h"
#include "ModelScreenLocation.h"
#include "../XmlSerializer/XmlNodeKeys.h"

SpinnerModel::SpinnerModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::Spinner;
}

SpinnerModel::~SpinnerModel()
{
}

void SpinnerModel::DecodeStartLocation(int sl)
{
    switch(sl)
    {
    case 0:
        _alternate = false;
        isBotToTop = false;
        IsLtoR = true;
        break;
    case 1:
        _alternate = false;
        isBotToTop = false;
        IsLtoR = false;
        break;
    case 2:
        _alternate = false;
        isBotToTop = true;
        IsLtoR = true;
        break;
    case 3:
        _alternate = false;
        isBotToTop = true;
        IsLtoR = false;
        break;
    case 4:
        isBotToTop = false;
        _alternate = true;
        IsLtoR = true;
        _zigzag = false;
        break;
    case 5:
        isBotToTop = false;
        _alternate = true;
        IsLtoR = false;
        _zigzag = false;
        break;
    default:
        assert(false);
        break;
    }
}

int SpinnerModel::EncodeStartLocation()
{
    if (_alternate)
    {
        if (IsLtoR)
        {
            // Alternate CCW
            return 4;
        }
        else
        {
            // Alternate CW
            return 5;
        }
    }
    else
    {
        if (isBotToTop)
        {
            if (IsLtoR)
            {
                return 2; // Outsde CCW
            }
            else
            {
                return 3; // Outside CW
            }
        }
        else
        {
            if (IsLtoR)
            {
                return 0; // Center CCW
            }
            else
            {
                return 1; // Center CW
            }
        }
    }
}

void SpinnerModel::GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
    int &BufferWi, int &BufferHi, int stagger) const
{
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = this->BufferWi * this->BufferHt;
        AdjustForTransform(transform, BufferWi, BufferHi);
    }
    else {
        Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi, stagger);
    }
}

int SpinnerModel::GetNumStrands() const {
    if (SingleChannel) {
        return _numStrings;
    }
    return _numStrings*_armsPerString;
}

void SpinnerModel::InitModel() {
    int stringcount = _numStrings;
    int nodesperarm = _nodesPerArm;
    int armsperstring = _armsPerString;
    int pixelsperstring = _armsPerString*_nodesPerArm;
    int armcount = _armsPerString*_numStrings;
    SetNodeCount(stringcount, pixelsperstring, rgbOrder);
    screenLocation.SetRenderSize(2 * nodesperarm + 3 + (_hollow * 2.0 * nodesperarm) / 100.0, 2 * nodesperarm + 3 + (_hollow * 2.0 * nodesperarm) / 100.0);

    // create output mapping
    if (SingleNode) {
        SetBufferSize(1, armcount);
        for (size_t x = 0; x<Nodes.size(); x++) {
            Nodes[x]->ActChan = stringStartChan[x];
            Nodes[x]->StringNum = x;
            for (size_t c = 0; c<GetCoordCount(x); c++) {
                Nodes[x]->Coords[c].bufX = IsLtoR ? Nodes.size() - x - 1 : x;
                Nodes[x]->Coords[c].bufY = 0;
            }
        }
    }
    else {
        int chanPerNode = GetNodeChannelCount(StringType);
        SetBufferSize(nodesperarm, armcount);
        for (int x = 0; x < armcount; x++) {
            int stringnum = x / armsperstring;
            int segmentnum = x % armsperstring;
            for (int y = 0; y < nodesperarm; y++)
            {
                int idx = x * nodesperarm + y;
                Nodes[idx]->ActChan = stringStartChan[stringnum] + segmentnum * nodesperarm * chanPerNode + y * chanPerNode;
                Nodes[idx]->Coords[0].bufX = IsLtoR ? x : armcount - x - 1;
                if (_alternate)
                {
                    if (y + 1 <= (nodesperarm+1) / 2)
                    {
                        Nodes[idx]->Coords[0].bufY = 2 * y;
                    }
                    else
                    {
                        Nodes[idx]->Coords[0].bufY = (nodesperarm - (y+1)) * 2 + 1;
                    }
                    Nodes[idx]->Coords[0].bufY = nodesperarm - Nodes[idx]->Coords[0].bufY - 1;
                }
                else
                {
                    if (!_zigzag)
                    {
                        Nodes[idx]->Coords[0].bufY = isBotToTop ? y : nodesperarm - y - 1;
                    }
                    else
                    {
                        if (x % 2 == 0)
                        {
                            Nodes[idx]->Coords[0].bufY = isBotToTop ? y : nodesperarm - y - 1;
                        }
                        else
                        {
                            Nodes[idx]->Coords[0].bufY = isBotToTop ? nodesperarm - y - 1 : y;
                        }
                    }
                }
            }
        }
    }

    SetSpinnerCoord();
    screenLocation.RenderDp = 10.0f;  // give the bounding box a little depth
}

void SpinnerModel::InitRenderBufferNodes(const std::string &tp, const std::string &camera, const std::string &transform,
    std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi, int stagger, bool deep) const {
    std::string type = tp.starts_with("Per Model ") ? tp.substr(10) : tp;
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = GetNodeCount();

        int stringcount = _numStrings;
        int nodesperarm = _nodesPerArm;
        int armsperstring = _armsPerString;
        int cur = 0;
        if (SingleNode)
        {
            for (int i = 0; i < stringcount; i++)
            {
                newNodes.push_back(NodeBaseClassPtr(Nodes[i]->clone()));
                for (size_t c = 0; c < newNodes[cur]->Coords.size(); c++) {
                    newNodes[cur]->Coords[c].bufX = cur;
                    newNodes[cur]->Coords[c].bufY = 0;
                }
                cur++;
            }
        }
        else
        {
            for (int y = 0; y < stringcount*armsperstring; y++) {
                for (int x = 0; x < nodesperarm; x++) {
                    int idx = y * nodesperarm + x;
                    newNodes.push_back(NodeBaseClassPtr(Nodes[idx]->clone()));
                    for (size_t c = 0; c < newNodes[cur]->Coords.size(); c++) {
                        newNodes[cur]->Coords[c].bufX = cur;
                        newNodes[cur]->Coords[c].bufY = 0;
                    }
                    cur++;
                }
            }
        }
        ApplyTransform(transform, newNodes, BufferWi, BufferHi);
    }
    else {
        Model::InitRenderBufferNodes(type, camera, transform, newNodes, BufferWi, BufferHi, stagger);
    }
}

void SpinnerModel::SetSpinnerCoord() {
    int stringcount = _numStrings;
    int nodesperarm = _nodesPerArm;
    int armsperstring = _armsPerString;
    int armcount = _armsPerString*_numStrings;

    float angle = ((float)M_PI * 2.0f * (270.0f + _startAngle)) / 360.0f;
    float angleincrement = (M_PI * 2.0f * (float)_arc) / ((float)stringcount * (float)armsperstring * 360.0f);
    if (_arc < 360 && armsperstring * stringcount > 1)
    {
        angleincrement = (M_PI * 2.0f * (float)_arc) / (((float)stringcount * (float)armsperstring - 1) * 360.0f);
    }

    bool cw = !IsLtoR;
    bool fromcentre = !isBotToTop;

    // top = centre
    // bottom = end
    // left = ccw
    // right = cw

    for (int a = 0; a < armcount; a++)
    {
        if (SingleNode)
        {
            int a1 = a / armsperstring;
            int start = a * nodesperarm - a1 * armsperstring * nodesperarm;
            int end = start + nodesperarm;
            for (int c = start; c < end; c++) {
                int c2 = c - start;
                int c1 = 0;
                if (!fromcentre) {
                    c1 = nodesperarm - c2 - 1;
                }
                else {
                    c1 = c2;
                }

                if (_zigzag && a % 2 > 0) {
                    if (!fromcentre) {
                        c1 = c2;
                    }
                    else {
                        c1 = nodesperarm - c2 - 1;
                    }
                }

                Nodes[a1]->Coords[c].screenX = (0.5f + (float)c1 + ((float)_hollow * 2.0 * (float)nodesperarm) / 100.0) * cos(angle);
                Nodes[a1]->Coords[c].screenY = (0.5f + (float)c1 + ((float)_hollow * 2.0 * (float)nodesperarm) / 100.0) * sin(angle);
            }
        }
        else
        {
            for (int n = 0; n < nodesperarm; n++)
            {
                int n1 = 0;
                if (_alternate)
                {
                    if (n + 1 <= (nodesperarm + 1) / 2)
                    {
                        n1 = 2 * n;
                    }
                    else
                    {
                        n1 = (nodesperarm - (n + 1)) * 2 + 1;
                    }
                }
                else
                {
                    if (!fromcentre)
                    {
                        n1 = nodesperarm - n - 1;
                    }
                    else
                    {
                        n1 = n;
                    }

                    if (_zigzag && a % 2 > 0)
                    {
                        if (!fromcentre)
                        {
                            n1 = n;
                        }
                        else
                        {
                            n1 = nodesperarm - n - 1;
                        }
                    }
                }

                // process each node on each arm
                size_t CoordCount = GetCoordCount(n);
                for (size_t c = 0; c < CoordCount; c++) {
                    Nodes[n + a * nodesperarm]->Coords[c].screenX = (0.5f + (float)n1 + ((float)_hollow * 2.0 * (float)nodesperarm) / 100.0) * cos(angle);
                    Nodes[n + a * nodesperarm]->Coords[c].screenY = (0.5f + (float)n1 + ((float)_hollow * 2.0 * (float)nodesperarm) / 100.0) * sin(angle);
                }
            }
        }
        if (cw)
        {
            angle -= angleincrement;
        }
        else
        {
            angle += angleincrement;
        }
    }
    
    float min = 99999;
    float max = -9999;
    for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
        for (auto it2 : (*it)->Coords) {
            min = std::min(min, it2.screenY);
            max = std::max(max, it2.screenY);
        }
    }
}

int SpinnerModel::MapToNodeIndex(int strand, int node) const {
    if (SingleNode) {
        return strand;
    }

    return strand * _nodesPerArm + node;
}
int SpinnerModel::CalcChannelsPerString() {
    if (SingleNode) {
        return GetNodeChannelCount(StringType);
    }
    return GetNodeChannelCount(StringType) * _nodesPerArm * _armsPerString;
}

int SpinnerModel::NodesPerString() const {
    if (SingleNode) {
        return 1;
    }
    else {
        int ts = GetSmartTs();
        if (ts <= 1) {
            return _nodesPerArm * _armsPerString;
        }
        else {
            return _nodesPerArm * _armsPerString * ts;
        }
    }
}

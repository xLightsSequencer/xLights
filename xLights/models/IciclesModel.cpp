/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/


#include "IciclesModel.h"
#include "../XmlSerializer/XmlNodeKeys.h"

IciclesModel::IciclesModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::Icicles;
    screenLocation.SetMHeight(-0.5);
    screenLocation.SetSupportsShear(true);
}

IciclesModel::~IciclesModel()
{
}

int IciclesModel::NodesPerString() const
{
    if (SingleNode) {
        return 1;
    }
    int ts = GetSmartTs();
    if (ts <= 1) {
        return _lightsPerString;
    }
    return _lightsPerString * ts;
}

void IciclesModel::InitModel()
{
    int numStrings = _numStrings;
    int lightsPerString = _lightsPerString;

    SetNodeCount(numStrings, lightsPerString, rgbOrder);
    ParseDropSizes();

    int width = -1;
    size_t curNode = 0;
    size_t curCoord = 0;
    for (size_t x = 0; x < numStrings; x++) {
        int lights = lightsPerString;
        size_t y = 0;
        size_t curDrop = 0;
        size_t nodesInDrop = _dropSizes[curDrop];
        width++;
        while (lights > 0) {
            if (curCoord >= Nodes[curNode]->Coords.size()) {
                curNode++;
                curCoord = 0;
            }
            while (y >= _dropSizes[curDrop]) {
                width++;
                y = 0;
                curDrop++;
                if (curDrop >= _dropSizes.size()) {
                    curDrop = 0;
                }
                nodesInDrop = _dropSizes[curDrop];
            }
            Nodes[curNode]->ActChan = stringStartChan[0] + curNode * GetNodeChannelCount(StringType);
            Nodes[curNode]->StringNum = x;
            Nodes[curNode]->Coords[curCoord].bufX = width;
            if (HasAlternateNodes()) {
                if (y + 1 <= (nodesInDrop + 1) / 2) {
                    Nodes[curNode]->Coords[curCoord].bufY = _maxH - 1 - (2 * y);
                    Nodes[curNode]->Coords[curCoord].screenY = (2 * y);
                }
                else {
                    Nodes[curNode]->Coords[curCoord].bufY = _maxH - 1 - ((nodesInDrop - (y + 1)) * 2 + 1);
                    Nodes[curNode]->Coords[curCoord].screenY = ((nodesInDrop - (y + 1)) * 2 + 1);
                }
            }
            else {
                Nodes[curNode]->Coords[curCoord].bufY = _maxH - y - 1;
                Nodes[curNode]->Coords[curCoord].screenY = y;
            }
            Nodes[curNode]->Coords[curCoord].screenX = width;
            lights--;
            y++;
            curCoord++;
        }
    }
    if (!IsLtoR) {
        for (size_t n = 0; n < Nodes.size(); n++) {
            for (auto& cd : Nodes[n]->Coords) {
                cd.bufX = width - cd.bufX;
                cd.screenX = width - cd.screenX;
            }
        }
    }
    SetBufferSize(_maxH, width + 1);

    //single icicle move to the center
    if (width == 0) {
        for (auto& n : Nodes) {
            for (auto& c : n->Coords) {
                c.screenX = 0.5;
            }
        }
        width++;
    }
    screenLocation.SetRenderSize(width, _maxH);
}

void IciclesModel::SetDropPattern(const std::string & pattern)
{
    _dropPatternString = pattern;
    ParseDropSizes();
}

void IciclesModel::ParseDropSizes()
{
    _dropSizes.clear();
    auto pat = Split(_dropPatternString, ',');
    _maxH = 0;
    for (int x = 0; x < (int)pat.size(); x++) {
        int d = (int)std::strtol(pat[x].c_str(), nullptr, 10);
        if (d >= 0) { // we dont handle drops of less than zero
            _dropSizes.push_back(d);
            _maxH = std::max(_maxH, (size_t)d);
        }
    }
    if (_dropSizes.size() == 0) {
        _dropSizes.push_back(5);
    }
}

std::string IciclesModel::GetDimension() const
{
    // the height does not make sense for icicles
    return static_cast<TwoPointScreenLocation>(screenLocation).GetDimension(1.0);
}


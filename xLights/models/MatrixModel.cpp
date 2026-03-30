/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <format>

#include "MatrixModel.h"
#include "ModelScreenLocation.h"
#include "../xLightsVersion.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"
#include "../ModelPreview.h"
#include "../XmlSerializer/XmlNodeKeys.h"

#include <log.h>

MatrixModel::MatrixModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::Matrix;
}

MatrixModel::~MatrixModel()
{
}

std::list<std::string> MatrixModel::CheckModelSettings()
{
   std::list<std::string> res;

   if (!StartsWith(StringType, "Single Color") && _nodesPerString % _strandsPerString != 0)
   {
       res.push_back(std::format("    ERR: Model {} strands are not equally sized {} does not divide into string length {} evenly. As a result only {} of {} nodes are initialised.", GetName(), _strandsPerString, _nodesPerString, (int)GetNodeCount(), _numStrings * _nodesPerString));
   }

   res.splice(res.end(), Model::CheckModelSettings());
   return res;
}

int MatrixModel::GetNumStrands() const {
    if (SingleChannel || SingleNode) {
        return _numStrings;
    }
    return _numStrings*_strandsPerString;
}

int MatrixModel::NodesPerString() const
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

bool MatrixModel::ChangeStringCount(long count, std::string & message)
{
    if (count == _numStrings) {
        return true;
    }
    auto oldTotalPix = _numStrings * _nodesPerString;
    auto oldTotalStands = _numStrings * _strandsPerString;
    if (oldTotalPix % count != 0) {
        message = "Pixel Count (" + std::to_string(oldTotalPix) + ") is not divisible by " + std::to_string(count);
        return false;
    }
    if (oldTotalStands % count != 0) {
        message = "Stand Count (" + std::to_string(oldTotalStands) + ") is not divisible by " + std::to_string(count);
        return false;
    }

    auto newNodesPerString = oldTotalPix / count;
    auto newStrandsPerString = oldTotalStands / count;

    _numStrings = (int)count;
    _nodesPerString = (int)newNodesPerString;
    _strandsPerString = (int)newStrandsPerString;
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                OutputModelManager::WORK_RELOAD_MODELLIST |
                OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "MatrixModel::ChangeStringCount::MatrixStringCount");
    return true;
}

// We do this separately as derived models such as Tree assume BufX/Y is set as if it was not single channel
// This goes back and sets them to more appropriate values.
void MatrixModel::InitSingleChannelModel()
{
    // rework bufX/bufY for singleChannel
    if (SingleNode)
    {
        int NumStrands = _numStrings * _strandsPerString;
        int PixelsPerStrand = _nodesPerString / _strandsPerString;
        if (_vMatrix)
        {
            SetBufferSize(SingleNode ? 1 : PixelsPerStrand, SingleNode ? _numStrings : NumStrands);
            int x = 0;
            for (size_t n = 0; n < Nodes.size(); n++) {
                for (auto& c : Nodes[n]->Coords)
                {
                    c.bufX = IsLtoR ? x : NumStrands - x - 1;
                    c.bufY = 0;
                }
                x++;
            }
        }
        else
        {
            SetBufferSize(SingleNode ? _numStrings : NumStrands, SingleNode ? 1 : PixelsPerStrand);
            int y = 0;
            for (size_t n = 0; n < Nodes.size(); n++) {
                for (auto& c : Nodes[n]->Coords)
                {
                    c.bufY = IsLtoR ? y : NumStrands - y - 1;
                    c.bufX = 0;
                }
                y++;
            }
        }
    }
}

void MatrixModel::InitModel() {
    if (_vMatrix) {
        InitVMatrix();
    } else {
        InitHMatrix();
    }
    InitSingleChannelModel();
    screenLocation.RenderDp = 10.0f;  // give the bounding box a little depth
}

// initialize buffer coordinates
// _numStrings=number of strings
// _nodesPerString=pixels per string
// _strandsPerString=strands per string
void MatrixModel::InitVMatrix(int firstExportStrand)
{
    
    int stringnum, segmentnum;
    if (_strandsPerString > _nodesPerString) {
        _strandsPerString = _nodesPerString;
    }
    int NumStrands = _numStrings * _strandsPerString;
    int PixelsPerStrand = _nodesPerString / _strandsPerString;
    int PixelsPerString = PixelsPerStrand * _strandsPerString;
    SetBufferSize(ApplyLowDefinition(PixelsPerStrand), ApplyLowDefinition(NumStrands));
    SetNodeCount(_numStrings, PixelsPerString, rgbOrder);
    screenLocation.SetRenderSize(NumStrands, PixelsPerStrand, 2.0f);
    int chanPerNode = GetNodeChannelCount(StringType);

    // create output mapping
    if (SingleNode) {
        int x = 0;
        float sx = 0;
        for (size_t n = 0; n < Nodes.size(); n++) {
            Nodes[n]->ActChan = stringStartChan[n];
            float sy = 0;
            for (auto& c : Nodes[n]->Coords) {
                c.screenX = isBotToTop ? sx : NumStrands - sx - 1;
                c.screenX -= ((float)NumStrands - 1.0) / 2.0;
                c.screenY = sy - ((float)PixelsPerStrand - 1.0) / 2.0 - 0.5;
                c.screenZ = 0;
                sy++;
                if (sy >= PixelsPerStrand) {
                    sy = 0;
                    sx++;
                }
            }

            int y = 0;
            int yincr = 1;
            for (int c = 0; c < PixelsPerString; c++) {
                Nodes[n]->Coords[c].bufX = IsLtoR ? x : NumStrands - x - 1;
                Nodes[n]->Coords[c].bufY = y;
                y += yincr;
                if (y < 0 || y >= PixelsPerStrand) {
                    yincr = -yincr;
                    y += yincr;
                    x++;
                }
            }
        }
        GetModelScreenLocation().SetRenderSize(NumStrands, PixelsPerStrand, GetModelScreenLocation().GetRenderDp());

    } else {
        std::vector<int> strandStartChan;
        strandStartChan.clear();
        strandStartChan.resize(NumStrands);
        for (int x2 = 0; x2 < NumStrands; x2++) {
            stringnum = x2 / _strandsPerString;
            segmentnum = x2 % _strandsPerString;
            strandStartChan[x2] = stringStartChan[stringnum] + segmentnum * PixelsPerStrand * chanPerNode;
        }
        if (firstExportStrand > 0 && firstExportStrand < NumStrands) {
            int offset = strandStartChan[firstExportStrand];
            for (int x2 = 0; x2 < NumStrands; x2++) {
                strandStartChan[x2] = strandStartChan[x2] - offset;
                if (strandStartChan[x2] < 0) {
                    strandStartChan[x2] += (PixelsPerStrand * NumStrands * chanPerNode);
                }
            }
        }

        if (_lowDefFactor == 100 || !SupportsLowDefinitionRender() || !GetModelManager().GetXLightsFrame()->IsLowDefinitionRender()) {
            for (int x = 0; x < NumStrands; x++) {
                stringnum = x / _strandsPerString;
                segmentnum = x % _strandsPerString;
                for (int y = 0; y < PixelsPerStrand; y++) {
                    int idx = stringnum * PixelsPerString + segmentnum * PixelsPerStrand + y;
                    Nodes[idx]->ActChan = strandStartChan[x] + y * chanPerNode;
                    Nodes[idx]->Coords[0].bufX = IsLtoR ? x : NumStrands - x - 1;
                    Nodes[idx]->StringNum = stringnum;
                    if (HasAlternateNodes()) {
                        if (isBotToTop) {
                            if (y + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufY = y * 2;
                            } else {
                                Nodes[idx]->Coords[0].bufY = ((PixelsPerStrand - (y + 1)) * 2 + 1);
                            }
                        } else {
                            if (y + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufY = (PixelsPerStrand - 1) - (y * 2);
                            } else {
                                Nodes[idx]->Coords[0].bufY = (PixelsPerStrand - 1) - ((PixelsPerStrand - (y + 1)) * 2 + 1);
                            }
                        }
                    } else {
                        if (IsNoZigZag())
                        {
                            Nodes[idx]->Coords[0].bufY = isBotToTop == true ? y : PixelsPerStrand - y - 1;
                        } else {
                            Nodes[idx]->Coords[0].bufY = isBotToTop == (segmentnum % 2 == 0) ? y : PixelsPerStrand - y - 1;
                        }
                    }
                }
            }
            CopyBufCoord2ScreenCoord();
        } else {
            spdlog::debug("Building low definition buffer at {}%", _lowDefFactor);

            int xoffset = NumStrands / 2;
            int yoffset = PixelsPerStrand / 2;

            for (int x = 0; x < NumStrands; x++) {
                stringnum = x / _strandsPerString;
                segmentnum = x % _strandsPerString;
                for (int y = 0; y < PixelsPerStrand; y++) {
                    int idx = stringnum * PixelsPerString + segmentnum * PixelsPerStrand + y;
                    Nodes[idx]->ActChan = strandStartChan[x] + y * chanPerNode;
                    Nodes[idx]->Coords[0].bufX = IsLtoR ? x : NumStrands - x - 1;
                    Nodes[idx]->StringNum = stringnum;
                    if (HasAlternateNodes()) {
                        if (isBotToTop) {
                            if (y + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufY = y * 2;
                            } else {
                                Nodes[idx]->Coords[0].bufY = ((PixelsPerStrand - (y + 1)) * 2 + 1);
                            }
                        } else {
                            if (y + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufY = (PixelsPerStrand - 1) - (y * 2);
                            } else {
                                Nodes[idx]->Coords[0].bufY = (PixelsPerStrand - 1) - ((PixelsPerStrand - (y + 1)) * 2 + 1);
                            }
                        }
                    } else {
                        if (IsNoZigZag())
                        {
                            Nodes[idx]->Coords[0].bufY = isBotToTop == true ? y : PixelsPerStrand - y - 1;
                        } else {
                            Nodes[idx]->Coords[0].bufY = isBotToTop == (segmentnum % 2 == 0) ? y : PixelsPerStrand - y - 1;
                        }
                    }
                    // before we adjust the buffer capture the screen coordinates
                    for (size_t c = 0; c < GetCoordCount(idx); c++) {
                        Nodes[idx]->Coords[c].screenX = Nodes[idx]->Coords[0].bufX - xoffset;
                        Nodes[idx]->Coords[c].screenY = Nodes[idx]->Coords[0].bufY - yoffset;
                    }
                    // now we need to adjust the buffer coords
                    Nodes[idx]->Coords[0].bufX = ApplyLowDefinition(Nodes[idx]->Coords[0].bufX);
                    Nodes[idx]->Coords[0].bufY = ApplyLowDefinition(Nodes[idx]->Coords[0].bufY);
                }
            }
        }
    }
}

// initialize buffer coordinates
// _numStrings=number of strings
// _nodesPerString=pixels per string
// _strandsPerString=strands per string
void MatrixModel::InitHMatrix() {
    
    int idx,stringnum,segmentnum,xincr;
    if (_strandsPerString > _nodesPerString) {
        _strandsPerString = _nodesPerString;
    }
    int NumStrands=_numStrings*_strandsPerString;
    int PixelsPerStrand=_nodesPerString/_strandsPerString;
    int PixelsPerString=PixelsPerStrand*_strandsPerString;
    SetBufferSize(ApplyLowDefinition(NumStrands), ApplyLowDefinition(PixelsPerStrand));
    SetNodeCount(_numStrings,PixelsPerString,rgbOrder);
    screenLocation.SetRenderSize(PixelsPerStrand, NumStrands, 2.0f);
    
    int chanPerNode = GetNodeChannelCount(StringType);

    // create output mapping
    if (SingleNode) {
        int y=0;
        float sy = 0;
        for (size_t n=0; n<Nodes.size(); n++) {
            Nodes[n]->ActChan = stringStartChan[n];
            float sx = 0;
            for (auto& c : Nodes[n]->Coords)
            {
                c.screenY = isBotToTop ? sy : NumStrands - sy - 1;
                c.screenY -= ((float)NumStrands-1.0) / 2.0;
                c.screenX = sx - ((float)PixelsPerStrand-1.0) / 2.0 - 0.5;
                c.screenZ = 0;
                sx++;
                if (sx >= PixelsPerStrand)
                {
                    sx = 0;
                    sy++;
                }
            }

            int x = 0;
            xincr = 1;
            for (int c = 0; c < PixelsPerString; c++) {
                Nodes[n]->Coords[c].bufX = x;
                Nodes[n]->Coords[c].bufY = isBotToTop ? y : NumStrands - y - 1;
                x += xincr;
                if (x < 0 || x >= PixelsPerStrand) {
                    xincr = -xincr;
                    x += xincr;
                    y++;
                }
            }
        }
        GetModelScreenLocation().SetRenderSize(PixelsPerStrand, NumStrands, GetModelScreenLocation().GetRenderDp());

    } else {
        if (_lowDefFactor == 100 || !SupportsLowDefinitionRender() || !GetModelManager().GetXLightsFrame()->IsLowDefinitionRender()) {
            for (int y = 0; y < NumStrands; y++) {
                stringnum = y / _strandsPerString;
                segmentnum = y % _strandsPerString;
                for (int x = 0; x < PixelsPerStrand; x++) {
                    idx = stringnum * PixelsPerString + segmentnum * PixelsPerStrand + x;
                    Nodes[idx]->ActChan = stringStartChan[stringnum] + segmentnum * PixelsPerStrand * chanPerNode + x * chanPerNode;

                    Nodes[idx]->Coords[0].bufY = isBotToTop ? y : NumStrands - y - 1;
                    Nodes[idx]->StringNum = stringnum;

                    if (HasAlternateNodes()) {
                        if (IsLtoR) {
                            if (x + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufX = x * 2;
                            } else {
                                Nodes[idx]->Coords[0].bufX = ((PixelsPerStrand - (x + 1)) * 2 + 1);
                            }
                        } else {
                            if (x + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufX = (PixelsPerStrand - 1) - (x * 2);
                            } else {
                                Nodes[idx]->Coords[0].bufX = (PixelsPerStrand - 1) - ((PixelsPerStrand - (x + 1)) * 2 + 1);
                            }
                        }
                    } else {
                        if (IsNoZigZag())
                        {
                            Nodes[idx]->Coords[0].bufX = IsLtoR != true ? PixelsPerStrand - x - 1 : x;
                        } else {
                            Nodes[idx]->Coords[0].bufX = IsLtoR != (segmentnum % 2 == 0) ? PixelsPerStrand - x - 1 : x;
                        }
                    }
                }
            }
            CopyBufCoord2ScreenCoord();
        } else {
            spdlog::debug("Building low definition buffer at {}%", _lowDefFactor);

            int xoffset = PixelsPerStrand / 2;
            int yoffset = NumStrands / 2;

            for (int y = 0; y < NumStrands; y++) {
                stringnum = y / _strandsPerString;
                segmentnum = y % _strandsPerString;
                for (int x = 0; x < PixelsPerStrand; x++) {
                    idx = stringnum * PixelsPerString + segmentnum * PixelsPerStrand + x;
                    Nodes[idx]->ActChan = stringStartChan[stringnum] + segmentnum * PixelsPerStrand * chanPerNode + x * chanPerNode;

                    Nodes[idx]->Coords[0].bufY = isBotToTop ? y : NumStrands - y - 1;
                    Nodes[idx]->StringNum = stringnum;

                    if (HasAlternateNodes()) {
                        if (IsLtoR) {
                            if (x + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufX = x * 2;
                            } else {
                                Nodes[idx]->Coords[0].bufX = ((PixelsPerStrand - (x + 1)) * 2 + 1);
                            }
                        } else {
                            if (x + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufX = (PixelsPerStrand - 1) - (x * 2);
                            } else {
                                Nodes[idx]->Coords[0].bufX = (PixelsPerStrand - 1) - ((PixelsPerStrand - (x + 1)) * 2 + 1);
                            }
                        }
                    } else {
                        if (IsNoZigZag())
                        {
                            Nodes[idx]->Coords[0].bufX = IsLtoR != true ? PixelsPerStrand - x - 1 : x;
                        } else {
                            Nodes[idx]->Coords[0].bufX = IsLtoR != (segmentnum % 2 == 0) ? PixelsPerStrand - x - 1 : x;
                        }
                    }
                    // before we adjust the buffer capture the screen coordinates
                    for (size_t c = 0; c < GetCoordCount(idx); c++) {
                        Nodes[idx]->Coords[c].screenX = Nodes[idx]->Coords[0].bufX - xoffset;
                        Nodes[idx]->Coords[c].screenY = Nodes[idx]->Coords[0].bufY - yoffset;
                    }
                    // now we need to adjust the buffer coords
                    Nodes[idx]->Coords[0].bufX = ApplyLowDefinition(Nodes[idx]->Coords[0].bufX);
                    Nodes[idx]->Coords[0].bufY = ApplyLowDefinition(Nodes[idx]->Coords[0].bufY);
                }
            }
        }
    }
}


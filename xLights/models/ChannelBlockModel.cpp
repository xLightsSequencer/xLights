/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/


#include "ChannelBlockModel.h"
#include "ModelScreenLocation.h"
#include "../OutputModelManager.h"
#include "../UtilFunctions.h"
#include "../XmlSerializer/XmlNodeKeys.h"

#define MAX_CB_CHANNELS 128

std::vector<std::string> ChannelBlockModel::LINE_BUFFER_STYLES;

ChannelBlockModel::ChannelBlockModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::ChannelBlock;
}

ChannelBlockModel::~ChannelBlockModel()
{
}

const std::vector<std::string> &ChannelBlockModel::GetBufferStyles() const {
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

void ChannelBlockModel::GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi, int stagger) const {
    BufferHi = 1;
    BufferWi = GetNodeCount();
}

void ChannelBlockModel::InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                        std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi, int stagger, bool deep) const {
    BufferHi = 1;
    BufferWi = GetNodeCount();
        
    int NumChannels=parm1;
    int cur = 0;
    for (int y=0; y < NumChannels; ++y) {
		int idx = y;
		newNodes.push_back(NodeBaseClassPtr(Nodes[idx]->clone()));
		for(size_t c=0; c < newNodes[cur]->Coords.size(); ++c) {
			newNodes[cur]->Coords[c].bufX=cur;
			newNodes[cur]->Coords[c].bufY=0;
		}
		cur++;
    }
}

void ChannelBlockModel::InitModel() {
    StringType = "Single Color Custom";
    customColor = xlWHITE;
    SingleNode = true;

    InitChannelBlock();

    for (auto node = Nodes.begin(); node != Nodes.end(); ++node) {
        int count = 0;
        int num = node->get()->Coords.size();
        float offset = 0.0;
        if (num == 1) {
            offset = 0.5;
        }
        else {
            offset = (float)1 / (float)num / 2.0;
        }
        for (auto &coord : node->get()->Coords) {
            coord.screenY = 0;
            if (num > 1) {
                coord.screenX = (float)coord.bufX + (float)count / (float)num + offset;
                count++;
            }
            else {
                coord.screenX = coord.bufX + offset;
            }
        }
    }
    screenLocation.SetRenderSize(BufferWi, 1);
}

void ChannelBlockModel::InitChannelBlock() {
    size_t NodeCount = GetNodeCount();
    if (NodeCount != parm1) {
        SetNodeCount(parm1, 1, rgbOrder);
        NodeCount = parm1;
    }
    SetBufferSize(1, parm1);
    int LastStringNum = -1;
    int chan = 0;
    int ChanIncr = 1;
    _channelColors.resize(parm1);

    for (int x = 0; x < parm1; ++x) {
        if (_channelColors[x] == xlEMPTY_STRING) {
            _channelColors[x] = "white";
        }
    }

    int idx = 0;
    for (size_t n = 0; n<NodeCount; ++n) {
        if (Nodes[n]->StringNum != LastStringNum) {
            LastStringNum = Nodes[n]->StringNum;
            chan = stringStartChan[LastStringNum];
        }
        Nodes[n]->ActChan = chan;
        chan += ChanIncr;
        Nodes[n]->Coords.resize(1);
        size_t CoordCount = GetCoordCount(n);
        for (size_t c = 0; c < CoordCount; ++c) {
            Nodes[n]->Coords[c].bufX = idx;
            Nodes[n]->Coords[c].bufY = 0;
        }
        idx++;
        xlColor c = xlColor(_channelColors[n]);
        NodeClassCustom* ncc = dynamic_cast<NodeClassCustom*>(Nodes[n].get());
        ncc->SetMaskColor(c);
        ncc->SetCustomColor(c);
    }
}

int ChannelBlockModel::MapToNodeIndex(int strand, int node) const {
    return strand;
}
int ChannelBlockModel::GetNumStrands() const {
     return parm1;
}
int ChannelBlockModel::CalcChannelsPerString() {
	return GetNodeChannelCount(StringType);
}

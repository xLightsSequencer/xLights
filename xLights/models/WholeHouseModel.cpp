
#include <wx/wx.h>
#include <wx/xml/xml.h>

#include "WholeHouseModel.h"
#include "ModelScreenLocation.h"

WholeHouseModel::WholeHouseModel(wxXmlNode *node, const ModelManager &manager, bool zb) : ModelWithScreenLocation(manager)
{
    SetFromXml(node, zb);
}
WholeHouseModel::WholeHouseModel(const ModelManager &manager) : ModelWithScreenLocation(manager) {
    
}


WholeHouseModel::~WholeHouseModel()
{
    //dtor
}


void WholeHouseModel::InitModel() {
    std::string WholeHouseData = ModelXml->GetAttribute("WholeHouseData").ToStdString();
    InitWholeHouse(WholeHouseData);
    CopyBufCoord2ScreenCoord();
}

void WholeHouseModel::InitWholeHouse(const std::string &WholeHouseData) {
    long xCoord,yCoord,actChn;
    int lastActChn=0;
    wxArrayString data;
    SetBufferSize(parm2,parm1);
    screenLocation.SetRenderSize(parm1, parm2);
    wxString stringType;
    
    Nodes.clear();
    int minChan = 9999999;
    int maxChan = -1;
    if(WholeHouseData.length()> 0) {
        wxArrayString wholeHouseDataArr=wxSplit(WholeHouseData,';');
        int coordinateCount=wholeHouseDataArr.size();
        
        // Load first coordinate
        data=wxSplit(wholeHouseDataArr[0],',');
        data[0].ToLong(&actChn);
        if (actChn > maxChan) {
            maxChan = actChn;
        }
        if (actChn < minChan) {
            minChan = actChn;
        }
        data[1].ToLong(&xCoord);
        data[2].ToLong(&yCoord);
        if (data.size() > 3) {
            stringType = data[3];
        } else {
            stringType = rgbOrder;
        }
        Nodes.push_back(NodeBaseClassPtr(createNode(1, stringType.ToStdString(), 1, stringType.ToStdString())));
        Nodes.back()->StringNum = 0;
        Nodes.back()->ActChan = actChn;
        Nodes.back()->Coords[0].bufX = xCoord;
        Nodes.back()->Coords[0].bufY = yCoord;
        lastActChn = actChn;
        for(size_t i=1; i < coordinateCount; i++) {
            data=wxSplit(wholeHouseDataArr[i],',');
            data[0].ToLong(&actChn);
            data[1].ToLong(&xCoord);
            data[2].ToLong(&yCoord);
            if (data.size() > 3) {
                stringType = data[3];
            } else {
                stringType = rgbOrder;
            }
            if(actChn != lastActChn) {
                Nodes.push_back(NodeBaseClassPtr(createNode(1, stringType.ToStdString(), 1, stringType.ToStdString())));
                Nodes.back()->StringNum = 0;
                Nodes.back()->ActChan = actChn;
                Nodes.back()->Coords[0].bufX = xCoord;
                Nodes.back()->Coords[0].bufY = yCoord;
            } else {
                Nodes.back()->AddBufCoord(xCoord,yCoord);
            }
            lastActChn = actChn;
        }
    }
    if (zeroBased && minChan != 0) {
        for (int x = 0; x < Nodes.size(); x++) {
            Nodes[x]->ActChan -= minChan;
        }
    }
}

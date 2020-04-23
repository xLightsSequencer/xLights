/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/


#include <wx/wx.h>
#include <wx/xml/xml.h>

#include "WholeHouseModel.h"
#include "ModelScreenLocation.h"


/*
 The WholeHouseModel is an ancient and long since deprecated class to represent a group of models.  There
 is currently no way to create one of these.  The class is only here to allow loading of older setups that
 have one
 */

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

NodeBaseClass* WholeHouseModel::createNode(int ns, const std::string &StringType, size_t NodesPerString, const std::string &rgbOrder) const
{
    NodeBaseClass *ret;
    if (StringType=="Single Color Red" || StringType == "R") {
        ret = new NodeClassRed(ns, NodesPerString);
    } else if (StringType=="Single Color Green" || StringType == "G") {
        ret = new NodeClassGreen(ns, NodesPerString);
    } else if (StringType=="Single Color Blue" || StringType == "B") {
        ret = new NodeClassBlue(ns, NodesPerString);
    } else if (StringType=="Single Color White" || StringType == "W") {
        ret = new NodeClassWhite(ns, NodesPerString);
    } else if (StringType[0] == '#') {
        ret = new NodeClassCustom(ns, NodesPerString, xlColor(StringType));
    } else if (StringType=="Strobes White 3fps" || StringType=="Strobes") {
        ret = new NodeClassWhite(ns, NodesPerString);
    } else if (StringType=="4 Channel RGBW" || StringType == "RGBW") {
        ret = new NodeClassRGBW(ns, NodesPerString, "RGB", true, 0);
    } else if (StringType=="4 Channel WRGB" || StringType == "WRGB") {
        ret = new NodeClassRGBW(ns, NodesPerString, "RGB", false, 0);
    } else {
        ret = new NodeBaseClass(ns, 1, rgbOrder);
    }
    ret->model = this;
    return ret;
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

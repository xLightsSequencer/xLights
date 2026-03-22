/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MultiPointModel.h"
#include "../xLightsMain.h"
#include "../xLightsVersion.h"
#include "../XmlSerializer/XmlNodeKeys.h"

MultiPointModel::MultiPointModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::MultiPoint;
    parm1 = parm2 = parm3 = 0;
}

MultiPointModel::~MultiPointModel()
{
}

bool MultiPointModel::IsNodeFirst(int n) const
{
    return (GetIsLtoR() && n == 0) || (!GetIsLtoR() && n == Nodes.size() - 1);
}

int MultiPointModel::MapToNodeIndex(int strand, int node) const {
    return strand * screenLocation.num_points + node;
}

void MultiPointModel::InitModel()
{
    parm1 = 1;
    
    InitLine();

    // calculate min/max for the model
    float minX = 100000.0f;
    float minY = 100000.0f;
    float minZ = 100000.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    float maxZ = 0.0f;

    std::vector<PolyPointScreenLocation::xlPolyPoint> pPos(screenLocation.num_points);

    for (int i = 0; i < screenLocation.num_points; ++i) {
        if (screenLocation.mPos[i].x < minX) minX = screenLocation.mPos[i].x;
        if (screenLocation.mPos[i].y < minY) minY = screenLocation.mPos[i].y;
        if (screenLocation.mPos[i].z < minZ) minZ = screenLocation.mPos[i].z;
        if (screenLocation.mPos[i].x > maxX) maxX = screenLocation.mPos[i].x;
        if (screenLocation.mPos[i].y > maxY) maxY = screenLocation.mPos[i].y;
        if (screenLocation.mPos[i].z > maxZ) maxZ = screenLocation.mPos[i].z;
    }
    float deltax = maxX - minX;
    float deltay = maxY - minY;
    float deltaz = maxZ - minZ;

    // normalize all the point data
    for (int i = 0; i < screenLocation.num_points; ++i) {
        if (deltax == 0.0f) {
            pPos[i].x = 0.0f;
        } else {
            pPos[i].x = (screenLocation.mPos[i].x - minX) / deltax;
        }
        if (deltay == 0.0f) {
            pPos[i].y = 0.0f;
        } else {
            pPos[i].y = (screenLocation.mPos[i].y - minY) / deltay;
        }
        if (deltaz == 0.0f) {
            pPos[i].z = 0.0f;
        } else {
            pPos[i].z = (screenLocation.mPos[i].z - minZ) / deltaz;
        }
    }

    screenLocation.SetRenderSize(1.0, 1.0);

    double model_height = deltay;
    if (model_height < GetModelScreenLocation().GetRenderHt()) {
        model_height = GetModelScreenLocation().GetRenderHt();
    }

    // place the nodes/coords along each line segment
    int i = 0;
    if ( Nodes.size() > 0 && (BufferWi > 1 || Nodes.front()->Coords.size() > 1))
    {
        for (auto& n : Nodes)
        {
            for (auto& c : n->Coords)
            {
                c.screenX = pPos[i].x;
                c.screenY = pPos[i].y;
                c.screenZ = pPos[i].z;
            }
            i++;
        }
    }
    else if (Nodes.size() > 0)
    {
        // 1 node 1 light
        Nodes.front()->Coords.front().screenY = 0.0;
        Nodes.front()->Coords.front().screenX = 0.5;
    }
}

// initialize buffer coordinates
// parm1=Number of Strings/Arches/Canes
// parm2=Pixels Per String/Arch/Cane
void MultiPointModel::InitLine() {
    int numLights = parm1 * screenLocation.num_points;
    Nodes.clear();
    SetNodeCount(parm1,screenLocation.num_points,rgbOrder);
    SetBufferSize(1,SingleNode?parm1:numLights);
    int LastStringNum=-1;
    int chan = 0;
    int ChanIncr = GetNodeChannelCount(StringType);
    size_t NodeCount=GetNodeCount();
    if (!IsLtoR) {
        ChanIncr = -ChanIncr;
    }

    int idx = 0;
    for(size_t n=0; n<NodeCount; n++) {
        if (Nodes[n]->StringNum != LastStringNum) {
            LastStringNum=Nodes[n]->StringNum;
            chan=stringStartChan[LastStringNum];
            if (!IsLtoR) {
                chan += NodesPerString(LastStringNum) * GetNodeChannelCount(StringType);
                chan += ChanIncr;
            }
        }
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        Nodes[n]->Coords.resize(SingleNode?screenLocation.num_points:parm3);
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            Nodes[n]->Coords[c].bufX=idx;
            Nodes[n]->Coords[c].bufY=0;
        }
        idx++;
    }
}
int MultiPointModel::GetNumPhysicalStrings() const
{
    int ts = GetSmartTs();
    if (ts <= 1) {
        return _strings;
    } else {
        int strings = _strings / ts;
        if (strings == 0)
            strings = 1;
        return strings;
    }
}

void MultiPointModel::DeleteHandle(int handle_) {
    // handle is offset by 1 due to the center handle at 0
    int handle = handle_ - 1;
    GetModelScreenLocation().DeleteHandle(handle);
}

// Call this before exporting the points
void MultiPointModel::NormalizePointData()
{

}

//TODO:  Do we need this code to Normalize...was used for an export
/*std::string MultiPointModel::GetPointDataAsString() const
{
    // First normalize all the data
    float minX = 100000.0f;
    float minY = 100000.0f;
    float minZ = 100000.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    float maxZ = 0.0f;

    for (int i = 0; i < screenLocation.num_points; ++i) {
        if (screenLocation.mPos[i].x < minX)
            minX = screenLocation.mPos[i].x;
        if (screenLocation.mPos[i].y < minY)
            minY = screenLocation.mPos[i].y;
        if (screenLocation.mPos[i].z < minZ)
            minZ = screenLocation.mPos[i].z;
        if (screenLocation.mPos[i].x > maxX)
            maxX = screenLocation.mPos[i].x;
        if (screenLocation.mPos[i].y > maxY)
            maxY = screenLocation.mPos[i].y;
        if (screenLocation.mPos[i].z > maxZ)
            maxZ = screenLocation.mPos[i].z;
    }
    float deltax = maxX - minX;
    float deltay = maxY - minY;
    float deltaz = maxZ - minZ;

    // normalize all the point data
    std::vector<xlMultiPoint> pPos;
    pPos.resize(screenLocation.num_points);
    for (int i = 0; i < screenLocation.num_points; ++i) {
        if (deltax == 0.0f) {
            pPos[i].x = 0.0f;
        } else {
            pPos[i].x = (screenLocation.mPos[i].x - minX) / deltax;
        }
        if (deltay == 0.0f) {
            pPos[i].y = 0.0f;
        } else {
            pPos[i].y = (screenLocation.mPos[i].y - minY) / deltay;
        }
        if (deltaz == 0.0f) {
            pPos[i].z = 0.0f;
        } else {
            pPos[i].z = (screenLocation.mPos[i].z - minZ) / deltaz;
        }
    }

    std::string point_data = "";
    for (int i = 0; i < screenLocation.num_points; ++i) {
        point_data += wxString::Format("%f,", screenLocation.mPos[i].x);
        point_data += wxString::Format("%f,", screenLocation.mPos[i].y);
        point_data += wxString::Format("%f", screenLocation.mPos[i].z);
        if (i != screenLocation.num_points - 1) {
            point_data += ",";
        }
    }
    return point_data;
}*/

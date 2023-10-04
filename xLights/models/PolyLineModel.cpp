/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/log.h>
#include <wx/filedlg.h>

#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/mat3x3.hpp>

#include "PolyLineModel.h"
#include "ModelScreenLocation.h"
#include "Shapes.h"
#include "../support/VectorMath.h"
#include "../xLightsMain.h"
#include "../xLightsVersion.h"
#include "UtilFunctions.h"
#include "../ModelPreview.h"

#include <log4cpp/Category.hh>

PolyLineModel::PolyLineModel(const ModelManager &manager) : ModelWithScreenLocation(manager) {
    parm1 = parm2 = parm3 = 0;
}

PolyLineModel::PolyLineModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    PolyLineModel::SetFromXml(node, zeroBased);
}

PolyLineModel::~PolyLineModel()
{
    //dtor
}

static const char* POLY_CORNER_VALUES[] = {
    "Leading Segment",
    "Trailing Segment",
    "Neither"
};
static wxPGChoices POLY_CORNERS(wxArrayString(3, POLY_CORNER_VALUES));

std::vector<std::string> PolyLineModel::POLYLINE_BUFFER_STYLES;

const std::vector<std::string> &PolyLineModel::GetBufferStyles() const {

    if (!hasIndivSeg)
    {
        return Model::DEFAULT_BUFFER_STYLES;
    }

    struct Initializer {
        Initializer() {
            POLYLINE_BUFFER_STYLES = Model::DEFAULT_BUFFER_STYLES;
            POLYLINE_BUFFER_STYLES.push_back("Line Segments");
        }
    };
    static Initializer ListInitializationGuard;
    return POLYLINE_BUFFER_STYLES;
}

bool PolyLineModel::IsNodeFirst(int n) const
{
    return (GetIsLtoR() && n == 0) || (!GetIsLtoR() && n == Nodes.size() - 1);
}

void PolyLineModel::InitRenderBufferNodes(const std::string& type, const std::string& camera,
    const std::string& transform,
    std::vector<NodeBaseClassPtr>& newNodes, int& BufferWi, int& BufferHi, int stagger, bool deep) const
{
    if (type == "Line Segments" && hasIndivSeg) {
        BufferHi = num_segments;
        BufferWi = 0;
        for (int x = 0; x < num_segments; x++) {
            int w = polyLineSizes[x];
            if (w > BufferWi) {
                BufferWi = w;
            }
        }
        for (const auto& it : Nodes) {
            newNodes.push_back(NodeBaseClassPtr(it.get()->clone()));
        }

        int idx = 0;
        for (size_t m = 0; m < num_segments; m++) {
            int seg_idx = 0;
            int end_node = idx + polyLineSizes[m];
            float scale = (float)BufferWi / (float)polyLineSizes[m];
            for (size_t n = idx; n < end_node; n++) {
                newNodes[idx]->Coords.resize(SingleNode ? parm2 : parm3);
                size_t CoordCount = GetCoordCount(idx);
                int location = seg_idx * scale + scale / 2.0;
                for (size_t c = 0; c < CoordCount; c++) {
                    newNodes[idx]->Coords[c].bufX = location;
                    newNodes[idx]->Coords[c].bufY = m;
                    newNodes[idx]->Coords[c].bufZ = 0;
                }
                idx++;
                seg_idx++;
            }
        }
        ApplyTransform(transform, newNodes, BufferWi, BufferHi);
    }
    else {
        Model::InitRenderBufferNodes(type, camera, transform, newNodes, BufferWi, BufferHi, stagger);
    }
}

int PolyLineModel::GetPolyLineSize(int polyLineLayer) const {
    if (polyLineLayer >= polyLineSizes.size()) return 0;
    if (polyLineSegDropSizes[polyLineLayer]) {
        return polyLineSegDropSizes[polyLineLayer];
    }
    return polyLineSizes[polyLineLayer];
}

int PolyLineModel::GetStrandLength(int strand) const {
    return SingleNode ? 1 : GetPolyLineSize(strand);
}

int PolyLineModel::MapToNodeIndex(int strand, int node) const {
    int idx = 0;
    for (int x = 0; x < strand; x++) {
        idx += GetPolyLineSize(x);
    }
    idx += node;
    return idx;
}

int PolyLineModel::GetNumStrands() const {
    return SingleNode ? 1 : polyLineSizes.size();
}

void PolyLineModel::SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString) {
    std::string tempstr = ModelXml->GetAttribute("Advanced", "0").ToStdString();
    _strings = wxAtoi(ModelXml->GetAttribute("PolyStrings", "1").ToStdString());
    bool HasIndividualStartChans = tempstr == "1";
    if( HasIndividualStartChans && !SingleNode ) {
        // if individual start channels defer to InitModel where we know all the segment length data
    } else {
        if (_strings == 1) {
            Model::SetStringStartChannels(zeroBased, NumberOfStrings, StartChannel, ChannelsPerString);
        } else {
            ChannelsPerString /= _strings;
            stringStartChan.clear();
            stringStartChan.resize(_strings);

            for (int i = 0; i < _strings; i++) {
                wxString nm = StartNodeAttrName(i);
                int node = wxAtoi(ModelXml->GetAttribute(nm, "-1"));
                if (node < 0) {
                    node = ((ChannelsPerString * i) / GetNodeChannelCount(StringType)) + 1;
                }
                stringStartChan[i] = (zeroBased ? 0 : StartChannel - 1) + (node - 1) * GetNodeChannelCount(StringType);
            }
        }
    }
}

void PolyLineModel::InsertHandle(int after_handle, float zoom, int scale) {
    if( polyLineSizes.size() > after_handle ) {
        for (int x = num_segments-1; x > after_handle; --x) {
            std::string val = ModelXml->GetAttribute(SegAttrName(x)).ToStdString();
            if (val == "") {
                val = wxString::Format("%d", polyLineSizes[x]);
            }
            ModelXml->DeleteAttribute(SegAttrName(x));
            ModelXml->AddAttribute(SegAttrName(x+1), val);
        }
        int segment1_size = polyLineSizes[after_handle] / 2;
        int segment2_size = polyLineSizes[after_handle] - segment1_size;
        polyLineSizes[after_handle] = segment1_size;
        polyLineSizes.insert(polyLineSizes.begin() + after_handle + 1, segment2_size);
        std::string val = "";
        val = wxString::Format("%d", polyLineSizes[after_handle]);
        ModelXml->DeleteAttribute(SegAttrName(after_handle));
        ModelXml->AddAttribute(SegAttrName(after_handle), val);
        val = wxString::Format("%d", polyLineSizes[after_handle+1]);
        ModelXml->DeleteAttribute(SegAttrName(after_handle+1));
        ModelXml->AddAttribute(SegAttrName(after_handle+1), val);
    }
    GetModelScreenLocation().InsertHandle(after_handle, zoom, scale);
}

void PolyLineModel::DeleteHandle(int handle_) {
    // handle is offset by 1 due to the center handle at 0
    int handle = handle_ - 1;
    if( polyLineSizes.size() > handle ) {
        ModelXml->DeleteAttribute(SegAttrName(handle));
        for (int x = handle; x < num_segments-1; ++x) {
            std::string val = ModelXml->GetAttribute(SegAttrName(x+1)).ToStdString();
            if (val == "") {
                val = wxString::Format("%d", polyLineSizes[x+1]);
            }
            ModelXml->AddAttribute(SegAttrName(x), val);
            ModelXml->DeleteAttribute(SegAttrName(x+1));
        }
        polyLineSizes.erase(polyLineSizes.begin() + handle);
    }
    else {
        ModelXml->DeleteAttribute(SegAttrName(handle-1));
    }
    GetModelScreenLocation().DeleteHandle(handle);
}

void PolyLineModel::SetSegsCollapsed(bool collapsed)
{
    segs_collapsed = collapsed;

    // We have to add it to the xml so it survives reconstruction
    GetModelXml()->DeleteAttribute("SegsExpanded");
    if (segs_collapsed)
    {
        GetModelXml()->AddAttribute("SegsExpanded", "TRUE");
    }
    else
    {
        GetModelXml()->AddAttribute("SegsExpanded", "FALSE");
    }
}

void PolyLineModel::InitModel()
{
    _alternateNodes = (ModelXml->GetAttribute("AlternateNodes", "false") == "true");
    wxString dropPattern = GetModelXml()->GetAttribute("DropPattern", "1");
    wxArrayString pat = wxSplit(dropPattern, ',');

    segs_collapsed = GetModelXml()->GetAttribute("SegsCollapsed", "TRUE") == "FALSE";

    _strings = wxAtoi(ModelXml->GetAttribute("PolyStrings", "1"));

    // parse drop sizes
    std::vector<int> dropSizes;
    unsigned int maxH = 0;
    for (int x = 0; x < pat.size(); x++) {
        int pat_size = wxAtoi(pat[x]);
        if( pat_size == 0 ) {
            pat_size = 1;
        }
        dropSizes.push_back(pat_size);
        maxH = std::max(maxH, (unsigned int)std::abs(dropSizes[x]));
    }
    if (dropSizes.size() == 0) {
        dropSizes.push_back(5);
    }

    // establish light and segment counts
    int numLights = 0;
    int num_points = wxAtoi(ModelXml->GetAttribute("NumPoints", "2"));

    if (num_points < 2) {
        // This is not good ... so add in a second point
        num_points = 2;
    }

    num_segments = num_points - 1;
    hasIndivSeg = ModelXml->GetAttribute("IndivSegs", "0") == "1";
    numDropPoints = 0;
    size_t lights_per_node = GetCoordCount(0);

    // setup number of lights per line segment
    unsigned int drop_index = 0;
    polyLineSizes.resize(num_segments);
    polyLineSegDropSizes.resize(num_segments);
    polyLeadOffset.resize(num_segments);
    polyTrailOffset.resize(num_segments);
    polyGapSize.resize(num_segments);
    if (hasIndivSeg) {
        parm1 = SingleNode ? 1 : num_segments;
        for (int x = 0; x < num_segments; x++) {
            wxString val = ModelXml->GetAttribute(SegAttrName(x));
            if (val == "") {
                val = "0";
                ModelXml->DeleteAttribute(SegAttrName(x));
                ModelXml->AddAttribute(SegAttrName(x), val);
            }
            int num_drop_points_this_segment = wxAtoi(val);
            unsigned int drop_lights_this_segment = 0;
            for (size_t z = 0; z < num_drop_points_this_segment; z++) {
                drop_lights_this_segment += std::abs(dropSizes[drop_index++]);
                drop_index %= dropSizes.size();
            }
            numLights += drop_lights_this_segment;
            numDropPoints += num_drop_points_this_segment;
            polyLineSizes[x] = num_drop_points_this_segment;
            polyLineSegDropSizes[x] = drop_lights_this_segment;
        }
        parm2 = numLights;
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%ld", parm2));
        for (int x = 0; x <= num_segments; x++) {
            wxString val = ModelXml->GetAttribute(CornerAttrName(x));
            if( x == 0 ) {
                polyLeadOffset[x] = (val == "Leading Segment" ? 1.0 : val == "Trailing Segment" ? 0.0 : 0.5);
            } else if( x == num_segments ) {
                polyTrailOffset[x-1] = (val == "Leading Segment" ? 0.0 : val == "Trailing Segment" ? 1.0 : 0.5);
            } else {
                polyTrailOffset[x-1] = (val == "Leading Segment" ? 0.0 : val == "Trailing Segment" ? 1.0 : 0.5);
                polyLeadOffset[x] = (val == "Leading Segment" ? 1.0 : val == "Trailing Segment" ? 0.0 : 0.5);
            }
        }
        for (int x = 0; x < num_segments; x++) {
            float num_gaps = polyLeadOffset[x] + polyTrailOffset[x] + (float(polyLineSizes[x]) * (float)lights_per_node) - 1.0f;
            polyGapSize[x] = float(polyLineSizes[x]) / num_gaps;
        }
    }
    else {
        parm1 = 1;
        int lights = parm2;
        while (lights > 0) {
            unsigned int lights_this_drop = std::abs(dropSizes[drop_index++]);
            numLights += lights_this_drop;
            drop_index %= dropSizes.size();
            numDropPoints++;
            lights -= lights_this_drop;
        }
        if (numLights != parm2) {
            parm2 = numLights;
            ModelXml->DeleteAttribute("parm2");
            ModelXml->AddAttribute("parm2", wxString::Format("%ld", parm2));
        }
    }

    // reset node information
    Nodes.clear();
    SetNodeCount(1, numLights, rgbOrder);

    if (!SingleNode) {
        if (parm3 > 1) {
            for (size_t x = 0; x < Nodes.size(); x++) {
                Nodes[x]->Coords.resize(parm3);
            }
        }
    }

    // process our own start channels
    drop_index = 0;
    std::string tempstr = ModelXml->GetAttribute("Advanced", "0").ToStdString();
    bool HasIndividualStartChans = tempstr == "1";
    if (HasIndividualStartChans && !SingleNode) {
        std::string dependsonmodel;
        int StartChannel = GetNumberFromChannelString(ModelXml->GetAttribute("StartChannel", "1").ToStdString(), CouldComputeStartChannel, dependsonmodel);
        stringStartChan.clear();
        stringStartChan.resize(num_segments);
        for (int i = 0; i < num_segments; i++) {
            tempstr = StartChanAttrName(i);
            if (!zeroBased && ModelXml->HasAttribute(tempstr)) {
                bool b = false;
                stringStartChan[i] = GetNumberFromChannelString(ModelXml->GetAttribute(tempstr, "1").ToStdString(), b, dependsonmodel) - 1;
                CouldComputeStartChannel &= b;
            }
            else {
                stringStartChan[i] = (zeroBased ? 0 : StartChannel - 1) + polyLineSegDropSizes[i] * GetNodeChannelCount(StringType);
            }
        }
    }

    // fix the string numbers for each node since model is non-standard
    size_t idx = 0;
    if (HasIndividualStartChans && hasIndivSeg && !SingleNode) {
        for (int x = 0; x < num_segments; x++) {
            for (int n = 0; n < polyLineSegDropSizes[x]; ++n) {
                Nodes[idx++]->StringNum = x;
            }
        }
    } else if ( _strings > 1 ) {
        wxString nm = StartNodeAttrName(0);
        int node_count = GetNodeCount();
        bool hasIndivNodes = ModelXml->HasAttribute(nm);
        for (int s = 0; s < _strings; ++s) {
            int v1 = 0;
            int v2 = node_count;
            if (hasIndivNodes) {
                wxString nm = StartNodeAttrName(s);
                std::string val = ModelXml->GetAttribute(nm, "").ToStdString();
                v1 = wxAtoi(val)-1;
                if (s < _strings - 1) { // not last string
                    nm = StartNodeAttrName(s + 1);
                    val = ModelXml->GetAttribute(nm, "").ToStdString();
                    v2 = wxAtoi(val)-1;
                }
            } else {
                v1 = wxAtoi(ComputeStringStartNode(s))-1;
                if (s < _strings - 1) { // not last string
                    v2 = wxAtoi(ComputeStringStartNode(s + 1))-1;
                }
            }
            if (!IsLtoR) {
                for (int n = v1; n < v2; ++n) {
                    Nodes[node_count-n-1]->StringNum = s;
                }
            } else {
                for (int n = v1; n < v2; ++n) {
                    Nodes[n]->StringNum = s;
                }
            }
        }
    }

    // read in the point data from xml
    std::vector<xlPolyPoint> pPos(num_points);
    wxString point_data = ModelXml->GetAttribute("PointData", "0.0, 0.0, 0.0, 0.0, 0.0, 0.0");
    wxArrayString point_array = wxSplit(point_data, ',');
    while (point_array.size() < num_points * 3) point_array.push_back("0.0");
    for (int i = 0; i < num_points; ++i) {
        pPos[i].x = wxAtof(point_array[i * 3]);
        pPos[i].y = wxAtof(point_array[i * 3 + 1]);
        pPos[i].z = wxAtof(point_array[i * 3 + 2]);
        pPos[i].has_curve = false;
        pPos[i].curve = nullptr;
    }
    wxString cpoint_data = ModelXml->GetAttribute("cPointData", "");
    wxArrayString cpoint_array = wxSplit(cpoint_data, ',');
    int num_curves = cpoint_array.size() / 7;
    glm::vec3 def_scaling(100.0f, 100.0f, 100.0f);
    glm::vec3 def_pos(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < num_curves; ++i) {
        int seg_num = wxAtoi(cpoint_array[i * 7]);
        pPos[seg_num].has_curve = true;
        pPos[seg_num].curve = new BezierCurveCubic3D();
        pPos[seg_num].curve->set_p0(pPos[seg_num].x, pPos[seg_num].y, pPos[seg_num].z);
        pPos[seg_num].curve->set_p1(pPos[seg_num + 1].x, pPos[seg_num + 1].y, pPos[seg_num + 1].z);
        pPos[seg_num].curve->set_cp0(wxAtof(cpoint_array[i * 7 + 1]), wxAtof(cpoint_array[i * 7 + 2]), wxAtof(cpoint_array[i * 7 + 3]));
        pPos[seg_num].curve->set_cp1(wxAtof(cpoint_array[i * 7 + 4]), wxAtof(cpoint_array[i * 7 + 5]), wxAtof(cpoint_array[i * 7 + 6]));
        pPos[seg_num].curve->SetPositioning(def_scaling, def_pos);
        pPos[seg_num].curve->UpdatePoints();
        pPos[seg_num].curve->UpdateMatrices();
    }

    // calculate segment lengths if we need to auto-distribute lights
    total_length = 0.0f;
    if (!hasIndivSeg) {
        for (int i = 0; i < num_points - 1; ++i) {
            if (pPos[i].has_curve) {
                total_length += pPos[i].curve->GetLength();
            }
            else {
                float length = std::sqrt((pPos[i + 1].z - pPos[i].z) * (pPos[i + 1].z - pPos[i].z) + (pPos[i + 1].y - pPos[i].y) * (pPos[i + 1].y - pPos[i].y) + (pPos[i + 1].x - pPos[i].x) * (pPos[i + 1].x - pPos[i].x));
                pPos[i].length = length;
                total_length += length;
            }
        }
    }

    // calculate min/max for the model
    float minX = 100000.0f;
    float minY = 100000.0f;
    float minZ = 100000.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    float maxZ = 0.0f;

    for (int i = 0; i < num_points; ++i) {
        if (pPos[i].x < minX) minX = pPos[i].x;
        if (pPos[i].y < minY) minY = pPos[i].y;
        if (pPos[i].z < minZ) minZ = pPos[i].z;
        if (pPos[i].x > maxX) maxX = pPos[i].x;
        if (pPos[i].y > maxY) maxY = pPos[i].y;
        if (pPos[i].z > maxZ) maxZ = pPos[i].z;
        if (pPos[i].has_curve) {
            pPos[i].curve->check_min_max(minX, maxX, minY, maxY, minZ, maxZ);
        }
    }
    float deltax = maxX - minX;
    float deltay = maxY - minY;
    float deltaz = maxZ - minZ;

    // normalize all points from 0.0 to 1.0 and create
    // a matrix for each line segment
    for (int i = 0; i < num_points - 1; ++i) {
        float x1p, y1p, z1p, x2p, y2p, z2p;
        if (deltax == 0.0f) {
            x1p = 0.0f;
            x2p = 0.0f;
        }
        else {
            x1p = (pPos[i].x - minX) / deltax;
            x2p = (pPos[i + 1].x - minX) / deltax;
        }
        if (deltay == 0.0f) {
            y1p = 0.0f;
            y2p = 0.0f;
        }
        else {
            y1p = (pPos[i].y - minY) / deltay;
            y2p = (pPos[i + 1].y - minY) / deltay;
        }
        if (deltaz == 0.0f) {
            z1p = 0.0f;
            z2p = 0.0f;
        }
        else {
            z1p = (pPos[i].z - minZ) / deltaz;
            z2p = (pPos[i + 1].z - minZ) / deltaz;
        }

        // where the model is just a line and has no curve then we want to draw it closer to its orientation ... the problem is without this
        // any line not perfectly flat is drawn at 45 degrees
        float scaley = 1.0f;
        float scalez = 1.0f;
        if (num_points == 2 && !pPos[i].has_curve) {
            scaley = 0.0f;
            scalez = 0.0f;
        }

        glm::vec3 pt1(x1p, y1p, z1p);
        glm::vec3 pt2(x2p, y2p, z2p);
        glm::vec3 a = pt2 - pt1;
        float scale = glm::length(a);
        glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector(a);
        glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scaley, scalez));
        glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x1p, y1p, z1p));
        glm::mat4 mat = translateMatrix * rotationMatrix * scalingMatrix;

        if (pPos[i].matrix != nullptr) {
            delete pPos[i].matrix;
        }
        pPos[i].matrix = new glm::mat4(mat);

        // update any curves
        if (pPos[i].has_curve) {
            pPos[i].curve->CreateNormalizedMatrix(minX, maxX, minY, maxY, minZ, maxZ);
        }
    }

    // define the buffer positions
    int chan = 0;
    int LastStringNum = -1;
    int ChanIncr = GetNodeChannelCount(StringType);
    int lights = numLights * (SingleNode ? 1 : parm3);
    int y = 0;
    drop_index = 0;
    int width = 0;
    int curNode = 0;
    int curCoord = 0;
    bool up = dropSizes[drop_index] < 0;
    int nodesInDrop = std::abs(dropSizes[drop_index]);
    int nodesInDropLast = nodesInDrop;
    while (lights) {
        if (curCoord >= Nodes[curNode]->Coords.size()) {
            curNode++;
            curCoord = 0;
            if (!SingleNode) {
                chan += ChanIncr;
            }
        }
        while (y >= std::abs(dropSizes[drop_index])) {
            width++;
            y = 0;
            drop_index++;
            if (drop_index >= dropSizes.size()) {
                drop_index = 0;
            }
            nodesInDrop = std::abs(dropSizes[drop_index]);
            if (!IsLtoR && !SingleNode && curCoord == 0) {
                chan -= ((nodesInDropLast + nodesInDrop) * GetNodeChannelCount(StringType));
            }
            nodesInDropLast = nodesInDrop;
            up = dropSizes[drop_index] < 0;
        }
        if (Nodes[curNode]->StringNum != LastStringNum) {
            LastStringNum = Nodes[curNode]->StringNum;
            chan = stringStartChan[LastStringNum];
            if (!IsLtoR && !SingleNode && curCoord == 0) {
                chan += (NodesPerString(LastStringNum) - nodesInDrop) * GetNodeChannelCount(StringType);
            }
        }
        Nodes[curNode]->ActChan = chan;
        Nodes[curNode]->Coords[curCoord].bufX = width;
        if (_alternateNodes) {
            if (y + 1 <= (nodesInDrop + 1) / 2) {
                if (up) {
                    Nodes[curNode]->Coords[curCoord].bufY = 2 * y;
                    Nodes[curNode]->Coords[curCoord].screenY = -1 * (maxH - 1) + (2 * y);
                }
                else {
                    Nodes[curNode]->Coords[curCoord].bufY = maxH - 1 - (2 * y);
                    Nodes[curNode]->Coords[curCoord].screenY = maxH - 1 - (2 * y);
                }
            }
            else {
                if (up) {
                    Nodes[curNode]->Coords[curCoord].bufY = (nodesInDrop - (y + 1)) * 2 + 1;
                    Nodes[curNode]->Coords[curCoord].screenY = -1 * (maxH - 1) + ((nodesInDrop - (y + 1)) * 2 + 1);
                }
                else {
                    Nodes[curNode]->Coords[curCoord].bufY = maxH - 1 - ((nodesInDrop - (y + 1)) * 2 + 1);
                    Nodes[curNode]->Coords[curCoord].screenY = maxH - 1 - ((nodesInDrop - (y + 1)) * 2 + 1);
                }
            }
        }
        else {
            if (up) {
                Nodes[curNode]->Coords[curCoord].bufY = y;
                Nodes[curNode]->Coords[curCoord].screenY = y;
            }
            else {
                Nodes[curNode]->Coords[curCoord].bufY = maxH - y - 1;
                Nodes[curNode]->Coords[curCoord].screenY = maxH - y - 1;
            }
        }

        Nodes[curNode]->Coords[curCoord].screenX = width;
        lights--;
        curCoord++;
        if( SingleNode || curCoord == parm3 ) {
            y++;
        }
    }

    SetBufferSize(maxH, SingleNode ? 1 : width + 1);
    screenLocation.SetRenderSize(1.0, maxH);

    height = wxAtof(GetModelXml()->GetAttribute("ModelHeight", "1.0"));
    double model_height = deltay;
    if (model_height < GetModelScreenLocation().GetRenderHt()) {
        model_height = GetModelScreenLocation().GetRenderHt();
    }
    float mheight = height * 10.0f / model_height;

    // place the nodes/coords along each line segment
    drop_index = 0;
    if (hasIndivSeg) {
        // distribute the lights as defined by the polysize string
        DistributeLightsAcrossIndivSegments( pPos, dropSizes, mheight, maxH );
    }
    else {
        // distribute the lights evenly across the line segments
        DistributeLightsEvenly( pPos, dropSizes, mheight, maxH, numLights );
    }

    // cleanup curves and matrices
    for (int i = 0; i < num_points; ++i) {
        if (pPos[i].has_curve) {
            delete pPos[i].curve;
            pPos[i].curve = nullptr;
        }
        if (pPos[i].matrix != nullptr) {
            delete pPos[i].matrix;
        }
    }
}

void PolyLineModel::DistributeLightsEvenly( const std::vector<xlPolyPoint>& pPos,
                                            const std::vector<int>&         dropSizes,
                                            const float&                    mheight,
                                            const int                       maxH,
                                            const int                       numLights )
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool using_icicles = maxH > 1;
    unsigned int drop_index = 0;
    size_t idx = 0;
    size_t seg_count = 0;
    int coords_per_node = Nodes[0].get()->Coords.size();
    float coord_offset = using_icicles ? 1.0f / (float)coords_per_node : 0.0f;
    int lights_to_distribute = SingleNode ? numLights : numLights * coords_per_node;
    float offset;
    if (!SingleNode) {
        offset = total_length / ((float)numDropPoints * (using_icicles ? 1.0f : (float)coords_per_node));
    } else {
        offset = total_length / (float)numDropPoints;
    }
    float current_pos = offset / 2.0f;
    size_t c = 0;
    int segment = 0;
    int sub_segment = 0;
    int last_seg_count = 0;
    float seg_start = current_pos;
    float segment_length = pPos[segment].has_curve ? pPos[segment].curve->GetSegLength(sub_segment) : pPos[segment].length;
    float seg_end = seg_start + segment_length;
    int xpos = 0;  // the horizontal position in the buffer
    for (int x = 0; x < polyLineSizes.size(); x++) {
        polyLineSizes[x] = 0;
        polyLineSegDropSizes[x] = 0;
    }
    for (size_t m = 0; m < lights_to_distribute;) {
        while (current_pos > seg_end) {
            sub_segment++;
            if (pPos[segment].has_curve && (sub_segment < pPos[segment].curve->GetNumSegments())) {
                seg_start = seg_end;
                segment_length = pPos[segment].curve->GetSegLength(sub_segment);
                seg_end = seg_start + segment_length;
            }
            else {
                if (segment == polyLineSizes.size() - 1) {
                    // cant increase segment ... so just fudge the segment end
                    seg_end += 0.0001f;
                }
                else {
                    sub_segment = 0;
                    polyLineSizes[segment] = seg_count - last_seg_count;
                    last_seg_count = seg_count;
                    segment++;
                    seg_start = seg_end;
                    segment_length = pPos[segment].has_curve ? pPos[segment].curve->GetSegLength(sub_segment) : pPos[segment].length;
                    seg_end = seg_start + segment_length;
                }
            }
        }
        glm::vec3 v;
        float pos = (current_pos - seg_start) / segment_length;
        if (pPos[segment].has_curve) {
            v = glm::vec3(*pPos[segment].curve->GetMatrix(sub_segment) * glm::vec4(pos, 0, 0, 1));
        }
        else {
            v = glm::vec3(*pPos[segment].matrix * glm::vec4(pos, 0, 0, 1));
        }
        bool up = dropSizes[drop_index] < 0;
        unsigned int drops_this_node = std::abs(dropSizes[drop_index++]);
        for (size_t z = 0; z < drops_this_node; z++) {
            if (SingleNode) {
                Nodes[0]->Coords[c].screenX = v.x;
                if (up) {
                    Nodes[0]->Coords[c].screenY = v.y + (z + ((float)c * coord_offset)) * mheight;
                }
                else {
                    Nodes[0]->Coords[c].screenY = v.y - (z + ((float)c * coord_offset)) * mheight;
                }
                Nodes[0]->Coords[c].screenZ = v.z;
                m++;
                c++;
            }
            else {
                int node = -1;
                if (up) {
                    node = FindNodeAtXY(xpos, z);
                }
                else {
                    node = FindNodeAtXY(xpos, maxH - z - 1);
                }
                if (node == -1) {
                    logger_base.error("Polyline buffer x,y %d, %d not found.", xpos, maxH - z - 1);
                }
                else {
                    size_t current_coord = c;
                    for ( using_icicles ? c = 0 : c; using_icicles ? c < coords_per_node : c == current_coord; ++c) {
                        Nodes[node]->Coords[c].screenX = v.x;
                        if (up) {
                            Nodes[node]->Coords[c].screenY = v.y + (z + ((float)c * coord_offset)) * mheight;
                        }
                        else {
                            Nodes[node]->Coords[c].screenY = v.y - (z + ((float)c * coord_offset)) * mheight;
                        }
                        Nodes[node]->Coords[c].screenZ = v.z;
                        m++;
                    }
                }
            }
            
            if (SingleNode) {
                seg_count++;
            } else {
                if (c == coords_per_node ) {
                    c = 0;
                    idx++;
                    seg_count++;
                }
            }
        }
        polyLineSegDropSizes[segment] += drops_this_node;
        drop_index %= dropSizes.size();
        current_pos += offset;
        if( c == 0 ) {
            xpos++;
        }
    }
    polyLineSizes[segment] = seg_count - last_seg_count;
}

void PolyLineModel::DistributeLightsAcrossIndivSegments( const std::vector<xlPolyPoint>& pPos,
                                                         const std::vector<int>&         dropSizes,
                                                         const float&                    mheight,
                                                         const int                       maxH )
{
    unsigned int drop_index = 0;
    size_t idx = 0;
    int xpos = 0;  // the horizontal position in the buffer
    for (size_t m = 0; m < num_segments; m++) {
        DistributeLightsAcrossSegment(m, idx, pPos, dropSizes, drop_index, mheight, xpos, maxH, pPos[m].has_curve);
    }
}

void PolyLineModel::DistributeLightsAcrossSegment( const int                       segment,
                                                         size_t&                   idx,
                                                   const std::vector<xlPolyPoint>& pPos,
                                                   const std::vector<int>&         dropSizes,
                                                         unsigned int&             drop_index,
                                                   const float&                    mheight,
                                                         int&                      xpos,
                                                   const int                       maxH,
                                                   const bool                      isCurve )
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // distribute the lights evenly across the line segments
    bool using_icicles = maxH > 1;
    int coords_per_node = Nodes[0].get()->Coords.size();
    float coord_offset = using_icicles ? 1.0f / (float)coords_per_node : 0.0f;
    int lights = 0;
    
    // get the total number of nodes including icicle drops for this segment
    unsigned int idrop = drop_index;
    for (size_t i = 0; i < polyLineSizes[segment]; ++i) {
        unsigned int drops_this_node = std::abs(dropSizes[idrop]);
        lights += drops_this_node;
        idrop++;
        idrop %= dropSizes.size();
    }
    
    int lights_to_distribute = SingleNode ? lights : lights * coords_per_node;
    float total_length = isCurve ? pPos[segment].curve->GetLength() : polyLineSizes[segment];
    
    float num_gaps;
    if (using_icicles) {
        num_gaps = polyLeadOffset[segment] + polyTrailOffset[segment] + polyLineSizes[segment] - 1.0f;
    } else {
        num_gaps = polyLeadOffset[segment] + polyTrailOffset[segment] + float(lights_to_distribute) - 1.0f;
    }
    float offset = total_length / num_gaps;
    float current_pos = polyLeadOffset[segment] * offset;
    size_t c = 0;
    int sub_segment = 0;
    float seg_start = 0;
    float segment_length = isCurve ? pPos[segment].curve->GetSegLength(sub_segment) : polyLineSizes[segment];
    float seg_end = seg_start + segment_length;
    for (size_t m = 0; m < lights_to_distribute;) {
        bool up = dropSizes[drop_index] < 0;
        unsigned int drops_this_node = std::abs(dropSizes[drop_index]);
        while (current_pos > seg_end) {
            sub_segment++;
            seg_start = seg_end;
            segment_length = isCurve ? pPos[segment].curve->GetSegLength(sub_segment) : 1.0f;
            seg_end = seg_start + segment_length;
        }
        glm::vec3 v;
        if (isCurve) {
            v = glm::vec3(*pPos[segment].curve->GetMatrix(sub_segment) * glm::vec4((current_pos - seg_start) / segment_length, 0, 0, 1));
        } else {
            v = glm::vec3(*pPos[segment].matrix * glm::vec4(current_pos / polyLineSizes[segment], 0, 0, 1));
        }
        if (SingleNode) {
            for (size_t z = 0; z < drops_this_node; z++) {
                Nodes[0]->Coords[idx].screenX = v.x;
                if (up) {
                    Nodes[0]->Coords[idx].screenY = v.y + (z + ((float)c * coord_offset)) * mheight;
                }
                else {
                    Nodes[0]->Coords[idx].screenY = v.y - (z + ((float)c * coord_offset)) * mheight;
                }
                Nodes[0]->Coords[idx].screenZ = v.z;
                idx++;
                m++;
            }
            drop_index++;
            drop_index %= dropSizes.size();
        }
        else {
            for (size_t z = 0; z < drops_this_node; z++) {
                auto node = FindNodeAtXY(xpos, maxH - z - 1);
                if (node == -1) {
                    logger_base.error("Polyline buffer x,y %d, %d not found.", xpos, maxH - z - 1);
                }
                else {
                    size_t current_coord = c;
                    for ( using_icicles ? c = 0 : c; using_icicles ? c < coords_per_node : c == current_coord; ++c) {
                        Nodes[node]->Coords[c].screenX = v.x;
                        if (up) {
                            Nodes[node]->Coords[c].screenY = v.y + (z + ((float)c * coord_offset)) * mheight;
                        }
                        else {
                            Nodes[node]->Coords[c].screenY = v.y - (z + ((float)c * coord_offset)) * mheight;
                        }
                        Nodes[node]->Coords[c].screenZ = v.z;
                        m++;
                    }
                }
                if (c == coords_per_node ) {
                    c = 0;
                }
            }
            drop_index++;
            drop_index %= dropSizes.size();
        }
        current_pos += offset;
        if ( c == 0 ) {
            xpos++;
        }
    }
}

static const char* LEFT_RIGHT_VALUES[] = { "Green Square", "Blue Square" };
static wxPGChoices LEFT_RIGHT(wxArrayString(2, LEFT_RIGHT_VALUES)); 

void PolyLineModel::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    wxPGProperty* p;
    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("# Lights", "PolyLineNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        p->Enable(!hasIndivSeg);
    }
    else {
        p = grid->Append(new wxUIntProperty("# Nodes", "PolyLineNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        p->Enable(!hasIndivSeg);

        p = grid->Append(new wxUIntProperty("Lights/Node", "PolyLineLights", parm3));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 300);
        p->SetEditor("SpinCtrl");
    }

    p = grid->Append(new wxUIntProperty("Strings", "PolyLineStrings", _strings));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 48);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (_strings == 1) {
        // cant set start node
    } else {
        wxString nm = StartNodeAttrName(0);
        bool hasIndivNodes = ModelXml->HasAttribute(nm);

        p = grid->Append(new wxBoolProperty("Indiv Start Nodes", "ModelIndividualStartNodes", hasIndivNodes));
        p->SetAttribute("UseCheckbox", true);

        wxPGProperty* psn = grid->AppendIn(p, new wxUIntProperty(nm, nm, wxAtoi(ModelXml->GetAttribute(nm, "1"))));
        psn->SetAttribute("Min", 1);
        psn->SetAttribute("Max", (int)GetNodeCount());
        psn->SetEditor("SpinCtrl");

        if (hasIndivNodes) {
            int c = _strings;
            for (int x = 0; x < c; x++) {
                nm = StartNodeAttrName(x);
                std::string val = ModelXml->GetAttribute(nm, "").ToStdString();
                if (val == "") {
                    val = ComputeStringStartNode(x);
                    ModelXml->DeleteAttribute(nm);
                    ModelXml->AddAttribute(nm, val);
                }
                int v = wxAtoi(val);
                if (v < 1)
                    v = 1;
                if (v > NodesPerString())
                    v = NodesPerString();
                if (x == 0) {
                    psn->SetValue(v);
                } else {
                    grid->AppendIn(p, new wxUIntProperty(nm, nm, v));
                }
            }
        } else {
            psn->Enable(false);
        }
    }

    grid->Append(new wxEnumProperty("Starting Location", "PolyLineStart", LEFT_RIGHT, IsLtoR ? 0 : 1));

    p = grid->Append(new wxDropPatternProperty("Drop Pattern", "IciclesDrops", GetModelXml()->GetAttribute("DropPattern", "1")));

    p = grid->Append(new wxBoolProperty("Alternate Drop Nodes", "AlternateNodes", _alternateNodes));
    p->SetEditor("CheckBox");

    p = grid->Append(new wxFloatProperty("Height", "ModelHeight", height));
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("Indiv Segments", "ModelIndividualSegments", hasIndivSeg));
    p->SetAttribute("UseCheckbox", true);
    p->Enable(num_segments > 1);
    if (hasIndivSeg) {
        for (int x = 0; x < num_segments; x++) {
            std::string val = ModelXml->GetAttribute(SegAttrName(x)).ToStdString();
            if (val == "") {
                //TODO this needs to be improved like the model individual start channels code
                val = wxString::Format("%d", polyLineSizes[x]);
                ModelXml->DeleteAttribute(SegAttrName(x));
                ModelXml->AddAttribute(SegAttrName(x), val);
            }
            wxString nm = wxString::Format("Segment %d", x + 1);
            grid->AppendIn(p, new wxUIntProperty(nm, SegAttrName(x), wxAtoi(ModelXml->GetAttribute(SegAttrName(x), ""))));
        }
        if (segs_collapsed) {
            grid->Collapse(p);
        }
        
        p = grid->Append(new wxStringProperty("Corner Settings", "PolyCornerProperties", ""));
        for (int x = 0; x < num_segments + 1; x++) {
            std::string val = ModelXml->GetAttribute(CornerAttrName(x)).ToStdString();
            if (val == "") {
                val = "Neither";
                ModelXml->DeleteAttribute(CornerAttrName(x));
                ModelXml->AddAttribute(CornerAttrName(x), val);
            }
            wxString nm = wxString::Format("Corner %d", x + 1);
            grid->AppendIn(p, new wxEnumProperty(nm, CornerAttrName(x), POLY_CORNERS, val == "Leading Segment" ? 0 : val == "Trailing Segment" ? 1 : 2 ));
       }
    }
    else {
        for (int x = 0; x < 100; x++) {
            ModelXml->DeleteAttribute(SegAttrName(x));
            ModelXml->DeleteAttribute(CornerAttrName(x));
        }
        // If we dont have individual segments ... then we dont have individual start channels
        ModelXml->DeleteAttribute("Advanced");
    }


}

int PolyLineModel::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if ("PolyLineNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        wxPGProperty* sp = grid->GetPropertyByLabel("# Nodes");
        if (sp == nullptr) {
            sp = grid->GetPropertyByLabel("# Lights");
        }
        sp->SetValueFromInt((int)event.GetPropertyValue().GetLong());
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::PolyLineNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::PolyLineNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::PolyLineNodes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::PolyLineNodes");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "PolyLineModel::OnPropertyGridChange::PolyLineNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "PolyLineModel::OnPropertyGridChange::PolyLineNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::OnPropertyGridChange::PolyLineNodes");
        return 0;
    }
    else if ("PolyLineLights" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::PolyLineLights");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::PolyLineLights");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::PolyLineLights");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::PolyLineLights");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "PolyLineModel::OnPropertyGridChange::PolyLineLights");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "PolyLineModel::OnPropertyGridChange::PolyLineLights");
        return 0;
    }
    else if ("PolyLineStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 ? "L" : "R");
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::PolyLineStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::PolyLineStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::PolyLineStart");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::PolyLineStart");
        return 0;
    }
    else if (event.GetPropertyName() == "ModelIndividualSegments") {
        ModelXml->DeleteAttribute("IndivSegs");
        if (event.GetValue().GetBool()) {
            hasIndivSeg = true;
            SetSegsCollapsed(false);
            ModelXml->AddAttribute("IndivSegs", "1");
            int count = polyLineSizes.size();
            for (int x = 0; x < count; x++) {
                if (ModelXml->GetAttribute(SegAttrName(x)) == "") {
                    ModelXml->DeleteAttribute(SegAttrName(x));
                    //TODO This needs to be immproved like the individual start channels code in model
                    ModelXml->AddAttribute(SegAttrName(x), wxString::Format("%d", polyLineSizes[x]));
                }
            }
        }
        else {
            hasIndivSeg = false;
            for (int x = 0; x < 100; x++) {
                ModelXml->DeleteAttribute(SegAttrName(x));
            }
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments");
        return 0;
    }
    else if (event.GetPropertyName().StartsWith("ModelIndividualSegments.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());
        ModelXml->DeleteAttribute(str);
        ModelXml->AddAttribute(str, event.GetValue().GetString());
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
        return 0;
    }
    else if (event.GetPropertyName().StartsWith("PolyCornerProperties.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());
        ModelXml->DeleteAttribute(str);
        ModelXml->AddAttribute(str, POLY_CORNER_VALUES[event.GetPropertyValue().GetLong()]);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::PolyCornerProperties");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::PolyCornerProperties");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::PolyCornerProperties");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::PolyCornerProperties");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "PolyLineModel::OnPropertyGridChange::PolyCornerProperties");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "PolyLineModel::OnPropertyGridChange::PolyCornerProperties");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::OnPropertyGridChange::PolyCornerProperties");
        return 0;
    }
    else if ("PolyLineStrings" == event.GetPropertyName()) {
        int old_string_count = _strings;
        int new_string_count = event.GetValue().GetInteger();
        _strings = new_string_count;
        if (old_string_count != new_string_count) {
            wxString nm = StartNodeAttrName(0);
            bool hasIndivNodes = ModelXml->HasAttribute(nm);
            if (hasIndivNodes) {
                for (int x = 0; x < old_string_count; x++) {
                    wxString nm = StartNodeAttrName(x);
                    ModelXml->DeleteAttribute(nm);
                }
                for (int x = 0; x < new_string_count; x++) {
                    wxString nm = StartNodeAttrName(x);
                    ModelXml->AddAttribute(nm, ComputeStringStartNode(x));
                }
            }
        }
        ModelXml->DeleteAttribute("PolyStrings");
        ModelXml->AddAttribute("PolyStrings", wxString::Format("%d", _strings));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::PolyLineStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::PolyLineStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::PolyLineStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "PolyLineModel::OnPropertyGridChange::PolyLineStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::OnPropertyGridChange::PolyLineStrings");
        return 0;
    }
    else if (event.GetPropertyName() == "ModelIndividualStartNodes") {
        bool hasIndiv = event.GetValue().GetBool();
        for (int x = 0; x < _strings; x++) {
            wxString nm = StartNodeAttrName(x);
            ModelXml->DeleteAttribute(nm);
        }
        if (hasIndiv) {
            for (int x = 0; x < _strings; x++) {
                wxString nm = StartNodeAttrName(x);
                ModelXml->AddAttribute(nm, ComputeStringStartNode(x));
            }
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes");
        return 0;
    }
    else if (event.GetPropertyName().StartsWith("ModelIndividualStartNodes.PolyNode")) {
        wxString s = event.GetPropertyName().substr(strlen("ModelIndividualStartNodes.PolyNode"));
        int string = wxAtoi(s);

        wxString nm = StartNodeAttrName(string - 1);

        int value = event.GetValue().GetInteger();
        if (value < 1)
            value = 1;
        if (value > NodesPerString())
            value = NodesPerString();

        ModelXml->DeleteAttribute(nm);
        ModelXml->AddAttribute(nm, wxString::Format("%d", value));

        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        return 0;
    }
    else if (event.GetPropertyName() == "ModelIndividualStartChannels") {
        ModelXml->DeleteAttribute("Advanced");
        if (event.GetValue().GetBool()) {
            ModelXml->AddAttribute("Advanced", "1");
            int start_channel = 1;
            for (int x = 0; x < num_segments; x++) {
                if (ModelXml->GetAttribute(StartChanAttrName(x)) == "") {
                    ModelXml->DeleteAttribute(StartChanAttrName(x));
                    ModelXml->AddAttribute(StartChanAttrName(x), wxString::Format("%d", start_channel));
                }
                start_channel += polyLineSizes[x] * GetNodeChannelCount(StringType);
            }
        }
        else {
            for (int x = 0; x < num_segments; x++) {
                ModelXml->DeleteAttribute(StartChanAttrName(x));
            }
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartChannels");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartChannels");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartChannels");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartChannels");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartChannels");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartChannels");
        return 0;
    }
    else if ("IciclesDrops" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DropPattern");
        ModelXml->AddAttribute("DropPattern", event.GetPropertyValue().GetString());
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::OnPropertyGridChange::IciclesDrops");
        return 0;
    }
    else if ("AlternateNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("AlternateNodes");
        ModelXml->AddAttribute("AlternateNodes", event.GetPropertyValue().GetBool() ? "true" : "false");
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::AlternateNodes");
        return 0;
    } else if (!GetModelScreenLocation().IsLocked() && !IsFromBase() && "ModelHeight" == event.GetPropertyName()) {
        height = event.GetValue().GetDouble();
        if (std::abs(height) < 0.01f) {
            if (height < 0.0f) {
                height = -0.01f;
            }
            else {
                height = 0.01f;
            }
        }
        ModelXml->DeleteAttribute("ModelHeight");
        ModelXml->AddAttribute("ModelHeight", event.GetPropertyValue().GetString());
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::ModelHeight");
        return 0;
    }
        else if ((GetModelScreenLocation().IsLocked() || IsFromBase()) && "ModelHeight" == event.GetPropertyName())
        {
        event.Veto();
        return 0;
    }
    return Model::OnPropertyGridChange(grid, event);
}

std::string PolyLineModel::ComputeStringStartNode(int x) const
{
    if (x == 0)
        return "1";

    int strings = GetNumPhysicalStrings();
    int nodes = GetNodeCount();
    float nodesPerString = (float)nodes / (float)strings;

    return wxString::Format("%d", (int)(x * nodesPerString + 1)).ToStdString();
}

int PolyLineModel::NodesPerString() const
{
    return Model::NodesPerString();
}

 int PolyLineModel::NodesPerString(int string) const
{
     int num_nodes = 0;
     if (_strings == 1) {
        return NodesPerString();
     } else {
        if (SingleNode) {
            return 1;
        } else {
            wxString nm = StartNodeAttrName(0);
            bool hasIndivNodes = ModelXml->HasAttribute(nm);
            int v1 = 0;
            int v2 = 0;
            if (hasIndivNodes) {
                nm = StartNodeAttrName(string);
                std::string val = ModelXml->GetAttribute(nm, "").ToStdString();
                v1 = wxAtoi(val);
                if (string < _strings - 1) { // not last string
                    nm = StartNodeAttrName(string + 1);
                    val = ModelXml->GetAttribute(nm, "").ToStdString();
                    v2 = wxAtoi(val);
                }
            } else {
                v1 = wxAtoi(ComputeStringStartNode(string));
                if (string < _strings - 1) { // not last string
                    v2 = wxAtoi(ComputeStringStartNode(string + 1));
                }
            }
            if (string < _strings - 1) { // not last string
                num_nodes = v2 - v1;
            } else {
                num_nodes = GetNodeCount() - v1 + 1;
            }
        }
        int ts = GetSmartTs();
        if (ts <= 1) {
            return num_nodes;
        } else {
            return num_nodes * ts;
        }
     }
}

/* int PolyLineModel::NodesPerString(int string) const
{
    if (_strings == 1) {
        return NodesPerString();
    }

    int32_t lowestStartChannel = 2000000000;
    for (int i = 0; i < _strings; i++) {
        if (stringStartChan[i] < lowestStartChannel)
            lowestStartChannel = stringStartChan[i];
    }

    int32_t ss = stringStartChan[string];
    int32_t len = GetChanCount() - (ss - lowestStartChannel);
    for (int i = 0; i < _strings; i++) {
        if (i != string) {
            if (stringStartChan[i] > ss && len > stringStartChan[i] - ss) {
                len = stringStartChan[i] - ss;
            }
        }
    }
    return len / GetNodeChannelCount(StringType);
}*/

int PolyLineModel::OnPropertyGridSelection(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if (event.GetPropertyName().StartsWith("ModelIndividualSegments.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());
        str = str.SubString(3, str.length());
        int segment = wxAtoi(str) - 1;
        return segment;
    }
    else if (event.GetPropertyName().StartsWith("PolyCornerProperties.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());
        str = str.SubString(6, str.length());
        int segment = wxAtoi(str) - 1;
        return segment;
    }
    return -1;
}

void PolyLineModel::OnPropertyGridItemCollapsed(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if (event.GetPropertyName() == "ModelIndividualSegments") {
        SetSegsCollapsed(true);
    }
}

void PolyLineModel::OnPropertyGridItemExpanded(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if (event.GetPropertyName() == "ModelIndividualSegments") {
        SetSegsCollapsed(false);
    }
}

void PolyLineModel::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "polylinemodel") {
        wxString name = root->GetAttribute("name");
        wxString p1 = root->GetAttribute("parm1");
        wxString p2 = root->GetAttribute("parm2");
        wxString p3 = root->GetAttribute("parm3");
        wxString st = root->GetAttribute("StringType");
        wxString ps = root->GetAttribute("PixelSize");
        wxString t = root->GetAttribute("Transparency", "0");
        wxString mb = root->GetAttribute("ModelBrightness", "0");
        wxString a = root->GetAttribute("Antialias");
        wxString ss = root->GetAttribute("StartSide");
        wxString dir = root->GetAttribute("Dir");
        wxString sn = root->GetAttribute("StrandNames");
        wxString nn = root->GetAttribute("NodeNames");
        wxString v = root->GetAttribute("SourceVersion");
        wxString is = root->GetAttribute("IndivSegs");
        wxString pts = root->GetAttribute("NumPoints");
        wxString point_data = root->GetAttribute("PointData");
        wxString cpoint_data = root->GetAttribute("cPointData");
        wxString pc = root->GetAttribute("PixelCount");
        wxString pt = root->GetAttribute("PixelType");
        wxString psp = root->GetAttribute("PixelSpacing");

        // Add any model version conversion logic here
        // Source version will be the program version that created the custom model

        SetProperty("parm1", p1);
        SetProperty("parm2", p2);
        SetProperty("parm3", p3);
        SetProperty("StringType", st);
        SetProperty("PixelSize", ps);
        SetProperty("Transparency", t);
        SetProperty("ModelBrightness", mb);
        SetProperty("Antialias", a);
        SetProperty("StartSide", ss);
        SetProperty("Dir", dir);
        SetProperty("StrandNames", sn);
        SetProperty("NodeNames", nn);
        SetProperty("PixelCount", pc);
        SetProperty("PixelType", pt);
        SetProperty("PixelSpacing", psp);
        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        SetProperty("name", newname, true);

        ImportSuperStringColours(root);
        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y);

        int num_points = wxAtoi(pts);
        ModelXml->DeleteAttribute("NumPoints");
        ModelXml->AddAttribute("NumPoints", pts);

        // handle all the line segments
        ModelXml->DeleteAttribute("IndivSegs");
        if (is == "1") {
            ModelXml->AddAttribute("IndivSegs", "1");
            num_segments = num_points - 1;
            for (int x = 0; x < num_segments; x++) {
                ModelXml->DeleteAttribute(SegAttrName(x));
                wxString seg = root->GetAttribute(SegAttrName(x), "");
                // TODO this needs to be fixed like the individual start channel code in model
                int seg_length = wxAtoi(seg);
                ModelXml->AddAttribute(SegAttrName(x), wxString::Format("%d", seg_length));
            }
            for (int x = 0; x < num_segments-1; x++) {
                ModelXml->DeleteAttribute(CornerAttrName(x));
                wxString corner = root->GetAttribute(CornerAttrName(x), "Neither");
                SetProperty(CornerAttrName(x), corner);
            }
        }

        ModelXml->DeleteAttribute("PointData");
        ModelXml->DeleteAttribute("cPointData");

        ModelXml->AddAttribute("PointData", point_data);
        ModelXml->AddAttribute("cPointData", cpoint_data);

        GetModelScreenLocation().Read(ModelXml);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::ImportXlightsModel");
    } else {
        DisplayError("Failure loading PolyLine model file.");
    }
}

void PolyLineModel::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty())
        return;
    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened())
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
    wxString p1 = ModelXml->GetAttribute("parm1");
    wxString p2 = ModelXml->GetAttribute("parm2");
    wxString p3 = ModelXml->GetAttribute("parm3");
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency", "0");
    wxString mb = ModelXml->GetAttribute("ModelBrightness", "0");
    wxString a = ModelXml->GetAttribute("Antialias");
    wxString ss = ModelXml->GetAttribute("StartSide");
    wxString dir = ModelXml->GetAttribute("Dir");
    wxString sn = ModelXml->GetAttribute("StrandNames");
    wxString nn = ModelXml->GetAttribute("NodeNames");
    wxString is = ModelXml->GetAttribute("IndivSegs");
    wxString pts = ModelXml->GetAttribute("NumPoints");
    NormalizePointData();
    wxString point_data = ModelXml->GetAttribute("PointData");
    wxString cpoint_data = ModelXml->GetAttribute("cPointData");
    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<polylinemodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%s\" ", p1));
    f.Write(wxString::Format("parm2=\"%s\" ", p2));
    f.Write(wxString::Format("parm3=\"%s\" ", p3));
    f.Write(wxString::Format("StringType=\"%s\" ", st));
    f.Write(wxString::Format("Transparency=\"%s\" ", t));
    f.Write(wxString::Format("PixelSize=\"%s\" ", ps));
    f.Write(wxString::Format("ModelBrightness=\"%s\" ", mb));
    f.Write(wxString::Format("Antialias=\"%s\" ", a));
    f.Write(wxString::Format("StartSide=\"%s\" ", ss));
    f.Write(wxString::Format("Dir=\"%s\" ", dir));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
    f.Write(wxString::Format("IndivSegs=\"%s\" ", is));
    f.Write(wxString::Format("NumPoints=\"%s\" ", pts));
    int count = wxAtoi(pts);
    for (int x = 0; x < count; x++) {
        wxString seg = ModelXml->GetAttribute(SegAttrName(x), "");
        f.Write(wxString::Format("%s=\"%s\" ", SegAttrName(x), seg));
    }
    for (int x = 0; x < count-1; x++) {
        wxString corner = ModelXml->GetAttribute(CornerAttrName(x), "Neither");
        f.Write(wxString::Format("%s=\"%s\" ", CornerAttrName(x), corner));
    }
    f.Write(wxString::Format("PointData=\"%s\" ", point_data));
    f.Write(wxString::Format("cPointData=\"%s\" ", cpoint_data));
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(ExportSuperStringColors());
    f.Write(" >\n");
    wxString state = SerialiseState();
    if (state != "") {
        f.Write(state);
    }
    wxString submodel = SerialiseSubmodel();
    if (submodel != "") {
        f.Write(submodel);
    }
    wxString groups = SerialiseGroups();
    if (groups != "") {
        f.Write(groups);
    }
    //ExportDimensions(f);
    f.Write("</polylinemodel>");
    f.Close();
}

void PolyLineModel::NormalizePointData()
{
    // read in the point data from xml
    int num_points = wxAtoi(ModelXml->GetAttribute("NumPoints"));
    if (num_points < 2)
        num_points = 2;
    std::vector<xlPolyPoint> pPos(num_points);
    wxString point_data = ModelXml->GetAttribute("PointData");
    wxArrayString point_array = wxSplit(point_data, ',');
    while (point_array.size() < num_points * 3)
        point_array.push_back("0.0");
    for (int i = 0; i < num_points; ++i) {
        pPos[i].x = wxAtof(point_array[i * 3]);
        pPos[i].y = wxAtof(point_array[i * 3 + 1]);
        pPos[i].z = wxAtof(point_array[i * 3 + 2]);
        pPos[i].has_curve = false;
        pPos[i].curve = nullptr;
    }
    wxString cpoint_data = ModelXml->GetAttribute("cPointData", "");
    wxArrayString cpoint_array = wxSplit(cpoint_data, ',');
    glm::vec3 def_scaling(100.0f, 100.0f, 100.0f);
    glm::vec3 def_pos(0.0f, 0.0f, 0.0f);
    int num_curves = cpoint_array.size() / 7;
    for (int i = 0; i < num_curves; ++i) {
        int seg_num = wxAtoi(cpoint_array[i * 7]);
        pPos[seg_num].has_curve = true;
        pPos[seg_num].curve = new BezierCurveCubic3D();
        pPos[seg_num].curve->set_p0(pPos[seg_num].x, pPos[seg_num].y, pPos[seg_num].z);
        pPos[seg_num].curve->set_p1(pPos[seg_num + 1].x, pPos[seg_num + 1].y, pPos[seg_num + 1].z);
        pPos[seg_num].curve->set_cp0(wxAtof(cpoint_array[i * 7 + 1]), wxAtof(cpoint_array[i * 7 + 2]), wxAtof(cpoint_array[i * 7 + 3]));
        pPos[seg_num].curve->set_cp1(wxAtof(cpoint_array[i * 7 + 4]), wxAtof(cpoint_array[i * 7 + 5]), wxAtof(cpoint_array[i * 7 + 6]));
        pPos[seg_num].curve->SetPositioning(def_scaling, def_pos);
        pPos[seg_num].curve->UpdatePoints();
    }

    float minX = 100000.0f;
    float minY = 100000.0f;
    float minZ = 100000.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    float maxZ = 0.0f;

    for (int i = 0; i < num_points; ++i) {
        if (pPos[i].x < minX)
            minX = pPos[i].x;
        if (pPos[i].y < minY)
            minY = pPos[i].y;
        if (pPos[i].z < minZ)
            minZ = pPos[i].z;
        if (pPos[i].x > maxX)
            maxX = pPos[i].x;
        if (pPos[i].y > maxY)
            maxY = pPos[i].y;
        if (pPos[i].z > maxZ)
            maxZ = pPos[i].z;
        if (pPos[i].has_curve) {
            pPos[i].curve->check_min_max(minX, maxX, minY, maxY, minZ, maxZ);
        }
    }
    float deltax = maxX - minX;
    float deltay = maxY - minY;
    float deltaz = maxZ - minZ;

    // normalize all the point data
    for (int i = 0; i < num_points; ++i) {
        if (deltax == 0.0f) {
            pPos[i].x = 0.0f;
        } else {
            pPos[i].x = (pPos[i].x - minX) / deltax;
        }
        if (deltay == 0.0f) {
            pPos[i].y = 0.0f;
        } else {
            pPos[i].y = (pPos[i].y - minY) / deltay;
        }
        if (deltaz == 0.0f) {
            pPos[i].z = 0.0f;
        } else {
            pPos[i].z = (pPos[i].z - minZ) / deltaz;
        }
        if (pPos[i].has_curve) {
            float cp0x = pPos[i].curve->get_cp0x();
            float cp0y = pPos[i].curve->get_cp0y();
            float cp0z = pPos[i].curve->get_cp0z();
            float cp1x = pPos[i].curve->get_cp1x();
            float cp1y = pPos[i].curve->get_cp1y();
            float cp1z = pPos[i].curve->get_cp1z();
            cp0x = (cp0x - minX) / deltax;
            cp0y = (cp0y - minY) / deltay;
            cp0z = (cp0z - minZ) / deltaz;
            cp1x = (cp1x - minX) / deltax;
            cp1y = (cp1y - minY) / deltay;
            cp1z = (cp1z - minZ) / deltaz;
            pPos[i].curve->set_cp0(cp0x, cp0y, cp0z);
            pPos[i].curve->set_cp1(cp1x, cp1y, cp1z);
        }
    }

    ModelXml->DeleteAttribute("PointData");
    ModelXml->DeleteAttribute("cPointData");
    point_data = "";
    for (int i = 0; i < num_points; ++i) {
        point_data += wxString::Format("%f,", pPos[i].x);
        point_data += wxString::Format("%f,", pPos[i].y);
        point_data += wxString::Format("%f", pPos[i].z);
        if (i != num_points - 1) {
            point_data += ",";
        }
    }
    cpoint_data = "";
    for (int i = 0; i < num_points; ++i) {
        if (pPos[i].has_curve) {
            cpoint_data += wxString::Format("%d,%f,%f,%f,%f,%f,%f,", i, pPos[i].curve->get_cp0x(), pPos[i].curve->get_cp0y(), pPos[i].curve->get_cp0z(),
                                            pPos[i].curve->get_cp1x(), pPos[i].curve->get_cp1y(), pPos[i].curve->get_cp1z());
        }
    }
    ModelXml->AddAttribute("PointData", point_data);
    ModelXml->AddAttribute("cPointData", cpoint_data);
}

// This is required because users dont need to have their start nodes for each string in ascending
// order ... this helps us name the strings correctly
int PolyLineModel::MapPhysicalStringToLogicalString(int string) const
{
    if (_strings == 1)
        return string;

    // FIXME
    // This is not very efficient ... n^2 algorithm ... but given most people will have a small
    // number of strings and it is super simple and only used on controller upload i am hoping
    // to get away with it

    std::vector<int> stringOrder;
    for (int curr = 0; curr < _strings; curr++) {
        int count = 0;
        for (int s = 0; s < _strings; s++) {
            if (stringStartChan[s] < stringStartChan[curr] && s != curr) {
                count++;
            }
        }
        stringOrder.push_back(count);
    }
    return stringOrder[string];
}

int PolyLineModel::GetNumPhysicalStrings() const
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

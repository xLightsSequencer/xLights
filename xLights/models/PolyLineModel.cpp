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

#include <log4cpp/Category.hh>

PolyLineModel::PolyLineModel(const ModelManager &manager) : ModelWithScreenLocation(manager) {
    parm1 = parm2 = parm3 = 0;
    segs_collapsed = true;
    num_segments = 0;
    total_length = 0.0f;
    height = 1.0f;
    hasIndivSeg = false;
    numDropPoints = 0;
}

PolyLineModel::PolyLineModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    segs_collapsed = true;
    PolyLineModel::SetFromXml(node, zeroBased);
}

PolyLineModel::~PolyLineModel()
{
    //dtor
}

std::vector<std::string> PolyLineModel::POLYLINE_BUFFER_STYLES;

const std::vector<std::string> &PolyLineModel::GetBufferStyles() const {
    struct Initializer {
        Initializer() {
            POLYLINE_BUFFER_STYLES = Model::DEFAULT_BUFFER_STYLES;
            POLYLINE_BUFFER_STYLES.push_back("Line Segments");
        }
    };
    static Initializer ListInitializationGuard;
    return POLYLINE_BUFFER_STYLES;
}

void PolyLineModel::InitRenderBufferNodes(const std::string &type, const std::string &camera,
                                          const std::string &transform,
                                          std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi) const {
    if (type == "Line Segments" && hasIndivSeg) {
        BufferHi = num_segments;
        BufferWi = 0;
        for (int x = 0; x < num_segments; x++) {
            int w = polyLineSizes[x];
            if (w > BufferWi) {
                BufferWi = w;
            }
        }
        for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
            newNodes.push_back(NodeBaseClassPtr(it->get()->clone()));
        }

        int idx = 0;
        for(size_t m=0; m<num_segments; m++) {
            int seg_idx = 0;
            int end_node = idx + polyLineSizes[m];
            float scale = (float)BufferWi / (float)polyLineSizes[m];
            for(size_t n=idx; n<end_node; n++) {
                newNodes[idx]->Coords.resize(SingleNode?parm2:parm3);
                size_t CoordCount=GetCoordCount(idx);
                int location = seg_idx * scale + scale / 2.0;
                for(size_t c=0; c < CoordCount; c++) {
                    newNodes[idx]->Coords[c].bufX = location;
                    newNodes[idx]->Coords[c].bufY = m;
                    newNodes[idx]->Coords[c].bufZ = 0;
                }
                idx++;
                seg_idx++;
            }
        }
        ApplyTransform(transform, newNodes, BufferWi, BufferHi);
    } else {
        Model::InitRenderBufferNodes(type, camera, transform, newNodes, BufferWi, BufferHi);
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
    bool HasIndividualStartChans=tempstr == "1";
    if( HasIndividualStartChans && !SingleNode ) {
        // if individual start channels defer to InitModel where we know all the segment length data
    } else {
        Model::SetStringStartChannels(zeroBased, NumberOfStrings, StartChannel, ChannelsPerString);
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

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _alternateNodes = (ModelXml->GetAttribute("AlternateNodes", "false") == "true");
    wxString dropPattern = GetModelXml()->GetAttribute("DropPattern", "1");
    wxArrayString pat = wxSplit(dropPattern, ',');

    segs_collapsed = GetModelXml()->GetAttribute("SegsCollapsed", "TRUE") == "FALSE";

    // parse drop sizes
    std::vector<int> dropSizes;
    unsigned int maxH = 0;
    for (int x = 0; x < pat.size(); x++) {
        dropSizes.push_back(wxAtoi(pat[x]));
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

    // setup number of lights per line segment
    unsigned int drop_index = 0;
    polyLineSizes.resize(num_segments);
    polyLineSegDropSizes.resize(num_segments);
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

    //if (num_points == 1) return;  // TODO:  Should we even allow this creation

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
        for (int x = 0; x < (SingleNode ? 1 : num_segments); x++) {
            for (int n = 0; n < polyLineSegDropSizes[x]; ++n) {
                Nodes[idx++]->StringNum = x;
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

        glm::vec3 pt1(x1p, y1p, z1p);
        glm::vec3 pt2(x2p, y2p, z2p);
        glm::vec3 a = pt2 - pt1;
        float scale = glm::length(a);
        glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector(a);
        glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale, 1.0f, 1.0f));
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
    int lights = numLights;
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
        }
        while (y >= std::abs(dropSizes[drop_index])) {
            width++;
            y = 0;
            drop_index++;
            if (drop_index >= dropSizes.size()) {
                drop_index = 0;
            }
            nodesInDrop = std::abs(dropSizes[drop_index]);
            if (!IsLtoR) {
                chan -= ((nodesInDropLast + nodesInDrop) * GetNodeChannelCount(StringType));
            }
            nodesInDropLast = nodesInDrop;
            up = dropSizes[drop_index] < 0;
        }
        if (Nodes[curNode]->StringNum != LastStringNum) {
            LastStringNum = Nodes[curNode]->StringNum;
            chan = stringStartChan[LastStringNum];
            if (!IsLtoR) {
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
        if (!SingleNode) {
            chan += ChanIncr;
        }
        y++;
        lights--;
        curCoord++;
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
    idx = 0;
    float loc_x;
    if (hasIndivSeg) {
        // distribute the lights as defined by the polysize string
        if (SingleNode) {
            int segment = 0;
            int seg_idx = 0;
            int count = 0;
            int num = Nodes[0].get()->Coords.size();
            float offset = 0.5f;
            if (num > 1) {
                offset -= 1.0f / (float)num;
            }
            size_t CoordCount = GetCoordCount(0);
            for (size_t c = 0; c < CoordCount;) {
                if (segment < polyLineSizes.size()) // this can be greater if coord count > segments * segment sizes
                {
                    if (num > 1) {
                        loc_x = seg_idx + offset + ((float)count / (float)num);
                        count++;
                    }
                    else {
                        loc_x = seg_idx + offset;
                    }

                    glm::vec3 v = glm::vec3(*pPos[segment].matrix * glm::vec4(loc_x / (float)polyLineSizes[segment], 0, 0, 1));

                    bool up = dropSizes[drop_index] < 0;
                    unsigned int drops_this_node = std::abs(dropSizes[drop_index++]);
                    for (size_t z = 0; z < drops_this_node && c < CoordCount; z++) {
                        Nodes[idx]->Coords[c].screenX = v.x;
                        if (up) {
                            Nodes[idx]->Coords[c].screenY = v.y + z * mheight;
                        }
                        else {
                            Nodes[idx]->Coords[c].screenY = v.y - z * mheight;
                        }
                        Nodes[idx]->Coords[c].screenZ = v.z;
                        c++;
                        if (!SingleNode) {
                            idx++;
                        }
                    }
                    drop_index %= dropSizes.size();
                    seg_idx++;
                    if (seg_idx >= polyLineSizes[segment]) {
                        segment++;
                        seg_idx = 0;
                        count = 0;
                        for (int x = segment; x < polyLineSizes.size(); ++x) {
                            if (polyLineSizes[x] == 0) {
                                segment++;
                            }
                            else {
                                break;
                            }
                        }
                    }
                    if (pPos[segment].has_curve) {
                        int xx = 0;
                        DistributeLightsAcrossCurveSegment(polyLineSizes[segment], segment, c, pPos, dropSizes, drop_index, mheight, xx, maxH);
                        c += std::abs(dropSizes[drop_index]);
                        segment++;
                        for (int x = segment; x < polyLineSizes.size(); ++x) {
                            if (polyLineSizes[x] == 0) {
                                segment++;
                            }
                            else {
                                break;
                            }
                        }
                    }
                }
                else {
                    logger_base.warn("Polyine segents * segment sizes < number of coordinates.");
                    break;
                }
            }
        }
        else {
            int xx = 0;
            for (size_t m = 0; m < num_segments; m++) {
                if (pPos[m].has_curve) {
                    DistributeLightsAcrossCurveSegment(polyLineSizes[m], m, idx, pPos, dropSizes, drop_index, mheight, xx, maxH);
                }
                else {
                    int seg_idx = 0;
                    for (size_t n = 0; n < polyLineSizes[m]; n++) {
                        int count = 0;
                        int num = Nodes[idx].get()->Coords.size();
                        float offset = 0.5f;
                        if (num > 1) {
                            offset -= 1.0f / (float)num;
                        }
                        size_t CoordCount = GetCoordCount(idx);
                        int x_pos = seg_idx;
                        bool up = dropSizes[drop_index] < 0;
                        unsigned int drops_this_node = std::abs(dropSizes[drop_index++]);
                        for (size_t c = 0; c < CoordCount; c++) {
                            if (num > 1) {
                                loc_x = x_pos + offset + ((float)count / (float)num);
                                count++;
                            }
                            else {
                                loc_x = x_pos + offset;
                            }

                            glm::vec3 v = glm::vec3(*pPos[m].matrix * glm::vec4(loc_x / (float)polyLineSizes[m], 0, 0, 1));

                            for (size_t z = 0; z < drops_this_node; z++) {
                                if (SingleNode) {
                                    Nodes[idx]->Coords[c].screenX = v.x;
                                    if (up) {
                                        Nodes[idx]->Coords[c].screenY = v.y + z * mheight;
                                    }
                                    else {
                                        Nodes[idx]->Coords[c].screenY = v.y - z * mheight;
                                    }
                                    Nodes[idx]->Coords[c].screenZ = v.z;
                                }
                                else {
                                    auto node = FindNodeAtXY(xx, maxH - z - 1);
                                    if (node == -1) {
                                        logger_base.error("Polyline buffer x,y %d, %d not found.", xx, maxH - z - 1);
                                    }
                                    else {
                                        Nodes[node]->Coords[c].screenX = v.x;
                                        if (up) {
                                            Nodes[node]->Coords[c].screenY = v.y + z * mheight;
                                        }
                                        else {
                                            Nodes[node]->Coords[c].screenY = v.y - z * mheight;
                                        }
                                        Nodes[node]->Coords[c].screenZ = v.z;
                                    }
                                }
                                if (!SingleNode) {
                                    idx++;
                                }
                            }
                            drop_index %= dropSizes.size();
                        }
                        seg_idx++;
                        xx++;
                    }
                }
            }
        }
    }
    else {
        // distribute the lights evenly across the line segments
        int coords_per_node = Nodes[0].get()->Coords.size();
        int lights_to_distribute = SingleNode ? numDropPoints : numDropPoints * coords_per_node;
        float offset = total_length / (float)lights_to_distribute;
        float current_pos = offset / 2.0f;
        idx = 0;
        size_t c = 0;
        int segment = 0;
        int sub_segment = 0;
        int last_seg_light_num = 0;
        float seg_start = current_pos;
        float segment_length = pPos[segment].has_curve ? pPos[segment].curve->GetSegLength(sub_segment) : pPos[segment].length;
        float seg_end = seg_start + segment_length;
        int xx = 0;
        for (int x = 0; x < polyLineSizes.size(); x++) {
            polyLineSizes[x] = 0;
            polyLineSegDropSizes[x] = 0;
        }
        for (size_t m = 0; m < lights_to_distribute; m++) {
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
                        polyLineSizes[segment] = m - last_seg_light_num;
                        last_seg_light_num = m;
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
                    Nodes[idx]->Coords[c].screenX = v.x;
                    if (up) {
                        Nodes[idx]->Coords[c].screenY = v.y + z * mheight;
                    }
                    else {
                        Nodes[idx]->Coords[c].screenY = v.y - z * mheight;
                    }
                    Nodes[idx]->Coords[c].screenZ = v.z;
                }
                else {
                    int node = -1;
                    if (up) {
                        node = FindNodeAtXY(xx, z);
                    }
                    else {
                        node = FindNodeAtXY(xx, maxH - z - 1);
                    }
                    if (node == -1) {
                        logger_base.error("Polyline buffer x,y %d, %d not found.", xx, maxH - z - 1);
                    }
                    else {
                        Nodes[node]->Coords[c].screenX = v.x;
                        if (up) {
                            Nodes[node]->Coords[c].screenY = v.y + z * mheight;
                        }
                        else {
                            Nodes[node]->Coords[c].screenY = v.y - z * mheight;
                        }
                        Nodes[node]->Coords[c].screenZ = v.z;
                    }
                }
                if (c < coords_per_node - 1) {
                    c++;
                }
                else {
                    c = 0;
                    if (!SingleNode) {
                        idx++;
                    }
                }
            }
            polyLineSegDropSizes[segment] += drops_this_node;
            drop_index %= dropSizes.size();
            current_pos += offset;
            xx++;
        }
        polyLineSizes[segment] = lights_to_distribute - last_seg_light_num;
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

void PolyLineModel::DistributeLightsAcrossCurveSegment(int lights, int segment, size_t& idx, std::vector<xlPolyPoint>& pPos,
    std::vector<int>& dropSizes, unsigned int& drop_index, float& mheight, int& xx, int maxH)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // distribute the lights evenly across the line segments
    int coords_per_node = Nodes[0].get()->Coords.size();
    int lights_to_distribute = SingleNode ? lights : lights * coords_per_node;
    float total_length = pPos[segment].curve->GetLength();
    float offset = total_length / (float)lights_to_distribute;
    float current_pos = offset / 2.0f;
    size_t c = 0;
    int sub_segment = 0;
    float seg_start = current_pos;
    float segment_length = pPos[segment].curve->GetSegLength(sub_segment);
    float seg_end = seg_start + segment_length;
    for (size_t m = 0; m < lights_to_distribute; m++) {
        bool up = dropSizes[drop_index] < 0;
        unsigned int drops_this_node = std::abs(dropSizes[drop_index]);
        while (current_pos > seg_end) {
            sub_segment++;
            seg_start = seg_end;
            segment_length = pPos[segment].curve->GetSegLength(sub_segment);
            seg_end = seg_start + segment_length;
        }
        glm::vec3 v = glm::vec3(*pPos[segment].curve->GetMatrix(sub_segment) * glm::vec4((current_pos - seg_start) / segment_length, 0, 0, 1));
        if (SingleNode) {
            for (size_t z = 0; z < drops_this_node; z++) {
                Nodes[0]->Coords[idx].screenX = v.x;
                if (up) {
                    Nodes[0]->Coords[idx].screenY = v.y + z * mheight;
                }
                else {
                    Nodes[0]->Coords[idx].screenY = v.y - z * mheight;
                }
                Nodes[0]->Coords[idx].screenZ = v.z;
                idx++;
            }
            drop_index++;
            drop_index %= dropSizes.size();
        }
        else {
            for (size_t z = 0; z < drops_this_node; z++) {
                auto node = FindNodeAtXY(xx, maxH - z - 1);
                if (node == -1) {
                    logger_base.error("Polyline buffer x,y %d, %d not found.", xx, maxH - z - 1);
                }
                else {
                    Nodes[node]->Coords[c].screenX = v.x;
                    if (up) {
                        Nodes[node]->Coords[c].screenY = v.y + z * mheight;
                    }
                    else {
                        Nodes[node]->Coords[c].screenY = v.y - z * mheight;
                    }
                    Nodes[node]->Coords[c].screenZ = v.z;
                }
                idx++;
            }
            if (c < coords_per_node - 1) {
                c++;
            }
            else {
                c = 0;
            }
            drop_index++;
            drop_index %= dropSizes.size();
        }
        current_pos += offset;
        xx++;
    }
}

static const char* LEFT_RIGHT_VALUES[] = { "Green Square", "Blue Square" };
static wxPGChoices LEFT_RIGHT(wxArrayString(2, LEFT_RIGHT_VALUES)); 

void PolyLineModel::AddTypeProperties(wxPropertyGridInterface* grid)
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

    grid->Append(new wxEnumProperty("Starting Location", "PolyLineStart", LEFT_RIGHT, IsLtoR ? 0 : 1));

    grid->Append(new wxStringProperty("Drop Pattern", "IciclesDrops", GetModelXml()->GetAttribute("DropPattern", "1")));

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
    }
    else {
        for (int x = 0; x < 100; x++) {
            ModelXml->DeleteAttribute(SegAttrName(x));
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
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
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
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::AlternateNodes");
        return 0;
    }
    else if (!GetModelScreenLocation().IsLocked() && "ModelHeight" == event.GetPropertyName()) {
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
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::ModelHeight");
        return 0;
    }
    else if (GetModelScreenLocation().IsLocked() && "ModelHeight" == event.GetPropertyName()) {
        event.Veto();
        return 0;
    }
    return Model::OnPropertyGridChange(grid, event);
}

int PolyLineModel::OnPropertyGridSelection(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if (event.GetPropertyName().StartsWith("ModelIndividualSegments.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());
        str = str.SubString(3, str.length());
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

void PolyLineModel::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    wxXmlDocument doc(filename);

    if (doc.IsOk()) {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "polylinemodel") {
            wxString name = root->GetAttribute("name");
            wxString p1 = root->GetAttribute("parm1");
            wxString p2 = root->GetAttribute("parm2");
            wxString p3 = root->GetAttribute("parm3");
            wxString st = root->GetAttribute("StringType");
            wxString ps = root->GetAttribute("PixelSize");
            wxString t = root->GetAttribute("Transparency");
            wxString mb = root->GetAttribute("ModelBrightness");
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

            for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext()) {
                if (n->GetName() == "stateInfo") {
                    AddState(n);
                }
                else if (n->GetName() == "subModel") {
                    AddSubmodel(n);
                }
            }

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
            }

            ModelXml->DeleteAttribute("PointData");
            ModelXml->DeleteAttribute("cPointData");

            ModelXml->AddAttribute("PointData", point_data);
            ModelXml->AddAttribute("cPointData", cpoint_data);

            GetModelScreenLocation().Read(ModelXml);

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::ImportXlightsModel");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::ImportXlightsModel");
        }
        else {
            DisplayError("Failure loading PolyLine model file.");
        }
    }
    else {
        DisplayError("Failure loading PolyLine model file.");
    }
}

void PolyLineModel::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
    wxString p1 = ModelXml->GetAttribute("parm1");
    wxString p2 = ModelXml->GetAttribute("parm2");
    wxString p3 = ModelXml->GetAttribute("parm3");
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency");
    wxString mb = ModelXml->GetAttribute("ModelBrightness");
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
    f.Write(wxString::Format("PointData=\"%s\" ", point_data));
    f.Write(wxString::Format("cPointData=\"%s\" ", cpoint_data));
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(" >\n");
    wxString state = SerialiseState();
    if (state != "") {
        f.Write(state);
    }
    wxString submodel = SerialiseSubmodel();
    if (submodel != "") {
        f.Write(submodel);
    }
    f.Write("</polylinemodel>");
    f.Close();
}

void PolyLineModel::NormalizePointData()
{
    // read in the point data from xml
    int num_points = wxAtoi(ModelXml->GetAttribute("NumPoints"));
    if (num_points < 2) num_points = 2;
    std::vector<xlPolyPoint> pPos(num_points);
    wxString point_data = ModelXml->GetAttribute("PointData");
    wxArrayString point_array = wxSplit(point_data, ',');
    while (point_array.size() < num_points * 3) point_array.push_back("0.0");
    for( int i = 0; i < num_points; ++i ) {
        pPos[i].x = wxAtof(point_array[i*3]);
        pPos[i].y = wxAtof(point_array[i*3+1]);
        pPos[i].z = wxAtof(point_array[i*3+2]);
        pPos[i].has_curve = false;
        pPos[i].curve = nullptr;
    }
    wxString cpoint_data = ModelXml->GetAttribute("cPointData", "");
    wxArrayString cpoint_array = wxSplit(cpoint_data, ',');
    glm::vec3 def_scaling(100.0f, 100.0f, 100.0f);
    glm::vec3 def_pos(0.0f, 0.0f, 0.0f);
    int num_curves = cpoint_array.size() / 7;
    for( int i = 0; i < num_curves; ++i ) {
        int seg_num = wxAtoi(cpoint_array[i*7]);
        pPos[seg_num].has_curve = true;
        pPos[seg_num].curve = new BezierCurveCubic3D();
        pPos[seg_num].curve->set_p0(pPos[seg_num].x, pPos[seg_num].y, pPos[seg_num].z);
        pPos[seg_num].curve->set_p1(pPos[seg_num+1].x, pPos[seg_num+1].y, pPos[seg_num+1].z);
        pPos[seg_num].curve->set_cp0( wxAtof(cpoint_array[i*7+1]), wxAtof(cpoint_array[i*7+2]), wxAtof(cpoint_array[i*7+3]));
        pPos[seg_num].curve->set_cp1( wxAtof(cpoint_array[i*7+4]), wxAtof(cpoint_array[i*7+5]), wxAtof(cpoint_array[i*7+6]));
        pPos[seg_num].curve->SetPositioning(def_scaling, def_pos);
        pPos[seg_num].curve->UpdatePoints();
    }

    float minX = 100000.0f;
    float minY = 100000.0f;
    float minZ = 100000.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    float maxZ = 0.0f;

    for( int i = 0; i < num_points; ++i ) {
        if( pPos[i].x < minX ) minX = pPos[i].x;
        if (pPos[i].y < minY) minY = pPos[i].y;
        if (pPos[i].z < minZ) minZ = pPos[i].z;
        if( pPos[i].x > maxX ) maxX = pPos[i].x;
        if (pPos[i].y > maxY) maxY = pPos[i].y;
        if (pPos[i].z > maxZ) maxZ = pPos[i].z;
        if( pPos[i].has_curve ) {
            pPos[i].curve->check_min_max(minX, maxX, minY, maxY, minZ, maxZ);
        }
    }
    float deltax = maxX-minX;
    float deltay = maxY-minY;
    float deltaz = maxZ-minZ;

    // normalize all the point data
    for( int i = 0; i < num_points; ++i ) {
        if (deltax == 0.0f) {
            pPos[i].x = 0.0f;
        }
        else {
            pPos[i].x = (pPos[i].x - minX) / deltax;
        }
        if (deltay == 0.0f) {
            pPos[i].y = 0.0f;
        }
        else {
            pPos[i].y = (pPos[i].y - minY) / deltay;
        }
        if (deltaz == 0.0f) {
            pPos[i].z = 0.0f;
        }
        else {
            pPos[i].z = (pPos[i].z - minZ) / deltaz;
        }
        if( pPos[i].has_curve ) {
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
    for( int i = 0; i < num_points; ++i ) {
        point_data += wxString::Format("%f,", pPos[i].x );
        point_data += wxString::Format("%f,", pPos[i].y);
        point_data += wxString::Format("%f", pPos[i].z);
        if( i != num_points-1 ) {
            point_data += ",";
        }
    }
    cpoint_data = "";
    for( int i = 0; i < num_points; ++i ) {
        if( pPos[i].has_curve ) {
            cpoint_data += wxString::Format( "%d,%f,%f,%f,%f,%f,%f,", i, pPos[i].curve->get_cp0x(), pPos[i].curve->get_cp0y(), pPos[i].curve->get_cp0z(),
                                                       pPos[i].curve->get_cp1x(), pPos[i].curve->get_cp1y(), pPos[i].curve->get_cp1z());
        }
    }
    ModelXml->AddAttribute("PointData", point_data);
    ModelXml->AddAttribute("cPointData", cpoint_data);
}

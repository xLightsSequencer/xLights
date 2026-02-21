/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
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
    stringStartChan.resize(_strings);
    _polyCorner.resize(2);
    _polyLineSizes.resize(1);
    _polyLineSegDropSizes.resize(1);
    _polyLeadOffset.resize(1);
    _polyTrailOffset.resize(1);
}

PolyLineModel::~PolyLineModel()
{
}

static const char* POLY_CORNER_VALUES[] = {
    "Leading Segment",
    "Trailing Segment",
    "Neither"
};
static wxPGChoices POLY_CORNERS(wxArrayString(3, POLY_CORNER_VALUES));

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

bool PolyLineModel::IsNodeFirst(int n) const
{
    return (GetIsLtoR() && n == 0) || (!GetIsLtoR() && n == Nodes.size() - 1);
}

void PolyLineModel::InitRenderBufferNodes(const std::string& tp, const std::string& camera,
    const std::string& transform,
    std::vector<NodeBaseClassPtr>& newNodes, int& BufferWi, int& BufferHi, int stagger, bool deep) const
{
    std::string type = tp.starts_with("Per Model ") ? tp.substr(10) : tp;

    if (type == "Line Segments") {
        BufferHi = _numSegments;
        BufferWi = 0;
        for (int x = 0; x < _numSegments; x++) {
            int w = _polyLineSizes[x];
            if (w > BufferWi) {
                BufferWi = w;
            }
        }
        for (const auto& it : Nodes) {
            newNodes.push_back(NodeBaseClassPtr(it.get()->clone()));
        }

        int idx = 0;
        for (size_t m = 0; m < _numSegments; m++) {
            int seg_idx = 0;
            int end_node = idx + _polyLineSizes[m];
            float scale = (float)BufferWi / (float)_polyLineSizes[m];
            for (size_t n = idx; n < end_node; n++) {
                newNodes[idx]->Coords.resize(SingleNode ? parm2 : parm3);
                size_t CoordCount = GetCoordCount(idx);
                int location = seg_idx * scale + scale / 2.0;
                for (size_t c = 0; c < CoordCount; c++) {
                    newNodes[idx]->Coords[c].bufX = location;
                    newNodes[idx]->Coords[c].bufY = m;
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
    if (polyLineLayer >= _polyLineSizes.size()) return 0;
    if (_polyLineSegDropSizes[polyLineLayer]) {
        return _polyLineSegDropSizes[polyLineLayer];
    }
    return _polyLineSizes[polyLineLayer];
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
    return SingleNode ? 1 : _polyLineSizes.size();
}

void PolyLineModel::SetStringStartChannels(int NumberOfStrings, int StartChannel, int ChannelsPerString) {
    if( _hasIndivChans && !SingleNode ) {
        // if individual start channels defer to InitModel where we know all the segment length data
    } else {
        if (_strings == 1) {
            Model::SetStringStartChannels(NumberOfStrings, StartChannel, ChannelsPerString);
        } else {
            ChannelsPerString /= _strings;
            stringStartChan.clear();
            stringStartChan.resize(_strings);

            for (int i = 0; i < _strings; i++) {
                int node = 1;
                if (_hasIndivNodes) {
                    node = _indivStartNodes[i];
                } else {
                    node = ((ChannelsPerString * i) / GetNodeChannelCount(StringType)) + 1;
                }
                stringStartChan[i] = (StartChannel - 1) + (node - 1) * GetNodeChannelCount(StringType);
            }
        }
    }
}

void PolyLineModel::SetSegmentSize(int idx, int val)
{
    _polyLineSizes[idx] = val;
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::SetSegmentSize");
    AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::SetSegmentSize");
}

void PolyLineModel::AddHandle() {
    _polyLineSizes.push_back(50);
    _polyLeadOffset.push_back(0.5);
    _polyTrailOffset.push_back(0.5);
    _polyCorner.push_back("Neither");
    _polyLineSegDropSizes.push_back(1);
    _autoDistributeLights = true;
}

void PolyLineModel::InsertHandle(int after_handle, float zoom, int scale) {
    if( _polyLineSizes.size() > after_handle ) {
        int segment1_size = _polyLineSizes[after_handle] / 2;
        int segment2_size = _polyLineSizes[after_handle] - segment1_size;
        _polyLineSizes[after_handle] = segment1_size;
        _polyLineSizes.insert(_polyLineSizes.begin() + after_handle + 1, segment2_size);
        _polyLeadOffset.insert(_polyLeadOffset.begin() + after_handle + 1, 0.5);
        _polyTrailOffset.insert(_polyTrailOffset.begin() + after_handle + 1, 0.5);
        _polyCorner.insert(_polyCorner.begin() + after_handle + 1, "Neither");
        _polyLineSegDropSizes.insert(_polyLineSegDropSizes.begin() + after_handle + 1, 1);
    }
    GetModelScreenLocation().InsertHandle(after_handle, zoom, scale);
}

void PolyLineModel::DeleteHandle(int handle_) {
    // handle is offset by 1 due to the center handle at 0
    int handle = handle_ - 1;
    if( _polyLineSizes.size() > handle ) {
        _polyLineSizes.erase(_polyLineSizes.begin() + handle);
        _polyLeadOffset.erase(_polyLeadOffset.begin() + handle);
        _polyTrailOffset.erase(_polyTrailOffset.begin() + handle);
        _polyCorner.erase(_polyCorner.begin() + handle);
        _polyLineSegDropSizes.erase(_polyLineSegDropSizes.begin() + handle);
    } else {
        _polyLineSizes.erase(_polyLineSizes.begin() + handle - 1);
    }
    GetModelScreenLocation().DeleteHandle(handle);
}

void PolyLineModel::InitModel()
{
    // establish light and segment counts
    int numLights = 0;

    _numSegments = screenLocation.num_points - 1;
    _numDropPoints = 0;
    
    // Detect and fix any size that changed from handle being added or deleted
    if (_dropSizes.size() == 0) {
        _dropSizes.push_back(1);
    }

    // setup number of lights per line segment
    unsigned int drop_index = 0;
    if (!_autoDistributeLights) {
        parm1 = SingleNode ? 1 : _numSegments;
        for (int x = 0; x < _numSegments; x++) {
            unsigned int drop_lights_this_segment = 0;
            for (size_t z = 0; z < _polyLineSizes[x]; z++) {
                drop_lights_this_segment += std::abs(_dropSizes[drop_index++]);
                drop_index %= _dropSizes.size();
            }
            numLights += drop_lights_this_segment;
            _numDropPoints += _polyLineSizes[x];
            _polyLineSegDropSizes[x] = drop_lights_this_segment;
        }
        parm2 = numLights;
    }
    else {
        parm1 = 1;
        int lights = parm2;
        while (lights > 0) {
            unsigned int lights_this_drop = std::abs(_dropSizes[drop_index++]);
            numLights += lights_this_drop;
            drop_index %= _dropSizes.size();
            _numDropPoints++;
            lights -= lights_this_drop;
        }
        if (numLights != parm2) {
            parm2 = numLights;
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
    if (_hasIndivChans && !SingleNode) {
        std::string dependsonmodel;
        int StartChannel = GetNumberFromChannelString(ModelStartChannel, CouldComputeStartChannel, dependsonmodel);
        stringStartChan.clear();
        stringStartChan.resize(_numSegments);
        for (int i = 0; i < _numSegments; i++) {
            if (_hasIndivChans) {
                bool b = false;
                stringStartChan[i] = GetNumberFromChannelString(_indivStartChannels[i], b, dependsonmodel) - 1;
                CouldComputeStartChannel &= b;
            }
            else {
                stringStartChan[i] = (StartChannel - 1) + _polyLineSegDropSizes[i] * GetNodeChannelCount(StringType);
            }
        }
    }

    // fix the string numbers for each node since model is non-standard
    size_t idx = 0;
    if (_hasIndivChans && !SingleNode) {
        for (int x = 0; x < _numSegments; x++) {
            for (int n = 0; n < _polyLineSegDropSizes[x]; ++n) {
                Nodes[idx++]->StringNum = x;
            }
        }
    } else if ( _strings > 1 ) {
        int node_count = GetNodeCount();
        for (int s = 0; s < _strings; ++s) {
            int v1 = 0;
            int v2 = node_count;
            if (_hasIndivNodes) {
                v1 = _indivStartNodes[s];
                if (s < _strings - 1) { // not last string
                    v2 = _indivStartNodes[s+1] - 1;
                }
            } else {
                v1 = ComputeStringStartNode(s) - 1;
                if (s < _strings - 1) { // not last string
                    v2 = ComputeStringStartNode(s + 1) - 1;
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
    
    // Copy point data from screenLocation....maybe this can be cleaned up later but I was having trouble since we delete the curves and matrix inside here
    std::vector<xlPolyPoint> pPos(screenLocation.num_points);
    for (int i = 0; i < screenLocation.num_points; ++i) {
        pPos[i].x = screenLocation.mPos[i].x;
        pPos[i].y = screenLocation.mPos[i].y;
        pPos[i].z = screenLocation.mPos[i].z;
        pPos[i].has_curve = screenLocation.mPos[i].has_curve;
        pPos[i].curve = nullptr;
    }
    glm::vec3 def_scaling(100.0f, 100.0f, 100.0f);
    glm::vec3 def_pos(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < screenLocation.num_points - 1; ++i) {
        if (pPos[i].has_curve) {
            pPos[i].curve = new BezierCurveCubic3D();
            pPos[i].curve->set_p0(pPos[i].x, pPos[i].y, pPos[i].z);
            pPos[i].curve->set_p1(pPos[i + 1].x, pPos[i + 1].y, pPos[i + 1].z);
            pPos[i].curve->set_cp0(screenLocation.mPos[i].curve->get_p0x(),screenLocation.mPos[i].curve->get_p0y(),screenLocation.mPos[i].curve->get_p0z());
            pPos[i].curve->set_cp1(screenLocation.mPos[i].curve->get_p1x(),screenLocation.mPos[i].curve->get_p1y(),screenLocation.mPos[i].curve->get_p1z());
            pPos[i].curve->SetPositioning(def_scaling, def_pos);
            pPos[i].curve->UpdatePoints();
            pPos[i].curve->UpdateMatrices();
        }
    }

    // calculate segment lengths if we need to auto-distribute lights
    _totalLength = 0.0f;
    if (_autoDistributeLights) {
        for (int i = 0; i < screenLocation.num_points - 1; ++i) {
            if (pPos[i].has_curve) {
                _totalLength += pPos[i].curve->GetLength();
            }
            else {
                float length = std::sqrt((pPos[i + 1].z - pPos[i].z) * (pPos[i + 1].z - pPos[i].z) +
                                         (pPos[i + 1].y - pPos[i].y) * (pPos[i + 1].y - pPos[i].y) +
                                         (pPos[i + 1].x - pPos[i].x) * (pPos[i + 1].x - pPos[i].x));
                pPos[i].length = length;
                _totalLength += length;
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

    for (int i = 0; i < screenLocation.num_points; ++i) {
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
    for (int i = 0; i < screenLocation.num_points - 1; ++i) {
        float x1p, y1p, z1p, x2p, y2p, z2p;
        if (deltax == 0.0f) {
            x1p = 0.0f;
            x2p = 0.0f;
        }
        else {
            x1p = (pPos[i].x - minX) / deltax;
            x2p = (pPos[i + 1].x - minX) / deltax;
        }
        if (std::abs(deltay) < 0.1f) {
            y1p = 0.0f;
            y2p = 0.0f;
        } else {
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
        if (screenLocation.num_points == 2 && !pPos[i].has_curve) {
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
    bool up = _dropSizes[drop_index] < 0;
    int nodesInDrop = std::abs(_dropSizes[drop_index]);
    int nodesInDropLast = nodesInDrop;
    while (lights) {
        if (curCoord >= Nodes[curNode]->Coords.size()) {
            curNode++;
            curCoord = 0;
            if (!SingleNode) {
                chan += ChanIncr;
            }
        }
        while (y >= std::abs(_dropSizes[drop_index])) {
            width++;
            y = 0;
            drop_index++;
            if (drop_index >= _dropSizes.size()) {
                drop_index = 0;
            }
            nodesInDrop = std::abs(_dropSizes[drop_index]);
            if (!IsLtoR && !SingleNode && curCoord == 0) {
                chan -= ((nodesInDropLast + nodesInDrop) * GetNodeChannelCount(StringType));
            }
            nodesInDropLast = nodesInDrop;
            up = _dropSizes[drop_index] < 0;
        }
        if (Nodes[curNode]->StringNum != LastStringNum) {
            LastStringNum = Nodes[curNode]->StringNum;
            chan = stringStartChan[LastStringNum];
            if (!IsLtoR && !SingleNode && curCoord == 0) {
                chan += (NodesPerString(LastStringNum) - nodesInDrop) * GetNodeChannelCount(StringType);
            }
        }
        Nodes[curNode]->ActChan = chan;
        Nodes[curNode]->Coords[curCoord].bufX = SingleNode ? 0 : width;
        if (_alternateNodes) {
            if (y + 1 <= (nodesInDrop + 1) / 2) {
                if (up) {
                    Nodes[curNode]->Coords[curCoord].bufY = 2 * y;
                    Nodes[curNode]->Coords[curCoord].screenY = -1 * (_maxH - 1) + (2 * y);
                }
                else {
                    Nodes[curNode]->Coords[curCoord].bufY = _maxH - 1 - (2 * y);
                    Nodes[curNode]->Coords[curCoord].screenY = _maxH - 1 - (2 * y);
                }
            }
            else {
                if (up) {
                    Nodes[curNode]->Coords[curCoord].bufY = (nodesInDrop - (y + 1)) * 2 + 1;
                    Nodes[curNode]->Coords[curCoord].screenY = -1 * (_maxH - 1) + ((nodesInDrop - (y + 1)) * 2 + 1);
                }
                else {
                    Nodes[curNode]->Coords[curCoord].bufY = _maxH - 1 - ((nodesInDrop - (y + 1)) * 2 + 1);
                    Nodes[curNode]->Coords[curCoord].screenY = _maxH - 1 - ((nodesInDrop - (y + 1)) * 2 + 1);
                }
            }
        }
        else {
            if (up) {
                Nodes[curNode]->Coords[curCoord].bufY = y;
                Nodes[curNode]->Coords[curCoord].screenY = y;
            }
            else {
                Nodes[curNode]->Coords[curCoord].bufY = _maxH - y - 1;
                Nodes[curNode]->Coords[curCoord].screenY = _maxH - y - 1;
            }
        }

        Nodes[curNode]->Coords[curCoord].screenX = width;
        lights--;
        curCoord++;
        if( SingleNode || curCoord == parm3 ) {
            y++;
        }
    }

    SetBufferSize(_maxH, SingleNode ? 1 : width + 1);
    screenLocation.SetRenderSize(1.0, _maxH);

    double model_height = deltay;
    if (model_height < GetModelScreenLocation().GetRenderHt()) {
        model_height = GetModelScreenLocation().GetRenderHt();
    }
    float mheight = _height * 10.0f / model_height;

    // place the nodes/coords along each line segment
    drop_index = 0;
    if (_autoDistributeLights) {
        // distribute the lights evenly across the line segments
        DistributeLightsEvenly( pPos, _dropSizes, mheight, _maxH, numLights );
        if (!_creatingNewPolyLine) {
            _autoDistributeLights = false;
        }
    } else {
        // distribute the lights as defined by the polysizes
        DistributeLightsAcrossIndivSegments( pPos, _dropSizes, mheight, _maxH );
    }

    // cleanup curves and matrices
    for (int i = 0; i < screenLocation.num_points; ++i) {
        if (pPos[i].has_curve) {
            delete pPos[i].curve;
            pPos[i].curve = nullptr;
        }
        if (pPos[i].matrix != nullptr) {
            delete pPos[i].matrix;
        }
    }
}

void PolyLineModel::DistributeLightsEvenly(       std::vector<xlPolyPoint>& pPos,
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
        offset = _totalLength / ((float)_numDropPoints * (using_icicles ? 1.0f : (float)coords_per_node));
    } else {
        offset = _totalLength / (float)_numDropPoints;
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
    for (int x = 0; x < _polyLineSizes.size(); x++) {
        _polyLineSizes[x] = 0;
        _polyLineSegDropSizes[x] = 0;
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
                if (segment == _polyLineSizes.size() - 1) {
                    // cant increase segment ... so just fudge the segment end
                    seg_end += 0.0001f;
                }
                else {
                    sub_segment = 0;
                    _polyLineSizes[segment] = seg_count - last_seg_count;
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
        _polyLineSegDropSizes[segment] += drops_this_node;
        drop_index %= dropSizes.size();
        current_pos += offset;
        if( c == 0 ) {
            xpos++;
        }
    }
    _polyLineSizes[segment] = seg_count - last_seg_count;
}

void PolyLineModel::DistributeLightsAcrossIndivSegments(       std::vector<xlPolyPoint>& pPos,
                                                         const std::vector<int>&         dropSizes,
                                                         const float&                    mheight,
                                                         const int                       maxH )
{
    unsigned int drop_index = 0;
    size_t idx = 0;
    int xpos = 0;  // the horizontal position in the buffer
    for (size_t m = 0; m < _numSegments; m++) {
        DistributeLightsAcrossSegment(m, idx, pPos, dropSizes, drop_index, mheight, xpos, maxH, pPos[m].has_curve);
    }
}

void PolyLineModel::DistributeLightsAcrossSegment( const int                       segment,
                                                         size_t&                   idx,
                                                         std::vector<xlPolyPoint>& pPos,
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
    for (size_t i = 0; i < _polyLineSizes[segment]; ++i) {
        unsigned int drops_this_node = std::abs(dropSizes[idrop]);
        lights += drops_this_node;
        idrop++;
        idrop %= dropSizes.size();
    }
    
    int lights_to_distribute = SingleNode ? lights : lights * coords_per_node;
    float total_length = isCurve ? pPos[segment].curve->GetLength() : _polyLineSizes[segment];
    
    float num_gaps;
    if (using_icicles) {
        num_gaps = _polyLeadOffset[segment] + _polyTrailOffset[segment] + _polyLineSizes[segment] - 1.0f;
    } else {
        num_gaps = _polyLeadOffset[segment] + _polyTrailOffset[segment] + float(lights_to_distribute) - 1.0f;
    }
    float offset = total_length / num_gaps;
    float current_pos = _polyLeadOffset[segment] * offset;
    size_t c = 0;
    int sub_segment = 0;
    float seg_start = 0;
    float segment_length = isCurve ? pPos[segment].curve->GetSegLength(sub_segment) : _polyLineSizes[segment];
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
            v = glm::vec3(*pPos[segment].matrix * glm::vec4(current_pos / _polyLineSizes[segment], 0, 0, 1));
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
    }
    else {
        p = grid->Append(new wxUIntProperty("# Nodes", "PolyLineNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");

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
        p = grid->Append(new wxStringProperty("Start Nodes", "ModelIndividualStartNodes", ""));

        std::string nm = StartChanAttrName(0);
        wxPGProperty* psn = grid->AppendIn(p, new wxUIntProperty(nm, nm, _indivStartNodes[0]));
        psn->SetAttribute("Min", 1);
        psn->SetAttribute("Max", (int)GetNodeCount());
        psn->SetEditor("SpinCtrl");

        if (_hasIndivNodes) {
            int c = _strings;
            for (int x = 0; x < c; x++) {
                int v = _indivStartNodes[x];
                if (v < 1)
                    v = 1;
                if (v > NodesPerString())
                    v = NodesPerString();
                if (x == 0) {
                    psn->SetValue(v);
                } else {
                    nm = StartChanAttrName(x);
                    grid->AppendIn(p, new wxUIntProperty(nm, nm, v));
                }
            }
        } else {
            psn->Enable(false);
        }
    }

    grid->Append(new wxEnumProperty("Starting Location", "PolyLineStart", LEFT_RIGHT, IsLtoR ? 0 : 1));

    p = grid->Append(new wxDropPatternProperty("Drop Pattern", "IciclesDrops", _dropPatternString));

    p = grid->Append(new wxBoolProperty("Alternate Drop Nodes", "AlternateNodes", _alternateNodes));
    p->SetEditor("CheckBox");

    p = grid->Append(new wxFloatProperty("Height", "ModelHeight", _height));
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxStringProperty("Segments", "ModelIndividualSegments", ""));
    for (int x = 0; x < _numSegments; x++) {
        wxString nm = wxString::Format("Segment %d", x + 1);
        grid->AppendIn(p, new wxUIntProperty(nm, SegAttrName(x), _polyLineSizes[x]));
    }
    if (_segsCollapsed) grid->Collapse(p);

    p = grid->Append(new wxStringProperty("Corner Settings", "PolyCornerProperties", ""));
    for (int x = 0; x < _numSegments + 1; x++) {
        wxString nm = wxString::Format("Corner %d", x + 1);
        grid->AppendIn(p, new wxEnumProperty(nm, CornerAttrName(x), POLY_CORNERS, _polyCorner[x] == "Leading Segment" ? 0 : _polyCorner[x] == "Trailing Segment" ? 1 : 2 ));
    }
    if (_segsCollapsed) grid->Collapse(p);
}

int PolyLineModel::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if ("PolyLineNodes" == event.GetPropertyName()) {
        parm2 = (int)event.GetPropertyValue().GetLong();
        wxPGProperty* sp = grid->GetPropertyByLabel("# Nodes");
        if (sp == nullptr) {
            sp = grid->GetPropertyByLabel("# Lights");
        }
        sp->SetValueFromInt(parm2);
        _autoDistributeLights = true;
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
        parm3 = (int)event.GetPropertyValue().GetLong();
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
        _dir = event.GetValue().GetLong() == 0 ? "L" : "R";
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::PolyLineStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::PolyLineStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::PolyLineStart");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::PolyLineStart");
        return 0;
    }
    else if (event.GetPropertyName().StartsWith("ModelIndividualSegments.")) {
        std::string segment = event.GetPropertyName().ToStdString();
        int idx = ExtractTrailingInt(segment) - 1;
        _polyLineSizes[idx] = event.GetPropertyValue().GetLong();
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
        std::string corner = event.GetPropertyName().ToStdString();
        int x = ExtractTrailingInt(corner) - 1;
        _polyCorner[x] = POLY_CORNER_VALUES[event.GetPropertyValue().GetLong()];
        if( x == 0 ) {
            _polyLeadOffset[x] = (_polyCorner[x] == "Leading Segment" ? 1.0 : _polyCorner[x] == "Trailing Segment" ? 0.0 : 0.5);
        } else if( x == _numSegments ) {
            _polyTrailOffset[x-1] = (_polyCorner[x] == "Leading Segment" ? 0.0 : _polyCorner[x] == "Trailing Segment" ? 1.0 : 0.5);
        } else {
            _polyTrailOffset[x-1] = (_polyCorner[x] == "Leading Segment" ? 0.0 : _polyCorner[x] == "Trailing Segment" ? 1.0 : 0.5);
            _polyLeadOffset[x] = (_polyCorner[x] == "Leading Segment" ? 1.0 : _polyCorner[x] == "Trailing Segment" ? 0.0 : 0.5);
        }
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
        _hasIndivNodes = _strings > 1;
        if (old_string_count != new_string_count) {
            if (_hasIndivNodes) {
                _indivStartNodes.resize(_strings);
                for (int x = 0; x < _strings; x++) {
                    _indivStartNodes[x] = ComputeStringStartNode(x);
                }
            }
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::PolyLineStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::PolyLineStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::PolyLineStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "PolyLineModel::OnPropertyGridChange::PolyLineStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::OnPropertyGridChange::PolyLineStrings");
        return 0;
    }
    else if (event.GetPropertyName() == "ModelIndividualStartNodes") {
        _hasIndivNodes = event.GetValue().GetBool();
        if (_hasIndivNodes) {
            _indivStartNodes.resize(_strings);
            for (int x = 0; x < _strings; x++) {
                _indivStartNodes[x] = ComputeStringStartNode(x);
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
    else if (event.GetPropertyName().StartsWith("ModelIndividualStartNodes.String")) {
        wxString s = event.GetPropertyName().substr(strlen("ModelIndividualStartNodes.String"));
        int string = wxAtoi(s) - 1;
        int value = event.GetValue().GetInteger();
        if (value < 1)
            value = 1;
        if (value > NodesPerString())
            value = NodesPerString();
        _indivStartNodes[string] = value;
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        return 0;
    }
    else if (event.GetPropertyName() == "ModelIndividualStartChannels") {
        _hasIndivChans = event.GetValue().GetBool();
        if (_hasIndivChans) {
            int start_channel = 1;
            for (int x = 0; x < _numSegments; x++) {
                if (_indivStartChannels[x] == "") {
                    _indivStartChannels[x] = wxString::Format("%d", start_channel);
                }
                start_channel += _polyLineSizes[x] * GetNodeChannelCount(StringType);
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
        _dropPatternString = event.GetPropertyValue().GetString();
        ParseDropSizes();
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::OnPropertyGridChange::IciclesDrops");
        return 0;
    }
    else if ("AlternateNodes" == event.GetPropertyName()) {
        _alternateNodes = event.GetPropertyValue().GetBool();
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyLineModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyLineModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "PolyLineModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyLineModel::OnPropertyGridChange::AlternateNodes");
        return 0;
    } else if (!GetModelScreenLocation().IsLocked() && !IsFromBase() && "ModelHeight" == event.GetPropertyName()) {
        _height = event.GetValue().GetDouble();
        if (std::abs(_height) < 0.01f) {
            if (_height < 0.0f) {
                _height = -0.01f;
            }
            else {
                _height = 0.01f;
            }
        }
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

int PolyLineModel::ComputeStringStartNode(int x) const
{
    if (x == 0)
        return 1;

    int strings = GetNumPhysicalStrings();
    int nodes = GetNodeCount();
    float nodesPerString = (float)nodes / (float)strings;

    return (int)(x * nodesPerString + 1);
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
                v1 = ComputeStringStartNode(string);
                if (string < _strings - 1) { // not last string
                    v2 = ComputeStringStartNode(string + 1);
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
        _segsCollapsed = true;
    }
}

void PolyLineModel::OnPropertyGridItemExpanded(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if (event.GetPropertyName() == "ModelIndividualSegments") {
        _segsCollapsed = false;
    }
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

void PolyLineModel::SetDropPattern(const std::string & pattern)
{
    _dropPatternString = pattern;
    ParseDropSizes();
}


void PolyLineModel::ParseDropSizes()
{
    _dropSizes.clear();
    wxArrayString pat = wxSplit(_dropPatternString, ',');
    // parse drop sizes
    _dropSizes.clear();
    _maxH = 0;
    for (int x = 0; x < pat.size(); x++) {
        int pat_size = wxAtoi(pat[x]);
        if( pat_size == 0 ) {
            pat_size = 1;
        }
        _dropSizes.push_back(pat_size);
        _maxH = std::max(_maxH, (unsigned int)std::abs(_dropSizes[x]));
    }
    if (_dropSizes.size() == 0) {
        _dropSizes.push_back(5);
    }
}

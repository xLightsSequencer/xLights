#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/log.h>
#include <wx/filedlg.h>

#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/mat3x3.hpp>

#include "PolyLineModel.h"
#include "ModelScreenLocation.h"
#include "../xLightsMain.h"
#include "../xLightsVersion.h"

PolyLineModel::PolyLineModel(const ModelManager &manager) : ModelWithScreenLocation(manager) {
    parm1 = parm2 = parm3 = 0;
    segs_collapsed = true;
    num_segments = 0;
    total_length = 0.0f;
    hasIndivSeg = false;
}

PolyLineModel::PolyLineModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    segs_collapsed = true;
    SetFromXml(node, zeroBased);
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

void PolyLineModel::InitRenderBufferNodes(const std::string &type,
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
                    newNodes[idx]->Coords[c].bufX=IsLtoR ? location : (SingleNode ? location : BufferWi-location-1);
                    newNodes[idx]->Coords[c].bufY=m;
                }
                idx++;
                seg_idx++;
            }
        }
        ApplyTransform(transform, newNodes, BufferWi, BufferHi);
    } else {
        Model::InitRenderBufferNodes(type, transform, newNodes, BufferWi, BufferHi);
    }
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

void PolyLineModel::InsertHandle(int after_handle) {
    if( polyLineSizes.size() > after_handle ) {
        for (int x = num_segments; x > after_handle; --x) {
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
    GetModelScreenLocation().InsertHandle(after_handle);
}

void PolyLineModel::DeleteHandle(int handle) {
    if( polyLineSizes.size() > handle ) {
        ModelXml->DeleteAttribute(SegAttrName(handle));
        for (int x = handle; x < num_segments; ++x) {
            std::string val = ModelXml->GetAttribute(SegAttrName(x+1)).ToStdString();
            if (val == "") {
                val = wxString::Format("%d", polyLineSizes[x+1]);
            }
            ModelXml->AddAttribute(SegAttrName(x), val);
            ModelXml->DeleteAttribute(SegAttrName(x+1));
        }
        polyLineSizes.erase(polyLineSizes.begin() + handle);
    }
    GetModelScreenLocation().DeleteHandle(handle);
}

void PolyLineModel::InitModel() {

    // establish light and segment counts
    int numLights = parm2;
    int num_points = wxAtoi(ModelXml->GetAttribute("NumPoints", "2"));
	num_segments = num_points - 1;
    hasIndivSeg = ModelXml->GetAttribute("IndivSegs", "0") == "1";

    // setup number of lights per line segment
    polyLineSizes.resize(num_segments);
    if (hasIndivSeg) {
        parm1 = SingleNode ? 1 : num_segments;
        numLights = 0;
        for (int x = 0; x < num_segments; x++) {
            wxString val = ModelXml->GetAttribute(SegAttrName(x));
            if (val == "") {
                val = "0";
                ModelXml->DeleteAttribute(SegAttrName(x));
                ModelXml->AddAttribute(SegAttrName(x), val);
            }
            int num_lights_this_segment = wxAtoi(val);
            polyLineSizes[x] = num_lights_this_segment;
            numLights += num_lights_this_segment;
        }
        parm2 = numLights;
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%ld", parm2));
    } else {
        parm1 = 1;
    }

    // reset node information
    Nodes.clear();
    SetNodeCount(1,numLights,rgbOrder);

    // process our own start channels
    std::string tempstr = ModelXml->GetAttribute("Advanced", "0").ToStdString();
    bool HasIndividualStartChans=tempstr == "1";
    if( HasIndividualStartChans && !SingleNode) {
        std::string dependsonmodel;
        int StartChannel = GetNumberFromChannelString(ModelXml->GetAttribute("StartChannel","1").ToStdString(), CouldComputeStartChannel, dependsonmodel);
        stringStartChan.clear();
        stringStartChan.resize(num_segments);
        for (int i=0; i<num_segments; i++) {
            tempstr = StartChanAttrName(i);
            if (!zeroBased && ModelXml->HasAttribute(tempstr)) {
                bool b = false;
                stringStartChan[i] = GetNumberFromChannelString(ModelXml->GetAttribute(tempstr, "1").ToStdString(), b, dependsonmodel)-1;
                CouldComputeStartChannel &= b;
            } else {
                stringStartChan[i] = (zeroBased? 0 : StartChannel-1) + polyLineSizes[i]*GetNodeChannelCount(StringType);
            }
        }
    }

    // fix the string numbers for each node since model is non-standard
    size_t idx=0;
    if (HasIndividualStartChans && hasIndivSeg && !SingleNode) {
        for (int x = 0; x < (SingleNode ? 1 : num_segments); x++) {
            for( int n = 0; n < polyLineSizes[x]; ++n ) {
                Nodes[idx++]->StringNum = x;
            }
        }
    }

    // read in the point data from xml
    std::vector<xlPolyPoint> pPos(num_points);
    wxString point_data = ModelXml->GetAttribute("PointData", "0.4, 0.6, 0.4, 0.6");
    wxArrayString point_array = wxSplit(point_data, ',');
    for( int i = 0; i < num_points; ++i ) {
        pPos[i].x = wxAtof(point_array[i*2]);
        pPos[i].y = wxAtof(point_array[i*2+1]);
        pPos[i].has_curve = false;
        pPos[i].curve = nullptr;
    }
    wxString cpoint_data = ModelXml->GetAttribute("cPointData", "");
    wxArrayString cpoint_array = wxSplit(cpoint_data, ',');
    int num_curves = cpoint_array.size() / 5;
    for( int i = 0; i < num_curves; ++i ) {
        int seg_num = wxAtoi(cpoint_array[i*5]);
        pPos[seg_num].has_curve = true;
        pPos[seg_num].curve = new BezierCurveCubic();
        pPos[seg_num].curve->set_p0(pPos[seg_num].x, pPos[seg_num].y);
        pPos[seg_num].curve->set_p1(pPos[seg_num+1].x, pPos[seg_num+1].y);
        pPos[seg_num].curve->set_cp0( wxAtof(cpoint_array[i*5+1]), wxAtof(cpoint_array[i*5+2]) );
        pPos[seg_num].curve->set_cp1( wxAtof(cpoint_array[i*5+3]), wxAtof(cpoint_array[i*5+4]) );
        pPos[seg_num].curve->SetScale(1.0, 1.0, 1.0);
        pPos[seg_num].curve->UpdatePoints();
        pPos[seg_num].curve->UpdateMatrices();
    }

    // calculate segment lengths if we need to auto-distribute lights
    total_length = 0.0f;
    if (!hasIndivSeg) {
        for( int i = 0; i < num_points-1; ++i ) {
            if( pPos[i].has_curve ) {
                total_length += pPos[i].curve->GetLength();
            } else {
                float length = std::sqrt((pPos[i+1].y - pPos[i].y)*(pPos[i+1].y - pPos[i].y) + (pPos[i+1].x - pPos[i].x)*(pPos[i+1].x - pPos[i].x));
                pPos[i].length = length;
                total_length += length;
            }
        }
    }

    // calculate min/max for the model
    float minX = 100.0f;
    float minY = 100.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;

    for( int i = 0; i < num_points; ++i ) {
        if( pPos[i].x < minX ) minX = pPos[i].x;
        if( pPos[i].y < minY ) minY = pPos[i].y;
        if( pPos[i].x > maxX ) maxX = pPos[i].x;
        if( pPos[i].y > maxY ) maxY = pPos[i].y;
        if( pPos[i].has_curve ) {
            pPos[i].curve->check_min_max(minX, maxX, minY, maxY);
        }
    }
    float deltax = maxX-minX;
    float deltay = maxY-minY;

    // normalize all points from 0.0 to 1.0 and create
    // a matrix for each line segment
    for( int i = 0; i < num_points-1; ++i ) {
        float x1p, y1p, x2p, y2p;
        if (deltax == 0.0f) {
            x1p = 0.0f;
            x2p = 0.0f;
        }
        else {
            x1p = (pPos[i].x - minX) / deltax;
            x2p = (pPos[i+1].x - minX) / deltax;
        }
        if (deltay == 0.0f) {
            y1p = 0.0f;
            y2p = 0.0f;
        }
        else {
            y1p = (pPos[i].y - minY) / deltay;
            y2p = (pPos[i + 1].y - minY) / deltay;
        }
        float angle = (float)M_PI/2.0f;
        if (pPos[i+1].x != pPos[i].x) {
            float slope = (y2p - y1p)/(x2p - x1p);
            angle = std::atan(slope);
            if (pPos[i].x > pPos[i+1].x) {
                angle += (float)M_PI;
            }
        } else if (pPos[i+1].y < pPos[i].y) {
            angle += (float)M_PI;
        }
        float scale = std::sqrt((y2p - y1p)*(y2p - y1p) + (x2p - x1p)*(x2p - x1p));

        glm::mat3 scalingMatrix = glm::scale(glm::mat3(1.0f), glm::vec2(scale, 1.0));
        glm::mat3 rotationMatrix = glm::rotate(glm::mat3(1.0f), (float)angle);
        glm::mat3 translateMatrix = glm::translate(glm::mat3(1.0f), glm::vec2(x1p, y1p));
        glm::mat3 mat3 = translateMatrix * rotationMatrix * scalingMatrix;

        if (pPos[i].matrix != nullptr) {
            delete pPos[i].matrix;
        }
        pPos[i].matrix = new glm::mat3(mat3);

        // update any curves
        if( pPos[i].has_curve ) {
            pPos[i].curve->CreateNormalizedMatrix(minX, maxX, minY, maxY);
        }
    }

    // define the buffer positions
    SetBufferSize(1, (SingleNode?1:numLights));
    int chan = 0;
    int LastStringNum=-1;
    int ChanIncr = GetNodeChannelCount(StringType);
    for(idx=0; idx<(SingleNode?1:numLights); idx++) {
        if (Nodes[idx]->StringNum != LastStringNum) {
            LastStringNum=Nodes[idx]->StringNum;
            chan=stringStartChan[LastStringNum];
        }
        Nodes[idx]->ActChan=chan;
        chan+=ChanIncr;
        Nodes[idx]->Coords.resize(SingleNode?parm2:parm3);
        size_t CoordCount=GetCoordCount(idx);
        for(size_t c=0; c < CoordCount; c++) {
            Nodes[idx]->Coords[c].bufX=IsLtoR ? idx : (SingleNode ? idx : numLights-idx-1);
            Nodes[idx]->Coords[c].bufY=0;
        }
    }

    // place the nodes/coords along each line segment
    idx = IsLtoR ? 0 : numLights-1;
    float loc_x;
    if (hasIndivSeg) {
        // distribute the lights as defined by the polysize string
        if( SingleNode ) {
            int segment = 0;
            int seg_idx = 0;
            int count = 0;
            int num = Nodes[0].get()->Coords.size();
            float offset = 0.5f;
            if( num > 1 ) {
                offset -= 1.0f / (float)num;
            }
            size_t CoordCount=GetCoordCount(0);
            for(size_t c=0; c < CoordCount; c++) {
                if (num > 1) {
                    loc_x = seg_idx + offset + ((float)count / (float)num);
                    count++;
                } else {
                    loc_x = seg_idx + offset;
                }

                glm::vec3 v = *pPos[segment].matrix * glm::vec3(loc_x / (float)polyLineSizes[segment], 0, 1);

                Nodes[0]->Coords[c].screenX = v.x;
                Nodes[0]->Coords[c].screenY = v.y;
                seg_idx++;
                if( seg_idx >= polyLineSizes[segment] ) {
                    segment++;
                    seg_idx = 0;
                    count = 0;
                    for(int x=segment; x < polyLineSizes.size(); ++x ) {
                        if( polyLineSizes[x] == 0 ) {
                            segment++;
                        } else {
                            break;
                        }
                    }
                }
                if( pPos[segment].has_curve ) {
                    DistributeLightsAcrossCurveSegment(polyLineSizes[segment], segment, c, pPos );
                    segment++;
                    for(int x=segment; x < polyLineSizes.size(); ++x ) {
                        if( polyLineSizes[x] == 0 ) {
                            segment++;
                        } else {
                            break;
                        }
                    }
                }
            }
        } else {
            for(size_t m=0; m<num_segments; m++) {
                if( pPos[m].has_curve ) {
                    DistributeLightsAcrossCurveSegment(polyLineSizes[m], m, idx, pPos );
                } else {
                    int seg_idx = 0;
                    for(size_t n=0; n<polyLineSizes[m]; n++) {
                        int count = 0;
                        int num = Nodes[idx].get()->Coords.size();
                        float offset = 0.5f;
                        if( num > 1 ) {
                            offset -= 1.0f / (float)num;
                        }
                        size_t CoordCount=GetCoordCount(idx);
                        int x_pos = seg_idx;
                        for(size_t c=0; c < CoordCount; c++) {
                            if (num > 1) {
                                loc_x = x_pos + offset + ((float)count / (float)num);
                                count++;
                            } else {
                                loc_x = x_pos + offset;
                            }

                            glm::vec3 v = *pPos[m].matrix * glm::vec3(loc_x / (float)polyLineSizes[m], 0, 1);

                            Nodes[idx]->Coords[c].screenX = v.x;
                            Nodes[idx]->Coords[c].screenY = v.y;
                        }
                        IsLtoR ? idx++ : idx--;
                        seg_idx++;
                    }
                }
            }
        }
    } else {
        // distribute the lights evenly across the line segments
        int coords_per_node = Nodes[0].get()->Coords.size();
        int lights_to_distribute = SingleNode ? coords_per_node : numLights * coords_per_node;
        float offset = total_length / (float)lights_to_distribute;
        float current_pos = offset / 2.0f;
        idx = (IsLtoR || SingleNode) ? 0 : numLights-1;
        size_t c=0;
        int segment = 0;
        int sub_segment = 0;
        int last_seg_light_num = 0;
        float seg_start = current_pos;
        float segment_length = pPos[segment].has_curve ? pPos[segment].curve->GetSegLength(sub_segment) : pPos[segment].length;
        float seg_end = seg_start + segment_length;
        for(size_t m=0; m<lights_to_distribute; m++) {
            while( current_pos > seg_end ) {
                sub_segment++;
                if( pPos[segment].has_curve && (sub_segment < pPos[segment].curve->GetNumSegments()) ) {
                    seg_start = seg_end;
                    segment_length = pPos[segment].curve->GetSegLength(sub_segment);
                    seg_end = seg_start + segment_length;
                } else {
                    sub_segment = 0;
                    polyLineSizes[segment] = m - last_seg_light_num;
                    last_seg_light_num = m;
                    segment++;
                    seg_start = seg_end;
                    segment_length = pPos[segment].has_curve ? pPos[segment].curve->GetSegLength(sub_segment) : pPos[segment].length;
                    seg_end = seg_start + segment_length;
                    // If this is the last segment then just set the end really high so all remaining lights are on this segment
                    // This is required when using really large numbers of lights for each node ... root cause ... lack of resolution in floating point number
                    if (segment == polyLineSizes.size() - 1)
                    {
                        seg_end += 1000;
                    }
                }
            }
            glm::vec3 v;
            float pos = (current_pos - seg_start) / segment_length;
            if( pPos[segment].has_curve ) {
                v = *pPos[segment].curve->GetMatrix(sub_segment) * glm::vec3(pos, 0, 1);
            } else {
                v = *pPos[segment].matrix * glm::vec3(pos, 0, 1);
            }
            Nodes[idx]->Coords[c].screenX = v.x;
            Nodes[idx]->Coords[c].screenY = v.y;
            if( c < coords_per_node-1 ) {
                c++;
            } else {
                c = 0;
                IsLtoR ? idx++ : idx--;
            }
            current_pos += offset;
        }
        polyLineSizes[segment] = lights_to_distribute - last_seg_light_num;
    }
    screenLocation.SetRenderSize(1.0, 1.0);

    // cleanup curves and matrices
    for( int i = 0; i < num_points; ++i ) {
        if( pPos[i].has_curve ) {
            delete pPos[i].curve;
            pPos[i].curve = nullptr;
        }
        if (pPos[i].matrix != nullptr) {
            delete pPos[i].matrix;
        }
    }
}

void PolyLineModel::DistributeLightsAcrossCurveSegment(int lights, int segment, size_t &idx, std::vector<xlPolyPoint> &pPos )
{
    // distribute the lights evenly across the line segments
    int coords_per_node = Nodes[0].get()->Coords.size();
    int lights_to_distribute = SingleNode ? lights : lights * coords_per_node;
    float total_length = pPos[segment].curve->GetLength();
    float offset = total_length / (float)lights_to_distribute;
    float current_pos = offset / 2.0f;
    size_t c=0;
    int sub_segment = 0;
    float seg_start = current_pos;
    float segment_length = pPos[segment].curve->GetSegLength(sub_segment);
    float seg_end = seg_start + segment_length;
    for(size_t m=0; m<lights_to_distribute; m++) {
        while( current_pos > seg_end ) {
            sub_segment++;
            seg_start = seg_end;
            segment_length = pPos[segment].curve->GetSegLength(sub_segment);
            seg_end = seg_start + segment_length;
        }
        glm::vec3 v = *pPos[segment].curve->GetMatrix(sub_segment) * glm::vec3((current_pos - seg_start) / segment_length, 0, 1);
        if( SingleNode ) {
            Nodes[0]->Coords[idx].screenX = v.x;
            Nodes[0]->Coords[idx].screenY = v.y;
            IsLtoR ? idx++ : idx--;
        } else {
            Nodes[idx]->Coords[c].screenX = v.x;
            Nodes[idx]->Coords[c].screenY = v.y;
            if( c < coords_per_node-1 ) {
                c++;
            } else {
                c = 0;
                IsLtoR ? idx++ : idx--;
            }
        }
        current_pos += offset;
    }
}

static wxPGChoices LEFT_RIGHT;

void PolyLineModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    if (LEFT_RIGHT.GetCount() == 0) {
        LEFT_RIGHT.Add("Green Square");
        LEFT_RIGHT.Add("Blue Square");
    }
    wxPGProperty *p;
    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("# Lights", "PolyLineNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        p->Enable(!hasIndivSeg);
    } else {
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
            wxString nm = wxString::Format("Segment %d", x+1);
            grid->AppendIn(p, new wxUIntProperty(nm, SegAttrName(x), wxAtoi(ModelXml->GetAttribute(SegAttrName(x),""))));
        }
        if( segs_collapsed ) {
            grid->Collapse(p);
        }
    }
    else
    {
        for (int x = 0; x < 100; x++) {
            ModelXml->DeleteAttribute(SegAttrName(x));
        }
        // If we dont have individual segments ... then we dont have individual start channels
        ModelXml->DeleteAttribute("Advanced");
    }
}

int PolyLineModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("PolyLineNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%ld", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("PolyLineLights" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%ld", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("PolyLineStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 ? "L" : "R");
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if (event.GetPropertyName() == "ModelIndividualSegments") {
        ModelXml->DeleteAttribute("IndivSegs");
        if (event.GetValue().GetBool()) {
            hasIndivSeg = true;
            segs_collapsed = false;
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
        SetFromXml(ModelXml, zeroBased);
        IncrementChangeCount();
        return 3 | 0x0004 | 0x0008;
    } else if (event.GetPropertyName().StartsWith("ModelIndividualSegments.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());
        ModelXml->DeleteAttribute(str);
        ModelXml->AddAttribute(str, event.GetValue().GetString());
        SetFromXml(ModelXml, zeroBased);
        IncrementChangeCount();
        return 3 | 0x0004 | 0x0008;
    } else if (event.GetPropertyName() == "ModelIndividualStartChannels") {
        ModelXml->DeleteAttribute("Advanced");
        if (event.GetValue().GetBool()) {
            ModelXml->AddAttribute("Advanced", "1");
            int start_channel = 1;
            for (int x = 0; x < num_segments; x++) {
                if (ModelXml->GetAttribute(StartChanAttrName(x)) == "") {
                    ModelXml->DeleteAttribute(StartChanAttrName(x));
                    ModelXml->AddAttribute(StartChanAttrName(x), wxString::Format("%d",start_channel));
                }
                start_channel += polyLineSizes[x]*GetNodeChannelCount(StringType);
            }
        } else {
            for (int x = 0; x < num_segments; x++) {
                ModelXml->DeleteAttribute(StartChanAttrName(x));
            }
        }
        RecalcStartChannels();
        AdjustStringProperties(grid, num_segments);
        IncrementChangeCount();
        return 3 | 0x0008;
    }

    return Model::OnPropertyGridChange(grid, event);
}

int PolyLineModel::OnPropertyGridSelection(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if (event.GetPropertyName().StartsWith("ModelIndividualSegments.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());
        str = str.SubString(3, str.length());
        int segment = wxAtoi(str)-1;
        return segment;
    }
    return -1;
}

void PolyLineModel::OnPropertyGridItemCollapsed(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if (event.GetPropertyName() == "ModelIndividualSegments") {
        segs_collapsed = true;
    }
}

void PolyLineModel::OnPropertyGridItemExpanded(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if (event.GetPropertyName() == "ModelIndividualSegments") {
        segs_collapsed = false;
    }
}

void PolyLineModel::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "polylinemodel")
        {
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

            for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
            {
                if (n->GetName() == "stateInfo")
                {
                    AddState(n);
                }
                else if (n->GetName() == "subModel")
                {
                    AddSubmodel(n);
                }
            }

            int num_points = wxAtoi(pts);
            ModelXml->DeleteAttribute("NumPoints");
            ModelXml->AddAttribute("NumPoints", pts);

            // handle all the line segments
            ModelXml->DeleteAttribute("IndivSegs");
            if( is == "1" ) {
                ModelXml->AddAttribute("IndivSegs", "1");
                num_segments = num_points-1;
                for (int x = 0; x < num_segments; x++) {
                    ModelXml->DeleteAttribute(SegAttrName(x));
                    wxString seg = root->GetAttribute(SegAttrName(x), "");
                    // TODO this needs to be fixed like the individual start channel code in model
                    int seg_length = wxAtoi(seg);
                    ModelXml->AddAttribute(SegAttrName(x), wxString::Format("%d", seg_length));
                }
            }

            // read in the point data from xml
            std::vector<xlPolyPoint> pPos(num_points);
            wxArrayString point_array = wxSplit(point_data, ',');
            for( int i = 0; i < num_points; ++i ) {
                pPos[i].x = wxAtof(point_array[i*2]);
                pPos[i].y = wxAtof(point_array[i*2+1]);
                pPos[i].has_curve = false;
                pPos[i].curve = nullptr;
            }
            wxArrayString cpoint_array = wxSplit(cpoint_data, ',');
            int num_curves = cpoint_array.size() / 5;
            for( int i = 0; i < num_curves; ++i ) {
                int seg_num = wxAtoi(cpoint_array[i*5]);
                pPos[seg_num].has_curve = true;
                pPos[seg_num].curve = new BezierCurveCubic();
                pPos[seg_num].curve->set_p0(pPos[seg_num].x, pPos[seg_num].y);
                pPos[seg_num].curve->set_p1(pPos[seg_num+1].x, pPos[seg_num+1].y);
                pPos[seg_num].curve->set_cp0( wxAtof(cpoint_array[i*5+1]), wxAtof(cpoint_array[i*5+2]) );
                pPos[seg_num].curve->set_cp1( wxAtof(cpoint_array[i*5+3]), wxAtof(cpoint_array[i*5+4]) );
                pPos[seg_num].curve->SetScale(1.0, 1.0, 1.0);
                pPos[seg_num].curve->UpdatePoints();
            }

            float deltax = max_x-min_x;
            float deltay = max_y-min_y;

            // adjust points for new min/max
            for( int i = 0; i < num_points; ++i ) {
                pPos[i].x = (pPos[i].x * deltax) + min_x;
                pPos[i].y = (pPos[i].y * deltay) + min_y;
                if( pPos[i].has_curve ) {
                    float cp0x = pPos[i].curve->get_cp0x();
                    float cp0y = pPos[i].curve->get_cp0y();
                    float cp1x = pPos[i].curve->get_cp1x();
                    float cp1y = pPos[i].curve->get_cp1y();
                    cp0x = (cp0x * deltax) + min_x;
                    cp0y = (cp0y * deltay) + min_y;
                    cp1x = (cp1x * deltax) + min_x;
                    cp1y = (cp1y * deltay) + min_y;
                    pPos[i].curve->set_cp0(cp0x, cp0y);
                    pPos[i].curve->set_cp1(cp1x, cp1y);
                }
            }

            ModelXml->DeleteAttribute("PointData");
            ModelXml->DeleteAttribute("cPointData");
            point_data = "";
            for( int i = 0; i < num_points; ++i ) {
                point_data += wxString::Format( "%f,", pPos[i].x );
                point_data += wxString::Format( "%f", pPos[i].y );
                if( i != num_points-1 ) {
                    point_data += ",";
                }
            }
            cpoint_data = "";
            for( int i = 0; i < num_points; ++i ) {
                if( pPos[i].has_curve ) {
                    cpoint_data += wxString::Format( "%d,%f,%f,%f,%f,", i, pPos[i].curve->get_cp0x(), pPos[i].curve->get_cp0y(),
                                                               pPos[i].curve->get_cp1x(), pPos[i].curve->get_cp1y() );
                }
            }
            ModelXml->AddAttribute("PointData", point_data);
            ModelXml->AddAttribute("cPointData", cpoint_data);

            // cleanup curves
            for( int i = 0; i < num_points; ++i ) {
                if( pPos[i].has_curve ) {
                    delete pPos[i].curve;
                    pPos[i].curve = nullptr;
                }
            }

            GetModelScreenLocation().Read(ModelXml);

            xlights->MarkEffectsFileDirty(true);
        }
        else
        {
            wxMessageBox("Failure loading PolyLine model file.");
        }
    }
    else
    {
        wxMessageBox("Failure loading PolyLine model file.");
    }
}

#define retmsg(msg)  \
{ \
wxMessageBox(msg, _("Export Error")); \
return; \
}

void PolyLineModel::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) retmsg(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()));
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
    if (state != "")
    {
        f.Write(state);
    }
    wxString submodel = SerialiseSubmodel();
    if (submodel != "")
    {
        f.Write(submodel);
    }
    f.Write("</polylinemodel>");
    f.Close();
}

void PolyLineModel::NormalizePointData()
{
    // read in the point data from xml
    int num_points = wxAtoi(ModelXml->GetAttribute("NumPoints"));
    std::vector<xlPolyPoint> pPos(num_points);
    wxString point_data = ModelXml->GetAttribute("PointData");
    wxArrayString point_array = wxSplit(point_data, ',');
    for( int i = 0; i < num_points; ++i ) {
        pPos[i].x = wxAtof(point_array[i*2]);
        pPos[i].y = wxAtof(point_array[i*2+1]);
        pPos[i].has_curve = false;
        pPos[i].curve = nullptr;
    }
    wxString cpoint_data = ModelXml->GetAttribute("cPointData", "");
    wxArrayString cpoint_array = wxSplit(cpoint_data, ',');
    int num_curves = cpoint_array.size() / 5;
    for( int i = 0; i < num_curves; ++i ) {
        int seg_num = wxAtoi(cpoint_array[i*5]);
        pPos[seg_num].has_curve = true;
        pPos[seg_num].curve = new BezierCurveCubic();
        pPos[seg_num].curve->set_p0(pPos[seg_num].x, pPos[seg_num].y);
        pPos[seg_num].curve->set_p1(pPos[seg_num+1].x, pPos[seg_num+1].y);
        pPos[seg_num].curve->set_cp0( wxAtof(cpoint_array[i*5+1]), wxAtof(cpoint_array[i*5+2]) );
        pPos[seg_num].curve->set_cp1( wxAtof(cpoint_array[i*5+3]), wxAtof(cpoint_array[i*5+4]) );
        pPos[seg_num].curve->SetScale(1.0, 1.0, 1.0);
        pPos[seg_num].curve->UpdatePoints();
    }

    float minX = 100.0f;
    float minY = 100.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;

    for( int i = 0; i < num_points; ++i ) {
        if( pPos[i].x < minX ) minX = pPos[i].x;
        if( pPos[i].y < minY ) minY = pPos[i].y;
        if( pPos[i].x > maxX ) maxX = pPos[i].x;
        if( pPos[i].y > maxY ) maxY = pPos[i].y;
        if( pPos[i].has_curve ) {
            pPos[i].curve->check_min_max(minX, maxX, minY, maxY);
        }
    }
    float deltax = maxX-minX;
    float deltay = maxY-minY;

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
        if( pPos[i].has_curve ) {
            float cp0x = pPos[i].curve->get_cp0x();
            float cp0y = pPos[i].curve->get_cp0y();
            float cp1x = pPos[i].curve->get_cp1x();
            float cp1y = pPos[i].curve->get_cp1y();
            cp0x = (cp0x - minX) / deltax;
            cp0y = (cp0y - minY) / deltay;
            cp1x = (cp1x - minX) / deltax;
            cp1y = (cp1y - minY) / deltay;
            pPos[i].curve->set_cp0(cp0x, cp0y);
            pPos[i].curve->set_cp1(cp1x, cp1y);
        }
    }

    ModelXml->DeleteAttribute("PointData");
    ModelXml->DeleteAttribute("cPointData");
    point_data = "";
    for( int i = 0; i < num_points; ++i ) {
        point_data += wxString::Format( "%f,", pPos[i].x );
        point_data += wxString::Format( "%f", pPos[i].y );
        if( i != num_points-1 ) {
            point_data += ",";
        }
    }
    cpoint_data = "";
    for( int i = 0; i < num_points; ++i ) {
        if( pPos[i].has_curve ) {
            cpoint_data += wxString::Format( "%d,%f,%f,%f,%f,", i, pPos[i].curve->get_cp0x(), pPos[i].curve->get_cp0y(),
                                                       pPos[i].curve->get_cp1x(), pPos[i].curve->get_cp1y() );
        }
    }
    ModelXml->AddAttribute("PointData", point_data);
    ModelXml->AddAttribute("cPointData", cpoint_data);
}

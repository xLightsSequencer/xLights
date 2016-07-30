#include "PolyLineModel.h"
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/mat3x3.hpp>
#include "ModelScreenLocation.h"

PolyLineModel::PolyLineModel(const ModelManager &manager) : ModelWithScreenLocation(manager) {
    parm1 = parm2 = parm3 = 0;
    segs_collapsed = true;
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
        for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
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
    return polyLineSizes.size();
}

void PolyLineModel::SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString) {
    std::string tempstr = ModelXml->GetAttribute("Advanced", "0").ToStdString();
    bool HasIndividualStartChans=tempstr == "1";
    if( HasIndividualStartChans ) {
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
        parm1 = num_segments;
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
        ModelXml->AddAttribute("parm2", wxString::Format("%d", parm2));
    } else {
        parm1 = 1;
    }

    // reset node information
    Nodes.clear();
    SetNodeCount(1,numLights,rgbOrder);
    size_t NodeCount=GetNodeCount();

    // process our own start channels
    std::string tempstr = ModelXml->GetAttribute("Advanced", "0").ToStdString();
    bool HasIndividualStartChans=tempstr == "1";
    if( HasIndividualStartChans ) {
        int StartChannel = GetNumberFromChannelString(ModelXml->GetAttribute("StartChannel","1").ToStdString(), CouldComputeStartChannel);
        stringStartChan.clear();
        stringStartChan.resize(num_segments);
        for (int i=0; i<num_segments; i++) {
            tempstr = StartChanAttrName(i);
            if (!zeroBased && ModelXml->HasAttribute(tempstr)) {
                bool b = false;
                stringStartChan[i] = GetNumberFromChannelString(ModelXml->GetAttribute(tempstr, "1").ToStdString(), b)-1;
                CouldComputeStartChannel &= b;
            } else {
                stringStartChan[i] = (zeroBased? 0 : StartChannel-1) + polyLineSizes[i]*GetNodeChannelCount(StringType);
            }
        }
    }

    // fix the string numbers for each node since model is non-standard
    if (HasIndividualStartChans && hasIndivSeg) {
        int idx = 0;
        for (int x = 0; x < num_segments; x++) {
            for( int n = 0; n < polyLineSizes[x]; ++n ) {
                Nodes[idx++]->StringNum = x;
            }
        }
    }

    // read in the point data from xml
    std::vector<xlPolyPoint> pPos;
    pPos.resize(num_points);
    wxString point_data = ModelXml->GetAttribute("PointData", "0.4, 0.6, 0.4, 0.6");
    wxArrayString point_array = wxSplit(point_data, ',');
    for( int i = 0; i < num_points; ++i ) {
        pPos[i].x = wxAtof(point_array[i*2]);
        pPos[i].y = wxAtof(point_array[i*2+1]);
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
    }
    float deltax = maxX-minX;
    float deltay = maxY-minY;
    total_length = 0.0f;

    // normalize all points from 0.0 to 1.0 and create
    // a matrix for each line segment
    for( int i = 0; i < num_points-1; ++i ) {
        float x1p = (pPos[i].x - minX) / deltax;
        float x2p = (pPos[i+1].x - minX) / deltax;
        float y1p = (pPos[i].y - minY) / deltay;
        float y2p = (pPos[i+1].y - minY) / deltay;

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
        pPos[i].length = scale;
        total_length += scale;

        glm::mat3 scalingMatrix = glm::scale(glm::mat3(1.0f), glm::vec2(scale, 1.0));
        glm::mat3 rotationMatrix = glm::rotate(glm::mat3(1.0f), (float)angle);
        glm::mat3 translateMatrix = glm::translate(glm::mat3(1.0f), glm::vec2(x1p, y1p));
        glm::mat3 mat3 = translateMatrix * rotationMatrix * scalingMatrix;

        if (pPos[i].matrix != nullptr) {
            delete pPos[i].matrix;
        }
        pPos[i].matrix = new glm::mat3(mat3);
    }

    // define the buffer positions
    SetBufferSize(1, numLights);
    int chan = 0;
    int LastStringNum=-1;
    int ChanIncr=SingleChannel ?  1 : 3;
    for(size_t idx=0; idx<numLights; idx++) {
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
    int idx = 0;
    float loc_x;
    if (hasIndivSeg) {
        // distribute the lights as defined by the polysize string
        for(size_t m=0; m<num_segments; m++) {
            int seg_idx = 0;
            for(size_t n=0; n<polyLineSizes[m]; n++) {
                int count = 0;
                int num = Nodes[idx].get()->Coords.size();
                float offset = 0.5f;
                if( num > 1 ) {
                    offset -= 1.0f / (float)num;
                }
                size_t CoordCount=GetCoordCount(idx);
                int x_pos = IsLtoR ? seg_idx : (SingleNode ? seg_idx : polyLineSizes[m]-seg_idx-1);
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
                idx++;
                seg_idx++;
            }
        }
    } else {
        // distribute the lights evenly across the line segments
        int coords_per_node = Nodes[0].get()->Coords.size();
        int lights_to_distribute = numLights * coords_per_node;
        float offset = total_length / lights_to_distribute;
        float current_pos = offset / 2.0f;
        size_t idx=0;
        size_t c=0;
        int segment = 0;
        int last_seg_light_num = 0;
        float seg_start = current_pos;
        float seg_end = seg_start + pPos[segment].length;
        for(size_t m=0; m<lights_to_distribute; m++) {
            if( current_pos > seg_end ) {
                polyLineSizes[segment] = m - last_seg_light_num;
                last_seg_light_num = m - 1;
                segment++;
                seg_start = seg_end;
                seg_end = seg_start + pPos[segment].length;
            }
            glm::vec3 v = *pPos[segment].matrix * glm::vec3((current_pos - seg_start) / pPos[segment].length, 0, 1);
            Nodes[idx]->Coords[c].screenX = v.x;
            Nodes[idx]->Coords[c].screenY = v.y;
            if( c < coords_per_node-1 ) {
                c++;
            } else {
                c = 0;
                idx++;
            }
            current_pos += offset;
        }
    }
    screenLocation.SetRenderSize(1.0, 1.0);
}

static wxPGChoices LEFT_RIGHT;

void PolyLineModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    if (LEFT_RIGHT.GetCount() == 0) {
        LEFT_RIGHT.Add("Left");
        LEFT_RIGHT.Add("Right");
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

    p = grid->Append(new wxEnumProperty("Starting Location", "PolyLineStart", LEFT_RIGHT, IsLtoR ? 0 : 1));

    p = grid->Append(new wxBoolProperty("Indiv Segments", "ModelIndividualSegments", hasIndivSeg));
    p->SetAttribute("UseCheckbox", true);
    p->Enable(num_segments > 1);
    wxPGProperty *sp;
    if (hasIndivSeg) {
        for (int x = 0; x < num_segments; x++) {
            std::string val = ModelXml->GetAttribute(SegAttrName(x)).ToStdString();
            if (val == "") {
                val = wxString::Format("%d", polyLineSizes[x]);
                ModelXml->DeleteAttribute(SegAttrName(x));
                ModelXml->AddAttribute(SegAttrName(x), val);
            }
            wxString nm = wxString::Format("Segment %d", x+1);
            sp = grid->AppendIn(p, new wxUIntProperty(nm, SegAttrName(x), wxAtoi(ModelXml->GetAttribute(SegAttrName(x),""))));
        }
        if( segs_collapsed ) {
            grid->Collapse(p);
        }
    }
}
int PolyLineModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("PolyLineNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("PolyLineLights" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", event.GetPropertyValue().GetLong()));
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
            for (int x = 0; x < num_segments; x++) {
                if (ModelXml->GetAttribute(SegAttrName(x)) == "") {
                    ModelXml->DeleteAttribute(SegAttrName(x));
                    ModelXml->AddAttribute(SegAttrName(x), wxString::Format("%d", polyLineSizes[x]));
                }
            }
        } else {
            hasIndivSeg = false;
        }
        SetFromXml(ModelXml, zeroBased);
        IncrementChangeCount();
        return 3 | 0x0004;
    } else if (event.GetPropertyName().StartsWith("ModelIndividualSegments.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());
        ModelXml->DeleteAttribute(str);
        ModelXml->AddAttribute(str, event.GetValue().GetString());
        SetFromXml(ModelXml, zeroBased);
        IncrementChangeCount();
        return 3;
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

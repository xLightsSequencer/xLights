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
}
PolyLineModel::PolyLineModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    SetFromXml(node, zeroBased);
}
PolyLineModel::PolyLineModel(int lights, const Model &pbc, int strand, int node) : ModelWithScreenLocation(pbc.GetModelManager())
{
    Reset(lights, pbc, strand, node);
}

void PolyLineModel::Reset(int lights, const Model &pbc, int strand, int node, bool forceDirection)
{
    parm1 = lights;
    parm2 = 1;
    parm3 = 1;

    StringType = pbc.GetStringType();
    rgbOrder = pbc.rgbOrder;
    SingleNode = pbc.SingleNode;
    SingleChannel = pbc.SingleChannel;
    IsLtoR = pbc.GetIsLtoR();
    customColor = pbc.customColor;

    bool flip = false;
    if (forceDirection) {
        int sn = pbc.GetNodeStringNumber(pbc.MapToNodeIndex(strand, 0));
        for (int s = strand - 1; s >= 0; s--) {
            if (pbc.GetNodeStringNumber(pbc.MapToNodeIndex(s, 0)) == sn) {
                flip = !flip;
            } else {
                break;
            }
        }
    }
    stringStartChan.resize(lights);
    if (node == -1) {
        for (int x = 0; x < lights; x++) {
            stringStartChan[x] = pbc.NodeStartChannel(pbc.MapToNodeIndex(strand, x));
        }
    } else {
        stringStartChan[0] = pbc.NodeStartChannel(pbc.MapToNodeIndex(strand, node));
    }
    InitModel();
    for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
        (*it)->model = &pbc;
    }
    if (flip) {
        int l = 0;
        int r = Nodes.size() - 1;
        while (l < r) {
            Nodes[l].swap(Nodes[r]);
            l++;
            r--;
        }
    }
}

PolyLineModel::~PolyLineModel()
{
    //dtor
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

void PolyLineModel::SavePolyLineSizes() {
    ModelXml->DeleteAttribute("polyLineSizes");
    wxString tempstr = "";
    for( int i = 0; i < polyLineSizes.size(); ++i ) {
        if( i == polyLineSizes.size()-1 ) {
            tempstr += wxString::Format("%d", polyLineSizes[i]);
        } else {
            tempstr += wxString::Format("%d,", polyLineSizes[i]);
        }
    }
    ModelXml->AddAttribute("polyLineSizes", tempstr);
}

void PolyLineModel::InsertHandle(int after_handle) {
    std::string sizes = ModelXml->GetAttribute("polyLineSizes").ToStdString();
    if( sizes != "" ) {
        if( polyLineSizes.size() > after_handle ) {
            int segment1_size = polyLineSizes[after_handle] / 2;
            int segment2_size = polyLineSizes[after_handle] - segment1_size;
            polyLineSizes[after_handle] = segment1_size;
            polyLineSizes.insert(polyLineSizes.begin() + after_handle + 1, segment2_size);
            SavePolyLineSizes();
        }
    }
    GetModelScreenLocation().InsertHandle(after_handle);
}

void PolyLineModel::DeleteHandle(int handle) {
    std::string sizes = ModelXml->GetAttribute("polyLineSizes").ToStdString();
    if( sizes != "" ) {
        if( polyLineSizes.size() > handle ) {
            polyLineSizes.erase(polyLineSizes.begin() + handle);
            SavePolyLineSizes();
        }
    }
    GetModelScreenLocation().DeleteHandle(handle);
}
/*
void PolyLineModel::InitModel() {
    wxString tempstr=ModelXml->GetAttribute("polyLineSizes");
    polyLineSizes.resize(0);
    while (tempstr.size() > 0) {
        wxString t2 = tempstr;
        if (tempstr.Contains(",")) {
            t2 = tempstr.SubString(0, tempstr.Find(","));
            tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
        } else {
            tempstr = "";
        }
        long i2 = 0;
        t2.ToLong(&i2);
        polyLineSizes.resize(polyLineSizes.size() + 1);
        polyLineSizes[polyLineSizes.size() - 1] = i2;
    }

    Nodes.clear();
    InitPolyLine();

    int idx = 0;
    int numLights = parm1 * parm2;
    for(size_t m=0; m<num_segments; m++) {
        int seg_idx = 0;
        int end_node = idx + polyLineSizes[m];
        float scale = (float)longest_segment / (float)polyLineSizes[m];
        for(size_t n=idx; n<end_node; n++) {
            int count = 0;
            int num = Nodes[idx].get()->Coords.size();
            float offset = scale / 2.0;
            if( num > 1 ) {
                offset -= scale / (float)num;
            }
            size_t CoordCount=GetCoordCount(idx);
            int x_pos = IsLtoR ? seg_idx : (SingleNode ? seg_idx : polyLineSizes[m]-seg_idx-1);
            for(size_t c=0; c < CoordCount; c++) {
                Nodes[idx]->Coords[c].screenY = Nodes[idx]->Coords[c].bufY;
                if (num > 1) {
                    Nodes[idx]->Coords[c].screenX = x_pos * scale + offset + ((float)count * scale / (float)num);
                    count++;
                } else {
                    Nodes[idx]->Coords[c].screenX = x_pos * scale + offset;
                }
            }
            idx++;
            seg_idx++;
        }
    }

    screenLocation.SetRenderSize(BufferWi, BufferHt);
}
*/
void PolyLineModel::InitModel() {
    wxString tempstr=ModelXml->GetAttribute("polyLineSizes");
    polyLineSizes.resize(0);
    while (tempstr.size() > 0) {
        wxString t2 = tempstr;
        if (tempstr.Contains(",")) {
            t2 = tempstr.SubString(0, tempstr.Find(","));
            tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
        } else {
            tempstr = "";
        }
        long i2 = 0;
        t2.ToLong(&i2);
        polyLineSizes.resize(polyLineSizes.size() + 1);
        polyLineSizes[polyLineSizes.size() - 1] = i2;
    }

    Nodes.clear();
    InitPolyLine();

    std::vector<xlPolyPoint> pPos;
    int num_points = wxAtoi(ModelXml->GetAttribute("NumPoints", "2"));
    pPos.resize(num_points);
    wxString point_data = ModelXml->GetAttribute("PointData", "0.4, 0.6, 0.4, 0.6");
    wxArrayString point_array = wxSplit(point_data, ',');
    for( int i = 0; i < num_points; ++i ) {
        pPos[i].x = wxAtof(point_array[i*2]);
        pPos[i].y = wxAtof(point_array[i*2+1]);
    }

    float minX = 100.0;
    float minY = 100.0;
    float maxX = 0.0;
    float maxY = 0.0;

    for( int i = 0; i < num_points; ++i ) {
        if( pPos[i].x < minX ) minX = pPos[i].x;
        if( pPos[i].y < minY ) minY = pPos[i].y;
        if( pPos[i].x > maxX ) maxX = pPos[i].x;
        if( pPos[i].y > maxY ) maxY = pPos[i].y;
    }
    float deltax = maxX-minX;
    float deltay = maxY-minY;

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

        glm::mat3 scalingMatrix = glm::scale(glm::mat3(1.0f), glm::vec2(scale, 1.0));
        glm::mat3 rotationMatrix = glm::rotate(glm::mat3(1.0f), (float)angle);
        glm::mat3 translateMatrix = glm::translate(glm::mat3(1.0f), glm::vec2(x1p, y1p));
        glm::mat3 mat3 = translateMatrix * rotationMatrix * scalingMatrix;

        if (pPos[i].matrix != nullptr) {
            delete pPos[i].matrix;
        }
        pPos[i].matrix = new glm::mat3(mat3);
    }

    int idx = 0;
    int numLights = parm1 * parm2;
    float loc_x, loc_y;
    for(size_t m=0; m<num_segments; m++) {
        int seg_idx = 0;
        int end_node = idx + polyLineSizes[m];
        float scale = (float)longest_segment / (float)polyLineSizes[m];
        for(size_t n=idx; n<end_node; n++) {
            int count = 0;
            int num = Nodes[idx].get()->Coords.size();
            float offset = scale / 2.0;
            if( num > 1 ) {
                offset -= scale / (float)num;
            }
            size_t CoordCount=GetCoordCount(idx);
            int x_pos = IsLtoR ? seg_idx : (SingleNode ? seg_idx : polyLineSizes[m]-seg_idx-1);
            for(size_t c=0; c < CoordCount; c++) {
                loc_y = Nodes[idx]->Coords[c].bufY;
                if (num > 1) {
                    loc_x = x_pos * scale + offset + ((float)count * scale / (float)num);
                    count++;
                } else {
                    loc_x = x_pos * scale + offset;
                }

                glm::vec3 v = *pPos[m].matrix * glm::vec3(loc_x / (float)BufferWi, 0, 1);

                Nodes[idx]->Coords[c].screenX = v.x;
                Nodes[idx]->Coords[c].screenY = v.y;
            }
            idx++;
            seg_idx++;
        }
    }

    screenLocation.SetRenderSize(1.0, 1.0);
}

// initialize buffer coordinates
// parm1=Number of Strings
// parm2=Pixels Per Stringp
void PolyLineModel::InitPolyLine() {
    SetNodeCount(parm1,parm2,rgbOrder);
    size_t NodeCount=GetNodeCount();

    int numLights = parm1 * parm2;
	num_segments = wxAtoi(ModelXml->GetAttribute("NumPoints", "2")) - 1;
    int nodes_per_segment = NodeCount/num_segments;
    int remainder = NodeCount - nodes_per_segment*num_segments;
    int nodes_this_segment = nodes_per_segment + ((remainder-- > 0) ? 1 : 0);

    //SetBufferSize(num_segments,(SingleNode?parm1:nodes_this_segment));
    int LastStringNum=-1;
    int chan = 0,idx;
    int ChanIncr=SingleChannel ?  1 : 3;

    longest_segment = 0;
    int cnt = 0;

    if (polyLineSizes.size() == 0) {
        polyLineSizes.resize(num_segments);
        longest_segment = nodes_this_segment;
        for(size_t m=0; m<num_segments; m++) {
            polyLineSizes[m] = nodes_this_segment;
            nodes_this_segment = nodes_per_segment + ((remainder-- > 0) ? 1 : 0);
        }
    } else {
        for (int x = 0; x < polyLineSizes.size(); x++) {
            if ((cnt + polyLineSizes[x]) > numLights) {
                polyLineSizes[x] = numLights - cnt;
            }
            cnt += polyLineSizes[x];
            if (polyLineSizes[x] > longest_segment) {
                longest_segment = polyLineSizes[x];
            }
        }
    }
    while( polyLineSizes.size() < num_segments ) {
        polyLineSizes.resize(polyLineSizes.size() + 1);
        polyLineSizes[polyLineSizes.size() - 1] = 0;
    }

    SetBufferSize(num_segments, longest_segment);

    idx = 0;
    for(size_t m=0; m<num_segments; m++) {
        int seg_idx = 0;
        int end_node = idx + polyLineSizes[m];
        float scale = (float)longest_segment / (float)polyLineSizes[m];
        for(size_t n=idx; n<end_node; n++) {
            if (Nodes[idx]->StringNum != LastStringNum) {
                LastStringNum=Nodes[idx]->StringNum;
                chan=stringStartChan[LastStringNum];
            }
            Nodes[idx]->ActChan=chan;
            chan+=ChanIncr;
            Nodes[idx]->Coords.resize(SingleNode?parm2:parm3);
            size_t CoordCount=GetCoordCount(idx);
            int location = seg_idx * scale + scale / 2.0;
            for(size_t c=0; c < CoordCount; c++) {
                Nodes[idx]->Coords[c].bufX=IsLtoR ? location : (SingleNode ? location : longest_segment-location-1);
                Nodes[idx]->Coords[c].bufY=m;
            }
            idx++;
            seg_idx++;
        }
    }
}

static wxPGChoices LEFT_RIGHT;

void PolyLineModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    if (LEFT_RIGHT.GetCount() == 0) {
        LEFT_RIGHT.Add("Left");
        LEFT_RIGHT.Add("Right");
    }
    wxPGProperty *p = grid->Append(new wxUIntProperty("# Strings", "SingleLineCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("Lights/String", "SingleLineNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "SingleLineNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxUIntProperty("Lights/Node", "SingleLineLights", parm3));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 300);
        p->SetEditor("SpinCtrl");
    }

    p = grid->Append(new wxEnumProperty("Starting Location", "SingleLineStart", LEFT_RIGHT, IsLtoR ? 0 : 1));
    p = grid->Append(new wxStringProperty("Segment Sizes", "PolyLayerSizes", ModelXml->GetAttribute("polyLineSizes")));
}
int PolyLineModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("SingleLineCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("SingleLineNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("SingleLineLights" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("SingleLineStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 ? "L" : "R");
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("PolyLayerSizes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("polyLineSizes");
        ModelXml->AddAttribute("polyLineSizes", event.GetValue().GetString());
        SetFromXml(ModelXml, zeroBased);
        return 3;
    }

    return Model::OnPropertyGridChange(grid, event);
}


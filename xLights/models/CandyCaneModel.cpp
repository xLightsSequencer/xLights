#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "CandyCaneModel.h"

CandyCaneModel::CandyCaneModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased) : _reverse(false)
{
    SetFromXml(node, netInfo, zeroBased);
}

CandyCaneModel::~CandyCaneModel()
{
}


void CandyCaneModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    wxPGProperty *p = grid->Append(new wxUIntProperty("# Canes", "CandyCaneCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 20);
    p->SetEditor("SpinCtrl");
    
    p = grid->Append(new wxUIntProperty("Nodes Per Cane", "CandyCaneNodes", parm2));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 250);
    p->SetEditor("SpinCtrl");
    
    p = grid->Append(new wxUIntProperty("Lights Per Node", "CandyCaneLights", parm3));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 250);
    p->SetEditor("SpinCtrl");

    //p = grid->Append(new wxUIntProperty("Arc Degrees", "ArchesArc", arc));
    //p->SetAttribute("Min", 1);
    //p->SetAttribute("Max", 180);
    //p->SetEditor("SpinCtrl");

	p = grid->Append(new wxBoolProperty("Reverse", "CandyCaneReverse", _reverse));
	p->SetEditor("CheckBox");
    p->Enable(!_sticks);

	p = grid->Append(new wxBoolProperty("Sticks", "CandyCaneSticks", _sticks));
	p->SetEditor("CheckBox");
    
}

int CandyCaneModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("CandyCaneCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, *ModelNetInfo, zeroBased);
        return 3;
    } else if ("CandyCaneNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, *ModelNetInfo, zeroBased);
        return 3;
    } else if ("CandyCaneLights" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, *ModelNetInfo, zeroBased);
        return 3;
    } else if ("CandyCaneReverse" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("CandyCaneReverse");
		ModelXml->AddAttribute("CandyCaneReverse", event.GetPropertyValue().GetBool()? "true" : "false");
        SetFromXml(ModelXml, *ModelNetInfo, zeroBased);
        return 3;
    }
	else if ("CandyCaneSticks" == event.GetPropertyName()) {
		ModelXml->DeleteAttribute("CandyCaneSticks");
		ModelXml->AddAttribute("CandyCaneSticks", event.GetPropertyValue().GetBool() ? "true" : "false");
		SetFromXml(ModelXml, *ModelNetInfo, zeroBased);
        grid->GetPropertyByName("CandyCaneReverse")->Enable(!event.GetPropertyValue().GetBool());
		return 3;
	}
    return Model::OnPropertyGridChange(grid, event);
}

void CandyCaneModel::GetBufferSize(const std::string &type, const std::string &transform, int &BufferWi, int &BufferHi) const {
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = this->BufferWi * this->BufferHt;
        AdjustForTransform(transform, BufferWi, BufferHi);
    } else {
        Model::GetBufferSize(type, transform, BufferWi, BufferHi);
    }
}
void CandyCaneModel::InitRenderBufferNodes(const std::string &type,  const std::string &transform,
                                        std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi) const {
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = GetNodeCount();
        
        int NumCanes=parm1;
        int SegmentsPerCane=parm2;
        int cur = 0;
        for (int y=0; y < NumCanes; y++) {
            for(int x=0; x<SegmentsPerCane; x++) {
                int idx = y * SegmentsPerCane + x;
                newNodes.push_back(NodeBaseClassPtr(Nodes[idx]->clone()));
                for(size_t c=0; c < newNodes[cur]->Coords.size(); c++) {
                    newNodes[cur]->Coords[c].bufX=cur;
                    newNodes[cur]->Coords[c].bufY=0;
                }
                cur++;
            }
        }
        ApplyTransform(transform, newNodes, BufferWi, BufferHi);
    } else {
        Model::InitRenderBufferNodes(type, transform, newNodes, BufferWi, BufferHi);
    }
}

void CandyCaneModel::InitModel() {
    int NumCanes=parm1;
    int SegmentsPerCane=parm2;
	_reverse = (ModelXml->GetAttribute("CandyCaneReverse", "false") == "true");
	_sticks = (ModelXml->GetAttribute("CandyCaneSticks", "false") == "true");

    SetBufferSize(NumCanes,SegmentsPerCane);
    if (SingleNode) 
	{
        SetNodeCount(NumCanes, SegmentsPerCane*parm3, rgbOrder);
    } else 
	{
        SetNodeCount(NumCanes, SegmentsPerCane, rgbOrder);
        if (parm3 > 1) 
		{
            for (int x = 0; x < Nodes.size(); x++) 
			{
                Nodes[x]->Coords.resize(parm3);
            }
        }
    }
    SetRenderSize(NumCanes,SegmentsPerCane);
    
    for (int y=0; y < NumCanes; y++) {
        for(int x=0; x<SegmentsPerCane; x++) {
            int idx = y * SegmentsPerCane + x;
            Nodes[idx]->ActChan = stringStartChan[y] + x*GetNodeChannelCount(StringType);
            Nodes[idx]->StringNum=y;
            for(size_t c=0; c < GetCoordCount(idx); c++) {
                Nodes[idx]->Coords[c].bufX=y;
                Nodes[idx]->Coords[c].bufY=x;
            }
        }
    }
    SetCaneCoord();
}

int CandyCaneModel::MapToNodeIndex(int strand, int node) const {
    return strand * parm2 + node;
}
int CandyCaneModel::GetNumStrands() const {
     return parm1;
}
int CandyCaneModel::CalcCannelsPerString() {
    SingleChannel = false;
    return GetNodeChannelCount(StringType) * parm2;
}
inline double toRadians(long degrees) {
    return 2.0*M_PI*double(degrees)/360.0;
}
void CandyCaneModel::SetCaneCoord() {
	int NumCanes = parm1;
	int SegmentsPerCane = parm2;
	int LightsPerNode = parm3;
	int lightspercane = SegmentsPerCane * LightsPerNode;
    
	double height;
    double width;
	if (_sticks)
	{
		height = lightspercane;
        width = NumCanes;
        for (int x = 0; x < NumCanes; x++){
            int y = 0;
            for (size_t n = 0; n < SegmentsPerCane; n++) {
                size_t CoordCount = GetCoordCount(n);
                for (size_t c = 0; c < CoordCount; c++) {
                    Nodes[n + x * SegmentsPerCane]->Coords[c].screenX = x - (double)NumCanes/2.0 + 0.5;
                    Nodes[n + x * SegmentsPerCane]->Coords[c].screenY = y - height/2.0;
                    y++;
                }
            }
        }
	} else {
        double caneGap = 2.0;
        int upright = SegmentsPerCane * 6.0 / 9.0;
        upright *= parm3;
        double widthPerCane = double(lightspercane)*3.0/9.0;
        width = (double)NumCanes*widthPerCane + (NumCanes - 1) * caneGap;
        height = lightspercane - widthPerCane/2.0;

        int arclights = lightspercane - upright;
        for (int i = 0; i < NumCanes; i++) {
            // draw the uprights
            double x = i*(widthPerCane + caneGap) - (double)(NumCanes - 1.0)*(widthPerCane + caneGap)/2.0 - widthPerCane/2.0;
            if (_reverse) {
                x += widthPerCane;
            }
            double y = -height/2.0;
            int curLight = 0;
            int curNode = 0;
            while (curLight < upright) {
                size_t CoordCount = GetCoordCount(curNode);
                for (size_t c = 0; c < CoordCount; c++) {
                    Nodes[curNode + i * SegmentsPerCane]->Coords[c].screenX = x;
                    Nodes[curNode + i * SegmentsPerCane]->Coords[c].screenY = y;
                    y++;
                    curLight++;
                }
                curNode++;
            }
            y--;
            if (_reverse) {
                x -= widthPerCane/2;
            } else {
                x += widthPerCane/2;
            }
            while (curLight < lightspercane) {
                size_t CoordCount = GetCoordCount(curNode);
                for (size_t c = 0; c < CoordCount; c++)
                {
                    // drawing left to right
                    double angle = M_PI - M_PI * (curLight - upright + 1) / arclights;
                    double y2 = sin(angle)*widthPerCane/2;
                    double x2 = cos(angle)*widthPerCane/2;
                    if (_reverse) {
                        Nodes[curNode + i * SegmentsPerCane]->Coords[c].screenX = x - x2;
                    } else {
                        Nodes[curNode + i * SegmentsPerCane]->Coords[c].screenX = x + x2;
                    }
                    Nodes[curNode + i * SegmentsPerCane]->Coords[c].screenY = y + y2;
                    curLight++;
                }
                curNode++;
            }
        }
    }
    SetRenderSize(height, width);
}

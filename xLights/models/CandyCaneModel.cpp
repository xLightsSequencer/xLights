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

static wxPGChoices LEFT_RIGHT;

void CandyCaneModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    //if (LEFT_RIGHT.GetCount() == 0) {
    //    LEFT_RIGHT.Add("Left");
    //    LEFT_RIGHT.Add("Right");
    //}
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
        SetNodeCount(NumCanes * SegmentsPerCane, parm3, rgbOrder);
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
                Nodes[idx]->Coords[c].bufX=IsLtoR ? x : SegmentsPerCane-x-1;
                Nodes[idx]->Coords[c].bufY=isBotToTop ? y : NumCanes-y-1;
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
	double x, y;
	int NumCanes = parm1;
	int SegmentsPerCane = parm2;
	int LightsPerNode = parm3;
	int numlights = NumCanes * SegmentsPerCane * LightsPerNode;
	int lightspercane = SegmentsPerCane * LightsPerNode;
	size_t NodeCount = GetNodeCount();
	double stickheight = numlights;
	double upright = lightspercane * 2.0 / 3.0;
	int uprightlights = upright;
	int arclights = lightspercane - uprightlights;
	double canewidth = (double)lightspercane / (2.0 * (double)NumCanes - 1.0);
	double width = canewidth * (2.0 * (double)NumCanes - 1.0);
	double radius = canewidth / 2.0;
	double height;
	if (_sticks)
	{
		height = lightspercane;
	}
	else
	{
		height = upright + canewidth / 2.0;
	}
	SetRenderSize(height, width);

	// TODO	this needs work;
	if (_sticks)
	{
		for (int i = 0; i < NumCanes; i++)
		{
			x = (-1 * width / 2.0) + 2.0 * i * canewidth;
			for (size_t n = 0; n < lightspercane; n++)
			{
				size_t CoordCount = GetCoordCount(n);
				for (size_t c = 0; c < CoordCount; c++)
				{
					y = ((double)lightspercane * (double)n) / (double)lightspercane - (lightspercane / 2.0);
					Nodes[n + i * lightspercane]->Coords[c].screenX = x;
					Nodes[n + i * lightspercane]->Coords[c].screenY = y;
				}
			}
		}
	}
	else
	{
		#define ARCXSCALE 0.6 // this controls how far off a circle the crook is
		if (!_reverse)
		{
			for (int i = 0; i < NumCanes; i++)
			{
				// draw the uprights
				x = (-1.0 * width / 2.0) + 2.0 * (double)i * canewidth;
				for (size_t n = 0; n < uprightlights; n++)
				{
					size_t CoordCount = GetCoordCount(n);
					for (size_t c = 0; c < CoordCount; c++)
					{
						y = upright * (double)n / upright - (height / 2.0);
						Nodes[n + i * lightspercane]->Coords[c].screenX = x;
						Nodes[n + i * lightspercane]->Coords[c].screenY = y;
					}
				}

				// draw the hook
				for (size_t n = uprightlights; n < lightspercane; n++)
				{
					size_t CoordCount = GetCoordCount(n);
					for (size_t c = 0; c < CoordCount; c++)
					{
						// drawing left to right
						double angle = M_PI - M_PI * (n - uprightlights) / arclights;
						y = upright - height / 2.0 + radius * sin(angle);
						x = (-1 * width / 2.0) + 
							(2.0 * i * canewidth) + 
							radius * ARCXSCALE * cos(angle) + 
							radius * ARCXSCALE;
						Nodes[n + i * lightspercane]->Coords[c].screenX = x;
						Nodes[n + i * lightspercane]->Coords[c].screenY = y;
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < NumCanes; i++)
			{
				// draw the uprights
				x = (-1.0 * width / 2.0) + (2.0 * (double)i + 1.0) * canewidth;
				for (size_t n = 0; n < uprightlights; n++)
				{
					size_t CoordCount = GetCoordCount(n);
					for (size_t c = 0; c < CoordCount; c++)
					{
						y = upright * (double)n / upright - (height / 2.0);
						Nodes[n + i * lightspercane]->Coords[c].screenX = x;
						Nodes[n + i * lightspercane]->Coords[c].screenY = y;
					}
				}

				// draw the hook
				for (size_t n = uprightlights; n < lightspercane; n++)
				{
					size_t CoordCount = GetCoordCount(n);
					for (size_t c = 0; c < CoordCount; c++)
					{
						// drawing left to right
						double angle = M_PI * (n - uprightlights) / arclights;
						y = upright - height / 2.0 + radius * sin(angle);
						x = (-1 * width / 2.0) +
							((2.0 * i + 1.0) * canewidth) +
							radius * ARCXSCALE * cos(angle) -
							radius * ARCXSCALE;
						Nodes[n + i * lightspercane]->Coords[c].screenX = x;
						Nodes[n + i * lightspercane]->Coords[c].screenY = y;
					}
				}
			}
		}
	}
}

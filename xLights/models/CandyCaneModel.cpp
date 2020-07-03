/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "../OutputModelManager.h"
#include "CandyCaneModel.h"
#include "ModelScreenLocation.h"

#include <log4cpp/Category.hh>

CandyCaneModel::CandyCaneModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    screenLocation.SetModelHandleHeight(true);
    screenLocation.SetSupportsAngle(true);
    SetFromXml(node, zeroBased);
}

CandyCaneModel::~CandyCaneModel()
{
}

static const char *LEFT_RIGHT_VALUES[] = {
    "Green Square", 
    "Blue Square"
};
static wxPGChoices LEFT_RIGHT(wxArrayString(2, LEFT_RIGHT_VALUES));

void CandyCaneModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    wxPGProperty *p = grid->Append(new wxUIntProperty("# Canes", "CandyCaneCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 20);
    p->SetEditor("SpinCtrl");
    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("Lights Per Cane", "CandyCaneNodes", parm3));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 250);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes Per Cane", "CandyCaneNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 250);
        p->SetEditor("SpinCtrl");
    }
    p = grid->Append(new wxUIntProperty("Lights Per Node", "CandyCaneLights", parm3));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 250);
    p->SetEditor("SpinCtrl");
    if (SingleNode) {
        p->Hide(true);
    }
    p = grid->Append(new wxFloatProperty("Height", "CandyCaneHeight", _caneheight));
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Cane Rotation", "CandyCaneSkew", screenLocation.GetAngle()));
    p->SetAttribute("Min", -180 );
    p->SetAttribute("Max", 180);
    p->SetEditor("SpinCtrl");

	p = grid->Append(new wxBoolProperty("Reverse", "CandyCaneReverse", _reverse));
	p->SetEditor("CheckBox");
    p->Enable(!_sticks);

	p = grid->Append(new wxBoolProperty("Sticks", "CandyCaneSticks", _sticks));
	p->SetEditor("CheckBox");

    p = grid->Append(new wxBoolProperty("Alternate Nodes", "AlternateNodes", _alternateNodes));
    p->SetEditor("CheckBox");
    if (SingleNode) {
        p->Enable(false);
    }

    grid->Append(new wxEnumProperty("Starting Location", "CandyCaneStart", LEFT_RIGHT, IsLtoR ? 0 : 1));
}

void CandyCaneModel::UpdateTypeProperties(wxPropertyGridInterface* grid) {
    if (SingleNode) {
        grid->GetPropertyByName("CandyCaneLights")->Hide(true);
        grid->GetPropertyByName("AlternateNodes")->Enable(false);
    }
    else
    {
        grid->GetPropertyByName("CandyCaneLights")->Hide(false);
        grid->GetPropertyByName("AlternateNodes")->Enable();
    }

    grid->GetPropertyByName("CandyCaneReverse")->Enable(!_sticks);
}

int CandyCaneModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("CandyCaneCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        //AdjustStringProperties(grid, event.GetPropertyValue().GetLong());
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CandyCaneModel::OnPropertyGridChange::CandyCaneCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CandyCaneModel::OnPropertyGridChange::CandyCaneCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CandyCaneModel::OnPropertyGridChange::CandyCaneCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CandyCaneModel::OnPropertyGridChange::CandyCaneCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "CandyCaneModel::OnPropertyGridChange::CandyCaneCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CandyCaneModel::OnPropertyGridChange::CandyCaneCount");
        AddASAPWork(OutputModelManager::WORK_UPDATE_PROPERTYGRID, "ArchesModel::OnPropertyGridChange::CandyCaneCount");
        if (ModelXml->GetAttribute("Advanced", "0") == "1")
        {
            AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "CandyCaneModel::OnPropertyGridChange::CandyCaneCount");
        }
        return 0;
    } else if ("CandyCaneNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CandyCaneModel::OnPropertyGridChange::CandyCaneNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CandyCaneModel::OnPropertyGridChange::CandyCaneNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CandyCaneModel::OnPropertyGridChange::CandyCaneNodes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CandyCaneModel::OnPropertyGridChange::CandyCaneNodes");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "CandyCaneModel::OnPropertyGridChange::CandyCaneNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CandyCaneModel::OnPropertyGridChange::CandyCaneNodes");
        return 0;
    } else if ("CandyCaneLights" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CandyCaneModel::OnPropertyGridChange::CandyCaneLights");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CandyCaneModel::OnPropertyGridChange::CandyCaneLights");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CandyCaneModel::OnPropertyGridChange::CandyCaneLights");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "CandyCaneModel::OnPropertyGridChange::CandyCaneLights");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CandyCaneModel::OnPropertyGridChange::CandyCaneLights");
        return 0;
    } else if ("CandyCaneReverse" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("CandyCaneReverse");
		ModelXml->AddAttribute("CandyCaneReverse", event.GetPropertyValue().GetBool()? "true" : "false");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CandyCaneModel::OnPropertyGridChange::CandyCaneReverse");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CandyCaneModel::OnPropertyGridChange::CandyCaneReverse");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CandyCaneModel::OnPropertyGridChange::CandyCaneReverse");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CandyCaneModel::OnPropertyGridChange::CandyCaneReverse");
        return 0;
    } else if ("CandyCaneSkew" == event.GetPropertyName()) {
        screenLocation.SetAngle(event.GetPropertyValue().GetLong());
        ModelXml->DeleteAttribute("Angle");
        ModelXml->AddAttribute("Angle", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CandyCaneModel::OnPropertyGridChange::CandyCaneSkew");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CandyCaneModel::OnPropertyGridChange::CandyCaneSkew");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CandyCaneModel::OnPropertyGridChange::CandyCaneSkew");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CandyCaneModel::OnPropertyGridChange::CandyCaneSkew");
        return 0;
    } else if ("CandyCaneHeight" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("CandyCaneHeight");
        ModelXml->AddAttribute("CandyCaneHeight", wxString::Format("%lf", event.GetPropertyValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CandyCaneModel::OnPropertyGridChange::CandyCaneHeight");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CandyCaneModel::OnPropertyGridChange::CandyCaneHeight");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CandyCaneModel::OnPropertyGridChange::CandyCaneHeight");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CandyCaneModel::OnPropertyGridChange::CandyCaneHeight");
        return 0;
    } else if ("CandyCaneSticks" == event.GetPropertyName()) {
		ModelXml->DeleteAttribute("CandyCaneSticks");
		ModelXml->AddAttribute("CandyCaneSticks", event.GetPropertyValue().GetBool() ? "true" : "false");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CandyCaneModel::OnPropertyGridChange::CandyCaneSticks");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CandyCaneModel::OnPropertyGridChange::CandyCaneSticks");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CandyCaneModel::OnPropertyGridChange::CandyCaneSticks");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CandyCaneModel::OnPropertyGridChange::CandyCaneSticks");
        AddASAPWork(OutputModelManager::WORK_UPDATE_PROPERTYGRID, "CandyCaneModel::OnPropertyGridChange::CandyCaneSticks");
        return 0;
    } else if ("AlternateNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("AlternateNodes");
        ModelXml->AddAttribute("AlternateNodes", event.GetPropertyValue().GetBool() ? "true" : "false");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CandyCaneModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CandyCaneModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CandyCaneModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CandyCaneModel::OnPropertyGridChange::AlternateNodes");
        return 0;
    } else if ("CandyCaneStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 ? "L" : "R");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CandyCaneModel::OnPropertyGridChange::CandyCaneStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CandyCaneModel::OnPropertyGridChange::CandyCaneStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CandyCaneModel::OnPropertyGridChange::CandyCaneStart");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CandyCaneModel::OnPropertyGridChange::CandyCaneStart");
    } else if (event.GetPropertyName() == "ModelStringType") {
        wxPGProperty *p = grid->GetPropertyByName("CandyCaneLights");
        p->Hide(SingleNode);
        p = grid->GetPropertyByName("CandyCaneNodes");
        if (SingleNode) {
            p->SetLabel("Lights Per Cane");
        } else {
            p->SetLabel("Nodes Per Cane");
        }
    }
    return Model::OnPropertyGridChange(grid, event);
}

void CandyCaneModel::GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi) const {
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = this->BufferWi * this->BufferHt;
        AdjustForTransform(transform, BufferWi, BufferHi);
    } else {
        Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi);
    }
}

void CandyCaneModel::InitRenderBufferNodes(const std::string &type, const std::string &camera,  const std::string &transform,
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
        Model::InitRenderBufferNodes(type, camera, transform, newNodes, BufferWi, BufferHi);
    }
}

void CandyCaneModel::InitModel() {
    int NumCanes=parm1;
    int SegmentsPerCane=parm2;
	_reverse = (ModelXml->GetAttribute("CandyCaneReverse", "false") == "true");
	_sticks = (ModelXml->GetAttribute("CandyCaneSticks", "false") == "true");
    _alternateNodes = (ModelXml->GetAttribute("AlternateNodes", "false") == "true");
    if (ModelXml->HasAttribute("CandyCaneSkew")) {
        ModelXml->DeleteAttribute("CandyCaneSkew");
        int skew = wxAtoi(ModelXml->GetAttribute("CandyCaneSkew", "0"));
        screenLocation.SetAngle(skew);
    }
    _caneheight = wxAtof(ModelXml->GetAttribute("CandyCaneHeight", "1.0"));

    SetNodeCount(NumCanes, SegmentsPerCane, rgbOrder);
    if (SingleNode) {
        SegmentsPerCane = 1;
        parm3 = parm2;
        parm2 = 1;
    } else {
        if (parm3 > 1)
		{
            for (size_t x = 0; x < Nodes.size(); x++)
			{
                Nodes[x]->Coords.resize(parm3);
            }
        }
    }
    SetBufferSize(SegmentsPerCane, NumCanes);

    if (!IsLtoR) {
        for (int y = 0; y < (NumCanes / 2); y++) {
            int i = stringStartChan[y];
            stringStartChan[y] = stringStartChan[NumCanes - 1 - y];
            stringStartChan[NumCanes - 1 - y] = i;
        }
    }

    for (int y=0; y < NumCanes; y++) {
        for(int x=0; x<SegmentsPerCane; x++) {
            int idx = y * SegmentsPerCane + x;
            Nodes[idx]->ActChan = stringStartChan[y] + x*GetNodeChannelCount(StringType);
            Nodes[idx]->StringNum=y;
            for(size_t c=0; c < GetCoordCount(idx); c++) {
                Nodes[idx]->Coords[c].bufX=y;
                if (_alternateNodes)
                {
                    if (x + 1 <= (SegmentsPerCane + 1) / 2)
                    {
                        Nodes[idx]->Coords[c].bufY = 2 * x;
                    }
                    else
                    {
                        Nodes[idx]->Coords[c].bufY = (SegmentsPerCane - (x + 1)) * 2 + 1;
                    }
                }
                else
                {
                    Nodes[idx]->Coords[c].bufY = x;
                }
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
    if (SingleNode) {
        return GetNodeChannelCount(StringType);
    }
    return GetNodeChannelCount(StringType) * parm2;
}

static void rotate_point(float cx,float cy, float angle, float &x, float &y)
{
    float s = sin(angle);
    float c = cos(angle);

    // translate point back to origin:
    x -= cx;
    y -= cy;

    // rotate point
    float xnew = x * c - y * s;
    float ynew = x * s + y * c;

    // translate point back:
    x = xnew + cx;
    y = ynew + cy;
}

void CandyCaneModel::SetCaneCoord() {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int NumCanes = parm1;
    size_t SegmentsPerCane = parm2;
    int LightsPerNode = parm3;

    int lightspercane = SegmentsPerCane * LightsPerNode;
    float angle = toRadians(screenLocation.GetAngle());

    double height;
    double width;


    double caneGap = 2.0;
    int upright = SegmentsPerCane * 6.0 / 9.0;
    upright *= parm3;
    if (SingleNode) {
        upright = parm3 * 6.0 / 9.0;
    }
    double widthPerCane = double(lightspercane)*3.0/9.0;
    width = (double)NumCanes*widthPerCane + (NumCanes - 1) * caneGap;
    height = lightspercane - widthPerCane/2.0;

    if (_sticks) {
        height = lightspercane * _caneheight;
        for (int i = 0; i < NumCanes; i++){
            int y = 0;
            double x = i*(widthPerCane + caneGap) + widthPerCane / 2.0;
            for (size_t n = 0; n < SegmentsPerCane; n++) {
                size_t CoordCount = GetCoordCount(n);
                if (SingleNode)
                {
                    auto node = n + i * SegmentsPerCane;
                    for (size_t c = 0; c < CoordCount; c++) {
                        Nodes[node]->Coords[c].screenX = x;
                        Nodes[node]->Coords[c].screenY = _caneheight * (float)y * screenLocation.GetHeight();
                        rotate_point(x, 0, angle,
                            Nodes[node]->Coords[c].screenX,
                            Nodes[node]->Coords[c].screenY);
                        y++;
                    }
                }
                else
                {
                    auto node = FindNodeAtXY(i, y / parm3);
                    for (size_t c = 0; c < CoordCount; c++) {
                        if (node == -1)
                        {
                            logger_base.error("Candy Cane buffer x,y %d, %d not found.", i, y);
                        }
                        else
                        {
                            Nodes[node]->Coords[c].screenX = x;
                            Nodes[node]->Coords[c].screenY = _caneheight * (float)y * screenLocation.GetHeight();
                            rotate_point(x, 0, angle,
                                Nodes[node]->Coords[c].screenX,
                                Nodes[node]->Coords[c].screenY);
                        }
                        y++;
                    }
                }
            }
        }
    } else {
        int arclights = lightspercane - upright;
        for (int i = 0; i < NumCanes; i++) {
            // draw the uprights
            double x = i*(widthPerCane + caneGap);
            if (_reverse) {
                x += widthPerCane;
            }
            double y = 0;
            int curLight = 0;
            int curNode = 0;
            double cx = x;
            if (_reverse) {
                cx -= widthPerCane/2 * screenLocation.GetHeight();
            } else {
                cx += widthPerCane/2 * screenLocation.GetHeight();
            }

            double ox = x;
            while (curLight < upright) {
                size_t CoordCount = GetCoordCount(curNode);
                if (SingleNode)
                {
                    CoordCount = upright;
                    int node = curNode + i * SegmentsPerCane;
                    for (size_t c = 0; c < CoordCount; c++) {
                        Nodes[node]->Coords[c].screenX = x;
                        Nodes[node]->Coords[c].screenY = _caneheight * (float)y * screenLocation.GetHeight();
                        rotate_point(x, 0, angle,
                            Nodes[node]->Coords[c].screenX,
                            Nodes[node]->Coords[c].screenY);
                        y++;
                        curLight++;
                    }
                }
                else
                {
                    auto node = FindNodeAtXY(i, y / parm3);
                    for (size_t c = 0; c < CoordCount; c++) {
                        if (node == -1)
                        {
                            logger_base.error("Candy Cane buffer x,y %d, %d not found.", i, y);
                        }
                        else
                        {
                            Nodes[node]->Coords[c].screenX = x;
                            Nodes[node]->Coords[c].screenY = _caneheight * (float)y * screenLocation.GetHeight();
                            rotate_point(x, 0, angle,
                                Nodes[node]->Coords[c].screenX,
                                Nodes[node]->Coords[c].screenY);
                        }
                        y++;
                        curLight++;
                    }
                    curNode++;
                }
            }
            y--;
            x = cx;
            while (curLight < lightspercane) {
                size_t CoordCount = GetCoordCount(curNode);
                size_t c = 0;
                if (SingleNode)
                {
                    CoordCount = lightspercane;
                    c = curLight;
                    int node = curNode + i * SegmentsPerCane;
                    for (; c < CoordCount; c++)
                    {
                        // drawing left to right
                        double aangle = M_PI - M_PI * (curLight - upright + 1) / arclights;
                        double y2 = sin(aangle) * widthPerCane / 2 * screenLocation.GetHeight();
                        double x2 = cos(aangle) * widthPerCane / 2 * screenLocation.GetHeight();
                        if (_reverse) {
                            Nodes[node]->Coords[c].screenX = x - x2;
                        }
                        else {
                            Nodes[node]->Coords[c].screenX = x + x2;
                        }
                        Nodes[node]->Coords[c].screenY = _caneheight * (float)(y * screenLocation.GetHeight() + y2);
                        rotate_point(ox, 0, angle,
                            Nodes[node]->Coords[c].screenX,
                            Nodes[node]->Coords[c].screenY);
                        curLight++;
                    }
                }
                else
                {
                    auto node = FindNodeAtXY(i, curLight / parm3);
                    for (; c < CoordCount; c++)
                    {
                        // drawing left to right
                        double aangle = M_PI - M_PI * (curLight - upright + 1) / arclights;
                        double y2 = sin(aangle) * widthPerCane / 2 * screenLocation.GetHeight();
                        double x2 = cos(aangle) * widthPerCane / 2 * screenLocation.GetHeight();
                        if (node == -1)
                        {
                            logger_base.error("Candy Cane buffer x,y %d, %d not found.", i, curLight);
                        }
                        else
                        {
                            if (_reverse) {
                                Nodes[node]->Coords[c].screenX = x - x2;
                            }
                            else {
                                Nodes[node]->Coords[c].screenX = x + x2;
                            }
                            Nodes[node]->Coords[c].screenY = _caneheight * (float)(y * screenLocation.GetHeight() + y2);
                            rotate_point(ox, 0, angle,
                                Nodes[node]->Coords[c].screenX,
                                Nodes[node]->Coords[c].screenY);
                        }
                        curLight++;
                    }
                }
                curNode++;
            }
        }
    }
    float min = 99999;
    float max = -9999;
    for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
        for (auto it2 = (*it)->Coords.begin(); it2 != (*it)->Coords.end(); ++it2) {
            min = std::min(min, it2->screenY);
            max = std::max(max, it2->screenY);
        }
    }

    screenLocation.SetRenderSize(width, height);
}

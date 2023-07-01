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
#include <wx/filedlg.h>
#include <wx/file.h>
#include <wx/log.h>
#include <wx/msgdlg.h>

#include "StarModel.h"
#include "../xLightsVersion.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"
#include "../ModelPreview.h"

#include <math.h>

std::vector<std::string> StarModel::STAR_BUFFER_STYLES;

// Converts from the old start position formats into the new one
std::string StarModel::ConvertFromDirStartSide(const wxString& dir, const wxString& startSide)
{
    if (dir == "L" && startSide == "B") {
        return "Bottom Ctr-CW";
    }
    if (dir == "L") {
        return "Top Ctr-CCW";
    }

    if (startSide == "B") {
        return "Bottom Ctr-CCW";
    }

    return "Top Ctr-CW";
}

StarModel::StarModel(wxXmlNode* node, const ModelManager& manager, bool zeroBased) : ModelWithScreenLocation(manager), starRatio(2.618034f)
{
    // convert old star sizes to new Layer sizes setting
    if (node->GetAttribute("starSizes", "") != "") {
        node->DeleteAttribute("LayerSizes");
        node->AddAttribute("LayerSizes", node->GetAttribute("starSizes", ""));
        node->DeleteAttribute("starSizes");
    }

    auto dir = node->GetAttribute("Dir", "");
    auto startSide = node->GetAttribute("StartSide", "");
    auto startStartLocation = node->GetAttribute("StarStartLocation", "");

    if (startStartLocation == "" && (dir != "" || startSide != "")) {
        if (dir == "") dir = "L";
        if (startSide == "") startSide = "B";

        node->DeleteAttribute("Dir");
        node->DeleteAttribute("StartSide");
        node->AddAttribute("StarStartLocation", ConvertFromDirStartSide(dir, startSide));
    }

    SetFromXml(node, zeroBased);
}

StarModel::~StarModel()
{
}

const std::vector<std::string> &StarModel::GetBufferStyles() const {
    struct Initializer {
        Initializer() {
            STAR_BUFFER_STYLES = Model::DEFAULT_BUFFER_STYLES;
            STAR_BUFFER_STYLES.push_back("Layer Star");
        }
    };
    static Initializer ListInitializationGuard;
    return STAR_BUFFER_STYLES;
}

void StarModel::GetBufferSize(const std::string& type, const std::string& camera, const std::string& transform, int& BufferWi, int& BufferHi, int stagger) const
{
    if (type == "Layer Star") {
        BufferHi = GetNumStrands();
        BufferWi = 0;
        for (int x = 0; x < BufferHi; x++) {
            int w = GetStarSize(x);
            if (w > BufferWi) {
                BufferWi = w;
            }
        }
        AdjustForTransform(transform, BufferWi, BufferHi);
    }
    else if (SingleChannel || SingleNode) {
        BufferHi = GetNumStrands();
        BufferWi = 1;
        AdjustForTransform(transform, BufferWi, BufferHi);
    }
    else {
        Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi, stagger);
    }
}

void StarModel::InitRenderBufferNodes(const std::string& type,
    const std::string& camera,
    const std::string& transform,
    std::vector<NodeBaseClassPtr>& newNodes, int& BufferWi, int& BufferHi, int stagger, bool deep) const
{
    if (type == "Layer Star") {
        BufferHi = GetNumStrands();
        BufferWi = 0;
        for (int x = 0; x < BufferHi; x++) {
            int w = GetStarSize(x);
            if (w > BufferWi) {
                BufferWi = w;
            }
        }
        for (const auto& it : Nodes) {
            newNodes.push_back(NodeBaseClassPtr(it.get()->clone()));
        }

        int start = 0;
        for (int cur = 0; cur < GetLayerSizeCount(); cur++) {

            int layer = cur;
            if (!Contains(_starStartLocation, "Inside")) {
                layer = (GetLayerSizeCount() - cur) - 1;
            }

            int numlights = GetLayerSize(layer);
            if (numlights == 0) {
                continue;
            }

            for (size_t cnt = 0; cnt < numlights; cnt++) {
                int n;
                if (!SingleNode) {
                    n = cnt + start;
                } else {
                    n = cur;
                }
                if (n >= Nodes.size()) {
                    n = Nodes.size() - 1;
                }
                for (auto& it : newNodes[n]->Coords) {
                    it.bufY = layer;
                    it.bufX = cnt * BufferWi / numlights;
                }
            }
            start += numlights;
        }
        ApplyTransform(transform, newNodes, BufferWi, BufferHi);
    }
    else if (SingleChannel || SingleNode) {
        // I am not 100% about this change but it makes sense to me
        // While the custom model may have a height and width if it is single channel then the render buffer really should be Nodes x 1
        // and all nodes should point to one cell.
        // Without this change effects like twinkle do really strange things
        Model::InitRenderBufferNodes(type, camera, transform, newNodes, BufferWi, BufferHi, stagger);
        BufferHi = Nodes.size();
        BufferWi = 1;
        int x = 0;
        for (auto& it : Nodes) {
            for (auto& it2 : it->Coords) {
                it2.bufX = 0;
                it2.bufY = x;
            }
            x++;
        }
        return;
    }
    else {
        Model::InitRenderBufferNodes(type, camera, transform, newNodes, BufferWi, BufferHi, stagger);
    }
}

int StarModel::GetStrandLength(int strand) const
{
    if (SingleNode) {
        return 1;
    }
    else {
        if (Contains(_starStartLocation, "Inside")) {
            return GetStarSize(GetLayerSizeCount() - 1 - strand);
        }
        else {
            return GetStarSize(strand);
        }
    }
}

int StarModel::MapToNodeIndex(int strand, int node) const
{
    int idx = 0;
    for (int x = GetLayerSizeCount() - 1; x > strand; x--) {
        idx += GetStrandLength(x);
    }
    idx += node;
    return idx;
}

int StarModel::GetNumStrands() const {
    return GetLayerSizeCount();
}

int StarModel::GetMappedStrand(int strand) const {
    if (GetLayerSizeCount() != 0) {
        return GetLayerSizeCount() - strand - 1;
    }
    return strand;
}

bool StarModel::AllNodesAllocated() const
{
    int allocated = 0;
    for (const auto& it : GetLayerSizes()) {
        allocated += it;
    }

    return (allocated == GetNodeCount());
}

// parm3 is number of points
// top left=top ccw, top right=top cw, bottom left=bottom cw, bottom right=bottom ccw

wxRealPoint StarModel::GetPointOnCircle(double radius, double angle)
{
    return wxRealPoint(radius * std::sin(angle), radius * std::cos(angle));
}

double StarModel::LineLength(wxRealPoint start, wxRealPoint end)
{
    return std::sqrt((end.x - start.x) * (end.x - start.x) + (end.y - start.y) * (end.y - start.y));
}

wxRealPoint StarModel::GetPositionOnLine(wxRealPoint start, wxRealPoint end, double distance)
{
    if (LineLength(start, end) == 0) return start;
    double t = distance / LineLength(start, end);
    return wxRealPoint(((1.0 - t) * start.x + t * end.x), ((1.0 - t) * start.y + t * end.y));
}

void StarModel::InitModel()
{
    starRatio = wxAtof(ModelXml->GetAttribute("starRatio", "2.618034"));
    _starStartLocation = ModelXml->GetAttribute("StarStartLocation", "Bottom Ctr-CW");
    innerPercent = wxAtoi(ModelXml->GetAttribute("starCenterPercent", "-1"));

    if (parm3 < 2) parm3 = 2; // need at least 2 arms
    SetNodeCount(parm1, parm2, rgbOrder);

    // Found a problem where a user had multiple layer sizes but just 1 string and set to RGB dumb string type.
    // I think the commented out code would fix this but I am not sure it would work in all situations.
    // It needs more testing and late november is not a good time to be doing it. So throwing an assertion in
    // If this fires for us a lot when there is nothing wrong with our models then we will know the code is bad and we wont implement it.
    // Maybe you can help fix the condition at that time ... rather than just commenting out the assert.
    // wxASSERT(starSizes.size() <= Nodes.size());
    //if (starSizes.size() > Nodes.size())
    //{
    //    starSizes.resize(Nodes.size());
    //}

    // stars are drawn using pairs of circles. The outer radius touches the edge of the grid.
    // the inside is proportionate to the ratio
    // each layer is then applied inside the prior one by some factor
    // the radius of the outer circle starts are bufferWi / 2

    int numlights = parm1 * parm2;
    if (numlights == 0) return;
    if (GetLayerSizeCount() == 0) {
        SetLayerSizeCount(1);
    }
    if (GetLayerSizeCount() == 1) {
        SetLayerSize(0, numlights);
    }

    int maxLightsOnLayer = 0;
    for (int l = 0; l < GetLayerSizeCount(); l++) {
        // we inflate a layer for every layer outside it by 1 / number of layers ... so 5th layer of 10 should be inflated by 50%
        int layersoutside = GetLayerSizeCount() - l - 1;
        maxLightsOnLayer = std::max(maxLightsOnLayer, 1 + (int)((float)GetLayerSize(l) * (1.0 + ((float)layersoutside / (float)GetLayerSizeCount()))));
    }
    SetBufferSize(maxLightsOnLayer, maxLightsOnLayer);

    double outerRadius = (double)BufferWi / 2.0; // stars are 2 circles ... and inner and an outer with lines travelling between them
    if (starRatio < 1) starRatio = 1;
    double innerRadius = outerRadius / starRatio;
    int layerCount = GetLayerSizeCount();
    double layerRadiusDelta = 0;
    if (layerCount > 1) {
        if (innerPercent == -1) {
            innerPercent = 100.0f / layerCount;
        }
        layerRadiusDelta = (outerRadius * (float)(100.0f-innerPercent)) / (100.0f * ((float)layerCount - 1.0f)); // space between the outer layer radii
    }
    if (parm3 == 0) parm3 = 1;
    double pointAngleGap = (M_PI * 2.0) / parm3; // angle between star points
    double directionUnit = Contains(_starStartLocation, "-CCW") ? -1.0 : 1.0; // which way the angle should be applied
    double startAngle;
    if (Contains(_starStartLocation, "Top")) { // head
        startAngle = (M_PI * 2.0 * 0.0) / 4.0;
    } else if (Contains(_starStartLocation, "Bottom Ctr")) { // crotch
        startAngle = (M_PI * 2.0 * 2.0) / 4.0;
    } else if (Contains(_starStartLocation, "Left")) { // left leg
        startAngle = (M_PI * 2.0 * 2.0) / 4.0;
        if (parm3 % 2 == 1) {
            startAngle += pointAngleGap / 2.0;
        }
    } else { // Right leg
        startAngle = (M_PI * 2.0 * 2.0) / 4.0;
        if (parm3 % 2 == 1) {
            startAngle -= pointAngleGap / 2.0;
        }
    }
    int starSegments = 2 * parm3; // parm3 is points
    int channelsPerNode = GetNodeChannelCount(StringType);
    int coordsPerNode = GetCoordCount(0);
    if (coordsPerNode == 0) return;

    int startLayer = GetLayerSizeCount() - 1;
    int endLayer = -1;
    int layerIncr = -1;
    if (Contains(_starStartLocation, "Inside")) {
        // when inside we process the layers in reverse
        startLayer = 0;
        endLayer = GetLayerSizeCount();
        layerIncr = 1;
        outerRadius -= layerRadiusDelta * (GetLayerSizeCount() - 1);
        innerRadius = outerRadius / starRatio;
        layerRadiusDelta *= -1;
    }

    uint32_t chan = 0;
    int currentNode = 0;

    if (!SingleNode) {
        for (int l = startLayer; l != endLayer; l+= layerIncr) {

            if (currentNode >= Nodes.size()) break;

            int layerNodes = GetLayerSize(l);
            int endNodeForLayer = currentNode + layerNodes;

            if (layerNodes == 0) continue;

            bool startOuter = !Contains(_starStartLocation, "Bottom Ctr");

            // segments are all the same length so i can calculate length once
            wxRealPoint start = GetPointOnCircle(startOuter ? outerRadius : innerRadius, startAngle);
            wxRealPoint end = GetPointOnCircle(startOuter ? innerRadius : outerRadius, startAngle + (pointAngleGap / 2.0));
            double segmentLength = LineLength(start, end);
            double totalSegmentLength = starSegments * segmentLength;
            double coordGap = totalSegmentLength / (layerNodes * coordsPerNode);

            double curPos = 0; // This is our position along the stretched out lines of the star
            double curAngle = startAngle; // This is the angle on the circle of the starting point for each segment
            double segStartLen = 0;
            double segEndLen = 0;
            for (int s = 0; s < starSegments; s++) {

                if (currentNode >= Nodes.size()) break;

                start = GetPointOnCircle(startOuter ? outerRadius : innerRadius, curAngle);
                end = GetPointOnCircle(startOuter ? innerRadius : outerRadius, curAngle + (pointAngleGap * directionUnit) / 2.0);
                segStartLen = segEndLen;
                segEndLen = segStartLen + segmentLength;
                if (s == starSegments - 1) {
                    // last segment so beware rounding issues ... so bump it slightly
                    segEndLen += 0.001;
                }

                while (curPos < segEndLen && currentNode < endNodeForLayer) {

                    int currentString = currentNode / parm2;
                    int nodeInString = currentNode % parm2;
                    if (nodeInString == 0 && currentString < GetNumStrings()) {
                        chan = stringStartChan[currentString];
                    }
                    Nodes[currentNode]->ActChan = chan;

                    for (int c = 0; c < coordsPerNode; c++) {
                        wxRealPoint point = GetPositionOnLine(start, end, curPos - segStartLen);

                        Nodes[currentNode]->Coords[c].bufX = point.x + BufferWi / 2;
                        Nodes[currentNode]->Coords[c].bufY = point.y + BufferHt / 2 - 1;

                        Nodes[currentNode]->Coords[c].screenX = point.x;
                        Nodes[currentNode]->Coords[c].screenY = point.y;

                        curPos += coordGap;
                    }

                    chan += channelsPerNode;


                    currentNode++;
                    if (currentNode >= Nodes.size()) break;
                }

                // move to the next arm
                curAngle += (pointAngleGap * directionUnit) / 2.0;
                startOuter = !startOuter;
            }

            // step in
            outerRadius -= layerRadiusDelta;
            innerRadius = outerRadius / starRatio;
        }

        // handle any left over nodes
        for (int n = currentNode; n < Nodes.size(); n++) {
            int currentString = n / parm2;
            int nodeInString = n % parm2;
            if (nodeInString == 0) {
                chan = stringStartChan[currentString];
            }
            Nodes[n]->ActChan = chan;

            for (int c = 0; c < coordsPerNode; c++) {
                Nodes[n]->Coords[c].bufX = 0;
                Nodes[n]->Coords[c].bufY = 0;
            }

            if (!SingleNode) {
                chan += channelsPerNode;
            }
        }
    } else {
        for (int l = startLayer; l != endLayer; l += layerIncr) {

            wxRealPoint lastCoord; // we remember this so any excess coords are placed with the last coord

            if (currentNode >= Nodes.size()) break;

            int layerNodes = 1;

            // we need to use the min of layer size and string length
            int coordsPerNode = std::min(GetLayerSize(l), (int)GetCoordCount(currentNode));
            if (coordsPerNode == 0) continue;

            chan = stringStartChan[currentNode];

            bool startOuter = !Contains(_starStartLocation, "Bottom Ctr");

            // segments are all the same length so i can calculate length once
            wxRealPoint start = GetPointOnCircle(startOuter ? outerRadius : innerRadius, startAngle);
            wxRealPoint end = GetPointOnCircle(startOuter ? innerRadius : outerRadius, startAngle + (pointAngleGap / 2.0));
            double segmentLength = LineLength(start, end);
            double totalSegmentLength = starSegments * segmentLength;
            double coordGap = totalSegmentLength / (layerNodes * coordsPerNode);

            int currentCoord = 0;
            double curPos = 0; // This is our position along the stretched out lines of the star
            double curAngle = startAngle; // This is the angle on the circle of the starting point for each segment
            for (int s = 0; s < starSegments; s++) {

                if (currentCoord >= coordsPerNode) break;

                start = GetPointOnCircle(startOuter ? outerRadius : innerRadius, curAngle);
                end = GetPointOnCircle(startOuter ? innerRadius : outerRadius, curAngle + (pointAngleGap * directionUnit) / 2.0);
                double segStartLen = s * segmentLength;
                double segEndLen = segStartLen + segmentLength;

                while (curPos < segEndLen) {

                    Nodes[currentNode]->ActChan = chan;

                    wxRealPoint point = GetPositionOnLine(start, end, curPos - segStartLen);

                    Nodes[currentNode]->Coords[currentCoord].bufX = point.x + BufferWi / 2;
                    Nodes[currentNode]->Coords[currentCoord].bufY = point.y + BufferHt / 2 - 1;
                    Nodes[currentNode]->Coords[currentCoord].screenX = point.x;
                    Nodes[currentNode]->Coords[currentCoord].screenY = point.y;
                    lastCoord = wxPoint(Nodes[currentNode]->Coords[currentCoord].bufX, Nodes[currentNode]->Coords[currentCoord].bufY);

                    curPos += coordGap;

                    currentCoord++;
                    if (currentCoord >= coordsPerNode) break;
                }

                // move to the next arm
                curAngle += (pointAngleGap * directionUnit) / 2.0;
                startOuter = !startOuter;
            }

            coordsPerNode = GetCoordCount(currentNode);

            // handle any left over nodes
            for (int c = currentCoord; c < coordsPerNode; c++) {
                Nodes[currentNode]->ActChan = chan;

                Nodes[currentNode]->Coords[c].bufX = lastCoord.x;
                Nodes[currentNode]->Coords[c].bufY = lastCoord.y;
                Nodes[currentNode]->Coords[c].screenX = lastCoord.x - BufferWi / 2;
                Nodes[currentNode]->Coords[c].screenY = lastCoord.y - BufferHt / 2;
            }

            currentNode++;

            // step in
            outerRadius -= layerRadiusDelta;
            innerRadius = outerRadius / starRatio;
        }
    }
    GetModelScreenLocation().SetRenderSize(BufferWi, BufferHt, GetModelScreenLocation().GetRenderDp());
    screenLocation.RenderDp = 10.0f;  // give the bounding box a little depth
}

static const char* TOP_BOT_LEFT_RIGHT_VALUES[] = { 
        "Top Ctr-CCW",
        "Top Ctr-CW",
        "Top Ctr-CCW Inside",
        "Top Ctr-CW Inside",
        "Bottom Ctr-CW",
        "Bottom Ctr-CCW",
        "Bottom Ctr-CW Inside",
        "Bottom Ctr-CCW Inside",
        "Left Bottom-CW",
        "Left Bottom-CCW",
        "Right Bottom-CW",
        "Right Bottom-CCW"
};

static wxPGChoices TOP_BOT_LEFT_RIGHT(wxArrayString(12, TOP_BOT_LEFT_RIGHT_VALUES));

void StarModel::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    wxPGProperty* p = grid->Append(new wxUIntProperty("# Strings", "StarStringCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 640);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("Lights/String", "StarLightCount", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "StarLightCount", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        p->SetHelpString("This is typically the total number of pixels per #String.");
    }

    p = grid->Append(new wxUIntProperty("# Points", "StarStrandCount", parm3));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 250);
    p->SetEditor("SpinCtrl");

    int ssl = 0;
    for (size_t i = 0; i < TOP_BOT_LEFT_RIGHT.GetCount(); i++) {
        if (TOP_BOT_LEFT_RIGHT[i].GetText() == _starStartLocation) {
            ssl = i;
            break;
        }
    }

    grid->Append(new wxEnumProperty("Starting Location", "StarStart", TOP_BOT_LEFT_RIGHT, ssl));
    AddLayerSizeProperty(grid);

    p = grid->Append(new wxFloatProperty("Outer to Inner Ratio", "StarRatio", starRatio));
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    if (GetLayerSizeCount() > 1) {
        p = grid->Append(new wxUIntProperty("Inner Layer %", "StarCenterPercent", innerPercent));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 100);
        p->SetEditor("SpinCtrl");
    }
}

int StarModel::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if ("StarStringCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        //AdjustStringProperties(grid, parm1);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::OnPropertyGridChange::StarStringCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::OnPropertyGridChange::StarStringCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "StarModel::OnPropertyGridChange::StarStringCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "StarModel::OnPropertyGridChange::StarStringCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "StarModel::OnPropertyGridChange::StarStringCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "StarModel::OnPropertyGridChange::StarStringCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "StarModel::OnPropertyGridChange::StarStringCount");
        return 0;
    } else if ("StarLightCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::OnPropertyGridChange::StarLightCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::OnPropertyGridChange::StarLightCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "StarModel::OnPropertyGridChange::StarLightCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "StarModel::OnPropertyGridChange::StarLightCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "StarModel::OnPropertyGridChange::StarLightCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "StarModel::OnPropertyGridChange::StarLightCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "StarModel::OnPropertyGridChange::StarLightCount");
        return 0;
    } else if ("StarStrandCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::OnPropertyGridChange::StarStrandCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::OnPropertyGridChange::StarStrandCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "StarModel::OnPropertyGridChange::StarStrandCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "StarModel::OnPropertyGridChange::StarStrandCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "StarModel::OnPropertyGridChange::StarStrandCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "StarModel::OnPropertyGridChange::StarStrandCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "StarModel::OnPropertyGridChange::StarStrandCount");
        return 0;
    } else if ("StarStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("StarStartLocation");
        ModelXml->AddAttribute("StarStartLocation", TOP_BOT_LEFT_RIGHT_VALUES[event.GetValue().GetLong()]);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::OnPropertyGridChange::StarStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::OnPropertyGridChange::StarStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "StarModel::OnPropertyGridChange::StarStart");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "StarModel::OnPropertyGridChange::StarStart");
        return 0;
    } else if ("StarCenterPercent" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("starCenterPercent");
        ModelXml->AddAttribute("starCenterPercent", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::OnPropertyGridChange::CircleCenterPercent");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::OnPropertyGridChange::CircleCenterPercent");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "StarModel::OnPropertyGridChange::CircleCenterPercent");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "StarModel::OnPropertyGridChange::CircleCenterPercent");
        return 0;
    } else if ("StarRatio" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("starRatio");
        ModelXml->AddAttribute("starRatio", wxString::Format("%lf", event.GetValue().GetDouble()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::OnPropertyGridChange::StarRatio");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::OnPropertyGridChange::StarRatio");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "StarModel::OnPropertyGridChange::StarRatio");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "StarModel::OnPropertyGridChange::StarRatio");
        return 0;
    }

    return Model::OnPropertyGridChange(grid, event);
}

void StarModel::OnLayerSizesChange(bool countChanged)
{
    // if string count is 1 then adjust nodes per string to match sum of nodes
    if (parm1 == 1) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)GetLayerSizesTotalNodes()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "StarModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "StarModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "StarModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "StarModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "StarModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "StarModel::OnLayerSizesChange");
    }
}

void StarModel::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    //    bool isnew = !FileExists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
    wxString p1 = ModelXml->GetAttribute("parm1");
    wxString p2 = ModelXml->GetAttribute("parm2");
    wxString p3 = ModelXml->GetAttribute("parm3");
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency", "0");
    wxString mb = ModelXml->GetAttribute("ModelBrightness", "0");
    wxString a = ModelXml->GetAttribute("Antialias");
    wxString ls = ModelXml->GetAttribute("LayerSizes");
    wxString sr = ModelXml->GetAttribute("starRatio", "2.618034");
    wxString scp = ModelXml->GetAttribute("starCenterPercent", "-1");
    wxString ssl = ModelXml->GetAttribute("StarStartLocation");
    wxString sn = ModelXml->GetAttribute("StrandNames");
    wxString nn = ModelXml->GetAttribute("NodeNames");
    wxString da = ModelXml->GetAttribute("DisplayAs");
    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<starmodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%s\" ", p1));
    f.Write(wxString::Format("parm2=\"%s\" ", p2));
    f.Write(wxString::Format("parm3=\"%s\" ", p3));
    f.Write(wxString::Format("DisplayAs=\"%s\" ", da));
    f.Write(wxString::Format("StringType=\"%s\" ", st));
    f.Write(wxString::Format("Transparency=\"%s\" ", t));
    f.Write(wxString::Format("PixelSize=\"%s\" ", ps));
    f.Write(wxString::Format("ModelBrightness=\"%s\" ", mb));
    f.Write(wxString::Format("Antialias=\"%s\" ", a));
    f.Write(wxString::Format("LayerSizes=\"%s\" ", ls));
    f.Write(wxString::Format("starRatio=\"%s\" ", sr));
    if (scp != "-1") {
        f.Write(wxString::Format("starCenterPercent=\"%s\" ", scp));
    }
    f.Write(wxString::Format("StarStartLocation=\"%s\" ", ssl));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(ExportSuperStringColors());
    f.Write(" >\n");
    wxString groups = SerialiseGroups();
    if (groups != "") {
        f.Write(groups);
    }
    wxString state = SerialiseState();
    if (state != "") {
        f.Write(state);
    }
    wxString face = SerialiseFace();
    if (face != "") {
        f.Write(face);
    }
    wxString submodel = SerialiseSubmodel();
    if (submodel != "") {
        f.Write(submodel);
    }
    ExportDimensions(f);
    f.Write("</starmodel>");
    f.Close();
}

void StarModel::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "starmodel") {
        wxString name = root->GetAttribute("name");
        wxString p1 = root->GetAttribute("parm1");
        wxString p2 = root->GetAttribute("parm2");
        wxString p3 = root->GetAttribute("parm3");
        wxString st = root->GetAttribute("StringType");
        wxString ps = root->GetAttribute("PixelSize");
        wxString t = root->GetAttribute("Transparency", "0");
        wxString mb = root->GetAttribute("ModelBrightness", "0");
        wxString a = root->GetAttribute("Antialias");
        wxString sts = root->GetAttribute("StartSide");
        wxString ls = root->GetAttribute("starSizes");
        if (ls == "") {
            ls = root->GetAttribute("LayerSizes");
        }
        wxString sr = root->GetAttribute("starRatio");
        wxString scp = root->GetAttribute("starCenterPercent", "-1");
        wxString dir = root->GetAttribute("Dir");
        wxString ssl = root->GetAttribute("StarStartLocation");
        wxString sn = root->GetAttribute("StrandNames");
        wxString nn = root->GetAttribute("NodeNames");
        wxString v = root->GetAttribute("SourceVersion");
        wxString da = root->GetAttribute("DisplayAs");
        wxString pc = root->GetAttribute("PixelCount");
        wxString pt = root->GetAttribute("PixelType");
        wxString psp = root->GetAttribute("PixelSpacing");

        // Add any model version conversion logic here
        // Source version will be the program version that created the custom model
        if (sr.IsEmpty())
            sr = "2.618034";

        SetProperty("parm1", p1);
        SetProperty("parm2", p2);
        SetProperty("parm3", p3);
        SetProperty("StringType", st);
        SetProperty("PixelSize", ps);
        SetProperty("Transparency", t);
        SetProperty("ModelBrightness", mb);
        SetProperty("Antialias", a);
        SetProperty("LayerSizes", ls);
        SetProperty("starRatio", sr);
        if (scp != "-1") {
            SetProperty("starCenterPercent", scp);
        }
        SetProperty("StrandNames", sn);
        SetProperty("NodeNames", nn);
        SetProperty("DisplayAs", da);
        SetProperty("PixelCount", pc);
        SetProperty("PixelType", pt);
        SetProperty("PixelSpacing", psp);

        if (ssl != "") {
            SetProperty("StarStartLocation", ssl);
        } else {
            SetProperty("StarStartLocation", ConvertFromDirStartSide(dir, sts));
        }

        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        GetModelScreenLocation().Write(ModelXml);
        SetProperty("name", newname, true);

        ImportSuperStringColours(root);
        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::ImportXlightsModel");
    } else {
        DisplayError("Failure loading Star model file.");
    }
}

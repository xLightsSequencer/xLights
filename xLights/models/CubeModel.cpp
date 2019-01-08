#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>
#include <wx/log.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>

#include "CubeModel.h"
#include "ModelScreenLocation.h"
#include "../xLightsVersion.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"

static std::vector<std::tuple<int, int, int, int>> transformations =
{
{ 0,1,-1,0 }, // FBL V FB
{ 0,0,-1,1 }, //     V LR
{ 0,1,0,1 },  //     H FB
{ 0,0,0,0 },  //     H LR
{ -1,1,0,1 }, // FBR V FB
{ 0,0,1,0 },  //     V LR
{ 0,1,0,0 },  //     H FB
{ 0,0,0,1 },  //     H LR
{ -1,-1,0,1 },// FTL V FB
{0,0,-1,0},   //     V LR
{0,-1,2,0},   //     H FB
{0,0,2,1},    //     H LR
{0,1,-1,0},   // FTR V FB
{0,0,1,1},    //     V LR
{0,1,2,1},    //     H FB
{0,0,2,0},    //     H LR
{0,-1,-1,1},  // BBL V FB
{0,2,1,0},    //     V LR
{0,-1,0,0},   //     H FB
{0,2,0,1},    //     H LR
{0,1,1,0},    // BBR V FB
{0,2,-1,1},   //     V LR
{0,1,0,1},    //     H FB
{0,2,0,0},    //     H LR
{0,-1,-1,0},  // BTL V FB
{0,2,1,1},    //     V LR
{0,1,2,0},    //     H FB
{2,0,0,0},    //     H LR
{0,1,1,1},    // BTR V FB
{0,2,-1,0},   //     V LR
{0,1,2,0},    //     H FB
{2,0,0,1}     //     H LR
};

static wxPGChoices TOP_BOT_LEFT_RIGHT;
static wxPGChoices CUBE_STYLES;

void CubeModel::InitialiseChoices()
{
    if (TOP_BOT_LEFT_RIGHT.GetCount() == 0) {
        TOP_BOT_LEFT_RIGHT.Add("Front Bottom Left");
        TOP_BOT_LEFT_RIGHT.Add("Front Bottom Right");
        TOP_BOT_LEFT_RIGHT.Add("Front Top Left");
        TOP_BOT_LEFT_RIGHT.Add("Front Top Right");
        TOP_BOT_LEFT_RIGHT.Add("Back Bottom Left");
        TOP_BOT_LEFT_RIGHT.Add("Back Bottom Right");
        TOP_BOT_LEFT_RIGHT.Add("Back Top Left");
        TOP_BOT_LEFT_RIGHT.Add("Back Top Right");

        CUBE_STYLES.Add("Vertical Front/Back");
        CUBE_STYLES.Add("Vertical Left/Right");
        CUBE_STYLES.Add("Horizontal Front/Back");
        CUBE_STYLES.Add("Horizontal Left/Right");
    }
}

CubeModel::CubeModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    InitialiseChoices();
    screenLocation.SetSupportsZScaling(true);
    Model::SetFromXml(node, zeroBased);
}

CubeModel::CubeModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    InitialiseChoices();
}

CubeModel::~CubeModel()
{
    //dtor
}

int CubeModel::CalcTransformationIndex() const
{
    auto style = ModelXml->GetAttribute("Style", CUBE_STYLES.GetLabel(0));
    bool leftright = style.Contains("Left");
    bool horizontal = style.Contains("Horizontal");
    return (GetStartIndex() << 2) + (horizontal << 1) + leftright;
}

void CubeModel::AddTypeProperties(wxPropertyGridInterface *grid) {

    grid->Append(new wxEnumProperty("Starting Location", "CubeStart", TOP_BOT_LEFT_RIGHT, GetStartIndex()));
    grid->Append(new wxEnumProperty("Direction", "CubeStyle", CUBE_STYLES, GetStyleIndex()));
    auto p = grid->Append(new wxBoolProperty("No Zig Zag", "StrandPerLine", IsStrandPerLine()));
    p->SetAttribute("UseCheckbox", true);
    p = grid->Append(new wxBoolProperty("Layers All Start in Same Place", "StrandPerLayer", IsStrandPerLayer()));
    p->SetAttribute("UseCheckbox", true);

    p = grid->Append(new wxUIntProperty("Width", "CubeWidth", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Height", "CubeHeight", parm2));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Depth", "CubeDepth", parm3));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Strings", "CubeStrings", GetStrings()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1000);
    p->SetEditor("SpinCtrl");
}

int CubeModel::GetStrings() const
{
    return wxAtoi(ModelXml->GetAttribute("Strings", "1"));
}

int CubeModel::GetStartIndex() const
{
    auto start = ModelXml->GetAttribute("Start", "");

    for (int i = 0; i < TOP_BOT_LEFT_RIGHT.GetCount(); i++)
    {
        if (start == TOP_BOT_LEFT_RIGHT.GetLabel(i))
        {
            return i;
        }
    }
    return 0;
}

int CubeModel::GetStyleIndex() const
{
    auto start = ModelXml->GetAttribute("Style", "");

    for (int i = 0; i < CUBE_STYLES.GetCount(); i++)
    {
        if (start == CUBE_STYLES.GetLabel(i))
        {
            return i;
        }
    }
    return 3;
}

int CubeModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {

    if ("CubeStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Start");
        ModelXml->AddAttribute("Start", TOP_BOT_LEFT_RIGHT.GetLabel(event.GetValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return GRIDCHANGE_MARK_DIRTY_AND_REFRESH;
    } else if ("CubeStyle" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Style");
        ModelXml->AddAttribute("Style", CUBE_STYLES.GetLabel(event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return GRIDCHANGE_MARK_DIRTY_AND_REFRESH;
    } else if ("StrandPerLine" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("StrandPerLine");
        if (event.GetPropertyValue().GetBool())
        {
            ModelXml->AddAttribute("StrandPerLine", "TRUE");
        }
        SetFromXml(ModelXml, zeroBased);
        return GRIDCHANGE_MARK_DIRTY_AND_REFRESH | GRIDCHANGE_REBUILD_MODEL_LIST;
    } else if ("CubeStrandPerLayer" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("StrandPerLayer");
        if (event.GetPropertyValue().GetBool())
        {
            ModelXml->AddAttribute("StrandPerLayer", "TRUE");
        }
        SetFromXml(ModelXml, zeroBased);
        return GRIDCHANGE_MARK_DIRTY_AND_REFRESH | GRIDCHANGE_REBUILD_MODEL_LIST;
    } else if ("CubeWidth" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", static_cast<int>(event.GetPropertyValue().GetLong())));
        SetFromXml(ModelXml, zeroBased);
        return GRIDCHANGE_MARK_DIRTY_AND_REFRESH | GRIDCHANGE_REBUILD_MODEL_LIST;
    } else if ("CubeHeight" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", static_cast<int>(event.GetPropertyValue().GetLong())));
        SetFromXml(ModelXml, zeroBased);
        return GRIDCHANGE_MARK_DIRTY_AND_REFRESH | GRIDCHANGE_REBUILD_MODEL_LIST;
    } else if ("CubeDepth" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", static_cast<int>(event.GetPropertyValue().GetLong())));
        SetFromXml(ModelXml, zeroBased);
        return GRIDCHANGE_MARK_DIRTY_AND_REFRESH | GRIDCHANGE_REBUILD_MODEL_LIST;
    } else if ("CubeStrings" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Strings");
        ModelXml->AddAttribute("Strings", wxString::Format("%d", static_cast<int>(event.GetPropertyValue().GetLong())));
        SetFromXml(ModelXml, zeroBased);
        return GRIDCHANGE_MARK_DIRTY_AND_REFRESH | GRIDCHANGE_REBUILD_MODEL_LIST;
    }

    return Model::OnPropertyGridChange(grid, event);
}

std::tuple<int, int, int>& CubeModel::FlipX(std::tuple<int, int, int>& pt) const
{
    auto& x = std::get<0>(pt);
    x = parm1 - x - 1;
    return pt;
}

// +ve is clockwise assumes looking down on cube from top
std::tuple<int, int, int>& CubeModel::RotateY90Degrees(std::tuple<int, int, int>& pt, int by) const
{
    auto& x = std::get<0>(pt);
    auto& z = std::get<2>(pt);
    for (int i = 0; i < abs(by); i++)
    {
        if (by > 0)
        {
            auto temp = z;
            z = parm1 - x - 1;
            x = temp;
        }
        else
        {
            auto temp = x;
            x = parm3 - z - 1;
            z = temp;
        }
    }
    return pt;
}

// +ve is clockwise assumes looking at cube from front
std::tuple<int, int, int>& CubeModel::RotateZ90Degrees(std::tuple<int, int, int>& pt, int by) const
{
    auto& x = std::get<0>(pt);
    auto& y = std::get<1>(pt);
    for (int i = 0; i < abs(by); i++)
    {
        if (by > 0)
        {
            auto temp = x;
            x = parm2 - y - 1;
            y = parm1 - temp - 1;
        }
        else
        {
            auto temp = x;
            x = parm2 - y - 1;
            y = temp;
        }
    }
    return pt;
}

// +ve is up assumes looking at cube from front
std::tuple<int, int, int>& CubeModel::RotateX90Degrees(std::tuple<int, int, int>& pt, int by) const
{
    auto& y = std::get<1>(pt);
    auto& z = std::get<2>(pt);
    for (int i = 0; i < abs(by); i++)
    {
        if (by > 0)
        {
            // up
            auto temp = y;
            y = parm3 - z - 1;
            z = temp;
        }
        else
        {
            // down
            auto temp = z;
            z = parm2 - y - 1;
            y = temp;
        }
    }
    return pt;
}

bool CubeModel::IsStrandPerLayer() const
{
    return ModelXml->GetAttribute("StrandPerLayer", "FALSE") == "TRUE";
}

bool CubeModel::IsStrandPerLine() const
{
    return ModelXml->GetAttribute("StrandPerLine", "FALSE") == "TRUE";
}

// FIXME THERE IS SOMETHING WRONG WITH THIS ONCE YOU START PLAYING WITH START LOCATION AND SYLE
std::vector<std::tuple<int, int, int>> CubeModel::BuildCube(int w, int h, int d) const
{
    int width = w;
    int height = h;
    int depth = d;

    std::vector<std::tuple<int, int, int>> nodes;
    nodes.resize(width*height*depth);

    bool strandPerLine = IsStrandPerLine();
    bool strandPerLayer = IsStrandPerLayer();

    std::tuple<int, int, int, int> rotation = transformations[CalcTransformationIndex()];
    auto xr = std::get<0>(rotation);
    auto yr = std::get<1>(rotation);
    auto zr = std::get<2>(rotation);
    auto xf = std::get<3>(rotation);

    for(int i = 0; i < width*height*depth; i++)
    {
        int z = i / (width * height);
        int baselayer = i % (width*height);
        int y = baselayer / width;
        int x;
        if (strandPerLine || y % 2 == 0)
        {
            x = baselayer % width;
        }
        else
        {
            x = width - baselayer % width - 1;
        }

        if (!strandPerLayer)
        {
            // every 2nd layer starts at the top
            if (z % 2 != 0)
            {
                y = height - y - 1;
                if (height % 2 != 0)
                {
                    x = width - x - 1;
                }
            }
        }

        std::tuple<int,int,int> node = { x,y,z };
        RotateX90Degrees(node, xr);
        RotateY90Degrees(node, yr);
        RotateZ90Degrees(node, zr);
        if(xf > 0) FlipX(node);

        nodes[i] = node;
    }
    return nodes;
}

static std::vector<std::string> CUBE_BUFFERSTYLES = 
{
    "Default",
    "Per Preview",
    "Single Line",
    "As Pixel",
    "Stacked X Horizontally",
    "Stacked Y Horizontally",
    "Stacked Z Horizontally",
    "Stacked X Vertically",
    "Stacked Y Vertically",
    "Stacked Z Vertically",
    "Overlaid X",
    "Overlaid Y",
    "Overlaid Z",
    "Unique X and Y X",
    "Unique X and Y Y",
    "Unique X and Y Z",
    "Left Side",
    "Right Side",
    "Front Side",
    "Top Side",
    "Back Side",
    "Bottom Side"
};

const std::vector<std::string> &CubeModel::GetBufferStyles() const {
    return CUBE_BUFFERSTYLES;
}

void CubeModel::GetBufferSize(const std::string& type, const std::string& camera, const std::string& transform, int& BufferWi, int& BufferHi) const
{
    int width = parm1;
    int height = parm2;
    int depth = parm3;

    if (SingleNode || SingleChannel)
    {
        BufferWi = 1;
        BufferHi = 1;
    }
    else if (type == "Per Preview" || type == "Single Line" || type == "As Pixel")
    {
        Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi);
    }
    else if (type == "Stacked X Horizontally")
    {
        BufferHi = height;
        BufferWi = width * depth;
    }
    else if (type == "Stacked Y Horizontally")
    {
        BufferHi = depth;
        BufferWi = width * height;
    }
    else if (type == "Default" || type == "Stacked Z Horizontally")
    {
        BufferHi = height;
        BufferWi = width * depth;
    }
    else if (type == "Stacked X Vertically")
    {
        BufferHi = height * width;
        BufferWi = depth;
    }
    else if (type == "Stacked Y Vertically")
    {
        BufferHi = height * depth;
        BufferWi = width;
    }
    else if (type == "Stacked Z Vertically")
    {
        BufferWi = width;
        BufferHi = depth * height;
    }
    else if (type == "Overlaid X")
    {
        BufferWi = depth;
        BufferHi = height;
    }
    else if (type == "Overlaid Y")
    {
        BufferWi = width;
        BufferHi = depth;
    }
    else if (type == "Overlaid Z")
    {
        BufferWi = width;
        BufferHi = height;
    }
    else if (type == "Unique X and Y X")
    {
        BufferWi = height * width;
        BufferHi = depth * width;
    }
    else if (type == "Unique X and Y Y")
    {
        BufferWi = width * height;
        BufferHi = depth * height;
    }
    else if (type == "Unique X and Y Z")
    {
        BufferWi = width * depth;
        BufferHi = height * depth;
    }
    else if (type == "Left Side" || type == "Right Side")
    {
        BufferWi = depth;
        BufferHi = height;
    }
    else if (type == "Front Side" || type == "Back Side")
    {
        BufferWi = width;
        BufferHi = height;
    }
    else if (type == "Top Side" || type == "Bottom Side")
    {
        BufferWi = width;
        BufferHi = depth;
    }
    else
    {
        wxASSERT(false);
    }

    AdjustForTransform(transform, BufferWi, BufferHi);
}

void CubeModel::InitRenderBufferNodes(const std::string& type, const std::string& camera, const std::string& transform, std::vector<NodeBaseClassPtr>& Nodes, int& BufferWi, int& BufferHi) const
{
    int width = parm1;
    int height = parm2;
    int depth = parm3;

    Model::InitRenderBufferNodes(type, camera, transform, Nodes, BufferWi, BufferHi);

    if (SingleNode || SingleChannel)
    {
        wxASSERT(Nodes.size() == 1);
    }
    else
    {
        wxASSERT(Nodes.size() == width * height * depth);
    }

    if (SingleChannel || SingleNode)
    {
        return;
    }

    if (type == "Per Preview" || type == "Single Line" || type == "As Pixel")
    {
        return; 
    }

    GetBufferSize(type, camera, transform, BufferWi, BufferHi);

    // need to pre-rotate dimensions before we build the model
    int w = width;
    int h = height;
    int d = depth;
    std::tuple<int, int, int, int> rotation = transformations[CalcTransformationIndex()];
    auto xr = std::get<0>(rotation);
    auto yr = std::get<1>(rotation);
    auto zr = std::get<2>(rotation);
    auto xf = std::get<3>(rotation);
    if (abs(xr) == 1) std::swap(h, d);
    if (abs(yr) == 1) std::swap(w, d);
    if (abs(zr) == 1) std::swap(w, h);

    auto locations = BuildCube(w, h, d);

    if (type == "Stacked X Horizontally")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = depth - std::get<2>(locations[n]) - 1 + std::get<0>(locations[n]) * depth;
            Nodes[n]->Coords[0].bufY = std::get<1>(locations[n]);
        }
    }
    else if (type == "Stacked Y Horizontally")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = std::get<0>(locations[n]) + (height - std::get<1>(locations[n]) -1) * width;
            Nodes[n]->Coords[0].bufY = std::get<2>(locations[n]);
        }
    }
    else if (type == "Default" || type == "Stacked Z Horizontally")
    {
        // dont need to do anything
    }
    else if (type == "Stacked X Vertically")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = depth - std::get<2>(locations[n]) - 1;
            Nodes[n]->Coords[0].bufY = std::get<1>(locations[n]) + height * std::get<0>(locations[n]);
        }
    }
    else if (type == "Stacked Y Vertically")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = std::get<0>(locations[n]);
            Nodes[n]->Coords[0].bufY = std::get<2>(locations[n]) + depth * (height - std::get<1>(locations[n]) - 1);
        }
    }
    else if (type == "Stacked Z Vertically")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = std::get<0>(locations[n]);
            Nodes[n]->Coords[0].bufY = std::get<1>(locations[n]) + depth * std::get<2>(locations[n]);
        }
    }
    else if (type == "Overlaid X")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = depth - std::get<2>(locations[n]) - 1;
            Nodes[n]->Coords[0].bufY = std::get<1>(locations[n]);
        }
    }
    else if (type == "Overlaid Y")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = std::get<0>(locations[n]);
            Nodes[n]->Coords[0].bufY = std::get<2>(locations[n]);
        }
    }
    else if (type == "Overlaid Z")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = std::get<0>(locations[n]);
            Nodes[n]->Coords[0].bufY = std::get<1>(locations[n]);
        }
    }
    else if (type == "Unique X and Y X")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = depth - std::get<2>(locations[n]) - 1 + std::get<0>(locations[n]) * depth;
            Nodes[n]->Coords[0].bufY = std::get<1>(locations[n]) + std::get<0>(locations[n]) * height;
        }
    }
    else if (type == "Unique X and Y Y")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = std::get<0>(locations[n]) + (height-std::get<1>(locations[n])-1) * width;
            Nodes[n]->Coords[0].bufY = std::get<2>(locations[n]) + (height - std::get<1>(locations[n]) - 1) * depth;
        }
    }
    else if (type == "Unique X and Y Z")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = std::get<0>(locations[n]) + std::get<2>(locations[n]) * width;
            Nodes[n]->Coords[0].bufY = std::get<1>(locations[n]) + std::get<1>(locations[n]) * height;
        }
    }
    else if (type == "Left Side")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            if (std::get<0>(locations[n]) == 0)
            {
                Nodes[n]->Coords[0].bufX = depth - std::get<2>(locations[n]) - 1;
                Nodes[n]->Coords[0].bufY = std::get<1>(locations[n]);
            }
            else
            {
                Nodes[n]->Coords[0].bufX = -1;
                Nodes[n]->Coords[0].bufY = -1;
            }
        }
    }
    else if (type == "Right Side")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            if (std::get<0>(locations[n]) == width - 1)
            {
                Nodes[n]->Coords[0].bufX = std::get<2>(locations[n]);
                Nodes[n]->Coords[0].bufY = std::get<1>(locations[n]);
            }
            else
            {
                Nodes[n]->Coords[0].bufX = -1;
                Nodes[n]->Coords[0].bufY = -1;
            }
        }
    }
    else if (type == "Front Side")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            if (std::get<2>(locations[n]) == 0)
            {
                Nodes[n]->Coords[0].bufX = std::get<0>(locations[n]);
                Nodes[n]->Coords[0].bufY = std::get<1>(locations[n]);
            }
            else
            {
                Nodes[n]->Coords[0].bufX = -1;
                Nodes[n]->Coords[0].bufY = -1;
            }
        }
    }
    else if(type == "Back Side")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            if (std::get<2>(locations[n]) == depth - 1)
            {
                Nodes[n]->Coords[0].bufX = width - std::get<0>(locations[n]) - 1;
                Nodes[n]->Coords[0].bufY = std::get<1>(locations[n]);
            }
            else
            {
                Nodes[n]->Coords[0].bufX = -1;
                Nodes[n]->Coords[0].bufY = -1;
            }
        }
    }
    else if (type == "Top Side")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            if (std::get<1>(locations[n]) == height - 1)
            {
                Nodes[n]->Coords[0].bufX = std::get<0>(locations[n]);
                Nodes[n]->Coords[0].bufY = std::get<2>(locations[n]);
            }
            else
            {
                Nodes[n]->Coords[0].bufX = -1;
                Nodes[n]->Coords[0].bufY = -1;
            }
        }
    }
    else if (type == "Bottom Side")
    {
        for (size_t n = 0; n < Nodes.size(); n++)
        {
            if (std::get<1>(locations[n]) == 0)
            {
                Nodes[n]->Coords[0].bufX = std::get<0>(locations[n]);
                Nodes[n]->Coords[0].bufY = std::get<2>(locations[n]);
            }
            else
            {
                Nodes[n]->Coords[0].bufX = -1;
                Nodes[n]->Coords[0].bufY = -1;
            }
        }
    }
    else
    {
        wxASSERT(false);
    }
}

void CubeModel::InitModel()
{
    int width = parm1;
    int height = parm2;
    int depth = parm3;

    if (SingleNode || SingleChannel)
    {
        SetNodeCount(1, 1, rgbOrder);
    }
    else
    {
        SetNodeCount(1, width*height*depth, rgbOrder);
    }

    int chanPerNode = GetNodeChannelCount(StringType);

    // need to pre-rotate dimensions before we build the model
    int w = width;
    int h = height;
    int d = depth;
    std::tuple<int, int, int, int> rotation = transformations[CalcTransformationIndex()];
    auto xr = std::get<0>(rotation);
    auto yr = std::get<1>(rotation);
    auto zr = std::get<2>(rotation);
    auto xf = std::get<3>(rotation);
    if (abs(xr) == 1) std::swap(h, d);
    if (abs(yr) == 1) std::swap(w, d);
    if (abs(zr) == 1) std::swap(w, h);
    auto locations = BuildCube(w,h,d);

    for (size_t n = 0; n < Nodes.size(); n++)
    {
        Nodes[n]->ActChan = stringStartChan[0] + n * chanPerNode;
        Nodes[n]->StringNum = 1;
        if (SingleNode)
        {
            Nodes[n]->Coords[0].bufX = 0;
            Nodes[n]->Coords[0].bufY = 0;
            Nodes[n]->Coords[0].screenX = std::get<0>(locations[n]) - width / 2;
            Nodes[n]->Coords[0].screenY = std::get<1>(locations[n]) - height / 2;
            Nodes[n]->Coords[0].screenZ = depth - std::get<2>(locations[n]) - 1 - depth / 2;
        }
        else
        {
            Nodes[n]->Coords[0].bufX = std::get<0>(locations[n]) + std::get<2>(locations[n]) * width;
            Nodes[n]->Coords[0].bufY = std::get<1>(locations[n]);
            Nodes[n]->Coords[0].screenX = std::get<0>(locations[n]) - width / 2;
            Nodes[n]->Coords[0].screenY = std::get<1>(locations[n]) - height / 2;
            Nodes[n]->Coords[0].screenZ = depth - std::get<2>(locations[n]) - 1 - depth / 2;
        }
    }

    _strandLength = width * height;
    _strands = depth;

    screenLocation.SetRenderSize(width, height, depth);

    DisplayAs = "Cube";
    screenLocation.SetPerspective2D(0.1); // if i dont do this you cant see the back nodes in 2D
}

void CubeModel::ExportXlightsModel()
{
    // FIXME
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
    wxString da = ModelXml->GetAttribute("DisplayAs");
    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Cubemodel \n");
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
    f.Write(wxString::Format("StartSide=\"%s\" ", ss));
    f.Write(wxString::Format("Dir=\"%s\" ", dir));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(" >\n");
    wxString state = SerialiseState();
    if (state != "")
    {
        f.Write(state);
    }
    wxString face = SerialiseFace();
    if (face != "")
    {
        f.Write(face);
    }
    wxString submodel = SerialiseSubmodel();
    if (submodel != "")
    {
        f.Write(submodel);
    }
    f.Write("</Cubemodel>");
    f.Close();
}

void CubeModel::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    // FIXME
    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "Cubemodel")
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
            wxString da = root->GetAttribute("DisplayAs");
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
            SetProperty("DisplayAs", da);
            SetProperty("PixelCount", pc);
            SetProperty("PixelType", pt);
            SetProperty("PixelSpacing", psp);

            wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
            GetModelScreenLocation().Write(ModelXml);
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
                else if (n->GetName() == "faceInfo")
                {
                    AddFace(n);
                }
            }

            xlights->MarkEffectsFileDirty(true);
        }
        else
        {
            DisplayError("Failure loading Cube model file.");
        }
    }
    else
    {
        DisplayError("Failure loading Cube model file.");
    }
}

int CubeModel::MapToNodeIndex(int strand, int node) const
{
    if (SingleChannel || SingleNode)
    {
        return node;
    }
    else
    {
        return strand * _strandLength + node;
    }
}

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
#include <wx/log.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>

#include "CubeModel.h"
#include "ModelScreenLocation.h"
#include "../xLightsVersion.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Controller.h"

#include <log4cpp/Category.hh>

static std::vector<std::tuple<int, int, int, int>> transformations =
{
    { 1,0,-1,0 },   // FBL V FB          ok
    { 0,0,-1,1 },   //     V LR          ok
    { 0,-1,0,1 },   //     H FB          ok
    { 0,0,0,0 },    //     H LR          ok
    { -1,2,0,1 },   //     S FB          ok
    { -1,-1,0,0 },  //     S LR          ok
    { 1,0,-1,1 },   // FBR V FB          ok
    { 0,0,-1,0 },   //     V LR          ok
    { 0,-1,0,0 },   //     H FB          ok
    { 0,0,0,1 },    //     H LR          ok
    { -1,2,0,0 },   //     S FB          ok
    { -1,-1,0,1 },  //     S LR          ok
    { 1,0,1,1 },    // FTL V FB          ok
    { 0,0,1,0 },    //     V LR          ok
    { 0,-1,2,0 },   //     H FB          ok
    { 0,0,2,1 },    //     H LR          ok
    { -1,2,2,0 },   //     S FB          ok
    { -1,-1,2,1 },  //     S LR          ok
    { 1,0,1,0 },    // FTR V FB          ok
    { 0,0,1,1 },    //     V LR          ok
    { 0,-1,2,1 },   //     H FB          ok
    { 0,0,2,0 },    //     H LR          ok
    { -1,2,2,1 },   //     S FB          ok
    { -1,-1,2,0 },  //     S LR          ok
    { -1,0,-1,1 },  // BBL V FB          ok
    { 0,2,1,0 },    //     V LR          ok
    { 0,1,0,0 },    //     H FB          ok
    { 0,2,0,1 },    //     H LR          ok
    { -1,0,0,0 },   //     S FB          ok
    { -1,1,0,1 },   //     S LR          ok
    { -1,0,-1,0 },  // BBR V FB          ok
    { 0,2,1,1 },    //     V LR          ok
    { 0,1,0,1 },    //     H FB          ok
    { 0,2,0,0 },    //     H LR          ok
    { -1,0,0,1 },   //     S FB          ok
    { -1,1,0,0 },   //     S LR          ok
    { -1,0,1,0 },   // BTL V FB          ok
    { 0,2,-1,1 },   //     V LR          ok
    { 0,-1,2,0 },   //     H FB          ok
    { 2,0,0,0 },    //     H LR          ok
    { -1,2,2,0 },   //     S FB          ok
    { 1,-1,0,0 },   //     S LR          ok
    { -1,0,1,1 },   // BTR V FB          ok
    { 0,2,-1,0 },   //     V LR          ok
    { 0,-1,2,1 },   //     H FB          ok
    { 2,0,0,1 },    //     H LR          ok
    { -1,2,2,1 },   //     S FB          ok
    { 1,-1,0,1 }    //     S LR          ok
};

static const char* TOP_BOT_LEFT_RIGHT_VALUES[] = {
        "Front Bottom Left",
        "Front Bottom Right",
        "Front Top Left",
        "Front Top Right",
        "Back Bottom Left",
        "Back Bottom Right",
        "Back Top Left",
        "Back Top Right"
};
static wxPGChoices TOP_BOT_LEFT_RIGHT(wxArrayString(8, TOP_BOT_LEFT_RIGHT_VALUES));

static const char* CUBE_STYLES_VALUES[] = { 
        "Vertical Front/Back",
        "Vertical Left/Right",
        "Horizontal Front/Back",
        "Horizontal Left/Right",
        "Stacked Front/Back",
        "Stacked Left/Right"
};
static wxPGChoices CUBE_STYLES(wxArrayString(6, CUBE_STYLES_VALUES));

static const char* STRAND_STYLES_VALUES[] = { 
        "Zig Zag",
        "No Zig Zag",
        "Aternate Pixel"
};
static wxPGChoices STRAND_STYLES(wxArrayString(3, STRAND_STYLES_VALUES));

CubeModel::CubeModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    screenLocation.SetSupportsZScaling(true);
    Model::SetFromXml(node, zeroBased);
}

CubeModel::CubeModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
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
    bool stacked = style.Contains("Stacked");
    return (GetStartIndex() * CUBE_STYLES.GetCount()) + (horizontal << 1) + (stacked << 2) + leftright;
}

void CubeModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    grid->Append(new wxEnumProperty("Starting Location", "CubeStart", TOP_BOT_LEFT_RIGHT, GetStartIndex()));
    grid->Append(new wxEnumProperty("Direction", "CubeStyle", CUBE_STYLES, GetStyleIndex()));
    grid->Append(new wxEnumProperty("Strand Style", "StrandPerLine", STRAND_STYLES, GetStrandStyleIndex()));
    auto p = grid->Append(new wxBoolProperty("Layers All Start in Same Place", "StrandPerLayer", IsStrandPerLayer()));
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

    for (size_t i = 0; i < TOP_BOT_LEFT_RIGHT.GetCount(); i++)
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

    for (size_t i = 0; i < CUBE_STYLES.GetCount(); i++)
    {
        if (start == CUBE_STYLES.GetLabel(i))
        {
            return i;
        }
    }
    return 3;
}

int CubeModel::GetStrandStyleIndex() const
{
    auto start = ModelXml->GetAttribute("StrandPerLine", "Zig Zag");

    for (size_t i = 0; i < STRAND_STYLES.GetCount(); i++)
    {
        if (start == STRAND_STYLES.GetLabel(i))
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
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CubeModel::OnPropertyGridChange::CubeStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CubeModel::OnPropertyGridChange::CubeStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CubeModel::OnPropertyGridChange::CubeStart");
        return 0;
    } else if ("CubeStyle" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Style");
        ModelXml->AddAttribute("Style", CUBE_STYLES.GetLabel(event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CubeModel::OnPropertyGridChange::CubeStyle");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CubeModel::OnPropertyGridChange::CubeStyle");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CubeModel::OnPropertyGridChange::CubeStyle");
        return 0;
    } else if ("StrandPerLine" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("StrandPerLine");
        ModelXml->AddAttribute("StrandPerLine", STRAND_STYLES.GetLabel(event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CubeModel::OnPropertyGridChange::StrandPerLine");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CubeModel::OnPropertyGridChange::StrandPerLine");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CubeModel::OnPropertyGridChange::StrandPerLine");
        return 0;
    } else if ("StrandPerLayer" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("StrandPerLayer");
        if (event.GetPropertyValue().GetBool())
        {
            ModelXml->AddAttribute("StrandPerLayer", "TRUE");
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CubeModel::OnPropertyGridChange::StrandPerLayer");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CubeModel::OnPropertyGridChange::StrandPerLayer");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CubeModel::OnPropertyGridChange::StrandPerLayer");
        return 0;
    } else if ("CubeWidth" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", static_cast<int>(event.GetPropertyValue().GetLong())));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CubeModel::OnPropertyGridChange::CubeWidth");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CubeModel::OnPropertyGridChange::CubeWidth");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CubeModel::OnPropertyGridChange::CubeWidth");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "CubeModel::OnPropertyGridChange::CubeWidth");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CubeModel::OnPropertyGridChange::CubeWidth");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "CubeModel::OnPropertyGridChange::CubeWidth");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CubeModel::OnPropertyGridChange::CubeWidth");
        return 0;
    } else if ("CubeHeight" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", static_cast<int>(event.GetPropertyValue().GetLong())));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CubeModel::OnPropertyGridChange::CubeHeight");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CubeModel::OnPropertyGridChange::CubeHeight");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CubeModel::OnPropertyGridChange::CubeHeight");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "CubeModel::OnPropertyGridChange::CubeHeight");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CubeModel::OnPropertyGridChange::CubeHeight");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CubeModel::OnPropertyGridChange::CubeHeight");
        return 0;
    } else if ("CubeDepth" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", static_cast<int>(event.GetPropertyValue().GetLong())));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CubeModel::OnPropertyGridChange::CubeDepth");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CubeModel::OnPropertyGridChange::CubeDepth");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CubeModel::OnPropertyGridChange::CubeDepth");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "CubeModel::OnPropertyGridChange::CubeDepth");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CubeModel::OnPropertyGridChange::CubeDepth");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CubeModel::OnPropertyGridChange::CubeDepth");
        return 0;
    } else if ("CubeStrings" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Strings");
        ModelXml->AddAttribute("Strings", wxString::Format("%d", static_cast<int>(event.GetPropertyValue().GetLong())));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CubeModel::OnPropertyGridChange::CubeStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CubeModel::OnPropertyGridChange::CubeStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CubeModel::OnPropertyGridChange::CubeStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "CubeModel::OnPropertyGridChange::CubeStrings");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "CubeModel::OnPropertyGridChange::CubeStrings");
        return 0;
    }

    return Model::OnPropertyGridChange(grid, event);
}

std::tuple<int, int, int>& CubeModel::FlipX(std::tuple<int, int, int>& pt, int width) const
{
    auto& x = std::get<0>(pt);
    x = width - x - 1;
    return pt;
}

// +ve is clockwise assumes looking down on cube from top
std::tuple<int, int, int>& CubeModel::RotateY90Degrees(std::tuple<int, int, int>& pt, int by, int width, int depth) const
{
    auto& x = std::get<0>(pt);
    auto& z = std::get<2>(pt);
    for (int i = 0; i < abs(by); i++)
    {
        if (by > 0)
        {
            auto temp = z;
            z = width - x - 1;
            x = temp;
        }
        else
        {
            auto temp = x;
            x = depth - z - 1;
            z = temp;
        }
        std::swap(width, depth);
    }
    return pt;
}

// +ve is clockwise assumes looking at cube from front
std::tuple<int, int, int>& CubeModel::RotateZ90Degrees(std::tuple<int, int, int>& pt, int by, int width, int height) const
{
    auto& x = std::get<0>(pt);
    auto& y = std::get<1>(pt);
    for (int i = 0; i < abs(by); i++)
    {
        if (by > 0)
        {
            auto temp = x;
            x = y;
            y = width - temp - 1;
        }
        else
        {
            auto temp = x;
            x = height - y - 1;
            y = temp;
        }
        std::swap(width, height);
    }
    return pt;
}

// +ve is up assumes looking at cube from front
std::tuple<int, int, int>& CubeModel::RotateX90Degrees(std::tuple<int, int, int>& pt, int by, int height, int depth) const
{
    auto& y = std::get<1>(pt);
    auto& z = std::get<2>(pt);
    for (int i = 0; i < abs(by); i++)
    {
        if (by > 0)
        {
            // up
            auto temp = y;
            y = depth - z - 1;
            z = temp;
        }
        else
        {
            // down
            auto temp = z;
            z = height - y - 1;
            y = temp;
        }
        std::swap(height, depth);
    }
    return pt;
}

bool CubeModel::IsStrandPerLayer() const
{
    return ModelXml->GetAttribute("StrandPerLayer", "FALSE") == "TRUE";
}

std::string CubeModel::GetStartLocation() const
{
    return ModelXml->GetAttribute("Start", "") + " " + ModelXml->GetAttribute("Style", "");
}

std::vector<std::tuple<int, int, int>> CubeModel::BuildCube() const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int width = parm1;
    int height = parm2;
    int depth = parm3;

    std::vector<std::tuple<int, int, int>> nodes;
    nodes.resize(width*height*depth);

    bool strandPerLayer = IsStrandPerLayer();
    int strandStyle = GetStrandStyleIndex();

    std::tuple<int, int, int, int> rotation = transformations[CalcTransformationIndex()];
    auto xr = std::get<0>(rotation);
    auto yr = std::get<1>(rotation);
    auto zr = std::get<2>(rotation);
    auto xf = std::get<3>(rotation);

    // pre-rotate the dimensions to compensate for the latter rotation
    // need to pre-rotate dimensions before we build the model
    if (abs(zr) == 1) std::swap(width, height);
    if (abs(yr) == 1) std::swap(width, depth);
    if (abs(xr) == 1) std::swap(height, depth);

    logger_base.debug("%s %s StrandStyle: %s StrandPerLayer: %d", (const char*)ModelXml->GetAttribute("Start", "").c_str(), (const char*)ModelXml->GetAttribute("Style", "").c_str(), (const char*)ModelXml->GetAttribute("StrandPerLine", "").c_str(), IsStrandPerLayer());
    logger_base.debug("%dx%dx%d -> (%d,%d,%d,%d) -> %dx%dx%d", parm1, parm2, parm3, xr, yr, zr, xf, width, height, depth);

    for(int i = 0; i < width*height*depth; i++)
    {
        int z = i / (width * height);
        int baselayer = i % (width*height);
        int y = baselayer / width;
        int x;
        if ((strandStyle == 1 || y % 2 == 0) && strandStyle != 2)
        {
            x = baselayer % width;
        }
        else if (strandStyle == 2)
        {
            int pos = baselayer % width + 1;
            if (pos <= (width + 1) / 2)
            {
                x = 2 * (pos - 1);
            }
            else
            {
                x = (width - pos) * 2 + 1;
            }
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
                if (height % 2 != 0 && strandStyle == 0)
                {
                    x = width - x - 1;
                }
            }
        }

        std::tuple<int,int,int> node = { x,y,z };
        int w = width;
        int h = height;
        int d = depth;
        RotateX90Degrees(node, xr, h, d);
        if (abs(xr) == 1) std::swap(h, d);
        RotateY90Degrees(node, yr, w, d);
        if (abs(yr) == 1) std::swap(w, d);
        RotateZ90Degrees(node, zr, w, h);
        if (abs(zr) == 1) std::swap(w, h);

        wxASSERT(w == parm1 && h == parm2 && d == parm3);

        if(xf > 0) FlipX(node, w);

        nodes[i] = node;
    }

    //DumpNodes(nodes, parm1, parm2, parm3);

    return nodes;
}

int CubeModel::FindNodeIndex(std::vector<std::tuple<int, int, int>> nodes, int x, int y, int z) const
{
    int index = 0;
    for (auto it: nodes)
    {
        auto nx = std::get<0>(it);
        auto ny = std::get<1>(it);
        auto nz = std::get<2>(it);

        if (nx == x && ny == y && nz == z) return index;

        index++;
    }
    return -1;
}

void CubeModel::DumpNode(const std::string desc, const std::tuple<int, int, int>& node, int width, int height, int depth) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    auto x = std::get<0>(node);
    auto y = std::get<1>(node);
    auto z = std::get<2>(node);

    logger_base.debug("%s (%d,%d,%d) %dx%dx%d", (const char*)desc.c_str(), x, y, z, width, height, depth);
}

void CubeModel::DumpNodes(std::vector<std::tuple<int,int,int>> nodes, int width, int height, int depth) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    for (int z = 0; z < depth; z++)
    {
        std::string out = "\n";
        for (int y = height - 1; y >= 0; y--)
        {
            for (int x  = 0; x < width; x++)
            {
                out += wxString::Format("%d ", FindNodeIndex(nodes, x, y, z));
            }
            out += "\n";
        }
        logger_base.debug("Layer: %d %s", z, (const char *)out.c_str());
    }
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
    else if (StartsWith(type, "Per Preview") || type == "Single Line" || type == "As Pixel")
    {
        Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi);
    }
    else if (type == "Horizontal Per Strand")
    {
        // FIXME Pretty sure this isnt right
        BufferHi = GetStrandLength(0);
        BufferWi = GetNumStrands();
    }
    else if (type == "Vertical Per Strand")
    {
        // FIXME Pretty sure this isnt right
        BufferWi = GetStrandLength(0);
        BufferHi = GetNumStrands();
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

    int oldNodes = Nodes.size();

    Model::InitRenderBufferNodes(type, camera, transform, Nodes, BufferWi, BufferHi);

    if (SingleNode || SingleChannel)
    {
        wxASSERT(Nodes.size() - oldNodes == 1);
    }
    else
    {
        wxASSERT(Nodes.size() - oldNodes == width * height * depth);
    }

    if (SingleChannel || SingleNode)
    {
        return;
    }

    if (StartsWith(type, "Per Preview") || type == "Single Line" || type == "As Pixel")
    {
        return;
    }

    GetBufferSize(type, camera, transform, BufferWi, BufferHi);

    auto locations = BuildCube();

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
    else if (type == "Horizontal Per Strand")
    {
        // FIXME Pretty sure this isnt right
        int sl = BufferHi;
        for (auto n = 0; n < Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = n / sl;
            Nodes[n]->Coords[0].bufY = n % sl;
        }
    }
    else if (type == "Vertical Per Strand")
    {
        // FIXME Pretty sure this isnt right
        int sl = BufferWi;
        for (auto n = 0; n < Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = n % sl;
            Nodes[n]->Coords[0].bufY = n / sl;
        }
    }
    else if (type == "Stacked X Vertically")
    {
        for (auto n = 0; n < Nodes.size(); n++)
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

    auto locations = BuildCube();

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

    // save the default model size
    BufferWi = width * depth;
    BufferHt = height;

    DisplayAs = "Cube";
    screenLocation.SetPerspective2D(0.1f); // if i dont do this you cant see the back nodes in 2D
}

void CubeModel::ExportXlightsModel()
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
    wxString da = ModelXml->GetAttribute("DisplayAs");
    wxString s0 = ModelXml->GetAttribute("Strings");
    wxString s1 = ModelXml->GetAttribute("Start");
    wxString s2 = ModelXml->GetAttribute("Style");
    wxString s3 = ModelXml->GetAttribute("StrandPerLine");
    wxString s4 = ModelXml->GetAttribute("StrandPerLayer");
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
    f.Write(wxString::Format("Strings=\"%s\" ", s0));
    f.Write(wxString::Format("Start=\"%s\" ", s1));
    f.Write(wxString::Format("Style=\"%s\" ", s2));
    f.Write(wxString::Format("StrandsPerLine=\"%s\" ", s3));
    f.Write(wxString::Format("StrandsPerLayer=\"%s\" ", s4));
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
            wxString s0 = root->GetAttribute("Strings");
            wxString s1 = root->GetAttribute("Start");
            wxString s2 = root->GetAttribute("Style");
            wxString s3 = root->GetAttribute("StrandsPerLine");
            wxString s4 = root->GetAttribute("StrandsPerLayer");

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
            SetProperty("Strings", s0);
            SetProperty("Start", s1);
            SetProperty("Style", s2);
            SetProperty("StrandsPerLine", s3);
            SetProperty("StrandsPerLayer", s4);

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

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CubeModel::ImportXlightsModel");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CubeModel::ImportXlightsModel");
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

std::string CubeModel::ChannelLayoutHtml(OutputManager* outputManager)
{
    size_t NodeCount = GetNodeCount();

    std::vector<int> chmap;
    chmap.resize(BufferHt * BufferWi, 0);

    int strings = GetStrings();
    int nodes = GetNodeCount();
    int nodesPerString = std::ceil(static_cast<float>(nodes) / strings);

    std::string direction = GetStartLocation();

    int32_t sc;
    auto c = outputManager->GetController(this->GetFirstChannel() + 1, sc);

    std::string html = "<html><body><table border=0>";
    html += "<tr><td>Name:</td><td>" + name + "</td></tr>";
    html += "<tr><td>Display As:</td><td>" + DisplayAs + "</td></tr>";
    html += "<tr><td>String Type:</td><td>" + StringType + "</td></tr>";
    html += "<tr><td>Start Corner:</td><td>" + direction + "</td></tr>";
    html += wxString::Format("<tr><td>Total nodes:</td><td>%d</td></tr>", static_cast<int>(NodeCount));
    html += wxString::Format("<tr><td>Width:</td><td>%d</td></tr>", parm1);
    html += wxString::Format("<tr><td>Height:</td><td>%d</td></tr>", parm2);
    html += wxString::Format("<tr><td>Depth:</td><td>%d</td></tr>", parm3);

    if (c != nullptr)
    {
        html += wxString::Format("<tr><td>Controller:</td><td>%s</td></tr>", c->GetLongDescription());
    }

    if (GetControllerProtocol() != "") {
        html += wxString::Format("<tr><td>Pixel protocol:</td><td>%s</td></tr>", GetControllerProtocol().c_str());
        if (GetNumStrings() == 1) {
            html += wxString::Format("<tr><td>Controller Connection:</td><td>%d</td></tr>", GetControllerPort());
        }
        else {
            html += wxString::Format("<tr><td>Controller Connections:</td><td>%d-%d</td></tr>", GetControllerPort(), GetControllerPort() + GetNumPhysicalStrings() - 1);
        }
    }
    html += "</table><p>Node numbers starting with 1 followed by string number:</p><table border=1>";

    auto locations = BuildCube();

    for (int y = parm2 -1; y >=0; y--)
    {
        html += "<tr>";
        for (int j = 0; j < parm1 * parm3; j++)
        {
            int z = j / parm1;
            int x = j % parm1;

            int index = FindNodeIndex(locations, x, y, z);
            int string = index / nodesPerString + 1;
            int nodenum = index % nodesPerString + 1;
            wxString bgcolor = string % 2 == 1 ? "#ADD8E6" : "#90EE90";

            html += wxString::Format("<td bgcolor='" + bgcolor + "'>n%ds%d</td>", nodenum, string);
        }
        html += "</tr>";
    }

    html += "</table></body></html>";
    return html;
}

void CubeModel::ExportAsCustomXModel() const {

    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty()) return;

    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    wxString cm = "";
    int width = parm1;
    int height = parm2;
    int depth = parm3;

    auto locations = BuildCube();

    for (int l = 0; l < depth; l++)
    {
        if (cm != "") cm += "|";
        wxString ll = "";

        for (int r = height-1; r >=0; r--)
        {
            if (ll != "") ll += ";";
            wxString rr = "";

            for (int c = 0; c < width; c++)
            {
                if (rr != "") rr += ",";
                rr += wxString::Format("%d ", FindNodeIndex(locations, c, r, l));
            }
            ll += rr;
        }
        cm += ll;
    }

    wxString p1 = wxString::Format("%i", width);
    wxString p2 = wxString::Format("%i", height);
    wxString d = wxString::Format("%i", depth);
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency");
    wxString mb = ModelXml->GetAttribute("ModelBrightness");
    wxString a = ModelXml->GetAttribute("Antialias");
    wxString sn = ModelXml->GetAttribute("StrandNames");
    wxString nn = ModelXml->GetAttribute("NodeNames");
    wxString pc = ModelXml->GetAttribute("PixelCount");
    wxString pt = ModelXml->GetAttribute("PixelType");
    wxString psp = ModelXml->GetAttribute("PixelSpacing");

    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<custommodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%s\" ", p1));
    f.Write(wxString::Format("parm2=\"%s\" ", p2));
    f.Write(wxString::Format("Depth=\"%s\" ", d));
    f.Write(wxString::Format("StringType=\"%s\" ", st));
    f.Write(wxString::Format("Transparency=\"%s\" ", t));
    f.Write(wxString::Format("PixelSize=\"%s\" ", ps));
    f.Write(wxString::Format("ModelBrightness=\"%s\" ", mb));
    f.Write(wxString::Format("Antialias=\"%s\" ", a));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
    if (pc != "")
        f.Write(wxString::Format("PixelCount=\"%s\" ", pc));
    if (pt != "")
        f.Write(wxString::Format("PixelType=\"%s\" ", pt));
    if (psp != "")
        f.Write(wxString::Format("PixelSpacing=\"%s\" ", psp));
    f.Write("CustomModel=\"");
    f.Write(cm);
    f.Write("\" ");
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(" >\n");
    wxString face = SerialiseFace();
    if (face != "")
    {
        f.Write(face);
    }
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
    f.Write("</custommodel>");
    f.Close();
}

int CubeModel::NodesPerString() const
{
    int strings = GetStrings();
    if (strings == 0) strings = 1;
    int width = parm1;
    int height = parm2;
    int depth = parm3;
    return (width * height * depth) / strings;
}

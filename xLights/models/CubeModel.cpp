/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cassert>
#include <format>

#include "../XmlSerializer/FileSerializingVisitor.h"

#include "CubeModel.h"
#include "ModelScreenLocation.h"
#include "../xLightsVersion.h"
#include "UtilFunctions.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Controller.h"
#include "../graphics/IModelPreview.h"
#include "CustomModel.h"
#include "../XmlSerializer/XmlNodeKeys.h"

#include <log.h>

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
static const char* CUBE_STYLES_VALUES[] = {
        "Vertical Front/Back",
        "Vertical Left/Right",
        "Horizontal Front/Back",
        "Horizontal Left/Right",
        "Stacked Front/Back",
        "Stacked Left/Right"
};

static const char* STRAND_STYLES_VALUES[] = {
        "Zig Zag",
        "No Zig Zag",
        "Aternate Pixel"
};

CubeModel::CubeModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::Cube;
    screenLocation.SetSupportsZScaling(true);
}

CubeModel::~CubeModel()
{
    //dtor
}

int CubeModel::CalcTransformationIndex() const
{
    const std::string style = GetStrandStyle();
    bool leftright = Contains(style, "Left");
    bool horizontal = Contains(style, "Horizontal");
    bool stacked = Contains(style, "Stacked");
    return (_cubeStart * std::size(CUBE_STYLES_VALUES)) + (horizontal << 1) + (stacked << 2) + leftright;
}

std::string CubeModel::GetCubeStart() const
{
    if (_cubeStart >= 0 && _cubeStart < (int)std::size(TOP_BOT_LEFT_RIGHT_VALUES)) {
        return TOP_BOT_LEFT_RIGHT_VALUES[_cubeStart];
    }
    return TOP_BOT_LEFT_RIGHT_VALUES[0];
}

void CubeModel::SetCubeStart(const std::string & start)
{
    _cubeStart = 0;
    for (size_t i = 0; i < std::size(TOP_BOT_LEFT_RIGHT_VALUES); i++) {
        if (start == TOP_BOT_LEFT_RIGHT_VALUES[i]) {
            _cubeStart = i;
            return;
        }
    }
}

std::string CubeModel::GetCubeStyle() const
{
    if (_cubeStyle >= 0 && _cubeStyle < (int)std::size(CUBE_STYLES_VALUES)) {
        return CUBE_STYLES_VALUES[_cubeStyle];
    }
    return CUBE_STYLES_VALUES[0];
}

void CubeModel::SetCubeStyle(const std::string & style)
{
    _cubeStyle = 0;
    for (size_t i = 0; i < std::size(CUBE_STYLES_VALUES); i++) {
        if (style == CUBE_STYLES_VALUES[i]) {
            _cubeStyle = i;
            return;
        }
    }
}

std::string CubeModel::GetStrandStyle() const
{
    if (_strandStyle >= 0 && _strandStyle < (int)std::size(STRAND_STYLES_VALUES)) {
        return STRAND_STYLES_VALUES[_strandStyle];
    }
    return STRAND_STYLES_VALUES[0];
}

void CubeModel::SetStrandStyle(const std::string & style)
{
    _strandStyle = 0;
    for (size_t i = 0; i < std::size(STRAND_STYLES_VALUES); i++) {
        if (style == STRAND_STYLES_VALUES[i]) {
            _strandStyle = i;
            return;
        }
    }
}

void CubeModel::FlipX(std::tuple<int, int, int>& pt, int width) const
{
    auto& x = std::get<0>(pt);
    x = width - x - 1;
}

// +ve is clockwise assumes looking down on cube from top
void CubeModel::RotateY90Degrees(std::tuple<int, int, int>& pt, int by, int width, int depth) const
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
}

// +ve is clockwise assumes looking at cube from front
void CubeModel::RotateZ90Degrees(std::tuple<int, int, int>& pt, int by, int width, int height) const
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
}

// +ve is up assumes looking at cube from front
void CubeModel::RotateX90Degrees(std::tuple<int, int, int>& pt, int by, int height, int depth) const
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
}

std::string CubeModel::GetStartLocation() const
{
    return GetCubeStart() + " " + GetCubeStyle();
}

std::vector<std::tuple<int, int, int>> CubeModel::BuildCube() const
{
    int width = _cubeWidth;
    int height = _cubeHeight;
    int depth = _cubeDepth;

    std::vector<std::tuple<int, int, int>> nodes;
    nodes.resize(width*height*depth);

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

    for(int i = 0; i < width*height*depth; i++)
    {
        int z = i / (width * height);
        int baselayer = i % (width*height);
        int y = baselayer / width;
        int x;
        if ((_strandStyle == 1 || y % 2 == 0) && _strandStyle != 2)
        {
            x = baselayer % width;
        }
        else if (_strandStyle == 2)
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

        if (!_strandPerLayer)
        {
            // every 2nd layer starts at the top
            if (z % 2 != 0)
            {
                y = height - y - 1;
                if (height % 2 != 0 && _strandStyle == 0)
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

        assert(w == _cubeWidth && h == _cubeHeight && d == _cubeDepth);

        if(xf > 0) FlipX(node, w);

        nodes[i] = node;
    }

    //DumpNodes(nodes, _cubeWidth, _cubeHeight, _cubeDepth);

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
    

    auto x = std::get<0>(node);
    auto y = std::get<1>(node);
    auto z = std::get<2>(node);

    spdlog::debug("{} ({},{},{}) {}x{}x{}", (const char*)desc.c_str(), x, y, z, width, height, depth);
}

void CubeModel::DumpNodes(std::vector<std::tuple<int,int,int>> nodes, int width, int height, int depth) const
{
    

    for (int z = 0; z < depth; z++)
    {
        std::string out = "\n";
        for (int y = height - 1; y >= 0; y--)
        {
            for (int x  = 0; x < width; x++)
            {
                out += std::format("{} ", FindNodeIndex(nodes, x, y, z));
            }
            out += "\n";
        }
        spdlog::debug("Layer: {} {}", z, out);
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

void CubeModel::GetBufferSize(const std::string& tp, const std::string& camera, const std::string& transform, int& BufferWi, int& BufferHi, int stagger) const
{
    std::string type = tp.starts_with("Per Model ") ? tp.substr(10) : tp;
    int width = _cubeWidth;
    int height = _cubeHeight;
    int depth = _cubeDepth;

    if (SingleNode || SingleChannel)
    {
        BufferWi = 1;
        BufferHi = 1;
    }
    else if (StartsWith(type, "Per Preview") || type == "Single Line" || type == "As Pixel")
    {
        Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi, stagger);
    }
    else if (type == "Horizontal Per Strand" || type == "Per Model Horizontal Per Strand" || type == "Horizontal Per Model/Strand" )
    {
        // FIXME Pretty sure this isnt right
        BufferHi = GetStrandLength(0);
        BufferWi = GetNumStrands();
    }
    else if (type == "Vertical Per Strand" || type == "Per Model Vertical Per Strand" || type == "Vertical Per Model/Strand")
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
        assert(false);
    }

    AdjustForTransform(transform, BufferWi, BufferHi);
}

int CubeModel::GetNumPhysicalStrings() const 
{ 
    int ts = GetSmartTs();
    if (ts <= 1) {
        return _cubeStrings;
    }
    else {
        int strings = _cubeStrings / ts;
        if (strings == 0) strings = 1;
        return strings;
    }
}

void CubeModel::InitRenderBufferNodes(const std::string& tp, const std::string& camera, const std::string& transform, std::vector<NodeBaseClassPtr>& Nodes, int& BufferWi, int& BufferHi, int stagger, bool deep) const
{
    std::string type = tp.starts_with("Per Model ") ? tp.substr(10) : tp;

    int width = _cubeWidth;
    int height = _cubeHeight;
    int depth = _cubeDepth;

    int oldNodes = Nodes.size();

    Model::InitRenderBufferNodes(type, camera, transform, Nodes, BufferWi, BufferHi, stagger);

    if (SingleNode || SingleChannel)
    {
        assert(Nodes.size() - oldNodes == 1);
    }
    else
    {
        assert((int)Nodes.size() - oldNodes == width * height * depth);
    }

    if (SingleChannel || SingleNode)
    {
        return;
    }

    if (StartsWith(type, "Per Preview") || type == "Single Line" || type == "As Pixel")
    {
        return;
    }

    GetBufferSize(type, camera, transform, BufferWi, BufferHi, stagger);

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
    else if (type == "Horizontal Per Strand" || type == "Per Model Horizontal Per Strand" || type == "Horizontal Per Model/Strand")
    {
        int sl = BufferHi;
        for (auto n = oldNodes; n < (int)Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = (n - oldNodes) / sl;
            Nodes[n]->Coords[0].bufY = (n - oldNodes) % sl;
        }
    }
    else if (type == "Vertical Per Strand" || type == "Per Model Vertical Per Strand" || type == "Vertical Per Model/Strand")
    {
        int sl = BufferWi;
        for (auto n = oldNodes; n < (int)Nodes.size(); n++)
        {
            Nodes[n]->Coords[0].bufX = (n - oldNodes) % sl;
            Nodes[n]->Coords[0].bufY = (n - oldNodes) / sl;
        }
    }
    else if (type == "Stacked X Vertically")
    {
        for (auto n = 0; n < (int)Nodes.size(); n++)
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
        assert(false);
    }
}

void CubeModel::InitModel()
{
    int width = _cubeWidth;
    int height = _cubeHeight;
    int depth = _cubeDepth;

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

    SetStringStartChannels(_cubeStrings, stringStartChan[0]+1 , NodesPerString() * chanPerNode);

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

    if (Contains(CUBE_STYLES_VALUES[_cubeStyle], "Left/Right")) {
        _strandLength = width * height;
        _strands = depth;
    }
    else         {
        _strandLength = depth * height;
        _strands = width;
    }
    screenLocation.SetRenderSize(width, height, depth);

    // save the default model size
    BufferWi = width * depth;
    BufferHt = height;

    screenLocation.SetPerspective2D(0.1f); // if i dont do this you cant see the back nodes in 2D
}

int CubeModel::MapToNodeIndex(int strand, int node) const
{
    if (SingleChannel || SingleNode) {
        return node;
    } else {
        return strand * _strandLength + node;
    }
}

std::string CubeModel::ChannelLayoutHtml(OutputManager* outputManager, bool darkMode)
{
    size_t NodeCount = GetNodeCount();

    std::vector<int> chmap;
    chmap.resize(BufferHt * BufferWi, 0);

    int strings = _cubeStrings;
    int nodes = GetNodeCount();
    int nodesPerString = std::ceil(static_cast<float>(nodes) / strings);

    std::string direction = GetStartLocation();

    int32_t sc;
    auto c = outputManager->GetController(this->GetFirstChannel() + 1, sc);

    std::string html = "<html><body><table border=0>";
    html += "<tr><td>Name:</td><td>" + name + "</td></tr>";
    html += "<tr><td>Display As:</td><td>" + DisplayAsTypeToString(DisplayAs) + "</td></tr>";
    html += "<tr><td>String Type:</td><td>" + StringType + "</td></tr>";
    html += "<tr><td>Start Corner:</td><td>" + direction + "</td></tr>";
    html += std::format("<tr><td>Total nodes:</td><td>{}</td></tr>", NodeCount);
    html += std::format("<tr><td>Width:</td><td>{}</td></tr>", _cubeWidth);
    html += std::format("<tr><td>Height:</td><td>{}</td></tr>", _cubeHeight);
    html += std::format("<tr><td>Depth:</td><td>{}</td></tr>", _cubeDepth);

    if (c != nullptr) {
        html += std::format("<tr><td>Controller:</td><td>{}</td></tr>", c->GetLongDescription());
    }

    if (GetControllerProtocol() != "") {
        html += std::format("<tr><td>Pixel protocol:</td><td>{}</td></tr>", GetControllerProtocol());
        if (GetNumStrings() == 1) {
            html += std::format("<tr><td>Controller Connection:</td><td>{}</td></tr>", GetControllerPort());
        } else {
            html += std::format("<tr><td>Controller Connections:</td><td>{}-{}</td></tr>", GetControllerPort(), GetControllerPort() + GetNumPhysicalStrings() - 1);
        }
    }
    html += "</table><p>Node numbers starting with 1 followed by string number:</p><table border=1>";

    auto locations = BuildCube();

    for (int y = _cubeHeight - 1; y >= 0; y--) {
        html += "<tr>";
        for (int j = 0; j < _cubeWidth * _cubeDepth; j++) {
            int z = j / _cubeWidth;
            int x = j % _cubeWidth;

            int index = FindNodeIndex(locations, x, y, z);
            int string = index / nodesPerString + 1;
            int nodenum = index % nodesPerString + 1;
            std::string bgcolor = string % 2 == 1 ? "#ADD8E6" : "#90EE90";
            if (darkMode)
                bgcolor = string % 2 == 1 ? "#3f7c85" : "#962B09";
            html += std::format("<td bgcolor='{}'>n{}s{}</td>", bgcolor, nodenum, string);
        }
        html += "</tr>";
    }

    html += "</table></body></html>";
    return html;
}

void CubeModel::ExportAsCustomXModel3D(BaseSerializingVisitor& visitor) const
{
    int width = _cubeWidth;
    int height = _cubeHeight;
    int depth = _cubeDepth;

    auto locations = BuildCube();

    std::vector<std::vector<std::vector<int>>> data;
    data.reserve(depth);
    for (int l = 0; l < depth; l++) {
        std::vector<std::vector<int>> layer;
        layer.reserve(height);
        for (int r = height - 1; r >= 0; r--) {
            std::vector<int> row;
            row.reserve(width);
            for (int c = 0; c < width; c++) {
                row.push_back(FindNodeIndex(locations, c, r, l) + 1);
            }
            layer.push_back(row);
        }
        data.push_back(layer);
    }

    BaseSerializingVisitor::AttrCollector attrs;
    attrs.Add("name", GetName());
    attrs.Add("CustomWidth", std::to_string(width));
    attrs.Add("CustomHeight", std::to_string(height));
    attrs.Add("Depth", std::to_string(depth));
    attrs.Add("StringType", GetStringType());
    attrs.Add("Transparency", GetTransparency() ? "1" : "0");
    attrs.Add("PixelSize", std::to_string(GetPixelSize()));
    attrs.Add("Antialias", std::to_string((int)GetPixelStyle()));
    attrs.Add("StrandNames", GetStrandNames());
    attrs.Add("NodeNames", GetNodeNames());
    std::string pc = GetPixelCount();
    if (!pc.empty()) attrs.Add("PixelCount", pc);
    std::string pt = GetPixelType();
    if (!pt.empty()) attrs.Add("PixelType", pt);
    std::string psp = GetPixelSpacing();
    if (!psp.empty()) attrs.Add("PixelSpacing", psp);
    attrs.Add("CustomModel", CustomModel::ToCustomModel(data));
    attrs.Add("CustomModelCompressed", CustomModel::ToCompressed(data));
    attrs.Add("SourceVersion", std::string(xlights_version_string));
    visitor.AddSuperStrings(*this, attrs);

    visitor.WriteOpenTag("custommodel", attrs);
    visitor.WriteFacesAndStates(this);
    visitor.WriteSubmodels(this);
    visitor.WriteCloseTag();
}

int CubeModel::NodesPerString() const
{
    int strings = _cubeStrings;
    if (strings == 0)
        strings = 1;
    int width = _cubeWidth;
    int height = _cubeHeight;
    int depth = _cubeDepth;
    int nodes = (width * height * depth) / strings;

    int ts = GetSmartTs();
    if (ts <= 1) {
        return nodes;
    } else {
        return nodes * ts;
    }
}

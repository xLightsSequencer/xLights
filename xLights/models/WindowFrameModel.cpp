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

#include "WindowFrameModel.h"
#include "ModelScreenLocation.h"
#include "../XmlSerializer/XmlNodeKeys.h"

#include "spdlog/spdlog.h"

WindowFrameModel::WindowFrameModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::WindowFrame;
}

WindowFrameModel::~WindowFrameModel()
{
}
 
void WindowFrameModel::InitModel() {
    InitFrame();
    screenLocation.RenderDp = 10.0f;  // give the bounding box a little depth
}

int WindowFrameModel::NodesPerString() const
{
    int ts = GetSmartTs();
    if (ts <= 1) {
        return GetChanCount() / std::max(GetChanCountPerNode(), 1);
    }
    else {
        return ts * (GetChanCount() / std::max(GetChanCountPerNode(), 1));
    }
}

void WindowFrameModel::GetCoordinates(int side, bool clockwise, bool LtoR, bool TtoB, float& x, float& y, float& screenx, float& screeny)
{
    // sides - left, top, right, bottom

    float top = _topNodes;
    float height = _sideNodes;
    float bottom = _bottomNodes;

    float width = std::max(top, bottom) + 2;

    if (LtoR) {
        if (TtoB) {
            y = height - 1;
            screenx = -width / 2;
            screeny = height / 2;
            if (clockwise) {
                x = 1;
            }
            else {
                x = 0;
            }
        } 
        else {
            y = 0;
            screenx = -width / 2;
            screeny = -height / 2;
            if (clockwise) {
                x = 0;
            }
            else {
                x = 1;
            }
        }
    }
    else {
        if (TtoB) {
            y = height - 1;
            screenx = width / 2;
            screeny = height / 2;
            if (clockwise) {
                x = width;
            }
            else {
                x = width - 1;
            }
        }
        else {
            y = 0;
            screenx = width / 2;
            screeny = -height / 2;
            if (clockwise) {
                x = width -1;
            }
            else {
                x = width;
            }
        }
    }
}

// initialize buffer coordinates
// _topNodes=Nodes on Top
// _sideNodes=Nodes left and right
// _bottomNodes=Nodes on Bottom
void WindowFrameModel::InitFrame()
{
    //

    SetNodeCount(1, _topNodes + 2 * _sideNodes + _bottomNodes, rgbOrder);

    int left = _sideNodes;
    int top = _topNodes;
    int bottom = _bottomNodes;

    int width = std::max(top, bottom) + 2;
    int height = _sideNodes;

    SetBufferSize(height, width);   // treat as outside of matrix
    screenLocation.SetRenderSize(width, height);

    if (left + top + bottom == 0) return;

    int chan = stringStartChan[0];
    int ChanIncr = GetNodeChannelCount(StringType);

    float dir = _rotation == 0 ? 1.0 : -1.0;

    int wadj = 0;
    int hadj = 0;
    if (dir == -1) {
        if ((isBotToTop && IsLtoR) || (!isBotToTop && !IsLtoR)) {
            wadj = 2;
            hadj = -2;
        }
    }
    else         {
        if ((!isBotToTop && IsLtoR) || (isBotToTop && !IsLtoR)) {
            wadj = 2;
            hadj = -2;
        }
    }

    float top_screenincr = 1;
    if (top + wadj - 1 != 0) top_screenincr = (float)(width) / (float)(top + 1);
    float bot_screenincr = 1;
    if (bottom + wadj - 1  != 0) bot_screenincr = -1.0 * (float)(width) / (float)(bottom + 1);

    float top_incr = 1;
    if (top == 0) top_incr = width - 1;
    else if (top + wadj + 1 != 0) top_incr = (float)(width + wadj) / (float)(top + wadj + 1);
    
    float bot_incr = 1;
    if (bottom == 0) bot_incr = -(width - 1);
    else if (bottom + wadj + 1 != 0) bot_incr = -1.0 * (float)(width + wadj) / (float)(bottom + wadj + 1);

    assert(top_incr >= 1.0);
    assert(bot_incr <= -1.0);

    int lengths[] = { left + hadj, top + wadj, left + hadj , bottom + wadj };
    float xscreenincr[] = { 0, top_screenincr, 0, bot_screenincr };
    float yscreenincr[] = { 1, 0, -1, 0 };
    float xincr[] = { 0, top_incr, 0, bot_incr };
    float yincr[] = { 1, 0, -1, 0 };
    float xStart[4];
    float yStart[4];
    float xScreenStart[4];
    float yScreenStart[4];
    
    int indexes[] = { 0, 1, 2, 3 };

    if (dir == -1) {
        // handle bottom left and top right differently
        if ((isBotToTop && IsLtoR) || (!isBotToTop && !IsLtoR)) {
            xStart[0] = 0;
            xStart[1] = (float)width - 1;
            xStart[2] = (float)width - 1;
            xStart[3] = 0;
            yStart[0] = (float)height - 2;
            yStart[1] = (float)height - 1;
            yStart[2] = 1;
            yStart[3] = 0;

            xScreenStart[0] = -(float)(width) / 2.0;
            xScreenStart[1] = (float)(width) / 2.0;
            xScreenStart[2] = xScreenStart[1];
            xScreenStart[3] = xScreenStart[0];
            yScreenStart[0] = (float)(height - 1) / 2.0 - 1.0;
            yScreenStart[1] = (float)(height - 1) / 2.0;
            yScreenStart[2] = -(float)(height - 1) / 2.0 + 1.0;
            yScreenStart[3] = -(float)(height - 1) / 2.0;
        }
        else             {
            xStart[0] = 0;
            xStart[1] = (float)width - 2;
            xStart[2] = (float)width - 1;
            xStart[3] = 1;
            yStart[0] = (float)height - 1;
            yStart[1] = (float)height - 1;
            yStart[2] = 0;
            yStart[3] = 0;

            xScreenStart[0] = -(float)(width) / 2.0;
            xScreenStart[1] = (float)(width) / 2.0 - top_screenincr;
            xScreenStart[2] = (float)(width) / 2.0;
            xScreenStart[3] = xScreenStart[0] - bot_screenincr;
            yScreenStart[0] = (float)(height - 1) / 2.0;
            yScreenStart[1] = (float)(height - 1) / 2.0;
            yScreenStart[2] = -(float)(height - 1) / 2.0;
            yScreenStart[3] = -(float)(height - 1) / 2.0;
        }
    }
    else {
        // handle top left and bottom right differently
        if ((!isBotToTop && IsLtoR) || (isBotToTop && !IsLtoR)) {
            xStart[0] = 0;
            xStart[1] = 0;
            xStart[2] = (float)width - 1;
            xStart[3] = (float)width - 1;
            yStart[0] = 1;
            yStart[1] = (float)height - 1;
            yStart[2] = (float)height - 2;
            yStart[3] = 0;

            xScreenStart[0] = -(float)(width) / 2.0;
            xScreenStart[1] = xScreenStart[0];
            xScreenStart[2] = (float)(width) / 2.0;
            xScreenStart[3] = xScreenStart[2];
            yScreenStart[0] = -(float)(height - 1) / 2.0 + 1;
            yScreenStart[1] = (float)(height - 1) / 2.0;
            yScreenStart[2] = (float)(height - 1) / 2.0 - 1;
            yScreenStart[3] = -(float)(height - 1) / 2.0;
        }
        else {
            xStart[0] = 0;
            xStart[1] = 1;
            xStart[2] = (float)width - 1;
            xStart[3] = (float)width - 2;
            yStart[0] = 0;
            yStart[1] = (float)height - 1;
            yStart[2] = (float)height - 1;
            yStart[3] = 0;

            xScreenStart[0] = -(float)width / 2;
            xScreenStart[1] = -(float)width / 2 + top_screenincr;
            xScreenStart[2] = (float)width / 2;
            xScreenStart[3] = (float)width / 2 + bot_screenincr;
            yScreenStart[0] = -(float)(height - 1) / 2;
            yScreenStart[1] = (float)(height - 1) / 2;
            yScreenStart[2] = (float)(height - 1) / 2;
            yScreenStart[3] = -(float)(height - 1) / 2;
        }
    }

    if (IsLtoR) {
        // L to R
        if (isBotToTop) {
            // Bottom left
            if (dir == 1.0) {
                // CW
                indexes[0] = 0;
                indexes[1] = 1;
                indexes[2] = 2;
                indexes[3] = 3;
            }
            else {
                // CCW
                indexes[0] = 3;
                indexes[1] = 2;
                indexes[2] = 1;
                indexes[3] = 0;
            }
        }
        else {
            // Top left
            if (dir == 1.0) {
                // CW
                indexes[0] = 1;
                indexes[1] = 2;
                indexes[2] = 3;
                indexes[3] = 0;
            }
            else {
                // CCW
                indexes[0] = 0;
                indexes[1] = 3;
                indexes[2] = 2;
                indexes[3] = 1;
            }
        }
    }
    else {
        // R to L
        if (isBotToTop) {
            // Bottom right
            if (dir == 1.0) {
                // CW
                indexes[0] = 3;
                indexes[1] = 0;
                indexes[2] = 1;
                indexes[3] = 2;
            }
            else {
                // CCW
                indexes[0] = 2;
                indexes[1] = 1;
                indexes[2] = 0;
                indexes[3] = 3;
            }
        }
        else {
            // Top right
            if (dir == 1.0) {
                // CW
                indexes[0] = 2;
                indexes[1] = 3;
                indexes[2] = 0;
                indexes[3] = 1;
            }
            else {
                // CCW
                indexes[0] = 1;
                indexes[1] = 0;
                indexes[2] = 3;
                indexes[3] = 2;
            }
        }
    }

    int side = 0;
    while (lengths[indexes[side]] == 0) side++;

    float x = xStart[indexes[side]];
    float y = yStart[indexes[side]];
    float screenx = xScreenStart[indexes[side]];
    float screeny = yScreenStart[indexes[side]];
    int curLen = lengths[indexes[side]];

    size_t nd = 0;
    size_t cd = 0;
    size_t loops = GetNodeCount();
    size_t coordCount = 1;
    if (SingleNode) {
        assert(GetNodeCount() == 1);
        coordCount = GetCoordCount(0);
        loops = coordCount;
    }
    else {
        assert(GetCoordCount(0) == 1); // only one coord supported by this code
    }

    for (size_t n = 0; n < loops; n++) {
        assert(curLen > 0);

        Nodes[nd]->ActChan = chan;

        if (SingleNode) {
            Nodes[nd]->Coords[cd].bufX = 0;
            Nodes[nd]->Coords[cd].bufY = 0;
        }
        else {
            Nodes[nd]->Coords[cd].bufX = (xincr[indexes[side]] * dir > 0 ? std::floor(x) : std::ceil(x));
            Nodes[nd]->Coords[cd].bufY = y;
            chan += ChanIncr;
        }

        //spdlog::debug("Node {} ({:.3f},{:.3f}) -> {}, {}", n, x, y, Nodes[n]->Coords[c].bufX, Nodes[n]->Coords[c].bufY);
        Nodes[nd]->Coords[cd].screenX = screenx;
        Nodes[nd]->Coords[cd].screenY = screeny;

        screenx = screenx + (xscreenincr[indexes[side]] * dir);
        screeny = screeny + (yscreenincr[indexes[side]] * dir);
        x = x + xincr[indexes[side]] * dir;
        y = y + yincr[indexes[side]] * dir;
        curLen--;
        if (curLen <= 0) {
            side++;
            if (side >= 4) side = 0;
            while (lengths[indexes[side]] == 0) {
                side++;
                if (side >= 4) side = 0;
            }
            x = xStart[indexes[side]];
            y = yStart[indexes[side]];
            screenx = xScreenStart[indexes[side]];
            screeny = yScreenStart[indexes[side]];
            curLen = lengths[indexes[side]];
        }

        if (SingleNode) {
            cd++;
        }
        else {
            nd++;
        }
    }
}


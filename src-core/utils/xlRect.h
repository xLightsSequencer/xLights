#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

struct xlRect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    xlRect() = default;
    xlRect(int x_, int y_, int w_, int h_) : x(x_), y(y_), width(w_), height(h_) {}
    int GetLeft() const { return x; }
    int GetTop() const { return y; }
    int GetRight() const { return x + width - 1; }
    int GetBottom() const { return y + height - 1; }
    void Offset(int dx, int dy) { x += dx; y += dy; }
    bool operator==(const xlRect& r) const {
        return x == r.x && y == r.y && width == r.width && height == r.height;
    }
    bool operator!=(const xlRect& r) const {
        return !(*this == r);
    }
};

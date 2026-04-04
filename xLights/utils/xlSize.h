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

struct xlSize {
    int width = 0;
    int height = 0;
    xlSize() = default;
    xlSize(int w, int h) : width(w), height(h) {}
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    void SetWidth(int w) { width = w; }
    void SetHeight(int h) { height = h; }
    bool operator==(const xlSize& r) const {
        return width == r.width && height == r.height;
    }
    bool operator!=(const xlSize& r) const {
        return !(*this == r);
    }
};

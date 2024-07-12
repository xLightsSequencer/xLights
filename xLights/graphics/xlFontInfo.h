#pragma once

#include <wx/image.h>
#include <vector>

#include "xlGraphicsAccumulators.h"

class FontInfoStruct;

class xlFontInfo {
public:
    xlFontInfo();
    ~xlFontInfo();

    int getID() const { return id; }

    int getSize() const { return size; }
    float widthOf(const std::string &text, float factor = 1.0) const;
    void populate(xlVertexTextureAccumulator &va, float x, float yBase, const std::string &text, float factor = 1.0, bool invert = false, float z = 0) const;
    const wxImage &getImage() const { return image; }

    static const xlFontInfo &FindFont(int size);
    static float ComputeFontSize(int& toffset, const float height, const float factor);

private:
    bool valid() const;
    bool init(int sz);
    bool load(const FontInfoStruct &fi);
    size_t CountChar(const std::string& text, char termChar) const;


    int size;
    int id;
    wxImage image;
    float maxD, maxW, maxH;
    std::vector<float> widths;
};

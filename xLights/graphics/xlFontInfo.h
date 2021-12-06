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

    float widthOf(const std::string &text, float factor = 1.0) const;
    void populate(xlVertexTextureAccumulator &va, float x, float yBase, const std::string &text, float factor = 1.0) const;
    const wxImage &getImage() const { return image; }

    static const xlFontInfo &FindFont(int size);
private:
    bool valid() const;
    bool init(int sz);
    bool load(const FontInfoStruct &fi);


    int id;
    wxImage image;
    float maxD, maxW, maxH;
    std::vector<float> widths;
};

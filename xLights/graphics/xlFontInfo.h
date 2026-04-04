#pragma once

#include <vector>

#include "xlGraphicsAccumulators.h"
#include "../utils/xlImage.h"

class FontInfoStruct;

class xlFontInfo {
public:
    xlFontInfo();
    ~xlFontInfo();

    int getID() const { return id; }

    int getSize() const { return size; }
    float widthOf(const std::string &text, float factor = 1.0) const;
    void populate(xlVertexTextureAccumulator &va, float x, float yBase, const std::string &text, float factor = 1.0) const;
    const xlImage &getImage() const { return image; }

    static const xlFontInfo &FindFont(int size);
private:
    bool valid() const;
    bool init(int sz);
    bool load(const FontInfoStruct &fi);
    bool CreateMissingFont(int size);


    int size;
    int id;
    xlImage image;
    float maxD, maxW, maxH;
    std::vector<float> widths;
};

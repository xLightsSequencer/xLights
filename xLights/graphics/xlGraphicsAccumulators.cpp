

#include "xlGraphicsAccumulators.h"

#include <algorithm>
#include <cmath>

#include "xlGraphicsContext.h"

void xlVertexAccumulator::AddRectAsLines(float x1, float y1, float x2, float y2) {
    PreAlloc(8);
    AddVertex(x1, y1);
    AddVertex(x1, y2);
    AddVertex(x1, y2);
    AddVertex(x2, y2);
    AddVertex(x2, y2);
    AddVertex(x2, y1);
    AddVertex(x2, y1);
    AddVertex(x1, y1);
}
void xlVertexAccumulator::AddRectAsDashedLines(float x1, float y1, float x2, float y2, float dashSize) {
    float xs = std::min(x1, x2);
    float xf = std::max(x1, x2);
    float halfDash = dashSize / 2.0;
    for(float x = xs; x <= xf; x += dashSize) {
        AddVertex(x, y1);
        AddVertex(x + halfDash < xf ? x + halfDash : xf, y1);
        AddVertex(x, y2);
        AddVertex(x + halfDash < xf ? x + halfDash : xf, y2);
    }
    // Line 2
    float ys = std::min(y1, y2);
    float yf = std::max(y1, y2);
    for(int y = ys; y <= yf; y += dashSize) {
        AddVertex(x1, y);
        AddVertex(x1, y + halfDash < yf ? y + halfDash : yf);
        AddVertex(x2, y);
        AddVertex(x2, y + halfDash < yf ? y + halfDash : yf);
    }
}
void xlVertexAccumulator::AddRectAsTriangles(float x1, float y1, float x2, float y2) {
    PreAlloc(6);
    AddVertex(x1, y1);
    AddVertex(x1, y2);
    AddVertex(x2, y2);
    AddVertex(x2, y2);
    AddVertex(x2, y1);
    AddVertex(x1, y1);
}
void xlVertexAccumulator::AddCircleAsLines(float cx, float cy, float r) {
    static const int steps = 24;
    static const double inc = 2.0 * 3.14159 / float(steps);
    double d = 0;
    for (int x = 0; x <= steps; x++, d += inc) {
        AddVertex(std::cos(d) * r + cx, std::sin(d) * r + cy);
    }
}

//------------------------------------------------------------------------

void xlVertexColorAccumulator::AddRectAsTriangles(float x1, float y1,
                                                  float x2, float y2,
                                                  const xlColor &color) {
    AddRectAsTriangles(x1, y1, x2, y2, 0.0f, color);
}
void xlVertexColorAccumulator::AddRectAsTriangles(float x1, float y1,
                                                  float x2, float y2,
                                                  float z,
                                                  const xlColor &color) {
    PreAlloc(6);
    AddVertex(x1, y1, z, color);
    AddVertex(x1, y2, z, color);
    AddVertex(x2, y2, z, color);
    AddVertex(x2, y2, z, color);
    AddVertex(x2, y1, z, color);
    AddVertex(x1, y1, z, color);
}
void xlVertexColorAccumulator::AddRectAsLines(float x1, float y1, float x2, float y2,
                                              const xlColor &color) {
    AddRectAsLines(x1, y1, x2, y2, 0, color);
}
void xlVertexColorAccumulator::AddRectAsLines(float x1, float y1, float x2, float y2, float z,
                                              const xlColor &color) {
    PreAlloc(8);
    AddVertex(x1, y1, z, color);
    AddVertex(x1, y2, z, color);
    AddVertex(x1, y2, z, color);
    AddVertex(x2, y2, z, color);
    AddVertex(x2, y2, z, color);
    AddVertex(x2, y1, z, color);
    AddVertex(x2, y1, z, color);
    AddVertex(x1, y1, z, color);
}
void xlVertexColorAccumulator::AddRectAsDashedLines(float x1, float y1, float x2, float y2, float dashSize, const xlColor &color) {
    AddRectAsDashedLines(x1, y1, x2, y2, 0, dashSize, color);
}
void xlVertexColorAccumulator::AddRectAsDashedLines(float x1, float y1, float x2, float y2, float z, float dashSize, const xlColor &color) {
    // Line 1
    float xs = x1 < x2 ? x1 : x2;
    float xf = x1 > x2 ? x1 : x2;
    float halfDash = dashSize / 2;
    for(float x = xs; x <= xf; x += dashSize) {
        AddVertex(x, y1, z, color);
        AddVertex(x + halfDash < xf ? x + halfDash : xf, y1, z, color);
        AddVertex(x, y2, z, color);
        AddVertex(x + halfDash < xf ? x + halfDash : xf, y2, z, color);
    }
    // Line 2
    int ys = y1<y2?y1:y2;
    int yf = y1>y2?y1:y2;
    for(int y = ys; y <= yf; y += dashSize) {
        AddVertex(x1, y, z, color);
        AddVertex(x1, y + halfDash < yf ? y + halfDash : yf, z, color);
        AddVertex(x2, y, z, color);
        AddVertex(x2, y + halfDash < yf ? y + halfDash : yf, z, color);
    }
}

void xlVertexColorAccumulator::AddHBlendedRectangleAsTriangles(float x1, float y1,float x2, float y2, xlColor* colorMask, int offset, const xlColorVector &colors) {
    xlColor start;
    xlColor end;
    int cnt = colors.size();
    if (cnt == 0) {
        return;
    }
    start = colors[0 + offset];
    start.ApplyMask(colorMask);
    if (cnt == 1) {
        AddHBlendedRectangleAsTriangles(x1, y1, x2, y2, start, start);
        return;
    }
    int xl = x1;
    start = colors[0+offset];
    start.ApplyMask(colorMask);
    for (int x = 1+offset; x < cnt; x++) {
        end =  colors[x];
        end.ApplyMask(colorMask);
        int xr = x1 + (x2 - x1) * x / (cnt  - 1);
        if (x == (cnt - 1)) {
            xr = x2;
        }
        AddHBlendedRectangleAsTriangles( xl, y1, xr, y2, start, end);
        start = end;
        xl = xr;
    }
}

void xlVertexColorAccumulator::AddHBlendedRectangleAsTriangles(float x1, float y1, float x2, float y2, const xlColor &left, const xlColor &right) {
    AddVertex(x1, y1, 0, left);
    AddVertex(x1, y2, 0, left);
    AddVertex(x2, y2, 0, right);
    AddVertex(x2, y2, 0, right);
    AddVertex(x2, y1, 0, right);
    AddVertex(x1, y1, 0, left);
}
void xlVertexColorAccumulator::AddCircleAsLines(float cx, float cy, float r, const xlColor &color) {
    static const int steps = 24;
    static const double inc = 2.0 * 3.14159 / float(steps);
    double d = 0;
    for (int x = 0; x <= steps; x++, d += inc) {
        AddVertex(std::cos(d) * r + cx, std::sin(d) * r + cy, color);
    }
}

void xlVertexColorAccumulator::AddCircleAsTriangles(float cx, float cy, float radius, const xlColor &color) {
    AddCircleAsTriangles(cx, cy, 0.0f, radius, color, color, 0.0);
}

void xlVertexColorAccumulator::AddCircleAsTriangles(float cx, float cy, float radius, const xlColor &center, const xlColor &edge) {
    AddCircleAsTriangles(cx, cy, 0.0f, radius, center, edge, 0.0);
}

void xlVertexColorAccumulator::AddCircleAsTriangles(float cx, float cy, float cz, float radius, const xlColor &color) {
    AddCircleAsTriangles(cx, cy, cz, radius, color, color, 0.0);
}
void xlVertexColorAccumulator::AddCircleAsTriangles(float cx, float cy, float cz, float radius, const xlColor &center, const xlColor &edge) {
    AddCircleAsTriangles(cx, cy, cz, radius, center, edge, 0.0);
}
void xlVertexColorAccumulator::AddCircleAsTriangles(float cx, float cy, float cz, float radius, const xlColor& center, const xlColor& edge, float depthRatio, int numSegments) {
    int num_segments = numSegments;
    if (num_segments == -1) {
        num_segments = radius;
    }
    if (num_segments < 16) {
        num_segments = 16;
    }
    PreAlloc(num_segments * 4);
    float theta = 2 * 3.1415926 / float(num_segments);
    float tangetial_factor = std::tan(theta);//calculate the tangential factor
    float radial_factor = std::cos(theta);//calculate the radial factor

    float x = radius;//we start at angle = 0
    float y = 0;
    float z = depthRatio * radius;

    for(int ii = 0; ii < num_segments; ii++) {
        AddVertex(x + cx, y + cy, cz + z, edge);
        //calculate the tangential vector
        //remember, the radial vector is (x, y)
        //to get the tangential vector we flip those coordinates and negate one of them
        float tx = -y;
        float ty = x;

        //add the tangential vector
        x += tx * tangetial_factor;
        y += ty * tangetial_factor;
        x *= radial_factor;
        y *= radial_factor;
        AddVertex(x + cx, y + cy, cz + z, edge);
        AddVertex(cx, cy, cz, center);
    }
}
void xlVertexIndexedColorAccumulator::AddCircleAsTriangles(float cx, float cy, float cz, float radius, uint32_t center, uint32_t edge, int numSegments) {
    int num_segments = numSegments;
    if (num_segments == -1) {
        num_segments = radius;
    }
    if (num_segments < 16) {
        num_segments = 16;
    }
    PreAlloc(num_segments * 4);
    float theta = 2 * 3.1415926 / float(num_segments);
    float tangetial_factor = std::tan(theta);//calculate the tangential factor
    float radial_factor = std::cos(theta);//calculate the radial factor

    float x = radius;//we start at angle = 0
    float y = 0;

    for(int ii = 0; ii < num_segments; ii++) {
        AddVertex(x + cx, y + cy, cz, edge);
        //calculate the tangential vector
        //remember, the radial vector is (x, y)
        //to get the tangential vector we flip those coordinates and negate one of them
        float tx = -y;
        float ty = x;

        //add the tangential vector
        x += tx * tangetial_factor;
        y += ty * tangetial_factor;
        x *= radial_factor;
        y *= radial_factor;
        AddVertex(x + cx, y + cy, cz, edge);
        AddVertex(cx, cy, cz, center);
    }
}

void xlVertexColorAccumulator::AddCubeAsTriangles(float x, float y, float z, float width, const xlColor &color) {
    float halfwidth = width / 2.0f;

    // front
    AddVertex(x - halfwidth, y + halfwidth, z + halfwidth, color);
    AddVertex(x + halfwidth, y + halfwidth, z + halfwidth, color);
    AddVertex(x + halfwidth, y - halfwidth, z + halfwidth, color);

    AddVertex(x - halfwidth, y + halfwidth, z + halfwidth, color);
    AddVertex(x - halfwidth, y - halfwidth, z + halfwidth, color);
    AddVertex(x + halfwidth, y - halfwidth, z + halfwidth, color);

    // back
    AddVertex(x - halfwidth, y + halfwidth, z - halfwidth, color);
    AddVertex(x + halfwidth, y + halfwidth, z - halfwidth, color);
    AddVertex(x + halfwidth, y - halfwidth, z - halfwidth, color);

    AddVertex(x - halfwidth, y + halfwidth, z - halfwidth, color);
    AddVertex(x - halfwidth, y - halfwidth, z - halfwidth, color);
    AddVertex(x + halfwidth, y - halfwidth, z - halfwidth, color);

    // left side
    AddVertex(x - halfwidth, y + halfwidth, z + halfwidth, color);
    AddVertex(x - halfwidth, y - halfwidth, z + halfwidth, color);
    AddVertex(x - halfwidth, y - halfwidth, z - halfwidth, color);

    AddVertex(x - halfwidth, y + halfwidth, z + halfwidth, color);
    AddVertex(x - halfwidth, y + halfwidth, z - halfwidth, color);
    AddVertex(x - halfwidth, y - halfwidth, z - halfwidth, color);

    // right side
    AddVertex(x + halfwidth, y + halfwidth, z + halfwidth, color);
    AddVertex(x + halfwidth, y - halfwidth, z + halfwidth, color);
    AddVertex(x + halfwidth, y - halfwidth, z - halfwidth, color);

    AddVertex(x + halfwidth, y + halfwidth, z + halfwidth, color);
    AddVertex(x + halfwidth, y + halfwidth, z - halfwidth, color);
    AddVertex(x + halfwidth, y - halfwidth, z - halfwidth, color);

    // top side
    AddVertex(x - halfwidth, y + halfwidth, z + halfwidth, color);
    AddVertex(x + halfwidth, y + halfwidth, z + halfwidth, color);
    AddVertex(x - halfwidth, y + halfwidth, z - halfwidth, color);

    AddVertex(x - halfwidth, y + halfwidth, z - halfwidth, color);
    AddVertex(x + halfwidth, y + halfwidth, z + halfwidth, color);
    AddVertex(x + halfwidth, y + halfwidth, z - halfwidth, color);

    // bottom side
    AddVertex(x - halfwidth, y - halfwidth, z + halfwidth, color);
    AddVertex(x + halfwidth, y - halfwidth, z + halfwidth, color);
    AddVertex(x - halfwidth, y - halfwidth, z - halfwidth, color);

    AddVertex(x + halfwidth, y - halfwidth, z + halfwidth, color);
    AddVertex(x - halfwidth, y - halfwidth, z - halfwidth, color);
    AddVertex(x + halfwidth, y - halfwidth, z - halfwidth, color);
}

void xlVertexColorAccumulator::AddSphereAsTriangles(float x, float y, float z, float radius, const xlColor &color) {
    // FIXME:  draw a square until I get a good sphere routine
    AddCubeAsTriangles(x, y, z, radius*2, color);
}


void xlDisplayList::addToAccumulator(float xOffset, float yOffset,
                                     float width, float height,
                                     xlVertexColorAccumulator &bg) const {
    std::lock_guard<std::recursive_mutex> lg(lock);
    if (empty()) {
        return;
    }
    bg.PreAlloc(size());
    for (const auto &item : *this) {
        bg.AddVertex(xOffset + item.x * width, yOffset + item.y * height, item.color);
    }
}



xlGraphicsProgram::xlGraphicsProgram(xlVertexColorAccumulator *a) : accumulator(a) {
    
}
xlGraphicsProgram::~xlGraphicsProgram() {
    if (accumulator) {
        delete accumulator;
    }
}
void xlGraphicsProgram::runSteps(xlGraphicsContext *ctx) {
    if (accumulator) {
        accumulator->Finalize(false,  false);
    }
    for (auto &a : steps) {
        a(ctx);
    }
}
xlVertexColorAccumulator *xlGraphicsProgram::getAccumulator() {
    return accumulator;
}

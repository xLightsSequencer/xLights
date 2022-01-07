#pragma once

#include <vector>
#include <map>
#include <wx/bitmap.h>
#include <glm/fwd.hpp>

#include "../Color.h"
#include "xlGraphicsAccumulators.h"
#include "xlFontInfo.h"

class xlMesh;
class wxWindow;

class xlGraphicsContext {
public:

    xlGraphicsContext(wxWindow *w) : window(w) {}
    virtual ~xlGraphicsContext() {}

    // Setup the Viewport
    virtual xlGraphicsContext* SetViewport(int x1, int y1, int x2, int y2, bool is3D = false) = 0;


    //create various accumulators/buffers/textures
    virtual xlVertexAccumulator *createVertexAccumulator() = 0;
    virtual xlVertexColorAccumulator *createVertexColorAccumulator() = 0;
    virtual xlVertexTextureAccumulator *createVertexTextureAccumulator() = 0;
    virtual xlVertexIndexedColorAccumulator *createVertexIndexedColorAccumulator() = 0;
    virtual xlTexture *createTextureMipMaps(const std::vector<wxBitmap> &bitmaps) = 0;
    virtual xlTexture *createTextureMipMaps(const std::vector<wxImage> &images) = 0;
    virtual xlTexture *createTexture(const wxImage &image) = 0;
    virtual xlTexture *createTexture(int w, int h, bool bgr, bool alpha) = 0;
    virtual xlTexture *createTextureForFont(const xlFontInfo &font) = 0;
    virtual xlGraphicsProgram *createGraphicsProgram() = 0;
    virtual xlMesh *loadMeshFromObjFile(const std::string &file) = 0;


    //manipulating the matrices
    virtual xlGraphicsContext* PushMatrix() = 0;
    virtual xlGraphicsContext* PopMatrix() = 0;
    virtual xlGraphicsContext* Translate(float x, float y, float z) = 0;
    virtual xlGraphicsContext* Rotate(float angle, float x, float y, float z) = 0;
    virtual xlGraphicsContext* ApplyMatrix(const glm::mat4 &m) = 0;
    virtual xlGraphicsContext* Scale(float w, float h, float z) = 0;
    virtual xlGraphicsContext* SetCamera(const glm::mat4 &m) = 0;
    virtual xlGraphicsContext* SetModelMatrix(const glm::mat4 &m) = 0;
    

    //setters for various states
    virtual xlGraphicsContext* enableBlending(bool e = true) = 0;
    virtual xlGraphicsContext* disableBlending() { enableBlending(false); return this; }

    //drawing methods
    virtual xlGraphicsContext* drawLines(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) = 0;
    virtual xlGraphicsContext* drawLineStrip(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) = 0;
    virtual xlGraphicsContext* drawTriangles(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) = 0;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) = 0;
    virtual xlGraphicsContext* drawPoints(xlVertexAccumulator *vac, const xlColor &c, float pointSize, bool smoothPoints, int start = 0, int count = -1) = 0;

    virtual xlGraphicsContext* drawLines(xlVertexColorAccumulator *vac, int start = 0, int count = -1) = 0;
    virtual xlGraphicsContext* drawLineStrip(xlVertexColorAccumulator *vac, int start = 0, int count = -1) = 0;
    virtual xlGraphicsContext* drawTriangles(xlVertexColorAccumulator *vac, int start = 0, int count = -1) = 0;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexColorAccumulator *vac, int start = 0, int count = -1) = 0;
    virtual xlGraphicsContext* drawPoints(xlVertexColorAccumulator *vac, float pointSize, bool smoothPoints, int start = 0, int count = -1) = 0;

    virtual xlGraphicsContext* drawLines(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) = 0;
    virtual xlGraphicsContext* drawLineStrip(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) = 0;
    virtual xlGraphicsContext* drawTriangles(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) = 0;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) = 0;
    virtual xlGraphicsContext* drawPoints(xlVertexIndexedColorAccumulator *vac, float pointSize, bool smoothPoints, int start = 0, int count = -1) = 0;

    
    virtual xlGraphicsContext* drawTexture(xlTexture *texture,
                             float x, float y, float x2, float y2,
                             float tx = 0.0, float ty = 0.0, float tx2 = 1.0, float ty2 = 1.0,
                             bool smoothScale = true,
                             int brightness = 100, int alpha = 255) = 0;
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, int start = 0, int count = -1) {
        return drawTexture(vac, texture, 100, 255, start, count);
    }
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, const xlColor &c, int start = 0, int count = -1) = 0;
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, int brightness, uint8_t alpha, int start, int count) = 0;
    
    virtual xlGraphicsContext* drawMeshSolids(xlMesh *mesh, int brightness) = 0;
    virtual xlGraphicsContext* drawMeshTransparents(xlMesh *mesh, int brightness) = 0;
    virtual xlGraphicsContext* drawMeshWireframe(xlMesh *mesh, int brightness) = 0;
    
    
    virtual xlGraphicsContext* pushDebugContext(const std::string &label) { return this; }
    virtual xlGraphicsContext* popDebugContext() { return this; }

    
    xlGraphicsContext* setContextualValue(const std::string &n, void *v) {
        contextMap[n] = v;
        return this;
    }
    void *getConotextualValue(const std::string &n) {
        return contextMap[n];
    }
    wxWindow *getWindow() {
        return window;
    }
protected:
    std::map<std::string, void*> contextMap;
    wxWindow *window;
};

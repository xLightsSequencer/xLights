/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/
#pragma once

#include <stack>


#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../xlGraphicsContext.h"
#include "xlGLCanvas.h"


class xlOGL3GraphicsContext : public xlGraphicsContext {
public:
    class OGLFrameData {
    public:
        glm::mat4 MVP;
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;
        glm::mat4 perspectiveMatrix;
    };
    
    xlOGL3GraphicsContext(xlGLCanvas *c);
    virtual ~xlOGL3GraphicsContext();
    
    static bool InitializeSharedContext();
    
    
    virtual xlVertexAccumulator *createVertexAccumulator() override;
    virtual xlVertexColorAccumulator *createVertexColorAccumulator() override;
    virtual xlVertexTextureAccumulator *createVertexTextureAccumulator() override;
    virtual xlTexture *createTextureMipMaps(const std::vector<wxBitmap> &bitmaps) override;
    virtual xlTexture *createTextureMipMaps(const std::vector<wxImage> &images) override;
    virtual xlTexture *createTexture(const wxImage &image) override;
    virtual xlTexture *createTexture(int w, int h, bool bgr, bool alpha) override;
    virtual xlTexture *createTextureForFont(const xlFontInfo &font) override;
    virtual xlGraphicsProgram *createGraphicsProgram() override;


    //drawing methods
    xlGraphicsContext* drawPrimitive(int type, xlVertexAccumulator *vac, const xlColor &c, int start, int count);
    virtual xlGraphicsContext* drawLines(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawLineStrip(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangles(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawPoints(xlVertexAccumulator *vac, const xlColor &c, float pointSize, bool smoothPoints, int start = 0, int count = -1) override;

    xlGraphicsContext* drawPrimitive(int type, xlVertexColorAccumulator *vac, int start, int count);
    virtual xlGraphicsContext* drawLines(xlVertexColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawLineStrip(xlVertexColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangles(xlVertexColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawPoints(xlVertexColorAccumulator *vac, float pointSize, bool smoothPoints, int start = 0, int count = -1) override;


    virtual xlVertexIndexedColorAccumulator *createVertexIndexedColorAccumulator() override;
    virtual xlGraphicsContext* drawLines(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawLineStrip(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangles(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawPoints(xlVertexIndexedColorAccumulator *vac, float pointSize, bool smoothPoints, int start = 0, int count = -1) override;

    
    virtual xlGraphicsContext* drawTexture(xlTexture *texture,
                                           float x, float y, float x2, float y2,
                                           float tx = 0.0, float ty = 0.0, float tx2 = 1.0, float ty2 = 1.0,
                                           bool nearest = true,
                                           int brightness = 100, int alpha = 255) override;
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, int brightness, uint8_t alpha, int start, int count) override;
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, const xlColor &c, int start = 0, int count = -1) override;

    virtual xlMesh *loadMeshFromObjFile(const std::string &file) override;
    virtual xlGraphicsContext* drawMeshSolids(xlMesh *mesh, int brightness, bool useViewMatrix) override;
    virtual xlGraphicsContext* drawMeshTransparents(xlMesh *mesh, int brightness) override;
    virtual xlGraphicsContext* drawMeshWireframe(xlMesh *mesh, int brightness) override;
    
    void drawMesh(xlMesh *mesh, int brightness, bool useViewMatrix, bool transparents);


    virtual xlGraphicsContext* enableBlending(bool e = true) override;

    // Setup the Viewport
    xlGraphicsContext* SetViewport(int x1, int y1, int x2, int y2, bool is3D) override;

    //manipulating the matrices
    virtual xlGraphicsContext* PushMatrix() override;
    virtual xlGraphicsContext* PopMatrix() override;
    virtual xlGraphicsContext* Translate(float x, float y, float z) override;
    virtual xlGraphicsContext* Rotate(float angle, float x, float y, float z) override;
    virtual xlGraphicsContext* Scale(float w, float h, float z) override;
    virtual xlGraphicsContext* ScaleViewMatrix(float w, float h, float z) override;
    virtual xlGraphicsContext* TranslateViewMatrix(float x, float y, float z) override;

    virtual xlGraphicsContext* SetCamera(const glm::mat4 &m) override;
    virtual xlGraphicsContext* SetModelMatrix(const glm::mat4 &m) override;
    virtual xlGraphicsContext* ApplyMatrix(const glm::mat4 &m) override;

    int enableCapabilities = 0;
    bool isBlending = false;
    xlGLCanvas *canvas;
    
    std::stack<glm::mat4> matrixStack;
    OGLFrameData frameData;
    bool frameDataChanged = true;
};

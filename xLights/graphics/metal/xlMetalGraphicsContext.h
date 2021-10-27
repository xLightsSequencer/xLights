#pragma once

#include <stack>

#include "CPPMetal/CPPMetal.hpp"

#include "../xlGraphicsContext.h"
#include "xlMetalCanvas.h"

#include "Shaders/MetalShaderTypes.h"


class xlMetalGraphicsContext : public xlGraphicsContext {
public:
    xlMetalGraphicsContext(xlMetalCanvas *c);
    virtual ~xlMetalGraphicsContext();


    virtual xlVertexAccumulator *createVertexAccumulator() override;

    // Setup the Viewport
    virtual void SetViewport(int x1, int y1, int x2, int y2, bool is3D = false) override;

    //setters for various states


    //drawing methods
    virtual void drawLines(xlVertexAccumulator *vac, const xlColor &c) override;
    virtual void drawLineLoop(xlVertexAccumulator *vac, const xlColor &c) override;
    virtual void drawLineStrip(xlVertexAccumulator *vac, const xlColor &c) override;


    //manipulating the matrices
    virtual void PushMatrix() override;
    virtual void PopMatrix() override;
    virtual void Translate(float x, float y, float z) override;
    virtual void Rotate(float angle, float x, float y, float z) override;
    virtual void Scale(float w, float h, float z) override;

protected:
    xlMetalCanvas *canvas;
    MTL::CommandBuffer buffer;
    MTL::RenderCommandEncoder *encoder = nullptr;

    void setPipelineState(const std::string &name, const char *vShader, const char *fShader);

    std::string lastPipeline;
    FrameData frameData;
    std::stack<simd::float4x4> matrixStack;
};

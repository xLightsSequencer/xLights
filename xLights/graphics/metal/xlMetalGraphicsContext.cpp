
#include "xlMetalGraphicsContext.h"

#include "Shaders/SIMDMathUtilities.h"

xlMetalGraphicsContext::xlMetalGraphicsContext(xlMetalCanvas *c) : xlGraphicsContext(),  canvas(c), buffer(c->getMTLCommandQueue()->commandBuffer()) {
    MTL::RenderPassDescriptor *renderPass = new MTL::RenderPassDescriptor();

    drawable = c->getMTKView()->nextDrawable();
    renderPass->colorAttachments[0].texture(*(drawable->texture()));
    renderPass->colorAttachments[0].loadAction(MTL::LoadActionClear);

    xlColor bg = canvas->ClearBackgroundColor();
    float r = bg.red;
    float g = bg.green;
    float b = bg.blue;
    float a = bg.alpha;
    r /= 255.0f;
    g /= 255.0f;
    b /= 255.0f;
    a /= 255.0f;
    renderPass->colorAttachments[0].clearColor({r, g, b, a});
    renderPass->colorAttachments[0].storeAction(MTL::StoreActionStore);

    encoder = new MTL::RenderCommandEncoder(buffer.renderCommandEncoderWithDescriptor(*renderPass));


    frameData.MVP = matrix4x4_identity();
    frameData.fragmentColor = {0.0f, 0.0f, 0.0f, 1.0f};
}
xlMetalGraphicsContext::~xlMetalGraphicsContext() {
    encoder->endEncoding();
    buffer.presentDrawable(*drawable);
    buffer.commit();
    //buffer.waitUntilCompleted();
    delete encoder;
}



class xlMetalVertexAccumulator : public xlGraphicsContext::xlVertexAccumulator {
public:
    xlMetalVertexAccumulator() {}
    virtual ~xlMetalVertexAccumulator() {
        if (buffer) {
            delete buffer;
        }
    }

    virtual void Reset() override { if (!finalized) { count = 0; vertices.resize(0); } }
    virtual void PreAlloc(unsigned int i) override {  vertices.reserve(i); }
    virtual void AddVertex(float x, float y, float z) override {
        if (!finalized) {
            vertices.emplace_back((simd_float4){x, y, z, 1.0f});
            count++;
        }
    }
    virtual uint32_t getCount() override { return count; }


    virtual void Finalize(bool mc) override {
        finalized = true;
        mayChange = mc;
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z) override {
        if (vertex < count) {
            if (bufferVertices) {
                bufferVertices[vertex] = (simd_float4){x, y, z, 1.0f};
            } else {
                vertices[vertex] = (simd_float4){x, y, z, 1.0f};
            }
        }
    }
    virtual void FlushRange(uint32_t start, uint32_t len) override {
        if (buffer) {
            uint32_t s = start * sizeof(simd_float4);
            uint32_t l = len * sizeof(simd_float4);
            buffer->didModifyRange({s, l});
        }
    }


    void SetBufferBytes(MTL::Device *device, MTL::RenderCommandEncoder *encoder, int index) {
        int sz = count * sizeof(simd_float4);
        if (finalized) {
            if (!buffer) {
                buffer = device->newBufferWithBytes(&vertices[0], sizeof(simd_float4) * count, MTL::ResourceStorageModeManaged);
                if (mayChange) {
                    bufferVertices = (simd_float4 *)buffer->contents();
                }
            }
            encoder->setVertexBuffer(*buffer, 0, index);
        } else if (sz > 4095) {
            if (buffer) {
                delete buffer;
            }
            buffer = device->newBufferWithBytes(&vertices[0], sz, MTL::ResourceStorageModeManaged);
            encoder->setVertexBuffer(*buffer, 0, index);
        } else {
            encoder->setVertexBytes(&vertices[0], sizeof(simd_float4) * count, index);
        }
    }

    uint32_t count = 0;
    std::vector<simd_float4> vertices;

    bool finalized = false;
    bool mayChange = false;
    simd_float4 *bufferVertices = nullptr;
    MTL::Buffer *buffer = nullptr;
};



class xlMetalVertexColorAccumulator : public xlGraphicsContext::xlVertexColorAccumulator {
public:
    xlMetalVertexColorAccumulator() {}
    virtual ~xlMetalVertexColorAccumulator() {
        if (vbuffer) {
            delete vbuffer;
        }
        if (cbuffer) {
            delete cbuffer;
        }
    }

    virtual void Reset() override { if (!finalized) { count = 0; vertices.resize(0); colors.resize(0); } }
    virtual void PreAlloc(unsigned int i) override {  vertices.reserve(i); colors.reserve(i); }
    virtual void AddVertex(float x, float y, float z, const xlColor &c) override {
        if (!finalized) {
            vertices.emplace_back((simd_float4){x, y, z, 1.0f});
            colors.emplace_back((simd_uchar4){c.red, c.green, c.blue, c.alpha});
            count++;
        }
    }
    virtual uint32_t getCount() override { return count; }


    virtual void Finalize(bool mcv, bool mcc) override {
        finalized = true;
        mayChangeVertices = mcv;
        mayChangeColors = mcc;
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z, const xlColor &c) override {
        if (vertex < count) {
            if (bufferVertices) {
                bufferVertices[vertex] = (simd_float4){x, y, z, 1.0f};
            } else {
                vertices[vertex] = (simd_float4){x, y, z, 1.0f};
            }
            if (bufferColors) {
                bufferColors[vertex] = (simd_uchar4){c.red, c.green, c.blue, c.alpha};
            } else {
                colors[vertex] = (simd_uchar4){c.red, c.green, c.blue, c.alpha};
            }
        }
    }
    virtual void FlushRange(uint32_t start, uint32_t len) override {
        if (vbuffer) {
            uint32_t s = start * sizeof(simd_float4);
            uint32_t l = len * sizeof(simd_float4);
            vbuffer->didModifyRange({s, l});
        }
        if (cbuffer) {
            uint32_t s = start * sizeof(simd_uchar4);
            uint32_t l = len * sizeof(simd_uchar4);
            cbuffer->didModifyRange({s, l});
        }
    }


    void SetBufferBytes(MTL::Device *device, MTL::RenderCommandEncoder *encoder, int indexV, int indexC) {
        int sz = count * sizeof(simd_float4);
        if (finalized) {
            if (!vbuffer) {
                vbuffer = device->newBufferWithBytes(&vertices[0], sizeof(simd_float4) * count, MTL::ResourceStorageModeManaged);
                if (mayChangeVertices) {
                    bufferVertices = (simd_float4 *)vbuffer->contents();
                }
            }
            encoder->setVertexBuffer(*vbuffer, 0, indexV);
        } else if (sz > 4095) {
            if (vbuffer) {
                delete vbuffer;
            }
            vbuffer = device->newBufferWithBytes(&vertices[0], sz, MTL::ResourceStorageModeManaged);
            encoder->setVertexBuffer(*vbuffer, 0, indexV);
        } else {
            encoder->setVertexBytes(&vertices[0], sizeof(simd_float4) * count, indexV);
        }

        sz = count * sizeof(simd_uchar4);
        if (finalized) {
            if (!cbuffer) {
                cbuffer = device->newBufferWithBytes(&colors[0], sizeof(simd_uchar4) * count, MTL::ResourceStorageModeManaged);
                if (mayChangeColors) {
                    bufferColors = (simd_uchar4 *)cbuffer->contents();
                }
            }
            encoder->setVertexBuffer(*cbuffer, 0, indexC);
        } else if (sz > 4095) {
            if (cbuffer) {
                delete cbuffer;
            }
            cbuffer = device->newBufferWithBytes(&colors[0], sz, MTL::ResourceStorageModeManaged);
            encoder->setVertexBuffer(*cbuffer, 0, indexC);
        } else {
            encoder->setVertexBytes(&colors[0], sizeof(simd_uchar4) * count, indexC);
        }
    }

    uint32_t count = 0;
    std::vector<simd_float4> vertices;
    std::vector<simd_uchar4> colors;

    bool finalized = false;
    bool mayChangeColors = false;
    bool mayChangeVertices = false;
    simd_float4 *bufferVertices = nullptr;
    simd_uchar4 *bufferColors = nullptr;
    MTL::Buffer *vbuffer = nullptr;
    MTL::Buffer *cbuffer = nullptr;
};


xlGraphicsContext::xlVertexAccumulator *xlMetalGraphicsContext::createVertexAccumulator() {
    return new xlMetalVertexAccumulator();
}
xlGraphicsContext::xlVertexColorAccumulator *xlMetalGraphicsContext::createVertexColorAccumulator() {
    return new xlMetalVertexColorAccumulator();
}


//drawing methods
void xlMetalGraphicsContext::drawLines(xlGraphicsContext::xlVertexAccumulator *vac, const xlColor &c) {
    drawPrimitive(MTL::PrimitiveTypeLine, vac, c);
}
void xlMetalGraphicsContext::drawLineStrip(xlGraphicsContext::xlVertexAccumulator *vac, const xlColor &c) {
    drawPrimitive(MTL::PrimitiveTypeLineStrip, vac, c);
}
void xlMetalGraphicsContext::drawTriangles(xlVertexAccumulator *vac, const xlColor &c) {
    drawPrimitive(MTL::PrimitiveTypeTriangle, vac, c);
}
void xlMetalGraphicsContext::drawTriangleStrip(xlVertexAccumulator *vac, const xlColor &c) {
    drawPrimitive(MTL::PrimitiveTypeTriangleStrip, vac, c);
}
void xlMetalGraphicsContext::drawPrimitive(MTL::PrimitiveType type, xlVertexAccumulator *vac, const xlColor &c) {
    std::string name = blending ? "singleColorProgramBlend" : "singleColorProgram";
    setPipelineState(name, "singleColorVertexShader", "singleColorFragmentShader");
    xlMetalVertexAccumulator *mva = (xlMetalVertexAccumulator*)vac;
    mva->SetBufferBytes(canvas->getMTLDevice(), encoder, BufferIndexMeshPositions);

    frameData.RenderType = 0;
    frameData.fragmentColor.r = c.red;
    frameData.fragmentColor.g = c.green;
    frameData.fragmentColor.b = c.blue;
    frameData.fragmentColor.a = c.alpha;
    frameData.fragmentColor /= 255.0f;

    encoder->setVertexBytes(&frameData, sizeof(frameData), BufferIndexFrameData);
    encoder->setFragmentBytes(&frameData, sizeof(frameData), BufferIndexFrameData);
    encoder->drawPrimitives(type, 0, vac->getCount());
}
void xlMetalGraphicsContext::drawLines(xlGraphicsContext::xlVertexColorAccumulator *vac) {
    drawPrimitive(MTL::PrimitiveTypeLine, vac);
}
void xlMetalGraphicsContext::drawLineStrip(xlGraphicsContext::xlVertexColorAccumulator *vac) {
    drawPrimitive(MTL::PrimitiveTypeLineStrip, vac);
}
void xlMetalGraphicsContext::drawTriangles(xlVertexColorAccumulator *vac) {
    drawPrimitive(MTL::PrimitiveTypeTriangle, vac);
}
void xlMetalGraphicsContext::drawTriangleStrip(xlVertexColorAccumulator *vac) {
    drawPrimitive(MTL::PrimitiveTypeTriangleStrip, vac);
}
void xlMetalGraphicsContext::drawPrimitive(MTL::PrimitiveType type, xlVertexColorAccumulator *vac) {
    std::string name = blending ? "multiColorProgramBlend" : "multiColorProgram";
    setPipelineState(name, "multiColorVertexShader", "multiColorFragmentShader");
    xlMetalVertexColorAccumulator *mva = (xlMetalVertexColorAccumulator*)vac;
    mva->SetBufferBytes(canvas->getMTLDevice(), encoder, BufferIndexMeshPositions, BufferIndexMeshColors);

    encoder->setVertexBytes(&frameData, sizeof(frameData), BufferIndexFrameData);
    encoder->setFragmentBytes(&frameData, sizeof(frameData), BufferIndexFrameData);
    encoder->drawPrimitives(type, 0, vac->getCount());
}

void xlMetalGraphicsContext::SetViewport(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y, bool is3D) {
    if (is3D) {

    } else {
        int x, y, x2, y2;
        x = topleft_x;
        y = std::min(bottomright_y, topleft_y);
        x2 = bottomright_x;
        y2 = std::max(bottomright_y,topleft_y);

        double w = std::max(x, x2) - std::min(x, x2);
        double h = std::max(y, y2) - std::min(y, y2);

        MTL::Viewport vp = {  (double)topleft_x, (double)topleft_y, w, h, 1.0, 1.0 };
        encoder->setViewport(vp);
        frameData.MVP = matrix_ortho_left_hand(topleft_x, bottomright_x, bottomright_y, topleft_y, 1.0, 0.0);
    }
}



//manipulating the matrices
void xlMetalGraphicsContext::PushMatrix() {
    matrixStack.push(frameData.MVP);
}
void xlMetalGraphicsContext::PopMatrix() {
    if (!matrixStack.empty()) {
        frameData.MVP = matrixStack.top();
        matrixStack.pop();
    }
}
void xlMetalGraphicsContext::Translate(float x, float y, float z) {
    
}
void xlMetalGraphicsContext::Rotate(float angle, float x, float y, float z) {
}
void xlMetalGraphicsContext::Scale(float w, float h, float z) {
}



void xlMetalGraphicsContext::setPipelineState(const std::string &name, const char *vShader, const char *fShader) {
    if (lastPipeline != name) {
        encoder->setRenderPipelineState(canvas->getPipelineState(name, vShader, fShader, blending));
        lastPipeline = name;
    }
}

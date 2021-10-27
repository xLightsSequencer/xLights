
#include "xlMetalGraphicsContext.h"

#include "Shaders/SIMDMathUtilities.h"

xlMetalGraphicsContext::xlMetalGraphicsContext(xlMetalCanvas *c) : xlGraphicsContext(),  canvas(c), buffer(c->getMTLCommandQueue()->commandBuffer()) {
    MTL::RenderPassDescriptor *renderPass = new MTL::RenderPassDescriptor();

    renderPass->colorAttachments[0].texture(*(c->getMTKView()->currentDrawable()->texture()));
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
    buffer.presentDrawable(*(canvas->getMTKView()->currentDrawable()));
    buffer.commit();
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


xlGraphicsContext::xlVertexAccumulator *xlMetalGraphicsContext::createVertexAccumulator() {
    return new xlMetalVertexAccumulator();
}



//drawing methods
void xlMetalGraphicsContext::drawLines(xlGraphicsContext::xlVertexAccumulator *vac, const xlColor &c) {
    setPipelineState("singleColorProgram", "singleColorVertexShader", "singleColorFragmentShader");
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
    encoder->drawPrimitives(MTL::PrimitiveTypeLine, 0, mva->count);
}
void xlMetalGraphicsContext::drawLineLoop(xlGraphicsContext::xlVertexAccumulator *vac, const xlColor &c) {
    xlMetalVertexAccumulator *mva = (xlMetalVertexAccumulator*)vac;
    int count = mva->count;
    if (count) {
        mva->vertices.emplace_back(mva->vertices[0]);
        mva->count++;
        drawLineStrip(vac, c);
        mva->count--;
        mva->vertices.resize(mva->vertices.size() - 1);
    }
}
void xlMetalGraphicsContext::drawLineStrip(xlGraphicsContext::xlVertexAccumulator *vac, const xlColor &c) {
    setPipelineState("singleColorProgram", "singleColorVertexShader", "singleColorFragmentShader");
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
    encoder->drawPrimitives(MTL::PrimitiveTypeLineStrip, 0, vac->getCount());
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
        encoder->setRenderPipelineState(canvas->getPipelineState(name, vShader, fShader));
        lastPipeline = name;
    }
}


#include "xlMetalGraphicsContext.h"

#include "Shaders/SIMDMathUtilities.h"

xlMetalGraphicsContext::xlMetalGraphicsContext(xlMetalCanvas *c) : xlGraphicsContext(),  canvas(c) {
    buffer = [c->getMTLCommandQueue() commandBuffer];
    MTLRenderPassDescriptor *renderPass = [[MTLRenderPassDescriptor alloc] init];

    id<CAMetalDrawable> d2 = [c->getMTKView() currentDrawable];
    CAMetalLayer *layer = [d2 layer];
    drawable = [layer nextDrawable];
    renderPass.colorAttachments[0].texture = [drawable texture];
    renderPass.colorAttachments[0].loadAction = MTLLoadActionClear;

    xlColor bg = canvas->ClearBackgroundColor();
    float r = bg.red;
    float g = bg.green;
    float b = bg.blue;
    float a = bg.alpha;
    r /= 255.0f;
    g /= 255.0f;
    b /= 255.0f;
    a /= 255.0f;
    renderPass.colorAttachments[0].clearColor = {r, g, b, a};
    renderPass.colorAttachments[0].storeAction = MTLStoreActionStore;

    encoder = [buffer renderCommandEncoderWithDescriptor:renderPass];
    [renderPass release];

    frameData.MVP = matrix4x4_identity();
    frameData.fragmentColor = {0.0f, 0.0f, 0.0f, 1.0f};
}
xlMetalGraphicsContext::~xlMetalGraphicsContext() {
    [encoder endEncoding];
    [buffer presentDrawable:drawable];
    [buffer commit];

    //[buffer waitUntilCompleted];
}



class xlMetalVertexAccumulator : public xlGraphicsContext::xlVertexAccumulator {
public:
    xlMetalVertexAccumulator() {}
    virtual ~xlMetalVertexAccumulator() {
        if (buffer) {
            [buffer release];
        }
    }

    virtual void Reset() override { if (!finalized) { count = 0; vertices.resize(0); } }
    virtual void PreAlloc(unsigned int i) override {  vertices.reserve(i); }
    virtual void AddVertex(float x, float y, float z) override {
        if (!finalized) {
            vertices.emplace_back((simd_float3){x, y, z});
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
                bufferVertices[vertex] = (simd_float3){x, y, z};
            } else {
                vertices[vertex] = (simd_float3){x, y, z};
            }
        }
    }
    virtual void FlushRange(uint32_t start, uint32_t len) override {
        if (buffer) {
            uint32_t s = start * sizeof(simd_float4);
            uint32_t l = len * sizeof(simd_float4);
            [buffer didModifyRange:NSMakeRange(s, l)];
        }
    }


    void SetBufferBytes(id<MTLDevice> device, id<MTLRenderCommandEncoder> encoder, int index) {
        int sz = count * sizeof(simd_float4);
        if (finalized) {
            if (!buffer) {
                buffer = [device newBufferWithBytes:&vertices[0] length:(sizeof(simd_float3) * count) options:MTLResourceStorageModeManaged];
                if (mayChange) {
                    bufferVertices = (simd_float3 *)buffer.contents;
                }
            }
            [encoder setVertexBuffer:buffer offset:0 atIndex:index];
        } else if (sz > 4095) {
            if (buffer) {
                buffer = nil;
            }
            buffer = [device newBufferWithBytes:&vertices[0] length:sz options:MTLResourceStorageModeManaged];
            [encoder setVertexBuffer:buffer offset:0 atIndex:index];
        } else {
            [encoder setVertexBytes:&vertices[0] length:(sizeof(simd_float3) * count) atIndex:index];
        }
    }

    uint32_t count = 0;
    std::vector<simd_float3> vertices;

    bool finalized = false;
    bool mayChange = false;
    simd_float3 *bufferVertices = nullptr;
    id<MTLBuffer> buffer = nullptr;
};



class xlMetalVertexColorAccumulator : public xlGraphicsContext::xlVertexColorAccumulator {
public:
    xlMetalVertexColorAccumulator() {}
    virtual ~xlMetalVertexColorAccumulator() {
        if (vbuffer) {
            [vbuffer release];
        }
        if (cbuffer) {
            [cbuffer release];
        }
    }

    virtual void Reset() override { if (!finalized) { count = 0; vertices.resize(0); colors.resize(0); } }
    virtual void PreAlloc(unsigned int i) override {  vertices.reserve(i); colors.reserve(i); }
    virtual void AddVertex(float x, float y, float z, const xlColor &c) override {
        if (!finalized) {
            vertices.emplace_back((simd_float3){x, y, z});
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
                bufferVertices[vertex] = (simd_float3){x, y, z};
            } else {
                vertices[vertex] = (simd_float3){x, y, z};
            }
            if (bufferColors) {
                bufferColors[vertex] = (simd_uchar4){c.red, c.green, c.blue, c.alpha};
            } else {
                colors[vertex] = (simd_uchar4){c.red, c.green, c.blue, c.alpha};
            }
        }
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z) override {
        if (vertex < count) {
            if (bufferVertices) {
                bufferVertices[vertex] = (simd_float3){x, y, z};
            } else {
                vertices[vertex] = (simd_float3){x, y, z};
            }
        }
    }
    virtual void SetVertex(uint32_t vertex, const xlColor &c) override {
        if (vertex < count) {
            if (bufferColors) {
                bufferColors[vertex] = (simd_uchar4){c.red, c.green, c.blue, c.alpha};
            } else {
                colors[vertex] = (simd_uchar4){c.red, c.green, c.blue, c.alpha};
            }
        }
    }
    virtual void FlushRange(uint32_t start, uint32_t len) override {
        if (bufferVertices) {
            uint32_t s = start * sizeof(simd_float3);
            uint32_t l = len * sizeof(simd_float3);
            [vbuffer didModifyRange:NSMakeRange(s, l)];
        }
        if (bufferColors) {
            uint32_t s = start * sizeof(simd_uchar4);
            uint32_t l = len * sizeof(simd_uchar4);
            [cbuffer didModifyRange:NSMakeRange(s, l)];
        }
    }


    void SetBufferBytes(id<MTLDevice> device, id<MTLRenderCommandEncoder> encoder, int indexV, int indexC) {
        int sz = count * sizeof(simd_float3);
        if (finalized) {
            if (!vbuffer) {
                vbuffer = [device newBufferWithBytes:&vertices[0] length:sz options:MTLResourceStorageModeManaged];
                if (mayChangeVertices) {
                    bufferVertices = (simd_float3 *)vbuffer.contents;
                }
            }
            [encoder setVertexBuffer:vbuffer offset:0 atIndex:indexV];
        } else if (sz > 4095) {
            vbuffer = nil;
            vbuffer = [device newBufferWithBytes:&vertices[0] length:sz options:MTLResourceStorageModeManaged];
            [encoder setVertexBuffer:vbuffer offset:0 atIndex:indexV];
        } else {
            [encoder setVertexBytes:&vertices[0] length:sz atIndex:indexV];
        }

        sz = count * sizeof(simd_uchar4);
        if (finalized) {
            if (!cbuffer) {
                cbuffer = [device newBufferWithBytes:&colors[0] length:sz options:MTLResourceStorageModeManaged];
                if (mayChangeColors) {
                    bufferColors = (simd_uchar4 *)cbuffer.contents;
                }
            }
            [encoder setVertexBuffer:cbuffer offset:0 atIndex:indexC];
        } else if (sz > 4095) {
            cbuffer = nil;
            cbuffer = [device newBufferWithBytes:&colors[0] length:sz options:MTLResourceStorageModeManaged];
            [encoder setVertexBuffer:cbuffer offset:0 atIndex:indexC];
        } else {
            [encoder setVertexBytes:&colors[0] length:sz atIndex:indexC];
        }
    }

    uint32_t count = 0;
    std::vector<simd_float3> vertices;
    std::vector<simd_uchar4> colors;

    bool finalized = false;
    bool mayChangeColors = false;
    bool mayChangeVertices = false;
    simd_float3 *bufferVertices = nullptr;
    simd_uchar4 *bufferColors = nullptr;
    id<MTLBuffer> vbuffer = nil;
    id<MTLBuffer> cbuffer = nil;
};

class xlMetalTexture : public xlGraphicsContext::xlTexture {
public:
    xlMetalTexture() : xlGraphicsContext::xlTexture(), texture(nil) {
    }
    virtual ~xlMetalTexture() {
        if (texture) {
            [texture release];
        }
    }

    id<MTLTexture> texture;
};


xlGraphicsContext::xlVertexAccumulator *xlMetalGraphicsContext::createVertexAccumulator() {
    return new xlMetalVertexAccumulator();
}
xlGraphicsContext::xlVertexColorAccumulator *xlMetalGraphicsContext::createVertexColorAccumulator() {
    return new xlMetalVertexColorAccumulator();
}


static void getImageBytes(const wxImage &img, uint8_t *bytes) {
    unsigned char * alpha = img.HasAlpha() ? img.GetAlpha() : nullptr;
    unsigned char * data = img.GetData();
    int maxLen = img.GetWidth() * img.GetHeight();
    int bytesIdx = 0;
    int dataIdx = 0;
    for (int x = 0; x < maxLen; x++) {
        bytes[bytesIdx++] = data[dataIdx++];
        bytes[bytesIdx++] = data[dataIdx++];
        bytes[bytesIdx++] = data[dataIdx++];
        if (alpha) {
            bytes[bytesIdx++] = alpha[x];
        } else {
            bytes[bytesIdx++] = 255;
        }
    }
}
xlGraphicsContext::xlTexture *xlMetalGraphicsContext::createTextureMipMaps(const std::vector<wxBitmap> &bitmaps) {
    std::vector<wxImage> images;
    for (auto &a : bitmaps) {
        images.push_back(a.ConvertToImage());
    }
    return createTextureMipMaps(images);
}
xlGraphicsContext::xlTexture *xlMetalGraphicsContext::createTextureMipMaps(const std::vector<wxImage> &images) {
    xlMetalTexture *txt = new xlMetalTexture();

    @autoreleasepool {
        MTLTextureDescriptor * desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                        width:images[0].GetWidth()
                                                                                        height:images[0].GetHeight()
                                                                                        mipmapped:true];
        txt->texture = [canvas->getMTLDevice() newTextureWithDescriptor:desc];


        int levels = [txt->texture mipmapLevelCount];
        wxImage img;
        uint8_t *bytes = (uint8_t *)malloc(images[0].GetWidth() * images[0].GetHeight() * 4);
        for (int x = 0; x < levels; x++) {
            if (x < images.size()) {
                img = images[x];
            } else {
                img = img.Scale(img.GetWidth() / 2, img.GetHeight() / 2, wxIMAGE_QUALITY_HIGH);
            }
            getImageBytes(img, bytes);

            const uint32_t w = img.GetWidth();
            const uint32_t h = img.GetHeight();
            int rlen = w * 4;
            MTLRegion region = {0, 0, 0, w, h , 1};
            [txt->texture replaceRegion:region mipmapLevel:x withBytes:bytes bytesPerRow:rlen];
        }
        free(bytes);
    }
    return txt;
}
xlGraphicsContext::xlTexture *xlMetalGraphicsContext::createTexture(const wxImage &image) {
    xlMetalTexture *txt = new xlMetalTexture();

    @autoreleasepool {
        const uint32_t w = image.GetWidth();
        const uint32_t h = image.GetHeight();

        MTLTextureDescriptor * desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                        width:w
                                                                                        height:h
                                                                                        mipmapped:false];
        txt->texture = [canvas->getMTLDevice() newTextureWithDescriptor:desc];


        uint8_t *bytes = (uint8_t *)malloc(w * h * 4);
        getImageBytes(image, bytes);
        int rlen = w * 4;
        MTLRegion region = {0, 0, 0, w, h , 1};
        [txt->texture replaceRegion:region mipmapLevel:0 withBytes:bytes bytesPerRow:rlen];
        free(bytes);
    }
    return txt;
}

//drawing methods
void xlMetalGraphicsContext::drawLines(xlGraphicsContext::xlVertexAccumulator *vac, const xlColor &c) {
    drawPrimitive(MTLPrimitiveTypeLine, vac, c);
}
void xlMetalGraphicsContext::drawLineStrip(xlGraphicsContext::xlVertexAccumulator *vac, const xlColor &c) {
    drawPrimitive(MTLPrimitiveTypeLineStrip, vac, c);
}
void xlMetalGraphicsContext::drawTriangles(xlVertexAccumulator *vac, const xlColor &c) {
    drawPrimitive(MTLPrimitiveTypeTriangle, vac, c);
}
void xlMetalGraphicsContext::drawTriangleStrip(xlVertexAccumulator *vac, const xlColor &c) {
    drawPrimitive(MTLPrimitiveTypeTriangleStrip, vac, c);
}
void xlMetalGraphicsContext::drawPrimitive(MTLPrimitiveType type, xlVertexAccumulator *vac, const xlColor &c) {
    std::string name = blending ? "singleColorProgramBlend" : "singleColorProgram";
    setPipelineState(name, "singleColorVertexShader", "singleColorFragmentShader");
    xlMetalVertexAccumulator *mva = (xlMetalVertexAccumulator*)vac;
    mva->SetBufferBytes(canvas->getMTLDevice(), encoder, BufferIndexMeshPositions);

    frameData.fragmentColor.r = c.red;
    frameData.fragmentColor.g = c.green;
    frameData.fragmentColor.b = c.blue;
    frameData.fragmentColor.a = c.alpha;
    frameData.fragmentColor /= 255.0f;

    [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
    [encoder setFragmentBytes:&frameData length:sizeof(frameData) atIndex:BufferIndexFrameData];
    frameDataChanged = false;
    [encoder drawPrimitives:type vertexStart:0 vertexCount:vac->getCount()];
}
void xlMetalGraphicsContext::drawLines(xlGraphicsContext::xlVertexColorAccumulator *vac) {
    drawPrimitive(MTLPrimitiveTypeLine, vac);
}
void xlMetalGraphicsContext::drawLineStrip(xlGraphicsContext::xlVertexColorAccumulator *vac) {
    drawPrimitive(MTLPrimitiveTypeLineStrip, vac);
}
void xlMetalGraphicsContext::drawTriangles(xlVertexColorAccumulator *vac) {
    drawPrimitive(MTLPrimitiveTypeTriangle, vac);
}
void xlMetalGraphicsContext::drawTriangleStrip(xlVertexColorAccumulator *vac) {
    drawPrimitive(MTLPrimitiveTypeTriangleStrip, vac);
}
void xlMetalGraphicsContext::drawPrimitive(MTLPrimitiveType type, xlVertexColorAccumulator *vac) {
    std::string name = blending ? "multiColorProgramBlend" : "multiColorProgram";
    setPipelineState(name, "multiColorVertexShader", "multiColorFragmentShader");
    xlMetalVertexColorAccumulator *mva = (xlMetalVertexColorAccumulator*)vac;
    mva->SetBufferBytes(canvas->getMTLDevice(), encoder, BufferIndexMeshPositions, BufferIndexMeshColors);

    if (frameDataChanged) {
        [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
        [encoder setFragmentBytes:&frameData length:sizeof(frameData) atIndex:BufferIndexFrameData];
        frameDataChanged = false;
    }
    [encoder drawPrimitives:type vertexStart:0 vertexCount:vac->getCount()];
}

void xlMetalGraphicsContext::drawTexture(xlTexture *texture,
                                         float x, float y, float x2, float y2,
                                         float tx, float ty, float tx2, float ty2,
                                         bool linearScale) {
    xlMetalVertexAccumulator va;
    va.PreAlloc(6);
    va.AddVertex(x, y, 0);
    va.AddVertex(x2, y, 0);
    va.AddVertex(x2, y2, 0);
    va.AddVertex(x, y, 0);
    va.AddVertex(x, y2, 0);
    va.AddVertex(x2, y2, 0);

    std::string name = blending ? "textureProgramBlend" : "textureProgram";
    setPipelineState(name, "textureVertexShader", linearScale ? "textureFragmentShader" : "textureNearestFragmentShader");
    va.SetBufferBytes(canvas->getMTLDevice(), encoder, BufferIndexMeshPositions);

    float texturePoints[] {
        tx, ty,
        tx2, ty,
        tx2, ty2,
        tx, ty,
        tx, ty2,
        tx2, ty2
    };
    [encoder setVertexBytes:texturePoints length:(12*sizeof(float)) atIndex:BufferIndexTexturePositions];

    if (frameDataChanged) {
        [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
        [encoder setFragmentBytes:&frameData length:sizeof(frameData) atIndex:BufferIndexFrameData];
        frameDataChanged = false;
    }

    xlMetalTexture *txt = (xlMetalTexture*)texture;
    [encoder setFragmentTexture:txt->texture atIndex:TextureIndexBase];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:va.getCount()];
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

        MTLViewport vp = {  (double)topleft_x, (double)topleft_y, w, h, 1.0, 1.0 };
        [encoder setViewport:vp];
        frameData.MVP = matrix_ortho_left_hand(topleft_x, bottomright_x, bottomright_y, topleft_y, 1.0, 0.0);
        frameDataChanged = true;
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



bool xlMetalGraphicsContext::setPipelineState(const std::string &name, const char *vShader, const char *fShader) {
    if (lastPipeline != name) {
        [encoder setRenderPipelineState:canvas->getPipelineState(name, vShader, fShader, blending)];
        lastPipeline = name;
        return true;
    }
    return false;
}

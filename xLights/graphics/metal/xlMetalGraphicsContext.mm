
#include "xlMetalGraphicsContext.h"

#include "Shaders/SIMDMathUtilities.h"

xlMetalGraphicsContext::xlMetalGraphicsContext(xlMetalCanvas *c) : xlGraphicsContext(),  canvas(c) {
    id<CAMetalDrawable> d2 = [c->getMTKView() currentDrawable];
    CAMetalLayer *layer = [d2 layer];
    drawable = [layer nextDrawable];

    if (drawable != nil) {
        [drawable retain];

        buffer = [c->getMTLCommandQueue() commandBuffer];
        MTLRenderPassDescriptor *renderPass = [[MTLRenderPassDescriptor alloc] init];

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
    } else {
        buffer = nil;
        encoder = nil;
    }
}
#include <wx/time.h>
xlMetalGraphicsContext::~xlMetalGraphicsContext() {
    if (encoder != nil) {
        @autoreleasepool {
            [encoder endEncoding];
            [buffer presentDrawable:drawable];
            [buffer commit];
            //[buffer waitUntilCompleted];

            /*
            [buffer waitUntilScheduled];
            [drawable present];
            [drawable release];
            */
        }
    }
}

bool xlMetalGraphicsContext::hasDrawable() {
    return drawable != nil;
}



class xlMetalVertexAccumulator : public xlVertexAccumulator {
public:
    xlMetalVertexAccumulator() {}
    virtual ~xlMetalVertexAccumulator() {
        if (buffer) {
            [buffer release];
        }
    }

    virtual void Reset() override {
        if (!finalized) {
            count = 0;
            vertices.resize(0);
        }
    }
    virtual void PreAlloc(unsigned int i) override {
        vertices.reserve(i);
    }
    virtual void AddVertex(float x, float y, float z) override {
        if (!finalized) {
            if (bufferVertices) {
                if (count < bufferCount) {
                    bufferVertices[count] = (simd_float3){x, y, z};
                } else {
                    vertices.resize(count + 1);
                    memcpy(&vertices[0], bufferVertices, count * sizeof(simd_float3));
                    vertices[count] = (simd_float3){x, y, z};
                    bufferVertices = nullptr;
                    [buffer release];
                    buffer = nil;
                }
            } else {
                vertices.emplace_back((simd_float3){x, y, z});
            }
            count++;
        }
    }
    virtual uint32_t getCount() override {
        return count;
    }

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
        if (buffer && (!finalized || mayChange)) {
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
                    bufferCount = count;
                }
            }
            [encoder setVertexBuffer:buffer offset:0 atIndex:index];
        } else if (sz > 4095 || buffer != nil) {
            if (!buffer) {
                buffer = [device newBufferWithBytes:&vertices[0] length:sz options:MTLResourceStorageModeManaged];
                bufferVertices = (simd_float3 *)buffer.contents;
                bufferCount = count;
            }
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
    uint32_t bufferCount = 0;
};



class xlMetalVertexColorAccumulator : public xlVertexColorAccumulator {
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

    virtual uint32_t getCount() override {
        return count;
    }
    virtual void Reset() override {
        if (!finalized) {
            count = 0;
            vertices.resize(0);
            colors.resize(0);
        }
    }
    virtual void PreAlloc(unsigned int i) override {
        vertices.reserve(i);
        colors.reserve(i);
    }
    virtual void AddVertex(float x, float y, float z, const xlColor &c) override {
        if (!finalized) {
            if (bufferVertices) {
                if (count < bufferCount) {
                    bufferVertices[count] = (simd_float3){x, y, z};
                } else {
                    vertices.resize(count + 1);
                    memcpy(&vertices[0], bufferVertices, count * sizeof(simd_float3));
                    vertices[count] = (simd_float3){x, y, z};
                    bufferVertices = nullptr;
                    [vbuffer release];
                    vbuffer = nil;
                }
            } else {
                vertices.emplace_back((simd_float3){x, y, z});
            }

            if (bufferColors) {
                if (count < bufferCount) {
                    bufferColors[count] = (simd_uchar4){c.red, c.green, c.blue, c.alpha};
                } else {
                    colors.resize(count + 1);
                    memcpy(&colors[0], bufferColors, count * sizeof(simd_uchar4));
                    colors[count] = (simd_uchar4){c.red, c.green, c.blue, c.alpha};
                    bufferColors = nullptr;
                    [cbuffer release];
                    cbuffer = nil;
                }
            } else {
                colors.emplace_back((simd_uchar4){c.red, c.green, c.blue, c.alpha});
            }
            count++;
        }
    }

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
        if (bufferVertices && (!finalized || mayChangeVertices)) {
            uint32_t s = start * sizeof(simd_float3);
            uint32_t l = len * sizeof(simd_float3);
            [vbuffer didModifyRange:NSMakeRange(s, l)];
        }
        if (bufferColors && (!finalized || mayChangeColors)) {
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
        } else if (sz > 4095 || vbuffer != nil) {
            if (vbuffer == nil) {
                vbuffer = [device newBufferWithBytes:&vertices[0] length:sz options:MTLResourceStorageModeManaged];
                bufferVertices = (simd_float3 *)vbuffer.contents;
                bufferCount = count;
            }
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
        } else if (sz > 4095 || cbuffer != nil) {
            if (cbuffer == nil) {
                cbuffer = [device newBufferWithBytes:&colors[0] length:sz options:MTLResourceStorageModeManaged];
                bufferColors = (simd_uchar4 *)cbuffer.contents;
                bufferCount = count;
            }
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
    uint32_t bufferCount = 0;
};



class xlMetalVertexTextureAccumulator : public xlVertexTextureAccumulator {
public:
    xlMetalVertexTextureAccumulator() {}
    virtual ~xlMetalVertexTextureAccumulator() {
        if (vbuffer) {
            [vbuffer release];
        }
        if (tbuffer) {
            [tbuffer release];
        }
    }

    virtual void Reset() override {
        if (!finalized) {
            count = 0;
            vertices.resize(0);
            tvertices.resize(0);
        }
    }
    virtual void PreAlloc(unsigned int i) override {
        vertices.reserve(i);
        tvertices.reserve(i);
    }
    virtual void AddVertex(float x, float y, float z, float tx, float ty) override {
        if (!finalized) {
            if (bufferVertices) {
                if (count < bufferCount) {
                    bufferVertices[count] = (simd_float3){x, y, z};
                } else {
                    vertices.resize(count + 1);
                    memcpy(&vertices[0], bufferVertices, count * sizeof(simd_float3));
                    vertices[count] = (simd_float3){x, y, z};
                    bufferVertices = nullptr;
                    [vbuffer release];
                    vbuffer = nil;
                }
            } else {
                vertices.emplace_back((simd_float3){x, y, z});
            }

            if (bufferTexture) {
                if (count < bufferCount) {
                    bufferTexture[count] = (simd_float2){tx, ty};
                } else {
                    tvertices.resize(count + 1);
                    memcpy(&tvertices[0], bufferTexture, count * sizeof(simd_float2));
                    tvertices[count] = (simd_float2){tx, ty};
                    bufferTexture = nullptr;
                    [tbuffer release];
                    tbuffer = nil;
                }
            } else {
                tvertices.emplace_back((simd_float2){tx, ty});
            }
            count++;
        }
    }
    virtual uint32_t getCount() override { return count; }


    virtual void Finalize(bool mcv, bool mct) override {
        finalized = true;
        mayChangeVertices = mcv;
        mayChangeTextures = mct;
    }

    virtual void SetVertex(uint32_t vertex, float x, float y, float z, float tx, float ty) override {
        if (vertex < count) {
            if (bufferVertices) {
                bufferVertices[vertex] = (simd_float3){x, y, z};
            } else {
                vertices[vertex] = (simd_float3){x, y, z};
            }
            if (bufferTexture) {
                bufferTexture[vertex] = (simd_float2){tx, ty};
            } else {
                tvertices[vertex] = (simd_float2){tx, ty};
            }
        }
    }

    virtual void FlushRange(uint32_t start, uint32_t len) override {
        if (bufferVertices && (!finalized || mayChangeVertices)) {
            uint32_t s = start * sizeof(simd_float3);
            uint32_t l = len * sizeof(simd_float3);
            [vbuffer didModifyRange:NSMakeRange(s, l)];
        }
        if (bufferTexture && (!finalized || mayChangeTextures)) {
            uint32_t s = start * sizeof(simd_float2);
            uint32_t l = len * sizeof(simd_float2);
            [tbuffer didModifyRange:NSMakeRange(s, l)];
        }
    }

    void SetBufferBytes(id<MTLDevice> device, id<MTLRenderCommandEncoder> encoder, int indexV, int indexT) {
        int sz = count * sizeof(simd_float3);
        if (finalized) {
            if (!vbuffer) {
                vbuffer = [device newBufferWithBytes:&vertices[0] length:sz options:MTLResourceStorageModeManaged];
                if (mayChangeVertices) {
                    bufferVertices = (simd_float3 *)vbuffer.contents;
                }
            }
            [encoder setVertexBuffer:vbuffer offset:0 atIndex:indexV];
        } else if (sz > 4095 || vbuffer != nil) {
            if (vbuffer == nil) {
                vbuffer = [device newBufferWithBytes:&vertices[0] length:sz options:MTLResourceStorageModeManaged];
                bufferVertices = (simd_float3 *)vbuffer.contents;
                bufferCount = count;
            }
            [encoder setVertexBuffer:vbuffer offset:0 atIndex:indexV];
        } else {
            [encoder setVertexBytes:&vertices[0] length:sz atIndex:indexV];
        }

        sz = count * sizeof(simd_float2);
        if (finalized) {
            if (!tbuffer) {
                tbuffer = [device newBufferWithBytes:&tvertices[0] length:sz options:MTLResourceStorageModeManaged];
                if (mayChangeTextures) {
                    bufferTexture = (simd_float2 *)tbuffer.contents;
                }
            }
            [encoder setVertexBuffer:tbuffer offset:0 atIndex:indexT];
        } else if (sz > 4095 || tbuffer != nil) {
            if (tbuffer == nil) {
                tbuffer = [device newBufferWithBytes:&tvertices[0] length:sz options:MTLResourceStorageModeManaged];
                bufferTexture = (simd_float2 *)tbuffer.contents;
                bufferCount = count;
            }
            [encoder setVertexBuffer:tbuffer offset:0 atIndex:indexT];
        } else {
            [encoder setVertexBytes:&tvertices[0] length:sz atIndex:indexT];
        }
    }

    uint32_t count = 0;
    std::vector<simd_float3> vertices;
    std::vector<simd_float2> tvertices;

    bool finalized = false;
    bool mayChangeVertices = false;
    bool mayChangeTextures = false;
    simd_float3 *bufferVertices = nullptr;
    simd_float2 *bufferTexture = nullptr;
    id<MTLBuffer> vbuffer = nil;
    id<MTLBuffer> tbuffer = nil;
    uint32_t bufferCount = 0;
};


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

class xlMetalTexture : public xlTexture {
public:
    xlMetalTexture() : xlTexture(), texture(nil) {
    }
    xlMetalTexture(const wxImage &image) : xlTexture(), texture(nil) {
        LoadImage(image);
    }
    virtual ~xlMetalTexture() {
        if (texture) {
            [texture release];
        }
    }

    void LoadImage(const wxImage &image) {
        @autoreleasepool {
            const uint32_t w = image.GetWidth();
            const uint32_t h = image.GetHeight();

            MTLTextureDescriptor *description = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                                   width:w
                                                                                                  height:h
                                                                                               mipmapped:false];
            id <MTLTexture> srcTexture = [wxMetalCanvas::getMTLDevice() newTextureWithDescriptor:description];
            uint8_t *bytes = (uint8_t *)malloc(w * h * 4);
            getImageBytes(image, bytes);
            int rlen = w * 4;
            MTLRegion region = {0, 0, 0, w, h , 1};
            [srcTexture replaceRegion:region mipmapLevel:0 withBytes:bytes bytesPerRow:rlen];
            texture = srcTexture;
            textureSize = MTLSizeMake(w, h, 1);
            free(bytes);
        }
    }
    void Finalize() override {
        // Create a private texture.
        @autoreleasepool {
            id<MTLTexture> srcTexture = texture;
            int levels = [srcTexture mipmapLevelCount];
            if (levels == 0) {
                levels = 1;
            }
            MTLTextureDescriptor *description = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                                   width:textureSize.width
                                                                                                  height:textureSize.height
                                                                                               mipmapped:(levels > 1 ? true : false)];
            description.storageMode = MTLStorageModePrivate;
            id <MTLTexture> privateTexture = [wxMetalCanvas::getMTLDevice() newTextureWithDescriptor:description];
            // Encode a blit pass to copy data from the source texture to the private texture.
            id<MTLCommandBuffer> bltBuffer = [wxMetalCanvas::getMTLCommandQueue() commandBuffer];
            id <MTLBlitCommandEncoder> blitCommandEncoder = [bltBuffer blitCommandEncoder];
            MTLOrigin textureOrigin = MTLOriginMake(0, 0, 0);
            MTLSize size = MTLSizeMake(textureSize.width, textureSize.height, 1);
            for (int l = 0; l < levels; l++ ) {
                [blitCommandEncoder copyFromTexture:srcTexture
                                        sourceSlice:0
                                        sourceLevel:l
                                       sourceOrigin:textureOrigin
                                         sourceSize:size
                                          toTexture:privateTexture
                                   destinationSlice:0
                                   destinationLevel:l
                                  destinationOrigin:textureOrigin];
                size.width /= 2;
                size.height /= 2;
            }
            [blitCommandEncoder endEncoding];
            [bltBuffer addCompletedHandler:^(id<MTLCommandBuffer> cb) {
                // Private texture is populated, we can release the srcBuffer
                [srcTexture release];
            }];
            [bltBuffer commit];

            texture = privateTexture;
        }
    }


    virtual void UpdatePixel(int x, int y, const xlColor &c, bool copyAlpha) override {
        uint8_t bytes[4];
        bytes[0] = c.red;
        bytes[1] = c.green;
        bytes[2] = c.blue;
        if (copyAlpha) {
            bytes[3] = c.alpha;
        } else {
            bytes[3] = 255;
        }

        MTLRegion region = MTLRegionMake2D(x, y, 1, 1);
        [texture replaceRegion:region mipmapLevel:0 withBytes:bytes bytesPerRow:4];
    }

    id<MTLTexture> texture;
    MTLSize textureSize;
};


xlVertexAccumulator *xlMetalGraphicsContext::createVertexAccumulator() {
    return new xlMetalVertexAccumulator();
}
xlVertexColorAccumulator *xlMetalGraphicsContext::createVertexColorAccumulator() {
    return new xlMetalVertexColorAccumulator();
}
xlVertexTextureAccumulator *xlMetalGraphicsContext::createVertexTextureAccumulator() {
    return new xlMetalVertexTextureAccumulator();
}

xlTexture *xlMetalGraphicsContext::createTextureMipMaps(const std::vector<wxBitmap> &bitmaps) {
    std::vector<wxImage> images;
    for (auto &a : bitmaps) {
        images.push_back(a.ConvertToImage());
    }
    return createTextureMipMaps(images);
}
xlTexture *xlMetalGraphicsContext::createTextureMipMaps(const std::vector<wxImage> &images) {
    xlMetalTexture *txt = new xlMetalTexture();

    @autoreleasepool {
        MTLTextureDescriptor * desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                        width:images[0].GetWidth()
                                                                                        height:images[0].GetHeight()
                                                                                        mipmapped:true];
        txt->texture = [canvas->getMTLDevice() newTextureWithDescriptor:desc];
        txt->textureSize = MTLSizeMake(images[0].GetWidth(), images[0].GetHeight(), 1);

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
        txt->Finalize();
    }
    return txt;
}
xlTexture *xlMetalGraphicsContext::createTexture(const wxImage &image, bool pvt) {
    xlMetalTexture *txt = new xlMetalTexture(image);
    if (pvt) {
        txt->Finalize();
    }
    return txt;
}
xlTexture *xlMetalGraphicsContext::createTexture(const wxImage &image) {
    return createTexture(image, false);
}
xlTexture *xlMetalGraphicsContext::createTextureForFont(const xlFontInfo &font) {
    return createTexture(font.getImage(), true);
}


//drawing methods
void xlMetalGraphicsContext::drawLines(xlVertexAccumulator *vac, const xlColor &c) {
    drawPrimitive(MTLPrimitiveTypeLine, vac, c);
}
void xlMetalGraphicsContext::drawLineStrip(xlVertexAccumulator *vac, const xlColor &c) {
    drawPrimitive(MTLPrimitiveTypeLineStrip, vac, c);
}
void xlMetalGraphicsContext::drawTriangles(xlVertexAccumulator *vac, const xlColor &c) {
    drawPrimitive(MTLPrimitiveTypeTriangle, vac, c);
}
void xlMetalGraphicsContext::drawTriangleStrip(xlVertexAccumulator *vac, const xlColor &c) {
    drawPrimitive(MTLPrimitiveTypeTriangleStrip, vac, c);
}
void xlMetalGraphicsContext::drawPrimitive(MTLPrimitiveType type, xlVertexAccumulator *vac, const xlColor &c) {
    if (vac->getCount() == 0) {
        return;
    }
    setPipelineState("singleColorProgram", "singleColorVertexShader", "singleColorFragmentShader");
    xlMetalVertexAccumulator *mva = (xlMetalVertexAccumulator*)vac;
    mva->SetBufferBytes(canvas->getMTLDevice(), encoder, BufferIndexMeshPositions);

    frameData.fragmentColor.r = c.red;
    frameData.fragmentColor.g = c.green;
    frameData.fragmentColor.b = c.blue;
    frameData.fragmentColor.a = c.alpha;
    frameData.fragmentColor /= 255.0f;

    [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
    frameDataChanged = false;
    [encoder drawPrimitives:type vertexStart:0 vertexCount:vac->getCount()];
}
void xlMetalGraphicsContext::drawLines(xlVertexColorAccumulator *vac) {
    drawPrimitive(MTLPrimitiveTypeLine, vac);
}
void xlMetalGraphicsContext::drawLineStrip(xlVertexColorAccumulator *vac) {
    drawPrimitive(MTLPrimitiveTypeLineStrip, vac);
}
void xlMetalGraphicsContext::drawTriangles(xlVertexColorAccumulator *vac) {
    drawPrimitive(MTLPrimitiveTypeTriangle, vac);
}
void xlMetalGraphicsContext::drawTriangleStrip(xlVertexColorAccumulator *vac) {
    drawPrimitive(MTLPrimitiveTypeTriangleStrip, vac);
}
void xlMetalGraphicsContext::drawPrimitive(MTLPrimitiveType type, xlVertexColorAccumulator *vac) {
    if (vac->getCount() == 0) {
        return;
    }
    setPipelineState("multiColorProgram", "multiColorVertexShader", "multiColorFragmentShader");
    xlMetalVertexColorAccumulator *mva = (xlMetalVertexColorAccumulator*)vac;
    mva->SetBufferBytes(canvas->getMTLDevice(), encoder, BufferIndexMeshPositions, BufferIndexMeshColors);

    if (frameDataChanged) {
        [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
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

    std::string name = linearScale ? "textureProgramNearest" : "textureProgram";
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
        frameDataChanged = false;
    }

    xlMetalTexture *txt = (xlMetalTexture*)texture;
    [encoder setFragmentTexture:txt->texture atIndex:TextureIndexBase];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:va.getCount()];
}

void xlMetalGraphicsContext::drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture) {
    if (vac->getCount() == 0) {
        return;
    }
    setPipelineState("textureProgram", "textureVertexShader", "textureFragmentShader");
    xlMetalVertexTextureAccumulator *mva = (xlMetalVertexTextureAccumulator*)vac;
    mva->SetBufferBytes(canvas->getMTLDevice(), encoder, BufferIndexMeshPositions, BufferIndexTexturePositions);

    if (frameDataChanged) {
        [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
        frameDataChanged = false;
    }
    xlMetalTexture *txt = (xlMetalTexture*)texture;
    [encoder setFragmentTexture:txt->texture atIndex:TextureIndexBase];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vac->getCount()];
}
void xlMetalGraphicsContext::drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, const xlColor &c) {
    if (vac->getCount() == 0) {
        return;
    }
    xlMetalVertexTextureAccumulator *mva = (xlMetalVertexTextureAccumulator*)vac;
    setPipelineState("textureColorProgram", "textureVertexShader", "textureColorFragmentShader");
    mva->SetBufferBytes(canvas->getMTLDevice(), encoder, BufferIndexMeshPositions, BufferIndexTexturePositions);

    frameData.fragmentColor.r = c.red;
    frameData.fragmentColor.g = c.green;
    frameData.fragmentColor.b = c.blue;
    frameData.fragmentColor.a = c.alpha;
    frameData.fragmentColor /= 255.0f;
    [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
    xlMetalTexture *txt = (xlMetalTexture*)texture;
    [encoder setFragmentTexture:txt->texture atIndex:TextureIndexBase];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vac->getCount()];
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

        if (canvas->drawingUsingLogicalSize()) {
            MTLViewport vp = {  canvas->translateToBacking(topleft_x), canvas->translateToBacking(topleft_y),
                canvas->translateToBacking(w), canvas->translateToBacking(h), 1.0, 1.0 };
            [encoder setViewport:vp];
        } else {
            MTLViewport vp = {  (double)topleft_x, (double)topleft_y, w, h, 1.0, 1.0 };
            [encoder setViewport:vp];
        }
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
    if (lastPipeline != name || blending != lastPipelineBlend) {
        [encoder setRenderPipelineState:canvas->getPipelineState(name, vShader, fShader, blending)];
        lastPipeline = name;
        lastPipelineBlend = blending;
        return true;
    }
    return false;
}

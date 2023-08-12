#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <ModelIO/ModelIO.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "xlMetalGraphicsContext.h"

#include "Shaders/SIMDMathUtilities.h"
#include "../xlMesh.h"

xlMetalGraphicsContext::xlMetalGraphicsContext(xlMetalCanvas *c, id<MTLTexture> t, bool enqueImmediate) : xlGraphicsContext(c),  canvas(c), target(t) {
    id<MTLTexture> localTarget = t;
    if (target == nil) {
        id<CAMetalDrawable> d2 = [c->getMTKView() currentDrawable];
        CAMetalLayer *layer = [d2 layer];
        drawable = [layer nextDrawable];
        if (drawable != nil) {
            [drawable retain];
            localTarget = [drawable texture];
        }
    } else {
        drawable = nil;
    }

    if (localTarget != nil) {
        buffer = [c->getMTLCommandQueue() commandBuffer];
        
        std::string n2 = c->getName() + " CommandBuffer";
        NSString *n = [NSString stringWithUTF8String:n2.c_str()];
        [buffer setLabel:n];
        if (enqueImmediate) {
            [buffer enqueue];
        }

        MTLRenderPassDescriptor *renderPass = [[MTLRenderPassDescriptor alloc] init];

        renderPass.colorAttachments[0].texture = localTarget;
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

        int width = [localTarget width];
        int height = [localTarget height];
        if (c->usesMSAA()) {
            renderPass.colorAttachments[0].resolveTexture = renderPass.colorAttachments[0].texture;
            renderPass.colorAttachments[0].storeAction = MTLStoreActionMultisampleResolve;
            renderPass.colorAttachments[0].texture = c->getMSAATexture(width, height);
        }
        if (c->RequiresDepthBuffer()) {
            renderPass.depthAttachment.texture = c->getDepthTexture(width, height);
            renderPass.depthAttachment.clearDepth = 1.0;
            renderPass.depthAttachment.loadAction = MTLLoadActionClear;
            renderPass.depthAttachment.storeAction = MTLStoreActionDontCare;
        }
        
        encoder = [buffer renderCommandEncoderWithDescriptor:renderPass];
        n2 = c->getName() + " Encoder";
        n = [NSString stringWithUTF8String:n2.c_str()];
        [encoder setLabel:n];
        
        if (c->RequiresDepthBuffer()) {
            [encoder setDepthStencilState:c->getDepthStencilStateLE()];
        }
        
        [renderPass release];

        frameData.MVP = matrix4x4_identity();
        frameData.modelMatrix = matrix4x4_identity();
        frameData.viewMatrix = matrix4x4_identity();
        frameData.perspectiveMatrix = matrix4x4_identity();
        frameData.fragmentColor = {0.0f, 0.0f, 0.0f, 1.0f};
        frameData.pointSize = 0;
        frameData.pointSmoothMin = 0.25;
        frameData.pointSmoothMax = 0.5;
        frameData.brightness = 1.0;
        frameData.renderType = RenderTypeNormal;
        frameData.useViewMatrix = false;
        frameDataChanged = true;
    } else {
        buffer = nil;
        encoder = nil;
    }
}



xlMetalGraphicsContext::~xlMetalGraphicsContext() {
    if (drawable != nil) {
        [drawable release];
    }
}

void xlMetalGraphicsContext::Commit(bool displayOnScreen, id<MTLBuffer> captureBuffer) {
    if (encoder != nil) {
        @autoreleasepool {
            [encoder endEncoding];
            if (!xlMetalCanvas::isInSyncPoint() && displayOnScreen) {
                [buffer presentDrawable:drawable];
            }
            if (!displayOnScreen) {
                int w = [target width];
                int h = [target height];
                int bytesPerRow = w * 4;
                int bufferSize = bytesPerRow * h;
                
                MTLSize size = MTLSizeMake(w, h, 1);
                id <MTLBlitCommandEncoder> blitCommandEncoder = [buffer blitCommandEncoder];
                [blitCommandEncoder setLabel:@"CopyToCaptureBuffer"];
                [blitCommandEncoder copyFromTexture:target
                                        sourceSlice:0
                                        sourceLevel:0
                                       sourceOrigin:{0,0,0}
                                         sourceSize:size
                                           toBuffer:captureBuffer
                                  destinationOffset:0
                             destinationBytesPerRow:bytesPerRow
                           destinationBytesPerImage:bufferSize];
                [blitCommandEncoder endEncoding];
            }
            
            [buffer commit];

            if (!displayOnScreen) {
                [buffer waitUntilCompleted];
            } else if (!xlMetalCanvas::isInSyncPoint()) {
                [drawable release];
                drawable = nil;
            } else {
                [buffer retain];
                wxMetalCanvas::addToSyncPoint(buffer, drawable);
                drawable = nil;
                buffer = nil;
            }
        }
    }
}


bool xlMetalGraphicsContext::isValid() {
    return drawable != nil || target != nil;
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
            uint32_t s = start * sizeof(simd_float3);
            uint32_t l = len * sizeof(simd_float3);
            [buffer didModifyRange:NSMakeRange(s, l)];
        }
    }


    void SetBufferBytes(id<MTLDevice> device, id<MTLRenderCommandEncoder> encoder, int index) {
        int sz = count * sizeof(simd_float3);
        if (finalized) {
            if (!buffer) {
                buffer = [device newBufferWithBytes:&vertices[0] length:(sizeof(simd_float3) * count) options:MTLResourceStorageModeManaged];
                std::string n2 = name + " Vertices";
                NSString *n = [NSString stringWithUTF8String:n2.c_str()];
                [buffer setLabel:n];
                if (mayChange) {
                    bufferVertices = (simd_float3 *)buffer.contents;
                    bufferCount = count;
                }
            }
            [encoder setVertexBuffer:buffer offset:0 atIndex:index];
        } else if (sz > 4095 || buffer != nil) {
            if (!buffer) {
                buffer = [device newBufferWithBytes:&vertices[0] length:sz options:MTLResourceStorageModeManaged];
                std::string n2 = name + " Vertices";
                NSString *n = [NSString stringWithUTF8String:n2.c_str()];
                [buffer setLabel:n];
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
                std::string n2 = name + " Vertices";
                NSString *n = [NSString stringWithUTF8String:n2.c_str()];
                [vbuffer setLabel:n];
                if (mayChangeVertices) {
                    bufferVertices = (simd_float3 *)vbuffer.contents;
                }
            }
            [encoder setVertexBuffer:vbuffer offset:0 atIndex:indexV];
        } else if (sz > 4095 || vbuffer != nil) {
            if (vbuffer == nil) {
                vbuffer = [device newBufferWithBytes:&vertices[0] length:sz options:MTLResourceStorageModeManaged];
                std::string n2 = name + " Vertices";
                NSString *n = [NSString stringWithUTF8String:n2.c_str()];
                [vbuffer setLabel:n];
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
                std::string n2 = name + " Colors";
                NSString *n = [NSString stringWithUTF8String:n2.c_str()];
                [cbuffer setLabel:n];
                if (mayChangeColors) {
                    bufferColors = (simd_uchar4 *)cbuffer.contents;
                }
            }
            [encoder setVertexBuffer:cbuffer offset:0 atIndex:indexC];
        } else if (sz > 4095 || cbuffer != nil) {
            if (cbuffer == nil) {
                cbuffer = [device newBufferWithBytes:&colors[0] length:sz options:MTLResourceStorageModeManaged];
                std::string n2 = name + " Colors";
                NSString *n = [NSString stringWithUTF8String:n2.c_str()];
                [cbuffer setLabel:n];
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

class xlMetalVertexIndexedColorAccumulator : public xlVertexIndexedColorAccumulator {
public:
    struct IndexedColorVertex {
        float x;
        float y;
        float z;
        uint32_t colorIdx;
    };
    
    
    xlMetalVertexIndexedColorAccumulator() {}
    virtual ~xlMetalVertexIndexedColorAccumulator() {
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
    }
    virtual void AddVertex(float x, float y, float z, uint32_t cIdx) override {
        if (!finalized) {
            if (bufferVertices) {
                if (count < vbufferCount) {
                    bufferVertices[count].x = x;
                    bufferVertices[count].y = y;
                    bufferVertices[count].z = z;
                    bufferVertices[count].colorIdx = cIdx;
                } else {
                    vertices.resize(count + 1);
                    memcpy(&vertices[0], bufferVertices, count * sizeof(IndexedColorVertex));
                    vertices[count].x = x;
                    vertices[count].y = y;
                    vertices[count].z = z;
                    vertices[count].colorIdx = cIdx;
                    bufferVertices = nullptr;
                    [vbuffer release];
                    vbuffer = nil;
                }
            } else {
                IndexedColorVertex v;
                v.x = x;
                v.y = y;
                v.z = z;
                v.colorIdx = cIdx;
                vertices.push_back(v);
            }

            count++;
        }
    }
    virtual void SetColorCount(int c) override {
        if (!finalized) {
            colors.resize(c);
        }
    }
    virtual uint32_t GetColorCount() override {
        return colors.size();
    }
    virtual void SetColor(uint32_t idx, const xlColor &c) override {
        if (bufferColors) {
            if (idx < cbufferCount) {
                bufferColors[idx] = (simd_uchar4){c.red, c.green, c.blue, c.alpha};
            } else {
                colors.resize(idx + 1);
                memcpy(&colors[0], bufferColors, idx * sizeof(simd_uchar4));
                colors[idx] = (simd_uchar4){c.red, c.green, c.blue, c.alpha};
                bufferColors = nullptr;
                [cbuffer release];
                cbuffer = nil;
            }
        } else {
            colors[idx] = ((simd_uchar4){c.red, c.green, c.blue, c.alpha});
        }
    }

    virtual void Finalize(bool mcv, bool mcc) override {
        finalized = true;
        mayChangeVertices = mcv;
        mayChangeColors = mcc;
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z, uint32_t cIdx) override {
        if (vertex < count) {
            if (bufferVertices) {
                bufferVertices[vertex].x = x;
                bufferVertices[vertex].y = y;
                bufferVertices[vertex].z = z;
                bufferVertices[vertex].colorIdx = cIdx;
            } else {
                vertices[vertex].x = x;
                vertices[vertex].y = y;
                vertices[vertex].z = z;
                vertices[vertex].colorIdx = cIdx;
            }
        }
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z) override {
        if (vertex < count) {
            if (bufferVertices) {
                bufferVertices[vertex].x = x;
                bufferVertices[vertex].y = y;
                bufferVertices[vertex].z = z;
            } else {
                vertices[vertex].x = x;
                vertices[vertex].y = y;
                vertices[vertex].z = z;
            }
        }
    }
    virtual void SetVertex(uint32_t vertex, uint32_t cIdx) override {
        if (vertex < count) {
            if (bufferVertices) {
                bufferVertices[vertex].colorIdx = cIdx;
            } else {
                vertices[vertex].colorIdx = cIdx;
            }
        }
    }
    virtual void FlushRange(uint32_t start, uint32_t len) override {
        if (bufferVertices && (!finalized || mayChangeVertices)) {
            uint32_t s = start * sizeof(IndexedColorVertex);
            uint32_t l = len * sizeof(IndexedColorVertex);
            [vbuffer didModifyRange:NSMakeRange(s, l)];
        }
    }
    virtual void FlushColors(uint32_t start, uint32_t len) override {
        /*
        if (bufferColors && (!finalized || mayChangeColors)) {
            uint32_t s = start * sizeof(simd_uchar4);
            uint32_t l = len * sizeof(simd_uchar4);
            [cbuffer didModifyRange:NSMakeRange(s, l)];
        }
         */
    }

    void SetBufferBytes(id<MTLDevice> device, id<MTLRenderCommandEncoder> encoder, int indexV, int indexC) {
        int sz = count * sizeof(IndexedColorVertex);
        if (finalized) {
            if (!vbuffer) {
                vbuffer = [device newBufferWithBytes:&vertices[0] length:sz options:MTLResourceStorageModeManaged];
                std::string n2 = name + " Vertices";
                NSString *n = [NSString stringWithUTF8String:n2.c_str()];
                [vbuffer setLabel:n];
                if (mayChangeVertices) {
                    bufferVertices = (IndexedColorVertex *)vbuffer.contents;
                    vbufferCount = count;
                }
            }
            [encoder setVertexBuffer:vbuffer offset:0 atIndex:indexV];
        } else if (sz > 4095 || vbuffer != nil) {
            if (vbuffer == nil) {
                vbuffer = [device newBufferWithBytes:&vertices[0] length:sz options:MTLResourceStorageModeManaged];
                std::string n2 = name + " Vertices";
                NSString *n = [NSString stringWithUTF8String:n2.c_str()];
                [vbuffer setLabel:n];
                bufferVertices = (IndexedColorVertex *)vbuffer.contents;
                vbufferCount = count;
            }
            [encoder setVertexBuffer:vbuffer offset:0 atIndex:indexV];
        } else {
            [encoder setVertexBytes:&vertices[0] length:sz atIndex:indexV];
        }

        sz = colors.size() * sizeof(simd_uchar4);
        if (finalized) {
            if (!cbuffer) {
                //cbuffer = [device newBufferWithBytes:&colors[0] length:sz options:MTLResourceStorageModeManaged];
                cbuffer = [device newBufferWithBytes:&colors[0] length:sz options:MTLResourceStorageModeShared];
                std::string n2 = name + " Colors";
                NSString *n = [NSString stringWithUTF8String:n2.c_str()];
                [cbuffer setLabel:n];
                if (mayChangeColors) {
                    bufferColors = (simd_uchar4 *)cbuffer.contents;
                    cbufferCount = colors.size();
                }
            }
            [encoder setVertexBuffer:cbuffer offset:0 atIndex:indexC];
        } else if (sz > 4095 || cbuffer != nil) {
            if (cbuffer == nil) {
                cbuffer = [device newBufferWithBytes:&colors[0] length:sz options:MTLResourceStorageModeShared];
                std::string n2 = name + " Colors";
                NSString *n = [NSString stringWithUTF8String:n2.c_str()];
                [cbuffer setLabel:n];
                bufferColors = (simd_uchar4 *)cbuffer.contents;
                cbufferCount = colors.size();
            }
            [encoder setVertexBuffer:cbuffer offset:0 atIndex:indexC];
        } else {
            [encoder setVertexBytes:&colors[0] length:sz atIndex:indexC];
        }
    }
    
    
    uint32_t count = 0;
    std::vector<IndexedColorVertex> vertices;
    std::vector<simd_uchar4> colors;
    
    bool finalized = false;
    bool mayChangeColors = false;
    bool mayChangeVertices = false;
    IndexedColorVertex *bufferVertices = nullptr;
    simd_uchar4 *bufferColors = nullptr;
    id<MTLBuffer> vbuffer = nil;
    id<MTLBuffer> cbuffer = nil;
    uint32_t vbufferCount = 0;
    uint32_t cbufferCount = 0;
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
                std::string n2 = name + " Vertices";
                NSString *n = [NSString stringWithUTF8String:n2.c_str()];
                [vbuffer setLabel:n];
                if (mayChangeVertices) {
                    bufferVertices = (simd_float3 *)vbuffer.contents;
                }
            }
            [encoder setVertexBuffer:vbuffer offset:0 atIndex:indexV];
        } else if (sz > 4095 || vbuffer != nil) {
            if (vbuffer == nil) {
                vbuffer = [device newBufferWithBytes:&vertices[0] length:sz options:MTLResourceStorageModeManaged];
                std::string n2 = name + " Vertices";
                NSString *n = [NSString stringWithUTF8String:n2.c_str()];
                [vbuffer setLabel:n];
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
                std::string n2 = name + " Texture Vertices";
                NSString *n = [NSString stringWithUTF8String:n2.c_str()];
                [tbuffer setLabel:n];
                if (mayChangeTextures) {
                    bufferTexture = (simd_float2 *)tbuffer.contents;
                }
            }
            [encoder setVertexBuffer:tbuffer offset:0 atIndex:indexT];
        } else if (sz > 4095 || tbuffer != nil) {
            if (tbuffer == nil) {
                tbuffer = [device newBufferWithBytes:&tvertices[0] length:sz options:MTLResourceStorageModeManaged];
                std::string n2 = name + " Texture Vertices";
                NSString *n = [NSString stringWithUTF8String:n2.c_str()];
                [tbuffer setLabel:n];
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

extern void VideoToolboxCopyToTexture(CIImage *image, id<MTLTexture> texture, id<MTLCommandBuffer> cmdBuf);

class xlMetalTexture : public xlTexture {
public:
    xlMetalTexture() : xlTexture(), texture(nil) {
    }
    xlMetalTexture(const wxImage &image) : xlTexture(), texture(nil) {
        LoadImage(image);
    }
    xlMetalTexture(int w, int h, bool bgr, bool alpha) {
        @autoreleasepool {
            MTLPixelFormat pf = bgr? MTLPixelFormatBGRA8Unorm : MTLPixelFormatRGBA8Unorm;
            this->alpha = alpha;
            
            MTLTextureDescriptor *description = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pf
                                                                                                   width:w
                                                                                                  height:h
                                                                                               mipmapped:false];
            description.usage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
            texture = [wxMetalCanvas::getMTLDevice() newTextureWithDescriptor:description];

            textureSize = MTLSizeMake(w, h, 1);
        }
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
            description.usage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
            id <MTLTexture> srcTexture = [wxMetalCanvas::getMTLDevice() newTextureWithDescriptor:description];
            uint8_t *bytes = (uint8_t *)malloc(w * h * 4);
            getImageBytes(image, bytes);
            int rlen = w * 4;
            MTLRegion region = {0, 0, 0, w, h , 1};
            [srcTexture replaceRegion:region mipmapLevel:0 withBytes:bytes bytesPerRow:rlen];
            texture = srcTexture;
            
            std::string n2 = name + " Texture";
            NSString *n = [NSString stringWithUTF8String:n2.c_str()];
            [texture setLabel:n];

            textureSize = MTLSizeMake(w, h, 1);
            free(bytes);
        }
    }
    virtual void UpdateData(uint8_t *data, bool bgr, bool alpha) override {
        MTLPixelFormat pf = [texture pixelFormat];
        bool srcBGR = pf == MTLPixelFormatBGRA8Unorm;
        uint8_t *setData = data;
        int rlen = textureSize.width * 4;
        if (!alpha || srcBGR != bgr) {
            setData = new uint8_t[rlen * textureSize.height];
            uint8_t *src = data;
            uint8_t *dst = setData;
            for (int x = 0; x < (rlen * textureSize.height); x += 4) {
                dst[1] = src[1];
                if (bgr != srcBGR) {
                    dst[0] = src[2];
                    dst[2] = src[0];
                } else {
                    dst[0] = src[0];
                    dst[2] = src[2];
                }
                if (alpha) {
                    dst[3] = src[3];
                    src += 4;
                } else {
                    dst[3] = 255;
                    src += 3;
                }
                dst += 4;
            }
        }
        MTLRegion region = {0, 0, 0, textureSize.width, textureSize.height , 1};
        [texture replaceRegion:region mipmapLevel:0 withBytes:setData bytesPerRow:rlen];
        if (setData != data) {
            delete [] setData;
        }
    }
    virtual void UpdateData(xlGraphicsContext *ctx, void *data, const std::string &type) override {
        if (type == "vt") {
            @autoreleasepool {
                //xlMetalGraphicsContext *mctx = (xlMetalGraphicsContext*)ctx;
                CVPixelBufferRef pixbuf = (CVPixelBufferRef)data;
                CIImage *image = [CIImage imageWithCVImageBuffer:pixbuf];
                image = [image imageByApplyingCGOrientation: kCGImagePropertyOrientationDownMirrored];
                
                id<MTLCommandBuffer> buffer = [wxMetalCanvas::getMTLCommandQueue() commandBuffer];
                VideoToolboxCopyToTexture(image, texture, buffer);
                [buffer commit];
            }
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
            std::string n2 = name + " Texture";
            NSString *n = [NSString stringWithUTF8String:n2.c_str()];
            [privateTexture setLabel:n];

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
            [blitCommandEncoder optimizeContentsForGPUAccess:privateTexture];
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
    bool alpha = true;
};

xlVertexAccumulator *xlMetalGraphicsContext::createVertexAccumulator() {
    return new xlMetalVertexAccumulator();
}
xlVertexColorAccumulator *xlMetalGraphicsContext::createVertexColorAccumulator() {
    return new xlMetalVertexColorAccumulator();
}
xlVertexIndexedColorAccumulator *xlMetalGraphicsContext::createVertexIndexedColorAccumulator() {
    return new xlMetalVertexIndexedColorAccumulator();
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
    }
    return txt;
}
xlTexture *xlMetalGraphicsContext::createTexture(const wxImage &image, bool pvt, const std::string &name) {
    xlMetalTexture *txt = new xlMetalTexture(image);
    if (name != "") {
        txt->SetName(name);
    }
    if (pvt) {
        txt->Finalize();
    }
    return txt;
}
xlTexture *xlMetalGraphicsContext::createTexture(const wxImage &image) {
    return createTexture(image, false, "");
}
xlTexture *xlMetalGraphicsContext::createTexture(int w, int h, bool bgr, bool alpha) {
    return new xlMetalTexture(w, h, bgr, alpha);
}

xlTexture *xlMetalGraphicsContext::createTextureForFont(const xlFontInfo &font) {
    std::string n = "Font-" + std::to_string(font.getSize());
    return createTexture(font.getImage(), true, n);
}
xlGraphicsProgram *xlMetalGraphicsContext::createGraphicsProgram() {
    return new xlGraphicsProgram(createVertexColorAccumulator());
}


class xlMetalMesh : public xlMesh {
public:
    struct MeshVertexInput {
        float positionX;
        float positionY;
        float positionZ;
        float normalX;
        float normalY;
        float normalZ;
        float texcoordX;
        float texcoordY;
    };

    class xlMetalSubMesh {
    public:
        xlMetalSubMesh() {
            
        }
        ~xlMetalSubMesh() {
        }
        
        std::string name;
        int startIndex;
        int count;
        MTLPrimitiveType type;
        int material = 0;
    };
    
    struct CompareSimdInt3 {
        bool operator()(const simd::int3& lhs, const simd::int3& rhs) const {
            if (lhs.x != rhs.x) {
                return lhs.x < rhs.x;
            }
            if (lhs.y != rhs.y) {
                return lhs.y < rhs.y;
            }
            if (lhs.z != rhs.z) {
                return lhs.z < rhs.z;
            }
            return false;
        }
    };
    
    uint32_t getOrAddIndex(std::map<simd::int3, uint32_t, CompareSimdInt3> &indexMap, std::vector<MeshVertexInput> &input, const tinyobj::index_t &idx) {
        simd::int3 key;
        key.x = idx.vertex_index;
        key.y = idx.normal_index;
        key.z = idx.texcoord_index;
        int ret = indexMap[key];
        
        if (ret == 0) {
            ret = input.size();
            MeshVertexInput mvi;
            mvi.positionX = objects.GetAttrib().vertices[idx.vertex_index * 3];
            mvi.positionY = objects.GetAttrib().vertices[idx.vertex_index * 3 + 1];
            mvi.positionZ = objects.GetAttrib().vertices[idx.vertex_index * 3 + 2];
            if (idx.normal_index == -1) {
                mvi.normalX = 0;
                mvi.normalY = 0;
                mvi.normalZ = 0;
            } else {
                mvi.normalX = objects.GetAttrib().normals[idx.normal_index * 3];
                mvi.normalY = objects.GetAttrib().normals[idx.normal_index * 3 + 1];
                mvi.normalZ = objects.GetAttrib().normals[idx.normal_index * 3 + 2];
            }
            if (idx.texcoord_index == -1) {
                mvi.texcoordX = 0;
                mvi.texcoordY = 0;
            } else {
                mvi.texcoordX = objects.GetAttrib().texcoords[idx.texcoord_index * 2];
                mvi.texcoordY = objects.GetAttrib().texcoords[idx.texcoord_index * 2 + 1];
            }
            indexMap[key] = ret;
            input.emplace_back(mvi);
        }
        return ret;
    }
    xlMetalMesh(xlGraphicsContext *ctx, const std::string &file) : xlMesh(ctx, file), vbuffer(nil), ibuffer(nil) {
    }
    virtual ~xlMetalMesh() {
        for (auto a: subMeshes) {
            delete a;
        }
        if (vbuffer != nil) {
            [vbuffer release];
        }
        if (ibuffer != nil) {
            [ibuffer release];
        }
    }
    void LoadBuffers() {
        std::map<simd::int3, uint32_t, CompareSimdInt3> indexMap;
        
        std::vector<MeshVertexInput> input;
        input.reserve(objects.GetAttrib().vertices.size());
        input.resize(1); // 0 position is ignored, indexMap[key] == 0 means not found yet
        
        std::vector<uint32_t> indexes;
        indexes.reserve(objects.GetAttrib().vertices.size());
        
        std::vector<uint32_t> lines;
        lines.reserve(objects.GetAttrib().vertices.size() * 2);

        std::vector<uint32_t> wireFrame;
        wireFrame.reserve(objects.GetAttrib().vertices.size() * 2);

        xlMetalSubMesh *lastMesh = nullptr;
        for (auto &s : objects.GetShapes()) {
            if (!s.mesh.indices.empty()) {
                xlMetalSubMesh *sm;
                if (lastMesh && (s.mesh.material_ids[0] == lastMesh->material)) {
                    sm = lastMesh;
                } else {
                    sm = new xlMetalSubMesh();
                    sm->name = s.name;
                    sm->type = MTLPrimitiveTypeTriangle;
                    sm->startIndex = indexes.size();
                    sm->material = s.mesh.material_ids[0];
                    subMeshes.push_back(sm);
                    lastMesh = sm;
                }
                for (int idx = 0; idx < s.mesh.material_ids.size(); idx++) {
                    if (sm->material != s.mesh.material_ids[idx]) {
                        sm->count = indexes.size() - sm->startIndex;

                        sm = new xlMetalSubMesh();
                        sm->name = s.name;
                        sm->type = MTLPrimitiveTypeTriangle;
                        sm->startIndex = indexes.size();
                        sm->material = s.mesh.material_ids[idx];
                        lastMesh = sm;
                        subMeshes.push_back(sm);
                    }
                    uint32_t vidx1 = getOrAddIndex(indexMap, input, s.mesh.indices[idx*3]);
                    uint32_t vidx2 = getOrAddIndex(indexMap, input, s.mesh.indices[idx*3 + 1]);
                    uint32_t vidx3 = getOrAddIndex(indexMap, input, s.mesh.indices[idx*3 + 2]);
                    indexes.push_back(vidx1);
                    indexes.push_back(vidx2);
                    indexes.push_back(vidx3);

                    wireFrame.push_back(vidx1);
                    wireFrame.push_back(vidx2);
                    wireFrame.push_back(vidx2);
                    wireFrame.push_back(vidx3);
                    wireFrame.push_back(vidx3);
                    wireFrame.push_back(vidx1);
                }
                sm->count = indexes.size() - sm->startIndex;
            }
            if (!s.lines.indices.empty()) {
                for (auto &idx : s.lines.indices) {
                    uint32_t vindex = getOrAddIndex(indexMap, input, idx);
                    lines.push_back(vindex);
                }
            }
            if (!s.points.indices.empty()) {
                xlMetalSubMesh *sm = new xlMetalSubMesh();
                sm->name = s.name;
                sm->type = MTLPrimitiveTypePoint;
                sm->startIndex = indexes.size();
                for (auto &idx : s.points.indices) {
                    indexes.push_back(getOrAddIndex(indexMap, input, idx));
                }
                sm->count = indexes.size() - sm->startIndex;
                subMeshes.push_back(sm);
            }
        }
        int sz = input.size() * sizeof(MeshVertexInput);
        vbuffer = [wxMetalCanvas::getMTLDevice() newBufferWithBytes:&input[0] length:sz options:MTLResourceStorageModeManaged];
        
        if (!lines.empty()) {
            linesStart = indexes.size();
            linesCount = lines.size();
            indexes.insert(indexes.end(), lines.begin(), lines.end());
        }
        if (!wireFrame.empty()) {
            wireframeStart = indexes.size();
            wireframeCount = wireFrame.size();
            indexes.insert(indexes.end(), wireFrame.begin(), wireFrame.end());
        }

        std::string n2 = name + " Vertices";
        NSString *n = [NSString stringWithUTF8String:n2.c_str()];
        [vbuffer setLabel:n];
        sz = indexes.size() * sizeof(uint32_t);
        if (sz == 0) {
            ibuffer = [wxMetalCanvas::getMTLDevice() newBufferWithLength:12 options:MTLResourceStorageModeManaged];
        } else {
            ibuffer = [wxMetalCanvas::getMTLDevice() newBufferWithBytes:&indexes[0] length:sz options:MTLResourceStorageModeManaged];
        }
        n2 = name + " Indexes";
        n = [NSString stringWithUTF8String:n2.c_str()];
        [ibuffer setLabel:n];
    }

    
    id<MTLBuffer> vbuffer = nil;
    id<MTLBuffer> ibuffer = nil;
    std::vector<xlMetalSubMesh*> subMeshes;
    
    uint32_t wireframeStart = 0;
    uint32_t wireframeCount = 0;

    uint32_t linesStart = 0;
    uint32_t linesCount = 0;
};

xlMesh *xlMetalGraphicsContext::loadMeshFromObjFile(const std::string &file) {
    return new xlMetalMesh(this, file);
}

xlGraphicsContext* xlMetalGraphicsContext::drawMeshSolids(xlMesh *mesh, int brightness, bool useViewMatrix) {
    xlMetalMesh *xlm = (xlMetalMesh*)mesh;
    if (xlm->vbuffer == nil) {
        xlm->LoadBuffers();
    }
    setPipelineState("meshSolidProgram", "meshVertexShader", "meshSolidFragmentShader");
    id<MTLRenderPipelineState> texturePS = canvas->getPipelineState("meshTextureProgram", "meshVertexShader", "meshTextureFragmentShader", blending);
    id<MTLRenderPipelineState> solidPS = canvas->getPipelineState("meshSolidProgram", "meshVertexShader", "meshSolidFragmentShader", blending);


    std::string n2 = xlm->GetName() + " Solids";
    NSString *n = [NSString stringWithUTF8String:n2.c_str()];
    [encoder pushDebugGroup:n];

    [encoder setVertexBuffer:xlm->vbuffer offset:0 atIndex:BufferIndexMeshPositions];
    
    frameData.renderType = RenderTypeNormal;
    frameData.brightness = brightness;
    frameData.brightness /= 100.0;
    frameData.useViewMatrix = useViewMatrix;
    frameDataChanged = false;
    [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
    
    [encoder setDepthStencilState:canvas->getDepthStencilStateL()];

    
    xlTexture *lastTexture = nullptr;
    bool lastIsSolid = true;
    for (auto sm : xlm->subMeshes) {
        int mid = sm->material;
        if (mid < 0 || xlm->GetMaterial(mid).color.alpha == 255) {
            if (mid < 0 || !xlm->GetMaterial(mid).texture || xlm->GetMaterial(mid).forceColor) {
                if (!lastIsSolid) {
                    lastIsSolid = true;
                    [encoder setRenderPipelineState:solidPS];
                }
                simd::float4 color = mid < 0 ? xlWHITE.asFloat4() : xlm->GetMaterial(mid).color.asFloat4();
                if (sm->type == MTLPrimitiveTypeLine) {
                    color = {0.0, 0.0, 0.0, 1.0};
                }
                if (!simd_equal(color, frameData.fragmentColor)) {
                    frameData.fragmentColor = color;
                    [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
                }
            } else {
                if (lastIsSolid) {
                    lastIsSolid = false;
                    [encoder setRenderPipelineState:texturePS];
                }
                xlMetalTexture *t = (xlMetalTexture*)xlm->GetMaterial(mid).texture;
                if (t != lastTexture) {
                    [encoder setFragmentTexture:t->texture atIndex:BufferIndexTexturePositions];
                    lastTexture = t;
                }
            }
            // Draw the submesh.
            [encoder drawIndexedPrimitives:sm->type
                                indexCount:sm->count
                                 indexType:MTLIndexTypeUInt32
                               indexBuffer:xlm->ibuffer
                         indexBufferOffset:(sm->startIndex*4)];
        }
    }
    if (!lastIsSolid) {
        //lastIsSolid = true;
        [encoder setRenderPipelineState:solidPS];
    }
    if (xlm->linesCount) {
        simd::float4 color = {0.0, 0.0, 0.0, 1.0};
        if (!simd_equal(color, frameData.fragmentColor)) {
            frameData.fragmentColor = color;
            [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
        }
        [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
        // Draw the Lines.
        [encoder drawIndexedPrimitives:MTLPrimitiveTypeLine
                            indexCount:xlm->linesCount
                             indexType:MTLIndexTypeUInt32
                           indexBuffer:xlm->ibuffer
                     indexBufferOffset:(xlm->linesStart*4)];
    }
    [encoder setDepthStencilState:canvas->getDepthStencilStateLE()];

    [encoder popDebugGroup];
    return this;
}
xlGraphicsContext* xlMetalGraphicsContext::drawMeshTransparents(xlMesh *mesh, int brightness) {
    xlMetalMesh *xlm = (xlMetalMesh*)mesh;
    if (xlm->vbuffer == nil) {
        xlm->LoadBuffers();
    }

    setPipelineState("meshSolidProgram", "meshVertexShader", "meshSolidFragmentShader");
    id<MTLRenderPipelineState> texturePS = canvas->getPipelineState("meshTextureProgram", "meshVertexShader", "meshTextureFragmentShader", blending);
    id<MTLRenderPipelineState> solidPS = canvas->getPipelineState("meshSolidProgram", "meshVertexShader", "meshSolidFragmentShader", blending);

    std::string n2 = xlm->GetName() + " Transparents";
    NSString *n = [NSString stringWithUTF8String:n2.c_str()];
    [encoder pushDebugGroup:n];
    [encoder setVertexBuffer:xlm->vbuffer offset:0 atIndex:BufferIndexMeshPositions];
    frameData.renderType = RenderTypeNormal;
    frameData.brightness = brightness;
    frameData.brightness /= 100.0;
    frameDataChanged = false;
    [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
    [encoder setDepthStencilState:canvas->getDepthStencilStateL()];

    xlTexture *lastTexture = nullptr;
    bool lastIsSolid = true;
    for (auto sm : xlm->subMeshes) {
        int mid = sm->material;
        if (mid >= 0 && xlm->GetMaterial(mid).color.alpha != 255) {
            if (xlm->GetMaterial(mid).texture) {
                if (lastIsSolid) {
                    lastIsSolid = false;
                    [encoder setRenderPipelineState:texturePS];
                }
                xlMetalTexture *t = (xlMetalTexture*)xlm->GetMaterial(mid).texture;
                if (t != lastTexture) {
                    [encoder setFragmentTexture:t->texture atIndex:BufferIndexTexturePositions];
                    lastTexture = t;
                }
            } else {
                if (!lastIsSolid) {
                    lastIsSolid = true;
                    [encoder setRenderPipelineState:solidPS];
                }
                simd::float4 color =  xlm->GetMaterial(mid).color.asFloat4();
                if (sm->type == MTLPrimitiveTypeLine) {
                    color = {0.0, 0.0, 0.0, 1.0};
                }
                if (!simd_equal(color, frameData.fragmentColor)) {
                    frameData.fragmentColor = color;
                }
                [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
            }
            // Draw the submesh.
            [encoder drawIndexedPrimitives:sm->type
                                indexCount:sm->count
                                 indexType:MTLIndexTypeUInt32
                               indexBuffer:xlm->ibuffer
                         indexBufferOffset:(sm->startIndex*4)];
        }
    }
    if (!lastIsSolid) {
        [encoder setRenderPipelineState:solidPS];
    }
    [encoder setDepthStencilState:canvas->getDepthStencilStateLE()];
    [encoder popDebugGroup];
    return this;
}
xlGraphicsContext* xlMetalGraphicsContext::drawMeshWireframe(xlMesh *mesh, int brightness) {
    xlMetalMesh *m = (xlMetalMesh*)mesh;
    if (m->vbuffer == nil) {
        m->LoadBuffers();
    }
    setPipelineState("meshWireframeProgram", "meshWireframeVertexShader", "meshSolidFragmentShader");
    std::string n2 = m->GetName() + " Wireframe";
    NSString *n = [NSString stringWithUTF8String:n2.c_str()];
    [encoder pushDebugGroup:n];

    [encoder setVertexBuffer:m->vbuffer offset:0 atIndex:BufferIndexMeshPositions];
    frameData.renderType = RenderTypeNormal;
    frameData.brightness = brightness;
    frameData.brightness /= 100.0;
    frameData.fragmentColor = {0.0, 1.0, 0.0, 1.0};
    frameDataChanged = false;
    [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
    // Draw the Wireframe.
    [encoder drawIndexedPrimitives:MTLPrimitiveTypeLine
                        indexCount:m->wireframeCount
                         indexType:MTLIndexTypeUInt32
                       indexBuffer:m->ibuffer
                 indexBufferOffset:(m->wireframeStart*4)];
    [encoder popDebugGroup];
    return this;
}

//drawing methods
xlGraphicsContext* xlMetalGraphicsContext::drawLines(xlVertexAccumulator *vac, const xlColor &c, int start, int count) {
    if (frameData.renderType != RenderTypeNormal) {
        frameData.renderType = RenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(MTLPrimitiveTypeLine, vac, c, start, count);
}
xlGraphicsContext* xlMetalGraphicsContext::drawLineStrip(xlVertexAccumulator *vac, const xlColor &c, int start, int count) {
    if (frameData.renderType != RenderTypeNormal) {
        frameData.renderType = RenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(MTLPrimitiveTypeLineStrip, vac, c, start, count);
}
xlGraphicsContext* xlMetalGraphicsContext::drawTriangles(xlVertexAccumulator *vac, const xlColor &c, int start, int count) {
    if (frameData.renderType != RenderTypeNormal) {
        frameData.renderType = RenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(MTLPrimitiveTypeTriangle, vac, c, start, count);
}
xlGraphicsContext* xlMetalGraphicsContext::drawTriangleStrip(xlVertexAccumulator *vac, const xlColor &c, int start, int count) {
    if (frameData.renderType != RenderTypeNormal) {
        frameData.renderType = RenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(MTLPrimitiveTypeTriangleStrip, vac, c, start, count);
}

xlGraphicsContext* xlMetalGraphicsContext::drawPoints(xlVertexAccumulator *vac, const xlColor &c, float pointSize, bool smoothPoints, int start, int count) {
    setPointSize(pointSize, smoothPoints);
    drawPrimitive(MTLPrimitiveTypePoint, vac, c, start, count);
    return this;
}
xlGraphicsContext* xlMetalGraphicsContext::drawPrimitive(MTLPrimitiveType type, xlVertexAccumulator *vac, const xlColor &c, int start, int count) {
    if (vac->getCount() == 0) {
        return this;
    }
    if (type == MTLPrimitiveTypePoint && frameData.renderType == RenderTypePointsSmooth) {
        setPipelineState("singleColorPointsProgram", "singleColorVertexShader", "pointSmoothFragmentShader");
    } else {
        setPipelineState("singleColorProgram", "singleColorVertexShader", "colorFragmentShader");
    }
    xlMetalVertexAccumulator *mva = (xlMetalVertexAccumulator*)vac;
    if (vac != lastAccumulator || !mva->finalized) {
        lastAccumulator = vac;
        mva->SetBufferBytes(canvas->getMTLDevice(), encoder, BufferIndexMeshPositions);
    }

    frameData.fragmentColor.r = c.red;
    frameData.fragmentColor.g = c.green;
    frameData.fragmentColor.b = c.blue;
    frameData.fragmentColor.a = c.alpha;
    frameData.fragmentColor /= 255.0f;

    [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
    frameDataChanged = false;
    int ct = count;
    if (ct < 0) {
        ct = vac->getCount() - start;
    }
    if (ct <= 0) {
        return this;
    }
    [encoder drawPrimitives:type vertexStart:start vertexCount:ct];
    return this;
}
xlGraphicsContext* xlMetalGraphicsContext::drawLines(xlVertexColorAccumulator *vac, int start, int count) {
    if (frameData.renderType != RenderTypeNormal) {
        frameData.renderType = RenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(MTLPrimitiveTypeLine, vac, start, count);
}
xlGraphicsContext* xlMetalGraphicsContext::drawLineStrip(xlVertexColorAccumulator *vac, int start, int count) {
    if (frameData.renderType != RenderTypeNormal) {
        frameData.renderType = RenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(MTLPrimitiveTypeLineStrip, vac, start, count);
}
xlGraphicsContext* xlMetalGraphicsContext::drawTriangles(xlVertexColorAccumulator *vac, int start, int count) {
    if (frameData.renderType != RenderTypeNormal) {
        frameData.renderType = RenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(MTLPrimitiveTypeTriangle, vac, start, count);
}
xlGraphicsContext* xlMetalGraphicsContext::drawTriangleStrip(xlVertexColorAccumulator *vac, int start, int count) {
    if (frameData.renderType != RenderTypeNormal) {
        frameData.renderType = RenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(MTLPrimitiveTypeTriangleStrip, vac, start, count);
}
xlGraphicsContext* xlMetalGraphicsContext::drawPoints(xlVertexColorAccumulator *vac, float pointSize, bool smoothPoints, int start, int count) {
    setPointSize(pointSize, smoothPoints);
    drawPrimitive(MTLPrimitiveTypePoint, vac, start, count);
    return this;
}

xlGraphicsContext* xlMetalGraphicsContext::drawPrimitive(MTLPrimitiveType type, xlVertexColorAccumulator *vac, int start, int count) {
    if (vac->getCount() == 0) {
        return this;
    }
    if (type == MTLPrimitiveTypePoint && frameData.renderType == RenderTypePointsSmooth) {
        setPipelineState("multiColorPointsProgram", "multiColorVertexShader", "pointSmoothFragmentShader");
    } else {
        setPipelineState("multiColorProgram", "multiColorVertexShader", "colorFragmentShader");
    }
    xlMetalVertexColorAccumulator *mva = (xlMetalVertexColorAccumulator*)vac;
    if (vac != lastAccumulator || !mva->finalized) {
        lastAccumulator = vac;
        mva->SetBufferBytes(canvas->getMTLDevice(), encoder, BufferIndexMeshPositions, BufferIndexMeshColors);
    }

    if (frameDataChanged) {
        [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
        frameDataChanged = false;
    }
    int c = count;
    if (c < 0) {
        c = vac->getCount() - start;
    }
    if (c <= 0) {
        return this;
    }
    [encoder drawPrimitives:type vertexStart:start vertexCount:c];
    return this;
}

xlGraphicsContext* xlMetalGraphicsContext::drawLines(xlVertexIndexedColorAccumulator *vac, int start, int count) {
    if (frameData.renderType != RenderTypeNormal) {
        frameData.renderType = RenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(MTLPrimitiveTypeLine, vac, start, count);
}
xlGraphicsContext* xlMetalGraphicsContext::drawLineStrip(xlVertexIndexedColorAccumulator *vac, int start, int count) {
    if (frameData.renderType != RenderTypeNormal) {
        frameData.renderType = RenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(MTLPrimitiveTypeLineStrip, vac, start, count);
}
xlGraphicsContext* xlMetalGraphicsContext::drawTriangles(xlVertexIndexedColorAccumulator *vac, int start, int count) {
    if (frameData.renderType != RenderTypeNormal) {
        frameData.renderType = RenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(MTLPrimitiveTypeTriangle, vac, start, count);
}
xlGraphicsContext* xlMetalGraphicsContext::drawTriangleStrip(xlVertexIndexedColorAccumulator *vac, int start, int count) {
    if (frameData.renderType != RenderTypeNormal) {
        frameData.renderType = RenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(MTLPrimitiveTypeTriangleStrip, vac, start, count);
}
xlGraphicsContext* xlMetalGraphicsContext::drawPoints(xlVertexIndexedColorAccumulator *vac, float pointSize, bool smoothPoints, int start, int count) {
    setPointSize(pointSize, smoothPoints);
    drawPrimitive(MTLPrimitiveTypePoint, vac, start, count);
    return this;
}


xlGraphicsContext* xlMetalGraphicsContext::drawPrimitive(MTLPrimitiveType type, xlVertexIndexedColorAccumulator *vac, int start, int count) {
    if (vac->getCount() == 0) {
        return this;
    }
    if (type == MTLPrimitiveTypePoint && frameData.renderType == RenderTypePointsSmooth) {
        setPipelineState("indexedColorPointsProgram", "indexedColorVertexShader", "pointSmoothFragmentShader");
    } else {
        setPipelineState("indexedColorProgram", "indexedColorVertexShader", "colorFragmentShader");
    }
    xlMetalVertexIndexedColorAccumulator *mva = (xlMetalVertexIndexedColorAccumulator*)vac;
    if (vac != lastAccumulator || !mva->finalized) {
        lastAccumulator = vac;
        mva->SetBufferBytes(canvas->getMTLDevice(), encoder, BufferIndexMeshPositions, BufferIndexMeshColors);
    }

    if (frameDataChanged) {
        [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
        frameDataChanged = false;
    }
    int c = count;
    if (c < 0) {
        c = vac->getCount() - start;
    }
    if (c <= 0) {
        return this;
    }
    [encoder drawPrimitives:type vertexStart:start vertexCount:c];
    return this;
}

xlGraphicsContext* xlMetalGraphicsContext::drawTexture(xlTexture *texture,
                                                       float x, float y, float x2, float y2,
                                                       float tx, float ty, float tx2, float ty2,
                                                       bool linearScale,
                                                       int brightness, int alpha) {
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
    lastAccumulator = nullptr;

    float texturePoints[] {
        tx, ty,
        tx2, ty,
        tx2, ty2,
        tx, ty,
        tx, ty2,
        tx2, ty2
    };
    [encoder setVertexBytes:texturePoints length:(12*sizeof(float)) atIndex:BufferIndexTexturePositions];

    float b = brightness;
    b /= 100.0f;
    float a = alpha;
    a /= 255.0f;
    
    simd::float4 fc = {b, b, b, a};
    if (!simd_equal(fc, frameData.fragmentColor)) {
        frameData.fragmentColor = fc;
        frameDataChanged = true;
    }
    if (frameData.renderType != RenderTypeNormal) {
        frameData.renderType = RenderTypeNormal;
        frameDataChanged = true;
    }
    if (frameDataChanged) {
        [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
        frameDataChanged = false;
    }

    xlMetalTexture *txt = (xlMetalTexture*)texture;
    [encoder setFragmentTexture:txt->texture atIndex:TextureIndexBase];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:va.getCount()];
    return this;
}

xlGraphicsContext* xlMetalGraphicsContext::drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, int brightness, uint8_t alpha, int start, int count) {
    if (vac->getCount() == 0) {
        return this;
    }
    xlMetalVertexTextureAccumulator *mva = (xlMetalVertexTextureAccumulator*)vac;
    setPipelineState("textureProgram", "textureVertexShader", "textureFragmentShader");
    if (vac != lastAccumulator || !mva->finalized) {
        lastAccumulator = vac;
        mva->SetBufferBytes(canvas->getMTLDevice(), encoder, BufferIndexMeshPositions, BufferIndexTexturePositions);
    }

    float b = brightness;
    b /= 100.0;
    float a = alpha;
    a /= 255.0;
    simd::float4 fc = {b, b, b, a};
    if (!simd_equal(fc, frameData.fragmentColor)) {
        frameData.fragmentColor = fc;
        frameDataChanged = true;
    }
    if (frameData.renderType != RenderTypeNormal) {
        frameData.renderType = RenderTypeNormal;
        frameDataChanged = true;
    }
    if (frameDataChanged) {
        [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
        frameDataChanged = false;
    }
    xlMetalTexture *txt = (xlMetalTexture*)texture;
    [encoder setFragmentTexture:txt->texture atIndex:TextureIndexBase];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vac->getCount()];
    return this;
}
xlGraphicsContext* xlMetalGraphicsContext::drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, const xlColor &c, int start, int count) {
    if (vac->getCount() == 0) {
        return this;
    }
    xlMetalVertexTextureAccumulator *mva = (xlMetalVertexTextureAccumulator*)vac;
    setPipelineState("textureColorProgram", "textureVertexShader", "textureColorFragmentShader");
    if (vac != lastAccumulator || !mva->finalized) {
        lastAccumulator = vac;
        mva->SetBufferBytes(canvas->getMTLDevice(), encoder, BufferIndexMeshPositions, BufferIndexTexturePositions);
    }

    frameData.fragmentColor.r = c.red;
    frameData.fragmentColor.g = c.green;
    frameData.fragmentColor.b = c.blue;
    frameData.fragmentColor.a = c.alpha;
    frameData.fragmentColor /= 255.0f;
    [encoder setVertexBytes:&frameData  length:sizeof(frameData) atIndex:BufferIndexFrameData];
    frameDataChanged = false;
    xlMetalTexture *txt = (xlMetalTexture*)texture;
    [encoder setFragmentTexture:txt->texture atIndex:TextureIndexBase];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vac->getCount()];
    return this;
}


xlGraphicsContext* xlMetalGraphicsContext::SetViewport(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y, bool is3D) {
    double x, y, x2, y2;
    x = topleft_x;
    y = std::min(bottomright_y, topleft_y);
    x2 = bottomright_x;
    y2 = std::max(bottomright_y,topleft_y);
    float zMax = 0.0;
    double w = std::max(x, x2) - std::min(x, x2);
    double h = std::max(y, y2) - std::min(y, y2);
    if (is3D) {
        zMax = 0.0f;
    }
    if (canvas->drawingUsingLogicalSize()) {
        MTLViewport vp = {  canvas->translateToBacking(topleft_x), canvas->translateToBacking(y),
            canvas->translateToBacking(w), canvas->translateToBacking(h), zMax, 1.0 };
        [encoder setViewport:vp];
    } else {
        MTLViewport vp = {  (double)topleft_x, y, w, h, zMax, 1.0 };
        [encoder setViewport:vp];
    }
    if (is3D) {
        glm::mat4 m = glm::perspective(glm::radians(45.0f), (float) (bottomright_x-topleft_x) / (float)(topleft_y-bottomright_y), 1.0f, 200000.0f);
        simd::float4x4 *vm = (simd::float4x4*)glm::value_ptr(m);
        frameData.MVP = *vm;
        frameData.perspectiveMatrix = frameData.MVP;
    } else {
        frameData.MVP = matrix_ortho_left_hand(topleft_x, bottomright_x, bottomright_y, topleft_y, 1.0, 0.0);
        frameData.perspectiveMatrix = frameData.MVP;
    }
    frameDataChanged = true;
    return this;
}



//manipulating the matrices
xlGraphicsContext* xlMetalGraphicsContext::PushMatrix() {
    matrixStack.push(frameData.MVP);
    matrixStack.push(frameData.viewMatrix);
    matrixStack.push(frameData.modelMatrix);
    return this;
}
xlGraphicsContext* xlMetalGraphicsContext::PopMatrix() {
    if (!matrixStack.empty()) {
        if (frameData.modelMatrix != matrixStack.top()) {
            frameData.modelMatrix = matrixStack.top();
            frameDataChanged = true;
        }
        matrixStack.pop();
        if (frameData.viewMatrix != matrixStack.top()) {
            frameData.viewMatrix = matrixStack.top();
            frameDataChanged = true;
        }
        matrixStack.pop();
        if (frameData.MVP != matrixStack.top()) {
            frameData.MVP = matrixStack.top();
            frameDataChanged = true;
        }
        matrixStack.pop();
    }
    return this;
}

inline simd::float4x4 mapMatrix(const glm::mat4 &m) {
    simd_float4 m0 = {m[0][0], m[0][1], m[0][2], m[0][3]};
    simd_float4 m1 = {m[1][0], m[1][1], m[1][2], m[1][3]};
    simd_float4 m2 = {m[2][0], m[2][1], m[2][2], m[2][3]};
    simd_float4 m3 = {m[3][0], m[3][1], m[3][2], m[3][3]};
    return simd::float4x4(m0, m1, m2, m3);
}

xlGraphicsContext* xlMetalGraphicsContext::SetCamera(const glm::mat4 &m) {
    simd::float4x4 vm = mapMatrix(m);
    frameData.MVP = matrix_multiply(frameData.MVP, vm);
    frameData.viewMatrix = matrix_multiply(frameData.viewMatrix, vm);
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlMetalGraphicsContext::ScaleViewMatrix(float w, float h, float z) {
    simd::float4x4 m = matrix4x4_scale(w, h, z);
    frameData.MVP = matrix_multiply(frameData.MVP, m);
    frameData.viewMatrix = matrix_multiply(frameData.viewMatrix, m);
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlMetalGraphicsContext::TranslateViewMatrix(float x, float y, float z) {
    simd::float4x4 m = matrix4x4_translation(x, y, z);
    frameData.MVP = matrix_multiply(frameData.MVP, m);
    frameData.viewMatrix = matrix_multiply(frameData.viewMatrix, m);
    frameDataChanged = true;
    return this;
}


xlGraphicsContext* xlMetalGraphicsContext::SetModelMatrix(const glm::mat4 &m) {
    simd::float4x4 vm = mapMatrix(m);
    frameData.MVP = matrix_multiply(frameData.MVP, vm);
    frameData.modelMatrix = vm;
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlMetalGraphicsContext::ApplyMatrix(const glm::mat4 &m) {
    simd::float4x4 vm = mapMatrix(m);
    frameData.MVP = matrix_multiply(frameData.MVP, vm);
    frameData.modelMatrix = matrix_multiply(frameData.modelMatrix, vm);
    frameDataChanged = true;
    return this;
}

xlGraphicsContext* xlMetalGraphicsContext::Translate(float x, float y, float z) {
    simd::float4x4 m = matrix4x4_translation(x, y, z);
    frameData.MVP = matrix_multiply(frameData.MVP, m);
    frameData.modelMatrix = matrix_multiply(frameData.modelMatrix, m);
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlMetalGraphicsContext::Rotate(float angle, float x, float y, float z) {
    simd::float4x4 m = matrix4x4_rotation(angle * 3.14159f/180.0f, x, y, z);
    frameData.MVP = matrix_multiply(frameData.MVP, m);
    frameData.modelMatrix = matrix_multiply(frameData.modelMatrix, m);
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlMetalGraphicsContext::Scale(float w, float h, float z) {
    simd::float4x4 m = matrix4x4_scale(w, h, z);
    frameData.MVP = matrix_multiply(frameData.MVP, m);
    frameData.modelMatrix = matrix_multiply(frameData.modelMatrix, m);
    frameDataChanged = true;
    return this;
}

void xlMetalGraphicsContext::setPointSize(float ps, bool smoothPoints) {
    ps += 1.0;
    
    RenderType nrt = smoothPoints ? RenderTypePointsSmooth : RenderTypePoints;
    if (frameData.renderType != nrt) {
        frameData.renderType = nrt;
        frameDataChanged = true;
    }
    if (frameData.pointSize != ps) {
        frameData.pointSize = ps;
        
        float delta = 1.0 / (ps);
        float mid = 0.35 + 0.15 * ((ps - 2.0f)/25.0f);
        if (mid > 0.5) {
            mid = 0.5;
        }
        frameData.pointSmoothMin = std::max(0.0f, mid - delta);
        frameData.pointSmoothMax = std::min(1.0f, mid + delta);
        frameDataChanged = true;
    }
}


bool xlMetalGraphicsContext::setPipelineState(const std::string &name, const char *vShader, const char *fShader) {
    if (lastPipeline != name || blending != lastPipelineBlend) {
        [encoder setRenderPipelineState:canvas->getPipelineState(name, vShader, fShader, blending)];
        lastPipeline = name;
        lastPipelineBlend = blending;
        lastAccumulator = nullptr;
        return true;
    }
    return false;
}


xlGraphicsContext* xlMetalGraphicsContext::pushDebugContext(const std::string &label) {
    NSString *n = [NSString stringWithUTF8String:label.c_str()];
    [encoder pushDebugGroup:n];
    return this;
}
xlGraphicsContext* xlMetalGraphicsContext::popDebugContext() {
    [encoder popDebugGroup];
    return this;
}

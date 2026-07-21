#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <stdint.h>
#include <cassert>
#include <cmath>
#include <map>
#include <list>
#include <vector>
#include <atomic>
#include "../../include/globals.h"

#include "Color.h"
#include "TextDrawingContext.h"
#include "utils/xlSize.h"
#include "ColorCurve.h"
#include "models/Node.h"

//added hash_map, queue, vector: -DJ
#ifdef _MSC_VER
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif
#include <unordered_map>
#include <queue> //priority_queue
#include <deque>
#include <vector>

class AudioManager;
class RenderContext;
class DmxModel;
enum class HEADER_INFO_TYPES;

// eventually this will go in some header..
// the idea is to define this (currently) for the MS compiler
// and to switch its value based on creating vs using the dll
// NCCDLLIMPL is set by the project creating the dll
#ifdef _MSC_VER
#ifdef NCCDLLIMPL
#define NCCDLLEXPORT __declspec(dllexport)
#else
#define NCCDLLEXPORT __declspec(dllimport)
#endif
#else
#define NCCDLLEXPORT
#endif

class Effect;
class SettingsMap;
class SequenceElements;
class SequenceMedia;
class MetalRenderBufferComputeData;
class PixelBufferClass;
struct EffectFrameState;


// TextDrawingContext is now defined in TextDrawingContext.h

class PaletteClass
{
private:
    xlColorVector color;
    hsvVector hsv;
    xlColorCurveVector cc;
    const ColorCurve nilcc;

public:

    void UpdateForProgress(float progress)
    {
        int i = 0;
        for (const auto& it : cc)
        {
            if (it.IsActive())
            {
                color[i] = xlColor(it.GetValueAt(progress));
                hsv[i] = color[i].asHSV();
            }
            i++;
        }
    }

    void Set(const xlColorVector& newcolors, const xlColorCurveVector& newcc)
    {
        assert(newcolors.size() == newcc.size());

        cc = newcc;
        color = newcolors;
        hsv.clear();
        for (size_t i = 0; i < newcolors.size(); i++)
        {
            hsv.push_back(newcolors[i].asHSV());
        }
    }

    size_t Size() const
    {
        return std::max(1, (int)color.size());
    }

    size_t ExplicitSize() const
    {
        return color.size();
    }

    const ColorCurve& GetColorCurve(size_t idx) const
    {
        if (idx >= cc.size()) {
            return nilcc;
        }
        return cc[idx];
    }

    const xlColor& GetColor(size_t idx) const {
        if (idx >= color.size()) {
            return xlWHITE;
        }

        return color[idx];
    }

    xlColor GetColor(size_t idx, float progress) const {
        if (idx >= color.size()) {
            return xlWHITE;
        }

        if (cc[idx].IsActive())
        {
            return cc[idx].GetValueAt(progress);
        }
        return color[idx];
    }

    void GetColor(size_t idx, xlColor& c) const
    {
        if (idx >= color.size())
        {
            c.Set(255, 255, 255);
        }
        else
        {
            c = color[idx];
        }
    }

    bool IsSpatial(size_t idx) const
    {
        if (idx >= color.size()) return false;
        return (cc[idx].IsActive() && cc[idx].GetTimeCurve() != TC_TIME);
    }

    bool IsGradient(size_t idx) const {
        if (idx >= color.size())
            return false;
        return (cc[idx].IsActive() && cc[idx].GetTimeCurve() == TC_TIME);
    }

    bool IsRadial(size_t idx) const
    {
        if (idx >= color.size())
            return false;
        return (cc[idx].IsActive() && (cc[idx].GetTimeCurve() == TC_RADIALIN || cc[idx].GetTimeCurve() == TC_RADIALOUT || cc[idx].GetTimeCurve() == TC_CW || cc[idx].GetTimeCurve() == TC_CCW));
    }

    xlColor CalcRoundColor(size_t idx, double round, int type) const
    {
        if (idx < cc.size())
        {
            if (type == TC_CW)
            {
                return cc[idx].GetValueAt(round);
            }
            else
            {
                return cc[idx].GetValueAt(1.0 - round);
            }
        }
        else
        {
            return xlWHITE;
        }
    }

    xlColor CalcRadialColour(size_t idx, int centrex, int centrey, int maxradius, int x, int y, int type) const
    {
        if (idx < cc.size())
        {
            float len = sqrt((x - centrex) * (x - centrex) + (y - centrey) * (y - centrey));
            if (type == TC_RADIALIN)
                return cc[idx].GetValueAt(1.0f - len / maxradius);
            else
                return cc[idx].GetValueAt(len / maxradius);
        }
        else
        {
            return xlWHITE;
        }
    }

    void GetSpatialColor(size_t idx, float xcentre, float ycentre, float x, float y, float round, float maxradius, xlColor& c) const
    {
        if (idx >= color.size())
        {
            c.Set(255, 255, 255);
        }
        else
        {
            if (idx < cc.size() && cc[idx].IsActive())
            {
                switch (cc[idx].GetTimeCurve())
                {
                case TC_CW:
                    c = CalcRoundColor(idx, round, TC_CW);
                    break;
                case TC_CCW:
                    c = CalcRoundColor(idx, round, TC_CCW);
                    break;
                case TC_RADIALIN:
                    c = CalcRadialColour(idx, xcentre, ycentre, maxradius, x, y, TC_RADIALIN);
                    break;
                case TC_RADIALOUT:
                    c = CalcRadialColour(idx, xcentre, ycentre, maxradius, x, y, TC_RADIALOUT);
                    break;
                default:
                    c = color[idx];
                    break;
                }
            }
            else
            {
                c = color[idx];
            }
        }
    }

    void GetSpatialColor(size_t idx, float x, float y, xlColor& c) const
    {
        if (idx >= color.size())
        {
            c.Set(255, 255, 255);
        }
        else
        {
            if (idx < cc.size() && cc[idx].IsActive())
            {
                switch (cc[idx].GetTimeCurve())
                {
                case TC_RIGHT:
                    c = cc[idx].GetValueAt(x);
                    break;
                case TC_LEFT:
                    c = cc[idx].GetValueAt(1.0f - x);
                    break;
                case TC_UP:
                    c = cc[idx].GetValueAt(y);
                    break;
                case TC_DOWN:
                    c = cc[idx].GetValueAt(1.0f - y);
                    break;
                default:
                    c = color[idx];
                    break;
                }
            }
            else
            {
                c = color[idx];
            }
        }
    }

    void GetColor(size_t idx, xlColor& c, float progress) const
    {
        if (idx >= color.size())
        {
            c.Set(255, 255, 255);
        }
        else
        {
            if (idx < cc.size() && cc[idx].IsActive())
            {
                c = cc[idx].GetValueAt(progress);
            }
            else
            {
                c = color[idx];
            }
        }
    }

    void GetHSV(size_t idx, HSVValue& c) const
    {
        if (hsv.size() == 0)
        {
            // white
            c.hue = 0.0;
            c.saturation = 0.0;
            c.value = 1.0;
        }
        else
        {
            c = hsv[idx % hsv.size()];
        }
    }

    void GetHSV(size_t idx, HSVValue& c, float progress) const
    {
        if (hsv.size() == 0)
        {
            // white
            c.hue = 0.0;
            c.saturation = 0.0;
            c.value = 1.0;
        }
        else
        {
            if (idx < cc.size() && cc[idx].IsActive())
            {
                c = xlColor(cc[idx].GetValueAt(progress)).asHSV();
            }
            else
            {
                c = hsv[idx % hsv.size()];
            }
        }
    }
};

class /*NCCDLLEXPORT*/ EffectRenderCache {
public:
	EffectRenderCache();
	virtual ~EffectRenderCache();
};

class /*NCCDLLEXPORT*/ RenderBuffer {
public:
    RenderBuffer(RenderContext *ctx, PixelBufferClass *pbc, const Model *m);
    ~RenderBuffer();
    RenderBuffer(RenderBuffer& buffer);
    void InitBuffer(int newBufferHt, int newBufferWi, const std::string& bufferTransform, bool nodeBuffer = false);
    AudioManager* GetMedia() const;
    const Model* GetModel() const;
    const std::string &GetModelName() const;
    const std::string &GetXmlHeaderInfo(HEADER_INFO_TYPES node_type) const;
    SequenceMedia* GetSequenceMedia() const;

    void AlphaBlend(const RenderBuffer& src);
    bool IsNodeBuffer() const { return _nodeBuffer; }
    void Clear();
    void SetPalette(xlColorVector& newcolors, xlColorCurveVector& newcc);
    size_t GetColorCount();
    void SetAllowAlphaChannel(bool a);
    bool IsDmxBuffer() const { return dmx_buffer; }

    void SetState(int period, bool reset);

    void SetEffectDuration(int startMsec, int endMsec);
    void GetEffectPeriods(int& curEffStartPer, int& curEffEndPer) const;  // nobody wants endPer?
    void SetFrameTimeInMs(int i);
    long GetStartTimeMS() const { return curEffStartPer * frameTimeInMs; }
    long GetEndTimeMS() const { return curEffEndPer * frameTimeInMs; }

    const xlColor &GetPixel(int x, int y) const;
    void GetPixel(int x, int y, xlColor &color) const;
    void SetPixel(int x, int y, const xlColor &color, bool wrap = false, bool useAlpha = false, bool dmx_ignore = false);
    void SetPixel(int x, int y, const HSVValue& hsv, bool wrap = false);

    // Snapshot the current pixels into a persistent scratch buffer for callers
    // (rotate/small-blur) that need to read the pre-transform frame while
    // writing the live buffer in place - avoids a full RenderBuffer copy-ctor
    // (heap alloc + copy) every frame. Same GetPixel bounds/out-of-range
    // semantics, backed by the scratch buffer instead of pixels.
    void SnapshotTransformScratch();
    const xlColor &GetTransformScratchPixel(int x, int y) const;
    void GetTransformScratchPixel(int x, int y, xlColor &color) const;

    //optimized/direct versions only usable in cases where x/y are known to be within bounds
    void SetPixelDirect(int x, int y, const xlColor &color) {
        pixels[y * BufferWi + x] = color;
    }
    const xlColor& GetPixelDirect(int x, int y) const {
        return pixels[y * BufferWi + x];
    }

    int GetNodeCount() const { return Nodes.size();}
    const std::vector<NodeBaseClassPtr>& GetNodes() const { return Nodes; }
    void SetNodePixel(int nodeNum, const xlColor &color, bool dmx_ignore = false);

    void CopyPixel(int srcx, int srcy, int destx, int desty);
    void ProcessPixel(int x, int y, const xlColor &color, bool wrap_x = false, bool wrap_y = false);

    void ClearTempBuf();
    const xlColor &GetTempPixelRGB(int x, int y);
    void SetTempPixel(int x, int y, const xlColor &color, int alpha);
    void SetTempPixel(int x, int y, const xlColor &color);
    void GetTempPixel(int x, int y, xlColor &color);
    const xlColor &GetTempPixel(int x, int y);

    void Fill(const xlColor& color);
    void DrawHLine(int y, int xstart, int xend, const xlColor& color, bool wrap = false);
    void DrawVLine(int x, int ystart, int yend, const xlColor& color, bool wrap = false);
    void DrawBox(int x1, int y1, int x2, int y2, const xlColor& color, bool wrap = false, bool useAlpha = false);
    void DrawFadingCircle(int x0, int y0, int radius, const xlColor& rgb, bool wrap = false);
    void DrawCircle(int xc, int yc, int r, const xlColor& color, bool filled = false, bool wrap = false);
    void DrawLine(const int x1_, const int y1_, const int x2_, const int y2_, const xlColor& color, bool useAlpha = false);
    void DrawAALine(const float x0, const float y0, const float x1, const float y1, const xlColor& color);
    void DrawAACircle(const float cx, const float cy, const float radius, const xlColor& color);
    void DrawThickLine(const int x1_, const int y1_, const int x2_, const int y2_, const xlColor& color, int thickness, bool useAlpha = false);
    void DrawThickLine(const int x1_, const int y1_, const int x2_, const int y2_, const xlColor& color, bool direction);

    void FillConvexPoly(const std::vector<std::pair<int, int>>& poly, const xlColor& color);

    //approximation of sin/cos, but much faster
    template<typename T>
    static inline T sin(T rad) noexcept { return std::sin(rad); }
    template<typename T>
    static inline T cos(T rad) noexcept { return std::cos(rad); }
    template<typename T>
    static inline T cot(T rad) noexcept { return std::cos(rad) / std::sin(rad); }
    template<typename T>
    static inline T acot(T rad) noexcept { return M_PI/2.0f - std::atan(rad); }

    double calcAccel(double ratio, double accel);

    uint8_t ChannelBlend(uint8_t c1, uint8_t c2, float ratio);
    void Get2ColorBlend(int coloridx1, int coloridx2, float ratio, xlColor &color);
    void Get2ColorBlend(xlColor& color, xlColor color2, float ratio);
    void Get2ColorAlphaBlend(const xlColor& c1, const xlColor& c2, float ratio, xlColor &color);
    void GetMultiColorBlend(float n, bool circular, xlColor &color, int reserveColors = 0);
    void SetRangeColor(const HSVValue& hsv1, const HSVValue& hsv2, HSVValue& newhsv);
    double RandomRange(double num1, double num2);
    void Color2HSV(const xlColor& color, HSVValue& hsv) const;

    // --- Deterministic per-buffer RNG (reproducible renders) -----------------
    // Seeded from a stable hash of (model name, layer, effect-start) and reseeded
    // lazily on first use each frame, so a given draw reproduces regardless of
    // thread, render start frame, or what other effects drew. Use rand01()/
    // randInt() in serial code. Inside parallel_for bodies use the stateless
    // hashRand01()/hashRandom(index) keyed on the element index - they touch no
    // shared state, so they are thread-safe and order-independent.
    inline double rand01() {
        ensureRandomSeed();
        return (rngMix64(rngState += 0x9E3779B97F4A7C15ULL) >> 11) * (1.0 / 9007199254740992.0);
    }
    inline int randInt(int lo, int hi) { // inclusive [lo, hi]
        if (hi <= lo) return lo;
        ensureRandomSeed();
        uint32_t range = uint32_t(hi - lo) + 1u;
        uint32_t r = uint32_t(rngMix64(rngState += 0x9E3779B97F4A7C15ULL));
        return lo + int((uint64_t(r) * range) >> 32);
    }
    inline uint32_t hashRandom(uint32_t index) const {
        return uint32_t(rngMix64(rngHashInput(index)) >> 32);
    }
    inline double hashRand01(uint32_t index) const {
        return (rngMix64(rngHashInput(index)) >> 11) * (1.0 / 9007199254740992.0);
    }
    // Frame-INDEPENDENT stateless hash for position-anchored randomness that must
    // stay stable across frames (e.g. a per-pixel seed that shouldn't jitter).
    // Same (model, layer, effect, index) -> same value on every frame.
    inline uint32_t hashRandomStable(uint32_t index) const {
        return uint32_t(rngMix64(rngBaseSeed ^ (uint64_t(index) * 0xD1B54A32D192ED03ULL)) >> 32);
    }
    // Like hashRandomStable but keyed ONLY on (model, index) - independent of the
    // current effect/layer/frame. Use for per-node values that outlive a single
    // effect and must be identical no matter which effect/frame first computes
    // them (e.g. the sparkle phase, which the serial main buffer and the
    // frame-parallel clones lazily initialize on different frames/effects).
    inline uint32_t hashModelStable(uint32_t index) const {
        return uint32_t(rngMix64(rngModelHash ^ (uint64_t(index) * 0xD1B54A32D192ED03ULL)) >> 32);
    }
    // Raw 64-bit per-(effect,frame) seed behind hashRandom()/hashRand01(); lets
    // ISPC/Metal kernels reproduce the hash stream bit-exactly off-CPU:
    // hashRandom(index) == mix64(frameSeed ^ (uint64(index) * 0xD1B54A32D192ED03)) >> 32.
    inline uint64_t hashRandomFrameSeed() const {
        return rngHashInput(0);
    }
    // Force the next rand01()/randInt() this frame to reseed from scratch (as if
    // the frame were drawn for the first time). The serial RNG already reseeds
    // per frame via ensureRandomSeed(); this lets the XL_VERIFY_STATELESS harness
    // re-render the SAME frame twice and get the identical stream, instead of the
    // second pass continuing where the first left off - which made every
    // randInt()/rand01() effect falsely look like it carried cross-frame state.
    void resetSerialRandomForVerify() { rngSeededForPeriod = -1; }
    void SetLayerIndex(int idx) { rngLayerIndex = idx; }
    const PaletteClass& GetPalette() const { return palette; }

    HSVValue Get2ColorAdditive(HSVValue& hsv1, HSVValue& hsv2) const;
    float GetEffectTimeIntervalPosition() const;
    float GetEffectTimeIntervalPosition(float cycles) const;

    TextDrawingContext * GetTextDrawingContext();

    void CopyPixelsToDisplayListX(Effect *eff, int y, int sx, int ex, int inc = 1);
    // must hold the lock and be sized appropriately
    void SetDisplayListHRect(Effect *eff, int startIdx, float x1, float y1, float x2, float y2,
                             const xlColor &cx1, const xlColor &cx2);
    void SetDisplayListVRect(Effect *eff, int startIdx, float x1, float y1, float x2, float y2,
                             const xlColor &cy1, const xlColor &cy2);
    void SetDisplayListRect(Effect *eff, int startIdx, float x1, float y1, float x2, float y2,
                            const xlColor &cx1y1, const xlColor &cx1y2,
                            const xlColor &cx2y1, const xlColor &cx2y2);

    int BufferHt = 1;
    int BufferWi = 1;  // size of the buffer

private:
    xlColorVector pixelVector; // this is the calculation buffer
    xlColorVector tempbufVector;
    xlColor *pixels = nullptr;
    xlColor *tempbuf = nullptr;
    // Scratch for SnapshotTransformScratch()/GetTransformScratchPixel(); resized
    // (never proactively shrunk in capacity) to pixelVector's size on each snapshot.
    xlColorVector transformScratch;
    void ensureTempBuf();

    std::vector<uint32_t> blendBuffer;
    std::vector<uint32_t> indexVector;
    bool allSimpleIndex = true;
    // true when two nodes share an ActChan (e.g. a group listing a nested
    // group plus that group's members) — parallel per-node channel writes
    // would race for the shared channel, so callers must use serial paths
    bool dupActChans = false;
public:
    uint32_t GetPixelCount() { return pixelVector.size(); }
    xlColor *GetPixels() { return pixels; }
    xlColor *GetTempBuf() { ensureTempBuf(); return tempbuf; }
    // Whole temp-buffer snapshot/restore for frame-parallel Snapshottable effects
    // (e.g. Snowflakes) whose cross-frame state lives in the temp pixel layer.
    const xlColorVector& GetTempBufVector() { ensureTempBuf(); return tempbufVector; }
    void SetTempBufVector(const xlColorVector& v) {
        tempbufVector = v;
        tempbuf = tempbufVector.empty() ? nullptr : &tempbufVector[0];
    }
    void CopyTempBufToPixels();
    void CopyPixelsToTempBuf();
    xlSize GetMaxBuffer(const SettingsMap& SettingsMap) const;
    void EnableFixedDMXChannels(const DmxModel* dmx);

    PaletteClass palette;
    bool _nodeBuffer = false;

    RenderContext *renderContext = nullptr;
    std::string cur_model; //model currently in effect

    int curPeriod = 0;
    int curEffStartPer = 0;    /**< Start period of current effect. */
    int curEffEndPer = 0;      /**<  */
    int frameTimeInMs = 50;
    bool isTransformed = false;

    int fadeinsteps = 0;
    int fadeoutsteps = 0;

    bool needToInit = false;
    bool allowAlpha = false;
    bool dmx_buffer = false;
    bool _isCopy = false;

    /* Places to store and data that is needed from one frame to another */
    std::map<int, EffectRenderCache*> infoCache;

    // Tier-2 frame-parallel plumbing (Snapshottable effects), set transiently by
    // the render engine around a single effect render; effects never read
    // captureSnapshot.
    //  * captureSnapshot: engine-internal.  Non-null marks the serial capture
    //    pre-pass - the engine calls the effect's AdvanceState() and stores the
    //    returned draw-snapshot here (no draw).  The effect's Render() is not
    //    invoked on this pass.
    //  * pendingSnapshot: the draw-protocol input.  Non-null means Render() must
    //    rasterise this previously captured snapshot and NOT advance (check it
    //    first, at the top of Render).  Set in both serial and parallel drawing.
    // Both null for the fused advance+draw of a Pure/Stateful effect.  Owned by
    // the engine.
    std::unique_ptr<EffectFrameState>* captureSnapshot = nullptr;
    const EffectFrameState* pendingSnapshot = nullptr;

    //place for GPU Renderers to attach extra data/objects it needs
    void *gpuRenderData = nullptr;

    uint32_t perModelIndex = 0;
    AudioManager* _mediaOverride = nullptr; // set by effects to use an alt audio track

private:
    PixelBufferClass *parent;
    const Model *model;
    friend class PixelBufferClass;
    std::vector<NodeBaseClassPtr> Nodes;
    TextDrawingContext *_textDrawingContext = nullptr;

    // Deterministic per-buffer RNG state (see rand01()/hashRandom()).
    uint64_t rngModelHash = 0;   // stable hash of cur_model, computed once in ctor
    uint64_t rngBaseSeed = 0;    // hash(modelHash, layer, curEffStartPer); per effect
    uint64_t rngState = 0;       // stateful stream, serial path only
    int rngLayerIndex = 0;
    int rngSeededForPeriod = -1; // lazy per-frame reseed guard (serial path)

    void computeRandomBaseSeed(); // recompute rngBaseSeed when the effect changes
    static inline uint64_t rngMix64(uint64_t z) {
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }
    inline uint64_t rngHashInput(uint32_t index) const {
        return rngBaseSeed ^ (uint64_t(uint32_t(curPeriod)) * 0x9E3779B97F4A7C15ULL)
                           ^ (uint64_t(index) * 0xD1B54A32D192ED03ULL);
    }
    inline void ensureRandomSeed() {
        if (rngSeededForPeriod != curPeriod) {
            rngState = rngBaseSeed ^ (uint64_t(uint32_t(curPeriod)) * 0x9E3779B97F4A7C15ULL);
            rngSeededForPeriod = curPeriod;
        }
    }

    void SetPixelDMXModel(int x, int y, const xlColor& color);
    void Forget();
    
    friend class MetalPixelBufferComputeData;
    friend class MetalRenderBufferComputeData;
    friend class VulkanPixelBufferComputeData;
    friend class VulkanRenderBufferComputeData;
    friend class ISPCComputeUtilities;
};

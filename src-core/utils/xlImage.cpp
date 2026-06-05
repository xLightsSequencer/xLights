/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "xlImage.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <unordered_map>

#include "Parallel.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO // we handle file I/O ourselves
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include "stb/stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize2.h"

// Decode GIF LZW compressed data for one frame into palette indices.
// minCodeSize: the LZW minimum code size byte from the image block.
// lzwData: concatenated raw LZW bytes (sub-blocks stripped).
// out: destination buffer, exactly framePixels = fw*fh entries.
static void gifLZWDecode(int minCodeSize, const std::vector<uint8_t>& lzwData,
                         std::vector<uint8_t>& out, int framePixels) {
    out.assign(framePixels, 0);
    if (lzwData.empty() || minCodeSize < 2 || minCodeSize > 11) return;

    const int MAXCODES = 4096;
    const int clearCode = 1 << minCodeSize;
    const int eoiCode   = clearCode + 1;

    uint16_t prefix[MAXCODES];
    uint8_t  suffix[MAXCODES];
    // Initialize root entries
    for (int i = 0; i < clearCode; i++) { prefix[i] = 0xFFFF; suffix[i] = (uint8_t)i; }

    int codeSize = minCodeSize + 1;
    int nextCode  = eoiCode + 1;
    int maxCode   = 1 << codeSize;

    // Bit-stream reader (LSB first, as GIF specifies)
    size_t bitPos = 0;
    const size_t totalBits = lzwData.size() * 8;
    auto readCode = [&]() -> int {
        if (bitPos + codeSize > totalBits) return eoiCode;
        int val = 0;
        for (int i = 0; i < codeSize; i++) {
            val |= ((lzwData[bitPos >> 3] >> (bitPos & 7)) & 1) << i;
            bitPos++;
        }
        return val;
    };

    int outIdx   = 0;
    int prevCode = -1;
    uint8_t firstByte = 0;

    while (outIdx < framePixels) {
        int code = readCode();
        if (code == clearCode) {
            codeSize = minCodeSize + 1;
            nextCode = eoiCode + 1;
            maxCode  = 1 << codeSize;
            prevCode = -1;
            continue;
        }
        if (code == eoiCode || code > nextCode || code >= MAXCODES) break;

        // Decode code to string via stack
        uint8_t stack[MAXCODES];
        int     top = 0;
        int     c   = code;

        if (code == nextCode) {
            // Special case: code not yet in tabBle – string is prevString + prevString[0]
            if (top < MAXCODES) stack[top++] = firstByte;
            c = prevCode;
        }
        while (c >= 0 && c < MAXCODES && prefix[c] != 0xFFFF) {
            if (top >= MAXCODES - 1) break;
            stack[top++] = suffix[c];
            c = prefix[c];
        }
        if (c >= 0 && c < MAXCODES && top < MAXCODES)
            stack[top++] = suffix[c];

        firstByte = stack[top - 1]; // first byte of decoded string

        // Output (stack is reversed)
        for (int i = top - 1; i >= 0 && outIdx < framePixels; i--)
            out[outIdx++] = stack[i];

        // Add new table entry
        if (prevCode >= 0 && nextCode < MAXCODES) {
            prefix[nextCode] = (uint16_t)prevCode;
            suffix[nextCode] = firstByte;
            nextCode++;
            if (nextCode == maxCode && codeSize < 12) {
                codeSize++;
                maxCode <<= 1;
            }
        }
        prevCode = code;
    }
}

AnimatedImageData LoadAnimatedGIFFromMemory(const uint8_t* data, size_t len) {
    AnimatedImageData result;
    if (len < 13 || data[0] != 'G' || data[1] != 'I' || data[2] != 'F') return result;

    // ---- Simple binary reader ----
    size_t pos = 6; // skip "GIF87a" / "GIF89a"
    auto r8  = [&]() -> uint8_t  { return pos < len ? data[pos++] : 0; };
    auto r16 = [&]() -> int      { uint8_t lo = r8(), hi = r8(); return lo | (hi << 8); };
    auto skipBlocks = [&]() {
        while (pos < len) { uint8_t n = r8(); if (!n) break; pos += n; }
    };
    auto readBlocks = [&](std::vector<uint8_t>& out) {
        while (pos < len) {
            uint8_t n = r8(); if (!n) break;
            for (int i = 0; i < n && pos < len; i++) out.push_back(data[pos++]);
        }
    };
    auto readPal = [&](uint8_t pal[][4], int count) {
        for (int i = 0; i < count; i++) {
            pal[i][0] = r8(); pal[i][1] = r8(); pal[i][2] = r8(); pal[i][3] = 255;
        }
    };

    // ---- Logical Screen Descriptor ----
    int cW = r16(), cH = r16();
    uint8_t packed = r8();
    uint8_t bgIdx  = r8();
    r8(); // pixel aspect ratio

    bool hasGCT  = (packed & 0x80) != 0;
    int  gctSize = 2 << (packed & 0x07);

    uint8_t gct[256][4] = {};
    if (hasGCT) readPal(gct, gctSize);

    if (cW <= 0 || cH <= 0) return result;
    result.width  = cW;
    result.height = cH;

    xlColor bgColor;
    if (hasGCT) bgColor = xlColor(gct[bgIdx][0], gct[bgIdx][1], gct[bgIdx][2]);
    result.backgroundColor = bgColor;

    // Single compositing canvas, transparent background (matching old wxGIFDecoder
    // behaviour). Transparent GIF pixels are left as alpha=0 so previews show the
    // panel colour behind the image. BuildNoBGFrames() in AnimatedImage handles the
    // suppress-background rendering case on top of this.
    const size_t canvasBytes = (size_t)cW * cH * 4;

    std::vector<uint8_t> canvas(canvasBytes, 0);

    // Saved canvas state for disposal==3 (restore-to-previous)
    std::vector<uint8_t> saved(canvasBytes, 0);

    // GCE state carried from one image block to the next
    // (GCE describes how to dispose the frame that follows it)
    int prevDisposal   = 0;   // disposal method for the PREVIOUS frame
    int prevFx = 0, prevFy = 0, prevFw = 0, prevFh = 0; // bounding box of prev frame
    int currDisposal   = 0;   // from the most-recently-seen GCE
    int currDelay      = 100; // ms
    int currTranspIdx  = -1;
    bool hadPrevFrame  = false;

    // ---- Main parsing loop ----
    while (pos < len) {
        uint8_t tag = r8();
        if (tag == 0x3B) break; // GIF trailer

        if (tag == 0x21) { // Extension block
            uint8_t ext = r8();
            if (ext == 0xF9) { // Graphic Control Extension
                uint8_t bsz = r8();
                if (bsz >= 4) {
                    uint8_t ef  = r8();
                    currDisposal  = (ef >> 2) & 0x07;
                    int deciSecs  = r16();
                    currDelay     = (deciSecs > 0) ? deciSecs * 10 : 100;
                    if (ef & 0x01) {
                        currTranspIdx = (int)r8();
                    } else {
                        r8(); // consume the byte (always present in the 4-byte block)
                        currTranspIdx = -1;
                    }
                    bsz -= 4;
                }
                if (bsz > 0) pos += bsz;
                skipBlocks(); // consume terminator
            } else {
                skipBlocks();
            }
            continue;
        }

        if (tag != 0x2C) continue; // unknown — skip

        // ---- Image Descriptor ----
        int fx = r16(), fy = r16(), fw = r16(), fh = r16();
        uint8_t imgFlags   = r8();
        bool hasLCT        = (imgFlags & 0x80) != 0;
        bool interlaced    = (imgFlags & 0x40) != 0;
        int  lctEntries    = hasLCT ? (2 << (imgFlags & 0x07)) : 0;

        uint8_t lct[256][4] = {};
        if (hasLCT) readPal(lct, lctEntries);
        const uint8_t (*pal)[4] = hasLCT ? lct : gct;

        // Build a local RGBA palette with transparency applied
        uint8_t localPal[256][4];
        std::memcpy(localPal, pal, sizeof(localPal));
        if (currTranspIdx >= 0 && currTranspIdx < 256)
            localPal[currTranspIdx][3] = 0;

        // LZW decode
        int minCodeSz = r8();
        std::vector<uint8_t> lzwData;
        readBlocks(lzwData);

        int clampW = (fx < cW && fw > 0) ? std::min(fw, cW - fx) : 0;
        int clampH = (fy < cH && fh > 0) ? std::min(fh, cH - fy) : 0;

        std::vector<uint8_t> indices;
        if (clampW > 0 && clampH > 0)
            gifLZWDecode(minCodeSz, lzwData, indices, fw * fh);

        // ---- Apply PREVIOUS frame's disposal before drawing this frame ----
        if (hadPrevFrame) {
            if (prevDisposal == 2) {
                // Restore entire previous-frame rectangle to transparent.
                // GIF spec says "restore to background colour", but we keep the canvas
                // transparent (alpha=0) so that previews show the panel colour behind
                // the image, matching the old wxGIFDecoder behaviour.
                int clampPW = std::min(prevFw, cW - prevFx);
                int clampPH = std::min(prevFh, cH - prevFy);
                for (int r = 0; r < clampPH; r++) {
                    size_t rowOff = ((size_t)(prevFy + r) * cW + prevFx) * 4;
                    std::memset(canvas.data() + rowOff, 0, (size_t)clampPW * 4);
                }
            } else if (prevDisposal == 3) {
                // Restore to the state saved before the previous frame was drawn
                std::copy(saved.begin(), saved.end(), canvas.begin());
            }
            // disposal 0 or 1: leave canvas unchanged
        }

        // Save canvas state before drawing (used if the NEXT frame has disposal==3)
        std::copy(canvas.begin(), canvas.end(), saved.begin());

        // ---- Paint this frame's pixels onto both canvases ----
        // For interlaced frames we must iterate every storage row (not clampH —
        // a clipped storage range can still emit display rows that fall inside
        // the canvas), so the per-row dispY check below does the bounds clip.
        if (!indices.empty() && clampW > 0 && clampH > 0) {
            const int loopH = interlaced ? fh : clampH;
            for (int row = 0; row < loopH; row++) {
                // Map storage row to display row. For interlaced GIFs, LZW
                // bytes are stored in pass order (0,8,16,…,4,12,…,2,6,…,1,3,…)
                // so storage row N decodes to a different display row.
                int dispRow = row;
                if (interlaced) {
                    int cnt = 0;
                    const int starts[4] = {0, 4, 2, 1};
                    const int steps[4]  = {8, 8, 4, 2};
                    for (int p = 0; p < 4; p++) {
                        for (int r2 = starts[p]; r2 < fh; r2 += steps[p], cnt++) {
                            if (cnt == row) { dispRow = r2; goto doneInterlace; }
                        }
                    }
                    doneInterlace:;
                }
                int dispY = fy + dispRow;
                if (dispY < 0 || dispY >= cH) continue;

                for (int col = 0; col < clampW; col++) {
                    uint8_t idx = indices[(size_t)row * fw + col];
                    if (currTranspIdx >= 0 && idx == (uint8_t)currTranspIdx)
                        continue; // transparent pixel — leave canvas unchanged

                    const uint8_t* c = localPal[idx];
                    size_t o = ((size_t)dispY * cW + (fx + col)) * 4;
                    canvas[o]   = c[0]; canvas[o+1] = c[1];
                    canvas[o+2] = c[2]; canvas[o+3] = 255;
                }
            }
        }

        // ---- Snapshot canvas as output frame ----
        {
            uint8_t* frameData = new uint8_t[canvasBytes];
            std::memcpy(frameData, canvas.data(), canvasBytes);
            result.frames.emplace_back(cW, cH, frameData);
            result.frameTimes.push_back((long)currDelay);
        }

        // Carry disposal forward; reset GCE state for next frame
        prevDisposal  = currDisposal;
        prevFx = fx; prevFy = fy; prevFw = fw; prevFh = fh;
        hadPrevFrame  = true;
        currDisposal  = 0;
        currDelay     = 100;
        currTranspIdx = -1;
    }

    if (result.frames.empty()) return result;

    // If every frame delay is 0 (malformed GIF), default to 100 ms
    long total = 0;
    for (long t : result.frameTimes) total += t;
    if (total == 0)
        std::fill(result.frameTimes.begin(), result.frameTimes.end(), 100L);

    return result;
}

bool xlImage::LoadFromMemory(const uint8_t* data, size_t len) {
    int w = 0, h = 0, channels = 0;
    // Request 4 channels (RGBA) regardless of source format
    uint8_t* pixels = stbi_load_from_memory(data, static_cast<int>(len), &w, &h, &channels, 4);
    if (!pixels) {
        return false;
    }
    _width = w;
    _height = h;
    // stbi_load uses malloc, but we need new[] for unique_ptr
    size_t sz = w * h * 4;
    _data.reset(new uint8_t[sz]);
    std::memcpy(_data.get(), pixels, sz);
    stbi_image_free(pixels);
    return true;
}

bool xlImage::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }
    auto size = file.tellg();
    if (size <= 0) {
        return false;
    }
    file.seekg(0);
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    if (!file) {
        return false;
    }
    return LoadFromMemory(buffer.data(), buffer.size());
}

// Callback for stb_image_write to append to a vector
static void stbi_write_to_vector(void* context, void* data, int size) {
    auto* vec = static_cast<std::vector<uint8_t>*>(context);
    auto* bytes = static_cast<uint8_t*>(data);
    vec->insert(vec->end(), bytes, bytes + size);
}

bool xlImage::SaveAsPNG(std::vector<uint8_t>& output) const {
    if (!IsOk()) return false;
    output.clear();
    int result = stbi_write_png_to_func(stbi_write_to_vector, &output,
                                         _width, _height, 4, _data.get(), _width * 4);
    return result != 0;
}

void xlImage::Rescale(int newWidth, int newHeight) {
    if (!IsOk() || newWidth <= 0 || newHeight <= 0) return;
    if (newWidth == _width && newHeight == _height) return;

    uint8_t* resized = new uint8_t[newWidth * newHeight * 4];

    // Use threaded resize for large images (>512px on either dimension)
    constexpr int THREADED_SIZE_THRESHOLD = 512;
    bool useThreaded = (_width > THREADED_SIZE_THRESHOLD || _height > THREADED_SIZE_THRESHOLD
                        || newWidth > THREADED_SIZE_THRESHOLD || newHeight > THREADED_SIZE_THRESHOLD);
    if (useThreaded) {
        STBIR_RESIZE resize;
        stbir_resize_init(&resize,
                          _data.get(), _width, _height, _width * 4,
                          resized, newWidth, newHeight, newWidth * 4,
                          STBIR_RGBA, STBIR_TYPE_UINT8);

        int numThreads = ParallelJobPool::POOL.maxSize();
        if (numThreads < 2) numThreads = 2;
        int actualSplits = stbir_build_samplers_with_splits(&resize, numThreads);

        if (actualSplits > 1) {
            parallel_for(0, actualSplits, [&resize](int i) {
                stbir_resize_extended_split(&resize, i, 1);
            });
        } else {
            stbir_resize_extended(&resize);
        }
        stbir_free_samplers(&resize);
    } else {
        stbir_resize_uint8_linear(
            _data.get(), _width, _height, _width * 4,
            resized, newWidth, newHeight, newWidth * 4,
            STBIR_RGBA);
    }

    _data.reset(resized);
    _width = newWidth;
    _height = newHeight;
}

void xlImage::Clear() {
    if (_data && _width > 0 && _height > 0) {
        std::memset(_data.get(), 0, static_cast<size_t>(_width) * _height * 4);
    }
}

xlImage xlImage::Mirror(bool horizontal) const {
    if (!IsOk()) return xlImage();
    xlImage result(_width, _height);
    for (int y = 0; y < _height; y++) {
        for (int x = 0; x < _width; x++) {
            int sx = horizontal ? (_width - 1 - x) : x;
            int sy = horizontal ? y : (_height - 1 - y);
            int srcIdx = (sy * _width + sx) * 4;
            int dstIdx = (y * _width + x) * 4;
            std::memcpy(result._data.get() + dstIdx, _data.get() + srcIdx, 4);
        }
    }
    return result;
}

xlImage xlImage::Rotate90(bool clockwise) const {
    if (!IsOk()) return xlImage();
    xlImage result(_height, _width); // swapped dimensions
    for (int y = 0; y < _height; y++) {
        for (int x = 0; x < _width; x++) {
            int dx, dy;
            if (clockwise) {
                dx = _height - 1 - y;
                dy = x;
            } else {
                dx = y;
                dy = _width - 1 - x;
            }
            int srcIdx = (y * _width + x) * 4;
            int dstIdx = (dy * _height + dx) * 4;
            std::memcpy(result._data.get() + dstIdx, _data.get() + srcIdx, 4);
        }
    }
    return result;
}

xlImage xlImage::Rotate180() const {
    if (!IsOk()) return xlImage();
    xlImage result(_width, _height);
    for (int y = 0; y < _height; y++) {
        for (int x = 0; x < _width; x++) {
            int srcIdx = (y * _width + x) * 4;
            int dstIdx = ((_height - 1 - y) * _width + (_width - 1 - x)) * 4;
            std::memcpy(result._data.get() + dstIdx, _data.get() + srcIdx, 4);
        }
    }
    return result;
}

xlImage xlImage::ApplyOrientation(int orient) const {
    switch (orient) {
    case 2: return Mirror(true);  // horizontal flip
    case 3: return Rotate180();
    case 4: return Mirror(false); // vertical flip
    case 5: return Mirror(true).Rotate90(false); // horizontal flip + 90 CCW
    case 6: return Rotate90(true);  // 90 CW
    case 7: return Mirror(true).Rotate90(true);  // horizontal flip + 90 CW
    case 8: return Rotate90(false); // 90 CCW
    default: return Copy();
    }
}

// Parse a hex digit; returns -1 on invalid input
static int hexDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Parse an XPM color spec string into RGBA. Returns false on failure.
static bool parseXpmColor(const char* colorStr, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
    // Skip leading whitespace
    while (*colorStr == ' ' || *colorStr == '\t') colorStr++;

    if (*colorStr == '#') {
        colorStr++;
        size_t len = 0;
        while (colorStr[len] && colorStr[len] != ' ' && colorStr[len] != '\t') len++;
        if (len == 6) {
            // #RRGGBB
            int r1 = hexDigit(colorStr[0]), r2 = hexDigit(colorStr[1]);
            int g1 = hexDigit(colorStr[2]), g2 = hexDigit(colorStr[3]);
            int b1 = hexDigit(colorStr[4]), b2 = hexDigit(colorStr[5]);
            if (r1 < 0 || r2 < 0 || g1 < 0 || g2 < 0 || b1 < 0 || b2 < 0) return false;
            r = (r1 << 4) | r2;
            g = (g1 << 4) | g2;
            b = (b1 << 4) | b2;
            a = 255;
            return true;
        }
        if (len == 3) {
            // #RGB — expand each nibble
            int rv = hexDigit(colorStr[0]);
            int gv = hexDigit(colorStr[1]);
            int bv = hexDigit(colorStr[2]);
            if (rv < 0 || gv < 0 || bv < 0) return false;
            r = (rv << 4) | rv;
            g = (gv << 4) | gv;
            b = (bv << 4) | bv;
            a = 255;
            return true;
        }
        if (len == 12) {
            // #RRRRGGGGBBBB — take high byte of each 16-bit component
            int r1 = hexDigit(colorStr[0]), r2 = hexDigit(colorStr[1]);
            int g1 = hexDigit(colorStr[4]), g2 = hexDigit(colorStr[5]);
            int b1 = hexDigit(colorStr[8]), b2 = hexDigit(colorStr[9]);
            if (r1 < 0 || r2 < 0 || g1 < 0 || g2 < 0 || b1 < 0 || b2 < 0) return false;
            r = (r1 << 4) | r2;
            g = (g1 << 4) | g2;
            b = (b1 << 4) | b2;
            a = 255;
            return true;
        }
        return false;
    }

    // Named colors — case-insensitive comparison
    std::string name(colorStr);
    // Trim trailing whitespace
    while (!name.empty() && (name.back() == ' ' || name.back() == '\t'))
        name.pop_back();
    // Convert to lowercase
    for (auto& ch : name) {
        if (ch >= 'A' && ch <= 'Z') ch += 32;
    }

    if (name == "none") {
        r = g = b = a = 0;
        return true;
    }
    if (name == "black") {
        r = g = b = 0; a = 255;
        return true;
    }
    if (name == "white") {
        r = g = b = 255; a = 255;
        return true;
    }
    if (name == "red") {
        r = 255; g = b = 0; a = 255;
        return true;
    }
    if (name == "green") {
        r = 0; g = 128; b = 0; a = 255;
        return true;
    }
    if (name == "blue") {
        r = g = 0; b = 255; a = 255;
        return true;
    }
    if (name == "yellow") {
        r = g = 255; b = 0; a = 255;
        return true;
    }
    if (name == "cyan") {
        r = 0; g = b = 255; a = 255;
        return true;
    }
    if (name == "magenta") {
        r = b = 255; g = 0; a = 255;
        return true;
    }

    // Handle "grayNN" / "greyNN" (0-100, maps to 0-255)
    if (name.size() >= 5 && (name.compare(0, 4, "gray") == 0 || name.compare(0, 4, "grey") == 0)) {
        const char* numStr = name.c_str() + 4;
        char* end = nullptr;
        long val = std::strtol(numStr, &end, 10);
        if (end != numStr && *end == '\0' && val >= 0 && val <= 100) {
            r = g = b = static_cast<uint8_t>(val * 255 / 100);
            a = 255;
            return true;
        }
    }

    // Unknown named color — default to black
    r = g = b = 0;
    a = 255;
    return true;
}

xlImage::xlImage(const char* const* xpmData) {
    if (!xpmData || !xpmData[0]) return;

    // Parse header: "width height num_colors chars_per_pixel"
    int w = 0, h = 0, numColors = 0, cpp = 0;
    const char* header = xpmData[0];
    char* end = nullptr;

    w = static_cast<int>(std::strtol(header, &end, 10));
    if (end == header) return;
    h = static_cast<int>(std::strtol(end, &end, 10));
    numColors = static_cast<int>(std::strtol(end, &end, 10));
    cpp = static_cast<int>(std::strtol(end, &end, 10));

    if (w <= 0 || h <= 0 || numColors <= 0 || cpp <= 0 || cpp > 4) return;

    // Parse color table
    struct ColorEntry {
        uint8_t r, g, b, a;
    };
    std::unordered_map<std::string, ColorEntry> colorMap;
    colorMap.reserve(numColors);

    for (int i = 0; i < numColors; i++) {
        const char* line = xpmData[1 + i];
        if (!line) return;

        // First cpp characters are the color key
        std::string key(line, cpp);

        // Find the "c " color specifier (skip past any symbolic name / mono specs)
        const char* p = line + cpp;
        const char* colorVal = nullptr;
        while (*p) {
            // Skip whitespace
            while (*p == ' ' || *p == '\t') p++;
            if (*p == '\0') break;

            // Check for color type key: c, m, g, g4, s
            char type = *p;
            if ((type == 'c' || type == 'm' || type == 'g' || type == 's') &&
                (p[1] == ' ' || p[1] == '\t')) {
                p += 2;
                while (*p == ' ' || *p == '\t') p++;
                if (type == 'c') {
                    colorVal = p;
                    break;
                }
                // Skip past this value to look for 'c'
                while (*p && *p != '\t') {
                    if (*p == ' ') {
                        // Check if next non-space is a type key
                        const char* next = p + 1;
                        while (*next == ' ') next++;
                        if ((*next == 'c' || *next == 'm' || *next == 'g' || *next == 's') &&
                            (next[1] == ' ' || next[1] == '\t')) {
                            break;
                        }
                    }
                    p++;
                }
            } else if (type == 'g' && p[1] == '4' && (p[2] == ' ' || p[2] == '\t')) {
                p += 3;
                while (*p == ' ' || *p == '\t') p++;
                while (*p && *p != '\t') {
                    if (*p == ' ') {
                        const char* next = p + 1;
                        while (*next == ' ') next++;
                        if ((*next == 'c' || *next == 'm' || *next == 'g' || *next == 's') &&
                            (next[1] == ' ' || next[1] == '\t')) {
                            break;
                        }
                    }
                    p++;
                }
            } else {
                // Skip unknown token
                while (*p && *p != ' ' && *p != '\t') p++;
            }
        }

        ColorEntry entry{0, 0, 0, 255};
        if (colorVal) {
            parseXpmColor(colorVal, entry.r, entry.g, entry.b, entry.a);
        }
        colorMap[key] = entry;
    }

    // Allocate image
    _width = w;
    _height = h;
    _data.reset(new uint8_t[static_cast<size_t>(w) * h * 4]);

    // For cpp==1, build a flat lookup table indexed by character for fast pixel decoding
    ColorEntry flatLookup[256];
    if (cpp == 1) {
        for (int i = 0; i < 256; i++) {
            flatLookup[i] = {0, 0, 0, 255};
        }
        for (auto& [key, entry] : colorMap) {
            flatLookup[static_cast<unsigned char>(key[0])] = entry;
        }
    }

    // Parse pixel data
    size_t expectedRowLen = static_cast<size_t>(w) * cpp;
    for (int y = 0; y < h; y++) {
        const char* row = xpmData[1 + numColors + y];
        if (!row) {
            // Missing row — fill with transparent black
            std::memset(_data.get() + static_cast<size_t>(y) * w * 4, 0, static_cast<size_t>(w) * 4);
            continue;
        }
        size_t rowLen = std::strlen(row);
        if (rowLen < expectedRowLen) {
            // Short row — fill with transparent black
            std::memset(_data.get() + static_cast<size_t>(y) * w * 4, 0, static_cast<size_t>(w) * 4);
            continue;
        }
        if (cpp == 1) {
            for (int x = 0; x < w; x++) {
                const auto& ce = flatLookup[static_cast<unsigned char>(row[x])];
                int idx = (y * w + x) * 4;
                _data[idx] = ce.r;
                _data[idx + 1] = ce.g;
                _data[idx + 2] = ce.b;
                _data[idx + 3] = ce.a;
            }
        } else {
            for (int x = 0; x < w; x++) {
                std::string key(row + x * cpp, cpp);
                int idx = (y * w + x) * 4;
                auto it = colorMap.find(key);
                if (it != colorMap.end()) {
                    _data[idx] = it->second.r;
                    _data[idx + 1] = it->second.g;
                    _data[idx + 2] = it->second.b;
                    _data[idx + 3] = it->second.a;
                } else {
                    _data[idx] = 0;
                    _data[idx + 1] = 0;
                    _data[idx + 2] = 0;
                    _data[idx + 3] = 255;
                }
            }
        }
    }
}


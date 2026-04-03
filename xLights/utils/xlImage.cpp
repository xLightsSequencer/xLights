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


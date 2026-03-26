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
#include <fstream>

#include "../Parallel.h"

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
        std::memset(_data.get(), 0, _width * _height * 4);
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


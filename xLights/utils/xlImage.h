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

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>


class xlImage {
public:
    xlImage() = default;

    xlImage(int width, int height) : _width(width), _height(height) {
        if (width > 0 && height > 0) {
            _data.reset(new uint8_t[static_cast<size_t>(width) * height * 4]());
        }
    }

    // Takes ownership of rgbaData (must be allocated with new[])
    xlImage(int width, int height, uint8_t* rgbaData) : _width(width), _height(height), _data(rgbaData) {}

    xlImage(const xlImage& other) : _width(other._width), _height(other._height) {
        if (other._data && _width > 0 && _height > 0) {
            size_t sz = static_cast<size_t>(_width) * _height * 4;
            _data.reset(new uint8_t[sz]);
            std::memcpy(_data.get(), other._data.get(), sz);
        }
    }

    xlImage(xlImage&& other) noexcept : _data(std::move(other._data)), _width(other._width), _height(other._height) {
        other._width = 0;
        other._height = 0;
    }

    xlImage& operator=(const xlImage& other) {
        if (this != &other) {
            _width = other._width;
            _height = other._height;
            if (other._data && _width > 0 && _height > 0) {
                size_t sz = static_cast<size_t>(_width) * _height * 4;
                _data.reset(new uint8_t[sz]);
                std::memcpy(_data.get(), other._data.get(), sz);
            } else {
                _data.reset();
            }
        }
        return *this;
    }

    xlImage& operator=(xlImage&& other) noexcept {
        if (this != &other) {
            _data = std::move(other._data);
            _width = other._width;
            _height = other._height;
            other._width = 0;
            other._height = 0;
        }
        return *this;
    }

    ~xlImage() = default;

    // Loading
    bool LoadFromMemory(const uint8_t* data, size_t len);
    bool LoadFromFile(const std::string& filepath);

    // Saving
    bool SaveAsPNG(std::vector<uint8_t>& output) const;

    // Properties
    int GetWidth() const { return _width; }
    int GetHeight() const { return _height; }
    bool IsOk() const { return _data != nullptr && _width > 0 && _height > 0; }
    bool HasAlpha() const { return IsOk(); }

    // Pixel access
    uint8_t GetRed(int x, int y) const { return _data[(y * _width + x) * 4]; }
    uint8_t GetGreen(int x, int y) const { return _data[(y * _width + x) * 4 + 1]; }
    uint8_t GetBlue(int x, int y) const { return _data[(y * _width + x) * 4 + 2]; }
    uint8_t GetAlpha(int x, int y) const { return _data[(y * _width + x) * 4 + 3]; }

    bool IsTransparent(int x, int y, uint8_t threshold = 0) const {
        return GetAlpha(x, y) <= threshold;
    }

    void SetRGB(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
        int idx = (y * _width + x) * 4;
        _data[idx] = r;
        _data[idx + 1] = g;
        _data[idx + 2] = b;
    }

    void SetAlpha(int x, int y, uint8_t a) {
        _data[(y * _width + x) * 4 + 3] = a;
    }

    void SetRGBA(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        int idx = (y * _width + x) * 4;
        _data[idx] = r;
        _data[idx + 1] = g;
        _data[idx + 2] = b;
        _data[idx + 3] = a;
    }

    // Bulk data access — RGBA interleaved, 4 bytes per pixel
    const uint8_t* GetData() const { return _data.get(); }
    uint8_t* GetData() { return _data.get(); }

    // Operations
    void Rescale(int width, int height);
    xlImage Copy() const { return xlImage(*this); }
    void Clear();
    xlImage Mirror(bool horizontal) const;
    xlImage Rotate90(bool clockwise) const;
    xlImage Rotate180() const;
    xlImage ApplyOrientation(int orient) const;

    // Initialize to a blank image (all black, fully transparent)
    void Create(int width, int height) {
        _width = width;
        _height = height;
        if (width > 0 && height > 0) {
            _data.reset(new uint8_t[static_cast<size_t>(width) * height * 4]());
        } else {
            _data.reset();
        }
    }

private:
    std::unique_ptr<uint8_t[]> _data;
    int _width = 0;
    int _height = 0;
};

// Result from decoding an animated image (GIF, WebP, etc.)
struct AnimatedImageData {
    std::vector<xlImage> frames;           // composited frames (background filled with backgroundColor)
    std::vector<xlImage> framesNoBG;       // composited frames (background is transparent)
    std::vector<long> frameTimes;          // delay in ms per frame
    xlColor backgroundColor;               // background color from the file
    int width = 0;
    int height = 0;
};

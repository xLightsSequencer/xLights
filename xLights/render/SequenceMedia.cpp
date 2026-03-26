/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceMedia.h"
#include "pugixml.hpp"
#include "../utils/Base64.h"
#include "../utils/xlImage.h"
#include <cstring>
#include <filesystem>
#include <fstream>

#include <log.h>

#include "../UtilFunctions.h"
#include "../ui/wxUtilities.h"
#include "../ExternalHooks.h"

AnimationLoaderFunc ImageCacheEntry::_gifLoader;
AnimationLoaderFunc ImageCacheEntry::_webpLoader;

// ImageCacheEntry Implementation
ImageCacheEntry::ImageCacheEntry() : _used(false) {
    invalidImage = std::make_shared<xlImage>();
}

ImageCacheEntry::ImageCacheEntry(const std::string &filePath) : _used(false), _filePath(filePath) {
    invalidImage = std::make_shared<xlImage>();
    _isEmbedded = false;
}
ImageCacheEntry::ImageCacheEntry(const std::string &path, const std::string &base64Data): _used(false), _filePath(path), _embeddedData(base64Data) {
    invalidImage = std::make_shared<xlImage>();
    _isEmbedded = true;
}
ImageCacheEntry::ImageCacheEntry(const std::string &path, const std::vector<xlImage> &imgs, int ft, const std::string &base64Data): _used(false), _filePath(path), _embeddedData(base64Data) {
    invalidImage = std::make_shared<xlImage>();
    _isEmbedded = false;
    _frameBasedAnimation = true;
    _imageCount = (int)imgs.size();
    _frameImages.resize(_imageCount);
    _frameImagesNoBG.resize(_imageCount);
    _frameTimes.resize(_imageCount);
    _totalTime = 0;
    for (int x = 0; x < _imageCount; x++) {
        auto i = std::make_shared<xlImage>(imgs[x]);
        _frameImages[x] = i;
        _frameImagesNoBG[x] = i;
        _frameTimes[x] = ft;
        _totalTime += ft;
    }
    _imageWidth = imgs[0].GetWidth();
    _imageHeight = imgs[0].GetHeight();
    _loadingDone = true;
}
ImageCacheEntry::~ImageCacheEntry()
{
}
void ImageCacheEntry::Load() {
    std::scoped_lock lock(_cacheMutex);
    if (!_loadingDone) {
        if (_isEmbedded) {
            LoadFromData(_embeddedData);
        } else {
            LoadFromFile(_filePath);
        }
        _loadingDone = true;
    }
}
void ImageCacheEntry::LoadFromData(const std::string& data) {
    std::vector<uint8_t> buffer = Base64::Decode(data);
    if (buffer.size() >= 4 && buffer[0] == 'G' && buffer[1] == 'I' && buffer[2] == 'F') {
        loadAnimated(buffer, _gifLoader);
    } else if (buffer.size() >= 12 && buffer[0] == 'R' && buffer[1] == 'I' && buffer[2] == 'F' && buffer[3] == 'F'
               && buffer[8] == 'W' && buffer[9] == 'E' && buffer[10] == 'B' && buffer[11] == 'P') {
        loadAnimated(buffer, _webpLoader);
    } else {
        loadImage(buffer);
    }
}
void ImageCacheEntry::LoadFromFile(const std::string& filepath) {
    ObtainAccessToURL(filepath);
    FileExists(filepath, true);
    std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
    if (stream.is_open()) {
        auto size = stream.tellg();
        if (size <= 0) return;
        stream.seekg(0);
        std::vector<uint8_t> buffer(static_cast<size_t>(size));
        stream.read(reinterpret_cast<char*>(buffer.data()), size);
        if (!stream) return;
        _embeddedData = Base64::Encode(buffer.data(), buffer.size());
        LoadFromData(_embeddedData);
    }
}


int ImageCacheEntry::GetExifOrientation(const uint8_t* data, size_t maxLen) {

    if (maxLen < 2) return 1;
    unsigned char byte1 = data[0];
    unsigned char byte2 = data[1];
    if (byte1 != 0xFF || byte2 != 0xD8) {
        return 1;
    }

    size_t curPos = 2;
    while (curPos < maxLen) {
        byte1 = data[curPos++];
        if (byte1 != 0xFF) break;
        byte2 = data[curPos++];
        if (byte2 == 0xD9 || byte2 == 0xDA) break;

        unsigned short len = data[curPos++] << 8;
        len += data[curPos++];

        len = ((len >> 8) & 0xFF) | ((len << 8) & 0xFF00); // big-endian
        if (len < 2) break;

        if (byte2 == 0xE1) { // APP1 segment
            const char *ldata = reinterpret_cast<const char*>(&data[curPos]);
            int ldataSize = len - 2;

            if (ldataSize < 16) continue; // too small to hold Exif header

            if (memcmp(ldata, "Exif\0\0", 6) != 0) {
                continue;
            }

            size_t tiff_header = 6; // TIFF header starts right after Exif\0\0
            bool littleEndian = (ldata[tiff_header] == 'I' && ldata[tiff_header + 1] == 'I');
            unsigned short fortytwo = littleEndian ?
                ((unsigned char)ldata[tiff_header + 3] << 8) | (unsigned char)ldata[tiff_header + 2] :
                ((unsigned char)ldata[tiff_header + 2] << 8) | (unsigned char)ldata[tiff_header + 3];

            if (fortytwo != 42) {
                spdlog::debug("Invalid TIFF header identifier in {}", _filePath);
                return 1;
            }

            // Read offset to IFD0
            unsigned int ifd_offset;
            if (littleEndian) {
                ifd_offset =  (unsigned char)ldata[tiff_header + 4]       |
                    ((unsigned char)ldata[tiff_header + 5] << 8) |
                    ((unsigned char)ldata[tiff_header + 6] << 16)|
                    ((unsigned char)ldata[tiff_header + 7] << 24);
            } else {
                ifd_offset = ((unsigned char)ldata[tiff_header + 4] << 24)|
                    ((unsigned char)ldata[tiff_header + 5] << 16)|
                    ((unsigned char)ldata[tiff_header + 6] << 8) |
                    (unsigned char)ldata[tiff_header + 7];
            }

            size_t pos = tiff_header + ifd_offset;
            if (pos + 2 > (size_t)ldataSize) return 1;

            unsigned short num_entries = littleEndian ?
                ((unsigned char)ldata[pos + 1] << 8) | (unsigned char)ldata[pos] :
                ((unsigned char)ldata[pos] << 8) | (unsigned char)ldata[pos + 1];
            pos += 2;

            for (unsigned short i = 0; i < num_entries; ++i) {
                if (pos + 12 > (size_t)ldataSize) break;

                unsigned short tag = littleEndian ?
                    ((unsigned char)ldata[pos + 1] << 8) | (unsigned char)ldata[pos] :
                    ((unsigned char)ldata[pos] << 8) | (unsigned char)ldata[pos + 1];

                if (tag == 0x0112) { // Orientation
                    unsigned short orient = littleEndian ?
                        ((unsigned char)ldata[pos + 9] << 8) | (unsigned char)ldata[pos + 8] :
                        ((unsigned char)ldata[pos + 8] << 8) | (unsigned char)ldata[pos + 9];
                    return static_cast<int>(orient);
                }
                pos += 12;
            }
        } else {
            curPos += len - 2;
        }
    }

    return 1; // default
}

void ImageCacheEntry::loadAnimated(const std::vector<uint8_t> &data, const AnimationLoaderFunc &loader) {
    if (!loader) {
        spdlog::warn("Animation loader not registered, cannot load: {}", _filePath);
        return;
    }
    auto result = loader(data.data(), data.size(), _filePath);
    if (result.frames.empty()) return;

    _imageCount = (int)result.frames.size();
    _frameTimes = std::move(result.frameTimes);
    _imageWidth = result.width;
    _imageHeight = result.height;

    _totalTime = 0;
    for (int x = 0; x < _imageCount; x++) {
        _frameImages.push_back(std::make_shared<xlImage>(std::move(result.frames[x])));
        _totalTime += _frameTimes[x];
    }

    // Use pre-composited background-suppressed frames if available
    if (!result.framesNoBG.empty()) {
        for (int x = 0; x < _imageCount; x++) {
            _frameImagesNoBG.push_back(std::make_shared<xlImage>(std::move(result.framesNoBG[x])));
        }
    } else {
        _frameImagesNoBG = _frameImages;
    }

    if (_imageCount == 1) {
        _totalTime = 0;
        _frameTimes[0] = 0;
    }
    _frameBasedAnimation = _imageCount <= 1;
}

void ImageCacheEntry::loadImage(const std::vector<uint8_t> &data) {
    auto i = std::make_shared<xlImage>();
    if (!i->LoadFromMemory(data.data(), data.size())) {
        spdlog::error("Error loading image file: {}.", _filePath);
        i = std::make_shared<xlImage>(5, 5);
    }
    _imageCount = 1;
    int orientation = GetExifOrientation(data.data(), data.size());
    *i = ApplyOrientation(*i, orientation);
    _frameTimes.push_back(0);
    _frameImages.emplace_back(i);
    _imageWidth = i->GetWidth();
    _imageHeight = i->GetHeight();
}


static std::string PngToBase64(const xlImage& img)
{
    std::vector<uint8_t> pngData;
    img.SaveAsPNG(pngData);
    return Base64::Encode(pngData.data(), pngData.size());
}

// --- pugixml implementations ---

bool ImageCacheEntry::LoadFromXml(const pugi::xml_node& node)
{
    if (!node || strcmp(node.name(), "Image") != 0) {
        return false;
    }

    _filePath = node.attribute("path").as_string("");
    auto child = node.first_child();
    if (!child) return true;

    std::string childName = child.name();
    if (childName == "Data") {
        _embeddedData = child.text().as_string("");
        _isEmbedded = true;
    } else if (childName == "Frame") {
        // Multi-frame: each <Frame time="ms">base64png</Frame>
        _isEmbedded = true;
        for (auto f = child; f; f = f.next_sibling()) {
            if (strcmp(f.name(), "Frame") != 0) continue;
            long ft = f.attribute("time").as_int(0);
            std::string b64 = f.text().as_string("");
            _frameData.push_back(b64);
            std::vector<uint8_t> buf = Base64::Decode(b64);
            auto sp = std::make_shared<xlImage>();
            sp->LoadFromMemory(buf.data(), buf.size());
            _frameImages.push_back(sp);
            _frameImagesNoBG.push_back(sp);
            _frameTimes.push_back(ft);
            _totalTime += ft;
        }
        _frameBasedAnimation = true;
        _imageCount = (int)_frameImages.size();
        if (_imageCount > 0) {
            _imageWidth = _frameImages[0]->GetWidth();
            _imageHeight = _frameImages[0]->GetHeight();
        }
        _loadingDone = true;
    }
    return true;
}

void ImageCacheEntry::SaveToXml(pugi::xml_node& parent) const
{
    auto node = parent.append_child("Image");
    node.append_attribute("path") = _filePath;

    if (!_isEmbedded) return;

    if (!_embeddedData.empty()) {
        auto dataNode = node.append_child("Data");
        dataNode.text().set(_embeddedData);
    } else if (_imageCount > 0 && !_frameImages.empty()) {
        for (int i = 0; i < _imageCount; i++) {
            std::string b64;
            if (i < (int)_frameData.size()) {
                b64 = _frameData[i];
            } else if (_frameImages[i] && _frameImages[i]->IsOk()) {
                b64 = PngToBase64(*_frameImages[i]);
                if ((int)_frameData.size() == i) _frameData.push_back(b64);
            }
            if (!b64.empty()) {
                auto fNode = node.append_child("Frame");
                fNode.append_attribute("time") = (int)_frameTimes[i];
                fNode.text().set(b64);
            }
        }
    }
}

std::shared_ptr<xlImage> ImageCacheEntry::GetFrame(int x, bool suppressGIFBackground) {
    if (x >= (int)_frameImages.size()) {
        return invalidImage;
    }
    if (suppressGIFBackground && !_frameImagesNoBG.empty()) {
        return _frameImagesNoBG[x];
    }
    return _frameImages[x];
}

int ImageCacheEntry::GetFrameForTime(int msec, bool loop) {
    if (_totalTime == 0) {
        return 0;
    }

    if (loop) {
        while (msec >= _totalTime) {
            msec -= _totalTime;
        }
    }

    if (msec > _totalTime) {
        return _frameTimes.size();
    }

    int frame = 0;
    for (auto it = _frameTimes.begin(); it != _frameTimes.end(); ++it) {
        if (msec < *it) {
            return frame;
        }
        msec -= *it;
        frame++;
    }
    return frame - 1;
}

std::shared_ptr<xlImage> ImageCacheEntry::GetScaledImage(int frameNumber, int width, int height, bool suppressedBg) {
    ScaledImageCacheKey key;
    key.frameNumber = frameNumber;
    key.width = width;
    key.height = height;
    key.suppressGIFBackground = suppressedBg;

    std::scoped_lock lock(_cacheMutex);
    auto it = _scaledImageCache.find(key);
    if (it != _scaledImageCache.end()) {
        return it->second;
    }

    std::shared_ptr<xlImage> img = GetFrame(frameNumber, suppressedBg);
    if (!img->IsOk()) {
        return img;
    }
    xlImage *image = new xlImage(*img);
    image->Rescale(width, height);
    _scaledImageCache.emplace(key, image);
    return _scaledImageCache[key];
}

void ImageCacheEntry::ClearScaledImageCache() {
    std::scoped_lock lock(_cacheMutex);
    _scaledImageCache.clear();
}


// SequenceMedia Implementation

SequenceMedia::SequenceMedia()
{
}

SequenceMedia::~SequenceMedia()
{
    Clear();
}

std::shared_ptr<ImageCacheEntry> SequenceMedia::GetImage(const std::string& filepath)
{
    std::unique_lock lock(_cacheMutex);
    // Check if image is already cached
    auto it = _imageCache.find(filepath);
    if (it != _imageCache.end()) {
        auto ret = it->second;
        if (!ret->isLoaded()) {
            lock.unlock();
            ret->Load();
        }
        return ret;
    }

    // For relative paths, resolve to an absolute path using FixFile so the
    // entry can be loaded from disk.  The cache key stays as the relative path.
    std::string loadPath = filepath;
    if (!std::filesystem::path(filepath).is_absolute()) {
        std::string resolved = FixFile("", filepath).ToStdString();
        if (!resolved.empty())
            loadPath = resolved;
    }

    //wasn't found, we'll create it and add to the cache
    std::shared_ptr<ImageCacheEntry> np = std::make_shared<ImageCacheEntry>(loadPath);
    _imageCache.emplace(filepath, np);
    lock.unlock();

    np->Load();
    return np;
}

bool SequenceMedia::HasImage(const std::string& filepath) const
{
    std::scoped_lock lock(_cacheMutex);
    return _imageCache.find(filepath) != _imageCache.end();
}

void SequenceMedia::RemoveImage(const std::string& filepath)
{
    std::scoped_lock lock(_cacheMutex);
    _imageCache.erase(filepath);
}

void SequenceMedia::AddAnimatedImage(const std::string& filepath, int msFrameTime) {
    // Resolve relative paths the same way GetImage does, so FileExists and
    // LoadFile operate on a valid absolute path.
    std::string loadPath = filepath;
    if (!std::filesystem::path(filepath).is_absolute()) {
        std::string resolved = FixFile("", filepath).ToStdString();
        if (!resolved.empty()) {
            loadPath = resolved;
        }
    }
    std::filesystem::path loadFsPath(loadPath);
    std::string extension = loadFsPath.extension().string();
    std::string stemStr = loadFsPath.stem().string();
    std::string BasePicture = (loadFsPath.parent_path() / "").string() + stemStr.substr(0, stemStr.length() - 2) + "-";
    int cur = 1;
    std::string fname = BasePicture + std::to_string(cur++) + extension;
    std::vector<xlImage> images;
    while (FileExists(fname)) {
        xlImage i;
        i.LoadFromFile(fname);
        images.push_back(std::move(i));
        fname = BasePicture + std::to_string(cur++) + extension;
    }

    {
        std::unique_lock lock(_cacheMutex);
        if (_imageCache.find(filepath) == _imageCache.end()) {
            auto np = std::make_shared<ImageCacheEntry>(filepath, images, msFrameTime);
            np->EmbedImage();
            _imageCache.emplace(filepath, np);
        }
    }
}


void SequenceMedia::Clear()
{
    std::scoped_lock lock(_cacheMutex);
    _imageCache.clear();
}

void SequenceMedia::EmbedImage(const std::string& filepath)
{
    std::scoped_lock lock(_cacheMutex);
    auto it = _imageCache.find(filepath);
    if (it != _imageCache.end()) {
        it->second->EmbedImage();
    }
}

void SequenceMedia::EmbedAllImages()
{
    std::scoped_lock lock(_cacheMutex);
    for (auto& pair : _imageCache) {
        pair.second->EmbedImage();
    }
}
void SequenceMedia::AddEmbeddedImage(const std::string& name, const std::string& imageData) {
    std::shared_ptr<ImageCacheEntry> entry;
    {
        std::scoped_lock lock(_cacheMutex);
        if (_imageCache.find(name) != _imageCache.end()) return; // already exists
        entry = std::make_shared<ImageCacheEntry>(name, imageData);
        _imageCache.emplace(name, entry);
    }
    entry->Load();
}

void SequenceMedia::AddEmbeddedImage(const std::string& name, const xlImage& image)
{
    // Encode the xlImage as PNG, then base64-encode it
    std::vector<uint8_t> pngData;
    if (!image.SaveAsPNG(pngData)) {
        return;
    }
    std::string b64 = Base64::Encode(pngData.data(), pngData.size());

    std::shared_ptr<ImageCacheEntry> entry;
    {
        std::scoped_lock lock(_cacheMutex);
        if (_imageCache.find(name) != _imageCache.end()) return; // already exists
        entry = std::make_shared<ImageCacheEntry>(name, b64);
        _imageCache.emplace(name, entry);
    }
    entry->Load();
}

void SequenceMedia::AddEmbeddedImage(const std::string& name, const std::vector<xlImage>& frames, int frameTimeMs)
{
    if (frames.empty()) return;
    std::scoped_lock lock(_cacheMutex);
    if (_imageCache.find(name) != _imageCache.end()) return;
    // Store frames directly as PNGs — no GIF quantization to preserve colour/transparency.
    // Pre-encode base64 now so SaveToXml never re-encodes from pixels.
    auto entry = std::make_shared<ImageCacheEntry>(name, frames, frameTimeMs);
    entry->EmbedImage();
    std::vector<std::string> frameData;
    frameData.reserve(frames.size());
    for (const auto& f : frames)
        frameData.push_back(PngToBase64(f));
    entry->SetFrameData(std::move(frameData));
    _imageCache.emplace(name, entry);
}

void SequenceMedia::ExtractImage(const std::string& filepath)
{
    std::scoped_lock lock(_cacheMutex);
    auto it = _imageCache.find(filepath);
    if (it != _imageCache.end()) {
        it->second->ExtractImage();
    }
}

void SequenceMedia::ExtractAllImages()
{
    std::scoped_lock lock(_cacheMutex);
    for (auto& pair : _imageCache) {
        pair.second->ExtractImage();
    }
}

bool ImageCacheEntry::SaveToFile(const std::string& path) const
{
    if (_embeddedData.empty()) return false;
    ObtainAccessToURL(path);
    std::vector<uint8_t> buf = Base64::Decode(_embeddedData);
    if (buf.empty()) return false;
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) return false;
    f.write(reinterpret_cast<const char*>(buf.data()), buf.size());
    return f.good();
}

bool SequenceMedia::ExtractImageToFile(const std::string& oldPath, const std::string& newPath)
{
    std::shared_ptr<ImageCacheEntry> entry;
    {
        std::scoped_lock lock(_cacheMutex);
        auto it = _imageCache.find(oldPath);
        if (it == _imageCache.end()) return false;
        entry = it->second;
    }
    ObtainAccessToURL(newPath);
    if (!entry->SaveToFile(newPath)) return false;
    RenameImage(oldPath, newPath);
    ExtractImage(newPath);
    return true;
}

bool SequenceMedia::RenameImage(const std::string& oldPath, const std::string& newPath)
{
    if (oldPath == newPath) return true;
    std::scoped_lock lock(_cacheMutex);
    auto it = _imageCache.find(oldPath);
    if (it == _imageCache.end()) return false;
    if (_imageCache.find(newPath) != _imageCache.end()) return false;
    auto entry = it->second;
    entry->SetFilePath(newPath);
    _imageCache.erase(it);
    _imageCache.emplace(newPath, entry);
    return true;
}

// --- pugixml implementations ---

bool SequenceMedia::LoadFromXml(const pugi::xml_node& node)
{

    if (!node || strcmp(node.name(), "SequenceMedia") != 0) {
        return false;
    }

    std::scoped_lock lock(_cacheMutex);
    _imageCache.clear();

    for (auto imageNode : node.children("Image")) {
        auto entry = std::make_shared<ImageCacheEntry>();
        if (entry->LoadFromXml(imageNode)) {
            _imageCache[entry->GetFilePath()] = entry;
        } else {
            spdlog::warn("Failed to load image entry from XML");
        }
    }

    return true;
}

void SequenceMedia::SaveToXml(pugi::xml_node& parent) const
{
    std::scoped_lock lock(_cacheMutex);

    auto node = parent.append_child("SequenceMedia");
    for (const auto& pair : _imageCache) {
        if (pair.second->IsEmbedded()) {
            pair.second->SaveToXml(node);
        }
    }
}

std::vector<std::string> SequenceMedia::GetImagePaths() const
{
    std::scoped_lock lock(_cacheMutex);
    std::vector<std::string> paths;
    for (const auto& pair : _imageCache) {
        paths.push_back(pair.first);
    }
    return paths;
}

void SequenceMedia::RemoveUnusedImages()
{

    std::scoped_lock lock(_cacheMutex);

    // Remove unused static images
    std::vector<std::string> toRemove;

    for (const auto& pair : _imageCache) {
        if (!pair.second->IsUsed()) {
            toRemove.push_back(pair.first);
        }
    }

    for (const auto& path : toRemove) {
        spdlog::debug("Removing unused image from cache: {}", path);
        _imageCache.erase(path);
    }
}

void SequenceMedia::MarkAllUnused() {
    std::scoped_lock lock(_cacheMutex);
    for (const auto& pair : _imageCache) {
        pair.second->MarkIsUsed(false);
    }
}

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
#include <limits>

#include <log.h>

#include "../utils/FileUtils.h"
#include "utils/ExternalHooks.h"
#include "../utils/nanosvg_xl.h"
#include "../effects/ShaderEffect.h"
#include "../media/VideoReader.h"
#include "Element.h"
#include "EffectLayer.h"
#include "SequenceElements.h"

// =====================================================================
// MediaCacheEntry (base class) Implementation
// =====================================================================

MediaCacheEntry::MediaCacheEntry(MediaType type) : _type(type), _used(false) {}
MediaCacheEntry::MediaCacheEntry(MediaType type, const std::string& filePath)
    : _type(type), _filePath(filePath), _used(false) {}
MediaCacheEntry::MediaCacheEntry(MediaType type, const std::string& path, const std::string& base64Data)
    : _type(type), _filePath(path), _embeddedData(base64Data), _used(false) {
    _isEmbedded.store(true);
}
MediaCacheEntry::~MediaCacheEntry() {}

void MediaCacheEntry::LoadRawFromFile(const std::string& filepath) {
    bool accessOK = ObtainAccessToURL(filepath);
    bool exists = FileExists(filepath, true);
    std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
    if (stream.is_open()) {
        auto size = stream.tellg();
        if (size <= 0) {
            spdlog::warn("MediaCacheEntry::LoadRawFromFile: 0-byte file '{}'", filepath);
            return;
        }
        stream.seekg(0);
        std::vector<uint8_t> buffer(static_cast<size_t>(size));
        stream.read(reinterpret_cast<char*>(buffer.data()), size);
        if (!stream) {
            spdlog::warn("MediaCacheEntry::LoadRawFromFile: read failed for '{}'", filepath);
            return;
        }
        _embeddedData = Base64::Encode(buffer.data(), buffer.size());
        RecordFileTimestamp();
    } else {
        // Surface the failure so sandbox / path-resolution regressions
        // are visible in the log instead of showing up only as the
        // red-pixel fallback from PicturesEffect. Logs the flags we
        // care about so we can tell whether the problem was access
        // (sandbox), existence (path wrong), or something else.
        spdlog::warn("MediaCacheEntry::LoadRawFromFile: could not open '{}' "
                     "(ObtainAccessToURL={}, FileExists={}, errno={})",
                     filepath, accessOK, exists, errno);
    }
}

// Preview frame cache base implementations
void MediaCacheEntry::GeneratePreview(int maxWidth, int maxHeight) {
    // Default: no preview (text, binary, etc.)
}

std::shared_ptr<xlImage> MediaCacheEntry::GetPreviewFrame(size_t index) const {
    std::scoped_lock lock(_cacheMutex);
    if (index < _previewFrames.size()) return _previewFrames[index];
    return nullptr;
}

long MediaCacheEntry::GetPreviewFrameTime(size_t index) const {
    std::scoped_lock lock(_cacheMutex);
    if (index < _previewFrameTimes.size()) return _previewFrameTimes[index];
    return 50; // default 50ms
}

void MediaCacheEntry::ClearPreview() {
    std::scoped_lock lock(_cacheMutex);
    _previewFrames.clear();
    _previewFrameTimes.clear();
    _previewWidth = 0;
    _previewHeight = 0;
}

void MediaCacheEntry::SetPreviewFrames(std::vector<std::shared_ptr<xlImage>>&& frames, int frameTimeMs) {
    std::scoped_lock lock(_cacheMutex);
    if (_previewFrames.empty()) {
        _previewFrames = std::move(frames);
        for (size_t i = 0; i < _previewFrames.size(); i++) {
            _previewFrameTimes.push_back(frameTimeMs);
        }
        if (!_previewFrames.empty() && _previewFrames[0]) {
            _previewWidth = _previewFrames[0]->GetWidth();
            _previewHeight = _previewFrames[0]->GetHeight();
        }
    }
}

bool MediaCacheEntry::SaveToFile(const std::string& path) const {
    if (_embeddedData.empty()) return false;
    ObtainAccessToURL(path, true);
    std::vector<uint8_t> buf = Base64::Decode(_embeddedData);
    if (buf.empty()) return false;
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) return false;
    f.write(reinterpret_cast<const char*>(buf.data()), buf.size());
    return f.good();
}

void MediaCacheEntry::RecordFileTimestamp() {
    if (!_filePath.empty()) {
        std::error_code ec;
        _fileTimestamp = std::filesystem::last_write_time(_filePath, ec);
        // On error, _fileTimestamp stays default — HasFileChanged() will return false
    }
}

bool MediaCacheEntry::HasFileChanged() const {
    if (_isEmbedded.load() || _filePath.empty() || _fileTimestamp == std::filesystem::file_time_type{}) {
        return false;
    }
    std::error_code ec;
    auto current = std::filesystem::last_write_time(_filePath, ec);
    if (ec) return false;
    return current != _fileTimestamp;
}

void MediaCacheEntry::ReloadIfChanged() {
    if (!_loadingDone.load() || _isEmbedded.load()) return;
    if (!HasFileChanged()) return;
    std::scoped_lock lock(_cacheMutex);
    // Double-check under lock
    if (!HasFileChanged()) return;
    spdlog::debug("MediaCacheEntry::ReloadIfChanged - file changed on disk: '{}'", _filePath);
    _loadingDone = false;
    _embeddedData.clear();
    ClearPreview();
    Load();
}

// =====================================================================
// ImageCacheEntry Implementation
// =====================================================================

AnimationLoaderFunc ImageCacheEntry::_webpLoader;

ImageCacheEntry::ImageCacheEntry() : MediaCacheEntry(MediaType::Image) {
    invalidImage = std::make_shared<xlImage>();
}

ImageCacheEntry::ImageCacheEntry(const std::string &filePath) : MediaCacheEntry(MediaType::Image, filePath) {
    invalidImage = std::make_shared<xlImage>();
}
ImageCacheEntry::ImageCacheEntry(const std::string &path, const std::string &base64Data) : MediaCacheEntry(MediaType::Image, path, base64Data) {
    invalidImage = std::make_shared<xlImage>();
}
ImageCacheEntry::ImageCacheEntry(const std::string &path, const std::vector<xlImage> &imgs, int ft, const std::string &base64Data) : MediaCacheEntry(MediaType::Image, path) {
    _embeddedData = base64Data;
    invalidImage = std::make_shared<xlImage>();
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
    if (!imgs.empty()) {
        _imageWidth = imgs[0].GetWidth();
        _imageHeight = imgs[0].GetHeight();
    }
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
void ImageCacheEntry::ReloadIfChanged() {
    if (!_loadingDone.load() || _isEmbedded.load()) return;
    if (!HasFileChanged()) return;
    std::scoped_lock lock(_cacheMutex);
    if (!HasFileChanged()) return;
    spdlog::debug("ImageCacheEntry::ReloadIfChanged - file changed on disk: '{}'", _filePath);
    _loadingDone = false;
    _embeddedData.clear();
    _frameImages.clear();
    _frameImagesNoBG.clear();
    _frameTimes.clear();
    _frameData.clear();
    _scaledImageCache.clear();
    _imageCount = 0;
    _imageWidth = 0;
    _imageHeight = 0;
    _totalTime = 0;
    ClearPreview();
    Load();
}
void ImageCacheEntry::LoadFromData(const std::string& data) {
    std::vector<uint8_t> buffer = Base64::Decode(data);
    if (buffer.size() >= 4 && buffer[0] == 'G' && buffer[1] == 'I' && buffer[2] == 'F') {
        storeAnimated(LoadAnimatedGIFFromMemory(buffer.data(), buffer.size()));
    } else if (buffer.size() >= 12 && buffer[0] == 'R' && buffer[1] == 'I' && buffer[2] == 'F' && buffer[3] == 'F'
               && buffer[8] == 'W' && buffer[9] == 'E' && buffer[10] == 'B' && buffer[11] == 'P') {
        loadAnimated(buffer, _webpLoader);
    } else {
        loadImage(buffer);
    }
}
void ImageCacheEntry::LoadFromFile(const std::string& filepath) {
    LoadRawFromFile(filepath);
    if (!_embeddedData.empty()) {
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
    while (curPos + 1 < maxLen) {
        byte1 = data[curPos++];
        if (byte1 != 0xFF) break;
        byte2 = data[curPos++];
        if (byte2 == 0xD9 || byte2 == 0xDA) break;

        if (curPos + 1 >= maxLen) break;
        unsigned short len = (data[curPos] << 8) | data[curPos + 1];
        curPos += 2;
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

void ImageCacheEntry::storeAnimated(AnimatedImageData result) {
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

void ImageCacheEntry::loadAnimated(const std::vector<uint8_t> &data, const AnimationLoaderFunc &loader) {
    if (!loader) {
        spdlog::warn("Animation loader not registered, cannot load: {}", _filePath);
        return;
    }
    storeAnimated(loader(data.data(), data.size(), _filePath));
}

void ImageCacheEntry::loadImage(const std::vector<uint8_t> &data) {
    auto i = std::make_shared<xlImage>();
    if (!i->LoadFromMemory(data.data(), data.size())) {
        spdlog::error("Error loading image file: {}.", _filePath);
        i = std::make_shared<xlImage>(5, 5);
    }
    _imageCount = 1;
    int orientation = GetExifOrientation(data.data(), data.size());
    *i = i->ApplyOrientation(orientation);
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
    if (x < 0 || x >= (int)_frameImages.size()) {
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
        return std::max(0, (int)_frameTimes.size() - 1);
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

void ImageCacheEntry::GeneratePreview(int maxWidth, int maxHeight) {
    std::scoped_lock lock(_cacheMutex);
    if (_previewWidth == maxWidth && _previewHeight == maxHeight && !_previewFrames.empty()) return;

    _previewFrames.clear();
    _previewFrameTimes.clear();

    if (_frameImages.empty() || _imageWidth <= 0 || _imageHeight <= 0) return;

    // Calculate scaled size maintaining aspect ratio
    double scale = std::min((double)maxWidth / _imageWidth, (double)maxHeight / _imageHeight);
    int sw = std::max(1, (int)(_imageWidth * scale));
    int sh = std::max(1, (int)(_imageHeight * scale));

    for (int i = 0; i < _imageCount; i++) {
        auto scaled = GetScaledImage(i, sw, sh, false);
        _previewFrames.push_back(scaled);
        _previewFrameTimes.push_back(i < (int)_frameTimes.size() ? _frameTimes[i] : 50);
    }
    _previewWidth = maxWidth;
    _previewHeight = maxHeight;
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
    if (filepath.empty()) return nullptr;
    std::unique_lock lock(_cacheMutex);
    // Check if image is already cached
    auto it = _imageCache.find(filepath);
    if (it != _imageCache.end()) {
        auto ret = it->second;
        if (!ret->isLoaded()) {
            lock.unlock();
            ret->Load();
        }
        ret->ReloadIfChanged();
        return ret;
    }

    // Resolve to an absolute (and existing) path using FileUtils::FixFile so
    // the entry can be loaded from disk. Absolute paths go through FixFile too
    // -- sequences moved between machines keep their saved absolute paths, and
    // FixFile re-resolves them against the current show/media folders. The
    // cache key stays as the original path.
    std::string loadPath = ResolvePath(filepath);
    // Check if the resolved path matches an existing entry
    for (auto& [key, entry] : _imageCache) {
        if (entry->GetFilePath() == loadPath || ResolvePath(key) == loadPath) {
            if (!entry->isLoaded()) {
                lock.unlock();
                entry->Load();
            }
            entry->ReloadIfChanged();
            return entry;
        }
    }
    std::shared_ptr<ImageCacheEntry> np = std::make_shared<ImageCacheEntry>(loadPath);
    _imageCache.emplace(filepath, np);
    lock.unlock();

    np->Load();
    // Surface resolution failures in the log — the pictures effect
    // otherwise just shows a red frame with no indication why.
    // Desktop historically relied on the file-picker + user vetting,
    // so this path is quiet; on iPad the path can mis-resolve
    // (sandbox, missing bookmark, desktop-saved absolute path with no
    // matching file in the show folder) and we need visibility.
    if (!np->IsOk()) {
        spdlog::warn("SequenceMedia::GetImage: not OK after load. "
                     "requested='{}' resolved='{}'",
                     filepath, loadPath);
    }
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
    // Resolve paths the same way GetImage does, so FileExists and LoadFile
    // operate on a valid absolute path.
    std::string loadPath = ResolvePath(filepath);
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

    if (!images.empty()) {
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
    _textCache.clear();
    _svgCache.clear();
    _shaderCache.clear();
    _binaryCache.clear();
    _videoCache.clear();
    _audioCache.clear();
}

void SequenceMedia::PurgePreviewCaches()
{
    std::scoped_lock lock(_cacheMutex);
    // Every MediaCacheEntry holds `_previewFrames` — the scaled
    // thumbnails built by `GeneratePreview` for the media picker
    // and effect panels. Those dominate the UI-side memory; drop
    // them first. On rebuild, the media picker re-requests with
    // the current layout bounds.
    for (auto& [path, entry] : _imageCache) {
        if (entry) {
            entry->ClearPreview();
            entry->ClearScaledImageCache();
        }
    }
    for (auto& [path, entry] : _textCache) {
        if (entry) entry->ClearPreview();
    }
    for (auto& [path, entry] : _svgCache) {
        if (entry) entry->ClearPreview();
    }
    for (auto& [path, entry] : _shaderCache) {
        if (entry) entry->ClearPreview();
    }
    for (auto& [path, entry] : _binaryCache) {
        if (entry) entry->ClearPreview();
    }
    for (auto& [path, entry] : _videoCache) {
        if (entry) entry->ClearPreview();
    }
}

void SequenceMedia::EmbedImage(const std::string& filepath)
{
    std::scoped_lock lock(_cacheMutex);
    auto it = _imageCache.find(filepath);
    if (it != _imageCache.end()) {
        it->second->EmbedImage();
        it->second->SetFilePath(filepath);
    }
}

void SequenceMedia::EmbedAllImages()
{
    std::scoped_lock lock(_cacheMutex);
    for (auto& pair : _imageCache) {
        pair.second->EmbedImage();
        pair.second->SetFilePath(pair.first);
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

bool SequenceMedia::RenameMedia(const std::string& oldPath, const std::string& newPath)
{
    if (oldPath == newPath) return true;
    std::scoped_lock lock(_cacheMutex);

    // Generic re-key across every cache. Mirrors RenameImage's
    // shape — keep the entry alive, swap the map key, update the
    // entry's stored file path. Same collision check as the
    // image-specific version: refuse if newPath already has an
    // entry of any type. That avoids merging two distinct entries
    // into one.
    auto rekey = [&](auto& cache) -> int {
        auto it = cache.find(oldPath);
        if (it == cache.end()) return 0; // not in this cache
        auto entry = it->second;
        entry->SetFilePath(newPath);
        cache.erase(it);
        cache.emplace(newPath, entry);
        return 1;
    };

    if (HasMedia(newPath)) return false;

    // Try each cache; first hit wins. HasMedia above guarantees
    // newPath collides in none of them, so we don't need to
    // double-check per cache.
    if (rekey(_imageCache))  return true;
    if (rekey(_svgCache))    return true;
    if (rekey(_shaderCache)) return true;
    if (rekey(_textCache))   return true;
    if (rekey(_binaryCache)) return true;
    if (rekey(_videoCache))  return true;
    if (rekey(_audioCache))  return true;
    return false;
}

// --- pugixml implementations ---

bool SequenceMedia::LoadFromXml(const pugi::xml_node& node)
{
    if (!node || strcmp(node.name(), "SequenceMedia") != 0) {
        return false;
    }

    std::scoped_lock lock(_cacheMutex);
    _imageCache.clear();
    _textCache.clear();
    _svgCache.clear();
    _shaderCache.clear();
    _binaryCache.clear();
    _videoCache.clear();
    _audioCache.clear();

    for (auto child : node.children()) {
        std::string name = child.name();
        if (name == "Image") {
            auto entry = std::make_shared<ImageCacheEntry>();
            if (entry->LoadFromXml(child)) {
                _imageCache[entry->GetFilePath()] = entry;
            } else {
                spdlog::warn("Failed to load image entry from XML");
            }
        } else if (name == "TextFile") {
            auto entry = std::make_shared<TextMediaCacheEntry>();
            if (entry->LoadFromXml(child)) {
                _textCache[entry->GetFilePath()] = entry;
            }
        } else if (name == "SVG") {
            auto entry = std::make_shared<SVGMediaCacheEntry>();
            if (entry->LoadFromXml(child)) {
                _svgCache[entry->GetFilePath()] = entry;
            }
        } else if (name == "Shader") {
            auto entry = std::make_shared<ShaderMediaCacheEntry>();
            if (entry->LoadFromXml(child)) {
                _shaderCache[entry->GetFilePath()] = entry;
            }
        } else if (name == "BinaryFile") {
            auto entry = std::make_shared<BinaryMediaCacheEntry>();
            if (entry->LoadFromXml(child)) {
                _binaryCache[entry->GetFilePath()] = entry;
            }
        } else if (name == "Video") {
            auto entry = std::make_shared<VideoMediaCacheEntry>();
            if (entry->LoadFromXml(child)) {
                _videoCache[entry->GetFilePath()] = entry;
            }
        } else if (name == "Audio") {
            auto entry = std::make_shared<AudioMediaCacheEntry>();
            if (entry->LoadFromXml(child)) {
                _audioCache[entry->GetFilePath()] = entry;
            }
        }
    }

    return true;
}

void SequenceMedia::SaveToXml(pugi::xml_node& parent) const
{
    std::scoped_lock lock(_cacheMutex);

    auto node = parent.append_child("SequenceMedia");

    // Save all embedded entries across all types
    auto saveEmbedded = [&node](const auto& cache) {
        for (const auto& pair : cache) {
            if (pair.second->IsEmbedded()) {
                pair.second->SaveToXml(node);
            }
        }
    };
    saveEmbedded(_imageCache);
    saveEmbedded(_textCache);
    saveEmbedded(_svgCache);
    saveEmbedded(_shaderCache);
    saveEmbedded(_binaryCache);

    // Videos are path-only (not embedded) — only save used entries
    for (const auto& pair : _videoCache) {
        if (pair.second->IsUsed()) {
            pair.second->SaveToXml(node);
        }
    }

    // Audio files are path-only — only save used entries
    for (const auto& pair : _audioCache) {
        if (pair.second->IsUsed()) {
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

std::vector<std::string> SequenceMedia::GetShaderPaths() const
{
    std::scoped_lock lock(_cacheMutex);
    std::vector<std::string> paths;
    for (const auto& pair : _shaderCache) {
        paths.push_back(pair.first);
    }
    return paths;
}

std::vector<std::pair<std::string, std::string>> SequenceMedia::GetImageRelocations() const
{
    std::scoped_lock lock(_cacheMutex);
    return _pendingRelocations;
}

void SequenceMedia::RecordRelocation(const std::string& from, const std::string& to)
{
    std::scoped_lock lock(_cacheMutex);
    _pendingRelocations.emplace_back(from, to);
}

void SequenceMedia::ClearRelocations()
{
    std::scoped_lock lock(_cacheMutex);
    _pendingRelocations.clear();
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
    for (const auto& pair : _textCache) {
        pair.second->MarkIsUsed(false);
    }
    for (const auto& pair : _svgCache) {
        pair.second->MarkIsUsed(false);
    }
    for (const auto& pair : _shaderCache) {
        pair.second->MarkIsUsed(false);
    }
    for (const auto& pair : _binaryCache) {
        pair.second->MarkIsUsed(false);
    }
    for (const auto& pair : _videoCache) {
        pair.second->MarkIsUsed(false);
    }
    for (const auto& pair : _audioCache) {
        pair.second->MarkIsUsed(false);
    }
}

// =====================================================================
// TextMediaCacheEntry Implementation
// =====================================================================

TextMediaCacheEntry::TextMediaCacheEntry() : MediaCacheEntry(MediaType::TextFile) {}
TextMediaCacheEntry::TextMediaCacheEntry(const std::string& filePath)
    : MediaCacheEntry(MediaType::TextFile, filePath) {}
TextMediaCacheEntry::TextMediaCacheEntry(const std::string& path, const std::string& base64Data)
    : MediaCacheEntry(MediaType::TextFile, path, base64Data) {}

void TextMediaCacheEntry::Load() {
    std::scoped_lock lock(_cacheMutex);
    if (!_loadingDone) {
        spdlog::debug("TextMediaCacheEntry::Load - path: '{}'", _filePath);
        if (_isEmbedded && !_embeddedData.empty()) {
            std::vector<uint8_t> buf = Base64::Decode(_embeddedData);
            _content.assign(buf.begin(), buf.end());
            _loadingDone = true;
            spdlog::debug("TextMediaCacheEntry::Load - loaded {} bytes from embedded data", _content.size());
        } else {
            LoadRawFromFile(_filePath);
            if (!_embeddedData.empty()) {
                std::vector<uint8_t> buf = Base64::Decode(_embeddedData);
                _content.assign(buf.begin(), buf.end());
                _loadingDone = true;
            } else {
                spdlog::debug("TextMediaCacheEntry::Load - failed to read file '{}' (file not found or unreadable)", _filePath);
            }
        }
    }
}

bool TextMediaCacheEntry::LoadFromXml(const pugi::xml_node& node) {
    if (!node || strcmp(node.name(), "TextFile") != 0) return false;
    _filePath = node.attribute("path").as_string("");
    auto dataChild = node.child("Data");
    if (dataChild) {
        _embeddedData = dataChild.text().as_string("");
        _isEmbedded = true;
        // Decode content immediately for embedded entries
        std::vector<uint8_t> buf = Base64::Decode(_embeddedData);
        _content.assign(buf.begin(), buf.end());
        _loadingDone = true;
    }
    return true;
}

void TextMediaCacheEntry::SaveToXml(pugi::xml_node& parent) const {
    if (!_isEmbedded) return;
    auto node = parent.append_child("TextFile");
    node.append_attribute("path") = _filePath;
    if (!_embeddedData.empty()) {
        auto dataNode = node.append_child("Data");
        dataNode.text().set(_embeddedData);
    }
}

// =====================================================================
// SVGMediaCacheEntry Implementation
// =====================================================================

SVGMediaCacheEntry::SVGMediaCacheEntry() : MediaCacheEntry(MediaType::SVG) {}
SVGMediaCacheEntry::SVGMediaCacheEntry(const std::string& filePath)
    : MediaCacheEntry(MediaType::SVG, filePath) {}
SVGMediaCacheEntry::SVGMediaCacheEntry(const std::string& path, const std::string& base64Data)
    : MediaCacheEntry(MediaType::SVG, path, base64Data) {}

void SVGMediaCacheEntry::Load() {
    std::scoped_lock lock(_cacheMutex);
    if (!_loadingDone) {
        if (_isEmbedded && !_embeddedData.empty()) {
            std::vector<uint8_t> buf = Base64::Decode(_embeddedData);
            _svgContent.assign(buf.begin(), buf.end());
            _loadingDone = true;
        } else {
            LoadRawFromFile(_filePath);
            if (!_embeddedData.empty()) {
                std::vector<uint8_t> buf = Base64::Decode(_embeddedData);
                _svgContent.assign(buf.begin(), buf.end());
                _loadingDone = true;
            }
        }
    }
}
void SVGMediaCacheEntry::ReloadIfChanged() {
    if (!_loadingDone.load() || _isEmbedded.load()) return;
    if (!HasFileChanged()) return;
    std::scoped_lock lock(_cacheMutex);
    if (!HasFileChanged()) return;
    spdlog::debug("SVGMediaCacheEntry::ReloadIfChanged - file changed on disk: '{}'", _filePath);
    _loadingDone = false;
    _embeddedData.clear();
    _svgContent.clear();
    _thumbnail.reset();
    _thumbW = 0;
    _thumbH = 0;
    ClearPreview();
    Load();
}

bool SVGMediaCacheEntry::LoadFromXml(const pugi::xml_node& node) {
    if (!node || strcmp(node.name(), "SVG") != 0) return false;
    _filePath = node.attribute("path").as_string("");
    auto dataChild = node.child("Data");
    if (dataChild) {
        _embeddedData = dataChild.text().as_string("");
        _isEmbedded = true;
        std::vector<uint8_t> buf = Base64::Decode(_embeddedData);
        _svgContent.assign(buf.begin(), buf.end());
        _loadingDone = true;
    }
    return true;
}

void SVGMediaCacheEntry::SaveToXml(pugi::xml_node& parent) const {
    if (!_isEmbedded) return;
    auto node = parent.append_child("SVG");
    node.append_attribute("path") = _filePath;
    if (!_embeddedData.empty()) {
        auto dataNode = node.append_child("Data");
        dataNode.text().set(_embeddedData);
    }
}

std::shared_ptr<xlImage> SVGMediaCacheEntry::GetThumbnail(int maxWidth, int maxHeight) {
    std::scoped_lock lock(_cacheMutex);
    if (_thumbnail && _thumbW == maxWidth && _thumbH == maxHeight) {
        return _thumbnail;
    }
    if (_svgContent.empty()) return nullptr;

    // Parse SVG
    std::string svgCopy = _svgContent;  // nsvgParse modifies input
    xl_NSVGimage* img = nsvgParse(svgCopy.data(), "px", 96);
    if (!img || img->width <= 0 || img->height <= 0) {
        if (img) nsvgDelete(img);
        return nullptr;
    }

    // Calculate scaled dimensions maintaining aspect ratio
    float scale = std::min((float)maxWidth / img->width, (float)maxHeight / img->height);
    int w = std::max(1, (int)(img->width * scale));
    int h = std::max(1, (int)(img->height * scale));

    // Rasterize
    xl_NSVGrasterizer* rast = nsvgCreateRasterizer();
    std::vector<uint8_t> buf(w * h * 4, 0);
    nsvgRasterize(rast, img, 0, 0, scale, buf.data(), w, h, w * 4);
    nsvgDeleteRasterizer(rast);
    nsvgDelete(img);

    // Convert RGBA buffer to xlImage
    _thumbnail = std::make_shared<xlImage>(w, h);
    memcpy(_thumbnail->GetData(), buf.data(), w * h * 4);
    _thumbW = maxWidth;
    _thumbH = maxHeight;
    return _thumbnail;
}

void SVGMediaCacheEntry::GeneratePreview(int maxWidth, int maxHeight) {
    std::scoped_lock lock(_cacheMutex);
    if (_previewWidth == maxWidth && _previewHeight == maxHeight && !_previewFrames.empty()) return;

    _previewFrames.clear();
    _previewFrameTimes.clear();

    auto thumb = GetThumbnail(maxWidth, maxHeight);
    if (thumb) {
        _previewFrames.push_back(thumb);
        _previewFrameTimes.push_back(0); // single frame, no animation
    }
    _previewWidth = maxWidth;
    _previewHeight = maxHeight;
}

// =====================================================================
// ShaderMediaCacheEntry Implementation
// =====================================================================

ShaderMediaCacheEntry::ShaderMediaCacheEntry() : MediaCacheEntry(MediaType::Shader) {}
ShaderMediaCacheEntry::ShaderMediaCacheEntry(const std::string& filePath)
    : MediaCacheEntry(MediaType::Shader, filePath) {}
ShaderMediaCacheEntry::ShaderMediaCacheEntry(const std::string& path, const std::string& base64Data)
    : MediaCacheEntry(MediaType::Shader, path, base64Data) {}
ShaderMediaCacheEntry::~ShaderMediaCacheEntry() {
    delete _shaderConfig;
}

ShaderConfig* ShaderMediaCacheEntry::GetShaderConfig(SequenceElements* sequenceElements) {
    std::scoped_lock lock(_cacheMutex);
    if (_shaderConfig == nullptr && !_shaderSource.empty()) {
        _shaderConfig = ShaderEffect::ParseShaderFromSource(_filePath, _shaderSource, sequenceElements);
    }
    return _shaderConfig;
}

void ShaderMediaCacheEntry::Load() {
    std::scoped_lock lock(_cacheMutex);
    if (!_loadingDone) {
        if (_isEmbedded && !_embeddedData.empty()) {
            std::vector<uint8_t> buf = Base64::Decode(_embeddedData);
            _shaderSource.assign(buf.begin(), buf.end());
            _loadingDone = true;
        } else {
            LoadRawFromFile(_filePath);
            if (!_embeddedData.empty()) {
                std::vector<uint8_t> buf = Base64::Decode(_embeddedData);
                _shaderSource.assign(buf.begin(), buf.end());
                _loadingDone = true;
            }
        }
    }
}
void ShaderMediaCacheEntry::ReloadIfChanged() {
    if (!_loadingDone.load() || _isEmbedded.load()) return;
    if (!HasFileChanged()) return;
    std::scoped_lock lock(_cacheMutex);
    if (!HasFileChanged()) return;
    spdlog::debug("ShaderMediaCacheEntry::ReloadIfChanged - file changed on disk: '{}'", _filePath);
    _loadingDone = false;
    _embeddedData.clear();
    _shaderSource.clear();
    delete _shaderConfig;
    _shaderConfig = nullptr;
    ClearPreview();
    Load();
}

bool ShaderMediaCacheEntry::LoadFromXml(const pugi::xml_node& node) {
    if (!node || strcmp(node.name(), "Shader") != 0) return false;
    _filePath = node.attribute("path").as_string("");
    auto dataChild = node.child("Data");
    if (dataChild) {
        _embeddedData = dataChild.text().as_string("");
        _isEmbedded = true;
        std::vector<uint8_t> buf = Base64::Decode(_embeddedData);
        _shaderSource.assign(buf.begin(), buf.end());
        _loadingDone = true;
    }
    return true;
}

void ShaderMediaCacheEntry::SaveToXml(pugi::xml_node& parent) const {
    if (!_isEmbedded) return;
    auto node = parent.append_child("Shader");
    node.append_attribute("path") = _filePath;
    if (!_embeddedData.empty()) {
        auto dataNode = node.append_child("Data");
        dataNode.text().set(_embeddedData);
    }
}

// =====================================================================
// BinaryMediaCacheEntry Implementation
// =====================================================================

BinaryMediaCacheEntry::BinaryMediaCacheEntry() : MediaCacheEntry(MediaType::BinaryFile) {}
BinaryMediaCacheEntry::BinaryMediaCacheEntry(const std::string& filePath, const std::string& subtype)
    : MediaCacheEntry(MediaType::BinaryFile, filePath), _subtype(subtype) {}
BinaryMediaCacheEntry::BinaryMediaCacheEntry(const std::string& path, const std::string& base64Data, const std::string& subtype)
    : MediaCacheEntry(MediaType::BinaryFile, path, base64Data), _subtype(subtype) {}

void BinaryMediaCacheEntry::Load() {
    std::scoped_lock lock(_cacheMutex);
    if (!_loadingDone) {
        // Binary files (Glediator) are not embeddable — readers open files directly.
        // Just verify the file exists, don't load into memory.
        if (FileExists(_filePath)) {
            RecordFileTimestamp();
            _loadingDone = true;
        }
    }
}

bool BinaryMediaCacheEntry::LoadFromXml(const pugi::xml_node& node) {
    if (!node || strcmp(node.name(), "BinaryFile") != 0) return false;
    _filePath = node.attribute("path").as_string("");
    _subtype = node.attribute("subtype").as_string("");
    auto dataChild = node.child("Data");
    if (dataChild) {
        _embeddedData = dataChild.text().as_string("");
        _isEmbedded = true;
        _data = Base64::Decode(_embeddedData);
        _loadingDone = true;
    }
    return true;
}

void BinaryMediaCacheEntry::SaveToXml(pugi::xml_node& parent) const {
    if (!_isEmbedded) return;
    auto node = parent.append_child("BinaryFile");
    node.append_attribute("path") = _filePath;
    if (!_subtype.empty()) {
        node.append_attribute("subtype") = _subtype;
    }
    if (!_embeddedData.empty()) {
        auto dataNode = node.append_child("Data");
        dataNode.text().set(_embeddedData);
    }
}

// =====================================================================
// VideoMediaCacheEntry Implementation
// =====================================================================

VideoMediaCacheEntry::VideoMediaCacheEntry() : MediaCacheEntry(MediaType::Video) {}
VideoMediaCacheEntry::VideoMediaCacheEntry(const std::string& filePath)
    : MediaCacheEntry(MediaType::Video, filePath) {}

void VideoMediaCacheEntry::Load() {
    std::scoped_lock lock(_cacheMutex);
    if (!_loadingDone) {
        // Videos are path-only: resolve to an absolute (and existing) path
        // for VideoReader. Absolute paths go through FixFile too so sequences
        // saved on another machine get their paths re-resolved against the
        // current show/media folders.
        std::string resolved = FileUtils::FixFile("", _filePath);
        _resolvedPath = resolved.empty() ? _filePath : resolved;
        RecordFileTimestamp();
        _loadingDone = true;
    }
}

std::shared_ptr<xlImage> VideoMediaCacheEntry::GetThumbnail(int maxWidth, int maxHeight) {
    std::scoped_lock lock(_cacheMutex);
    if (_thumbnail && _thumbW == maxWidth && _thumbH == maxHeight) {
        return _thumbnail;
    }
    if (_resolvedPath.empty() || !FileExists(_resolvedPath)) return nullptr;

    // Open video at thumbnail dimensions, grab first frame
    VideoReader reader(_resolvedPath, maxWidth, maxHeight, true, false, true);
    if (!reader.IsValid()) return nullptr;

    VideoFrame* frame = reader.GetNextFrame(0);
    if (!frame || !frame->data) return nullptr;

    int w = reader.GetWidth();
    int h = reader.GetHeight();
    if (w <= 0 || h <= 0) return nullptr;

    // VideoFrame with wantAlpha=true is RGBA, matching xlImage layout
    _thumbnail = std::make_shared<xlImage>(w, h);
    int srcStride = frame->linesize;
    uint8_t* dst = _thumbnail->GetData();
    for (int y = 0; y < h; y++) {
        memcpy(dst + y * w * 4, frame->data + y * srcStride, w * 4);
    }
    _thumbW = maxWidth;
    _thumbH = maxHeight;
    return _thumbnail;
}

int VideoMediaCacheEntry::GetDurationMS() {
    int cached = _durationMS.load();
    if (cached >= 0) return cached;

    std::string path;
    {
        std::scoped_lock lock(_cacheMutex);
        path = _resolvedPath;
    }
    if (path.empty() || !FileExists(path)) {
        _durationMS.store(0);
        return 0;
    }
    // Static VideoReader probe — doesn't open decoders, just reads
    // the container header. Much cheaper than constructing a full
    // VideoReader when duration is the only thing needed.
    long ms = VideoReader::GetVideoLength(path);
    int val = (ms > 0 && ms < std::numeric_limits<int>::max()) ? (int)ms : 0;
    _durationMS.store(val);
    return val;
}

void VideoMediaCacheEntry::GeneratePreview(int maxWidth, int maxHeight) {
    {
        std::scoped_lock lock(_cacheMutex);
        if (_previewWidth == maxWidth && _previewHeight == maxHeight && !_previewFrames.empty()) return;
    }

    std::string path;
    {
        std::scoped_lock lock(_cacheMutex);
        path = _resolvedPath;
    }
    if (path.empty() || !FileExists(path)) return;

    // Decode video frames without holding the mutex (may do network I/O)
    VideoReader reader(path, maxWidth, maxHeight, true, false, true);
    if (!reader.IsValid()) return;

    int w = reader.GetWidth();
    int h = reader.GetHeight();
    if (w <= 0 || h <= 0) return;

    std::vector<std::shared_ptr<xlImage>> frames;
    std::vector<long> frameTimes;

    // Extract first 1 second of frames at 50ms intervals
    int lengthMS = reader.GetLengthMS();
    // Free side-effect: cache the full duration here so a subsequent
    // `GetDurationMS()` doesn't have to reopen the file.
    _durationMS.store(lengthMS > 0 ? lengthMS : 0);
    int maxMS = std::min(lengthMS, 1000);
    int frameTimeMS = 50;

    for (int ts = 0; ts < maxMS; ts += frameTimeMS) {
        VideoFrame* frame = reader.GetNextFrame(ts);
        if (!frame || !frame->data) break;

        auto img = std::make_shared<xlImage>(w, h);
        int srcStride = frame->linesize;
        uint8_t* dst = img->GetData();
        for (int y = 0; y < h; y++) {
            memcpy(dst + y * w * 4, frame->data + y * srcStride, w * 4);
        }
        frames.push_back(img);
        frameTimes.push_back(frameTimeMS);
    }

    // Store results under lock
    std::scoped_lock lock(_cacheMutex);
    if (_previewFrames.empty()) {
        _previewFrames = std::move(frames);
        _previewFrameTimes = std::move(frameTimes);
        _previewWidth = maxWidth;
        _previewHeight = maxHeight;
    }
}

bool VideoMediaCacheEntry::LoadFromXml(const pugi::xml_node& node) {
    if (!node || strcmp(node.name(), "Video") != 0) return false;
    _filePath = node.attribute("path").as_string("");
    return true;
}

void VideoMediaCacheEntry::SaveToXml(pugi::xml_node& parent) const {
    // Videos are never embedded but we track them in SequenceMedia for inventory
    auto node = parent.append_child("Video");
    node.append_attribute("path") = _filePath;
}

// =====================================================================
// AudioMediaCacheEntry Implementation
// =====================================================================

AudioMediaCacheEntry::AudioMediaCacheEntry()
    : MediaCacheEntry(MediaType::Audio) {}

AudioMediaCacheEntry::AudioMediaCacheEntry(const std::string& filePath)
    : MediaCacheEntry(MediaType::Audio, filePath) {}

void AudioMediaCacheEntry::Load() {
    // Audio data is loaded by AudioManager in SequenceFile, not here.
    // Just mark loading done so IsOk() returns true for path-valid entries.
    _loadingDone = true;
}

bool AudioMediaCacheEntry::LoadFromXml(const pugi::xml_node& node) {
    if (!node || strcmp(node.name(), "Audio") != 0) return false;
    _filePath = node.attribute("path").as_string("");
    return !_filePath.empty();
}

void AudioMediaCacheEntry::SaveToXml(pugi::xml_node& parent) const {
    // Audio files are never embedded — track by path only
    auto node = parent.append_child("Audio");
    node.append_attribute("path") = _filePath;
}

// =====================================================================
// SequenceMedia — New type-specific retrieval methods
// =====================================================================

std::string SequenceMedia::ResolvePath(const std::string& filepath) {
    // Absolute paths also go through FixFile so sequences saved on a different
    // machine (e.g. desktop-saved sequence opened on iPad) get their embedded
    // absolute paths re-resolved against the current show/media folders.
    // FixFile's fast path is FileExists(file, false) → early return, so
    // untouched paths stay cheap on desktop.
    std::string resolved = FileUtils::FixFile("", filepath);
    if (!resolved.empty()) {
        return resolved;
    }
    return filepath;
}

std::shared_ptr<TextMediaCacheEntry> SequenceMedia::GetTextFile(const std::string& filepath) {
    if (filepath.empty()) return nullptr;
    std::unique_lock lock(_cacheMutex);
    auto it = _textCache.find(filepath);
    if (it != _textCache.end()) {
        auto ret = it->second;
        if (!ret->isLoaded()) {
            lock.unlock();
            ret->Load();
        }
        ret->ReloadIfChanged();
        return ret;
    }
    std::string loadPath = ResolvePath(filepath);
    for (auto& [key, entry] : _textCache) {
        if (entry->GetFilePath() == loadPath || ResolvePath(key) == loadPath) {
            if (!entry->isLoaded()) { lock.unlock(); entry->Load(); }
            entry->ReloadIfChanged();
            return entry;
        }
    }
    auto np = std::make_shared<TextMediaCacheEntry>(loadPath);
    _textCache.emplace(filepath, np);
    lock.unlock();
    np->Load();
    return np;
}

std::shared_ptr<SVGMediaCacheEntry> SequenceMedia::GetSVG(const std::string& filepath) {
    if (filepath.empty()) return nullptr;
    std::unique_lock lock(_cacheMutex);
    auto it = _svgCache.find(filepath);
    if (it != _svgCache.end()) {
        auto ret = it->second;
        if (!ret->isLoaded()) {
            lock.unlock();
            ret->Load();
        }
        ret->ReloadIfChanged();
        return ret;
    }
    std::string loadPath = ResolvePath(filepath);
    for (auto& [key, entry] : _svgCache) {
        if (entry->GetFilePath() == loadPath || ResolvePath(key) == loadPath) {
            if (!entry->isLoaded()) { lock.unlock(); entry->Load(); }
            entry->ReloadIfChanged();
            return entry;
        }
    }
    auto np = std::make_shared<SVGMediaCacheEntry>(loadPath);
    _svgCache.emplace(filepath, np);
    lock.unlock();
    np->Load();
    return np;
}

std::shared_ptr<ShaderMediaCacheEntry> SequenceMedia::GetShader(const std::string& filepath) {
    if (filepath.empty()) return nullptr;
    std::unique_lock lock(_cacheMutex);
    auto it = _shaderCache.find(filepath);
    if (it != _shaderCache.end()) {
        auto ret = it->second;
        if (!ret->isLoaded()) {
            lock.unlock();
            ret->Load();
        }
        ret->ReloadIfChanged();
        return ret;
    }
    // Check if the resolved path matches an existing entry (avoids duplicates
    // when the same file is referenced by both relative and absolute paths)
    std::string loadPath = ResolvePath(filepath);
    for (auto& [key, entry] : _shaderCache) {
        if (entry->GetFilePath() == loadPath || ResolvePath(key) == loadPath) {
            if (!entry->isLoaded()) { lock.unlock(); entry->Load(); }
            entry->ReloadIfChanged();
            return entry;
        }
    }
    auto np = std::make_shared<ShaderMediaCacheEntry>(loadPath);
    _shaderCache.emplace(filepath, np);
    lock.unlock();
    np->Load();
    return np;
}

std::shared_ptr<BinaryMediaCacheEntry> SequenceMedia::GetBinaryFile(const std::string& filepath, const std::string& subtype) {
    if (filepath.empty()) return nullptr;
    std::unique_lock lock(_cacheMutex);
    auto it = _binaryCache.find(filepath);
    if (it != _binaryCache.end()) {
        auto ret = it->second;
        if (!ret->isLoaded()) {
            lock.unlock();
            ret->Load();
        }
        ret->ReloadIfChanged();
        return ret;
    }
    std::string loadPath = ResolvePath(filepath);
    for (auto& [key, entry] : _binaryCache) {
        if (entry->GetFilePath() == loadPath || ResolvePath(key) == loadPath) {
            if (!entry->isLoaded()) { lock.unlock(); entry->Load(); }
            entry->ReloadIfChanged();
            return entry;
        }
    }
    auto np = std::make_shared<BinaryMediaCacheEntry>(loadPath, subtype);
    _binaryCache.emplace(filepath, np);
    lock.unlock();
    np->Load();
    return np;
}

std::shared_ptr<VideoMediaCacheEntry> SequenceMedia::GetVideo(const std::string& filepath) {
    if (filepath.empty()) return nullptr;
    std::unique_lock lock(_cacheMutex);
    auto it = _videoCache.find(filepath);
    if (it != _videoCache.end()) {
        auto ret = it->second;
        if (!ret->isLoaded()) {
            lock.unlock();
            ret->Load();
        }
        ret->ReloadIfChanged();
        return ret;
    }
    // Check if the resolved path matches an existing entry
    std::string resolved = ResolvePath(filepath);
    for (auto& [key, entry] : _videoCache) {
        if (entry->GetFilePath() == resolved || ResolvePath(key) == resolved) {
            if (!entry->isLoaded()) { lock.unlock(); entry->Load(); }
            entry->ReloadIfChanged();
            return entry;
        }
    }
    auto np = std::make_shared<VideoMediaCacheEntry>(filepath);
    _videoCache.emplace(filepath, np);
    lock.unlock();
    np->Load();
    return np;
}

std::shared_ptr<AudioMediaCacheEntry> SequenceMedia::GetAudio(const std::string& filepath) {
    if (filepath.empty()) return nullptr;
    std::unique_lock lock(_cacheMutex);
    auto it = _audioCache.find(filepath);
    if (it != _audioCache.end()) {
        auto ret = it->second;
        if (!ret->isLoaded()) {
            lock.unlock();
            ret->Load();
        }
        ret->ReloadIfChanged();
        return ret;
    }
    // Check if the resolved path matches an existing entry
    std::string resolved = ResolvePath(filepath);
    for (auto& [key, entry] : _audioCache) {
        if (entry->GetFilePath() == resolved || ResolvePath(key) == resolved) {
            if (!entry->isLoaded()) { lock.unlock(); entry->Load(); }
            entry->ReloadIfChanged();
            return entry;
        }
    }
    auto np = std::make_shared<AudioMediaCacheEntry>(filepath);
    _audioCache.emplace(filepath, np);
    lock.unlock();
    np->Load();
    return np;
}

// =====================================================================
// SequenceMedia — Cross-type queries
// =====================================================================

bool SequenceMedia::HasMedia(const std::string& filepath) const {
    std::scoped_lock lock(_cacheMutex);
    return _imageCache.count(filepath) || _textCache.count(filepath) ||
           _svgCache.count(filepath) || _shaderCache.count(filepath) ||
           _binaryCache.count(filepath) || _videoCache.count(filepath) ||
           _audioCache.count(filepath);
}

std::pair<bool, bool> SequenceMedia::GetMediaEmbedState(const std::string& filepath) const {
    std::scoped_lock lock(_cacheMutex);
    auto checkCache = [&](const auto& cache) -> std::optional<std::pair<bool, bool>> {
        auto it = cache.find(filepath);
        if (it != cache.end())
            return std::pair{ it->second->IsEmbedded(), it->second->IsEmbeddable() };
        return std::nullopt;
    };
    if (auto r = checkCache(_imageCache)) return *r;
    if (auto r = checkCache(_textCache)) return *r;
    if (auto r = checkCache(_svgCache)) return *r;
    if (auto r = checkCache(_shaderCache)) return *r;
    if (auto r = checkCache(_binaryCache)) return *r;
    if (auto r = checkCache(_videoCache)) return *r;
    if (auto r = checkCache(_audioCache)) return *r;
    return { false, false };
}

void SequenceMedia::RemoveMedia(const std::string& filepath) {
    std::scoped_lock lock(_cacheMutex);
    _imageCache.erase(filepath);
    _textCache.erase(filepath);
    _svgCache.erase(filepath);
    _shaderCache.erase(filepath);
    _binaryCache.erase(filepath);
    _videoCache.erase(filepath);
    _audioCache.erase(filepath);
}

bool SequenceMedia::ReloadMedia(const std::string& filepath) {
    std::scoped_lock lock(_cacheMutex);

    // Helper: if found and not embedded, erase from cache and re-create to reload from disk
    auto found = [&](auto& cache) -> bool {
        auto it = cache.find(filepath);
        if (it == cache.end()) return false;
        if (it->second->IsEmbedded()) return false;  // embedded entries don't reload from disk
        return true;
    };

    if (found(_imageCache)) {
        _imageCache.erase(filepath);
        GetImage(filepath);
        return true;
    }
    if (found(_textCache)) {
        _textCache.erase(filepath);
        GetTextFile(filepath);
        return true;
    }
    if (found(_svgCache)) {
        _svgCache.erase(filepath);
        GetSVG(filepath);
        return true;
    }
    if (found(_shaderCache)) {
        _shaderCache.erase(filepath);
        GetShader(filepath);
        return true;
    }
    if (found(_binaryCache)) {
        auto subtype = _binaryCache[filepath]->GetSubtype();
        _binaryCache.erase(filepath);
        GetBinaryFile(filepath, subtype);
        return true;
    }
    if (found(_videoCache)) {
        _videoCache.erase(filepath);
        GetVideo(filepath);
        return true;
    }
    if (found(_audioCache)) {
        _audioCache.erase(filepath);
        GetAudio(filepath);
        return true;
    }
    return false;
}

void SequenceMedia::ForceRefreshEntry(const std::string& filepath,
                                      const std::string& resolvedPath,
                                      MediaType type) {
    if (filepath.empty()) return;

    // resolvedPath is the known absolute file path supplied by the caller (e.g. from
    // the file picker). We use it directly so we bypass FixFile's stale positive/negative
    // caches, which can misdirect GetXxx() to an old location after a re-select.
    const std::string& loadPath = resolvedPath.empty() ? ResolvePath(filepath) : resolvedPath;

    std::shared_ptr<MediaCacheEntry> entry;
    {
        std::scoped_lock lock(_cacheMutex);

        // Generic helper: erase the exact key plus any other entry whose resolved path
        // collides with loadPath (these would cause GetXxx(filepath)'s duplicate-path
        // check to short-circuit and return without inserting the new key).
        // Then insert a fresh entry at filepath.
        auto doInsert = [&](auto& cache, auto makeEntry) -> std::shared_ptr<MediaCacheEntry> {
            cache.erase(filepath);
            for (auto it = cache.begin(); it != cache.end(); ) {
                if (it->second->GetFilePath() == loadPath || ResolvePath(it->first) == loadPath)
                    it = cache.erase(it);
                else
                    ++it;
            }
            auto np = makeEntry();
            cache.emplace(filepath, np);
            return np;
        };

        switch (type) {
            case MediaType::Image:
                entry = doInsert(_imageCache,
                    [&]{ return std::make_shared<ImageCacheEntry>(loadPath); });
                break;
            case MediaType::Shader:
                entry = doInsert(_shaderCache,
                    [&]{ return std::make_shared<ShaderMediaCacheEntry>(loadPath); });
                break;
            case MediaType::SVG:
                entry = doInsert(_svgCache,
                    [&]{ return std::make_shared<SVGMediaCacheEntry>(loadPath); });
                break;
            case MediaType::TextFile:
                entry = doInsert(_textCache,
                    [&]{ return std::make_shared<TextMediaCacheEntry>(loadPath); });
                break;
            case MediaType::BinaryFile: {
                // Preserve the subtype (e.g. "glediator") from the existing entry if present.
                std::string subtype;
                auto existing = _binaryCache.find(filepath);
                if (existing != _binaryCache.end()) subtype = existing->second->GetSubtype();
                entry = doInsert(_binaryCache,
                    [&]{ return std::make_shared<BinaryMediaCacheEntry>(loadPath, subtype); });
                break;
            }
            case MediaType::Video:
                entry = doInsert(_videoCache,
                    [&]{ return std::make_shared<VideoMediaCacheEntry>(loadPath); });
                break;
            default:
                return;
        }
    }

    if (entry) entry->Load();
}

size_t SequenceMedia::GetMediaCount() const {
    std::scoped_lock lock(_cacheMutex);
    return _imageCache.size() + _textCache.size() + _svgCache.size() +
           _shaderCache.size() + _binaryCache.size() + _videoCache.size() +
           _audioCache.size();
}

std::vector<std::pair<std::string, MediaType>> SequenceMedia::GetAllMediaPaths() const {
    std::scoped_lock lock(_cacheMutex);
    std::vector<std::pair<std::string, MediaType>> paths;
    paths.reserve(_imageCache.size() + _textCache.size() + _svgCache.size() +
                   _shaderCache.size() + _binaryCache.size() + _videoCache.size() +
                   _audioCache.size());
    for (const auto& p : _imageCache) paths.emplace_back(p.first, MediaType::Image);
    for (const auto& p : _svgCache) paths.emplace_back(p.first, MediaType::SVG);
    for (const auto& p : _shaderCache) paths.emplace_back(p.first, MediaType::Shader);
    for (const auto& p : _textCache) paths.emplace_back(p.first, MediaType::TextFile);
    for (const auto& p : _binaryCache) paths.emplace_back(p.first, MediaType::BinaryFile);
    for (const auto& p : _videoCache) paths.emplace_back(p.first, MediaType::Video);
    for (const auto& p : _audioCache) paths.emplace_back(p.first, MediaType::Audio);
    return paths;
}

std::vector<std::string> SequenceMedia::GetVideoFilePaths() const {
    std::scoped_lock lock(_cacheMutex);
    std::vector<std::string> paths;
    paths.reserve(_videoCache.size());
    for (const auto& p : _videoCache) {
        auto& entry = p.second;
        const std::string& resolved = entry->GetResolvedPath();
        paths.push_back(resolved.empty() ? p.first : resolved);
    }
    return paths;
}

// =====================================================================
// SequenceMedia — Generalized embed/extract
// =====================================================================

void SequenceMedia::EmbedMedia(const std::string& filepath) {
    std::scoped_lock lock(_cacheMutex);
    auto ii = _imageCache.find(filepath);
    if (ii != _imageCache.end()) { ii->second->Embed(); ii->second->SetFilePath(filepath); return; }
    auto it = _textCache.find(filepath);
    if (it != _textCache.end()) { it->second->Embed(); it->second->SetFilePath(filepath); return; }
    auto is = _svgCache.find(filepath);
    if (is != _svgCache.end()) { is->second->Embed(); is->second->SetFilePath(filepath); return; }
    auto ih = _shaderCache.find(filepath);
    if (ih != _shaderCache.end()) { ih->second->Embed(); ih->second->SetFilePath(filepath); return; }
    auto ib = _binaryCache.find(filepath);
    if (ib != _binaryCache.end()) { ib->second->Embed(); ib->second->SetFilePath(filepath); return; }
    // Videos are not embeddable
}

void SequenceMedia::EmbedAllMedia() {
    std::scoped_lock lock(_cacheMutex);
    for (auto& p : _imageCache) { p.second->Embed(); p.second->SetFilePath(p.first); }
    for (auto& p : _textCache) { p.second->Embed(); p.second->SetFilePath(p.first); }
    for (auto& p : _svgCache) { p.second->Embed(); p.second->SetFilePath(p.first); }
    for (auto& p : _shaderCache) { p.second->Embed(); p.second->SetFilePath(p.first); }
    for (auto& p : _binaryCache) { p.second->Embed(); p.second->SetFilePath(p.first); }
    // Videos are not embeddable
}

void SequenceMedia::ExtractMedia(const std::string& filepath) {
    std::scoped_lock lock(_cacheMutex);
    auto ii = _imageCache.find(filepath);
    if (ii != _imageCache.end()) { ii->second->Extract(); return; }
    auto it = _textCache.find(filepath);
    if (it != _textCache.end()) { it->second->Extract(); return; }
    auto is = _svgCache.find(filepath);
    if (is != _svgCache.end()) { is->second->Extract(); return; }
    auto ih = _shaderCache.find(filepath);
    if (ih != _shaderCache.end()) { ih->second->Extract(); return; }
    auto ib = _binaryCache.find(filepath);
    if (ib != _binaryCache.end()) { ib->second->Extract(); return; }
}

void SequenceMedia::ExtractAllMedia() {
    std::scoped_lock lock(_cacheMutex);
    for (auto& p : _imageCache) p.second->Extract();
    for (auto& p : _textCache) p.second->Extract();
    for (auto& p : _svgCache) p.second->Extract();
    for (auto& p : _shaderCache) p.second->Extract();
    for (auto& p : _binaryCache) p.second->Extract();
}

void SequenceMedia::RemoveUnusedMedia() {
    std::scoped_lock lock(_cacheMutex);
    auto removeUnused = [](auto& cache, const char* typeName) {
        std::vector<std::string> toRemove;
        for (const auto& pair : cache) {
            if (!pair.second->IsUsed()) {
                toRemove.push_back(pair.first);
            }
        }
        for (const auto& path : toRemove) {
            spdlog::debug("Removing unused {} from cache: {}", typeName, path);
            cache.erase(path);
        }
    };
    removeUnused(_imageCache, "image");
    removeUnused(_textCache, "text file");
    removeUnused(_svgCache, "SVG");
    removeUnused(_shaderCache, "shader");
    removeUnused(_binaryCache, "binary file");
    removeUnused(_videoCache, "video");
}

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

#include <wx/wx.h>
#include <wx/image.h>
#include <wx/gifdecod.h>
#include <atomic>
#include <map>
#include <string>
#include <memory>
#include <mutex>


class wxXmlNode;
class GIFImage;

/**
 * ScaledImageCacheKey - Key for scaled image cache
 */
struct ScaledImageCacheKey {
    int frameNumber;
    int width;
    int height;
    bool suppressGIFBackground;
    
    bool operator<(const ScaledImageCacheKey& other) const {
        if (frameNumber != other.frameNumber) return frameNumber < other.frameNumber;
        if (width != other.width) return width < other.width;
        if (height != other.height) return height < other.height;
        return suppressGIFBackground == other.suppressGIFBackground ? 0 : suppressGIFBackground ? 1 : -1;
    }
};

/**
 * ImageCacheEntry - Represents a cached image with its metadata
 */
class ImageCacheEntry
{
public:
    ImageCacheEntry();
    ImageCacheEntry(const std::string &filePath);
    ImageCacheEntry(const std::string &path, const std::string &base64Data);
    ImageCacheEntry(const std::string &path, const std::vector<wxImage> &images, int ft, const std::string &base64Data = "");
    ~ImageCacheEntry();

    // Accessors
    const std::string &GetFilePath() const { return _filePath; }
    const std::string &GetEmbeddedData() const { return _embeddedData; }
    bool IsEmbedded() const { return _isEmbedded; }
    bool IsEmbeddable() const { return !_embeddedData.empty(); }
    int GetImageCount() const { return _imageCount; }

    void MarkIsUsed(bool used = true) { _used = used; }
    bool IsUsed() const { return _used; }
    void SetFilePath(const std::string& path) { _filePath = path; }
    
    wxSize GetImageSize() const { return _imageSize; }

    // Serialization
    bool LoadFromXml(wxXmlNode* node);
    wxXmlNode* SaveToXml() const;

    // Conversion to/from embedded format (base64 encoded PNG)
    void EmbedImage() {
        _isEmbedded = true;
    }
    void ExtractImage() {
        _isEmbedded = false;
    }
    // Write the raw embedded bytes to disk. Returns true on success.
    bool SaveToFile(const std::string& path) const;
    bool IsOk() {
        return !_frameImages.empty() && _frameImages[0]->IsOk();
    }
    
    std::shared_ptr<wxImage> GetFrame(int x, bool suppressGIFBackground);
    int GetFrameForTime(int ms, bool loop);
    bool IsFrameBasedAnimation() const { return _frameBasedAnimation; }
    
    std::shared_ptr<wxImage> GetScaledImage(int frameNumber, int width, int height, bool bgSuppressed);

    void ClearScaledImageCache();

    void Load();
    bool isLoaded() const { return _loadingDone; }

    // Pre-cache a base64 PNG string for frame i to avoid re-encoding on save.
    void SetFrameData(std::vector<std::string> data) { _frameData = std::move(data); }

private:
    void LoadFromFile(const std::string& filepath);
    void LoadFromData(const std::string& base64Data);
    void loadGIF(wxMemoryBuffer &ins);
    void loadWEBP(wxMemoryBuffer &ins);
    void loadImage(wxMemoryBuffer &ins);
    int GetExifOrientation(wxMemoryBuffer& buffer);
    
    std::string _filePath;          // Original file path
    std::string _embeddedData;      // Base64 encoded image data (when embedded, single-frame)
    mutable std::vector<std::string> _frameData; // Base64 encoded PNG per frame (multi-frame embedded, cached to avoid re-encode)
    int _imageCount = 0;                // Number of frames in image (1 for static, >1 for animated)
    bool _frameBasedAnimation = true;
    std::atomic_bool _used;

    std::vector<long> _frameTimes;
    std::vector<std::shared_ptr<wxImage>> _frameImages;
    std::vector<std::shared_ptr<wxImage>> _frameImagesNoBG;
    wxSize _imageSize;
    long _totalTime = 0;
    
    bool _loadingDone = false;
    bool _isEmbedded = false;
    std::shared_ptr<wxImage> invalidImage;
    
    // Scaled image cache
    mutable std::recursive_mutex _cacheMutex;
    mutable std::map<ScaledImageCacheKey, std::shared_ptr<wxImage>> _scaledImageCache;
};

/**
 * SequenceMedia - Manages media caching for a sequence
 * 
 * This class currently caches images referenced by PicturesEffect to avoid
 * reloading them repeatedly. It also supports embedding images
 * directly in the xsq file for portability.
 */
class SequenceMedia
{
public:
    SequenceMedia();
    ~SequenceMedia();

    // Image retrieval and caching
    std::shared_ptr<ImageCacheEntry> GetImage(const std::string& filepath);
    bool HasImage(const std::string& filepath) const;
    void AddAnimatedImage(const std::string& filepath, int msFrameTime);
        
    // Remove an image from the cache
    void RemoveImage(const std::string& filepath);
    
    // Clear all cached images
    void Clear();
    
    
    // Embed/extract operations
    void EmbedImage(const std::string& filepath);
    void EmbedAllImages();
    // Add a wxImage directly as an embedded entry with the given name.
    void AddEmbeddedImage(const std::string& name, const wxImage& image);
    void AddEmbeddedImage(const std::string& name, const std::string& imageData);
    // Add multiple frames as a single animated embedded entry. frameTimeMs is the
    // duration of each frame in milliseconds.
    void AddEmbeddedImage(const std::string& name, const std::vector<wxImage>& frames, int frameTimeMs);
    void ExtractImage(const std::string& filepath);
    void ExtractAllImages();
    // Save embedded image data to newPath on disk, rename cache key, mark external.
    // Returns true on success.
    bool ExtractImageToFile(const std::string& oldPath, const std::string& newPath);

    // Rename the cache key (and internal path) for an image entry.
    // Returns true if the rename succeeded (oldPath existed, newPath didn't conflict).
    bool RenameImage(const std::string& oldPath, const std::string& newPath);
    
    // Serialization for xsq file format
    bool LoadFromXml(wxXmlNode* node);
    wxXmlNode* SaveToXml() const;
    
    // Utilities
    size_t GetImageCount() const { return _imageCache.size(); }
    std::vector<std::string> GetImagePaths() const;
    void RemoveUnusedImages(); // Remove images with zero references
    void MarkAllUnused();
private:
    // Cache mapping file path to image entry
    std::map<std::string, std::shared_ptr<ImageCacheEntry>> _imageCache;
        
    // Mutex for thread-safe access to the cache
    mutable std::recursive_mutex _cacheMutex;
};

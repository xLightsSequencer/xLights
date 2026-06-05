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

#include "../utils/xlImage.h"
#include <atomic>
#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <vector>


class AnimatedImage;
namespace pugi { class xml_node; }

/**
 * MediaType - Identifies the type of media cached in SequenceMedia
 */
enum class MediaType {
    Image,
    SVG,
    Shader,
    TextFile,
    BinaryFile,
    Video,
    Audio
};

/**
 * MediaCacheEntry - Base class for all cached media entries
 *
 * Provides common path management, embedding, usage tracking, and serialization.
 */
class MediaCacheEntry
{
public:
    MediaCacheEntry(MediaType type);
    MediaCacheEntry(MediaType type, const std::string& filePath);
    MediaCacheEntry(MediaType type, const std::string& path, const std::string& base64Data);
    virtual ~MediaCacheEntry();

    MediaType GetType() const { return _type; }
    const std::string& GetFilePath() const { return _filePath; }
    void SetFilePath(const std::string& path) { _filePath = path; }
    const std::string& GetEmbeddedData() const { return _embeddedData; }
    bool IsEmbedded() const { return _isEmbedded; }
    virtual bool IsEmbeddable() const { return true; }

    void MarkIsUsed(bool used = true) { _used = used; }
    bool IsUsed() const { return _used; }

    void Embed() { if (IsEmbeddable()) _isEmbedded.store(true); }
    void Extract() { _isEmbedded.store(false); }
    bool SaveToFile(const std::string& path) const;

    virtual void Load() = 0;
    bool isLoaded() const { return _loadingDone; }
    virtual bool IsOk() const { return _loadingDone; }

    // Check if the on-disk file has changed since last load; reload if so.
    virtual void ReloadIfChanged();

    virtual bool LoadFromXml(const pugi::xml_node& node) = 0;
    virtual void SaveToXml(pugi::xml_node& parent) const = 0;

    // Preview frame cache - generates scaled frames for panel previews
    virtual void GeneratePreview(int maxWidth, int maxHeight);
    bool HasPreview() const { std::scoped_lock lock(_cacheMutex); return !_previewFrames.empty(); }
    size_t GetPreviewFrameCount() const { std::scoped_lock lock(_cacheMutex); return _previewFrames.size(); }
    std::shared_ptr<xlImage> GetPreviewFrame(size_t index) const;
    long GetPreviewFrameTime(size_t index) const;
    void ClearPreview();
    void SetPreviewFrames(std::vector<std::shared_ptr<xlImage>>&& frames, int frameTimeMs);

protected:
    // Read a file from disk into _embeddedData as base64
    void LoadRawFromFile(const std::string& filepath);

    // Timestamp helpers for auto-reload of external files
    void RecordFileTimestamp();
    bool HasFileChanged() const;

    MediaType _type;
    std::string _filePath;
    std::string _embeddedData;      // Base64 encoded data
    std::atomic_bool _used;
    std::atomic_bool _loadingDone{false};
    std::atomic_bool _isEmbedded{false};
    std::filesystem::file_time_type _fileTimestamp{}; // mtime when loaded from disk
    mutable std::recursive_mutex _cacheMutex;

    // Preview frame cache
    std::vector<std::shared_ptr<xlImage>> _previewFrames;
    std::vector<long> _previewFrameTimes;  // ms per frame
    int _previewWidth = 0;
    int _previewHeight = 0;
};

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
        return suppressGIFBackground < other.suppressGIFBackground;
    }
};

// Callback type for loading animated images (GIF, WebP). Registered by UI layer.
using AnimationLoaderFunc = std::function<AnimatedImageData(const uint8_t* data, size_t len, const std::string& filename)>;

/**
 * ImageCacheEntry - Represents a cached image with its metadata
 */
class ImageCacheEntry : public MediaCacheEntry
{
public:
    ImageCacheEntry();
    ImageCacheEntry(const std::string &filePath);
    ImageCacheEntry(const std::string &path, const std::string &base64Data);
    ImageCacheEntry(const std::string &path, const std::vector<xlImage> &images, int ft, const std::string &base64Data = "");
    ~ImageCacheEntry() override;

    // Image-specific accessors
    bool IsEmbeddable() const override { return !_embeddedData.empty(); }
    int GetImageCount() const { return _imageCount; }

    int GetImageWidth() const { return _imageWidth; }
    int GetImageHeight() const { return _imageHeight; }

    // Serialization
    bool LoadFromXml(const pugi::xml_node& node) override;
    void SaveToXml(pugi::xml_node& parent) const override;

    // Legacy embed/extract names (delegate to base)
    void EmbedImage() { Embed(); }
    void ExtractImage() { Extract(); }

    bool IsOk() const override {
        return !_frameImages.empty() && _frameImages[0]->IsOk();
    }

    std::shared_ptr<xlImage> GetFrame(int x, bool suppressGIFBackground);
    int GetFrameForTime(int ms, bool loop);
    bool IsFrameBasedAnimation() const { return _frameBasedAnimation; }

    std::shared_ptr<xlImage> GetScaledImage(int frameNumber, int width, int height, bool bgSuppressed);

    void ClearScaledImageCache();

    void GeneratePreview(int maxWidth, int maxHeight) override;

    void Load() override;
    void ReloadIfChanged() override;

    // Pre-cache a base64 PNG string for frame i to avoid re-encoding on save.
    void SetFrameData(std::vector<std::string> data) { _frameData = std::move(data); }

    // Set animation loaders (called once during app init)
    static void SetWebPLoader(AnimationLoaderFunc loader) { _webpLoader = std::move(loader); }

private:
    void LoadFromFile(const std::string& filepath);
    void LoadFromData(const std::string& base64Data);
    void storeAnimated(AnimatedImageData result);
    void loadAnimated(const std::vector<uint8_t> &data, const AnimationLoaderFunc &loader);
    void loadImage(const std::vector<uint8_t> &data);
    int GetExifOrientation(const uint8_t* data, size_t len);

    mutable std::vector<std::string> _frameData; // Base64 encoded PNG per frame (multi-frame embedded, cached to avoid re-encode)
    int _imageCount = 0;                // Number of frames in image (1 for static, >1 for animated)
    bool _frameBasedAnimation = true;

    std::vector<long> _frameTimes;
    std::vector<std::shared_ptr<xlImage>> _frameImages;
    std::vector<std::shared_ptr<xlImage>> _frameImagesNoBG;
    int _imageWidth = 0;
    int _imageHeight = 0;
    long _totalTime = 0;

    std::shared_ptr<xlImage> invalidImage;

    // Scaled image cache
    mutable std::map<ScaledImageCacheKey, std::shared_ptr<xlImage>> _scaledImageCache;

    static AnimationLoaderFunc _webpLoader;
};

/**
 * TextMediaCacheEntry - Cached text file content (for TextEffect text-from-file)
 */
class TextMediaCacheEntry : public MediaCacheEntry
{
public:
    TextMediaCacheEntry();
    TextMediaCacheEntry(const std::string& filePath);
    TextMediaCacheEntry(const std::string& path, const std::string& base64Data);

    const std::string& GetContent() const { return _content; }

    void Load() override;
    bool LoadFromXml(const pugi::xml_node& node) override;
    void SaveToXml(pugi::xml_node& parent) const override;

private:
    std::string _content;
};

/**
 * SVGMediaCacheEntry - Cached SVG file content
 */
class SVGMediaCacheEntry : public MediaCacheEntry
{
public:
    SVGMediaCacheEntry();
    SVGMediaCacheEntry(const std::string& filePath);
    SVGMediaCacheEntry(const std::string& path, const std::string& base64Data);

    const std::string& GetSVGContent() const { return _svgContent; }

    // Generate a thumbnail by rasterizing the SVG at the given max dimensions
    std::shared_ptr<xlImage> GetThumbnail(int maxWidth, int maxHeight);

    void GeneratePreview(int maxWidth, int maxHeight) override;

    void Load() override;
    void ReloadIfChanged() override;
    bool LoadFromXml(const pugi::xml_node& node) override;
    void SaveToXml(pugi::xml_node& parent) const override;

private:
    std::string _svgContent;
    std::shared_ptr<xlImage> _thumbnail;
    int _thumbW = 0, _thumbH = 0;
};

class ShaderConfig;
class SequenceElements;

/**
 * ShaderMediaCacheEntry - Cached shader source (.fs files)
 */
class ShaderMediaCacheEntry : public MediaCacheEntry
{
public:
    ShaderMediaCacheEntry();
    ShaderMediaCacheEntry(const std::string& filePath);
    ShaderMediaCacheEntry(const std::string& path, const std::string& base64Data);
    ~ShaderMediaCacheEntry() override;

    const std::string& GetShaderSource() const { return _shaderSource; }

    // Get or create the parsed ShaderConfig. Caches the result.
    // sequenceElements is needed for timing track enumeration.
    ShaderConfig* GetShaderConfig(SequenceElements* sequenceElements);

    void Load() override;
    void ReloadIfChanged() override;
    bool LoadFromXml(const pugi::xml_node& node) override;
    void SaveToXml(pugi::xml_node& parent) const override;

private:
    std::string _shaderSource;
    ShaderConfig* _shaderConfig = nullptr;
};

/**
 * BinaryMediaCacheEntry - Cached binary file data (Glediator files, etc.)
 */
class BinaryMediaCacheEntry : public MediaCacheEntry
{
public:
    BinaryMediaCacheEntry();
    BinaryMediaCacheEntry(const std::string& filePath, const std::string& subtype = "");
    BinaryMediaCacheEntry(const std::string& path, const std::string& base64Data, const std::string& subtype);

    // Binary files (Glediator, etc.) cannot be embedded — readers open files directly from disk
    bool IsEmbeddable() const override { return false; }

    const std::vector<uint8_t>& GetData() const { return _data; }
    const std::string& GetSubtype() const { return _subtype; }

    void Load() override;
    bool LoadFromXml(const pugi::xml_node& node) override;
    void SaveToXml(pugi::xml_node& parent) const override;

private:
    std::vector<uint8_t> _data;
    std::string _subtype;  // e.g., "glediator"
};

/**
 * VideoMediaCacheEntry - Path-only entry for video files (too large to embed)
 */
class VideoMediaCacheEntry : public MediaCacheEntry
{
public:
    VideoMediaCacheEntry();
    VideoMediaCacheEntry(const std::string& filePath);

    bool IsEmbeddable() const override { return false; }

    // Returns the resolved absolute path for VideoReader to open
    const std::string& GetResolvedPath() const { return _resolvedPath; }

    // Generate a thumbnail from the first frame of the video
    std::shared_ptr<xlImage> GetThumbnail(int maxWidth, int maxHeight);

    void GeneratePreview(int maxWidth, int maxHeight) override;

    // Total duration in milliseconds. Lazily probed on first call
    // (via `VideoReader::GetVideoLength`) and cached for the entry's
    // lifetime — subsequent calls are free. Returns 0 when the file
    // can't be opened.
    int GetDurationMS();

    void Load() override;
    bool LoadFromXml(const pugi::xml_node& node) override;
    void SaveToXml(pugi::xml_node& parent) const override;

private:
    std::string _resolvedPath;
    std::shared_ptr<xlImage> _thumbnail;
    int _thumbW = 0, _thumbH = 0;
    // -1 sentinel: not yet probed. 0: probed and failed. >0: duration.
    std::atomic<int> _durationMS{-1};
};

/**
 * AudioMediaCacheEntry - Path-only entry for audio files (too large to embed)
 *
 * Audio files are never embedded in the sequence — they are always referenced
 * by path. Loading the actual audio data is handled separately by AudioManager
 * in SequenceFile; this entry exists purely for inventory and file management
 * purposes within the media panel.
 */
class AudioMediaCacheEntry : public MediaCacheEntry
{
public:
    AudioMediaCacheEntry();
    explicit AudioMediaCacheEntry(const std::string& filePath);

    bool IsEmbeddable() const override { return false; }

    void Load() override;
    bool LoadFromXml(const pugi::xml_node& node) override;
    void SaveToXml(pugi::xml_node& parent) const override;
};

/**
 * SequenceMedia - Manages all media caching for a sequence
 *
 * Provides unified access to images, SVGs, shaders, text files, binary files,
 * and videos referenced by effects. Supports embedding media directly in the
 * xsq file for portability. All file resolution goes through FileUtils::FixFile() so
 * effects don't need access to xLightsFrame::CurrentDir.
 */
class SequenceMedia
{
public:
    SequenceMedia();
    ~SequenceMedia();

    // === Image-specific API (existing, unchanged) ===
    std::shared_ptr<ImageCacheEntry> GetImage(const std::string& filepath);
    bool HasImage(const std::string& filepath) const;
    void AddAnimatedImage(const std::string& filepath, int msFrameTime);
    void RemoveImage(const std::string& filepath);
    void EmbedImage(const std::string& filepath);
    void EmbedAllImages();
    void AddEmbeddedImage(const std::string& name, const xlImage& image);
    void AddEmbeddedImage(const std::string& name, const std::string& imageData);
    void AddEmbeddedImage(const std::string& name, const std::vector<xlImage>& frames, int frameTimeMs);
    void ExtractImage(const std::string& filepath);
    void ExtractAllImages();
    bool ExtractImageToFile(const std::string& oldPath, const std::string& newPath);
    bool RenameImage(const std::string& oldPath, const std::string& newPath);
    size_t GetImageCount() const { return _imageCache.size(); }
    std::vector<std::string> GetImagePaths() const;
    void RemoveUnusedImages();

    std::vector<std::pair<std::string, std::string>> GetImageRelocations() const;
    void RecordRelocation(const std::string& from, const std::string& to);
    void ClearRelocations();

    std::vector<std::string> GetShaderPaths() const;

    // === Type-specific retrieval (create-on-first-access, resolve via FileUtils::FixFile) ===
    std::shared_ptr<TextMediaCacheEntry> GetTextFile(const std::string& filepath);
    std::shared_ptr<SVGMediaCacheEntry> GetSVG(const std::string& filepath);
    std::shared_ptr<ShaderMediaCacheEntry> GetShader(const std::string& filepath);
    std::shared_ptr<BinaryMediaCacheEntry> GetBinaryFile(const std::string& filepath, const std::string& subtype = "");
    std::shared_ptr<VideoMediaCacheEntry> GetVideo(const std::string& filepath);
    std::shared_ptr<AudioMediaCacheEntry> GetAudio(const std::string& filepath);

    // === Cross-type queries ===
    bool HasMedia(const std::string& filepath) const;
    void RemoveMedia(const std::string& filepath);
    // Re-key a cache entry across any media type. Only touches the
    // in-memory cache — callers are responsible for any on-disk
    // move + walking effect settings to rewrite references. No-op
    // if `oldPath` isn't found or `newPath` already exists.
    // Returns true on successful rename.
    bool RenameMedia(const std::string& oldPath, const std::string& newPath);
    // Reload a non-embedded entry from disk (erases and re-creates the cache entry)
    bool ReloadMedia(const std::string& filepath);
    // Force-insert a fresh entry at `filepath` with `resolvedPath` as its physical
    // file path. Unlike GetXxx(), this always creates a new entry at `filepath` and
    // never returns an existing entry that happens to share the same resolved path.
    // Also removes any other cache entry whose resolved path equals `resolvedPath`
    // to prevent the duplicate-path check from suppressing future GetXxx() calls.
    // Use this after a re-select or bulk-find where the exact replacement file is known.
    void ForceRefreshEntry(const std::string& filepath, const std::string& resolvedPath, MediaType type);
    size_t GetMediaCount() const;
    std::vector<std::pair<std::string, MediaType>> GetAllMediaPaths() const;
    // Returns {isEmbedded, isEmbeddable} for any media path across all caches
    std::pair<bool, bool> GetMediaEmbedState(const std::string& filepath) const;

    // === Generalized embed/extract ===
    void EmbedMedia(const std::string& filepath);
    void EmbedAllMedia();
    void ExtractMedia(const std::string& filepath);
    void ExtractAllMedia();

    // === Query helpers ===
    std::vector<std::string> GetVideoFilePaths() const;

    // === Lifecycle ===
    void Clear();
    void MarkAllUnused();
    void RemoveUnusedMedia();

    // Memory-pressure helpers. `PurgePreviewCaches` drops every
    // entry's preview-frame strip (the thumbnail arrays built for
    // the media picker / effect panels) and, for image entries,
    // the unbounded `_scaledImageCache`. Entries themselves stay —
    // only the render-time / UI-time derivatives are freed. Cheap
    // to call; previews rebuild on next access.
    void PurgePreviewCaches();

    // === Serialization ===
    bool LoadFromXml(const pugi::xml_node& node);
    void SaveToXml(pugi::xml_node& parent) const;

private:
    // Helper to resolve relative paths via FileUtils::FixFile
    static std::string ResolvePath(const std::string& filepath);

    std::vector<std::pair<std::string, std::string>> _pendingRelocations;

    // Per-type caches
    std::map<std::string, std::shared_ptr<ImageCacheEntry>> _imageCache;
    std::map<std::string, std::shared_ptr<TextMediaCacheEntry>> _textCache;
    std::map<std::string, std::shared_ptr<SVGMediaCacheEntry>> _svgCache;
    std::map<std::string, std::shared_ptr<ShaderMediaCacheEntry>> _shaderCache;
    std::map<std::string, std::shared_ptr<BinaryMediaCacheEntry>> _binaryCache;
    std::map<std::string, std::shared_ptr<VideoMediaCacheEntry>> _videoCache;
    std::map<std::string, std::shared_ptr<AudioMediaCacheEntry>> _audioCache;

    mutable std::recursive_mutex _cacheMutex;
};

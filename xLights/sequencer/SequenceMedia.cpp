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
#include <wx/xml/xml.h>
#include <wx/base64.h>
#include <wx/mstream.h>
#include <wx/wfstream.h>
#include <wx/imaggif.h>
#include <wx/anidecod.h>
#include <wx/quantize.h>
#include <wx/filename.h>
#include <wx/file.h>

#include <log4cpp/Category.hh>

#include "../UtilFunctions.h"
#include "../ExternalHooks.h"
#include "../effects/GIFImage.h"

// ImageCacheEntry Implementation
ImageCacheEntry::ImageCacheEntry() : _used(false) {
    invalidImage = std::make_shared<wxImage>(wxImage());
}

ImageCacheEntry::ImageCacheEntry(const std::string &filePath) : _used(false), _filePath(filePath) {
    invalidImage = std::make_shared<wxImage>(wxImage());
    _isEmbedded = false;
}
ImageCacheEntry::ImageCacheEntry(const std::string &path, const std::string &base64Data): _used(false), _filePath(path), _embeddedData(base64Data) {
    invalidImage = std::make_shared<wxImage>(wxImage());
    _isEmbedded = true;
}
ImageCacheEntry::ImageCacheEntry(const std::string &path, const std::vector<wxImage> &imgs, int ft, const std::string &base64Data): _used(false), _filePath(path), _embeddedData(base64Data) {
    invalidImage = std::make_shared<wxImage>(wxImage());
    _isEmbedded = false;
    _imageCount = (int)imgs.size();
    _frameImages.resize(_imageCount);
    _frameImagesNoBG.resize(_imageCount);
    _frameTimes.resize(_imageCount);
    _totalTime = 0;
    for (int x = 0; x < _imageCount; x++) {
        std::shared_ptr<wxImage> i = std::make_shared<wxImage>(imgs[x]);
        _frameImages[x] = i;
        _frameImagesNoBG[x] = i;
        _frameTimes[x] = ft;
        _totalTime += ft;
    }
    _imageSize = imgs[0].GetSize();
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
    wxMemoryBuffer buffer = wxBase64Decode(data.c_str());
    wxFileName fn(_filePath);
    if (buffer[0] == 'G' && buffer[1] == 'I' && buffer[2] == 'F') {
        loadGIF(buffer);
    } else {
        loadImage(buffer);
    }
}
void ImageCacheEntry::LoadFromFile(const std::string& filepath) {
    FileExists(filepath, true);
    wxFileName fn(filepath);
    bool isGif = (fn.GetExt().Lower() == "gif");
    wxFileInputStream stream(filepath);
    if (stream.IsOk()) {
        wxMemoryBuffer buffer;
        uint8_t tempBuf[4096]; // 4KB chunks
        while (!stream.Eof()) {
            stream.Read(tempBuf, sizeof(tempBuf));
            if (stream.LastRead() > 0) {
                buffer.AppendData(tempBuf, stream.LastRead());
            }
        }
        
        if (isGif) {
            loadGIF(buffer);
        } else {
            loadImage(buffer);
        }
        _embeddedData = wxBase64Encode(buffer).ToStdString();
    }
}


int ImageCacheEntry::GetExifOrientation(wxMemoryBuffer& buffer) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    char *data = reinterpret_cast<char*>(buffer.GetData());
    int maxLen = buffer.GetDataLen();
    unsigned char byte1 = data[0];
    unsigned char byte2 = data[1];
    if (byte1 != 0xFF || byte2 != 0xD8) {
        return 1;
    }

    int curPos = 2;
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
            char *ldata = &data[curPos];
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
                logger_base.debug("Invalid TIFF header identifier in %s", (const char*)_filePath.c_str());
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
            if (pos + 2 > ldataSize) return 1;

            unsigned short num_entries = littleEndian ?
                ((unsigned char)ldata[pos + 1] << 8) | (unsigned char)ldata[pos] :
                ((unsigned char)ldata[pos] << 8) | (unsigned char)ldata[pos + 1];
            pos += 2;

            for (unsigned short i = 0; i < num_entries; ++i) {
                if (pos + 12 > ldataSize) break;

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

    // Fallback: wxImage may know the orientation
    wxLogNull logNo;
    wxImage img;
    wxMemoryInputStream ins(data, maxLen);
    img.SetLoadFlags(0);
    if (img.LoadFile(ins, wxBITMAP_TYPE_JPEG)) {
        if (img.HasOption("Orientation")) {
            int orient = img.GetOptionInt("Orientation");
            return orient;
        }
    }
    return 1; // default
}

void ImageCacheEntry::loadGIF(wxMemoryBuffer &ins) {

    wxMemoryInputStream stream(ins.GetData(), ins.GetDataLen());
    
    GIFImage gif(_filePath, stream, false);
    if (gif.IsOk()) {
        _imageCount = gif.GetNumFrames();
        _frameImages.resize(_imageCount);
        _frameImagesNoBG.resize(_imageCount);
        _frameTimes.resize(_imageCount);
        _imageSize = gif.GetImageSize();
        //printf("%s\n", _filePath.c_str());
        for (int x = 0; x < _imageCount; x++) {
            _frameImages[x] = std::make_shared<wxImage>(gif.GetFrame(x));
            _frameTimes[x] = gif.GetFrameTime(x);
            //printf("    Frame %d:   %d\n", x, (int)_frameTimes[x]);
        }
        gif.ResetSuppressBackground(true);
        for (int x = 0; x < _imageCount; x++) {
            _frameImagesNoBG[x] = std::make_shared<wxImage>(gif.GetFrame(x));
        }
        _totalTime = gif.GetTotalTime();
        if (_imageCount == 1) {
            // if only a single image, we can optimize things a bit
            _totalTime = 0;
            _frameTimes[0] = 0;
        }
    }
}
void ImageCacheEntry::loadImage(wxMemoryBuffer &ins) {
    wxImage i;
    wxMemoryInputStream stream(ins.GetData(), ins.GetDataLen());
    i.SetLoadFlags(0);
    if (!i.LoadFile(stream, wxBITMAP_TYPE_ANY, 0)) {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.error("Error loading image file: %s.", (const char*)_filePath.c_str());
        i.Create(5, 5, true);
    }
    _imageCount = 1;
    int orientation = GetExifOrientation(ins);
    ApplyOrientation(i, orientation);
    _frameTimes.push_back(0);
    _frameImages.emplace_back(std::make_shared<wxImage>(i));
    _imageSize = i.GetSize();
}


static std::string PngToBase64(const wxImage& img)
{
    wxMemoryOutputStream mos;
    img.SaveFile(mos, wxBITMAP_TYPE_PNG);
    wxStreamBuffer* buf = mos.GetOutputStreamBuffer();
    return wxBase64Encode(buf->GetBufferStart(), buf->GetIntPosition()).ToStdString();
}

bool ImageCacheEntry::LoadFromXml(wxXmlNode* node)
{
    if (node == nullptr || node->GetName() != "Image") {
        return false;
    }
    
    _filePath = node->GetAttribute("path", "").ToStdString();
    wxXmlNode* child = node->GetChildren();
    if (child == nullptr) return true;

    if (child->GetName() == "Data") {
        // Single-frame embedded (PNG or GIF base64)
        _embeddedData = child->GetNodeContent().ToStdString();
        _isEmbedded = true;
    } else if (child->GetName() == "Frame") {
        // Multi-frame: each <Frame time="ms">base64png</Frame>
        _isEmbedded = true;
        for (wxXmlNode* f = child; f != nullptr; f = f->GetNext()) {
            if (f->GetName() != "Frame") continue;
            long ft = 0;
            f->GetAttribute("time", "0").ToLong(&ft);
            std::string b64 = f->GetNodeContent().ToStdString();
            _frameData.push_back(b64);  // cache for lossless re-save
            wxMemoryBuffer buf = wxBase64Decode(b64);
            wxMemoryInputStream mis(buf.GetData(), buf.GetDataLen());
            auto sp = std::make_shared<wxImage>();
            sp->LoadFile(mis, wxBITMAP_TYPE_PNG);
            _frameImages.push_back(sp);
            _frameImagesNoBG.push_back(sp);
            _frameTimes.push_back(ft);
            _totalTime += ft;
        }
        _imageCount = (int)_frameImages.size();
        if (_imageCount > 0) _imageSize = _frameImages[0]->GetSize();
        _loadingDone = true;
    }
    return true;
}

wxXmlNode* ImageCacheEntry::SaveToXml() const
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "Image");
    node->AddAttribute("path", _filePath);
    
    if (!_isEmbedded) return node;

    if (!_embeddedData.empty()) {
        // Single-frame (or GIF): save as <Data>
        wxXmlNode* dataNode = new wxXmlNode(wxXML_ELEMENT_NODE, "Data");
        dataNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", _embeddedData));
        node->AddChild(dataNode);
    } else if (_imageCount > 0 && !_frameImages.empty()) {
        // Multi-frame: save each as <Frame time="ms">base64png</Frame>
        // Use cached base64 if available to avoid lossy re-encode
        for (int i = 0; i < _imageCount; i++) {
            wxXmlNode* fNode = new wxXmlNode(wxXML_ELEMENT_NODE, "Frame");
            fNode->AddAttribute("time", wxString::Format("%ld", _frameTimes[i]));
            std::string b64;
            if (i < (int)_frameData.size()) {
                b64 = _frameData[i];  // use cached encoding
            } else if (_frameImages[i] && _frameImages[i]->IsOk()) {
                b64 = PngToBase64(*_frameImages[i]);
                // cache for future saves
                if ((int)_frameData.size() == i) _frameData.push_back(b64);
            }
            if (!b64.empty()) {
                fNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", b64));
                node->AddChild(fNode);
            } else {
                delete fNode;
            }
        }
    }
    
    return node;
}
std::shared_ptr<wxImage> ImageCacheEntry::GetFrame(int x, bool suppressGIFBackground) {
    if (x >= _frameImages.size()) {
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

std::shared_ptr<wxImage> ImageCacheEntry::GetScaledImage(int frameNumber, int width, int height, bool suppressedBg) {
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
    
    std::shared_ptr<wxImage> img = GetFrame(frameNumber, suppressedBg);
    if (!img->IsOk()) {
        return img;
    }
    wxImage *image = new wxImage(*img.get());
    image->SetOption(wxIMAGE_OPTION_GIF_TRANSPARENCY, wxIMAGE_OPTION_GIF_TRANSPARENCY_UNCHANGED);
    if (!image->HasAlpha()) {
        image->InitAlpha();
    }
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
    //wasn't found, we'll create it and add to the cache
    std::shared_ptr<ImageCacheEntry> np = std::make_shared<ImageCacheEntry>(filepath);
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
    wxFileName fn(filepath);
    std::string extension = "." + fn.GetExt().ToStdString();
    std::string BasePicture = fn.GetPathWithSep().ToStdString() + fn.GetName().Left(fn.GetName().Length() - 2).ToStdString() + "-";
    int cur = 1;
    std::string fname = BasePicture + std::to_string(cur++) + extension;
    std::vector<wxImage> images;
    while (FileExists(fname)) {
        wxImage i;
        i.SetLoadFlags(0);
        i.LoadFile(fname);
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

void SequenceMedia::AddEmbeddedImage(const std::string& name, const wxImage& image)
{
    // Encode the wxImage as PNG into a memory stream, then base64-encode it
    wxMemoryOutputStream mos;
    if (!image.SaveFile(mos, wxBITMAP_TYPE_PNG)) {
        return;
    }
    wxStreamBuffer* buf = mos.GetOutputStreamBuffer();
    wxMemoryBuffer mb;
    mb.AppendData(buf->GetBufferStart(), buf->GetIntPosition());
    std::string b64 = wxBase64Encode(mb).ToStdString();

    std::shared_ptr<ImageCacheEntry> entry;
    {
        std::scoped_lock lock(_cacheMutex);
        if (_imageCache.find(name) != _imageCache.end()) return; // already exists
        entry = std::make_shared<ImageCacheEntry>(name, b64);
        _imageCache.emplace(name, entry);
    }
    entry->Load();
}

void SequenceMedia::AddEmbeddedImage(const std::string& name, const std::vector<wxImage>& frames, int frameTimeMs)
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
    wxMemoryBuffer buf = wxBase64Decode(_embeddedData);
    if (buf.GetDataLen() == 0) return false;
    wxFile f;
    if (!f.Open(wxString(path), wxFile::write)) return false;
    return f.Write(buf.GetData(), buf.GetDataLen()) == buf.GetDataLen();
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

bool SequenceMedia::LoadFromXml(wxXmlNode* node)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (node == nullptr || (node->GetName() != "SequenceMedia")) {
        return false;
    }
    
    std::scoped_lock lock(_cacheMutex);
    _imageCache.clear();
    
    // Load each image entry
    for (wxXmlNode* imageNode = node->GetChildren(); imageNode != nullptr; imageNode = imageNode->GetNext()) {
        if (imageNode->GetName() == "Image") {
            auto entry = std::make_shared<ImageCacheEntry>();
            if (entry->LoadFromXml(imageNode)) {
                _imageCache[entry->GetFilePath()] = entry;
            } else {
                logger_base.warn("Failed to load image entry from XML");
            }
        }
    }
    
    return true;
}

wxXmlNode* SequenceMedia::SaveToXml() const
{
    std::scoped_lock lock(_cacheMutex);

    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "SequenceMedia");
    // Save each image entry
    for (const auto& pair : _imageCache) {
        if (pair.second->IsEmbedded()) {
            wxXmlNode* imageNode = pair.second->SaveToXml();
            node->AddChild(imageNode);
        }
    }
    
    return node;
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
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    
    std::scoped_lock lock(_cacheMutex);
    
    // Remove unused static images
    std::vector<std::string> toRemove;
    
    for (const auto& pair : _imageCache) {
        if (!pair.second->IsUsed()) {
            toRemove.push_back(pair.first);
        }
    }
    
    for (const auto& path : toRemove) {
        logger_base.debug("Removing unused image from cache: %s", path.c_str());
        _imageCache.erase(path);
    }
}

void SequenceMedia::MarkAllUnused() {
    std::scoped_lock lock(_cacheMutex);
    for (const auto& pair : _imageCache) {
        pair.second->MarkIsUsed(false);
    }
}

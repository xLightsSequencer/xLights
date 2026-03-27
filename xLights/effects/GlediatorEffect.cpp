/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "GlediatorEffect.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <format>

#include "../render/SequenceElements.h"
#include "../render/SequenceMedia.h"
#include "../utils/string_utils.h"

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../render/SequenceFile.h"
#include "../xLightsMain.h" 
#include "../UtilFunctions.h"
#include "../ui/wxUtilities.h"
#include "../ExternalHooks.h"

#include "../../include/glediator-16.xpm"
#include "../../include/glediator-64.xpm"
#include <log.h>
#include "../UtilFunctions.h"
#include "../ui/wxUtilities.h"

GlediatorReader::GlediatorReader(const std::string& filename, const xlSize& size)
{
    _filename = filename;
    _size = size;
    _frames = 0;

    _f.open(_filename, std::ios::in | std::ios::binary);

    if (_f.is_open())
    {
        _f.seekg(0, std::ios::end);
        auto fileSize = static_cast<size_t>(_f.tellg());
        _f.seekg(0, std::ios::beg);

        _frames = fileSize / GetBufferSize();

        if (_frames * GetBufferSize() != fileSize)
        {
            spdlog::warn("Opening glediator file {} size ({},{}) looks suspicious as it does not match file size {}.", _filename.c_str(), _size.width, _size.height, (long)fileSize);
        }
    }
    else
    {
        spdlog::warn("Failed to open file {}", _filename.c_str());
    }
}

CSVReader::CSVReader(const std::string& filename)
{
    _line = -1;
    _filename = filename;

    std::ifstream f(_filename, std::ios::in);

    if (f.is_open())
    {
        std::string line;
        while (std::getline(f, line))
        {
            _lines.push_back(line);
        }
        f.close();
    }
    else
    {
        spdlog::warn("Failed to open file {}", _filename.c_str());
    }
}

GlediatorReader::~GlediatorReader()
{
    if (_f.is_open())
    {
        _f.close();
    }
}

CSVReader::~CSVReader()
{
    _lines.clear();
}

void GlediatorReader::GetFrame(size_t frame, char* buffer, size_t size)
{
    if (size != GetBufferSize() || frame >= GetFrameCount())
    {
        // invalid sized buffer ... so fill it with red
        // or illegal frame
        for (size_t i = 0; i < size; i++)
        {
            if (i%3 == 0)
            {
                *(buffer + i) = (char)0xFF;
            }
            else
            {
                *(buffer + i) = (char)0x00;
            }
        }
    }
    else
    {
        size_t offset = frame * GetBufferSize();
        _f.seekg(offset, std::ios::beg);
        _f.read(buffer, size); // Read one period of channels

        assert(static_cast<size_t>(_f.gcount()) == size);
    }
}

void CSVReader::GetFrame(size_t frame, char* buffer, size_t size)
{
    if (frame >= _lines.size())
        return;

    auto data = Split(_lines[frame], ',');

    for (size_t i = 0; i < std::min(data.size(), size); i++)
    {
        *(buffer + i) = static_cast<char>(std::strtol(data[i].c_str(), nullptr, 10));
    }
}

size_t CSVReader::GetFrameCount() const
{
    return _lines.size();
}

GlediatorEffect::GlediatorEffect(int id) : RenderableEffect(id, "Glediator", glediator_16, glediator_64, glediator_64, glediator_64, glediator_64)
{
    //ctor
}

GlediatorEffect::~GlediatorEffect()
{
    //dtor
}

std::list<std::string> GlediatorEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    std::string GledFilename = settings.Get("E_FILEPICKERCTRL_Glediator_Filename", "");

    if (GledFilename.empty()) {
        res.push_back(std::format("    ERR: Glediator effect cant find file '{}'. Model '{}', Start {}", GledFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    } else {
        auto& mm = eff->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
        auto entry = mm.GetBinaryFile(GledFilename, "glediator");
        entry->MarkIsUsed();

        if (!entry->isLoaded()) {
            res.push_back(std::format("    ERR: Glediator effect cant find file '{}'. Model '{}', Start {}", GledFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
        } else if (!entry->IsEmbedded()) {
            if (!IsFileInShowDir(std::string(), GledFilename)) {
                res.push_back(std::format("    WARN: Glediator effect file '{}' not under show directory. Model '{}', Start {}", GledFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
            }
        }
    }

    return res;
}

void GlediatorEffect::SetSequenceElements(SequenceElements *els) {
    mSequenceElements = els;
}

bool GlediatorEffect::IsGlediatorFile(std::string filename)
{
    auto ext = std::filesystem::path(filename).extension().string();
    if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "gled" ||
        ext == "csv" ||
        ext == "out"
        )
    {
        return true;
    }

    return false;
}

bool GlediatorEffect::IsCSVFile(std::string filename) const
{
    auto ext = std::filesystem::path(filename).extension().string();
    if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "csv")
    {
        return true;
    }

    return false;
}

void GlediatorEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }

    SettingsMap &settings = effect->GetSettings();

    std::string file = settings.Get("E_TEXTCTRL_Glediator_Filename", "");

    if (file != "")
    {
        settings.erase("E_TEXTCTRL_Glediator_Filename");
        settings["E_FILEPICKERCTRL_Glediator_Filename"] = file;
    }

    // Resolve broken paths first, then convert to relative for portability
    file = settings["E_FILEPICKERCTRL_Glediator_Filename"];
    if (!file.empty() && !FileExists(file)) {
        std::string fixed = FixFile("", file);
        if (!fixed.empty() && fixed != file) {
            settings["E_FILEPICKERCTRL_Glediator_Filename"] = fixed;
            file = fixed;
        }
    }
    if (!file.empty()) {
        if (std::filesystem::path(file).is_absolute()) {
            if (!FileExists(file, false)) {
                std::string fixed = FixFile("", file);
                std::string rel = MakeRelativeFile(fixed);
                settings["E_FILEPICKERCTRL_Glediator_Filename"] = rel.empty() ? fixed : rel;
            } else {
                std::string rel = MakeRelativeFile(file);
                if (!rel.empty())
                    settings["E_FILEPICKERCTRL_Glediator_Filename"] = rel;
            }
        }
        // Register with SequenceMedia so it appears in the Media tab
        auto& media = effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
        media.GetBinaryFile(settings["E_FILEPICKERCTRL_Glediator_Filename"], "glediator");
    }
}

std::list<std::string> GlediatorEffect::GetFileReferences(Model* model, const SettingsMap &SettingsMap) const 
{
    std::list<std::string> res;
    if (SettingsMap["E_FILEPICKERCTRL_Glediator_Filename"] != "") {
        res.push_back(SettingsMap["E_FILEPICKERCTRL_Glediator_Filename"]);
    }
    return res;
}

bool GlediatorEffect::CleanupFileLocations(xLightsFrame* frame, SettingsMap &SettingsMap)
{
    bool rc = false;
    std::string file = SettingsMap["E_FILEPICKERCTRL_Glediator_Filename"];
    if (FileExists(file))
    {
        if (!frame->IsInShowFolder(file))
        {
            SettingsMap["E_FILEPICKERCTRL_Glediator_Filename"] = frame->MoveToShowFolder(file, std::string(1, std::filesystem::path::preferred_separator) + "Glediator");
            rc = true;
        }
    }

    return rc;
}

class GlediatorRenderCache : public EffectRenderCache {
public:
    GlediatorRenderCache()
    {
        _glediatorReader = nullptr;
        _csvReader = nullptr;
        _loops = 0;
        _frameMS = 50.0;
    };
    virtual ~GlediatorRenderCache() {
        if (_glediatorReader != nullptr)
        {
            delete _glediatorReader;
            _glediatorReader = nullptr;
        }
        if (_csvReader != nullptr)
        {
            delete _csvReader;
            _csvReader = nullptr;
        }
    };

    GlediatorReader* _glediatorReader;
    CSVReader* _csvReader;
    int _loops;
    float _frameMS;
};

void GlediatorEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer)
{
    

    std::string glediatorFilename = SettingsMap["FILEPICKERCTRL_Glediator_Filename"];
    std::string durationTreatment = SettingsMap["CHOICE_Glediator_DurationTreatment"];

    GlediatorRenderCache *cache = (GlediatorRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new GlediatorRenderCache();
        buffer.infoCache[id] = cache;
    }

    int &_loops = cache->_loops;
    GlediatorReader* &_glediatorReader = cache->_glediatorReader;
    CSVReader* &_csvReader = cache->_csvReader;
    float& _frameMS = cache->_frameMS;

    if (buffer.needToInit)
    {
        buffer.needToInit = false;

        _loops = 0;
        _frameMS = buffer.frameTimeInMs;
        if (_glediatorReader != nullptr)
        {
            delete _glediatorReader;
            _glediatorReader = nullptr;
        }
        if (_csvReader != nullptr)
        {
            delete _csvReader;
            _csvReader = nullptr;
        }

        auto* sm = buffer.GetSequenceMedia();
        if (sm == nullptr) return;
        auto binaryEntry = sm->GetBinaryFile(glediatorFilename, "glediator");
        if (!binaryEntry) return;
        binaryEntry->MarkIsUsed();
        // Use resolved path so the readers can find the file on disk
        std::string resolvedFile = binaryEntry->GetFilePath();
        if (binaryEntry->isLoaded())
        {
            if (IsCSVFile(glediatorFilename))
            {
                _csvReader = new CSVReader(resolvedFile);

                if (_csvReader == nullptr)
                {
                    spdlog::warn("GlediatorEffect: Failed to load csv file {}.", (const char *)glediatorFilename.c_str());
                }
                else
                {
                    size_t glediatorFrames = _csvReader->GetFrameCount();
                    if (durationTreatment == "Slow/Accelerate")
                    {
                        size_t effectFrames = buffer.curEffEndPer - buffer.curEffStartPer + 1;
                        float speedFactor = (float)glediatorFrames / (float)effectFrames;
                        _frameMS = ((float)buffer.frameTimeInMs * speedFactor);
                    }
                    spdlog::debug("Glediator effect length: {}, glediator length: {}, duration treatment: {}.",
                        (int)(((float)(buffer.curEffEndPer - buffer.curEffStartPer + 1)) * _frameMS), 
                        (int)((float)glediatorFrames * _frameMS),
                        (const char *)durationTreatment.c_str());
                }
            }
            else
            {
                // have to open the file
                _glediatorReader = new GlediatorReader(resolvedFile, xlSize(buffer.BufferWi, buffer.BufferHt));

                if (_glediatorReader == nullptr)
                {
                    spdlog::warn("GlediatorEffect: Failed to load glediator file {}.", (const char *)glediatorFilename.c_str());
                }
                else
                {
                    if (durationTreatment == "Slow/Accelerate")
                    {
                        size_t effectFrames = buffer.curEffEndPer - buffer.curEffStartPer + 1;
                        size_t glediatorFrames = _glediatorReader->GetFrameCount();
                        float speedFactor = (float)glediatorFrames / (float)effectFrames;
                        _frameMS = ((float)buffer.frameTimeInMs * speedFactor);
                    }
                    spdlog::debug("Glediator effect length: {}, glediator length: {}, duration treatment: {}.",
                        (int)(((float)(buffer.curEffEndPer - buffer.curEffStartPer + 1)) * _frameMS), 
                        (int)((float)_glediatorReader->GetFrameCount() * _frameMS),
                        (const char *)durationTreatment.c_str());
                }
            }
        }
        else
        {
            spdlog::warn("GlediatorEffect: Glediator file '{}' not found.", (const char *)glediatorFilename.c_str());
        }
    }

    if (_csvReader != nullptr)
    {
        size_t frameCount = _csvReader->GetFrameCount();
        size_t frame = (float)((buffer.curPeriod - buffer.curEffStartPer) - _loops * frameCount) * _frameMS / (float)(buffer.frameTimeInMs);

        // if we have reached the end and we are to loop
        if (frame >= frameCount && durationTreatment == "Loop")
        {
            // jump back to start and try to read frame again
            _loops++;
            frame = (float)((buffer.curPeriod - buffer.curEffStartPer) - _loops * frameCount) * _frameMS / (float)buffer.frameTimeInMs;
            spdlog::debug("Glediator effect loop #{} at frame {}.", _loops, buffer.curPeriod - buffer.curEffStartPer);
        }

        if (frame >= frameCount)
        {
            // do nothing
        }
        else
        {
            size_t bufsize = buffer.BufferWi * buffer.BufferHt;
            char *frameBuffer = new char[bufsize];
            memset(frameBuffer, 0x00, bufsize);

            if (frameBuffer != nullptr)
            {
                _csvReader->GetFrame(frame, frameBuffer, bufsize);
                xlColor color;

                for (size_t j = 0; j < bufsize; j++)
                {
                    // Loop thru all channels
                    color.Set(frameBuffer[j], frameBuffer[j], frameBuffer[j]);
                    int x = j % buffer.BufferWi;
                    int y = (buffer.BufferHt - 1) - (j / buffer.BufferWi);
                    if (x < buffer.BufferWi && y < buffer.BufferHt && y >= 0)
                    {
                        buffer.SetPixel(x, y, color);
                    }
                }

                delete[] frameBuffer;
            }
        }
    }
    else if (_glediatorReader != nullptr)
    {
        bool rendered = false;

        size_t frame = (float)((buffer.curPeriod - buffer.curEffStartPer) - _loops * (_glediatorReader->GetFrameCount())) * _frameMS / (float)buffer.frameTimeInMs;

        // if we have reached the end and we are to loop
        if (frame >= _glediatorReader->GetFrameCount() && durationTreatment == "Loop")
        {
            // jump back to start and try to read frame again
            _loops++;
            frame = (float)((buffer.curPeriod - buffer.curEffStartPer) - _loops * (_glediatorReader->GetFrameCount())) * _frameMS / (float)buffer.frameTimeInMs;
            spdlog::debug("Glediator effect loop #{} at frame {}.", _loops, buffer.curPeriod - buffer.curEffStartPer);
        }

        if (frame >= _glediatorReader->GetFrameCount())
        {
            for (int y = 0; y < buffer.BufferHt; y++)
            {
                for (int x = 0; x < buffer.BufferWi; x++)
                {
                    buffer.SetPixel(x, y, xlBLACK);
                }
            }
            rendered = true;
        }
        else
        {
            size_t bufsize = _glediatorReader->GetBufferSize();
            char *frameBuffer = new char[bufsize];

            if (frameBuffer != nullptr)
            {
                _glediatorReader->GetFrame(frame, frameBuffer, bufsize);
                xlColor color;

                for (size_t j = 0; j < bufsize; j += 3)
                {
                    // Loop thru all channels
                    color.Set(frameBuffer[j], frameBuffer[j + 1], frameBuffer[j + 2]);
                    int x = (j % (buffer.BufferWi * 3)) / 3;
                    int y = (buffer.BufferHt - 1) - (j / (buffer.BufferWi * 3));
                    if (x < buffer.BufferWi && y < buffer.BufferHt && y >= 0)
                    {
                        buffer.SetPixel(x, y, color);
                    }
                }

                delete[] frameBuffer;
                rendered = true;
            }
        }

        // display a red background to show we have a problem
        if (!rendered)
        {
            for (int y = 0; y < buffer.BufferHt; y++)
            {
                for (int x = 0; x < buffer.BufferWi; x++)
                {
                    buffer.SetPixel(x, y, xlRED);
                }
            }
        }
    }
}


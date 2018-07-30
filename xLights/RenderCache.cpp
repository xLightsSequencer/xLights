#include "RenderCache.h"
#include "sequencer/SequenceElements.h"
#include "RenderBuffer.h"

#include <log4cpp/Category.hh>

#include <wx/filename.h>
#include <wx/dir.h>

#pragma region RenderCache

RenderCache::RenderCache()
{
	_cacheFolder = "";
}

RenderCache::~RenderCache()
{
    Close();
}

void RenderCache::SetSequence(const std::string& sequenceFile)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    Close();
    if (sequenceFile != "")
    {
        wxFileName fn(sequenceFile);
        _cacheFolder = fn.GetPath() + fn.GetPathSeparator() + fn.GetName() + "_RENDER_CACHE";

        if (!wxDir::Exists(_cacheFolder))
        {
            logger_base.debug("Creating render cache folder %s.", (const char *)_cacheFolder.c_str());
            wxDir::Make(_cacheFolder);
        }
        else
        {
            logger_base.debug("Opening render cache folder %s.", (const char *)_cacheFolder.c_str());
        }

        wxDir dir(_cacheFolder);
        wxArrayString files;
        dir.GetAllFiles(_cacheFolder, &files, "*.cache");

        for (auto it = files.begin(); it != files.end(); ++it)
        {
            auto rci = new RenderCacheItem(*it);
            if (rci != nullptr)
            {
                _cache.push_back(rci);
            }
            else
            {
                logger_base.warn("Failed to load cache item %s.", (const char*)it->c_str());
            }
        }

        logger_base.debug("Cache contained %d files.", (int)files.size());
    }
}

RenderCacheItem* RenderCache::GetItem(Effect* effect, RenderBuffer* buffer)
{
    if (!_enabled) return nullptr;
    if (_cacheFolder == "") return nullptr;

    // we cant cache effects with canvas turned on
    for (auto it = effect->GetSettings().begin(); it != effect->GetSettings().end(); ++it)
    {
        if (it->first == "T_CHECKBOX_Canvas" && it->second == "1")
        {
            return nullptr;
        }
    }

    for (auto it = _cache.begin(); it != _cache.end(); ++it)
    {
        if ((*it)->IsMatch(effect, buffer))
        {
            return *it;
        }
    }

    auto rci = new RenderCacheItem(this, effect, buffer);
    _cache.push_back(rci);
    return rci;
}


void RenderCache::Close()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Closing render cache folder %s.", (const char *)_cacheFolder.c_str());

    Purge(false);
    _cacheFolder = "";
}

void RenderCache::CleanupCache(SequenceElements* sequenceElements)
{
    // clean up cache but only for missing effects or wrong start times
    for (auto it = _cache.begin(); it != _cache.end(); ++it)
    {
        bool found = false;

        for (int i = 0; i < sequenceElements->GetElementCount() && !found; i++)
        {
            Element* em = sequenceElements->GetElement(i);
            for (int l = 0; l < em->GetEffectLayerCount() && !found; l++)
            {
                EffectLayer* el = em->GetEffectLayer(l);
                for (int e = 0; e < el->GetEffectCount() && !found; e++)
                {
                    if ((*it)->IsMatch(el->GetEffect(e), nullptr))
                    {
                        found = true;
                    }
                }
            }
        }

        if (!found)
        {
            (*it)->Delete();
        }
    }
}

void RenderCache::Purge(bool dodelete)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (dodelete && _cacheFolder != "")
    {
        logger_base.debug("Purging render cache folder %s.", (const char *)_cacheFolder.c_str());
    }

    while (_cache.size() > 0)
    {
        if (dodelete)
            _cache.front()->Delete();
        delete _cache.front();
        _cache.pop_front();
    }
}

#pragma endregion RenderCache

#pragma region RenderCacheItem

RenderCacheItem::~RenderCacheItem()
{
    PurgeFrames();
}

void RenderCacheItem::PurgeFrames()
{
    for (auto it = _frames.begin(); it != _frames.end(); ++it)
    {
        if (it->second != nullptr)
        {
            free(it->second);
            it->second = nullptr;
        }
    }
    _frames.empty();
}

RenderCacheItem::RenderCacheItem(RenderCache* renderCache, Effect* effect, RenderBuffer* buffer)
{
    _purged = false;
    _frameSize = buffer->BufferWi * buffer->BufferHt;
    std::string file = wxString::Format("%s_%s_%d_%d.cache", effect->GetEffectName(), effect->GetParentEffectLayer()->GetParentElement()->GetName(), effect->GetParentEffectLayer()->GetLayerNumber(), effect->GetStartTimeMS()).ToStdString();
    _cacheFile = renderCache->GetCacheFolder() + wxFileName::GetPathSeparator() + file;
    _properties["Effect"] = effect->GetEffectName();
    _properties["Element"] = effect->GetParentEffectLayer()->GetParentElement()->GetFullName();
    _properties["EffectLayer"] = wxString::Format("%d", effect->GetParentEffectLayer()->GetLayerNumber());
    _properties["StartMS"] = wxString::Format("%d", effect->GetStartTimeMS());
    _properties["EndMS"] = wxString::Format("%d", effect->GetEndTimeMS());
    _properties["Width"] = wxString::Format("%d", buffer->BufferWi);
    _properties["Height"] = wxString::Format("%d", buffer->BufferHt);
    _properties["Frames"] = wxString::Format("%d", buffer->curEffEndPer - buffer->curEffStartPer);
    for (auto it = effect->GetSettings().begin(); it != effect->GetSettings().end(); ++it)
    {
        _properties[it->first] = it->second;
    }
    for (auto it = effect->GetPaletteMap().begin(); it != effect->GetPaletteMap().end(); ++it)
    {
        _properties[it->first] = it->second;
    }
}

bool RenderCacheItem::IsMatch(Effect* effect, RenderBuffer* buffer)
{
    if (_purged) return false;

    if (wxAtoi(_properties.at("StartMS")) != effect->GetStartTimeMS()) return false;

    EffectLayer* el = effect->GetParentEffectLayer();
    if (wxAtoi(_properties.at("EffectLayer")) != el->GetLayerNumber()) return false;

    Element* e = el->GetParentElement();
    if (_properties.at("Element") != e->GetFullName()) return false;

    // at this point it is the right element ... just has something may have changed
    bool ok = true;

    if (wxAtoi(_properties.at("EndMS")) != effect->GetEndTimeMS()) ok = false;
    if (_properties.at("Effect") != effect->GetEffectName()) ok = false;

    if (buffer == nullptr) return true;

    if (wxAtoi(_properties.at("Width")) != buffer->BufferWi) ok = false;
    if (wxAtoi(_properties.at("Height")) != buffer->BufferHt) ok = false;

    if (_properties.size() - 8 != effect->GetSettings().size() + effect->GetPaletteMap().size()) ok = false;

    for (auto it = effect->GetSettings().begin(); ok && it != effect->GetSettings().end(); ++it)
    {
        if (_properties.find(it->first) == _properties.end()) {
            ok = false;
        }
        else
        {
            if (_properties.at(it->first) != it->second) ok = false;
        }
    }

    for (auto it = effect->GetPaletteMap().begin(); ok && it != effect->GetPaletteMap().end(); ++it)
    {
        if (_properties.find(it->first) == _properties.end()) {
            ok = false;
        }
        else
        {
            if (_properties.at(it->first) != it->second) ok = false;
        }
    }

    if (!ok)
    {
        // this should have matched but didnt so this cache item has expired
        Delete();
    }

    return ok;
}

void RenderCacheItem::Delete()
{
    if (wxFile::Exists(_cacheFile))
    {
        wxRemoveFile(_cacheFile);
    }
    _purged = true;
    PurgeFrames();
}

void RenderCacheItem::AddFrame(RenderBuffer* buffer)
{
    int frame = buffer->curPeriod - buffer->curEffStartPer;
    wxASSERT(_frameSize == buffer->BufferWi * buffer->BufferHt);

    unsigned char* frameBuffer = (unsigned char *)malloc(_frameSize * 4);
    unsigned char* pc = frameBuffer;

    for (int y = 0; y < buffer->BufferHt; y++)
    {
        for (int x = 0; x < buffer->BufferWi; x++)
        {
            xlColor c = buffer->GetPixel(x, y);
            *pc = c.red;
            ++pc;
            *pc = c.green;
            ++pc;
            *pc = c.blue;
            ++pc;
            *pc = c.alpha;
            ++pc;
        }
    }

    if (_frames.find(frame) != _frames.end())
    {
        if (_frames[frame] != nullptr)
        {
            free(_frames[frame]);
            _frames[frame] = nullptr;
        }
    }

    _frames[frame] = frameBuffer;

    if (buffer->curPeriod == buffer->curEffEndPer)
    {
        Save();
    }
}

void RenderCacheItem::GetFrame(RenderBuffer* buffer)
{
    int frame = buffer->curPeriod - buffer->curEffStartPer;
    wxASSERT(_frameSize == buffer->BufferWi * buffer->BufferHt);

    if (_frames.find(frame) != _frames.end())
    {
        // its in memory ... read it from there
        unsigned char* pc = _frames.find(frame)->second;

        for (int y = 0; y < buffer->BufferHt; y++)
        {
            for (int x = 0; x < buffer->BufferWi; x++)
            {
                xlColor c(*pc, *(pc + 1), *(pc + 2), *(pc + 3));
                pc += 4;
                buffer->SetPixel(x, y, c);
            }
        }
    }
    else
    {
        // this should never happen if we are done
        wxASSERT(false);
    }
}

void RenderCacheItem::Save()
{
    if (_purged) return;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Saving render cache file %s.", (const char *)_cacheFile.c_str());

    char zero = 0x00;
    wxASSERT(_frameSize >= 0);

    wxFile file;

    if (file.Create(_cacheFile, true))
    {
        // write the header fields
        for (auto it = _properties.begin(); it != _properties.end(); ++it)
        {
            file.Write(it->first);
            file.Write(&zero, 1);
            file.Write(it->second);
            file.Write(&zero, 1);
        }

        file.Write("RC_HEADEREND");
        file.Write(&zero, 1);

        // write the frames
        for (auto it = _frames.begin(); it != _frames.end(); ++it)
        {
            file.Write(it->second, _frameSize * 4);
        }
        file.Close();
    }
    else
    {
        logger_base.warn("    Failed to create file.");
    }
}

bool RenderCacheItem::IsDone(RenderBuffer* buffer) const
{
    int frame = buffer->curPeriod - buffer->curEffStartPer;

    return _frames.find(frame) != _frames.end();
}

RenderCacheItem::RenderCacheItem(const std::string& filename)
{
    _cacheFile = filename;
    wxFileName fn(_cacheFile);
    _purged = false;

    wxFile file;

    if (file.Open(_cacheFile))
    {
        char headerBuffer[8192];
        memset(headerBuffer, 0x00, sizeof(headerBuffer));
        file.Read(headerBuffer, sizeof(headerBuffer));

        char* ps = headerBuffer;

        while (strcmp(ps, "RC_HEADEREND") != 0)
        {
            std::string key(ps);
            ps += strlen(ps) + 1;
            std::string value(ps);
            ps += strlen(ps) + 1;

            _properties[key] = value;
        }
        ps += strlen(ps) + 1;

        _frameSize = wxAtoi(_properties["Width"]) * wxAtoi(_properties["Height"]);
        long frames = wxAtoi(_properties["Frames"]);
        long firstFrameOffset = ps - headerBuffer;

        file.Seek(firstFrameOffset);

        for (int i = 0; i < frames; i++)
        {
            unsigned char* frameBuffer = (unsigned char *)malloc(_frameSize * 4);
            file.Read(frameBuffer, _frameSize * 4);
            _frames[i] = frameBuffer;
        }

        file.Close();
    }
}

#pragma endregion RenderCacheItem
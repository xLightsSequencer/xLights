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

void RenderCache::SetSequence(const std::string& path, const std::string& sequenceFile)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    Close();
    if (sequenceFile != "")
    {
        this->_cacheFolder = path + wxFileName::GetPathSeparator() + sequenceFile + "_RENDER_CACHE";

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

        std::unique_lock<std::recursive_mutex> lock(_cacheLock);

        for (auto it = files.begin(); it != files.end(); ++it)
        {
            auto rci = new RenderCacheItem(this, *it);
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
void RenderCache::RemoveItem(RenderCacheItem *item) {
    for (auto it = _cache.begin(); it != _cache.end(); ++it)
    {
        if (item == *it)
        {
            _cache.erase(it);
            break;
        }
    }
    delete item;
}

RenderCacheItem* RenderCache::GetItem(Effect* effect, RenderBuffer* buffer)
{
    if (!_enabled) return nullptr;
    if (_cacheFolder == "") return nullptr;

    for (auto it = effect->GetSettings().begin(); it != effect->GetSettings().end(); ++it) {
        // we cant cache effects with canvas turned on
        if (it->first == "T_CHECKBOX_Canvas" && it->second == "1") {
            return nullptr;
        }

        // we also can't handle per model render styles ... as the buffers keep changing
        if (it->first == "B_CHOICE_BufferStyle" && wxString(it->second).StartsWith("Per Model")) {
            return nullptr;
        }
    }
 
    std::unique_lock<std::recursive_mutex> lock(_cacheLock);
    for (auto it = _cache.begin(); it != _cache.end(); ++it) {
        if ((*it)->IsMatch(effect, buffer)) {
            RenderCacheItem *item = *it;
            _cache.erase(it);
            return item;
        }
    }

    return new RenderCacheItem(this, effect, buffer);
}


void RenderCache::Close()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Closing render cache folder %s.", (const char *)_cacheFolder.c_str());

    Purge(nullptr, false);
    _cacheFolder = "";
}


static void purgeCache(Element *em, bool del) {
    for (int l = 0; l < em->GetEffectLayerCount(); l++) {
        EffectLayer* el = em->GetEffectLayer(l);
        for (int e = 0; e < el->GetEffectCount(); e++) {
            el->GetEffect(e)->PurgeCache(del);
        }
    }
    if (em->GetType() == ELEMENT_TYPE_MODEL) {
        ModelElement *me = (ModelElement*)em;
        for (int x = 0; x < me->GetSubModelCount(); x++) {
            purgeCache(me->GetSubModel(x), del);
        }
    }
}
static bool findMatch(Element *em, RenderCacheItem* item) {
    for (int l = 0; l < em->GetEffectLayerCount(); l++) {
        EffectLayer* el = em->GetEffectLayer(l);
        for (int e = 0; e < el->GetEffectCount(); e++) {
            if (item->IsMatch(el->GetEffect(e), nullptr)) {
                return true;
            }
        }
    }
    if (em->GetType() == ELEMENT_TYPE_MODEL) {
        ModelElement *me = (ModelElement*)em;
        for (int x = 0; x < me->GetSubModelCount(); x++) {
            if (findMatch(me->GetSubModel(x), item)) {
                return true;
            }
        }
    }
    return false;
}
void RenderCache::CleanupCache(SequenceElements* sequenceElements)
{
    // clean up cache but only for missing effects or wrong start times
    std::unique_lock<std::recursive_mutex> lock(_cacheLock);
    for (auto it = _cache.begin(); it != _cache.end(); ++it) {
        bool found = false;

        for (int i = 0; i < sequenceElements->GetElementCount() && !found; i++) {
            Element* em = sequenceElements->GetElement(i);
            found = findMatch(em, *it);
        }

        if (!found) {
            (*it)->Delete();
        }
    }
    
    for (int i = 0; i < sequenceElements->GetElementCount(); i++) {
        Element* em = sequenceElements->GetElement(i);
        purgeCache(em, false);
    }

}

void RenderCache::Purge(SequenceElements* sequenceElements, bool dodelete)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (dodelete && _cacheFolder != "")
    {
        logger_base.debug("Purging render cache folder %s.", (const char *)_cacheFolder.c_str());
    }

    std::unique_lock<std::recursive_mutex> lock(_cacheLock);
    while (_cache.size() > 0)
    {
        if (dodelete)
        {
            _cache.front()->Delete();
        }
        else
        {
            _cache.front()->Save();
            delete _cache.front();
            _cache.pop_front();
        }
    }
    if (sequenceElements) {
        for (int i = 0; i < sequenceElements->GetElementCount(); i++) {
            Element* em = sequenceElements->GetElement(i);
            purgeCache(em, dodelete);
        }
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
    _purged = true;
    for (int x = _frames.size() - 1; x >= 0; --x) {
        if (_frames[x]) {
            free(_frames[x]);
            _frames[x] = nullptr;
        }
    }
}

RenderCacheItem::RenderCacheItem(RenderCache* renderCache, Effect* effect, RenderBuffer* buffer) : _renderCache(renderCache)
{
    _purged = false;
    _dirty = true;
    _frameSize = sizeof(xlColor) * buffer->pixels.size() ;
    wxString elname = effect->GetParentEffectLayer()->GetParentElement()->GetFullName();
    elname.Replace("/", "_");
    elname.Replace("\\", "_");
    elname.Replace(":", "_");
    elname.Replace("?", "_");
    elname.Replace("*", "_");
    elname.Replace("$", "_");
    std::string file = wxString::Format("%s_%s_%d_%d.cache", effect->GetEffectName(), elname, effect->GetParentEffectLayer()->GetLayerNumber(), effect->GetStartTimeMS()).ToStdString();
    _cacheFile = renderCache->GetCacheFolder() + wxFileName::GetPathSeparator() + file;
    _properties["Effect"] = effect->GetEffectName();
    _properties["Element"] = effect->GetParentEffectLayer()->GetParentElement()->GetFullName();
    _properties["EffectLayer"] = wxString::Format("%d", effect->GetParentEffectLayer()->GetLayerNumber());
    _properties["StartMS"] = wxString::Format("%d", effect->GetStartTimeMS());
    _properties["EndMS"] = wxString::Format("%d", effect->GetEndTimeMS());
    _properties["Width"] = wxString::Format("%d", buffer->BufferWi);
    _properties["Height"] = wxString::Format("%d", buffer->BufferHt);
    _properties["Frames"] = wxString::Format("%d", buffer->curEffEndPer - buffer->curEffStartPer + 1);
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

    return ok;
}

void RenderCacheItem::Delete()
{
    if (!_purged && wxFile::Exists(_cacheFile)) {
        wxRemoveFile(_cacheFile);
    }
    PurgeFrames();
    _renderCache->RemoveItem(this);
}

void RenderCacheItem::AddFrame(RenderBuffer* buffer)
{
    int frame = buffer->curPeriod - buffer->curEffStartPer;
    wxASSERT(_frameSize == (sizeof(xlColor) * buffer->pixels.size()));
    _frameSize = sizeof(xlColor) * buffer->pixels.size();
    
    if (frame >= _frames.size()) {
        int maxframe = buffer->curEffEndPer - buffer->curEffStartPer + 1;
        _frames.resize(maxframe);
    }
    
    unsigned char* frameBuffer = (unsigned char *)malloc(_frameSize);
    memcpy(frameBuffer, &buffer->pixels[0], _frameSize);
    
    if (_frames[frame]) {
        free(_frames[frame]);
        _frames[frame] = nullptr;
    }

    _frames[frame] = frameBuffer;
    _dirty = true;

    if (buffer->curPeriod == buffer->curEffEndPer)
    {
        Save();
    }
}

bool RenderCacheItem::GetFrame(RenderBuffer* buffer)
{
    int frame = buffer->curPeriod - buffer->curEffStartPer;
    wxASSERT(_frameSize == (sizeof(xlColor) * buffer->pixels.size()));

    if (frame < _frames.size() && _frames[frame]) {
        // its in memory ... read it from there
        unsigned char* pc = _frames[frame];
        memcpy(&buffer->pixels[0], pc, _frameSize);
        return true;
    }
    return false;
}

void RenderCacheItem::Save()
{
    if (_purged) return;
    if (!_dirty) return;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //logger_base.debug("Saving render cache file %s.", (const char *)_cacheFile.c_str());

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
            file.Write(*it, _frameSize);
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
    return _frames[frame];
}

RenderCacheItem::RenderCacheItem(RenderCache* renderCache, const std::string& filename) : _renderCache(renderCache)
{
    _cacheFile = filename;
    wxFileName fn(_cacheFile);
    _purged = false;
    _dirty = false;

    wxFile file;

    if (file.Open(_cacheFile)) {
        char headerBuffer[8192];
        memset(headerBuffer, 0x00, sizeof(headerBuffer));
        file.Read(headerBuffer, sizeof(headerBuffer));

        char* ps = headerBuffer;

        while (strcmp(ps, "RC_HEADEREND") != 0) {
            std::string key(ps);
            ps += strlen(ps) + 1;
            std::string value(ps);
            ps += strlen(ps) + 1;

            _properties[key] = value;
        }
        ps += strlen(ps) + 1;

        _frameSize = wxAtoi(_properties["Width"]) * wxAtoi(_properties["Height"]) * sizeof(xlColor);
        long frames = wxAtoi(_properties["Frames"]);
        _frames.resize(frames);
        long firstFrameOffset = ps - headerBuffer;

        file.Seek(firstFrameOffset);

        for (int i = 0; i < frames; i++) {
            unsigned char* frameBuffer = (unsigned char *)malloc(_frameSize);
            file.Read(frameBuffer, _frameSize);
            _frames[i] = frameBuffer;
        }

        file.Close();
    }
}

#pragma endregion RenderCacheItem

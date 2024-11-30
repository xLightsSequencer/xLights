/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderCache.h"
#include "sequencer/SequenceElements.h"
#include "RenderBuffer.h"
#include "models/Model.h"

#include <log4cpp/Category.hh>

#include <wx/filename.h>
#include <wx/dir.h>
#include <functional>
#include "xLightsVersion.h"
#include "UtilFunctions.h"
#include "TraceLog.h"
#include "ExternalHooks.h"

#ifdef __WXOSX__
#include <sys/mman.h>
#define USE_MMAP_RENDERCACHE
#endif

#pragma region RenderCache

class RenderCacheLoadThread : public wxThread
{
public:
    RenderCacheLoadThread(RenderCache* cache)
    {
        _cache = cache;
        Run();
    }

private:
    RenderCache* _cache;
    virtual void* Entry() override
    {
        std::unique_lock<std::mutex> lock(_cache->GetLoadMutex());

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        logger_base.debug("Loading cache.");

        wxString cacheFolder = _cache->GetCacheFolder();

        wxDir dir(cacheFolder);
        wxArrayString files;
        GetAllFilesInDir(cacheFolder, files, "*.cache");

        for (const auto& it : files) {
            // allow up to 3 times physical memory
            // This means the render cache will be swapped out ... but I think that is still better than re-rendering
            // Abandon loading render cache if we use too much memory
            if (IsExcessiveMemoryUsage(3.0)) {
                logger_base.warn("Render cache loading abandoned due to too much memory use.");
                break;
            }

            auto rci = new RenderCacheItem(_cache, it);
            if (rci != nullptr && !rci->IsPurged()) {
                _cache->AddCacheItem(rci);
            } else {
                if (rci != nullptr) {
                    delete rci;
                }
                logger_base.warn("Failed to load cache item %s.", (const char*)it.c_str());
            }
        }

        logger_base.debug("Cache contained %d files.", (int)files.size());
        TraceLog::ClearTraceMessages();
        return nullptr;
    }
};

RenderCache::RenderCache()
{
    _enabled = true;
	_cacheFolder = "";
}

RenderCache::~RenderCache()
{
    Close();

    EnforceMaximumSize();
}

void RenderCache::SetMaximumSizeMB(size_t mb)
{
    _maximumSizeMB = mb;
    EnforceMaximumSize();
}

void RenderCache::EnforceMaximumSize()
{
    // zero means no limit
    if (_maximumSizeMB == 0)
        return;

    if (_baseCache == "")
        return;

    if (!wxDir::Exists(_baseCache))
        return;

    wxDir dir(_baseCache);
    wxULongLong total = dir.GetTotalSize(_baseCache);
    if ((total / 1024 / 1024).ToULong() < _maximumSizeMB)
        return;

    // get the size and last written date of all render cache entries
    typedef struct CACHE_ENTRY {
        wxULongLong size = 0;
        std::string name;
        wxDateTime modified;

        bool operator<(const struct CACHE_ENTRY& ce) const
        {
            return modified < ce.modified;
        }
    } CACHE_ENTRY;

    std::list<CACHE_ENTRY> entries;

    // wxArrayString dirs;
    // GetAllFilesInDir(_baseCache, dirs, wxALL_FILES_PATTERN, wxDIR_DIRS);

    // each sequence is in a directory
    // for (const auto& d : dirs) {
    wxArrayString files;
    GetAllFilesInDir(_baseCache, files, "*.cache", wxDIR_FILES | wxDIR_DIRS);

    for (const auto& f : files) {
        wxFileName fn(f);
        CACHE_ENTRY ce;
        ce.name = f;
        ce.size = fn.GetSize();
        ce.modified = fn.GetModificationTime();

        if ((ce.size / 1024 / 1024).ToULong() > _maximumSizeMB / 2) {
            // we always delete anything larger than half the maximum as these are essentially making the cache useless
            wxRemoveFile(ce.name);
            total -= ce.size;
        } else {
            entries.push_back(ce);
        }
    }
    //}

    entries.sort();

    while ((total / 1024 / 1024).ToULong() > _maximumSizeMB && entries.size() > 0) {
        if (wxFile::Exists(entries.front().name)) {
            wxRemoveFile(entries.front().name);
            total -= entries.front().size;
        }
        entries.pop_front();
    }
}

void RenderCache::LoadCache()
{
    // the thread self deletes so we dont need to track it
    if (IsEnabled())
    {
        wxASSERT(GetBitness() != "32bit");
        new RenderCacheLoadThread(this);
    }
}

void RenderCache::AddCacheItem(RenderCacheItem* rci)
{
    if (rci != nullptr) {
        static log4cpp::Category& logger_rcache = log4cpp::Category::getInstance(std::string("log_rendercache"));
        logger_rcache.info("RenderCache item added " + rci->Description());
        PerEffectCache *cache = GetPerEffectCache(rci->EffectName());
        std::unique_lock<std::shared_mutex> lock(cache->lock);
        cache->cache.push_back(rci);
    }
}

void RenderCache::SetSequence(const std::string& path, const std::string& sequenceFile)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    Close();

    if (path != "") {
        _baseCache = path + GetPathSeparator() + "RenderCache";
        EnforceMaximumSize();
    }

    if (!IsEnabled())
    {
        if (sequenceFile != "")
        {
            _cacheFolder = path + GetPathSeparator() + "RenderCache" + GetPathSeparator() + sequenceFile + "_RENDER_CACHE";
            if (wxDir::Exists(_cacheFolder))
            {
                if (GetBitness() == "32bit")
                {
                    logger_base.debug("Render cache disabled but NOT removing folder %s as this is the 32 bt version.", (const char *)_cacheFolder.c_str());
                }
                else
                {
                    logger_base.debug("Render cache disabled so removing folder %s.", (const char *)_cacheFolder.c_str());
                    wxDir::Remove(_cacheFolder, wxPATH_RMDIR_RECURSIVE);
                }
            }
        }
        return;
    }

    if (sequenceFile != "")
    {
        _cacheFolder = path + GetPathSeparator() + "RenderCache" + GetPathSeparator() + sequenceFile + "_RENDER_CACHE";

        if (!wxDir::Exists(_cacheFolder))
        {
            wxString common = path + GetPathSeparator() + "RenderCache";
            if (!wxDir::Exists(common))
            {
                logger_base.debug("Creating render cache folder %s.", (const char *)common.c_str());
                wxDir::Make(common);
            }

            logger_base.debug("Creating render cache folder %s.", (const char *)_cacheFolder.c_str());
            wxDir::Make(_cacheFolder);
        }
        else
        {
            logger_base.debug("Opening render cache folder %s.", (const char *)_cacheFolder.c_str());
        }

        LoadCache();
    }
}

void RenderCache::RemoveItem(RenderCacheItem *item) {
    static log4cpp::Category& logger_rcache = log4cpp::Category::getInstance(std::string("log_rendercache"));
    PerEffectCache *c = GetPerEffectCache(item->EffectName());
    std::unique_lock<std::shared_mutex> lock(c->lock);
    auto &l = c->cache;
    for (auto it = l.begin(); it != l.end(); ++it) {
        if (item == *it) {
            logger_rcache.info("RenderCache item removed " + (*it)->Description());
            l.erase(it);
            break;
        }
    }
    delete item;
}

bool RenderCache::IsEffectOkForCaching(Effect* effect) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!IsEnabled()) return false;

    bool locked = false;

    for (const auto& it : effect->GetSettings()) {
        // we cant cache effects with canvas turned on
        if (it.first == "T_CHECKBOX_Canvas" && it.second == "1") {
            return false;
        }

        // we also shouldnt cache effects with persistent turned on
        if (it.first == "B_CHECKBOX_OverlayBkg" && it.second == "1") {
            return false;
        }

        if (_enabled == "Locked Only") {
            if (it.first == "X_Effect_Locked" && it.second == "True") {
                locked = true;
            }
        }
    }

    if (_enabled == "Locked Only" && !locked) {
        return false;
    }

    // allow up to 3 times physical memory
    // This means the render cache will be swapped out ... but I think that is still better than re-rendering
    if (IsExcessiveMemoryUsage(3.0)) {
        logger_base.error("RenderCache::IsEffectOkForCaching failed memory available test. This is a bad sign. Rendering will be really slow.");
        return false;
    }

    return true;
}

RenderCache::PerEffectCache* RenderCache::GetPerEffectCache(const std::string &s) {
    std::unique_lock<std::recursive_mutex> lock(_cacheLock);
    PerEffectCache *r = _cache[s];
    if (r == nullptr) {
        r = new PerEffectCache();
        _cache[s] = r;
    }
    return r;
}

RenderCacheItem* RenderCache::GetItem(Effect* effect, RenderBuffer* buffer)
{
    static log4cpp::Category& logger_rcache = log4cpp::Category::getInstance(std::string("log_rendercache"));
    if (!IsEnabled()) return nullptr;
    if (_cacheFolder == "") return nullptr;

    if (!IsEffectOkForCaching(effect)) return nullptr;

    {
        // wait for the cache to finish loading
        std::unique_lock<std::mutex> lock(_loadMutex);
    }

    PerEffectCache *cache = GetPerEffectCache(effect->GetEffectName());
    std::shared_lock<std::shared_mutex> lock(cache->lock);
    auto &l = cache->cache;
    for (auto it = l.begin(); it != l.end(); ++it) {
        if ((*it)->IsMatch(effect, buffer)) {
            RenderCacheItem *item = *it;
            //release the readlock
            lock.unlock();
            
            //grab the write lock
            std::unique_lock<std::shared_mutex> ulock(cache->lock);
            l.erase(it);
            logger_rcache.info("RenderCache GetItem found an existing render cache item for effect %s on model %s on layer %d at start time %dms.",
                (const char*)effect->GetEffectName().c_str(),
                (const char*)buffer->GetModelName().c_str(),
                effect->GetParentEffectLayer()->GetLayerNumber(),
                effect->GetStartTimeMS());

            if (item != nullptr)
                item->Touch();

            return item;
        }
    }
    lock.unlock();

    logger_rcache.info("RenderCache GetItem created a new render cache item for effect %s on model %s on layer %d at start time %dms.",
        (const char*)effect->GetEffectName().c_str(),
        (const char*)buffer->GetModelName().c_str(),
        effect->GetParentEffectLayer()->GetLayerNumber(),
        effect->GetStartTimeMS());

    return new RenderCacheItem(this, effect, buffer);
}

void RenderCache::Close()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_cacheFolder == "") return;

    logger_base.debug("Closing render cache folder %s.", (const char *)_cacheFolder.c_str());

    {
        // wait for the cache to finish loading
        std::unique_lock<std::mutex> lock(_loadMutex);
    }

    logger_base.debug("    Got lock.");

    Purge(nullptr, false);
    _cacheFolder = "";

    std::unique_lock<std::recursive_mutex> lock(_cacheLock);
    for (auto &a : _cache) {
        delete a.second;
        a.second = nullptr;
    }
    _cache.clear();
    logger_base.debug("    Closed.");
}

static bool doOnEffectsInternal(Element *em, std::function<bool(Effect*)>& func) {
    for (int l = 0; l < em->GetEffectLayerCount(); l++) {
        EffectLayer* el = em->GetEffectLayer(l);
        for (int e = 0; e < el->GetEffectCount(); e++) {
            Effect *eff = el->GetEffect(e);
            if (func(eff)) {
                return true;
            }
        }
    }
    if (em->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
        ModelElement *me = (ModelElement*)em;
        for (int x = 0; x < me->GetSubModelCount(); x++) {
            if (doOnEffectsInternal(me->GetSubModel(x), func)) {
                return true;
            }
        }
    } else if (em->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
        StrandElement *se = (StrandElement*)em;
        for (int x = 0; x < se->GetNodeLayerCount(); x++) {
            NodeLayer* el = se->GetNodeLayer(x);
            for (int e = 0; e < el->GetEffectCount(); e++) {
                Effect *eff = el->GetEffect(e);
                if (func(eff)) {
                    return true;
                }
            }
        }
    }
    return false;
}

static bool doOnEffects(Element *em, std::function<bool(Effect*)>&& f) {
    std::function<bool(Effect*)> func = f;
    return doOnEffectsInternal(em, func);
}

static void purgeCache(Element *em, bool del) {
    doOnEffects(em, [del] (Effect* e) {
        e->PurgeCache(del);
        return false;
    });
}

static bool findMatch(Element *em, RenderCacheItem* item) {
    return doOnEffects(em, [item] (Effect* e) {
        return item->IsMatch(e, nullptr);
    });
}

void RenderCache::CleanupCache(SequenceElements* sequenceElements)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Cleaning up the cache.");

    // clean up cache
    // Because effects are removed from the cache then if you go from cache enabled to cache disabled this wont actually
    // clean out all the cache items ... as we dont know about them.
    int deleted = 0;
    std::unique_lock<std::recursive_mutex> lock(_cacheLock);
    for (auto &l : _cache) {
        if (l.second != nullptr) {
            std::unique_lock<std::shared_mutex> ulock(l.second->lock);
            auto it = l.second->cache.begin();
            while (it != l.second->cache.end()) {
                bool found = false;

                for (int i = 0; i < sequenceElements->GetElementCount() && !found; i++) {
                    Element* em = sequenceElements->GetElement(i);
                    found = findMatch(em, *it);
                }

                if (!found) {
                    auto todelete = it;
                    ++it;
                    ulock.unlock();
                    (*todelete)->Delete();
                    ulock.lock();
                    deleted++;
                } else {
                    ++it;
                }
            }
        }
    }
    logger_base.debug("    Cleaned up %d items in the cache.", deleted);

    for (int i = 0; i < sequenceElements->GetElementCount(); ++i) {
        Element* em = sequenceElements->GetElement(i);
        purgeCache(em, false);
    }

    logger_base.debug("    Cache purge done.");
}

void RenderCache::SetRenderCacheFolder(const std::string& path)
{
    _baseCache = path + GetPathSeparator() + "RenderCache";
    EnforceMaximumSize();
}

void RenderCache::Purge(SequenceElements* sequenceElements, bool dodelete)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (dodelete && _cacheFolder != "")
    {
        logger_base.debug("Purging render cache folder %s.", (const char *)_cacheFolder.c_str());
    }

    std::unique_lock<std::recursive_mutex> lock(_cacheLock);
    for (auto &it : _cache) {
        if (it.second != nullptr) {
            auto& l = it.second;
            std::unique_lock<std::shared_mutex> ulock(l->lock);
            while (l->cache.size() > 0) {
                auto frnt = l->cache.front();
                if (dodelete) {
                    ulock.unlock();
                    frnt->Delete();
                    ulock.lock();
                } else {
                    frnt->Save();
                    delete frnt;
                    l->cache.pop_front();
                }
            }
        }
    }

    if (sequenceElements) {
        for (int i = 0; i < sequenceElements->GetElementCount(); i++) {
            Element* em = sequenceElements->GetElement(i);
            purgeCache(em, dodelete);
        }
    }
}
bool RenderCache::UseMMap() const {
#ifdef USE_MMAP_RENDERCACHE
    return true;
#else
    return false;
#endif
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
    for (auto& it : _frames) {
        for (int x = it.second.size() - 1; x >= 0; --x) {
            if (it.second[x] != nullptr) {
                if (!_mmap) {
                    free(it.second[x]);
                }
                it.second[x] = nullptr;
            }
        }
    }
#ifdef USE_MMAP_RENDERCACHE
    if (_mmap) {
        munmap(_mmap, _mmapSize);
        _mmap = nullptr;
        _mmapSize = 0;
    }
#endif
}

std::string RenderCacheItem::GetModelName(RenderBuffer* buffer)
{
    if (buffer == nullptr) {
        return "";
    } else {
        wxString mname = buffer->GetModelName();
        mname.Replace("/", "_");
        mname.Replace("\\", "_");
        mname.Replace(":", "_");
        mname.Replace("?", "_");
        mname.Replace("*", "_");
        mname.Replace("$", "_");
        return mname.ToStdString();
    }
}

RenderCacheItem::RenderCacheItem(RenderCache* renderCache, Effect* effect, RenderBuffer* buffer) : _renderCache(renderCache)
{
    _mmap = nullptr;
    _mmapSize = 0;
    _purged = false;
    _dirty = true;
    std::string mname = GetModelName(buffer);
    wxASSERT(mname != "");
    _frameSize[mname] = sizeof(xlColor) * buffer->GetPixelCount();
    wxString elname = effect->GetParentEffectLayer()->GetParentElement()->GetFullName();
    elname.Replace("/", "_");
    elname.Replace("\\", "_");
    elname.Replace(":", "_");
    elname.Replace("?", "_");
    elname.Replace("*", "_");
    elname.Replace("$", "_");
    std::string file = wxString::Format("%s_%s_%d_%d.cache",
            effect->GetEffectName(),
            elname,
            effect->GetParentEffectLayer()->GetLayerNumber(),
            effect->GetStartTimeMS()).ToStdString();
    _effectName = effect->GetEffectName();
    _cacheFile = renderCache->GetCacheFolder() + GetPathSeparator() + file;
    _properties["Effect"] = effect->GetEffectName();
    _properties["Element"] = effect->GetParentEffectLayer()->GetParentElement()->GetFullName();
    _properties["EffectLayer"] = wxString::Format("%d", effect->GetParentEffectLayer()->GetLayerNumber());
    _properties["StartMS"] = wxString::Format("%d", effect->GetStartTimeMS());
    _properties["EndMS"] = wxString::Format("%d", effect->GetEndTimeMS());
    _properties["Frames"] = wxString::Format("%d", buffer->curEffEndPer - buffer->curEffStartPer + 1);
    _properties["Models"] = "-1";
    for (const auto& it : effect->GetSettings())
    {
        _properties[it.first] = it.second;
    }
    for (const auto& it : effect->GetPaletteMap())
    {
        _properties[it.first] = it.second;
    }
}

bool RenderCacheItem::IsMatch(Effect* effect, RenderBuffer* buffer)
{
    static log4cpp::Category& logger_rcache = log4cpp::Category::getInstance(std::string("log_rendercache"));
    if (_purged) return false;

    if (wxAtoi(_properties.at("StartMS")) != effect->GetStartTimeMS()) return false;

    EffectLayer* el = effect->GetParentEffectLayer();
    if (wxAtoi(_properties.at("EffectLayer")) != el->GetLayerNumber()) return false;

    Element* e = el->GetParentElement();
    if (_properties.at("Element") != e->GetFullName()) return false;

    if (buffer != nullptr)
    {
        std::string mname = GetModelName(buffer);
        if (_frameSize.at(mname) != sizeof(xlColor) * buffer->GetPixelCount()) return false;
    }

    if (wxAtoi(_properties.at("EndMS")) != effect->GetEndTimeMS()) return false;
    if (_properties.at("Effect") != effect->GetEffectName()) return false;

    // We only log failures from here on because they should be relatively rare

    // 8 is the number of predefined tags
    if (_properties.size() - 7 != effect->GetSettings().size() + effect->GetPaletteMap().size())
    {
        logger_rcache.debug("RenderCache no mantch because number of proprerties different.");
        return false;
    }

    for (const auto& it : effect->GetSettings())
    {
        if (_properties.find(it.first) == _properties.end()) {
            logger_rcache.debug("RenderCache no match because proprerty not present: " + it.first);
            return false;
        }
        else
        {
            if (_properties.at(it.first) != it.second)
            {
                logger_rcache.debug("RenderCache no match because proprerty different: " + it.first);
                return false;
            }
        }
    }

    for (const auto& it : effect->GetPaletteMap())
    {
        if (_properties.find(it.first) == _properties.end()) {
            logger_rcache.debug("RenderCache no match because pallette map not present: " + it.first);
            return false;
        }
        else
        {
            if (_properties.at(it.first) != it.second)
            {
                logger_rcache.debug("RenderCache no match because pallette map different: " + it.first);
                return false;
            }
        }
    }

    return true;
}

void RenderCacheItem::Delete()
{
    static log4cpp::Category& logger_rcache = log4cpp::Category::getInstance(std::string("log_rendercache"));
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxLogNull logNo; //kludge: avoid user error messahe
    if (!_purged && FileExists(_cacheFile)) {
        if (!wxRemoveFile(_cacheFile)) {
            logger_base.warn("Unable to remove cache file " + _cacheFile);
        } else {
            logger_rcache.info("RenderCache removed file " + _cacheFile);
        }
    }
    PurgeFrames();
    _renderCache->RemoveItem(this);
}

void RenderCacheItem::AddFrame(RenderBuffer* buffer)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (buffer == nullptr) {
        logger_base.error("RenderCacheItem::AddFrame was passed a null buffer");
        return;
    }
    
    if (buffer->GetPixelCount() == 0) {
        logger_base.error("RenderCacheItem::AddFrame was passed a buffer with no pixels in it");
        return;
    }

    if (_purged) {
        return;
    }
    if (_mmap) {
        // need to undo the mmap so we can append frames
        unmmap();
    }
    // allow up to 3 times physical memory
    // This means the render cache will be swapped out ... but I think that is still better than re-rendering
    if (IsExcessiveMemoryUsage(3.0)) {
        logger_base.error("RenderCacheItem::AddFrame failed memory available test. This is a bad sign. Rendering will be really slow.");
        PurgeFrames();
        return;
    }

    int frame = buffer->curPeriod - buffer->curEffStartPer;

    std::string mname = GetModelName(buffer);
    if (_frameSize.find(mname) == _frameSize.end()) {
        _frameSize[mname] = sizeof(xlColor) * buffer->GetPixelCount();
    } else {
        if (_frameSize[mname] != sizeof(xlColor) * buffer->GetPixelCount()) {
            // the buffer size has changed ... we dont support this.
            logger_base.warn("RenderCacheItem::AddFrame buffer size changed ... we dont support this.");
            PurgeFrames();
            return;
        }
    }

    size_t totFramesSize = buffer->curEffEndPer - buffer->curEffStartPer + 1;
    totFramesSize *= _frameSize[mname];
    constexpr size_t MAX = 4LL * 1024LL * 1024LL * 1024LL;
    if (totFramesSize > MAX) {
        // more that 4GB in size, we're not going to cache this effect
        PurgeFrames();
        return;
    }

    if (_frames.find(mname) == _frames.end()) {
        std::vector<unsigned char *> n;
        _frames[mname] = n;
    }

    if (frame >= _frames.at(mname).size()) {
        int maxframe = std::max(frame+1,buffer->curEffEndPer - buffer->curEffStartPer + 1);
        _frames.at(mname).resize(maxframe);
    }

    unsigned char* frameBuffer = (unsigned char *)malloc(_frameSize.at(mname));
    if (frameBuffer == nullptr) {
        logger_base.warn("RenderCacheItem::AddFrame failed to allocate frameBuffer.");
        PurgeFrames();
        wxASSERT(false);
        return;
    }
    memcpy(frameBuffer, buffer->GetPixels(), _frameSize.at(mname));

    if (_frames.at(mname)[frame] != nullptr) {
        free(_frames.at(mname)[frame]);
        _frames.at(mname)[frame] = nullptr;
    }

    _frames.at(mname)[frame] = frameBuffer;
    _dirty = true;

    if (buffer->curPeriod == buffer->curEffEndPer) {
        // if multi models in this cache then only call save when none of them have null pointers at the end
        for (const auto& itm : _frames) {
            if (itm.second.size() == 0 || itm.second.back() == nullptr) {
                //logger_base.warn("RenderCacheItem::AddFrame save abandoned due to null frame.");
                return;
            }
        }
        Save();
    }
}

bool RenderCacheItem::GetFrame(RenderBuffer* buffer)
{
    static log4cpp::Category& logger_rcache = log4cpp::Category::getInstance(std::string("log_rendercache"));
    std::string mname = GetModelName(buffer);
    if (_frameSize.find(mname) == _frameSize.end()) {
        logger_rcache.info("RenderCache::GetFrame on model " + mname + " failed due to number of frames difference.");
        return false;
    }

    auto modelFrames = _frames[mname];
    if (_frameSize.at(mname) != (sizeof(xlColor) * buffer->GetPixelCount())) {
        logger_rcache.info("RenderCache::GetFrame on model " + mname + " failed due to frame size difference.");
        return false;
    }

    int frame = buffer->curPeriod - buffer->curEffStartPer;
    if (frame < modelFrames.size() && modelFrames[frame]) {
        // its in memory ... read it from there
        unsigned char* pc = modelFrames[frame];
        memcpy(buffer->GetPixels(), pc, _frameSize.at(mname));
        return true;
    }

    logger_rcache.info("RenderCache::GetFrame %d on model %s failed due to fall through.", frame, (const char*)mname.c_str());
    return false;
}

void RenderCacheItem::Touch() const
{
    wxFileName fn(_cacheFile);
    fn.Touch();
}

void RenderCacheItem::Save()
{
    if (_purged) return;
    if (!_dirty) return;
    
    if (_mmap) {
        return;
    }

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //logger_base.debug("Saving render cache file %s.", (const char *)_cacheFile.c_str());

    char zero = 0x00;

    // check all the data is there
    for (const auto& itm : _frames) {
        for (const auto& it : itm.second) {
            // we are missing data
            //wxASSERT(false);
            if (it == nullptr) return;
        }
    }

    wxFile file;

    if (file.Create(_cacheFile, true)) {
        _properties["Models"] = wxString::Format("%d", (int)_frames.size());
        // write the header fields
        for (const auto& it : _properties) {
            file.Write(it.first);
            file.Write(&zero, 1);
            file.Write(it.second);
            file.Write(&zero, 1);
        }

        file.Write("RC_HEADEREND");
        file.Write(&zero, 1);

        for (const auto& it : _frames) {
            file.Write(it.first);
            file.Write(&zero, 1);
            file.Write(wxString::Format("%d", (int)it.second.size()));
            file.Write(&zero, 1);
            file.Write(wxString::Format("%ld", _frameSize.at(it.first)));
            file.Write(&zero, 1);
        }
        _firstFrameOffset = file.Tell();
        
        // write the frames
        for (const auto& itm : _frames) {
            for (const auto& it : itm.second) {
                wxASSERT(it != nullptr);
                if (it != nullptr) {
                    file.Write(it, _frameSize.at(itm.first));
                }
            }
        }

        file.Close();
        
        remmap();
    } else {
        logger_base.warn("    Failed to create file.");
    }
}

bool RenderCacheItem::IsDone(RenderBuffer* buffer) const
{
    int frame = buffer->curPeriod - buffer->curEffStartPer;
    std::string mname = GetModelName(buffer);
    auto modelFrames = _frames.at(mname);
    return modelFrames[frame];
}

RenderCacheItem::RenderCacheItem(RenderCache* renderCache, const std::string& filename) : _renderCache(renderCache)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _mmap = nullptr;
    _mmapSize = 0;
    _cacheFile = filename;
    wxFileName fn(_cacheFile);
    _effectName = fn.GetName();
    int idx = _effectName.find('_');
    _effectName = _effectName.substr(0, idx);
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

            if (key == "") {
                // file looks corrupt
                logger_base.debug("Cache file %s appears corrupt.", (const char*)filename.c_str());
                _purged = true;
                return;
            } else {
                _properties[key] = value;
            }
        }
        ps += strlen(ps) + 1;

        int models = wxAtoi(_properties["Models"]);

        for (int i = 0; i < models; i++) {
            std::string model(ps);
            ps += strlen(ps) + 1;
            std::string frames(ps);
            ps += strlen(ps) + 1;
            int fs = wxAtoi(frames);
            std::string frameSize(ps);
            ps += strlen(ps) + 1;
            long fsz = wxAtol(frameSize);

            std::vector<unsigned char *> n;
            _frames[model] = n;
            _frames.at(model).resize(fs);
            _frameSize[model] = fsz;
        }

        _firstFrameOffset = ps - headerBuffer;
#ifdef USE_MMAP_RENDERCACHE
        if (renderCache->UseMMap()) {
            file.Seek(0);
            _mmapSize = file.Length();
            _mmap = (uint8_t*)mmap(nullptr, _mmapSize, PROT_READ, MAP_PRIVATE, file.fd(), 0);
            
            size_t cur = _firstFrameOffset;
            for (auto& itm : _frames) {
                for (int i = 0; i < itm.second.size(); i++) {
                    itm.second[i] = &_mmap[cur];
                    cur += _frameSize.at(itm.first);
                }
            }
        } else
#endif
        {
            file.Seek(_firstFrameOffset);
            for (auto& itm : _frames) {
                for (int i = 0; i < itm.second.size(); i++) {
                    uint8_t* frameBuffer = (uint8_t *)malloc(_frameSize.at(itm.first));
                    
                    if (frameBuffer == nullptr) {
                        file.Close();
                        PurgeFrames();
                        logger_base.debug("Render Cache Item file %s fails due to memory allocation issue.", (const char*)filename.c_str());
                        return;
                    }
                    
                    file.Read(frameBuffer, _frameSize.at(itm.first));
                    itm.second[i] = frameBuffer;
                }
            }
        }
        file.Close();
    }
}

void RenderCacheItem::unmmap() {
#ifdef USE_MMAP_RENDERCACHE
    if (_mmap) {
        for (auto& it : _frames) {
            for (int x = it.second.size() - 1; x >= 0; --x) {
                if (it.second[x]) {
                    uint8_t* frameBuffer = (uint8_t *)malloc(_frameSize.at(it.first));
                    memcpy(frameBuffer, it.second[x], _frameSize.at(it.first));
                    it.second[x] = frameBuffer;
                }
            }
        }
        munmap(_mmap, _mmapSize);
        _mmap = nullptr;
        _mmapSize = 0;
    }
#endif
}
void RenderCacheItem::remmap() {
#ifdef USE_MMAP_RENDERCACHE
    if (_mmap) return;
    if (!_renderCache->UseMMap()) return;

    wxFile file;
    if (file.Open(_cacheFile)) {
        file.Seek(0);
        _mmapSize = file.Length();
        _mmap = (uint8_t*)mmap(nullptr, _mmapSize, PROT_READ, MAP_PRIVATE, file.fd(), 0);
        
        size_t cur = _firstFrameOffset;
        for (auto& itm : _frames) {
            for (int i = 0; i < itm.second.size(); i++) {
                if (itm.second[i]) {
                    free(itm.second[i]);
                    itm.second[i] = nullptr;
                }
                itm.second[i] = &_mmap[cur];
                cur += _frameSize.at(itm.first);
            }
        }
    }
#endif
}


#pragma endregion RenderCacheItem

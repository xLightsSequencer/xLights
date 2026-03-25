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
#include "SequenceElements.h"
#include "RenderBuffer.h"
#include "models/Model.h"

#include <log.h>

#include <cassert>
#include <cstdio>
#include <filesystem>
#include <format>
#include <functional>
#include <thread>
#include "xLightsMain.h"
#include "xLightsApp.h"
#include "xLightsVersion.h"
#include "UtilFunctions.h"
#include "TraceLog.h"
#include "ExternalHooks.h"

#ifdef __WXOSX__
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#define USE_MMAP_RENDERCACHE
#endif

#pragma region RenderCache

namespace fs = std::filesystem;

static void RenderCacheLoadThreadEntry(RenderCache* cache)
{
    std::unique_lock<std::mutex> lock(cache->GetLoadMutex());

    

    spdlog::debug("Loading cache.");

    std::string cacheFolder = cache->GetCacheFolder();
    std::vector<std::string> files;
    std::error_code ec;
    if (fs::exists(cacheFolder, ec)) {
        for (const auto& entry : fs::recursive_directory_iterator(cacheFolder, ec)) {
            if (entry.is_regular_file() && entry.path().extension() == ".cache") {
                files.push_back(entry.path().string());
            }
        }
    }

    for (const auto& it : files) {
        // allow up to 3 times physical memory
        // This means the render cache will be swapped out ... but I think that is still better than re-rendering
        // Abandon loading render cache if we use too much memory
        if (IsExcessiveMemoryUsage(3.0)) {
            spdlog::warn("Render cache loading abandoned due to too much memory use.");
            break;
        }

        auto rci = new RenderCacheItem(cache, it);
        if (!rci->IsPurged()) {
            cache->AddCacheItem(rci);
        } else {
            delete rci;
            spdlog::warn("Failed to load cache item {}.", (const char*)it.c_str());
        }
    }

    spdlog::debug("Cache contained {} files.", (int)files.size());
    TraceLog::ClearTraceMessages();
}

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

    if (_baseCache.empty())
        return;

    std::error_code ec;
    if (!fs::exists(_baseCache, ec))
        return;

    // Calculate total size of cache directory
    uintmax_t total = 0;
    for (const auto& entry : fs::recursive_directory_iterator(_baseCache, ec)) {
        if (entry.is_regular_file()) {
            total += entry.file_size();
        }
    }
    if (total / 1024 / 1024 < _maximumSizeMB)
        return;

    // get the size and last written date of all render cache entries
    struct CacheEntry {
        uintmax_t size = 0;
        std::string name;
        fs::file_time_type modified;

        bool operator<(const CacheEntry& ce) const
        {
            return modified < ce.modified;
        }
    };

    std::list<CacheEntry> entries;

    for (const auto& entry : fs::recursive_directory_iterator(_baseCache, ec)) {
        if (entry.is_regular_file() && entry.path().extension() == ".cache") {
            CacheEntry ce;
            ce.name = entry.path().string();
            ce.size = entry.file_size();
            ce.modified = entry.last_write_time();

            if (ce.size / 1024 / 1024 > _maximumSizeMB / 2) {
                // we always delete anything larger than half the maximum as these are essentially making the cache useless
                fs::remove(ce.name, ec);
                total -= ce.size;
            } else {
                entries.push_back(ce);
            }
        }
    }

    entries.sort();

    while (total / 1024 / 1024 > _maximumSizeMB && entries.size() > 0) {
        if (fs::exists(entries.front().name, ec)) {
            fs::remove(entries.front().name, ec);
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
        assert(GetBitness() != "32bit");
        if (_loadThread.joinable()) {
            _loadThread.join();
        }
        _loadThread = std::thread(RenderCacheLoadThreadEntry, this);
    }
}

void RenderCache::AddCacheItem(RenderCacheItem* rci)
{
    if (rci != nullptr) {
        spdlog::info("RenderCache item added " + rci->Description());
        PerEffectCache *cache = GetPerEffectCache(rci->EffectName());
        std::unique_lock<std::shared_mutex> lock(cache->lock);
        cache->cache.push_back(rci);
    }
}

void RenderCache::SetSequence(const std::string& path, const std::string& sequenceFile)
{
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
            std::error_code ec;
            if (fs::exists(_cacheFolder, ec))
            {
                if (GetBitness() == "32bit")
                {
                    spdlog::debug("Render cache disabled but NOT removing folder {} as this is the 32 bt version.", _cacheFolder);
                }
                else
                {
                    spdlog::debug("Render cache disabled so removing folder {}.", _cacheFolder);
                    fs::remove_all(_cacheFolder, ec);
                }
            }
        }
        return;
    }

    if (sequenceFile != "")
    {
        _cacheFolder = path + GetPathSeparator() + "RenderCache" + GetPathSeparator() + sequenceFile + "_RENDER_CACHE";

        std::error_code ec;
        if (!fs::exists(_cacheFolder, ec))
        {
            std::string common = path + GetPathSeparator() + "RenderCache";
            if (!fs::exists(common, ec))
            {
                spdlog::debug("Creating render cache folder {}.", common);
                fs::create_directory(common, ec);
            }

            spdlog::debug("Creating render cache folder {}.", _cacheFolder);
            fs::create_directory(_cacheFolder, ec);
        }
        else
        {
            spdlog::debug("Opening render cache folder {}.", _cacheFolder);
        }

        LoadCache();
    }
}

void RenderCache::RemoveItem(RenderCacheItem *item) {
    PerEffectCache *c = GetPerEffectCache(item->EffectName());
    std::unique_lock<std::shared_mutex> lock(c->lock);
    auto &l = c->cache;
    for (auto it = l.begin(); it != l.end(); ++it) {
        if (item == *it) {
            spdlog::info("RenderCache item removed " + (*it)->Description());
            l.erase(it);
            break;
        }
    }
    delete item;
}

bool RenderCache::IsEffectOkForCaching(Effect* effect) const
{
    
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
        spdlog::error("RenderCache::IsEffectOkForCaching failed memory available test. This is a bad sign. Rendering will be really slow.");
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
            spdlog::info("RenderCache GetItem found an existing render cache item for effect {} on model {} on layer {} at start time {}ms.",
                effect->GetEffectName(),
                buffer->GetModelName(),
                effect->GetParentEffectLayer()->GetLayerNumber(),
                effect->GetStartTimeMS());

            if (item != nullptr)
                item->Touch();

            return item;
        }
    }
    lock.unlock();

    spdlog::info("RenderCache GetItem created a new render cache item for effect {} on model {} on layer {} at start time {}ms.",
        effect->GetEffectName(),
        buffer->GetModelName(),
        effect->GetParentEffectLayer()->GetLayerNumber(),
        effect->GetStartTimeMS());

    return new RenderCacheItem(this, effect, buffer);
}

void RenderCache::Close()
{
    if (_cacheFolder == "") return;

    spdlog::debug("Closing render cache folder {}.", _cacheFolder);

    // wait for the cache load thread to finish
    if (_loadThread.joinable()) {
        _loadThread.join();
    }

    spdlog::debug("    Got lock.");

    Purge(nullptr, false);
    _cacheFolder = "";

    std::unique_lock<std::recursive_mutex> lock(_cacheLock);
    for (auto &a : _cache) {
        delete a.second;
        a.second = nullptr;
    }
    _cache.clear();
    spdlog::debug("    Closed.");
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
    

    spdlog::debug("Cleaning up the cache.");

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
    spdlog::debug("    Cleaned up {} items in the cache.", deleted);

    for (int i = 0; i < sequenceElements->GetElementCount(); ++i) {
        Element* em = sequenceElements->GetElement(i);
        purgeCache(em, false);
    }

    spdlog::debug("    Cache purge done.");
}

void RenderCache::SetRenderCacheFolder(const std::string& path)
{
    _baseCache = path + GetPathSeparator() + "RenderCache";
    EnforceMaximumSize();
}

void RenderCache::Purge(SequenceElements* sequenceElements, bool dodelete)
{
    

    if (dodelete && _cacheFolder != "")
    {
        spdlog::debug("Purging render cache folder {}.", (const char *)_cacheFolder.c_str());
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
        std::string mname = buffer->GetModelName();
        Replace(mname, "/", "_");
        Replace(mname, "\\", "_");
        Replace(mname, ":", "_");
        Replace(mname, "?", "_");
        Replace(mname, "*", "_");
        Replace(mname, "$", "_");
        return mname;
    }
}

RenderCacheItem::RenderCacheItem(RenderCache* renderCache, Effect* effect, RenderBuffer* buffer) : _renderCache(renderCache)
{
    _mmap = nullptr;
    _mmapSize = 0;
    _purged = false;
    _dirty = true;
    std::string mname = GetModelName(buffer);
    assert(mname != "");
    _frameSize[mname] = sizeof(xlColor) * buffer->GetPixelCount();
    std::string elname = effect->GetParentEffectLayer()->GetParentElement()->GetFullName();
    Replace(elname, "/", "_");
    Replace(elname, "\\", "_");
    Replace(elname, ":", "_");
    Replace(elname, "?", "_");
    Replace(elname, "*", "_");
    Replace(elname, "$", "_");
    std::string file = std::format("{}_{}_{}_{}.cache",
            effect->GetEffectName(), elname,
            effect->GetParentEffectLayer()->GetLayerNumber(),
            effect->GetStartTimeMS());
    _effectName = effect->GetEffectName();
    _cacheFile = renderCache->GetCacheFolder() + GetPathSeparator() + file;
    _properties["Effect"] = effect->GetEffectName();
    _properties["Element"] = effect->GetParentEffectLayer()->GetParentElement()->GetFullName();
    _properties["EffectLayer"] = std::to_string(effect->GetParentEffectLayer()->GetLayerNumber());
    _properties["StartMS"] = std::to_string(effect->GetStartTimeMS());
    _properties["EndMS"] = std::to_string(effect->GetEndTimeMS());
    _properties["Frames"] = std::to_string(buffer->curEffEndPer - buffer->curEffStartPer + 1);
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
    if (_purged) return false;

    if (std::atoi(_properties.at("StartMS").c_str()) != effect->GetStartTimeMS()) return false;

    EffectLayer* el = effect->GetParentEffectLayer();
    if (std::atoi(_properties.at("EffectLayer").c_str()) != el->GetLayerNumber()) return false;

    Element* e = el->GetParentElement();
    if (_properties.at("Element") != e->GetFullName()) return false;

    int start_ms = std::atoi(_properties.at("StartMS").c_str());
    int end_ms = std::atoi(_properties.at("EndMS").c_str());
    int frame_count = std::atoi(_properties.at("Frames").c_str());
    if (frame_count < 1) frame_count = 1;
    long duration_ms = end_ms - start_ms;
    long fps = (duration_ms * 1000) / (frame_count * 1000);

    xLightsFrame* frame = xLightsApp::GetFrame();
    int seqFPS = frame->_seqData.FrameTime();

    if (seqFPS != fps) {
        spdlog::info("RenderCache no match because FPS {} doesn't match expected {}", fps, seqFPS);
        return false;
    }

    if (buffer != nullptr)
    {
        std::string mname = GetModelName(buffer);
        if (_frameSize.at(mname) != sizeof(xlColor) * buffer->GetPixelCount()) return false;
    }

    if (std::atoi(_properties.at("EndMS").c_str()) != effect->GetEndTimeMS()) return false;
    if (_properties.at("Effect") != effect->GetEffectName()) return false;

    // We only log failures from here on because they should be relatively rare

    // 8 is the number of predefined tags
    if (_properties.size() - 7 != effect->GetSettings().size() + effect->GetPaletteMap().size())
    {
        spdlog::debug("RenderCache no match because number of properties is different.");
        return false;
    }

    for (const auto& it : effect->GetSettings())
    {
        if (_properties.find(it.first) == _properties.end()) {
            spdlog::debug("RenderCache no match because proprerty not present: " + it.first);
            return false;
        }
        else
        {
            if (_properties.at(it.first) != it.second)
            {
                spdlog::debug("RenderCache no match because proprerty different: " + it.first);
                return false;
            }
        }
    }

    for (const auto& it : effect->GetPaletteMap())
    {
        if (_properties.find(it.first) == _properties.end()) {
            spdlog::debug("RenderCache no match because pallette map not present: " + it.first);
            return false;
        }
        else
        {
            if (_properties.at(it.first) != it.second)
            {
                spdlog::debug("RenderCache no match because pallette map different: " + it.first);
                return false;
            }
        }
    }

    return true;
}

void RenderCacheItem::Delete()
{
    
    if (!_purged && FileExists(_cacheFile)) {
        std::error_code ec;
        if (!fs::remove(_cacheFile, ec)) {
            spdlog::warn("Unable to remove cache file " + _cacheFile);
        } else {
            spdlog::info("RenderCache removed file " + _cacheFile);
        }
    }
    PurgeFrames();
    _renderCache->RemoveItem(this);
}

void RenderCacheItem::AddFrame(RenderBuffer* buffer)
{
    
    if (buffer == nullptr) {
        spdlog::error("RenderCacheItem::AddFrame was passed a null buffer");
        return;
    }
    
    if (buffer->GetPixelCount() == 0) {
        spdlog::error("RenderCacheItem::AddFrame was passed a buffer with no pixels in it");
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
        spdlog::error("RenderCacheItem::AddFrame failed memory available test. This is a bad sign. Rendering will be really slow.");
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
            spdlog::warn("RenderCacheItem::AddFrame buffer size changed ... we dont support this.");
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
        spdlog::warn("RenderCacheItem::AddFrame failed to allocate frameBuffer.");
        PurgeFrames();
        assert(false);
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
                //spdlog::warn("RenderCacheItem::AddFrame save abandoned due to null frame.");
                return;
            }
        }
        Save();
    }
}

bool RenderCacheItem::GetFrame(RenderBuffer* buffer)
{
    std::string mname = GetModelName(buffer);
    if (_frameSize.find(mname) == _frameSize.end()) {
        spdlog::info("RenderCache::GetFrame on model " + mname + " failed due to number of frames difference.");
        return false;
    }

    auto modelFrames = _frames[mname];
    if (_frameSize.at(mname) != (sizeof(xlColor) * buffer->GetPixelCount())) {
        spdlog::info("RenderCache::GetFrame on model " + mname + " failed due to frame size difference.");
        return false;
    }

    int frame = buffer->curPeriod - buffer->curEffStartPer;
    if (frame < modelFrames.size() && modelFrames[frame]) {
        // its in memory ... read it from there
        unsigned char* pc = modelFrames[frame];
        memcpy(static_cast<void*>(buffer->GetPixels()), pc, _frameSize.at(mname));
        return true;
    }

    spdlog::info("RenderCache::GetFrame {} on model {} failed due to fall through.", frame, mname);
    return false;
}

void RenderCacheItem::Touch() const
{
    std::error_code ec;
    fs::last_write_time(_cacheFile, fs::file_time_type::clock::now(), ec);
}

void RenderCacheItem::Save()
{
    if (_purged) return;
    if (!_dirty) return;
    
    if (_mmap) {
        return;
    }

    
    //spdlog::debug("Saving render cache file {}.", (const char *)_cacheFile.c_str());

    char zero = 0x00;

    // check all the data is there
    for (const auto& itm : _frames) {
        for (const auto& it : itm.second) {
            // we are missing data
            //assert(false);
            if (it == nullptr) return;
        }
    }

    FILE* fp = std::fopen(_cacheFile.c_str(), "wb");

    if (fp != nullptr) {
        _properties["Models"] = std::to_string((int)_frames.size());
        // write the header fields
        for (const auto& it : _properties) {
            std::fwrite(it.first.c_str(), 1, it.first.size(), fp);
            std::fwrite(&zero, 1, 1, fp);
            std::fwrite(it.second.c_str(), 1, it.second.size(), fp);
            std::fwrite(&zero, 1, 1, fp);
        }

        std::fwrite("RC_HEADEREND", 1, 12, fp);
        std::fwrite(&zero, 1, 1, fp);

        for (const auto& it : _frames) {
            std::fwrite(it.first.c_str(), 1, it.first.size(), fp);
            std::fwrite(&zero, 1, 1, fp);
            std::string numFrames = std::to_string((int)it.second.size());
            std::fwrite(numFrames.c_str(), 1, numFrames.size(), fp);
            std::fwrite(&zero, 1, 1, fp);
            std::string fsize = std::to_string(_frameSize.at(it.first));
            std::fwrite(fsize.c_str(), 1, fsize.size(), fp);
            std::fwrite(&zero, 1, 1, fp);
        }
        _firstFrameOffset = std::ftell(fp);

        // write the frames
        for (const auto& itm : _frames) {
            for (const auto& it : itm.second) {
                assert(it != nullptr);
                if (it != nullptr) {
                    std::fwrite(it, 1, _frameSize.at(itm.first), fp);
                }
            }
        }

        std::fclose(fp);

        remmap();
    } else {
        spdlog::warn("    Failed to create file.");
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
    

    _mmap = nullptr;
    _mmapSize = 0;
    _cacheFile = filename;
    fs::path fnPath(_cacheFile);
    _effectName = fnPath.stem().string();
    int idx = _effectName.find('_');
    _effectName = _effectName.substr(0, idx);
    _purged = false;
    _dirty = false;

    FILE* fp = std::fopen(_cacheFile.c_str(), "rb");

    if (fp != nullptr) {
        char headerBuffer[8192];
        memset(headerBuffer, 0x00, sizeof(headerBuffer));
        std::fread(headerBuffer, 1, sizeof(headerBuffer), fp);

        char* ps = headerBuffer;

        while (strcmp(ps, "RC_HEADEREND") != 0) {
            std::string key(ps);
            ps += strlen(ps) + 1;
            std::string value(ps);
            ps += strlen(ps) + 1;

            if (key == "") {
                // file looks corrupt
                spdlog::debug("Cache file {} appears corrupt.", (const char*)filename.c_str());
                _purged = true;
                std::fclose(fp);
                return;
            } else {
                _properties[key] = value;
            }
        }
        ps += strlen(ps) + 1;

        int models = std::atoi(_properties["Models"].c_str());

        for (int i = 0; i < models; i++) {
            std::string model(ps);
            ps += strlen(ps) + 1;
            std::string frames(ps);
            ps += strlen(ps) + 1;
            int frameCount = std::atoi(frames.c_str());
            std::string frameSize(ps);
            ps += strlen(ps) + 1;
            long fsz = std::strtol(frameSize.c_str(), nullptr, 10);

            std::vector<unsigned char *> n;
            _frames[model] = n;
            _frames.at(model).resize(frameCount);
            _frameSize[model] = fsz;
        }

        _firstFrameOffset = ps - headerBuffer;
#ifdef USE_MMAP_RENDERCACHE
        if (renderCache->UseMMap()) {
            struct stat st;
            fstat(fileno(fp), &st);
            _mmapSize = st.st_size;
            _mmap = (uint8_t*)mmap(nullptr, _mmapSize, PROT_READ, MAP_PRIVATE, fileno(fp), 0);

            if (_mmap == MAP_FAILED) {
                _mmap = nullptr;
                _mmapSize = 0;
            } else {
                size_t cur = _firstFrameOffset;
                for (auto& itm : _frames) {
                    for (int i = 0; i < itm.second.size(); i++) {
                        itm.second[i] = &_mmap[cur];
                        cur += _frameSize.at(itm.first);
                    }
                }
                std::fclose(fp);
                return;
            }
        }
#endif
        std::fseek(fp, _firstFrameOffset, SEEK_SET);
        for (auto& itm : _frames) {
            for (int i = 0; i < itm.second.size(); i++) {
                uint8_t* frameBuffer = (uint8_t *)malloc(_frameSize.at(itm.first));

                if (frameBuffer == nullptr) {
                    std::fclose(fp);
                    PurgeFrames();
                    spdlog::debug("Render Cache Item file {} fails due to memory allocation issue.", filename);
                    return;
                }

                std::fread(frameBuffer, 1, _frameSize.at(itm.first), fp);
                itm.second[i] = frameBuffer;
            }
        }
        std::fclose(fp);
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

    FILE* fp = std::fopen(_cacheFile.c_str(), "rb");
    if (fp != nullptr) {
        struct stat st;
        fstat(fileno(fp), &st);
        _mmapSize = st.st_size;
        _mmap = (uint8_t*)mmap(nullptr, _mmapSize, PROT_READ, MAP_PRIVATE, fileno(fp), 0);
        std::fclose(fp);
        if (_mmap == MAP_FAILED) {
            _mmap = nullptr;
            _mmapSize = 0;
            return;
        }
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

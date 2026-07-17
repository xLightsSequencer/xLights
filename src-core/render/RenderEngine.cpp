/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <spdlog/fmt/fmt.h>
#include <array>
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "utils/AutoReleasePool.h"
#include "RenderEngine.h"
#include "RenderContext.h"
#include "Effect.h"
#include "EffectLayer.h"
#include "Element.h"
#include "SequenceElements.h"
#include "SequenceFile.h"
#include "effects/RenderableEffect.h"
#include "effects/EffectManager.h"
#include "IRenderJobStatus.h"
#include "IRenderProgressSink.h"
#include "RenderProgressInfo.h"
#include "RenderUtils.h"
#include "models/ModelGroup.h"
#include "UtilFunctions.h"
#include "PixelBuffer.h"
#include "Parallel.h"
#include "utils/ExternalHooks.h"
#include "GPURenderUtils.h"
#include "RenderProfile.h"
#include "RenderCache.h"
#include "UtilClasses.h"
#include "JobPool.h"
#include "models/DMX/DmxMovingHeadAdv.h"

#include <log.h>
// END_OF_RENDER_FRAME is defined in RenderProgressInfo.h

// XL_EFFSUM=1 determinism diagnostic: emit content checksums at render stages
// (C = canvas preload result, O = post-blend seqData slice) to stderr.  Two
// runs' sorted outputs diff at the first non-deterministic producer.
static const bool xldbgEffSum = (getenv("XL_EFFSUM") != nullptr);

// XL_RENDER_PROFILE=1 diagnostic: accumulate per-row / per-effect render timing
// and dump aggregate tables to stderr when the batch completes.  Checked before
// any clock call so it costs nothing when unset (see RenderProfile.h).
static const bool profRender = (getenv("XL_RENDER_PROFILE") != nullptr);

// XL_VERIFY_STATELESS=1 diagnostic: for every effect that declares itself Pure
// (GetEffectiveFrameParallelism == Pure), re-render each frame a second time
// from a cleared infoCache / needToInit=true and warn if the pixels differ.  A
// truly frame-independent effect produces identical output; a mismatch means it
// secretly carried cross-frame state and the Pure declaration is wrong.  Same
// buffer + same GPU/CPU path both times, so there is no GPU-vs-CPU confound.
static const bool xldbgVerifyStateless = (getenv("XL_VERIFY_STATELESS") != nullptr);
static std::atomic<uint64_t> xldbgVerifyChecks{0};
static std::atomic<uint64_t> xldbgVerifyMismatches{0};

// XL_PARALLEL_WINDOWS=1 diagnostic: per model, classify each frame as
// parallel-safe / serial / empty from the effects' GetEffectiveFrameParallelism
// tier and log the contiguous parallel-safe window structure.  Read-only; it
// quantifies how much of a real show frame-parallel group rendering could apply
// to, the go/no-go input for the FrameState refactor.
static const bool xldbgParallelWindows = (getenv("XL_PARALLEL_WINDOWS") != nullptr);

// Frame-parallel group render (default ON): for an eligible group, render a
// contiguous run of Pure / Snapshottable frames concurrently - each frame
// produced into its own clone buffer - then output them serially, in frame
// order, still gated on upstream.  produce() is row-local so it needs nothing
// from upstream; output() touches only seqData[frame] (a distinct row per
// frame) so ordered output has no cross-frame conflict.  Byte-identical to the
// serial path.  Set XL_NO_PARALLEL_FRAMES=1 to force serial (A/B / bisecting).
static const bool xldbgParallelFrames = (getenv("XL_NO_PARALLEL_FRAMES") == nullptr);
static const int PAR_FRAME_MAX_CHUNK = 24;

// XL_PARALLEL_BLOCKERS=1: profile-only.  On every structurally-eligible group
// row, walk each frame and record which effect(s) prevent it from rendering in
// a parallel window - so a run over a whole show library ranks the effects
// worth converting next.  A frame is BLOCKED when a covering layer is an
// inherently-Stateful effect (the conversion target), or a Pure/Snapshottable
// effect vetoed by a buffer-continuity SETTING (OverlayBkg/Freeze/Suppress -
// not an effect-conversion target), or a frame+1 continuity boundary.
// "sole" = the frame's ONLY blocker is that one Stateful effect, so converting
// it unlocks the frame immediately; "any" = it is a blocker among others.
// Accumulated process-wide, dumped to stderr at exit.  Zero cost when unset.
static const bool xldbgParBlockers = (getenv("XL_PARALLEL_BLOCKERS") != nullptr);
// Plain accumulator (no mutex/destructor) - used both as the per-row local and
// as the payload of the process-wide tally.
struct ParBlockerData {
    std::map<std::string, uint64_t> statefulSole; // effect -> frames it alone blocks
    std::map<std::string, uint64_t> statefulAny;  // effect -> frames it blocks (with or without co-blockers)
    std::map<std::string, uint64_t> continuityAny; // Pure/Snap effect vetoed by a continuity setting
    uint64_t nextBoundaryFrames = 0;              // frames blocked (partly) by a frame+1 continuity boundary
    uint64_t eligSafe = 0;                        // parallel-safe frames on eligible groups
    uint64_t eligBlocked = 0;                     // blocked frames on eligible groups
    uint64_t eligEmpty = 0;                       // frames with no coverage
    uint64_t rowsEligible = 0;                    // structurally-eligible group rows analyzed
    uint64_t rowsSingleModel = 0;                 // non-group rows (not parallelizable by design)
    std::map<std::string, uint64_t> rowsIneligible; // group rows excluded by structure -> reason
    // Per-row detail: name, buffer pixels, safe frames, covered frames.  Sized
    // for the item-03 economics question (which excluded model rows would pay).
    std::vector<std::tuple<std::string, uint64_t, uint64_t, uint64_t>> rows;
    void merge(const ParBlockerData& o) {
        for (const auto& [k, v] : o.statefulSole) statefulSole[k] += v;
        for (const auto& [k, v] : o.statefulAny) statefulAny[k] += v;
        for (const auto& [k, v] : o.continuityAny) continuityAny[k] += v;
        for (const auto& [k, v] : o.rowsIneligible) rowsIneligible[k] += v;
        nextBoundaryFrames += o.nextBoundaryFrames;
        eligSafe += o.eligSafe;
        eligBlocked += o.eligBlocked;
        eligEmpty += o.eligEmpty;
        rowsEligible += o.rowsEligible;
        rowsSingleModel += o.rowsSingleModel;
        rows.insert(rows.end(), o.rows.begin(), o.rows.end());
    }
};
// Process-wide tally; the single static instance dumps a ranked table at exit.
// `d` = structurally-eligible group rows (the conversion-priority table).
// `m` = single-MODEL rows that would qualify except for the groups-only rule -
// the item-03 population; tallied separately so the two questions ("which
// effect to convert" vs "is extending to model rows worth it") stay distinct.
struct ParBlockerStats {
    std::mutex mtx;
    ParBlockerData d;
    ParBlockerData m;
    // Rows (group or model) excluded ONLY by submodel/strand/node effects -
    // the item-03 step-3 population; classified across main + submodel layers.
    ParBlockerData s;
    ~ParBlockerStats() { Dump(); }
    void Dump() {
        std::lock_guard<std::mutex> lg(mtx);
        uint64_t total = d.eligSafe + d.eligBlocked + d.eligEmpty;
        if (total == 0 && d.statefulAny.empty()) return;
        uint64_t nonEmpty = d.eligSafe + d.eligBlocked;
        fprintf(stderr, "\n========== XL_PARALLEL_BLOCKERS ==========\n");
        fprintf(stderr, "Eligible group rows analyzed: %llu   (single-model rows skipped: %llu)\n",
                (unsigned long long)d.rowsEligible, (unsigned long long)d.rowsSingleModel);
        fprintf(stderr, "Frame-rows on eligible groups: %llu total  |  %llu covered  |  %llu empty\n",
                (unsigned long long)total, (unsigned long long)nonEmpty, (unsigned long long)d.eligEmpty);
        if (nonEmpty > 0) {
            fprintf(stderr, "  parallel-safe: %llu (%.1f%% of covered)   blocked: %llu (%.1f%%)\n",
                    (unsigned long long)d.eligSafe, 100.0 * d.eligSafe / nonEmpty,
                    (unsigned long long)d.eligBlocked, 100.0 * d.eligBlocked / nonEmpty);
        }
        auto ranked = [](const std::map<std::string, uint64_t>& sole, const std::map<std::string, uint64_t>& any) {
            std::vector<std::pair<std::string, std::pair<uint64_t, uint64_t>>> v;
            for (const auto& [k, a] : any) {
                uint64_t s = 0;
                auto it = sole.find(k);
                if (it != sole.end()) s = it->second;
                v.push_back({k, {s, a}});
            }
            std::sort(v.begin(), v.end(), [](const auto& x, const auto& y) {
                if (x.second.first != y.second.first) return x.second.first > y.second.first;
                return x.second.second > y.second.second;
            });
            return v;
        };
        fprintf(stderr, "\n-- Stateful effects blocking eligible-group frames (convert these) --\n");
        fprintf(stderr, "   %-18s %14s %14s\n", "effect", "sole-blocker", "any-blocker");
        for (const auto& [name, counts] : ranked(d.statefulSole, d.statefulAny)) {
            fprintf(stderr, "   %-18s %14llu %14llu\n", name.c_str(),
                    (unsigned long long)counts.first, (unsigned long long)counts.second);
        }
        if (!d.continuityAny.empty()) {
            fprintf(stderr, "\n-- Pure/Snapshottable effects vetoed by a buffer-continuity SETTING (not conversion targets) --\n");
            std::vector<std::pair<std::string, uint64_t>> cv(d.continuityAny.begin(), d.continuityAny.end());
            std::sort(cv.begin(), cv.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
            for (const auto& [name, c] : cv) {
                fprintf(stderr, "   %-18s %14llu\n", name.c_str(), (unsigned long long)c);
            }
        }
        fprintf(stderr, "\nFrames blocked (partly) by a frame+1 continuity boundary: %llu\n",
                (unsigned long long)d.nextBoundaryFrames);
        if (!d.rowsIneligible.empty()) {
            fprintf(stderr, "\n-- Group rows excluded by structure (effect conversion cannot help these) --\n");
            std::vector<std::pair<std::string, uint64_t>> rv(d.rowsIneligible.begin(), d.rowsIneligible.end());
            std::sort(rv.begin(), rv.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
            for (const auto& [reason, c] : rv) {
                fprintf(stderr, "   %-28s %llu rows\n", reason.c_str(), (unsigned long long)c);
            }
        }
        auto rowSection = [&](const ParBlockerData& t, const char* title) {
            uint64_t covered = t.eligSafe + t.eligBlocked;
            if (covered + t.eligEmpty == 0) {
                return;
            }
            fprintf(stderr, "\n-- %s --\n", title);
            fprintf(stderr, "rows: %llu   covered frames: %llu   parallel-safe: %llu (%.1f%%)\n",
                    (unsigned long long)t.rowsEligible, (unsigned long long)covered,
                    (unsigned long long)t.eligSafe, covered ? 100.0 * t.eligSafe / covered : 0.0);
            fprintf(stderr, "   %-18s %14s %14s\n", "blocking effect", "sole-blocker", "any-blocker");
            for (const auto& [name, counts] : ranked(t.statefulSole, t.statefulAny)) {
                fprintf(stderr, "   %-18s %14llu %14llu\n", name.c_str(),
                        (unsigned long long)counts.first, (unsigned long long)counts.second);
            }
            // The economics view: aggregate per row across sequences and rank
            // by safe-frames x buffer-pixels (a produce-work proxy) so size
            // floors / step-3 priorities can be chosen from data.
            std::map<std::string, std::array<uint64_t, 3>> byName; // pixels, safe, covered
            for (const auto& [name, px, safe, cov] : t.rows) {
                auto& a = byName[name];
                a[0] = px;
                a[1] += safe;
                a[2] += cov;
            }
            std::vector<std::pair<std::string, std::array<uint64_t, 3>>> rowsByWeight(byName.begin(), byName.end());
            std::sort(rowsByWeight.begin(), rowsByWeight.end(), [](const auto& a, const auto& b) {
                return a.second[0] * a.second[1] > b.second[0] * b.second[1];
            });
            fprintf(stderr, "   top rows by safe-frames x buffer-pixels (all sequences):\n");
            fprintf(stderr, "   %-28s %10s %10s %10s\n", "row", "pixels", "safe", "covered");
            int shown = 0;
            for (const auto& [name, a] : rowsByWeight) {
                if (++shown > 25) break;
                fprintf(stderr, "   %-28s %10llu %10llu %10llu\n", name.c_str(),
                        (unsigned long long)a[0], (unsigned long long)a[1], (unsigned long long)a[2]);
            }
        };
        rowSection(m, "SINGLE-MODEL rows that would qualify except for the groups-only rule (item 03)");
        rowSection(s, "rows excluded ONLY by submodel/strand effects - main+submodel layers classified (item 03 step 3)");
        fprintf(stderr, "==========================================\n\n");
    }
};
static ParBlockerStats& parBlockers() {
    static ParBlockerStats s;
    return s;
}

static uint64_t xldbgFNV(const uint8_t* d, size_t n) {
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < n; i++) {
        h ^= d[i];
        h *= 1099511628211ULL;
    }
    return h;
}


class EffectLayerInfo {
public:
    EffectLayerInfo(): element(nullptr)
    {
        numLayers = 0;
        buffer.reset(nullptr);
        strand = -1;
    }

    EffectLayerInfo(int l) : element(nullptr) {
        resize(l);
        buffer.reset(nullptr);
        strand = -1;
    }

    void resize(int l) {
        numLayers = l;
        currentEffects.resize(l);
        currentEffectIdxs.resize(l);
        settingsMaps.resize(l);
        effectStates.resize(l);
        validLayers.resize(l + 1); //extra one for the blending layer
    }

    int numLayers;
    int strand;
    int submodel = -1;
    Element *element;
    PixelBufferClassPtr buffer;
    std::vector<Effect*> currentEffects;
    std::vector<int> currentEffectIdxs;
    std::vector<SettingsMap> settingsMaps;
    std::vector<bool> effectStates;
    std::vector<bool> validLayers;
    // Restricts which layers ProduceFrame renders: layer L is rendered only if
    // this is empty (the default - render every layer) OR processLayer[L] is true.
    // The tier-2 capture pre-pass sets it so only Snapshottable layers run (their
    // sim must advance) while Pure layers - which the parallel pass renders fresh -
    // are skipped instead of drawn twice.  NOT resized by resize(): stays empty
    // unless a caller opts in, so every other path is unaffected.
    std::vector<bool> processLayer;

    // Produce/output split (ARC phase A).  produce() renders every layer into
    // `buffer` (row-local) and records here what output() needs to blend into
    // seqData once the upstream gate clears; they live on the info so they
    // survive the one-frame suspend that can fall between the two halves.
    std::vector<bool> partOfCanvas;
    bool produceEffectsToUpdate = false;
    bool produceBlend = false;
    int produceEffectiveNumLayers = 0;
};

class NextRenderer {
public:

    NextRenderer() : nextLock(), previousFrameDone(-1) {
    }

    virtual ~NextRenderer() {}

    bool addNext(NextRenderer *n) {
        for (const auto& i : next) {
            if (i == n) return false;
        }
        next.push_back(n);
        return true;
    }

    bool HasNext() const {
        return !next.empty();
    }

    void FrameDone(int frame) {
        for (const auto& i : next) {
            i->setPreviousFrameDone(frame);
        }
    }

    // Nothing blocks on this anymore; nextLock makes the update atomic with a
    // suspended RenderJob's registered wake-up frame (see the RenderJob
    // override, which requeues the job instead of waking a sleeping thread).
    // Monotonic (max, not assign): with aggregator fan-in the relays for two
    // frames can run on different threads, and a stale lower frame landing
    // after END_OF_RENDER_FRAME would re-strand a job waiting on END forever.
    virtual void setPreviousFrameDone(int i) {
        std::unique_lock<std::mutex> lock(nextLock);
        if (i > previousFrameDone) {
            previousFrameDone = i;
        }
    }

    int GetPreviousFrameDone() const {
        return previousFrameDone;
    }

protected:
    std::mutex nextLock;
    std::atomic_int previousFrameDone;
private:
    std::vector<NextRenderer *> next;
};

class AggregatorRenderer: public NextRenderer {
public:

    AggregatorRenderer(int numFrames) : NextRenderer(), data(numFrames + 20), finalFrame(numFrames + 19) {
        for (int x = 0; x < (numFrames + 20); ++x) {
            data[x] = 0;
        }
        max = 0;
    }

    virtual ~AggregatorRenderer() {
    }

    void incNumAggregated() {
        ++max;
    }

    int getNumAggregated() const
    {
        return max;
    }

    virtual void setPreviousFrameDone(int frame) {
        if (max <= 1) {
            bumpPreviousFrameDone(frame);
            FrameDone(frame);
            return;
        }
        int idx = frame;
        if (idx == END_OF_RENDER_FRAME) {
            idx = finalFrame;
        }
        if (idx < 0 || idx >= (int)data.size()) {
            // Out-of-range frame index — clamp to the sentinel slot to avoid OOB.
            idx = finalFrame;
        }
        int i = data[idx].fetch_add(1);
        if (i == (max - 1)) {
            bumpPreviousFrameDone(frame);
            FrameDone(frame);
        }
    }

private:
    // Monotonic update without nextLock: relays for different frames can race
    // on different upstream threads, so a plain store could regress the value.
    void bumpPreviousFrameDone(int frame) {
        int prev = previousFrameDone.load();
        while (frame > prev && !previousFrameDone.compare_exchange_weak(prev, frame)) {
        }
    }

    std::vector<std::atomic_int> data;
    int max;
    const int finalFrame;
};

void RenderProgressInfo::CleanupJobs() {
    for (int i = 0; i < numRows; ++i) {
        delete jobs[i];
        delete aggregators[i];
    }
    delete[] jobs;
    jobs = nullptr;
    delete[] aggregators;
    aggregators = nullptr;
    delete progressSink;
    progressSink = nullptr;
}

class SNPair {
public:
    SNPair(int s, int n) : strand(s), node(n) {}

    SNPair(const SNPair &p) : strand(p.strand), node(p.node) {}

    bool operator>(const SNPair &p) const {
        return strand > p.strand
               || (strand == p.strand && node > p.node);
    }

    bool operator<(const SNPair &p) const {
        return strand < p.strand
               || (strand == p.strand && node < p.node);
    }

    bool operator==(const SNPair &p) const {
        return strand == p.strand && node == p.node;
    }

    const int strand;
    const int node;
};


class RenderJob: public Job, public NextRenderer, public IRenderJobStatus {
public:
    RenderJob(ModelElement *row, SequenceData &data, RenderContext *ctx, RenderEngine *engine, SequenceElements *seqElements = nullptr)
        : Job(), NextRenderer(), rowToRender(row), _ctx(ctx), _engine(engine), seqData(&data),
            _seqElements(seqElements),
            supportsModelBlending(false), statusMap(nullptr), m_logger(spdlog::get("render")),
            currentFrame(0), abort(false)
    {
        name = "";
        if (row != nullptr) {
            // Hold ~ModelElement's guard open for this job's whole lifetime -
            // queued, parked, suspended, or running (paired in CompleteJob /
            // the destructor).
            row->AttachRenderJob();
            attachedToRow = true;
            name = row->GetModelName();
            mainBuffer = new PixelBufferClass(_ctx);
            numLayers = rowToRender->GetEffectLayerCount();

            Model* mdl = _ctx->GetModel(name);
            if (mdl != nullptr) {
                mainBuffer->InitBuffer(*mdl, numLayers, seqData->FrameTime());
                const Model *model = mainBuffer->GetModel();
                if (DisplayAsType::ModelGroup == model->GetDisplayAs()) {
                    const ModelGroup* grp = dynamic_cast<const ModelGroup*>(model);
                    //for (int l = 0; l < numLayers; ++l) {
                    for (int l = numLayers - 1; l >= 0; --l) {
                        EffectLayer *layer = row->GetEffectLayer(l);
                        bool perModelEffects = false;
                        bool perModelEffectsDeep = false;
                        for (int e = 0; e < layer->GetEffectCount() && !perModelEffects; ++e) {
                            static const std::string CHOICE_BufferStyle("B_CHOICE_BufferStyle");
                            static const std::string DEFAULT("Default");
                            static const std::string PER_MODEL("Per Model");
                            static const std::string DEEP("Deep");
                            const std::string& bt = layer->GetEffect(e)->GetSettings().Get(CHOICE_BufferStyle, DEFAULT);
                            if (bt.compare(0, 9, PER_MODEL) == 0) {
                                if (bt.compare(bt.length() - 4, 4, DEEP) == 0) {
                                    perModelEffectsDeep = true;
                                } else {
                                    perModelEffects = true;
                                }
                            } else if (bt == DEFAULT) {
                                if (grp != nullptr && grp->GetDefaultBufferStyle().compare(0, 9, PER_MODEL) == 0) {
                                    perModelEffects = true;
                                }
                            }
                        }
                        // A Per-Model buffer style merges dependent per-model
                        // pixels during produce, so such a row is excluded from
                        // the ARC phase A produce/output split (see RenderFrame).
                        if (perModelEffects || perModelEffectsDeep) {
                            ctorHasPerModelBuffers = true;
                        }
                        if (perModelEffectsDeep) {
                            mainBuffer->InitPerModelBuffersDeep(*grp, l, data.FrameTime());
                        }
                        if (perModelEffects) {
                            mainBuffer->InitPerModelBuffers(*grp, l, data.FrameTime());
                        }
                    }
                }
                std::string duplicateIncludeSourceModel;
                for (int lyr = 0; lyr < (int)rowToRender->GetEffectLayerCount() && duplicateIncludeSourceModel.empty(); ++lyr) {
                    EffectLayer* elyr = rowToRender->GetEffectLayer(lyr);
                    std::unique_lock<std::recursive_mutex> elyrLock(elyr->GetLock());
                    for (int e = 0; e < elyr->GetEffectCount(); ++e) {
                        Effect* eff = elyr->GetEffect(e);
                        if (eff->GetEffectIndex() == EffectManager::eff_DUPLICATE &&
                            eff->GetSetting("E_CHECKBOX_Duplicate_Include_Submodels") == "1") {
                            duplicateIncludeSourceModel = eff->GetSetting("E_CHOICE_Duplicate_Model");
                            break;
                        }
                    }
                }

                std::unordered_map<std::string, Element*> srcSubmodelsWithEffects;
                if (!duplicateIncludeSourceModel.empty()) {
                    ModelElement* srcModelEl = dynamic_cast<ModelElement*>(
                        rowToRender->GetSequenceElements()->GetElement(duplicateIncludeSourceModel));
                    if (srcModelEl != nullptr) {
                        for (int x = 0; x < srcModelEl->GetSubModelAndStrandCount(); ++x) {
                            SubModelElement* srcSe = srcModelEl->GetSubModel(x);
                            if (srcSe != nullptr &&
                                srcSe->GetType() != ElementType::ELEMENT_TYPE_STRAND &&
                                srcSe->HasEffects()) {
                                srcSubmodelsWithEffects[srcSe->GetName()] = srcSe;
                            }
                        }
                    }
                }

                for (int x = 0; x < row->GetSubModelAndStrandCount(); ++x) {
                    SubModelElement *se = row->GetSubModel(x);
                    const bool addForInheritedDuplicate = !srcSubmodelsWithEffects.empty() &&
                                                          se->GetType() != ElementType::ELEMENT_TYPE_STRAND;
                    if (se->HasEffects()) {
                        if (se->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                            StrandElement *ste = (StrandElement*)se;
                            if (ste->GetStrand() < model->GetNumStrands()) {
                                subModelInfos.push_back(new EffectLayerInfo(se->GetEffectLayerCount() + 1));
                                subModelInfos.back()->element = se;
                                subModelInfos.back()->buffer.reset(new PixelBufferClass(_ctx));
                                subModelInfos.back()->strand = ste->GetStrand();
                                subModelInfos.back()->submodel = subModelInfos.size() -1;
                                subModelInfos.back()->buffer->InitStrandBuffer(*model, ste->GetStrand(), data.FrameTime(), se->GetEffectLayerCount());
                            }
                        } else {
                            Model *subModel = model->GetSubModel(se->GetName());
                            if (subModel != nullptr) {
                                int layerCount = (int)se->GetEffectLayerCount();
                                if (addForInheritedDuplicate) {
                                    auto srcIt = srcSubmodelsWithEffects.find(se->GetName());
                                    if (srcIt != srcSubmodelsWithEffects.end())
                                        layerCount = std::max(layerCount, (int)srcIt->second->GetEffectLayerCount());
                                }
                                subModelInfos.push_back(new EffectLayerInfo(layerCount + 1));
                                subModelInfos.back()->element = se;
                                subModelInfos.back()->submodel = subModelInfos.size() -1;
                                subModelInfos.back()->buffer.reset(new PixelBufferClass(_ctx));
                                subModelInfos.back()->buffer->InitBuffer(*subModel, layerCount + 1, data.FrameTime());
                            }
                        }
                    } else if (addForInheritedDuplicate) {
                        auto srcIt = srcSubmodelsWithEffects.find(se->GetName());
                        if (srcIt != srcSubmodelsWithEffects.end()) {
                            Model *subModel = model->GetSubModel(se->GetName());
                            if (subModel != nullptr) {
                                int layerCount = std::max(1, std::max((int)se->GetEffectLayerCount(),
                                                                      (int)srcIt->second->GetEffectLayerCount()));
                                subModelInfos.push_back(new EffectLayerInfo(layerCount + 1));
                                subModelInfos.back()->element = se;
                                subModelInfos.back()->submodel = subModelInfos.size() - 1;
                                subModelInfos.back()->buffer.reset(new PixelBufferClass(_ctx));
                                subModelInfos.back()->buffer->InitBuffer(*subModel, layerCount + 1, data.FrameTime());
                            }
                        }
                    }
                    if (se->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                        StrandElement *ste = (StrandElement*)se;
                        if (ste->GetStrand() < model->GetNumStrands()) {
                            for (int n = 0; n < ste->GetNodeLayerCount(); ++n) {
                                if (n < model->GetStrandLength(ste->GetStrand())) {
                                    EffectLayer *nl = ste->GetNodeLayer(n);
                                    if (nl -> GetEffectCount() > 0) {
                                        nodeBuffers[SNPair(ste->GetStrand(), n)].reset(new PixelBufferClass(_ctx));
                                        nodeBuffers[SNPair(ste->GetStrand(), n)]->InitNodeBuffer(*model, ste->GetStrand(), n, data.FrameTime());
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                delete mainBuffer;
                mainBuffer = nullptr;
            }
        } else {
            mainBuffer = nullptr;
        }
        startFrame = 0;
    }

    virtual ~RenderJob() {
        // Forced teardown (abort timed out, engine deleted mid-batch) can
        // delete a job that still owns or is parked on its row; clear the
        // row's raw pointers so a later render doesn't see a dangling job.
        if (rowToRender != nullptr) {
            rowToRender->AbandonRenderOwnership(this);
            if (attachedToRow) {
                rowToRender->DetachRenderJob();
                attachedToRow = false;
            }
        }
        if (mainBuffer != nullptr) {
            delete mainBuffer;
        }
        for (const auto& a : subModelInfos) {
            EffectLayerInfo *info = a;
            delete info;
        }
    }

    void SetProgressCallback(std::function<void(int, const std::string&)> cb) override { progressCallback = std::move(cb); }
    void UpdateProgress(int value, const std::string& tooltip = {}) override {
        if (progressCallback) progressCallback(value, tooltip);
    }
    int GetCurrentFrame() const override { return currentFrame;}
    int GetEndFrame() const override { return endFrame;}
    int GetStartFrame() const override { return startFrame;}
    const RenderJobProfile* GetRenderProfile() const override { return &profile; }

    const std::string GetName() const override {
        return name;
    }

    virtual bool DeleteWhenComplete() override {
        return false;
    }

    void LogToLogger(int logLevel) {
        // these can only be set at start time
        static bool debug = m_logger->level() == spdlog::level::debug;
        static bool info = m_logger->level() == spdlog::level::info;

        if ((debug && logLevel == spdlog::level::debug) ||
            (info && logLevel == spdlog::level::info))
        {
            m_logger->log((spdlog::level::level_enum)logLevel, GetStatusString());
        }
    }

    void SetGenericStatus(const std::string &msg, int frame, bool debugLog = false, bool includeStatusMap = false) {
        statusType = includeStatusMap ? 8 : 4;
        statusMsg = msg;
        statusFrame = frame;
        LogToLogger(debugLog ? spdlog::level::debug : spdlog::level::info);
    }

    void SetGenericStatus(const std::string &msg, int frame, int layer, bool debugLog = false, bool includeStatusMap = false) {
        statusType = includeStatusMap ? 9 : 5;
        statusMsg = msg;
        statusFrame = frame;
        statusLayer = layer;
        LogToLogger(debugLog ? spdlog::level::debug : spdlog::level::info);
    }

    std::string PrintStatusMap() const {
        if (statusMap == nullptr) return "";
        return statusMap->AsString();
    }

    void SetRenderingStatus(int frame, SettingsMap*map, int layer, int submodel, int strand, int node, bool debugLog = false) {
        statusType = 2;
        statusFrame = frame;
        statusLayer = layer;
        statusStrand = strand;
        statusSubmodel = submodel;
        statusNode = node;
        statusMap = map;
        LogToLogger(debugLog ? spdlog::level::debug : spdlog::level::info);
    }

    void SetCalOutputStatus(int frame, int submodel, int strand, int node, bool debugLog = true) {
        statusType = 3;
        statusFrame = frame;
        statusStrand = strand;
        statusSubmodel = submodel;
        statusNode = node;
        LogToLogger(debugLog ? spdlog::level::debug : spdlog::level::info);
    }

    void SetInializingStatus(int frame, int layer, int submodel, int strand, int node, bool debugLog = false) {
        statusType = 1;
        statusFrame = frame;
        statusLayer = layer;
        statusStrand = strand;
        statusSubmodel = submodel;
        statusNode = node;
        LogToLogger(debugLog ? spdlog::level::debug : spdlog::level::info);
    }
    void SetWaitingStatus(int frame, bool debugLog = false) {
        statusType = 13;
        statusFrame = frame;
    }
    void SetStatus(const std::string &st, bool debugLog = false) {
        statusMsg = st;
        statusType = 0;
        LogToLogger(debugLog ? spdlog::level::debug : spdlog::level::info);
    }

    std::string GetStatus() const override {
        return GetStatusString();
    }

    std::string GetStatusForUser() override
    {
        int lastIdx = 0;
        int submodel = -1;
        if (statusType >= 1 && statusType <= 3) {
            submodel = statusSubmodel;
        }
        Effect* effect = findEffectForFrame(this->statusLayer, GetCurrentFrame(), submodel, lastIdx);

        if (effect != nullptr) {
            std::string mname;
            if (submodel >= 0) {
                mname = "Submodel: " + subModelInfos[submodel]->element->GetName() + " ";
            }
            return fmt::format("{}Effect: {} Start: {} End {}", mname, effect->GetEffectName(),
                               FORMATTIME(effect->GetStartTimeMS()), FORMATTIME(effect->GetEndTimeMS()));
        }
        if (statusType == 13) {
            return fmt::format("Waiting to start frame {}", (int)statusFrame);
        }

        return "";
    }

    std::string GetStatusString() const override {
        int frame = statusFrame;
        int layer = statusLayer;
        int strand = statusStrand;
        int node = statusNode;
        int submodel = statusSubmodel;
        std::string n = (submodel == -1 || submodel >= (int)subModelInfos.size()) ? name : subModelInfos[submodel]->element->GetFullName();
        switch (statusType) {
        case 0:
            return statusMsg;
        case 1:
            if (strand == -1) {
                return fmt::format("Initializing effect at frame {} for {}, layer {}", frame, n, layer);
            } else if (node == -1) {
                return fmt::format("Initializing strand effect at frame {} for {}, strand {}", frame, n, strand);
            } else {
                return fmt::format("Initializing node effect at frame {} for {}, strand {}, node {}", frame, n, strand, node);
            }
        case 2:
            if (strand == -1) {
                return fmt::format("Rendering layer effect for frame {} of {}, layer {}: ", frame, n, layer) + PrintStatusMap();
            } else if (node == -1) {
                return fmt::format("Rendering strand effect for frame {} of {}, strand {}: ", frame, n, strand) + PrintStatusMap();
            } else {
                return fmt::format("Rendering node effect for frame {} of {}, strand {}, node {}: ", frame, n, layer, node) + PrintStatusMap();
            }
        case 3:
            if (strand == -1) {
                return fmt::format("Calculating output at frame {} for {}: ", frame, n) + PrintStatusMap();
            } else if (node == -1) {
                return fmt::format("Calculating output at frame {} for {}, strand {}: ", frame, n, strand) + PrintStatusMap();
            } else {
                return fmt::format("Calculating output at frame {} for {}, strand {}, node {}: ", frame, n, strand, node) + PrintStatusMap();
            }
        case 4:
            return fmt::vformat(statusMsg, fmt::make_format_args(name, frame));
        case 5:
            return fmt::vformat(statusMsg, fmt::make_format_args(name, frame, layer));
        case 8:
            return fmt::vformat(statusMsg, fmt::make_format_args(name, frame)) + PrintStatusMap();
        case 9:
            return fmt::vformat(statusMsg, fmt::make_format_args(name, frame, layer)) + PrintStatusMap();
        case 13:
            return fmt::format("Waiting to start frame {} for {}", frame, n);
        }
        return statusMsg;
    }

    SequenceData *createExportBuffer() {
        SequenceData *sb = new SequenceData();
        sb->init(_ctx->GetModel(mainBuffer->GetModelName())->GetActChanCount(), seqData->NumFrames(), seqData->FrameTime(), false);
        seqData = sb;
        return sb;
    }

    PixelBufferClass *getBuffer() const
    {
        return mainBuffer;
    }

    void setRenderRange(int start, int end) {
        startFrame = start;
        endFrame = end;
    }

    void SetRangeRestriction(const std::list<NodeRange> &rng) {
        if (!rng.empty()) {
            unsigned int maxEnd = 0;
            for (const auto& i : rng) {
                if (i.end > maxEnd) maxEnd = i.end;
            }
            rangeRestriction.resize(maxEnd + 1);
            for (const auto& i : rng) {
                for (unsigned int s = i.start; s <= i.end; s++) {
                    rangeRestriction[s] = true;
                }
            }
        }
    }

    void SetModelBlending() {
        supportsModelBlending = true;
    }

    int GetEffectFrame(Effect* ef, int frame, int frameTime)
    {
        return frame - (ef->GetStartTimeMS() / frameTime);
    }

    // Ground truth for touching seqData[frame], re-checked at output time:
    // the frame-entry gate's answer can go stale because effect edits land
    // without any lock this slice holds (effect add takes only the layer
    // mutex, effect move takes none).  It also converts any future
    // gate-coverage (superset) bug from a silent pixel race into a logged
    // skip - the dirty range re-renders the frame properly afterwards.
    bool CanOutputFrame(int frame) {
        if ((int)GetPreviousFrameDone() >= frame) {
            return true;
        }
        if (!gateMissWarned) {
            gateMissWarned = true;
            spdlog::warn("Render gate miss on {} frame {}: output produced on a frame the entry gate cleared as empty (concurrent effect edit, or a gate coverage bug). Output skipped; frame marked for re-render.", name, frame);
        }
        rowToRender->SetDirtyRange(frame * seqData->FrameTime(), (frame + 1) * seqData->FrameTime());
        return false;
    }

    // Row-local half of the old ProcessFrame (ARC phase A): renders every
    // layer's effect into `buffer` (effect render + blur/zoom + transitions +
    // this row's own canvas preload) and records what the seqData-touching
    // OutputFrame tail needs.  On the split path this stays purely row-local and
    // can run one frame ahead of the upstream wait; rows whose produce() might
    // read dependent data (canvas mix, canvas-"Blend", or Per-Model buffers) are
    // excluded from the split and gate before produce() (rowMustGateBeforeProduce,
    // see RenderFrame), where the canvas-"Blend" seqData preload below is valid.
    bool ProduceFrame(int frame, Element *el, EffectLayerInfo &info, PixelBufferClass *buffer, int strand = -1, bool blend = false, const std::string& inheritedDuplicateSourceModel = std::string()) {
        bool effectsToUpdate = false;
        Effect* tempEffect = nullptr;
        // Clamp to the layer count `info` (and the pixel buffer) were sized
        // for at job creation.  A slice holds the row's changeLock, so layer
        // geometry is frozen within a slice; cross-slice edits bump the change
        // count and the frame-loop bail forces a re-render.  The clamp is the
        // backstop for anything that mutates layers without the bail noticing.
        int numLayers = std::min((int)el->GetEffectLayerCount(), info.numLayers);
        const int effectiveNumLayers = !inheritedDuplicateSourceModel.empty() ? info.numLayers - 1 : numLayers;

        std::vector<bool>& partOfCanvas = info.partOfCanvas;
        partOfCanvas.resize(info.validLayers.size());
        for (int x = 0; x < (int)info.validLayers.size(); x++) {
            info.validLayers[x] = false;
            partOfCanvas[x] = false;
        }

        // To support canvas mix type we must render them bottom to top
        for (int layer = effectiveNumLayers - 1; layer >= 0; --layer) {
            // Layer restriction (tier-2 capture pre-pass): skip layers not opted in.
            // Empty processLayer (the default) renders every layer.
            if (layer < (int)info.processLayer.size() && !info.processLayer[layer]) {
                continue;
            }
            EffectLayer* elayer = (layer < numLayers) ? el->GetEffectLayer(layer) : nullptr;
            //must lock the layer so the Effect* stays valid
            std::unique_lock<std::recursive_mutex> elayerLock;
            if (elayer != nullptr) {
                elayerLock = std::unique_lock<std::recursive_mutex>(elayer->GetLock());
            }
            Effect* ef = nullptr;
            if (elayer != nullptr) {
                ef = findEffectForFrame(elayer, frame, info.currentEffectIdxs[layer]);
            }
            Effect* copy = nullptr;

            if (ef != nullptr && ef->GetEffectIndex() == EffectManager::eff_DUPLICATE) {
                // we are mirroring another model ... so find the right effect on that model/layer
                Effect* orig = ef;
                
                ef = findEffectForFrame(orig->GetSetting("E_CHOICE_Duplicate_Model"), orig->GetSettings().GetInt("E_SPINCTRL_Duplicate_Layer"), frame);

                if (ef != nullptr) {

                    copy = ef;

                    if (ef->GetEffectIndex() == EffectManager::eff_DUPLICATE) {
                        // we cant duplicate a duplicate
                        ef = nullptr;
                    } else {
                        tempEffect = new Effect(*ef);
                        ef = tempEffect;
                        
                        // disable the background display lists as the "duplicate" effect doesn't support this
                        tempEffect->EnableBackgroundDisplayLists(false);

                        if (orig->GetSetting("E_CHECKBOX_Duplicate_Override_Buffer") == "1") {
                            ef->EraseSettingsStartingWith("B_");
                            for (const auto& it : orig->GetSettings()) {
                                if (StartsWith(it.first, "B_"))
                                    ef->GetSettings()[it.first] = it.second;
                            }
                        }
                        if (orig->GetSetting("E_CHECKBOX_Duplicate_Override_Timing") == "1") {
                            ef->EraseSettingsStartingWith("T_");
                            for (const auto& it : orig->GetSettings()) {
                                if (StartsWith(it.first, "T_"))
                                    ef->GetSettings()[it.first] = it.second;
                            }
                        }
                        if (orig->GetSetting("E_CHECKBOX_Duplicate_Override_Palette") == "1") {
                            ef->ErasePalette();
                            for (const auto& it : orig->GetPaletteMap()) {
                                if (StartsWith(it.first, "C_BUTTON_Palette") || StartsWith(it.first, "C_CHECKBOX_Palette"))
                                    ef->GetPaletteMap()[it.first] = it.second;
                            }
                            ef->SetPalette(ef->GetPaletteAsString()); // this forces the colour parsing
                        }
                        if (orig->GetSetting("E_CHECKBOX_Duplicate_Override_Color") == "1") {
                            ef->EraseColourSettings();
                            for (const auto& it : orig->GetPaletteMap()) {
                                if (!StartsWith(it.first, "C_BUTTON_Palette") && !StartsWith(it.first, "C_CHECKBOX_Palette"))
                                    ef->GetPaletteMap()[it.first] = it.second;
                            }
                        }
                    }
                }
            } else if (!inheritedDuplicateSourceModel.empty()) {
                if (auto* sme = dynamic_cast<SubModelElement*>(el)) {
                    const std::string srcSubmodel = inheritedDuplicateSourceModel + "/" + sme->GetName();
                    Effect* srcEf = findEffectForFrame(srcSubmodel, layer + 1, frame);
                    if (srcEf != nullptr && srcEf->GetEffectIndex() != EffectManager::eff_DUPLICATE) {
                        copy = srcEf;
                        tempEffect = new Effect(*srcEf);
                        ef = tempEffect;
                        ef->EnableBackgroundDisplayLists(false);
                    }
                }
            }

            Effect* compare = copy != nullptr ? copy : ef;

            if (compare != info.currentEffects[layer]) {
                if (copy != nullptr) {
                    info.currentEffects[layer] = copy;
                } else {
                    info.currentEffects[layer] = ef;
                }
                SetInializingStatus(frame, layer, info.submodel, strand, -1);
                initialize(layer, frame, ef, info.settingsMaps[layer], buffer);
                info.effectStates[layer] = true;
            }

            if (buffer->IsVariableSubBuffer(layer)) {
                buffer->PrepareVariableSubBuffer(frame, layer);
            }

            bool persist = buffer->IsPersistent(layer);
            bool freeze = false;
            if (ef != nullptr && buffer != nullptr) {
                freeze = buffer->GetFreezeFrame(layer) != 999999 && buffer->GetFreezeFrame(layer) <= GetEffectFrame(ef, frame, mainBuffer->GetFrameTimeInMS());
            }

            if ((!persist && !freeze) || info.currentEffects[layer] == nullptr || info.currentEffects[layer]->GetEffectIndex() == -1) {
                buffer->Clear(layer);
            }

            bool suppress = false;
            if (ef != nullptr && buffer != nullptr) {
                suppress = buffer->GetSuppressUntil(layer) > GetEffectFrame(ef, frame, mainBuffer->GetFrameTimeInMS());
            }

            SetRenderingStatus(frame, &info.settingsMaps[layer], layer, info.submodel, strand, -1, true);
            bool b = info.effectStates[layer];

            if (!freeze) {
                // Mix canvas pre-loads the buffer with data from underlying layers
                if (buffer->IsCanvasMix(layer) && layer < numLayers - 1 && !buffer->IsRenderingDisabled(layer)) {
                    auto vl = info.validLayers;
                    bool doBlendLayer = false;
                    if (info.settingsMaps[layer].Get("LayersSelected", "") != "") {
                        // remove from valid layers any layers we dont need to include
                        std::vector<std::string> ls;
                        Split(info.settingsMaps[layer].Get("LayersSelected", ""), '|', ls);
                        if (!ls.empty() && ls.back() == "Blend") {
                            doBlendLayer = true;
                            ls.pop_back();
                        }
                        for (int i = layer + 1; i < (int)vl.size(); i++) {
                            if (vl[i]) {
                                bool found = false;
                                for (auto it = ls.begin(); !found && it != ls.end(); ++it) {
                                    if (std::atoi((*it).c_str()) + layer + 1 == i) {
                                        found = true;
                                    }
                                }
                                if (!found) {
                                    vl[i] = false;
                                } else {
                                    partOfCanvas[i] = true;
                                }
                            }
                        }
                        if (doBlendLayer && CanOutputFrame(frame)) {
                            { StageTimer st(profRender ? &profile.setColorsNs : nullptr);
                              buffer->SetColors(numLayers, &((*seqData)[frame][0]), seqData->NumChannels()); }
                            vl[numLayers] = true;
                            blend = false;
                        }
                    } else {
                        // default if not specified is all valid layers below it except the blend layer
                        // mark them as being part of the
                        for (int i = layer + 1; i < (int)vl.size(); i++) {
                            if (vl[i]) {
                                partOfCanvas[i] = true;
                            }
                        }
                    }

                    // preload the buffer with the output from the lower layers
                    RenderBuffer& rb = buffer->BufferForLayer(layer, -1);

                    // We have to calc the output here to apply blend, rotozoom and transitions
                    { StageTimer st(profRender ? &profile.blendNs : nullptr);
                      buffer->CalcOutput(frame, vl, layer, true); }
                    std::vector<uint8_t> done(rb.GetPixelCount());
                    parallel_for(0, rb.GetNodes().size(), [&](int n) {
                        for (auto &a : rb.GetNodes()[n]->Coords) {
                            int x = a.bufX;
                            int y = a.bufY;
                            if (x >= 0 && x < rb.BufferWi && y >= 0 && y < rb.BufferHt && y*rb.BufferWi + x < (int)rb.GetPixelCount()) {
                                done[y*rb.BufferWi+x] = true;
                            }
                        }
                    }, 500);
                    // now fill in any spaces in the buffer that don't have nodes mapped to them
                    buffer->PrepareMixedColorParams(vl, frame);
                    parallel_for(0, rb.BufferHt, [&rb, &buffer, &done, &vl, frame](int y) {
                        xlColor c;
                        for (int x = 0; x < rb.BufferWi; x++) {
                            if (!done[y * rb.BufferWi + x]) {
                                buffer->GetMixedColor(x, y, c, vl, frame);
                                rb.SetPixel(x, y, c);
                            }
                        }
                        });
                    buffer->UnMergeBuffersForLayer(layer);

                    if (xldbgEffSum) {
                        fprintf(stderr, "SUM C f=%d m=%s s=%d l=%d h=%016llx\n", frame, el->GetFullName().c_str(), strand, layer,
                                (unsigned long long)xldbgFNV((const uint8_t*)rb.GetPixels(), rb.GetPixelCount() * 4));
                    }
                }

                info.validLayers[layer] = _engine->RenderEffectFromMap(suppress, ef, layer, frame, info.settingsMaps[layer], *buffer, b);
                effectsToUpdate |= info.validLayers[layer];
                info.effectStates[layer] = b;

                if (suppress) {
                    info.validLayers[layer] = false;
                } else if (info.validLayers[layer]
                           && buffer->BufferForLayer(layer, -1).captureSnapshot == nullptr) {
                    // Skip blur/rotozoom/transitions during the tier-2 capture
                    // pre-pass (captureSnapshot set): they are per-frame stateless
                    // and recomputed on the parallel clone buffers in the draw
                    // pass, so doing them here just encodes GPU work whose pixels
                    // are discarded (this is what the RenderParallelWindow quiesce
                    // loop had to clean up - now a no-op).
                    { StageTimer st(profRender ? &profile.blurZoomNs : nullptr);
                      buffer->HandleLayerBlurZoom(frame, layer); }
                    { StageTimer st(profRender ? &profile.transitionNs : nullptr);
                      buffer->HandleLayerTransitions(frame, layer); }
                }

                if (xldbgEffSum && info.validLayers[layer]) {
                    RenderBuffer& rbl = buffer->BufferForLayer(layer, -1);
                    GPURenderUtils::waitForRenderCompletion(&rbl);
                    fprintf(stderr, "SUM L f=%d m=%s s=%d l=%d h=%016llx\n", frame, el->GetFullName().c_str(), strand, layer,
                            (unsigned long long)xldbgFNV((const uint8_t*)rbl.GetPixels(), rbl.GetPixelCount() * 4));

                    // XLDBG_LDUMP="<model>:<layer>:<frame>:<outfile>" dumps the raw layer pixels
                    static const char* ldump = getenv("XLDBG_LDUMP");
                    if (ldump != nullptr) {
                        static std::string ldModel, ldFile;
                        static int ldLayer = -1, ldFrame = -1;
                        if (ldLayer == -1) {
                            std::string spec = ldump;
                            size_t a = spec.find(':');
                            size_t b = spec.find(':', a + 1);
                            size_t c = spec.find(':', b + 1);
                            ldModel = spec.substr(0, a);
                            ldLayer = atoi(spec.substr(a + 1, b - a - 1).c_str());
                            ldFrame = atoi(spec.substr(b + 1, c - b - 1).c_str());
                            ldFile = spec.substr(c + 1);
                        }
                        if (frame >= ldFrame && frame <= ldFrame + 60 && layer == ldLayer && el->GetFullName() == ldModel) {
                            std::string path = ldFile + "." + std::to_string(frame) + ".bin";
                            FILE* f = fopen(path.c_str(), "wb");
                            if (f != nullptr) {
                                int wi = rbl.BufferWi, ht = rbl.BufferHt;
                                fwrite(&wi, 4, 1, f);
                                fwrite(&ht, 4, 1, f);
                                fwrite(rbl.GetPixels(), 4, rbl.GetPixelCount(), f);
                                fclose(f);
                            }
                        }
                    }
                }
            } else {
                info.validLayers[layer] = true;
                info.effectStates[layer] = b;
                effectsToUpdate = true;
            }
        }

        info.produceEffectiveNumLayers = effectiveNumLayers;
        info.produceBlend = blend;
        info.produceEffectsToUpdate = effectsToUpdate;

        if (tempEffect != nullptr)
            delete tempEffect;

        return effectsToUpdate;
    }

    // seqData-touching half of the old ProcessFrame (ARC phase A): the tail
    // that blends the produced layers into seqData.  Runs only after the
    // upstream gate has cleared this frame (CanOutputFrame re-verifies as the
    // TOCTOU backstop).  Consumes what ProduceFrame stored on `info`.
    void OutputFrame(int frame, Element *el, EffectLayerInfo &info, PixelBufferClass *buffer, int strand = -1) {
        const bool effectsToUpdate = info.produceEffectsToUpdate;
        const bool blend = info.produceBlend;
        const int effectiveNumLayers = info.produceEffectiveNumLayers;
        std::vector<bool>& partOfCanvas = info.partOfCanvas;

        if (effectsToUpdate && CanOutputFrame(frame)) {
            SetCalOutputStatus(frame, info.submodel, strand, -1);
            for (int x = 0; x < (int)partOfCanvas.size(); x++) {
                // if the layer was used for a canvas effect, we don't want it
                // reblended in
                if (partOfCanvas[x]) {
                    info.validLayers[x] = false;
                }
            }
            if (blend) {
                { StageTimer st(profRender ? &profile.setColorsNs : nullptr);
                  buffer->SetColors(effectiveNumLayers, &((*seqData)[frame][0]), seqData->NumChannels()); }
                info.validLayers[effectiveNumLayers] = true;
            }
            if (xldbgEffSum && blend) {
                RenderBuffer& blrb = buffer->BufferForLayer(effectiveNumLayers, -1);
                fprintf(stderr, "SUM B f=%d m=%s s=%d pfd=%d h=%016llx\n", frame, el->GetFullName().c_str(), strand, (int)GetPreviousFrameDone(),
                        (unsigned long long)xldbgFNV((const uint8_t*)blrb.GetPixels(), blrb.GetPixelCount() * 4));
            }
            { StageTimer st(profRender ? &profile.blendNs : nullptr);
              buffer->CalcOutput(frame, info.validLayers); }
            if (xldbgEffSum) {
                uint64_t h = 1469598103934665603ULL;
                for (const auto& n : buffer->BufferForLayer(0, -1).GetNodes()) {
                    xlColor c;
                    n->GetColor(c);
                    uint32_t v = c.GetRGBA();
                    const uint8_t* d = (const uint8_t*)&v;
                    for (int i = 0; i < 4; i++) {
                        h ^= d[i];
                        h *= 1099511628211ULL;
                    }
                }
                fprintf(stderr, "SUM N f=%d m=%s s=%d h=%016llx\n", frame, el->GetFullName().c_str(), strand, (unsigned long long)h);

                // XLDBG_NDUMP="<model>:<startFrame>:<endFrame>" dumps every
                // node's post-blend color in the window for A/B comparison
                static const char* ndump = getenv("XLDBG_NDUMP");
                if (ndump != nullptr) {
                    static std::string ndModel;
                    static int ndS = -1, ndE = -1;
                    if (ndS == -1) {
                        std::string spec = ndump;
                        size_t a = spec.find(':');
                        size_t b = spec.rfind(':');
                        ndModel = spec.substr(0, a);
                        ndS = atoi(spec.substr(a + 1, b - a - 1).c_str());
                        ndE = atoi(spec.substr(b + 1).c_str());
                    }
                    if (frame >= ndS && frame <= ndE && el->GetFullName() == ndModel) {
                        int ni = 0;
                        for (const auto& n : buffer->BufferForLayer(0, -1).GetNodes()) {
                            xlColor c;
                            n->GetColor(c);
                            fprintf(stderr, "ND f=%d n=%d c=%08x\n", frame, ni++, c.GetRGBA());
                        }
                    }
                }
            }
            { StageTimer st(profRender ? &profile.getColorsNs : nullptr);
              buffer->GetColors(&((*seqData)[frame][0]), rangeRestriction, seqData->NumChannels()); }

            if (xldbgEffSum) {
                // hash only this row's actual node channels — the model span
                // of a group includes gap channels owned by rows this one is
                // not ordered against, which made span hashes racy artifacts
                uint64_t h = 1469598103934665603ULL;
                for (const auto& n : buffer->BufferForLayer(0, -1).GetNodes()) {
                    uint32_t start = n->ActChan;
                    uint32_t cnt = n->GetChanCount();
                    if (start + cnt <= seqData->NumChannels()) {
                        const uint8_t* d = &((*seqData)[frame][start]);
                        for (uint32_t i = 0; i < cnt; i++) {
                            h ^= d[i];
                            h *= 1099511628211ULL;
                        }
                    }
                }
                fprintf(stderr, "SUM O f=%d m=%s s=%d pfd=%d h=%016llx\n", frame, el->GetFullName().c_str(), strand, (int)GetPreviousFrameDone(), (unsigned long long)h);
            }

            // Position Zone processing (DMX)
            if (_ctx->GetEnablePositionZones()) {
                const Model* model = buffer->GetModel();
                if (model != nullptr && model->GetDisplayAs() == DisplayAsType::DmxMovingHeadAdv) {
                    const DmxMovingHeadAdv* mh = dynamic_cast<const DmxMovingHeadAdv*>(model);
                    if (mh != nullptr) {
                        mh->ApplyPositionZones(&((*seqData)[frame][0]), mh->GetFirstChannel());
                    }
                }
            }
        }
    }

    // Scheduling state for the suspend/requeue scheduler
    // (phase 3 of plans/render-scheduler.md).
    enum class SchedPhase { Setup, Frames, Finish, Done };
    enum class FrameResult { Continue, Suspend, Stop };

    // Idempotent requeue: inPool is set while the job sits in the pool queue
    // and cleared at slice entry, so concurrent wake paths (upstream FrameDone,
    // owner handoff, abort, watchdog rescue) can all call this without ever
    // double-queuing the job - only one of them wins the CAS.
    void Requeue() {
        bool expected = false;
        if (inPool.compare_exchange_strong(expected, true)) {
            _engine->RequeueJob(this);
        }
    }

    // Called by the upstream renderer/aggregator as it completes each frame.
    // If this job suspended waiting for that frame, hand it back to the pool
    // instead of waking a sleeping thread.
    virtual void setPreviousFrameDone(int frame) override {
        bool requeue = false;
        {
            std::unique_lock<std::mutex> lock(nextLock);
            if (frame > previousFrameDone) {
                previousFrameDone = frame;
            }
            if (suspended && previousFrameDone >= wantFrame) {
                suspended = false;
                requeue = true;
            }
        }
        if (requeue) {
            Requeue();
        }
    }

    // Register to be rescheduled when upstream reaches the frame.  Returns
    // false if the frame is already available - keep running.
    bool trySuspendUntil(int frame) {
        std::unique_lock<std::mutex> lock(nextLock);
        if (previousFrameDone >= frame) {
            return false;
        }
        wantFrame = frame;
        suspended = true;
        if (_rpi) {
            ++_rpi->suspendCount;
        }
        // The slice effectively ends here: once nextLock releases, a waker may
        // requeue the job onto another pool thread (or it may complete and be
        // deleted) while this thread is still unwinding, so this is the last
        // point that may touch profile/members.  The resume delta is added at
        // the next ProcessSlice entry; always-on because the batch summary
        // reports total suspended time.
        EndSliceProfile();
        suspendStartTime = std::chrono::steady_clock::now();
        suspendTimingPending = true;
        if (profRender) {
            ++profile.suspends;
        }
        return true;
    }

    // Slice-profile segment control (XL_RENDER_PROFILE).  Begin/End must only
    // run while this thread exclusively owns the job's slice; End is called at
    // every point after which the job may be rescheduled or deleted
    // (trySuspendUntil success, park publication, CompleteJob).
    void BeginSliceProfile() {
        if (profRender) {
            sliceStartTime = std::chrono::steady_clock::now();
            tlsRenderProfile = &profile;
            sliceProfileArmed = true;
        }
    }
    void EndSliceProfile() {
        if (sliceProfileArmed) {
            profile.sliceNs += xlProfNs(sliceStartTime, std::chrono::steady_clock::now());
            tlsRenderProfile = nullptr;
            sliceProfileArmed = false;
        }
    }

    // Watchdog/abort rescue.  Covers every threadless-idle state IsIdle()
    // reports: a suspended job is requeued to re-check its frame, and a
    // parked job is pulled from the row queue (or, if it was promoted but
    // its handoff requeue was lost, requeued directly - the inPool CAS makes
    // a duplicate wake harmless).  A false alarm just re-parks/re-suspends.
    void NudgeIfSuspended() override {
        bool wasSuspended = false;
        bool wasParked = false;
        {
            std::unique_lock<std::mutex> lock(nextLock);
            if (suspended) {
                suspended = false;
                wasSuspended = true;
            } else if (parked) {
                wasParked = true;
            }
        }
        if (wasSuspended) {
            Requeue();
        } else if (wasParked) {
            rowToRender->CancelParkedRenderJob(this);
            Requeue();
        }
    }

    // True when the job holds no thread and is waiting to be rescheduled
    // (suspended on an upstream frame, or parked behind the row's owner).
    // The stall watchdog only fires on a batch whose unfinished jobs are all
    // idle - a job actively rendering a slow frame is not a stall.
    bool IsIdle() override {
        std::unique_lock<std::mutex> lock(nextLock);
        return suspended || parked;
    }

    // Whether this frame can read or write seqData[frame] and therefore must
    // wait for upstream renderers first.  Must stay a superset of the output
    // paths: main-model output requires an effect covering the frame on a main
    // layer; rows with submodels or node buffers always sync (they previously
    // waited on every frame).  Frames with no effects skip the gate so a row
    // with sparse coverage races through the gaps without trailing upstream.
    // gateEffectIdxs advances monotonically with the frame loop, so each
    // effect list is scanned once per render, not once per frame; on a false
    // return, gateSkipUntilFrame records the next covered frame so the caller
    // doesn't even re-enter (or re-lock the layers) during the gap.
    bool NeedsUpstreamFrame(int frame) {
        if (!subModelInfos.empty() || !nodeBuffers.empty()) {
            return true;
        }
        int time = frame * seqData->FrameTime();
        int nextStartMS = INT_MAX;
        for (int layer = 0; layer < numLayers; ++layer) {
            EffectLayer *elayer = rowToRender->GetEffectLayer(layer);
            if (elayer == nullptr) {
                continue;
            }
            std::unique_lock<std::recursive_mutex> elock(elayer->GetLock());
            int cnt = elayer->GetEffectCount();
            int &idx = gateEffectIdxs[layer];
            if (idx > cnt) {
                idx = cnt;
            }
            // effects fully before this frame can never cover a later one
            while (idx < cnt && elayer->GetEffect(idx)->GetEndTimeMS() <= time) {
                ++idx;
            }
            if (idx < cnt) {
                Effect *effect = elayer->GetEffect(idx);
                if (effect->GetStartTimeMS() <= time) {
                    return true;
                }
                // effects are time-ordered; this is the layer's next coverage
                nextStartMS = std::min(nextStartMS, effect->GetStartTimeMS());
            }
        }
        gateSkipUntilFrame = (nextStartMS == INT_MAX)
            ? INT_MAX
            : (nextStartMS + seqData->FrameTime() - 1) / seqData->FrameTime();
        return false;
    }

    void ComputeRenderRange() {
        int ss, es;
        rowToRender->GetAndResetDirtyRange(origChangeCount, ss, es);
        if (ss != -1) {
            //expand to cover the whole dirty range
            ss = ss / seqData->FrameTime();
            if (ss < 0) {
                ss = 0;
            }
            es = es / seqData->FrameTime();
            if (es > (int)seqData->NumFrames()) {
                es = seqData->NumFrames();
            }
            if (ss < startFrame) {
                startFrame = ss;
            }
            if (es > endFrame) {
                endFrame = es;
            }
        }
        if (startFrame < 0) startFrame = 0;
        if (endFrame >= (int)seqData->NumFrames()) endFrame = seqData->NumFrames() - 1;
    }

    // See xldbgParallelWindows.  Classifies [startFrame,endFrame] into
    // parallel-safe (every covering effect declares Pure), serial (some covering
    // effect is Stateful) and empty (no coverage) frames, then measures the
    // contiguous parallel-safe runs a frame-parallel scheduler could farm out.
    void AnalyzeParallelWindows() {
        int sf = startFrame, ef = endFrame;
        int nframes = ef - sf + 1;
        if (nframes <= 0) return;
        std::vector<uint8_t> hasCover(nframes, 0), allPure(nframes, 1);
        int frameTime = seqData->FrameTime();
        for (int l = 0; l < numLayers; ++l) {
            EffectLayer* el = rowToRender->GetEffectLayer(l);
            if (el == nullptr) continue;
            std::unique_lock<std::recursive_mutex> lock(el->GetLock());
            for (int e = 0; e < el->GetEffectCount(); ++e) {
                Effect* eff = el->GetEffect(e);
                if (eff == nullptr) continue;
                RenderableEffect* reff = _ctx->GetEffectManager().GetEffect(eff->GetEffectIndex());
                bool pure = false;
                if (reff != nullptr) {
                    SettingsMap sm;
                    eff->CopySettingsMap(sm, true);
                    pure = reff->GetEffectiveFrameParallelism(sm) == RenderableEffect::FrameParallelism::Pure;
                }
                int s = std::max((int)(eff->GetStartTimeMS() / frameTime), sf);
                int en = std::min((int)(eff->GetEndTimeMS() / frameTime), ef);
                for (int f = s; f <= en; ++f) {
                    int idx = f - sf;
                    hasCover[idx] = 1;
                    if (!pure) allPure[idx] = 0;
                }
            }
        }
        int parallel = 0, serial = 0, empty = 0, windows = 0, cur = 0, maxw = 0;
        int inW4 = 0, inW16 = 0, inW64 = 0;
        std::vector<int> runs;
        for (int i = 0; i < nframes; ++i) {
            bool p = hasCover[i] && allPure[i];
            if (!hasCover[i]) empty++;
            else if (p) parallel++;
            else serial++;
            if (p) {
                cur++;
            } else if (cur > 0) {
                runs.push_back(cur);
                maxw = std::max(maxw, cur);
                windows++;
                cur = 0;
            }
        }
        if (cur > 0) { runs.push_back(cur); maxw = std::max(maxw, cur); windows++; }
        for (int r : runs) {
            if (r >= 4) inW4 += r;
            if (r >= 16) inW16 += r;
            if (r >= 64) inW64 += r;
        }
        const Model* m = mainBuffer != nullptr ? mainBuffer->GetModel() : nullptr;
        bool isGroup = m != nullptr && m->GetDisplayAs() == DisplayAsType::ModelGroup;
        fprintf(stderr, "XL_PARALLEL_WINDOWS %s'%s' frames=%d parallel=%d serial=%d empty=%d windows=%d maxwin=%d inWin>=4=%d >=16=%d >=64=%d\n",
                isGroup ? "[GROUP] " : "", name.c_str(), nframes, parallel, serial, empty, windows, maxw, inW4, inW16, inW64);
    }

    // See xldbgParallelFrames.  True if EVERY covering effect at `frame` is Pure
    // (this frame can render out of order); false if any covering effect is
    // Stateful.  A frame with no coverage returns false - it renders nothing, so
    // there is nothing to parallelise.
    // A buffer-continuity effect (OverlayBkg/Freeze/Suppress) does not clear its
    // buffer between frames - it reads the *serial* mainBuffer's prior content.
    // Its own frames are already Stateful (GetEffectiveFrameParallelism vetoes
    // them) and render serially, so the mainBuffer stays continuous WITHIN the
    // effect.  (Genuinely-stateful effects like Fire re-init at their start and
    // don't need this.)
    static bool EffectIsBufferContinuity(Effect* eff) {
        SettingsMap sm;
        eff->CopySettingsMap(sm, true);
        return sm.GetBool("CHECKBOX_OverlayBkg", false)
            || sm.GetInt("SPINCTRL_FreezeEffectAtFrame", 999999) < 999999
            || sm.GetInt("SPINCTRL_SuppressEffectUntil", 0) > 0;
    }

    // A frame is parallel-safe if every covering layer is Pure OR Snapshottable.
    // hasSnapshot is SET (never cleared - the caller inits it) when any covering
    // layer is Snapshottable, so the window knows it needs a serial capture pre-pass.
    bool FrameIsParallelSafe(int frame, bool& hasSnapshot) {
        bool anyCover = false;
        for (int l = 0; l < numLayers; ++l) {
            EffectLayer* el = rowToRender->GetEffectLayer(l);
            if (el == nullptr) continue;
            std::unique_lock<std::recursive_mutex> lock(el->GetLock());
            int idx = 0;
            Effect* eff = findEffectForFrame(el, frame, idx);
            // The enter transition into a buffer-continuity effect: if one STARTS
            // at frame+1, this frame must render serially so the mainBuffer is
            // advanced/cleared for it (a Pure window would render it on a clone and
            // leave the mainBuffer stale).  Only a CHANGE of covering effect at
            // frame+1 is a boundary worth inspecting - the common case (same effect
            // spans both frames) is a cheap pointer compare that skips the settings
            // read.
            int idx2 = 0;
            Effect* effNext = findEffectForFrame(el, frame + 1, idx2);
            if (effNext != nullptr && effNext != eff && EffectIsBufferContinuity(effNext)) {
                return false;
            }
            if (eff == nullptr) continue;
            anyCover = true;
            RenderableEffect* reff = _ctx->GetEffectManager().GetEffect(eff->GetEffectIndex());
            if (reff == nullptr) return false;
            SettingsMap sm;
            eff->CopySettingsMap(sm, true);
            RenderableEffect::FrameParallelism fp = reff->GetEffectiveFrameParallelism(sm);
            if (fp == RenderableEffect::FrameParallelism::Snapshottable) {
                hasSnapshot = true;
            } else if (fp != RenderableEffect::FrameParallelism::Pure) {
                return false; // Stateful
            }
        }
        return anyCover;
    }

    // The covering effect on `layer` at `frame` is Snapshottable (needs the serial
    // capture pre-pass).  Used to restrict that pre-pass to just those layers.
    bool LayerIsSnapshottableAt(int layer, int frame) {
        EffectLayer* el = rowToRender->GetEffectLayer(layer);
        if (el == nullptr) return false;
        std::unique_lock<std::recursive_mutex> lock(el->GetLock());
        int idx = 0;
        Effect* eff = findEffectForFrame(el, frame, idx);
        if (eff == nullptr) return false;
        RenderableEffect* reff = _ctx->GetEffectManager().GetEffect(eff->GetEffectIndex());
        if (reff == nullptr) return false;
        SettingsMap sm;
        eff->CopySettingsMap(sm, true);
        return reff->GetEffectiveFrameParallelism(sm) == RenderableEffect::FrameParallelism::Snapshottable;
    }

    // XL_PARALLEL_BLOCKERS profiling.  Walk every frame of this (eligible group)
    // row, attribute each blocked frame to the effect(s) preventing a parallel
    // window, and merge into the process-wide tally.  Read-only; never perturbs
    // the render (runs once at row init, only when xldbgParBlockers).
    enum class BlockerTally { Group, Model, SubmodelRow };
    void AnalyzeFrameBlockers(BlockerTally tally = BlockerTally::Group) {
        ParBlockerData local;
        // The layer set to classify: the main-model layers, plus - for rows
        // excluded only by their submodel/strand effects (item-03 step 3
        // sizing) - every submodel layer, since step 3 would have to clone
        // those buffers too and their effects gate the frame.
        std::vector<EffectLayer*> layers;
        for (int l = 0; l < numLayers; ++l) {
            layers.push_back(rowToRender->GetEffectLayer(l));
        }
        if (tally == BlockerTally::SubmodelRow) {
            for (const auto& smi : subModelInfos) {
                Element* se = smi->element;
                if (se == nullptr) continue;
                for (int l = 0; l < se->GetEffectLayerCount(); ++l) {
                    layers.push_back(se->GetEffectLayer(l));
                }
            }
        }
        int sf = startFrame, ef = endFrame;
        for (int frame = sf; frame <= ef; ++frame) {
            std::set<std::string> statefulB, continuityB;
            bool nextBoundary = false, anyCover = false;
            for (EffectLayer* el : layers) {
                if (el == nullptr) continue;
                std::unique_lock<std::recursive_mutex> lock(el->GetLock());
                int idx = 0;
                Effect* eff = findEffectForFrame(el, frame, idx);
                int idx2 = 0;
                Effect* effNext = findEffectForFrame(el, frame + 1, idx2);
                if (effNext != nullptr && effNext != eff && EffectIsBufferContinuity(effNext)) {
                    nextBoundary = true;
                }
                if (eff == nullptr) continue;
                anyCover = true;
                RenderableEffect* reff = _ctx->GetEffectManager().GetEffect(eff->GetEffectIndex());
                if (reff == nullptr) {
                    statefulB.insert("<unknown>");
                    continue;
                }
                SettingsMap sm;
                eff->CopySettingsMap(sm, true);
                RenderableEffect::FrameParallelism base = reff->GetFrameParallelism(sm);
                RenderableEffect::FrameParallelism eff2 = reff->GetEffectiveFrameParallelism(sm);
                if (base == RenderableEffect::FrameParallelism::Stateful) {
                    statefulB.insert(reff->Name());
                } else if (eff2 == RenderableEffect::FrameParallelism::Stateful) {
                    // base is Pure/Snapshottable but a continuity setting vetoed it
                    continuityB.insert(reff->Name());
                }
            }
            if (!anyCover) {
                ++local.eligEmpty;
                continue;
            }
            int blockerCount = (int)statefulB.size() + (int)continuityB.size() + (nextBoundary ? 1 : 0);
            if (blockerCount == 0) {
                ++local.eligSafe;
                continue;
            }
            ++local.eligBlocked;
            for (const auto& n : statefulB) ++local.statefulAny[n];
            for (const auto& n : continuityB) ++local.continuityAny[n];
            if (nextBoundary) ++local.nextBoundaryFrames;
            if (blockerCount == 1 && statefulB.size() == 1) {
                ++local.statefulSole[*statefulB.begin()];
            }
        }
        local.rowsEligible = 1;
        if (tally != BlockerTally::Group) {
            const RenderBuffer& rb = mainBuffer->BufferForLayer(0, -1);
            local.rows.push_back({ rowToRender->GetModelName(),
                                   (uint64_t)rb.BufferWi * (uint64_t)rb.BufferHt,
                                   local.eligSafe, local.eligSafe + local.eligBlocked });
        }
        ParBlockerStats& g = parBlockers();
        std::lock_guard<std::mutex> lg(g.mtx);
        ParBlockerData& target = tally == BlockerTally::Group ? g.d
            : tally == BlockerTally::Model                    ? g.m
                                                              : g.s;
        target.merge(local);
    }

    void EnsureParPool(int n) {
        const Model* mdl = mainBuffer->GetModel();
        while ((int)parBuffers.size() < n) {
            auto b = std::make_unique<PixelBufferClass>(_ctx);
            b->InitBuffer(*mdl, numLayers, seqData->FrameTime());
            parBuffers.push_back(std::move(b));
            parInfos.push_back(std::make_unique<EffectLayerInfo>(numLayers));
        }
    }

    // Render frames [a,e] FULLY (produce + blur/transitions + blend/CalcOutput +
    // the seqData write) concurrently, each into its own clone buffer.  Every
    // frame writes only seqData[frame] - a distinct row - so the whole per-frame
    // pipeline is independent, not just produce().  The caller must have already
    // satisfied the upstream gate through `e` (OutputFrame's CanOutputFrame is a
    // backstop) and emits FrameDone in frame order afterwards, so downstream
    // still sees monotonic completion.  Each clone's EffectLayerInfo is reset so
    // ProduceFrame re-initialises from scratch (output-invariant for Pure effects).
    //
    // hasSnapshot: the window covers >=1 Snapshottable layer.  A Snapshottable
    // effect can't be re-simulated independently per clone (its advance carries
    // frame-to-frame state), so a SERIAL pre-pass first advances the simulation on
    // the main buffer and captures each frame's immutable draw snapshot (per layer)
    // without drawing (buffer.captureSnapshot); the parallel pass then draws those
    // snapshots (buffer.pendingSnapshot) instead of re-advancing.  Pure layers are
    // untouched by the pre-pass (their Render ignores captureSnapshot) and rendered
    // fresh in the parallel pass exactly as before.  Only the draw is parallel; the
    // cheap advance stays serial, so the effect is byte-identical.
    void RenderParallelWindow(int a, int e, bool hasSnapshot) {
        int n = e - a + 1;
        EnsureParPool(n);
        // snaps[layer][frame-a]: the captured draw snapshot, or null for Pure/empty layers.
        std::vector<std::vector<std::unique_ptr<EffectFrameState>>> snaps;
        if (hasSnapshot) {
            if (xldbgParallelWindows) {
                fprintf(stderr, "XL_PARALLEL_WINDOWS SNAP-WINDOW m='%s' a=%d e=%d layers=%d\n",
                        rowToRender->GetModelName().c_str(), a, e, numLayers);
            }
            snaps.resize(numLayers);
            for (auto& lv : snaps) lv.resize(n);
            for (int f = a; f <= e; ++f) {
                // Restrict the (serial) pre-pass to Snapshottable layers only: they
                // advance the sim + capture.  Pure layers are skipped entirely (they
                // may run expensive parallel_fors) and rendered fresh in the parallel
                // pass below.
                mainModelInfo.processLayer.assign(numLayers, false);
                for (int l = 0; l < numLayers; ++l) {
                    if (LayerIsSnapshottableAt(l, f)) {
                        mainModelInfo.processLayer[l] = true;
                        mainBuffer->BufferForLayer(l, -1).captureSnapshot = &snaps[l][f - a];
                    }
                }
                ProduceFrame(f, rowToRender, mainModelInfo, mainBuffer, -1, supportsModelBlending);
                for (int l = 0; l < numLayers; ++l) {
                    mainBuffer->BufferForLayer(l, -1).captureSnapshot = nullptr;
                }
            }
            // Restore full-layer rendering for any later serial frame on the main buffer.
            mainModelInfo.processLayer.clear();
            // Quiesce the main buffer's GPU state as a belt-and-suspenders
            // backstop.  ProduceFrame now SKIPs blur/rotozoom/transitions on
            // captured layers (captureSnapshot set), so the pre-pass should no
            // longer encode any GPU work here and this loop is expected to be a
            // no-op (cheap when there is nothing outstanding).  Kept because if
            // an effect's capture path ever encodes GPU work, leaving it open
            // would commit stale blits at the next serial frame's first wait,
            // corrupting exactly that frame (then self-healing).
            for (int l = 0; l < numLayers; ++l) {
                GPURenderUtils::waitForRenderCompletion(&mainBuffer->BufferForLayer(l, -1));
            }
        }
        for (int i = 0; i < n; ++i) {
            *parInfos[i] = EffectLayerInfo(numLayers);
        }
        static ParallelJobPool PAR_FRAME_POOL("par_frame_pool", PAR_FRAME_MAX_CHUNK);
        parallel_for(0, n, [this, a, hasSnapshot, &snaps](int i) {
            int f = a + i;
            if (hasSnapshot) {
                for (int l = 0; l < numLayers; ++l) {
                    if (snaps[l][i]) parBuffers[i]->BufferForLayer(l, -1).pendingSnapshot = snaps[l][i].get();
                }
            }
            ProduceFrame(f, rowToRender, *parInfos[i], parBuffers[i].get(), -1, supportsModelBlending);
            if (hasSnapshot) {
                for (int l = 0; l < numLayers; ++l) {
                    parBuffers[i]->BufferForLayer(l, -1).pendingSnapshot = nullptr;
                }
            }
            OutputFrame(f, rowToRender, *parInfos[i], parBuffers[i].get(), -1);
        }, 1, &PAR_FRAME_POOL, this->name + " - Frames");
    }

    // True if ANY layer's produce() might read dependent/upstream data mid-loop,
    // so the row must keep today's synchronous gate-before-everything ordering
    // (ARC phase A excludes it from the produce/output split).  Three categories
    // are not row-local: canvas-mix layers (CalcOutput preload), canvas-"Blend"
    // selections (seqData preload), and Per-Model buffer styles (dependent
    // per-model merge).  Detected once per job.  The error is asymmetric: a row
    // wrongly kept synchronous only forgoes the look-ahead (a perf miss), while
    // a row wrongly split blends against not-yet-ready seqData (silent wrong
    // output) - so bias hard toward exclusion and scan every effect on the main
    // model and its submodels.  A mid-render edit that introduces one of these
    // bumps the row change count, and the per-frame bail re-renders under a
    // fresh job that re-detects - so a job's flag is valid for its whole life.
    bool RowMustGateBeforeProduce() const {
        if (ctorHasPerModelBuffers) {
            return true;
        }
        auto layerNotRowLocal = [](EffectLayer* elayer) -> bool {
            if (elayer == nullptr) {
                return false;
            }
            std::unique_lock<std::recursive_mutex> elock(elayer->GetLock());
            for (int e = 0; e < elayer->GetEffectCount(); ++e) {
                Effect* eff = elayer->GetEffect(e);
                // Canvas-mix layers run CalcOutput to preload from lower layers
                // and, with model-blending / a "Blend" selection, from seqData -
                // i.e. upstream.  A Per-Model buffer style merges dependent
                // per-model pixels.  Either makes produce() non-row-local, so
                // the row keeps the synchronous gate-before-produce path.
                if (eff->GetSetting("T_CHECKBOX_Canvas") == "1"
                    || EndsWith(eff->GetSetting("T_LayersSelected"), "Blend")
                    || StartsWith(eff->GetSetting("B_CHOICE_BufferStyle"), "Per Model")) {
                    return true;
                }
            }
            return false;
        };
        for (int l = 0; l < numLayers; ++l) {
            if (layerNotRowLocal(rowToRender->GetEffectLayer(l))) {
                return true;
            }
        }
        for (const auto& a : subModelInfos) {
            Element* se = a->element;
            if (se == nullptr) {
                continue;
            }
            for (int l = 0; l < se->GetEffectLayerCount(); ++l) {
                if (layerNotRowLocal(se->GetEffectLayer(l))) {
                    return true;
                }
            }
        }
        return false;
    }

    void InitializeRenderStates() {
        mainModelInfo = EffectLayerInfo(numLayers);
        nodeEffects.clear();
        nodeSettingsMaps.clear();
        nodeEffectStates.clear();
        nodeEffectIdxs.clear();
        gateEffectIdxs.assign(numLayers, 0);
        rowMustGateBeforeProduce = RowMustGateBeforeProduce();
        producedFrame = -1;

        //for (int layer = 0; layer < numLayers; ++layer) {
        for (int layer = numLayers - 1; layer >= 0; --layer) {
            SetGenericStatus("Finding starting effect for {}, startFrame {}, and layer {} ", (int)startFrame, layer, false, true);
            EffectLayer *elayer = rowToRender->GetEffectLayer(layer);
            if (elayer == nullptr) {
                // layer removed between job creation and first slice
                mainModelInfo.currentEffects[layer] = nullptr;
                initialize(layer, startFrame, nullptr, mainModelInfo.settingsMaps[layer], mainBuffer);
                mainModelInfo.effectStates[layer] = true;
                continue;
            }
            std::unique_lock<std::recursive_mutex> elock(elayer->GetLock());
            mainModelInfo.currentEffects[layer] = findEffectForFrame(elayer, startFrame, mainModelInfo.currentEffectIdxs[layer]);
            SetGenericStatus("Initializing starting effect for {}, startFrame {}, and layer {} ", (int)startFrame, layer, false, true);
            initialize(layer, startFrame, mainModelInfo.currentEffects[layer], mainModelInfo.settingsMaps[layer], mainBuffer);
            mainModelInfo.effectStates[layer] = true;
        }

        // Frame-parallel eligibility (XL_PARALLEL_FRAMES).  Restricted to the
        // clean case the prototype handles: a group rendered through its main
        // buffer only (no submodels/nodes/per-model buffers), row-local produce
        // (no canvas-mix / canvas-Blend), and no model blending.  Per-frame Pure
        // coverage is checked separately (FrameIsParallelSafe).
        // Model blending is compatible: it blends over seqData[frame] in
        // output(), which runs after the gate and in frame order, so it reads a
        // fully-populated row exactly as the serial path does.  produce() stays
        // row-local. Canvas-mix / canvas-Blend / Per-Model (rowMustGateBeforeProduce)
        // are the ones that read dependent data mid-produce and stay excluded.
        bool isGroup = mainBuffer != nullptr && mainBuffer->GetModel() != nullptr
            && mainBuffer->GetModel()->GetDisplayAs() == DisplayAsType::ModelGroup;
        // Item 03: large plain-MODEL rows qualify under the same row-local
        // requirements - the window machinery is model-agnostic; groups were
        // just the first population.  The pixel floor keeps hundreds of small
        // props from paying the clone-pool memory + window overhead for
        // sub-millisecond frames (tunable: XL_PARALLEL_MODEL_MIN, pixels;
        // 0 disables model rows entirely).
        bool isBigModel = false;
        if (!isGroup && mainBuffer != nullptr && mainBuffer->GetModel() != nullptr) {
            static const long parModelMinPixels = []() {
                const char* e = getenv("XL_PARALLEL_MODEL_MIN");
                return e ? strtol(e, nullptr, 10) : 2048;
            }();
            if (parModelMinPixels > 0) {
                const RenderBuffer& rb0 = mainBuffer->BufferForLayer(0, -1);
                isBigModel = (long)rb0.BufferWi * (long)rb0.BufferHt >= parModelMinPixels;
            }
        }
        bool structurallyEligible = (isGroup || isBigModel)
            && !rowMustGateBeforeProduce
            && subModelInfos.empty() && nodeBuffers.empty()
            && !ctorHasPerModelBuffers;
        parEligible = xldbgParallelFrames && structurallyEligible;

        if (xldbgParBlockers) {
            // Rows held back ONLY by their submodel/strand effects - the
            // item-03 step-3 population (either kind of row).
            bool submodelOnly = !rowMustGateBeforeProduce && !ctorHasPerModelBuffers
                && (!subModelInfos.empty() || !nodeBuffers.empty());
            if (!isGroup) {
                {
                    ParBlockerStats& g = parBlockers();
                    std::lock_guard<std::mutex> lg(g.mtx);
                    ++g.d.rowsSingleModel;
                }
                // Item-03 population: model rows that would qualify except for
                // the groups-only rule.  Tally their coverage separately.
                if (!rowMustGateBeforeProduce && subModelInfos.empty()
                    && nodeBuffers.empty() && !ctorHasPerModelBuffers) {
                    AnalyzeFrameBlockers(BlockerTally::Model);
                } else if (submodelOnly) {
                    AnalyzeFrameBlockers(BlockerTally::SubmodelRow);
                }
            } else if (structurallyEligible) {
                AnalyzeFrameBlockers();
            } else {
                if (submodelOnly) {
                    AnalyzeFrameBlockers(BlockerTally::SubmodelRow);
                }
                std::string reason = rowMustGateBeforeProduce ? "canvas-mix / per-model gate"
                    : !subModelInfos.empty()                  ? "has submodels"
                    : !nodeBuffers.empty()                    ? "has per-node buffers"
                    : ctorHasPerModelBuffers                  ? "has per-model buffers"
                                                              : "other";
                ParBlockerStats& g = parBlockers();
                std::lock_guard<std::mutex> lg(g.mtx);
                ++g.d.rowsIneligible[reason];
            }
        }
    }

    // Row-local: source-model name if the main model has a Duplicate effect at
    // `frame` that includes submodels; else empty.
    std::string InheritedDuplicateSourceModel(int frame) {
        std::string inheritedDuplicateSourceModel;
        for (int lyr = 0; lyr < rowToRender->GetEffectLayerCount() && inheritedDuplicateSourceModel.empty(); ++lyr) {
            EffectLayer* elyr = rowToRender->GetEffectLayer(lyr);
            std::unique_lock<std::recursive_mutex> elyrLock(elyr->GetLock());
            int discard = 0;
            Effect* eff = findEffectForFrame(elyr, frame, discard);
            if (eff != nullptr && eff->GetEffectIndex() == EffectManager::eff_DUPLICATE &&
                eff->GetSetting("E_CHECKBOX_Duplicate_Include_Submodels") == "1") {
                inheritedDuplicateSourceModel = eff->GetSetting("E_CHOICE_Duplicate_Model");
            }
        }
        return inheritedDuplicateSourceModel;
    }

    // Row-local half of one node buffer: effect render + blur/zoom + transitions
    // into the node buffer.  Records in nodeShouldOutput whether OutputNode
    // should blend it into seqData.
    void ProduceNode(int frame, const SNPair& node, PixelBufferClass* buffer, bool cleared) {
        nodeShouldOutput[node] = false;
        if (buffer == nullptr) {
            spdlog::critical("RenderJob::Process PixelBufferPointer is null ... this is going to crash.");
        }
        int strand = node.strand;
        int inode = node.node;
        StrandElement *slayer = rowToRender->GetStrand(strand);
        if (slayer == nullptr) {
            //deleted strand
            return;
        }
        EffectLayer *nlayer = slayer->GetNodeLayer(inode, false);
        if (nlayer == nullptr) {
            //deleted node
            return;
        }
        std::unique_lock<std::recursive_mutex> nlayerLock(nlayer->GetLock());
        Effect *el = findEffectForFrame(nlayer, frame, nodeEffectIdxs[node]);
        if (el != nodeEffects[node] || frame == startFrame) {
            nodeEffects[node] = el;
            SetInializingStatus(frame, -1, -1, strand, inode);
            initialize(0, frame, el, nodeSettingsMaps[node], buffer);
            nodeEffectStates[node] = true;
        }
        bool persist=buffer->IsPersistent(0);
        if (!persist || nodeEffects[node] == nullptr || nodeEffects[node]->GetEffectIndex() == -1) {
            buffer->Clear(0);
        }

        SetRenderingStatus(frame, &nodeSettingsMaps[node], -1, -1, strand, inode, cleared);
        if (_engine->RenderEffectFromMap(false, el, 0, frame, nodeSettingsMaps[node], *buffer, nodeEffectStates[node])) {
            SetCalOutputStatus(frame, -1, strand, inode);
            { StageTimer st(profRender ? &profile.blurZoomNs : nullptr);
              buffer->HandleLayerBlurZoom(frame, 0); }
            { StageTimer st(profRender ? &profile.transitionNs : nullptr);
              buffer->HandleLayerTransitions(frame, 0); }
            nodeShouldOutput[node] = true;
        }
    }

    // seqData-touching half of one node buffer: blend the produced node buffer
    // into seqData.  CanOutputFrame re-verifies upstream (the gate guarantees it
    // for node rows, which always need the upstream frame).
    void OutputNode(int frame, const SNPair& node, PixelBufferClass* buffer) {
        auto it = nodeShouldOutput.find(node);
        if (it == nodeShouldOutput.end() || !it->second) {
            return;
        }
        if (buffer == nullptr || !CanOutputFrame(frame)) {
            return;
        }
        //copy to output
        std::vector<bool> valid(2, true);
        { StageTimer st(profRender ? &profile.setColorsNs : nullptr);
          buffer->SetColors(1, &((*seqData)[frame][0]), seqData->NumChannels()); }
        { StageTimer st(profRender ? &profile.blendNs : nullptr);
          buffer->CalcOutput(frame, valid); }
        { StageTimer st(profRender ? &profile.getColorsNs : nullptr);
          buffer->GetColors(&((*seqData)[frame][0]), rangeRestriction, seqData->NumChannels()); }
    }

    // Split-path produce (ARC phase A): all row-local work for the frame - main
    // model, submodels, node buffers - writing only the job's own buffers, never
    // seqData.  Runs before the upstream gate.
    void ProduceFrameAll(int frame) {
        ProduceFrame(frame, rowToRender, mainModelInfo, mainBuffer, -1, supportsModelBlending);
        const bool cleared = mainModelInfo.produceEffectsToUpdate;
        if (!subModelInfos.empty()) {
            const std::string inh = InheritedDuplicateSourceModel(frame);
            for (const auto& a : subModelInfos) {
                if (abort) {
                    rowToRender->SetDirtyRange(frame * seqData->FrameTime(), endFrame * seqData->FrameTime());
                    break;
                }
                ProduceFrame(frame, a->element, *a, a->buffer.get(), a->strand, supportsModelBlending ? true : cleared, inh);
            }
        }
        if (!nodeBuffers.empty()) {
            for (const auto& it : nodeBuffers) {
                if (abort) {
                    rowToRender->SetDirtyRange(frame * seqData->FrameTime(), endFrame * seqData->FrameTime());
                    break;
                }
                ProduceNode(frame, it.first, it.second.get(), cleared);
            }
        }
    }

    // Split-path output (ARC phase A): blend the produced buffers into seqData in
    // the old code's order - main model, then submodels, then node buffers - so
    // overlapping-channel last-writer-wins is preserved.  Runs after the gate.
    void OutputFrameAll(int frame) {
        OutputFrame(frame, rowToRender, mainModelInfo, mainBuffer, -1);
        for (const auto& a : subModelInfos) {
            if (abort) {
                rowToRender->SetDirtyRange(frame * seqData->FrameTime(), endFrame * seqData->FrameTime());
                break;
            }
            OutputFrame(frame, a->element, *a, a->buffer.get(), a->strand);
        }
        for (const auto& it : nodeBuffers) {
            if (abort) {
                rowToRender->SetDirtyRange(frame * seqData->FrameTime(), endFrame * seqData->FrameTime());
                break;
            }
            OutputNode(frame, it.first, it.second.get());
        }
    }

    // Canvas-"Blend" fallback: produce() then output() per unit, interleaved, so
    // a later unit's mid-produce seqData read sees this frame's earlier units
    // already blended in - byte-identical to the pre-split ProcessFrame ordering.
    void ProduceAndOutputFrameAll(int frame) {
        ProduceFrame(frame, rowToRender, mainModelInfo, mainBuffer, -1, supportsModelBlending);
        OutputFrame(frame, rowToRender, mainModelInfo, mainBuffer, -1);
        const bool cleared = mainModelInfo.produceEffectsToUpdate;
        if (!subModelInfos.empty()) {
            const std::string inh = InheritedDuplicateSourceModel(frame);
            for (const auto& a : subModelInfos) {
                if (abort) {
                    rowToRender->SetDirtyRange(frame * seqData->FrameTime(), endFrame * seqData->FrameTime());
                    break;
                }
                ProduceFrame(frame, a->element, *a, a->buffer.get(), a->strand, supportsModelBlending ? true : cleared, inh);
                OutputFrame(frame, a->element, *a, a->buffer.get(), a->strand);
            }
        }
        if (!nodeBuffers.empty()) {
            for (const auto& it : nodeBuffers) {
                if (abort) {
                    rowToRender->SetDirtyRange(frame * seqData->FrameTime(), endFrame * seqData->FrameTime());
                    break;
                }
                ProduceNode(frame, it.first, it.second.get(), cleared);
                OutputNode(frame, it.first, it.second.get());
            }
        }
    }

    // Renders one full frame: main model, submodels, then per-node buffers,
    // and notifies downstream renderers.  Stop = the frame loop must end
    // (aborted, or a newer render request for this row is waiting); Suspend =
    // upstream hasn't produced this frame yet and the job has registered to be
    // requeued when it does - the caller must return the thread to the pool.
    //
    // ARC phase A: the frame's work is split into produce() (row-local: effect
    // render + blur + transitions into the job's own buffers) and output() (the
    // tail that blends into seqData).  produce() needs nothing from upstream, so
    // it runs BEFORE the gate and output() after - one frame of look-ahead per
    // row.  A row whose produce() might read dependent data (canvas mix,
    // canvas-"Blend", or Per-Model buffers) keeps the old synchronous
    // gate-before-everything path (rowMustGateBeforeProduce).
    FrameResult RenderFrame(int frame) {
        AutoReleasePool pool;
        currentFrame = frame;
        SetGenericStatus("{}: Starting frame {} ", frame, true, true);

        // The change-count check applies to every job, downstream or not: the
        // per-slice render lock lets structural edits (insert/remove layer,
        // effect changes) land while this job is suspended, and rendering on
        // with the pre-edit layer state would index stale per-layer vectors.
        // Bailing sends END downstream (FinishRender) and the dirty range
        // re-renders the whole overlap chain.  On the split path this also
        // fires on resume, after produce() but before output(), so a frame whose
        // produced layers are stale (edited during the suspend) is dropped and
        // its dirty range re-renders it.
        if (abort ||
                origChangeCount != rowToRender->getChangeCount() ||
                (!HasNext() && rowToRender->HasParkedRenderJobs())) {
            //we're bailing out but make sure this range is reconsidered
            rowToRender->SetDirtyRange(frame * seqData->FrameTime(), endFrame * seqData->FrameTime());
            return FrameResult::Stop;
        }

        if (rowMustGateBeforeProduce) {
            // produce() might read dependent data mid-loop for this row, so the
            // whole frame must gate before any work - today's synchronous
            // behaviour (byte-identical to the pre-split code).
            if (frame > (int)GetPreviousFrameDone() && frame >= gateSkipUntilFrame && NeedsUpstreamFrame(frame)) {
                SetWaitingStatus(frame);
                if (trySuspendUntil(frame)) {
                    return FrameResult::Suspend;
                }
                SetGenericStatus("{}: Processing frame {} ", frame, true, true);
            }
            if (profRender) {
                ++profile.frames;
            }
            ProduceAndOutputFrameAll(frame);
            //mainBuffer->ApplyDimmingCurves(&((*seqData)[frame][0]));
            if (HasNext()) {
                SetGenericStatus("{}: Notifying next renderer of frame {} done", frame, true);
                FrameDone(frame);
            }
            return FrameResult::Continue;
        }

        // Split path: produce() is row-local, so it runs BEFORE the upstream
        // gate - one frame of look-ahead per row.  produce() runs exactly once
        // per frame (producedFrame guard) so its cross-frame effect state
        // advances in the same order as the old single pass; a resume after the
        // suspend skips produce() and goes straight to the gate + output().
        if (producedFrame != frame) {
            if (profRender) {
                ++profile.frames;
            }
            ProduceFrameAll(frame);
            producedFrame = frame;
        }

        // Gate the seqData-touching output() on upstream.  OutputFrame /
        // OutputNode re-verify previousFrameDone at their seqData touch points
        // (CanOutputFrame), backstopping any gate-coverage gap.  The !abort
        // guard prevents re-suspending after an abort landed during produce()
        // (nothing would nudge us again); output() then no-ops via CanOutputFrame
        // and the top-of-frame bail Stops on the next entry.
        if (!abort && frame > (int)GetPreviousFrameDone() && frame >= gateSkipUntilFrame && NeedsUpstreamFrame(frame)) {
            SetWaitingStatus(frame);
            if (trySuspendUntil(frame)) {
                return FrameResult::Suspend;
            }
            SetGenericStatus("{}: Processing frame {} ", frame, true, true);
        }

        OutputFrameAll(frame);
        //mainBuffer->ApplyDimmingCurves(&((*seqData)[frame][0]));
        if (HasNext()) {
            SetGenericStatus("{}: Notifying next renderer of frame {} done", frame, true);
            FrameDone(frame);
        }
        return FrameResult::Continue;
    }

    // May suspend awaiting the upstream END_OF_RENDER_FRAME; re-entered on
    // requeue.  Ends at CompleteJob() on every path.
    void FinishRender() {
        if (HasNext()) {
            //make sure the previous has told us we're at the end.  If we complete before it has,
            //the previous may still try to deliver frames to us.
            SetGenericStatus("{}: Waiting on previous renderer for final frame", 0, true);
            if (trySuspendUntil(END_OF_RENDER_FRAME)) {
                return;
            }

            //let the next know we're done
            SetGenericStatus("{}: Notifying next renderer of final frame", 0, true);
            FrameDone(END_OF_RENDER_FRAME);
            endDelivered = true;
            _engine->OnRenderJobComplete(rowToRender->GetModelName());
            SetGenericStatus("{}: All done - Completed frame {} ", endFrame, true, false);
        } else {
            _engine->OnAllRenderJobsComplete();
        }
        rowToRender->CleanupAfterRender();
        currentFrame = END_OF_RENDER_FRAME;
        //printf("Done rendering %lx (next %lx)\n", (unsigned long)this, (unsigned long)next);
        m_logger->debug("Rendering thread exiting.");
        CompleteJob();
    }

    // Terminal transition.  Hands the row to the next parked job (possibly
    // from a newer render batch), then signals batch completion.  Done is
    // assigned only after the region that can throw (Requeue allocates), so
    // Process()'s catch-all retries completion instead of stranding the
    // batch; ReleaseRenderOwnership and the detach are idempotent across a
    // retry.  NotifyJobFinished must be the very last touch of any state:
    // the thread that drops jobsRemaining to zero completes the batch and
    // the main thread may delete this job any time after.
    void CompleteJob() {
        // Final flush of the slice profile: NotifyJobFinished may dump every
        // job's profile (this one included) and nothing may write it after.
        EndSliceProfile();
        RenderEngine* engine = _engine;
        RenderProgressInfo* rpi = _rpi;
        void* next = rowToRender->ReleaseRenderOwnership(this);
        if (next != nullptr) {
            static_cast<RenderJob*>(next)->Requeue();
        }
        if (attachedToRow) {
            rowToRender->DetachRenderJob();
            attachedToRow = false;
        }
        schedPhase = SchedPhase::Done;
        engine->NotifyJobFinished(rpi);
    }

    // One scheduling slice: runs from wherever the job left off until it
    // completes or suspends.  The pool may call this many times per job.
    virtual void Process() override {
        try {
            ProcessSlice();
        } catch (...) {
            // Safety net for a throw outside the frame loop's own handlers.
            // Complete the job so the batch (and any downstream renderer
            // waiting on our END_OF_RENDER_FRAME) can still finish.
            assert(false); // so when we debug we catch them
            m_logger->error("Caught an exception on rendering thread outside the frame loop.");
            spdlog::error("Caught an exception on rendering thread outside the frame loop.");
            if (schedPhase != SchedPhase::Done) {
                // endDelivered guards against a throw AFTER FinishRender's
                // FrameDone(END) - a second END would over-count an
                // aggregator's fan-in slot and release downstream early.
                if (HasNext() && !endDelivered) {
                    FrameDone(END_OF_RENDER_FRAME);
                    endDelivered = true;
                }
                currentFrame = END_OF_RENDER_FRAME;
                CompleteJob();
            }
        }
    }

    void ProcessSlice() {
        // Resuming from a suspension: the gap since trySuspendUntil is this
        // job's idle-on-upstream time.  Always accounted for the batch summary;
        // per-row only when profiling.
        if (suspendTimingPending) {
            uint64_t d = xlProfNs(suspendStartTime, std::chrono::steady_clock::now());
            if (_rpi) {
                _rpi->suspendedNs += (long long)d;
            }
            if (profRender) {
                profile.suspendedNs += d;
            }
            suspendTimingPending = false;
        }
        BeginSliceProfile();
        if (profRender) {
            ++profile.slices;
        }

        // Order matters: parked must clear before inPool does.  The watchdog's
        // parked-rescue only requeues after winning the inPool CAS, so as long
        // as inPool is still true while parked can be stale, a rescue racing
        // this entry fails the CAS and can never double-run a live slice.
        {
            std::unique_lock<std::mutex> lock(nextLock);
            parked = false;
        }
        inPool = false;

        if (schedPhase == SchedPhase::Setup) {
            auto logger_jobpool = spdlog::get("job");
            // Log the thread ID as a hash value
            size_t tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
            logger_jobpool->debug("Render job thread id {0:x} or {0:d}", tid);

            SetGenericStatus("Initializing rendering thread for {}", 0);

            if (abort) {
                // Aborted before rendering started (possibly pulled out of the
                // row's parked queue by AbortRender).  Mark the range for
                // re-render and go straight to the END handshake so downstream
                // renderers converge; never take row ownership.
                rowToRender->SetDirtyRange(startFrame * seqData->FrameTime(), endFrame * seqData->FrameTime());
                currentFrame = END_OF_RENDER_FRAME;
                schedPhase = SchedPhase::Finish;
                FinishRender();
                return;
            }

            // All park bookkeeping happens BEFORE TryTakeRenderOwnership: the
            // instant it publishes us in the row's queue, the owner (or an
            // abort/rescue) can pop, requeue, run, and even complete+delete
            // us - nothing may touch `this` after a false return.
            EndSliceProfile();
            auto parkLogger = m_logger;
            {
                std::unique_lock<std::mutex> lock(nextLock);
                parked = true;
            }
            if (_rpi) {
                ++_rpi->parkCount;
            }
            if (!rowToRender->TryTakeRenderOwnership(this)) {
                // an earlier job is rendering this row; we're parked and will be
                // rescheduled when it completes - the thread goes back to the pool
                parkLogger->debug("Render job parked behind active render of row.");
                return;
            }
            {
                std::unique_lock<std::mutex> lock(nextLock);
                parked = false;
            }
            if (_rpi) {
                --_rpi->parkCount;
            }
            BeginSliceProfile();
            if (rowToRender->HasParkedRenderJobs() && !HasNext()) {
                // newer jobs for this model are parked, bail fast and let them handle this
                m_logger->debug("Rendering thread exiting early.");
                currentFrame = END_OF_RENDER_FRAME; // this is needed otherwise the job does not look done
                CompleteJob();
                return;
            }
            SetGenericStatus("Got render ownership of row for {}", 0);

            {
                std::unique_lock<std::recursive_timed_mutex> lock(rowToRender->GetRenderLock());
                ComputeRenderRange();
            }
            resumeFrame = startFrame;
            if (xldbgParallelWindows) {
                AnalyzeParallelWindows();
            }
            schedPhase = SchedPhase::Frames;
        }

        if (schedPhase == SchedPhase::Frames) {
            // The render lock is held per-slice (not across suspensions), so
            // structural layer edits can interleave between slices; the change
            // count / dirty range machinery triggers the re-render.
            std::unique_lock<std::recursive_timed_mutex> lock(rowToRender->GetRenderLock());
            try {
                if (!statesInitialized) {
                    InitializeRenderStates();
                    statesInitialized = true;
                }
                bool stopped = false;
                // Resume a gated parallel window that suspended awaiting upstream.
                if (parWindowPending) {
                    parWindowPending = false;
                    int a = parWindowStart, e = parWindowEnd;
                    if (e > (int)GetPreviousFrameDone() && e >= gateSkipUntilFrame && NeedsUpstreamFrame(e)) {
                        if (trySuspendUntil(e)) {
                            parWindowPending = true;
                            return;
                        }
                    }
                    if (abort || origChangeCount != rowToRender->getChangeCount() ||
                            (!HasNext() && rowToRender->HasParkedRenderJobs())) {
                        rowToRender->SetDirtyRange(a * seqData->FrameTime(), endFrame * seqData->FrameTime());
                        stopped = true;
                    } else {
                        RenderParallelWindow(a, e, parWindowHasSnapshot);
                        currentFrame = e; // advance the UI progress per window (not just per serial frame)
                        if (HasNext()) {
                            for (int f = a; f <= e; ++f) {
                                FrameDone(f);
                            }
                        }
                        resumeFrame = e + 1;
                    }
                }
                while (!stopped && resumeFrame <= endFrame) {
                    // Frame-parallel fast path: render a contiguous run of frames
                    // whose every layer is Pure or Snapshottable in parallel clones.
                    // Snapshottable layers get a serial capture pre-pass first (see
                    // RenderParallelWindow).  The window gates once on upstream and
                    // emits FrameDone in order.  Wins when the per-frame work is
                    // output/scatter-bound (serial per frame); effects whose internal
                    // parallel_for already saturates cores see no gain.
                    if (parEligible && !abort &&
                            origChangeCount == rowToRender->getChangeCount()) {
                        bool windowHasSnapshot = false;
                        if (FrameIsParallelSafe(resumeFrame, windowHasSnapshot)) {
                            int a = resumeFrame;
                            int cap = std::min((int)endFrame, (int)resumeFrame + PAR_FRAME_MAX_CHUNK - 1);
                            int e = a;
                            while (e + 1 <= cap && FrameIsParallelSafe(e + 1, windowHasSnapshot)) {
                                ++e;
                            }
                            if (e > a) {
                                // Gate the whole window on upstream frame e; monotonic,
                                // so done(e) implies every frame in [a,e] is available.
                                if (e > (int)GetPreviousFrameDone() && e >= gateSkipUntilFrame && NeedsUpstreamFrame(e)) {
                                    SetWaitingStatus(e);
                                    if (trySuspendUntil(e)) {
                                        parWindowStart = a;
                                        parWindowEnd = e;
                                        parWindowHasSnapshot = windowHasSnapshot;
                                        parWindowPending = true;
                                        return;
                                    }
                                }
                                RenderParallelWindow(a, e, windowHasSnapshot);
                                currentFrame = e; // advance the UI progress per window (not just per serial frame)
                                if (HasNext()) {
                                    for (int f = a; f <= e; ++f) {
                                        FrameDone(f);
                                    }
                                }
                                resumeFrame = e + 1;
                                continue;
                            }
                        }
                    }
                    FrameResult r = RenderFrame(resumeFrame);
                    if (r == FrameResult::Suspend) {
                        return;
                    }
                    if (r == FrameResult::Stop) {
                        stopped = true;
                    } else {
                        ++resumeFrame;
                    }
                }
                SetGenericStatus("{}: All done - Completed frame {} ", endFrame, true, false);
            } catch ( std::exception &ex) {
                assert(false); // so when we debug we catch them
                m_logger->error("Caught an exception on rendering thread: " + std::string(ex.what()));
                spdlog::error("Caught an exception on rendering thread: {}", ex.what());
            } catch ( ... ) {
                assert(false); // so when we debug we catch them
                m_logger->error("Caught an unknown exception on rendering thread.");
                spdlog::error("Caught an unknown exception on rendering thread.");
            }
            schedPhase = SchedPhase::Finish;
        }

        if (schedPhase == SchedPhase::Finish) {
            FinishRender();
        }
    }

    void AbortRender() override {
        abort = true;
        // Suspended and parked jobs hold no thread; wake them so they can run
        // their bail path (dirty range, END handshake, completion) promptly.
        NudgeIfSuspended();
    }

    ModelElement* GetModelElement() const { return rowToRender; }

    void SetRenderProgressInfo(RenderProgressInfo* rpi) { _rpi = rpi; }

private:

    void initialize(int layer, int frame, Effect *el, SettingsMap &settingsMap, PixelBufferClass *buffer) {
        bool layerEnabled = true;
        if (el == nullptr || el->GetEffectIndex() == -1) {
            settingsMap.clear();
            layerEnabled = false;
        } else {
            auto e = el->GetParentEffectLayer()->GetParentElement();
            if (e != nullptr) {
                layerEnabled = !e->IsRenderDisabled();
            }
            loadSettingsMap(el->GetEffectName(),
                            el,
                            settingsMap);
        }
        buffer->SetLayerSettings(layer, settingsMap, layerEnabled);
        if (el != nullptr) {
            xlColorVector newcolors;
            xlColorCurveVector newcc;
            el->CopyPalette(newcolors, newcc);
            buffer->SetPalette(layer, newcolors, newcc);
            buffer->SetTimes(layer, el->GetStartTimeMS(), el->GetEndTimeMS());
        }
    }

    Effect* findEffectForFrame(const std::string& model, const int layer, int frame)
    {
        Effect* res = nullptr;

        Element* e = rowToRender->GetSequenceElements()->GetElement(model);

        if (e != nullptr) {
            EffectLayer* el = e->GetEffectLayer(layer - 1);

            if (el != nullptr) {
                int discard = 0;
                res = findEffectForFrame(el, frame, discard);
            }
        }

        return res;
    }

    Effect *findEffectForFrame(EffectLayer* layer, int frame, int &lastIdx) {
        if (layer == nullptr) {
            return nullptr;
        }
        int time = frame * seqData->FrameTime();
        for (int e = lastIdx; e < layer->GetEffectCount(); ++e) {
            Effect *effect = layer->GetEffect(e);
            int st = effect->GetStartTimeMS();
            int et = effect->GetEndTimeMS();
            if (et > time && st <= time) {
                return effect;
            }
        }
        return nullptr;
    }

    Effect *findEffectForFrame(int layer, int frame, int &lastIdx) {
        return findEffectForFrame(rowToRender->GetEffectLayer(layer), frame, lastIdx);
    }
    Effect *findEffectForFrame(int layer, int frame, int submodel, int &lastIdx) {
        if (submodel == -1) {
            return findEffectForFrame(rowToRender->GetEffectLayer(layer), frame, lastIdx);
        }
        return findEffectForFrame(subModelInfos[submodel]->element->GetEffectLayer(layer), frame, lastIdx);
    }

    void loadSettingsMap(const std::string &effectName,
                         Effect *effect,
                         SettingsMap& settingsMap) {
        settingsMap.clear();
        effect->CopySettingsMap(settingsMap, true);
    }

    ModelElement *rowToRender;
    std::string name;
    PixelBufferClass *mainBuffer;
    int numLayers;
    std::atomic_int startFrame;
    std::atomic_int endFrame;
    RenderContext *_ctx;
    RenderEngine *_engine;
    SequenceData *seqData;
    SequenceElements *_seqElements;
    std::vector<bool> rangeRestriction;
    bool supportsModelBlending;

    //stuff for handling the status;
    std::string statusMsg;
    volatile int statusType;
    volatile int statusFrame;
    SettingsMap *statusMap;
    volatile int statusLayer;
    volatile int statusSubmodel = -1;
    volatile int statusStrand = -1;
    volatile int statusNode = -1;
    std::shared_ptr<spdlog::logger> m_logger{ nullptr };

    std::function<void(int, const std::string&)> progressCallback;
    std::atomic_int currentFrame;
    std::atomic_bool abort;
    RenderProgressInfo* _rpi = nullptr; // non-owning; set after construction by Render()

    // Frame-loop state lives on the object rather than the Process() stack so
    // the loop runs as resumable slices (see plans/render-scheduler.md).
    EffectLayerInfo mainModelInfo;
    std::map<SNPair, Effect*> nodeEffects;
    std::map<SNPair, SettingsMap> nodeSettingsMaps;
    std::map<SNPair, bool> nodeEffectStates;
    std::map<SNPair, int> nodeEffectIdxs;
    // ARC phase A: per-node produce() result - did output() have to blend it?
    std::map<SNPair, bool> nodeShouldOutput;
    int origChangeCount = 0;

    // Frame-parallel prototype state (XL_PARALLEL_FRAMES).  parEligible is
    // computed once at InitializeRenderStates.  The clone pool (parBuffers +
    // parInfos) is grown on demand and reused across chunks.  parProduced marks a
    // produced-but-not-fully-output chunk that must survive an upstream suspend;
    // parOutputCursor is where the ordered output resumes.
    bool parEligible = false;
    std::vector<std::unique_ptr<PixelBufferClass>> parBuffers;
    std::vector<std::unique_ptr<EffectLayerInfo>> parInfos;
    bool parWindowPending = false;   // a gated window is suspended awaiting upstream
    bool parWindowHasSnapshot = false;  // pending window has >=1 Snapshottable layer (needs the capture pre-pass)
    int parWindowStart = 0;
    int parWindowEnd = -1;

    // Scheduling state.  suspended/wantFrame/parked are guarded by nextLock;
    // inPool is its own atomic (see Requeue); the rest is only touched by the
    // single thread running the current slice.
    SchedPhase schedPhase = SchedPhase::Setup;
    bool suspended = false;
    bool parked = false;
    std::atomic<bool> inPool{true}; // jobs are born queued (Render() pushes them)
    bool attachedToRow = false;
    int wantFrame = 0;
    int resumeFrame = 0;
    bool statesInitialized = false;
    bool endDelivered = false;
    bool gateMissWarned = false;
    int gateSkipUntilFrame = 0;
    // ARC phase A produce/output split.  rowMustGateBeforeProduce (computed once
    // at InitializeRenderStates) forces the synchronous gate-before-produce path
    // for rows whose produce() might read dependent data mid-loop (canvas mix,
    // canvas-"Blend", or Per-Model buffers).  ctorHasPerModelBuffers is set in
    // the ctor when any group layer initialised Per-Model buffers (needs the
    // group default buffer style, only known there).  producedFrame is the frame
    // whose produce() has run but whose output() may still be pending across a
    // suspend on the split path.
    bool rowMustGateBeforeProduce = false;
    bool ctorHasPerModelBuffers = false;
    int producedFrame = -1;
    // Per-main-layer cursor into the (time-ordered) effect list for the
    // frame-entry gate; advances with the frame loop (see NeedsUpstreamFrame).
    std::vector<int> gateEffectIdxs;

    std::vector<EffectLayerInfo *> subModelInfos;

    std::map<SNPair, PixelBufferClassPtr> nodeBuffers;

    // XL_RENDER_PROFILE telemetry.  Written only by the thread running the
    // current slice; read after completion in NotifyJobFinished.  The suspend
    // timestamps are always maintained (cheap, once per suspension) so the
    // batch summary line can report total suspended time even when profiling
    // is off.
    RenderJobProfile profile;
    std::chrono::steady_clock::time_point suspendStartTime;
    bool suspendTimingPending = false;
    std::chrono::steady_clock::time_point sliceStartTime;
    bool sliceProfileArmed = false;
};


// RenderRange - moved to RenderUI.cpp (uses RenderCommandEvent wx type)

RenderEngine::RenderEngine(RenderContext& ctx, JobPool& pool, RenderCache& cache)
    : _ctx(ctx), _jobPool(pool), _renderCache(cache) {}

RenderEngine::~RenderEngine() {
    for (auto* rpi : _renderProgressInfo) {
        rpi->CleanupJobs();
        delete rpi;
    }
    _renderProgressInfo.clear();
}

// RenderProgressInfo is defined in RenderProgressInfo.h (included above).
// It was moved to a header so RenderUI.cpp can also access it.

// LogRenderStatus - moved to RenderUI.cpp (needs access to RenderProgressInfo)

static bool HasEffects(ModelElement *me) {
    if (me->HasEffects()) {
        return true;
    }

    for (int x = 0; x < me->GetSubModelAndStrandCount(); ++x) {
        if (me->GetSubModel(x)->HasEffects()) {
            return true;
        }
    }
    for (int x = 0; x < me->GetStrandCount(); ++x) {
        StrandElement *se = me->GetStrand(x);
        for (int n = 0; n < se->GetNodeLayerCount(); ++n) {
            if (se->GetNodeLayer(n)->GetEffectCount() > 0) {
                return true;
            }
        }
    }
    return false;
}

// OnProgressBarDoubleClick, OnRenderStatusTimerTrigger, UpdateRenderStatus,
// RenderDone - all moved to RenderUI.cpp (wx UI handlers / progress-bar updates).

class RenderTreeData {
public:
    RenderTreeData(Model *e): model(e) {

        
        if (e == nullptr) {
            spdlog::critical("Render tree has a null model ... this is not going to end well.");
        }

        ModelGroup *mg = dynamic_cast<ModelGroup*>(e);
        if (mg != nullptr) {
            // might need to recalculate the group nodes
            mg->CheckForChanges();
        }

        for (size_t node = 0; node < e->GetNodeCount(); ++node) {
            int32_t startCh = e->NodeStartChannel(node);
            int32_t endCh = e->NodeEndChannel(node);
            if (startCh < 0 || endCh < 0) continue;
            AddRange((unsigned int)startCh, (unsigned int)endCh);
        }
        sortRanges(ranges);
    }

    void AddRange(unsigned int start, unsigned int end) {
        if (!ranges.empty()) {
            if ((ranges.back().end + 1) == start) {
                ranges.back().end = end;
                return;
            }
        }
        ranges.push_back(NodeRange(start, end));
    }

    bool Overlaps(RenderTreeData &e) {
        for (const auto& it : ranges) {
            for (const auto& it2 : e.ranges) {
                if (it.Overlaps(it2)) {
                    return true;
                }
            }
        }
        return false;
    }

    static void sortRanges(std::list<NodeRange> &ranges) {
        ranges.sort();
        auto it = ranges.begin();
        auto it2 = ranges.begin();
        if (it2 != ranges.end()) {
            ++it2;
        }
        while (it2 != ranges.end()) {
            if ((it->end + 1) == it2->start) {
                //it2 is immediately at the end of it
                it->end = it2->end;
                it2 = ranges.erase(it2);
            } else if (it->end >= (it2->start - 1) && (it->end <= it2->end)) {
                // it2 overlaps the end of it
                it->end = it2->end;
                it2 = ranges.erase(it2);
            } else if (it2->start <= it->end && it->end >= it2->end) {
                //it2 fully contained in it
                it2 = ranges.erase(it2);
            } else {
                ++it;
                ++it2;
            }
        }
    }

    void Add(Model *el) {
        renderOrder.push_back(el);
    }
    std::list<Model *> renderOrder;
    std::list<NodeRange> ranges;
    Model *model;
};

void RenderEngine::RenderTree::Clear() {
    for (auto it : data) {
        delete it;
    }
    data.clear();
}

void RenderEngine::RenderTree::Add(Model *el) {
    RenderTreeData *elData = new RenderTreeData(el);
    for (const auto& it : data) {
        RenderTreeData *elData2 = it;
        if (elData2->Overlaps(*elData)) {
            elData->Add(elData2->model);
            elData2->Add(el);
        }
    }
    elData->Add(el);
    data.push_back(elData);
}

void RenderEngine::RenderTree::Print() {
    auto logger_render = spdlog::get("render");
    logger_render->debug("========== RENDER TREE");
    for (const auto& it : data) {
        //printf("%s:   (%d)\n", (*it)->model->GetName().c_str(), (int)(*it)->ranges.size());
        logger_render->debug("   {}:   ({})", it->model->GetName(), (int)it->ranges.size());
        for (const auto& it2 : it->renderOrder) {
            //printf("    %s     \n", it2->GetName().c_str());
            logger_render->debug("        {}", it2->GetName());
        }
    }
    logger_render->debug("========== END RENDER TREE");
}

std::list<Model*> RenderEngine::RenderTree::GetModels() const {
    std::list<Model*> models;
    for (const auto& it : data) {
        models.push_back(it->model);
    }
    return models;
}

void RenderEngine::BuildRenderTree(SequenceElements& elements, unsigned int modelsChangeCount) {
    // Include the model-manager generation so any model add/replace/delete/
    // clear forces the tree to rebuild — otherwise a cached raw Model* can
    // outlive the freed model and crash in PixelBufferClass::reset / GetColors
    // (crash sigs 7d28659359, 998b51b4b4, 62b47aa9b8).
    unsigned int curChangeCount = elements.GetMasterViewChangeCount() + modelsChangeCount + _ctx.GetModelGeneration();
    if (_renderTree.renderTreeChangeCount != curChangeCount) {
        _renderTree.Clear();
        const int numEls = elements.GetElementCount(MASTER_VIEW);
        if (numEls == 0) {
            //nothing to do....
            return;
        }
        for (size_t row = 0; row < (size_t)numEls; ++row) {
            Element *rowEl = elements.GetElement(row, MASTER_VIEW);
            if (rowEl != nullptr && rowEl->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                Model *model = _ctx.GetModel(rowEl->GetModelName());
                if (model != nullptr) {
                    _renderTree.Add(model);
                }
            }
        }
        _renderTree.Print();
        _renderTree.renderTreeChangeCount = curChangeCount;
    }
}

void RenderEngine::Render(SequenceElements& seqElements,
                          SequenceData& seqData,
                          const std::list<Model*> models,
                          const std::list<Model *> &restrictToModels,
                          int startFrame, int endFrame,
                          std::unique_ptr<IRenderProgressSink> sink, bool clear,
                          std::function<void(bool)>&& callback)
{
    _abortedRenderJobs = 0;

    auto logger_render = spdlog::get("render");
    if (startFrame < 0) {
        startFrame = 0;
    }
    if (endFrame >= (int)seqData.NumFrames()) {
        endFrame = seqData.NumFrames() - 1;
    }
    std::list<NodeRange> ranges;
    if (restrictToModels.empty()) {
        ranges.push_back(NodeRange(0, seqData.NumChannels()));
    } else {
        for (const auto& it : restrictToModels) {
            RenderTreeData data(it);
            ranges.insert(ranges.end(), data.ranges.begin(), data.ranges.end());
        }
        RenderTreeData::sortRanges(ranges);
    }
    int numRows = models.size();
    RenderJob **jobs = new RenderJob*[numRows];
    AggregatorRenderer **aggregators = new AggregatorRenderer*[numRows];
    std::vector<std::set<int>> channelMaps(seqData.NumChannels());

    size_t row = 0;
    for (auto it = models.begin(); it != models.end(); ++it, ++row) {
        jobs[row] = nullptr;
        aggregators[row] = new AggregatorRenderer(seqData.NumFrames());

        Element *rowEl = seqElements.GetElement((*it)->GetName());

        if (rowEl == nullptr) {
            //spdlog::critical("xLightsFrame::Render rowEl is nullptr ... this is going to crash looking for '{}'.", (const char *)(*it)->GetName().c_str());
        } else {
            if (rowEl->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                ModelElement *me = dynamic_cast<ModelElement *>(rowEl);

                if (me == nullptr) {
                    logger_render->critical("xLightsFrame::Render me is nullptr ... this is going to crash.");
                }

                bool hasEffects = HasEffects(me);
                bool isRestricted = std::find(restrictToModels.begin(), restrictToModels.end(), *it) != restrictToModels.end();
                if (hasEffects || (isRestricted && clear)) {
                    RenderJob *job = new RenderJob(me, seqData, &_ctx, this, &seqElements);

                    if (job == nullptr) {
                        logger_render->critical("xLightsFrame::Render job is nullptr ... this is going to crash.");
                    }

                    job->setRenderRange(startFrame, endFrame);
                    job->SetRangeRestriction(ranges);
                    // No progress sink == per-edit micro-batch (RenderEffectForModel);
                    // jump the JobPool queue ahead of a queued Render All so the
                    // grid/preview don't wait for its backlog to drain.
                    if (sink == nullptr) {
                        job->SetHighPriority(true);
                    }
                    if (seqElements.SupportsModelBlending()) {
                        job->SetModelBlending();
                    }
                    PixelBufferClass *buffer = job->getBuffer();
                    if (buffer == nullptr || buffer->GetNodeCount() == 0) {
                        delete job;
                        continue;
                    }

                    jobs[row] = job;
                    aggregators[row]->addNext(job);
                    if (xldbgEffSum) {
                        fprintf(stderr, "ROW %zu %s\n", row, (*it)->GetName().c_str());
                    }
                    size_t cn = buffer->GetChanCountPerNode();
                    for (size_t node = 0; node < buffer->GetNodeCount(); ++node) {
                        uint32_t start = buffer->NodeStartChannel(node);
                        for (size_t c = 0; c < cn; ++c) {
                            size_t cnum = start + c;
                            if (cnum < seqData.NumChannels()) {
                                for (const auto i : channelMaps[cnum]) {
                                    int idx = i;
                                    if ((size_t)idx != row) {
                                        if (jobs[idx]->addNext(aggregators[row])) {
                                            aggregators[row]->incNumAggregated();
                                            if (xldbgEffSum) {
                                                fprintf(stderr, "EDGE %d -> %zu\n", idx, row);
                                            }
                                        }
                                    }
                                }
                                channelMaps[cnum].insert(row);
                            }
                        }
                    }
                }
            }
        }
    }

    logger_render->debug("Aggregators created.");

    channelMaps.clear();
    if (clear) {
        for (int f = startFrame; f <= endFrame; f++) {
            for (const auto& it : ranges) {
                seqData[f].Zero(it.start, it.end - it.start + 1);
            }
        }
    }

    logger_render->debug("Data cleared.");

    // Count live jobs up front — rpi must exist and be linked to each RenderJob
    // BEFORE any job is pushed to the pool, otherwise a fast worker could call
    // NotifyJobFinished on a null rpi.
    unsigned int count = 0;
    for (row = 0; row < (size_t)numRows; ++row) {
        if (jobs[row]) ++count;
    }

    if (count == 0) {
        delete[] jobs;
        delete[] aggregators;
        callback(_abortedRenderJobs > 0);
        // sink auto-deleted by unique_ptr
        return;
    }

    // Copy RenderJob* into IRenderJobStatus* array for RenderProgressInfo.
    IRenderJobStatus** statusJobs = new IRenderJobStatus*[numRows];
    for (int i = 0; i < numRows; ++i) {
        statusJobs[i] = jobs[i]; // implicit upcast; nullptr rows allowed
    }

    RenderProgressInfo* pi = new RenderProgressInfo(std::move(callback));
    pi->numRows = numRows;
    pi->startFrame = startFrame;
    pi->endFrame = endFrame;
    pi->jobs = statusJobs;
    pi->progressSink = sink.release(); // RenderProgressInfo takes ownership
    pi->restriction = restrictToModels;
    pi->aggregators = aggregators;
    pi->jobsRemaining.store((int)count);
    pi->totalJobs = (int)count;

    // Link every live job to rpi so completion can signal.
    for (row = 0; row < (size_t)numRows; ++row) {
        if (jobs[row]) jobs[row]->SetRenderProgressInfo(pi);
    }

    _renderProgressInfo.push_back(pi);
    if (_onRenderStatusTimerStart) _onRenderStatusTimerStart();

    // First pass: push jobs that have no upstream dependencies so they can
    // start rendering while we finish setup on the rest.
    for (row = 0; row < (size_t)numRows; ++row) {
        if (jobs[row]) {
            if (aggregators[row]->getNumAggregated() == 0) {
                jobs[row]->setPreviousFrameDone(END_OF_RENDER_FRAME);
                _jobPool.PushJob(jobs[row]);
            }
            if (pi->progressSink) {
                pi->progressSink->SetupJobProgress(jobs[row]);
            }
        }
    }

    logger_render->debug("Job pool start size {}.", (int)_jobPool.size());

    // Second pass: push the dependent jobs.
    for (row = 0; row < (size_t)numRows; ++row) {
        if (jobs[row] && aggregators[row]->getNumAggregated() != 0) {
            _jobPool.PushJob(jobs[row]);
        }
    }
    logger_render->debug("Job pool new size {}.", (int)_jobPool.size());

    delete[] jobs;

    if (pi->progressSink) {
        pi->progressSink->OnRenderSetupComplete();
    }
}

static void addModelsUpTo(std::list<Model*> &models, const std::list<Model *> &toAdd, Model *upTo) {
    for (auto it = toAdd.begin(); it != toAdd.end(); ++it) {
        bool add = true;
        for (auto it2 = models.begin(); it2 != models.end() && add; ++it2) {
            if (*it2 == *it) {
                add = false;
            }
        }
        if (add) {
            models.push_back(*it);
        }
        if (upTo == *it) {
            return;
        }
    }
}

static void addModelsFrom(std::list<Model*> &models, const std::list<Model *> &toAdd, Model *from) {
    bool found = false;
    for (auto it = toAdd.begin(); it != toAdd.end(); ++it) {
        if (!found && from != *it) {
            continue;
        }
        found = true;
        bool add = true;
        for (auto it2 = models.begin(); it2 != models.end() && add; ++it2) {
            if (*it2 == *it) {
                add = false;
            }
        }
        if (add) {
            models.push_back(*it);
        }
    }
}

void RenderEngine::RenderDirtyModels(SequenceElements& _sequenceElements, SequenceData& _seqData,
                                     bool suspendRender, unsigned int modelsChangeCount) {

    if (suspendRender) return; // dont render if suspended

    BuildRenderTree(_sequenceElements, modelsChangeCount);
    if (_renderTree.data.empty()) {
        //nothing to do....
        return;
    }
    const int numRows = _sequenceElements.GetElementCount();
    if (numRows == 0) {
        return;
    }
    int startms = 9999999;
    int endms = -1;
    std::list<Model *> models;
    std::list<Model *> restricts;
    for (int x = 0; x < numRows; x++) {
        Element *el = _sequenceElements.GetElement(x);
        if (el->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
            int st, ed;
            el->GetDirtyRange(st, ed);
            if (st != -1) {
                startms = std::min(startms, st);
                endms = std::max(endms, ed);
                for (auto it = _renderTree.data.begin(); it != _renderTree.data.end(); ++it) {
                    if ((*it)->model->GetName() == el->GetModelName()) {
                        restricts.push_back((*it)->model);
                        addModelsUpTo(models, (*it)->renderOrder, (*it)->model);
                    }
                }
            }
        }
    }
    if (restricts.empty()) {
        return;
    }
    for (auto x = models.begin(); x != models.end(); ++x) {
        for (auto it = _renderTree.data.begin(); it != _renderTree.data.end(); ++it) {
            if ((*it)->model == *x) {
                addModelsFrom(models, (*it)->renderOrder, (*it)->model);
            }
        }
    }
    if (startms < 0) {
        startms = 0;
    }
    if (endms < 0) {
        endms = 0;
    }
    int startframe = startms /_seqData.FrameTime() - 1;
    if (startframe < 0) {
        startframe = 0;
    }
    int endframe = endms / _seqData.FrameTime() + 1;
    if (endframe >= (int)_seqData.NumFrames()) {
        endframe = _seqData.NumFrames() - 1;
    }
    if (endframe < startframe) {
        return;
    }
    Render(_sequenceElements, _seqData, models, restricts, startframe, endframe, nullptr, true, [] (bool) {});
}

void RenderEngine::SignalAbort() {
    for (auto rpi : _renderProgressInfo) {
        for (size_t row = 0; row < (size_t)rpi->numRows; ++row) {
            if (rpi->jobs[row]) {
                rpi->jobs[row]->AbortRender();
                ++_abortedRenderJobs;
            }
        }
    }
}

// RenderGridToSeqData - moved to RenderUI.cpp (creates WxRenderProgressSink)

static Effect* GetPersistentEffectOnModelStartingAtTime(SequenceElements& seqElements, const std::string& model, uint32_t startms) {
    Element* e = seqElements.GetElement(model);
    if (e == nullptr)
        return nullptr;
    for (size_t i = 0; i < e->GetEffectLayerCount(); ++i) {
        Effect* ef = e->GetEffectLayer(i)->GetEffectStartingAtTime(startms);
        if (ef != nullptr && ef->IsPersistent()) {
            return ef;
        }
    }
    return nullptr;
}


void RenderEngine::RenderEffectForModel(const std::string &model, int startms, int endms,
                                        SequenceElements& _sequenceElements, SequenceData& _seqData,
                                        bool suspendRender, unsigned int modelsChangeCount, bool clear) {

    if (suspendRender) return;

    BuildRenderTree(_sequenceElements, modelsChangeCount);

    spdlog::debug("Render tree built for model {} {}ms-{}ms. {} entries.",
        (const char *)model.c_str(),
        startms,
        endms,
        _renderTree.data.size());

    int startframe = startms / _seqData.FrameTime();

    // If there is an effect at the start time that has the persistent flag set then include the prior frame
    Effect* persistentEffectBefore = GetPersistentEffectOnModelStartingAtTime(_sequenceElements, model, startms);
    if (persistentEffectBefore != nullptr) {
        startframe -= 1;
    }

    if (startframe < 0) {
        startframe = 0;
    }
    int endframe = endms / _seqData.FrameTime();

    // If there is an effect at the end time that has the persistent flag set then include the next frame
    Effect* persistentEffectAfter = GetPersistentEffectOnModelStartingAtTime(_sequenceElements, model, endms);
    if (persistentEffectAfter != nullptr) {
        endframe = persistentEffectAfter->GetEndTimeMS() / _seqData.FrameTime();
    }

    if (endframe >= (int)_seqData.NumFrames()) {
        endframe = _seqData.NumFrames() - 1;
    }
    for (const auto& it : _renderTree.data) {
        if (it->model->GetName() == model) {

            for (const auto& it2 : _renderProgressInfo) {
                RenderProgressInfo *rpi = it2;
                if (std::find(rpi->restriction.begin(), rpi->restriction.end(), it->model) != rpi->restriction.end()) {
                    if (startframe > rpi->startFrame) {
                        startframe = rpi->startFrame;
                    }
                    if (endframe < rpi->endFrame) {
                        endframe = rpi->endFrame;
                    }
                    for (size_t row = 0; row < (size_t)rpi->numRows; ++row) {
                        if (rpi->jobs[row]) {
                            rpi->jobs[row]->AbortRender();
                        }
                    }
                }
            }
            std::list<Model *> m;
            m.push_back(it->model);

            spdlog::debug("Rendering {} models {} frames.", m.size(), endframe - startframe + 1);

            Render(_sequenceElements, _seqData, it->renderOrder, m, startframe, endframe, nullptr, true, [] (bool) {});
        }
    }
}

RenderEngine::ExportedModelData RenderEngine::ExportModelData(const std::string& modelName, SequenceData& sourceData) {
    ExportedModelData result;

    Model* model = _ctx.GetModel(modelName);
    if (model == nullptr)
        return result;

    PixelBufferClass buffer(&_ctx);
    buffer.InitBuffer(*model, 1, sourceData.FrameTime());

    auto exportData = std::make_unique<SequenceData>();
    exportData->init(model->GetActChanCount(), sourceData.NumFrames(), sourceData.FrameTime(), false);

    int firstChan = model->NodeStartChannel(0);
    for (size_t frame = 0; frame < sourceData.NumFrames(); ++frame) {
        for (size_t x = 0; x < buffer.GetNodeCount(); ++x) {
            int ostart = model->NodeStartChannel(x);
            int nstart = ostart - firstChan;
            buffer.SetNodeChannelValues(x, &sourceData[frame][ostart]);
            buffer.GetNodeChannelValues(x, &((*exportData)[frame][nstart]));
        }
    }

    result.data = std::move(exportData);
    result.chansPerNode = buffer.GetChanCountPerNode();
    return result;
}

// XL_VERIFY_STATELESS oracle (see xldbgVerifyStateless).  The frame has just
// been rendered normally into `rb`.  Re-render the same frame from an emptied
// infoCache + needToInit=true and compare pixel hashes: a genuinely
// frame-independent (Pure) effect reproduces its output exactly, so a mismatch
// proves the GetFrameParallelism()==Pure declaration is wrong (the effect reads
// state carried from a prior frame).  Same buffer and same GPU/CPU path both
// times -> no GPU-vs-CPU float confound.  Only the second render's pixels are
// left in `rb`; for a correctly-Pure effect they are identical to the first, so
// the sequence output is unchanged except on the very effects this flag exists
// to catch.  The original infoCache is restored so later frames stay correct.
static void VerifyStatelessRender(RenderableEffect* reff, Effect* effectObj,
                                  const SettingsMap& settings, RenderBuffer* rb,
                                  const std::string& modelName, int layer) {
    GPURenderUtils::waitForRenderCompletion(rb);
    const size_t nbytes = size_t(rb->GetPixelCount()) * sizeof(xlColor);
    uint64_t h1 = xldbgFNV(reinterpret_cast<const uint8_t*>(rb->GetPixels()), nbytes);

    std::map<int, EffectRenderCache*> savedCache;
    savedCache.swap(rb->infoCache);
    bool savedInit = rb->needToInit;

    rb->needToInit = true;
    rb->Clear();
    // Reseed the serial RNG as if this frame were being drawn fresh; otherwise the
    // second Render() continues the first pass's rand01()/randInt() stream and any
    // effect that draws from it falsely trips the check (the parallel path reseeds
    // every frame, so a matching first-time reseed is what "stateless" means here).
    rb->resetSerialRandomForVerify();
    reff->Render(effectObj, settings, *rb);
    GPURenderUtils::waitForRenderCompletion(rb);
    uint64_t h2 = xldbgFNV(reinterpret_cast<const uint8_t*>(rb->GetPixels()), nbytes);

    for (auto& it : rb->infoCache) {
        delete it.second;
    }
    rb->infoCache.clear();
    rb->infoCache.swap(savedCache);
    rb->needToInit = savedInit;

    uint64_t nChecks = xldbgVerifyChecks.fetch_add(1) + 1;
    if (h1 != h2) {
        uint64_t nBad = xldbgVerifyMismatches.fetch_add(1) + 1;
        fprintf(stderr, "XL_VERIFY_STATELESS MISMATCH #%llu: '%s' on model '%s' layer %d frame %d declared Pure but re-render from a clean cache changed output (%016llx vs %016llx) - it carries cross-frame state; the GetFrameParallelism() override is wrong.\n",
                (unsigned long long)nBad, reff->Name().c_str(), modelName.c_str(), layer, rb->curPeriod,
                (unsigned long long)h1, (unsigned long long)h2);
        auto l = spdlog::get("render");
        if (l) {
            l->warn("XL_VERIFY_STATELESS: '{}' on model '{}' layer {} frame {} declared Pure but re-render from a clean cache changed output (hash {:016x} vs {:016x}) - it carries cross-frame state and the GetFrameParallelism() override is wrong.",
                    reff->Name(), modelName, layer, rb->curPeriod, h1, h2);
        }
    }
    // Periodic progress so a clean headless run positively confirms the harness
    // ran (silence alone can't distinguish "no mismatches" from "never checked").
    if ((nChecks % 2000) == 0) {
        fprintf(stderr, "XL_VERIFY_STATELESS: %llu Pure frames checked, %llu mismatches so far.\n",
                (unsigned long long)nChecks, (unsigned long long)xldbgVerifyMismatches.load());
    }
}

bool RenderEngine::RenderEffectFromMap(bool suppress, Effect* effectObj, int layer, int period, SettingsMap& SettingsMap,
    PixelBufferClass& buffer, bool& resetEffectState)
{
    auto logger_render = spdlog::get("render");

    if (effectObj == nullptr) return false;

    if (layer >= buffer.GetLayerCount()) {
        logger_render->error("Model {} Effect {} at frame {} tried to render on a layer {} that does not exist (Only {} found).",
            (const char*)buffer.GetModel()->GetName().c_str(), (const char*)effectObj->GetEffectName().c_str(), period, layer + 1, buffer.GetLayerCount());
        assert(false);
        return false;
    }

    if (buffer.IsRenderingDisabled(layer)) {
        return false;
    }

    if (buffer.BufferForLayer(layer, -1).BufferHt == 0 || buffer.BufferForLayer(layer, -1).BufferWi == 0) {
        return false;
    }

    if (buffer.GetModel() != nullptr && buffer.GetModel()->GetNodeCount() == 0) {
        if (buffer.BufferForLayer(layer, 0).curEffStartPer == period) {
            logger_render->warn("Model {} has no nodes so skipping rendering.", (const char*)buffer.GetModel()->GetName().c_str());
        }
        return false;
    }

    bool retval = true;

    buffer.SetLayer(layer, period, resetEffectState);
    resetEffectState = false;
    int eidx = -1;

    xlColor colorMask = xlColor::NilColor();
    if (effectObj != nullptr) {
        eidx = effectObj->GetEffectIndex();

        const Model* m = buffer.GetModel();
        if (m == nullptr) {
            m = _ctx.GetModel(buffer.GetModelName());
        }

        if (m != nullptr) {
            if (m->GetStringType().compare(0, 12, "Single Color") == 0 || m->GetStringType() == "Node Single Color") {
                colorMask = buffer.GetNodeMaskColor(0);
                if (colorMask == xlBLACK) {
                    colorMask = xlColor::NilColor();
                }
            }
        }
        effectObj->SetColorMask(colorMask);
    }

    if (eidx >= 0) {
        RenderableEffect* reff = _ctx.GetEffectManager().GetEffect(eidx);

        if (reff) {
            RenderBuffer* b = &buffer.BufferForLayer(layer, -1);
            if (b == nullptr) {
                logger_render->warn("render on model {} layer {} effect {} from {}ms returned no buffer ... skipping rendering.", (const char*)buffer.GetModelName().c_str(), layer, (const char*)reff->Name().c_str(), effectObj->GetStartTimeMS());
            }
            else {
                // Time the whole dispatch on the slice thread (parallel_for
                // blocks it), so per-model group renders that fan out to worker
                // threads are still attributed to this effect.
                RenderJobProfile* effProf = profRender ? tlsRenderProfile : nullptr;
                std::chrono::steady_clock::time_point eff0;
                // Name is loop-invariant and stable for the app's life; resolve
                // it once here so the per-buffer lambda costs nothing when off.
                const char* effName = nullptr;
                if (effProf != nullptr) {
                    eff0 = std::chrono::steady_clock::now();
                    effName = reff->Name().c_str();
                }
                {
                    int bufCnt = buffer.BufferCountForLayer(layer);
                    std::function<void(int)> f([this, &buffer, layer, suppress, effectObj, reff, &SettingsMap, logger_render, effProf, effName](int bufn) {
                        RenderBuffer* rb = &buffer.BufferForLayer(layer, bufn);

                        if (rb != nullptr) {
                            RenderBuffer* oldBuffer = nullptr;
                            RenderBuffer* newBuffer = nullptr;

                            if (suppress) {
                                newBuffer = new RenderBuffer(*rb);
                                oldBuffer = rb;
                                rb = newBuffer;
                                rb->needToInit = oldBuffer->needToInit;
                                rb->infoCache = oldBuffer->infoCache;
                            }

                            auto sw = std::chrono::steady_clock::now();
                            {
                                // Any GPU work this effect encodes runs after the
                                // dispatch returns, so tag the thread and let the
                                // command buffer carry the attribution to whoever
                                // ends up waiting on it.
                                GpuEffectScope gpuScope(effProf, effName);
                                if (rb->pendingSnapshot != nullptr) {
                                    // Tier-2 draw pass (serial or parallel): Render()
                                    // rasterises the snapshot AdvanceState produced (it
                                    // checks pendingSnapshot itself and skips the sim
                                    // advance); no render-cache / verify on this path.
                                    reff->Render(effectObj, SettingsMap, *rb);
                                }
                                else if (rb->captureSnapshot != nullptr) {
                                    // Tier-2 capture pre-pass: advance the sim and store
                                    // this frame's draw snapshot without drawing.  Every
                                    // Snapshottable effect now produces it from AdvanceState;
                                    // a null here means the effect's GetFrameParallelism
                                    // classified it Snapshottable but AdvanceState returned
                                    // nothing - a classification bug.  Skip (never draw
                                    // during capture, which would corrupt the frame), and log.
                                    auto snap = reff->AdvanceState(effectObj, SettingsMap, *rb);
                                    if (snap != nullptr) {
                                        *rb->captureSnapshot = std::move(snap);
                                    } else {
                                        logger_render->error("Snapshottable effect '{}' produced no AdvanceState snapshot on model {} layer {} frame {} - classification bug (GetFrameParallelism==Snapshottable but AdvanceState==null); capture skipped.",
                                            (const char*)reff->Name().c_str(), (const char*)buffer.GetModelName().c_str(), layer, rb->curPeriod);
                                    }
                                }
                                else if (effectObj != nullptr && reff->SupportsRenderCache(SettingsMap) && _renderCache.IsEnabled()) {
                                    if (!effectObj->GetFrame(*rb, _renderCache)) {
                                        // Serial advance+draw: a migrated Snapshottable
                                        // effect advances here, then Render draws the
                                        // returned snapshot - identical to the draw pass.
                                        auto snap = reff->AdvanceState(effectObj, SettingsMap, *rb);
                                        if (snap != nullptr) rb->pendingSnapshot = snap.get();
                                        reff->Render(effectObj, SettingsMap, *rb);
                                        rb->pendingSnapshot = nullptr;
                                        GPURenderUtils::waitForRenderCompletion(rb);
                                        effectObj->AddFrame(*rb, _renderCache);
                                    }
                                }
                                else {
                                    auto snap = reff->AdvanceState(effectObj, SettingsMap, *rb);
                                    if (snap != nullptr) rb->pendingSnapshot = snap.get();
                                    reff->Render(effectObj, SettingsMap, *rb);
                                    rb->pendingSnapshot = nullptr;
                                    if (xldbgVerifyStateless && !suppress &&
                                        reff->GetEffectiveFrameParallelism(SettingsMap) == RenderableEffect::FrameParallelism::Pure) {
                                        VerifyStatelessRender(reff, effectObj, SettingsMap, rb, buffer.GetModelName(), layer);
                                    }
                                }
                            }

                            auto swElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - sw).count();
                            if (swElapsed > 150) {
                                logger_render->info("Frame #{} render on model {} ({}x{}) layer {} effect {} from {}ms (#{}) to {}ms (#{}) took more than 150 ms => {}ms.", rb->curPeriod, (const char*)buffer.GetModelName().c_str(), rb->BufferWi, rb->BufferHt, layer, (const char*)reff->Name().c_str(), effectObj->GetStartTimeMS(), rb->curEffStartPer, effectObj->GetEndTimeMS(), rb->curEffEndPer, swElapsed);
                            }

                            if (suppress && oldBuffer != nullptr) {
                                oldBuffer->needToInit = rb->needToInit;
                                oldBuffer->infoCache = rb->infoCache;
                                rb->infoCache.clear();
                                delete newBuffer;
                                rb = oldBuffer;
                                newBuffer = nullptr;
                                oldBuffer = nullptr;
                            }
                        }
                        });
                    // XL_SERIAL_PERMODEL=1: render per-model buffers serially —
                    // determinism diagnostic isolating this pool from other
                    // parallel_for uses.
                    static const bool serialPerModel = (getenv("XL_SERIAL_PERMODEL") != nullptr);
                    if (bufCnt > 1 && !serialPerModel) {
                        static ParallelJobPool PER_MODEL_POOL("per_model_pool");
                        parallel_for(0, bufCnt, [&f](int x) {f(x); }, 1, &PER_MODEL_POOL);
                    }
                    else {
                        for (int x = 0; x < bufCnt; x++) {
                            f(x);
                        }
                    }
                    buffer.MergeBuffersForLayer(layer);
                }
                if (effProf != nullptr) {
                    effProf->addEffect(reff->Name(), xlProfNs(eff0, std::chrono::steady_clock::now()));
                }
            }
        }
        else {
            retval = false;
        }
    }
    else {
        retval = false;
    }
    return retval;
}

void RenderEngine::OnRenderJobComplete(const std::string& modelName) {
    if (_onRenderJobComplete) _onRenderJobComplete(modelName);
}

void RenderEngine::OnAllRenderJobsComplete() {
    if (_onAllRenderJobsComplete) _onAllRenderJobsComplete();
}

void RenderEngine::RequeueJob(Job* job) {
    _jobPool.PushJob(job);
}

size_t RenderEngine::RecommendedPoolSize() {
    size_t hw = std::thread::hardware_concurrency();
    // Cap the GPU term: big-GPU Macs report 40-76 cores and the pool doesn't
    // need one thread per GPU core to keep the queues full.
    size_t gpu = std::min<size_t>((size_t)GPURenderUtils::GetGPUEffectConcurrency(), hw);
    return std::max<size_t>(8, hw + gpu + 4);
}

void RenderEngine::CheckForStalledRender() {
    if (_renderProgressInfo.empty()) {
        return;
    }
    // On iPad this is polled from more than one thread (main-actor timer plus
    // background drain loops); serialize the watchdog bookkeeping and let a
    // contended caller just skip - the 30s threshold doesn't need the sample.
    std::unique_lock<std::mutex> lk(_stallCheckLock, std::try_to_lock);
    if (!lk.owns_lock()) {
        return;
    }
    // The platforms poll this from ~10ms loops; the 30s stall threshold only
    // needs ~1s resolution, so skip the per-job scan most of the time.
    auto now = std::chrono::steady_clock::now();
    if (now - _lastStallCheck < std::chrono::seconds(1)) {
        return;
    }
    _lastStallCheck = now;

    // Per-batch, not gated on pool-wide idleness: another batch keeping the
    // pool busy must not mask a batch whose wake-up was lost.  A batch only
    // counts as stalled when every unfinished job is idle (suspended/parked,
    // holding no thread) - a job actively rendering a >30s frame is slow, not
    // stalled.  A spurious nudge is harmless: the job re-checks and re-suspends.
    for (auto rpi : _renderProgressInfo) {
        if (rpi->completed.load()) {
            continue;
        }
        long long sum = 0;
        bool anyUnfinished = false;
        bool allUnfinishedIdle = true;
        for (int i = 0; i < rpi->numRows; ++i) {
            if (rpi->jobs[i]) {
                int cur = rpi->jobs[i]->GetCurrentFrame();
                sum += cur;
                if (cur != END_OF_RENDER_FRAME) {
                    anyUnfinished = true;
                    if (!rpi->jobs[i]->IsIdle()) {
                        allUnfinishedIdle = false;
                    }
                }
            }
        }
        if (sum != rpi->lastProgressSum || !anyUnfinished || !allUnfinishedIdle) {
            rpi->lastProgressSum = sum;
            rpi->lastProgressTime = now;
        } else if (std::chrono::duration_cast<std::chrono::seconds>(now - rpi->lastProgressTime).count() >= 30) {
            spdlog::error("Render batch made no progress for 30s with all jobs idle. Rescheduling idle render jobs.");
            for (int i = 0; i < rpi->numRows; ++i) {
                if (rpi->jobs[i]) {
                    rpi->jobs[i]->NudgeIfSuspended();
                }
            }
            rpi->lastProgressTime = now;
        }
    }
}

// XL_RENDER_PROFILE dump: walk the batch's (still-alive) jobs, print a per-row
// table sorted by row wall time, a batch total, and a per-effect table sorted
// by total time.  Times in ms with one decimal.  Called only from the last
// job's thread before `completed` flips, so every profile is complete and no
// other thread is writing it.
static void DumpRenderProfile(RenderProgressInfo* rpi, long long elapsedMS) {
    struct Row {
        std::string name;
        const RenderJobProfile* p;
    };
    std::vector<Row> rows;
    RenderJobProfile total;
    for (int i = 0; i < rpi->numRows; ++i) {
        IRenderJobStatus* j = rpi->jobs[i];
        if (j == nullptr) {
            continue;
        }
        const RenderJobProfile* p = j->GetRenderProfile();
        if (p == nullptr || p->slices == 0) {
            continue;
        }
        rows.push_back({ j->GetName(), p });
        total.merge(*p);
    }
    std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) {
        return a.p->wallNs() > b.p->wallNs();
    });

    auto ms = [](uint64_t ns) { return (double)ns / 1.0e6; };
    auto pct = [](uint64_t part, uint64_t whole) { return whole ? (100.0 * (double)part / (double)whole) : 0.0; };
    auto lookup = [](const std::map<std::string, uint64_t>& m, const std::string& k) -> uint64_t {
        auto it = m.find(k);
        return it != m.end() ? it->second : 0ULL;
    };
    // Per-row effect split, top 3 by cpu+gpu - the cost that actually matters
    // when picking something to optimise.
    auto topEffects = [&ms, &lookup](const RenderJobProfile* p) {
        std::map<std::string, uint64_t> comb;
        for (const auto& e : p->perEffectNs) {
            comb[e.first] += e.second;
        }
        for (const auto& e : p->perEffectGpuNs) {
            comb[e.first] += e.second;
        }
        std::vector<std::pair<std::string, uint64_t>> v(comb.begin(), comb.end());
        std::sort(v.begin(), v.end(), [](const std::pair<std::string, uint64_t>& a, const std::pair<std::string, uint64_t>& b) {
            return a.second > b.second;
        });
        std::string s;
        char buf[96];
        for (size_t i = 0; i < v.size() && i < 3; i++) {
            snprintf(buf, sizeof(buf), "%s%s=%.1f", i ? " " : "", v[i].first.c_str(), ms(v[i].second));
            s += buf;
        }
        return s;
    };
    const bool gpuOn = GPURenderUtils::IsEnabled();
    const char* rowFmt = "%-28.28s %6llu %5llu %9.1f %9.1f %8.1f %8.1f %9.1f %9.1f %9.1f %8.1f %9.1f %9.1f %5.1f %5.1f  %s\n";

    fprintf(stderr, "\n=== XL_RENDER_PROFILE  frames %d-%d  wall %lldms  jobs %d  suspends %d  suspended %.1fms ===\n",
            rpi->startFrame, rpi->endFrame, elapsedMS, rpi->totalJobs,
            rpi->suspendCount.load(), ms((uint64_t)rpi->suspendedNs.load()));

    // Say plainly what the two cost columns are, because reading `effect` alone
    // as "what this effect costs" is precisely the mistake this table used to
    // invite: for a GPU effect it is only the dispatch encode.
    if (gpuOn) {
        if (total.gpuBusyNs == 0) {
            fprintf(stderr, "*** WARNING: GPU rendering is ON but no GPU time could be attributed.  The `effect`\n"
                            "*** column is CPU-side dispatch encode ONLY and UNDER-REPORTS GPU effects, possibly\n"
                            "*** by 100x.  Do NOT use it to pick optimisation targets.  (Backend did not report\n"
                            "*** command-buffer GPU timings - see GpuCommandBufferTag in RenderProfile.h.)\n");
        } else {
            fprintf(stderr, "GPU rendering ON.  `effect` = CPU dispatch/encode only.  `gpu` = GPU execution window of the\n"
                            "command buffers that effect opened (%llu buffers, %.1fms).  Rank by cpu+gpu, not by `effect`.\n"
                            "Caveat: command buffers from different rows overlap on the GPU, so `gpu` totals can exceed\n"
                            "the batch wall and inflate under contention - use it to rank, not as an absolute budget.\n",
                    (unsigned long long)total.gpuCbs, ms(total.gpuBusyNs));
            if (total.gpuSharedNs > 0) {
                fprintf(stderr, "Caveat: %.1fms (%.1f%% of gpu) ran on command buffers where GPU blur/rotozoom/transition\n"
                                "shared the effect's buffer; that time is charged to the effect and cannot be split out.\n",
                        ms(total.gpuSharedNs), pct(total.gpuSharedNs, total.gpuBusyNs));
            }
        }
    } else {
        fprintf(stderr, "GPU rendering OFF - `effect` is the whole cost; `gpu`/`gpuWait` are expected to be 0.\n");
    }

    fprintf(stderr, "%-28s %6s %5s %9s %9s %8s %8s %9s %9s %9s %8s %9s %9s %5s %5s  %s\n",
            "model", "frames", "slices", "effect", "gpu", "blurZ", "trans", "blend", "getCol", "setCol", "gpuWait", "suspend", "wall", "%gpu", "%sus", "top effects (ms)");
    for (const auto& r : rows) {
        const RenderJobProfile* p = r.p;
        fprintf(stderr, rowFmt,
                r.name.c_str(), (unsigned long long)p->frames, (unsigned long long)p->slices,
                ms(p->effectNs), ms(p->gpuBusyNs), ms(p->blurZoomNs), ms(p->transitionNs), ms(p->blendNs), ms(p->getColorsNs), ms(p->setColorsNs),
                ms(p->gpuWaitNs), ms(p->suspendedNs), ms(p->wallNs()),
                pct(p->gpuWaitNs, p->wallNs()), pct(p->suspendedNs, p->wallNs()),
                topEffects(p).c_str());
    }
    fprintf(stderr, rowFmt,
            "TOTAL", (unsigned long long)total.frames, (unsigned long long)total.slices,
            ms(total.effectNs), ms(total.gpuBusyNs), ms(total.blurZoomNs), ms(total.transitionNs), ms(total.blendNs), ms(total.getColorsNs), ms(total.setColorsNs),
            ms(total.gpuWaitNs), ms(total.suspendedNs), ms(total.wallNs()),
            pct(total.gpuWaitNs, total.wallNs()), pct(total.suspendedNs, total.wallNs()), "");

    // Per-effect table, ranked by cpu+gpu.  Keys are the union of the CPU and GPU
    // maps: GPU-only rows appear for stage work no effect owns ("(gpu blend)" etc).
    std::map<std::string, uint64_t> combined;
    for (const auto& e : total.perEffectNs) {
        combined[e.first] += e.second;
    }
    for (const auto& e : total.perEffectGpuNs) {
        combined[e.first] += e.second;
    }
    std::vector<std::pair<std::string, uint64_t>> effs(combined.begin(), combined.end());
    std::sort(effs.begin(), effs.end(), [](const std::pair<std::string, uint64_t>& a, const std::pair<std::string, uint64_t>& b) {
        return a.second > b.second;
    });
    fprintf(stderr, "--- per-effect (all rows), ranked by cpu+gpu ---\n");
    fprintf(stderr, "%-28s %9s %11s %11s %9s %11s\n", "effect", "renders", "cpu ms", "gpu ms", "gpu cbs", "cpu+gpu ms");
    for (const auto& e : effs) {
        uint64_t cpuNs = lookup(total.perEffectNs, e.first);
        uint64_t gNs = lookup(total.perEffectGpuNs, e.first);
        fprintf(stderr, "%-28.28s %9llu %11.1f %11.1f %9llu %11.1f\n",
                e.first.c_str(),
                (unsigned long long)lookup(total.perEffectCount, e.first),
                ms(cpuNs), ms(gNs),
                (unsigned long long)lookup(total.perEffectGpuCbs, e.first),
                ms(e.second));
    }
    fprintf(stderr, "\n");
}

void RenderEngine::NotifyJobFinished(RenderProgressInfo* rpi) {
    if (!rpi) return;
    // The thread that decrements the counter to zero is the last one out and
    // owns completion signaling. fetch_sub returns the pre-decrement value, so
    // returning 1 means "I just took the last slot". We only flip the atomic
    // flag here -- the callback itself typically touches UI and must run on
    // the platform's main thread, so firing is the platform drain's job
    // (desktop: UpdateRenderStatus on the wx main loop; iPad: IsRenderDone
    // poll).
    if (rpi->jobsRemaining.fetch_sub(1) != 1) return;

    // Log before flipping `completed` - once it flips, the main-thread drain
    // may delete rpi at any moment.  User-initiated renders (Render All,
    // batch render - the ones with a progress sink) log at info so the
    // summary is visible at default log levels; the per-edit micro-batches
    // only at debug to keep interactive editing from spamming the log.
    auto elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::steady_clock::now() - rpi->startTime).count();
    spdlog::log(rpi->progressSink ? spdlog::level::info : spdlog::level::debug,
                "Render batch complete: {} jobs over frames {}-{}, {} suspensions ({}ms), {} row parks, {}ms, {}",
                rpi->totalJobs, rpi->startFrame, rpi->endFrame,
                rpi->suspendCount.load(), (long long)(rpi->suspendedNs.load() / 1000000),
                rpi->parkCount.load(), (long long)elapsedMS,
                rpi->progressSink ? "background" : "interactive");

    if (profRender) {
        DumpRenderProfile(rpi, (long long)elapsedMS);
    }

    bool expected = false;
    rpi->completed.compare_exchange_strong(expected, true);
}


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
#include "effects/VideoEffect.h"
#include "IRenderJobStatus.h"
#include "IRenderProgressSink.h"
#include "RenderProgressInfo.h"
#include "RenderUtils.h"
#include "models/ModelGroup.h"
#include "UtilFunctions.h"
#include "PixelBuffer.h"
#include "Parallel.h"
#include "utils/RangeWorkPool.h"
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

// XL_RENDER_MEM=1 diagnostic: report what the render is spending memory on -
// per-row buffer bytes at setup, clone-slot growth, and the process footprint
// against the governor's budget.  Zero cost when unset.
static const bool xldbgRenderMem = (getenv("XL_RENDER_MEM") != nullptr);

// -------------------------------------------------------------------------
// Render memory governor
//
// The engine's buffer memory is a product: every row's buffers are built up
// front and held until the whole batch finishes, and each frame-parallel row
// then CLONES its entire buffer set once per concurrent frame
// (CreateParSlot).  Rows x layers x frame-concurrency is bounded only by the
// show, so a big enough one can ask for more than the machine has.
//
// This is a backstop, not a budget: on a real 63-row show the row buffers
// totalled 80MB and every clone slot together 394MB, so the governor stays
// dormant and costs nothing.  It exists for the show whose buffers ARE the
// memory - use XL_RENDER_MEM=1 first to find out whether that is what you are
// looking at, because a render that balloons past a few GB is usually
// something outside these buffers (a decoder cache, say) and throttling would
// only make it slow as well as doomed.
//
// Above a threshold it shrinks the discretionary term: fewer concurrent frames
// per row, and idle clone slots trimmed back to that allowance.  Purely
// advisory - it changes concurrency, never output - and it never throttles a
// row below one slot, so a row can always make progress.
//
// Thresholds are fractions of physical RAM.  Soft = stop growing the
// discretionary pools; Hard = shrink them.  XL_RENDER_MEM_LIMIT_MB overrides
// the soft limit outright (0 disables the governor) for A/B testing.
class RenderMemoryGovernor {
public:
    enum class Pressure { None, Soft, Hard };

    static RenderMemoryGovernor& Get() {
        static RenderMemoryGovernor g;
        return g;
    }

    bool Enabled() {
        Sample();
        std::unique_lock<std::mutex> lock(_lock);
        return _softLimitMB > 0;
    }
    uint64_t SoftLimitMB() {
        Sample();
        std::unique_lock<std::mutex> lock(_lock);
        return _softLimitMB;
    }
    uint64_t HardLimitMB() {
        Sample();
        std::unique_lock<std::mutex> lock(_lock);
        return _hardLimitMB;
    }

    // Cached because the allocation paths consult it often and it is a syscall.
    // 250ms is far finer than the rate a render can move the footprint by a
    // meaningful fraction of RAM.
    uint64_t FootprintMB() {
        Sample();
        std::unique_lock<std::mutex> lock(_lock);
        return _footprintMB;
    }
    uint64_t PeakMB() {
        Sample();
        std::unique_lock<std::mutex> lock(_lock);
        return _peakMB;
    }

    Pressure Level() {
        Sample();
        std::unique_lock<std::mutex> lock(_lock);
        if (_softLimitMB == 0) {
            return Pressure::None;
        }
        if (_footprintMB >= _hardLimitMB) {
            return Pressure::Hard;
        }
        if (_footprintMB >= _softLimitMB) {
            return Pressure::Soft;
        }
        return Pressure::None;
    }

    // How many frames of one row may render concurrently, given what one clone
    // of that row costs.  `want` is the tuned default for the row kind.  Never
    // returns less than 1: a row must always be able to render.
    int FrameConcurrency(int want, uint64_t perSlotBytes) {
        Sample();
        std::unique_lock<std::mutex> lock(_lock);
        if (_softLimitMB == 0 || want <= 1) {
            return want;
        }
        uint64_t mb = _footprintMB;
        uint64_t headroomMB = mb < _softLimitMB ? _softLimitMB - mb : 0;
        // Give any one row at most a slice of the remaining headroom, so the
        // first big row to start a window can't spend all of it before the
        // other rows get their first slot.
        uint64_t shareMB = headroomMB / 4;
        uint64_t perSlotMB = perSlotBytes / (1024 * 1024);
        if (perSlotMB == 0) {
            // Sub-MB clones are not what puts a machine in trouble.
            return want;
        }
        int allowed = (int)(shareMB / perSlotMB);
        return std::max(1, std::min(want, allowed));
    }

private:
    // Refresh footprint AND limits together.  The limits are re-derived rather
    // than fixed at construction because iOS/iPadOS can change a process's
    // memory cap during its life cycle (foreground/background, extended-memory
    // entitlement), and the governor is a process-lifetime singleton.
    void Sample() {
        auto now = std::chrono::steady_clock::now();
        std::unique_lock<std::mutex> lock(_lock);
        if (_sampled && now - _lastSample < std::chrono::milliseconds(250)) {
            return;
        }
        _sampled = true;
        _lastSample = now;
        _footprintMB = GetProcessMemoryUsageMB();
        if (_footprintMB > _peakMB) {
            _peakMB = _footprintMB;
        }

        if (_envLimitMB >= 0) {
            _softLimitMB = (uint64_t)_envLimitMB;
            _hardLimitMB = _softLimitMB + _softLimitMB / 8;
            return;
        }
        // Budget against whichever ceiling actually applies. On iPad that is
        // the per-process dirty-memory cap, which is well under installed RAM -
        // sizing off physical memory there would mean never throttling before
        // the OS kills us.
        uint64_t ceilingMB = GetPhysicalMemorySizeMB();
        const uint64_t procLimitMB = GetProcessMemoryLimitMB();
        if (procLimitMB != 0 && (ceilingMB == 0 || procLimitMB < ceilingMB)) {
            ceilingMB = procLimitMB;
        }
        if (ceilingMB == 0) {
            // Can't size a budget we can't measure; stay out of the way.
            _softLimitMB = 0;
            _hardLimitMB = 0;
            return;
        }
        // Fractions of that ceiling, with a floor so a small machine doesn't
        // throttle a render that was never the problem - but the floor is
        // itself capped, because on a device with a tight per-process cap a
        // floor above the ceiling means never throttling at all.
        _softLimitMB = std::min(std::max<uint64_t>(1024, ceilingMB * 60 / 100), ceilingMB * 70 / 100);
        _hardLimitMB = std::min(std::max<uint64_t>(1536, ceilingMB * 75 / 100), ceilingMB * 85 / 100);
    }

    // -1 = unset. Read once; getenv is not thread-safe to interleave with the
    // render's own environment reads.
    const long _envLimitMB = []() -> long {
        const char* e = getenv("XL_RENDER_MEM_LIMIT_MB");
        return e != nullptr ? strtol(e, nullptr, 10) : -1;
    }();

    std::mutex _lock;
    std::chrono::steady_clock::time_point _lastSample{};
    bool _sampled = false;
    uint64_t _footprintMB = 0;
    uint64_t _peakMB = 0;
    uint64_t _softLimitMB = 0;
    uint64_t _hardLimitMB = 0;
};

// XL_RENDER_MEM tallies. Only touched when xldbgRenderMem is set.
static std::atomic<uint64_t> xldbgCloneBytes{ 0 };
static std::atomic<uint64_t> xldbgCloneCount{ 0 };
static std::atomic<uint64_t> xldbgCloneDropped{ 0 };

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

// The frame-parallel worker pool.  Workers round-robin over every row's
// registered frame range, so a row can hold a long window without starving the
// other rows and no barrier is needed to hand the pool back (see RangeWorkPool).
// Each row caps its own frame concurrency (parChunkFrames), so pool size is
// what limits how many ROWS render frames in parallel, not how fast one row
// can go; one long run of frames is ONE registration rather than one job per
// frame, so it never wants more threads than there are cores.  Deliberately leaked:
// the workers are detached and must not become a static-destruction ordering
// dependency (same reasoning as JobPool.cpp's thread-name map).
// Frames, per-model buffers and every parallel_for share ONE pool.  Three pools
// of ~ncores workers each put ~3x ncores threads on the machine whenever the
// render nests (a frame row -> its per-model buffers -> an effect's own loop),
// and the round-robin only shares fairly WITHIN a pool - across them nothing
// coordinates, so the oversubscription just became context switches.  Measured
// on a 16-core box: sharing one pool cut total render CPU 22-28% and system time
// roughly in half, byte-identical.  A single pool is what the round-robin design
// was for; the fair claiming is what makes it safe, since a long frame index can
// now occupy a worker an inner loop wanted (that loop still progresses - its
// owner drains its own registration - just with less help).
// XL_SEPARATE_POOLS=1 restores the three-pool layout for A/B.
static bool UnifiedPool() {
    static const bool v = (getenv("XL_SEPARATE_POOLS") == nullptr);
    return v;
}

// The dedicated pools are constructed inside the !unified branch so the unified
// mode does not pay for their worker threads just by naming them.
static RangeWorkPool& PerModelPool() {
    if (UnifiedPool()) {
        return ParallelForPool();
    }
    static RangeWorkPool* pool = new RangeWorkPool("per_model_pool", std::max((int)std::thread::hardware_concurrency() - 1, 4));
    return *pool;
}

static RangeWorkPool& ParFramePool() {
    if (UnifiedPool()) {
        return ParallelForPool();
    }
    static RangeWorkPool* pool = []() {
        int c = (int)std::thread::hardware_concurrency() - 1; // 1 thread is the owner
        const char* e = getenv("XL_PARALLEL_FRAME_WORKERS");
        if (e != nullptr) {
            c = (int)strtol(e, nullptr, 10);
        }
        return new RangeWorkPool("par_frame_pool", std::max(c, 3));
    }();
    return *pool;
}

// Per row kind, clamped to [2, PAR_FRAME_MAX_CHUNK].  This is no longer a
// window LENGTH - a window now runs as long as the row's run of parallel-safe
// frames and upstream allow (see RenderParallelWindow).  It is the row's frame
// CONCURRENCY, which is what the old fixed window length actually controlled:
// how many frames of one row render at once, hence how many clone buffers it
// holds and how much nested parallelism (each frame's own parallel_fors) one
// row can provoke on the shared pool.  Measured: raising a group row from 8 to
// 16 concurrent frames costs ~10% more system time on IntoTheUnknown for no
// wall-clock gain - the frames contend on the default pool rather than adding
// throughput.  Groups stay lowest: their clone buffers are the big whole-house
// ones.  The same number also bounds how far ahead of upstream a new window may
// gate (ParWindowCap) and caps a Snapshottable window, whose serial capture
// pre-pass runs up front and holds a snapshot per captured frame.
static int ParChunkEnv(const char* name, int def) {
    const char* e = getenv(name);
    int v = e != nullptr ? (int)strtol(e, nullptr, 10) : def;
    return std::min(std::max(v, 2), PAR_FRAME_MAX_CHUNK);
}
static const int PAR_FRAME_MODEL_CHUNK = ParChunkEnv("XL_PARALLEL_CHUNK", PAR_FRAME_MAX_CHUNK);
static const int PAR_FRAME_GROUP_CHUNK = ParChunkEnv("XL_PARALLEL_GROUP_CHUNK", 8);
// Submodel rows also default shorter: each clone replicates the main buffer
// PLUS every submodel/strand buffer, so the pool build is the dominant cost on
// short sequences (Patriots-Touchdown 2.4s -> 4.4s at 24, 3.0s at 8) while the
// big wins are pool-size-independent (Baby Shark's Bushes -35% at both).
static const int PAR_FRAME_SUBMODEL_CHUNK = ParChunkEnv("XL_PARALLEL_SUBMODEL_CHUNK", 8);

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

    // Reset every per-window render field (current effects, cursors, settings,
    // valid flags, produce results) while keeping the identity fields and the
    // owned buffer.  The frame-parallel clone pool calls this before each
    // window so every clone re-initialises from scratch (output-invariant for
    // Pure effects), without reallocating the clone's PixelBufferClass.
    void resetRenderState() {
        currentEffects.assign(numLayers, nullptr);
        currentEffectIdxs.assign(numLayers, 0);
        settingsMaps.assign(numLayers, SettingsMap());
        effectStates.assign(numLayers, false);
        validLayers.assign(numLayers + 1, false);
        partOfCanvas.clear();
        processLayer.clear();
        produceEffectsToUpdate = false;
        produceBlend = false;
        produceEffectiveNumLayers = 0;
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

    bool IsFinished() override {
        return schedPhase == SchedPhase::Done;
    }

    // Deliberately avoids statusMsg / statusMap: those are a std::string and a
    // borrowed pointer written by the render thread without synchronisation, so
    // formatting them from the polling thread can fault - which would lose the
    // very log we are trying to produce.  Everything below is an atomic, a
    // volatile int, or read under a try_lock.
    std::string GetHangStatus() override {
        const char* phase = "?";
        switch (schedPhase) {
        case SchedPhase::Setup:  phase = "setup"; break;
        case SchedPhase::Frames: phase = "frames"; break;
        case SchedPhase::Finish: phase = "finish"; break;
        case SchedPhase::Done:   phase = "done"; break;
        }

        std::string sched;
        {
            // A try_lock, not a lock: if the hang IS this mutex, blocking here
            // would hang the caller instead of reporting the deadlock.
            std::unique_lock<std::mutex> lock(nextLock, std::try_to_lock);
            if (!lock.owns_lock()) {
                sched = "scheduler lock held by another thread";
            } else if (suspended) {
                sched = fmt::format("suspended waiting for upstream frame {} (upstream is at {})", wantFrame, (int)GetPreviousFrameDone());
            } else if (parked) {
                sched = "parked behind another job rendering this row";
            } else if (inPool.load()) {
                sched = "queued in the render pool";
            } else {
                sched = "holding a render thread";
            }
        }

        const char* what = "unknown";
        switch (statusType) {
        case 0:        what = "status message"; break;
        case 1:        what = "initializing effect"; break;
        case 2:        what = "rendering effect"; break;
        case 3:        what = "calculating output"; break;
        case 4: case 5:
        case 8: case 9: what = "between effects"; break;
        case 13:       what = "waiting to start frame"; break;
        }

        int cur = currentFrame;
        std::string progress = (cur == END_OF_RENDER_FRAME)
            ? fmt::format("all {} frames rendered", (int)endFrame - (int)startFrame + 1)
            : fmt::format("frame {} of {}-{}", cur, (int)startFrame, (int)endFrame);

        // A running frame-parallel window holds the row's thread for its whole
        // run, so "holding a render thread" alone cannot tell a frame that is
        // still rendering from one that will never finish.  Naming the frames
        // in flight (and for how long) separates the two, and an empty list
        // with the window short of its end means no thread is on the remaining
        // frames at all - a scheduling loss, not a slow effect.
        std::string window;
        if (parWinActive.load()) {
            std::string inflight;
            std::unique_lock<std::mutex> lock(parWinLock, std::try_to_lock);
            if (!lock.owns_lock()) {
                inflight = "in-flight list locked by another thread";
            } else if (parWinInFlight.empty()) {
                inflight = "NO frames in flight";
            } else {
                const auto now = std::chrono::steady_clock::now();
                for (const auto& [f, started] : parWinInFlight) {
                    if (!inflight.empty()) {
                        inflight += ", ";
                    }
                    inflight += fmt::format("frame {} for {}s", f,
                                            (long long)std::chrono::duration_cast<std::chrono::seconds>(now - started).count());
                }
            }
            window = fmt::format(", frame-parallel window [{},{}] finished {} of {} ({})",
                                 parWinA.load(), parWinE.load(), parWinDone.load(),
                                 parWinE.load() - parWinA.load() + 1, inflight);
        }

        return fmt::format("{}: {}, {}, phase {}{}, last {} at frame {} layer {} submodel {} strand {} node {}{}",
                           name, progress, sched, phase, abort.load() ? " (abort signalled)" : "",
                           what, (int)statusFrame, (int)statusLayer,
                           (int)statusSubmodel, (int)statusStrand, (int)statusNode, window);
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

    // Per-effect classification cache for the window-formation scans.  An
    // effect spans many frames but FrameIsParallelSafe runs per frame, and the
    // classification needs a full CopySettingsMap (string-map copy) - on
    // submodel rows that cost is multiplied by every submodel layer.  Any
    // effect edit bumps the row change count and the frame-loop bail replaces
    // the job (and this memo with it), so an entry is valid for the job's
    // life.  Only touched by the job's own slice thread.
    // Video is exempt: its classification consults a per-file registry primed
    // when Render first opens the file, so it can legitimately change mid-job.
    struct ParEffClass {
        RenderableEffect::FrameParallelism fp;
        bool continuity;
    };
    std::unordered_map<Effect*, ParEffClass> parClassMemo;
    const ParEffClass& ClassifyEffectForParallel(Effect* eff) {
        auto it = parClassMemo.find(eff);
        if (it == parClassMemo.end() || eff->GetEffectIndex() == EffectManager::eff_VIDEO) {
            ParEffClass c{ RenderableEffect::FrameParallelism::Stateful, false };
            RenderableEffect* reff = _ctx->GetEffectManager().GetEffect(eff->GetEffectIndex());
            if (reff != nullptr) {
                SettingsMap sm;
                eff->CopySettingsMap(sm, true);
                c.fp = reff->GetEffectiveFrameParallelism(sm);
            }
            c.continuity = EffectIsBufferContinuity(eff);
            it = parClassMemo.insert_or_assign(eff, c).first;
        }
        return it->second;
    }

    // One layer's contribution to FrameIsParallelSafe: -1 = blocked (Stateful
    // cover, unknown effect, or a buffer-continuity effect starting at frame+1),
    // 0 = no cover, 1 = safe cover (Pure or Snapshottable; the latter also sets
    // hasSnapshot).
    int LayerParallelSafety(EffectLayer* el, int frame, bool& hasSnapshot) {
        if (el == nullptr) return 0;
        std::unique_lock<std::recursive_mutex> lock(el->GetLock());
        int idx = 0;
        Effect* eff = findEffectForFrame(el, frame, idx);
        // The enter transition into a buffer-continuity effect: if one STARTS
        // at frame+1, this frame must render serially so the serial buffer is
        // advanced/cleared for it (a Pure window would render it on a clone and
        // leave the serial buffer stale).  Only a CHANGE of covering effect at
        // frame+1 is a boundary worth inspecting - the common case (same effect
        // spans both frames) is a cheap pointer compare that skips the settings
        // read.
        int idx2 = 0;
        Effect* effNext = findEffectForFrame(el, frame + 1, idx2);
        if (effNext != nullptr && effNext != eff && ClassifyEffectForParallel(effNext).continuity) {
            return -1;
        }
        if (eff == nullptr) return 0;
        RenderableEffect::FrameParallelism fp = ClassifyEffectForParallel(eff).fp;
        if (fp == RenderableEffect::FrameParallelism::Snapshottable) {
            hasSnapshot = true;
            return 1;
        }
        return fp == RenderableEffect::FrameParallelism::Pure ? 1 : -1; // Stateful
    }

    // A frame is parallel-safe if every covering layer - across the main model
    // AND every submodel/strand row it carries - is Pure OR Snapshottable.
    // hasSnapshot is SET (never cleared - the caller inits it) when any covering
    // layer is Snapshottable, so the window knows it needs a serial capture pre-pass.
    bool FrameIsParallelSafe(int frame, bool& hasSnapshot) {
        bool anyCover = false;
        for (int l = 0; l < numLayers; ++l) {
            int r = LayerParallelSafety(rowToRender->GetEffectLayer(l), frame, hasSnapshot);
            if (r < 0) return false;
            anyCover |= (r > 0);
        }
        for (const auto& smi : subModelInfos) {
            Element* se = smi->element;
            if (se == nullptr) continue;
            const int subLayers = std::min((int)se->GetEffectLayerCount(), smi->numLayers);
            for (int l = 0; l < subLayers; ++l) {
                int r = LayerParallelSafety(se->GetEffectLayer(l), frame, hasSnapshot);
                if (r < 0) return false;
                anyCover |= (r > 0);
            }
        }
        return anyCover;
    }

    // The covering effect on `el` at `frame` is Snapshottable (needs the serial
    // capture pre-pass).  Used to restrict that pre-pass to just those layers.
    bool LayerIsSnapshottableAt(EffectLayer* el, int frame) {
        if (el == nullptr) return false;
        std::unique_lock<std::recursive_mutex> lock(el->GetLock());
        int idx = 0;
        Effect* eff = findEffectForFrame(el, frame, idx);
        if (eff == nullptr) return false;
        return ClassifyEffectForParallel(eff).fp == RenderableEffect::FrameParallelism::Snapshottable;
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

    // One frame's worth of clone render context - held for the duration of one
    // frame, then returned to parFreeSlots.  parSlots owns them; the unique_ptr
    // indirection keeps a ParSlot's address stable as the vector grows, so a
    // thread rendering into a slot is unaffected by another thread building one.
    struct ParSlot {
        std::unique_ptr<PixelBufferClass> buffer;
        std::unique_ptr<EffectLayerInfo> info;
        // Per-submodel/strand infos (own buffers), mirroring subModelInfos -
        // empty for rows without submodels.
        std::vector<std::unique_ptr<EffectLayerInfo>> subs;
    };

    // Build one clone slot: a full per-frame render context - a main-buffer
    // clone plus, for submodel rows (item 03 step 3), one EffectLayerInfo+buffer
    // per subModelInfos entry, mirroring the shapes the ctor built (submodel
    // InitBuffer / strand InitStrandBuffer, same layer counts).  Returns null if
    // a submodel lookup fails (the model changed under us).
    std::unique_ptr<ParSlot> CreateParSlot() {
        const Model* mdl = mainBuffer->GetModel();
        auto slot = std::make_unique<ParSlot>();
        slot->buffer = std::make_unique<PixelBufferClass>(_ctx);
        slot->buffer->InitBuffer(*mdl, numLayers, seqData->FrameTime());
        slot->info = std::make_unique<EffectLayerInfo>(numLayers);
        for (const auto& smi : subModelInfos) {
            auto si = std::make_unique<EffectLayerInfo>(smi->numLayers);
            si->element = smi->element;
            si->strand = smi->strand;
            si->submodel = smi->submodel;
            si->buffer.reset(new PixelBufferClass(_ctx));
            if (smi->strand >= 0) {
                si->buffer->InitStrandBuffer(*mdl, smi->strand, seqData->FrameTime(), smi->numLayers - 1);
            } else {
                Model* subModel = mdl->GetSubModel(smi->element->GetName());
                if (subModel == nullptr) {
                    return nullptr;
                }
                si->buffer->InitBuffer(*subModel, smi->numLayers, seqData->FrameTime());
            }
            slot->subs.push_back(std::move(si));
        }
        return slot;
    }

    // Claim a slot for the duration of ONE frame, building a new one if the
    // free list is dry.  The list can only run dry while fewer slots exist than
    // threads currently inside this row's work item, and that is capped at
    // parChunkFrames, so a row allocates at most that many slots however long
    // its windows are - clone memory follows the row's concurrency, not the
    // length of the run being farmed out.  Creation stays under the lock: it is rare (once per
    // slot for the row's whole life) and it keeps PixelBufferClass/Model
    // initialisation for one model single-threaded, as it has always been.
    // Returns null only if CreateParSlot failed.
    ParSlot* AcquireParSlot() {
        std::unique_lock<std::mutex> lock(parSlotLock);
        if (!parFreeSlots.empty()) {
            ParSlot* s = parFreeSlots.back();
            parFreeSlots.pop_back();
            return s;
        }
        auto slot = CreateParSlot();
        if (slot == nullptr) {
            return nullptr;
        }
        ParSlot* s = slot.get();
        if (xldbgRenderMem) {
            uint64_t b = s->buffer->GetApproxMemoryBytes();
            for (const auto& si : s->subs) {
                if (si->buffer) {
                    b += si->buffer->GetApproxMemoryBytes();
                }
            }
            xldbgCloneBytes.fetch_add(b);
            xldbgCloneCount.fetch_add(1);
        }
        parSlots.push_back(std::move(slot));
        return s;
    }

    void ReleaseParSlot(ParSlot* s) {
        std::unique_lock<std::mutex> lock(parSlotLock);
        parFreeSlots.push_back(s);
    }

    // What one clone of this row costs, measured off the row's own buffers -
    // CreateParSlot replicates exactly that shape.  Cached after the first
    // call: the layer buffers settle once the row's effects have been applied,
    // and walking every node of a whole-house group is not free.
    uint64_t ParSlotCostBytes() {
        if (parSlotCostBytes == 0) {
            parSlotCostBytes = GetRowBufferMemoryBytes();
        }
        return parSlotCostBytes;
    }

    // The row's frame concurrency after the memory governor has had its say.
    // Under hard pressure the row also hands back clones it is no longer
    // allowed to use.
    int ThrottledFrameConcurrency() {
        RenderMemoryGovernor& gov = RenderMemoryGovernor::Get();
        if (!gov.Enabled()) {
            return parChunkFramesWanted;
        }
        int allowed = gov.FrameConcurrency(parChunkFramesWanted, ParSlotCostBytes());
        if (gov.Level() == RenderMemoryGovernor::Pressure::Hard) {
            TrimParSlotsTo(allowed);
        }
        if (xldbgRenderMem && allowed != parChunkFramesWanted) {
            fprintf(stderr, "XL_RENDER_MEM THROTTLE m='%s' chunk %d -> %d (clone %.1f MB, footprint %llu/%llu MB)\n",
                    name.c_str(), parChunkFramesWanted, allowed,
                    ParSlotCostBytes() / (1024.0 * 1024.0),
                    (unsigned long long)gov.FootprintMB(), (unsigned long long)gov.SoftLimitMB());
        }
        return allowed;
    }

    // Give back idle clone slots until the row holds no more than `keep`.
    // Trimming to the allowance rather than emptying the free list is what
    // makes this stable: once a row is down to its allowance the next window
    // finds nothing to drop, instead of dropping every slot and rebuilding it
    // on the next Acquire for as long as the pressure lasts.  Slots currently
    // held by a frame are not in parFreeSlots and are left alone.
    void TrimParSlotsTo(int keep) {
        std::unique_lock<std::mutex> lock(parSlotLock);
        size_t dropped = 0;
        while (parSlots.size() > (size_t)std::max(keep, 1) && !parFreeSlots.empty()) {
            const ParSlot* victim = parFreeSlots.back();
            parFreeSlots.pop_back();
            auto it = std::find_if(parSlots.begin(), parSlots.end(),
                                   [victim](const std::unique_ptr<ParSlot>& p) { return p.get() == victim; });
            if (it == parSlots.end()) {
                break;
            }
            parSlots.erase(it);
            ++dropped;
        }
        if (xldbgRenderMem && dropped != 0) {
            xldbgCloneDropped.fetch_add(dropped);
        }
    }

    // One frame's captured draw snapshots, by [unit][layer] - unit 0 is the main
    // model, unit u is subModelInfos[u-1].  Null for Pure/empty layers.  Held in
    // a small ring (see RenderParallelWindow) rather than one entry per window
    // frame, so a Snapshottable window's length is not bounded by how many
    // snapshots fit in memory.
    struct SnapFrame {
        std::vector<std::vector<std::unique_ptr<EffectFrameState>>> byUnit;
    };

    // Advance the serial simulation by one frame on the REAL buffers and capture
    // each Snapshottable layer's immutable draw snapshot into `sf`, without
    // drawing.  Inherently serial - the sim state is carried frame to frame on
    // mainBuffer and the submodel buffers - and inherently in frame order, so
    // this is the head of the window's pipeline and only the owner runs it.
    // Clearing `sf` first releases the snapshots of the frame that previously
    // held this ring slot.
    void CaptureSnapshotFrame(int f, SnapFrame& sf) {
        const int nUnits = 1 + (int)subModelInfos.size();
        sf.byUnit.clear();
        sf.byUnit.resize(nUnits);
        sf.byUnit[0].resize(numLayers);
        // Restrict the capture to Snapshottable layers only: they advance the
        // sim + capture.  Pure layers are skipped entirely (they may run
        // expensive parallel_fors) and rendered fresh in the parallel pass.
        mainModelInfo.processLayer.assign(numLayers, false);
        for (int l = 0; l < numLayers; ++l) {
            if (LayerIsSnapshottableAt(rowToRender->GetEffectLayer(l), f)) {
                mainModelInfo.processLayer[l] = true;
                mainBuffer->BufferForLayer(l, -1).captureSnapshot = &sf.byUnit[0][l];
            }
        }
        ProduceFrame(f, rowToRender, mainModelInfo, mainBuffer, -1, supportsModelBlending);
        for (int l = 0; l < numLayers; ++l) {
            mainBuffer->BufferForLayer(l, -1).captureSnapshot = nullptr;
        }
        // Same over each submodel/strand unit that has a Snapshottable cover at
        // this frame: advance the sim on the REAL submodel buffer (so serial
        // state continuity is preserved across and after the window) and
        // capture the draw snapshot.
        for (int u = 1; u < nUnits; ++u) {
            EffectLayerInfo* smi = subModelInfos[u - 1];
            Element* se = smi->element;
            if (se == nullptr) continue;
            const int subLayers = std::min((int)se->GetEffectLayerCount(), smi->numLayers);
            sf.byUnit[u].resize(smi->numLayers);
            smi->processLayer.assign(smi->numLayers, false);
            bool any = false;
            for (int l = 0; l < subLayers; ++l) {
                if (LayerIsSnapshottableAt(se->GetEffectLayer(l), f)) {
                    smi->processLayer[l] = true;
                    smi->buffer->BufferForLayer(l, -1).captureSnapshot = &sf.byUnit[u][l];
                    any = true;
                }
            }
            if (any) {
                ProduceFrame(f, se, *smi, smi->buffer.get(), smi->strand, supportsModelBlending);
            }
            for (int l = 0; l < subLayers; ++l) {
                smi->buffer->BufferForLayer(l, -1).captureSnapshot = nullptr;
            }
            smi->processLayer.clear();
        }
    }

    // Finish capturing: restore full-layer rendering for any later serial frame,
    // and quiesce the serial buffers' GPU state as a belt-and-suspenders
    // backstop.  ProduceFrame SKIPs blur/rotozoom/transitions on captured layers
    // (captureSnapshot set), so capturing should not encode any GPU work and
    // this is expected to be a no-op (cheap when there is nothing outstanding).
    // Kept because if an effect's capture path ever encodes GPU work, leaving it
    // open would commit stale blits at the next serial frame's first wait,
    // corrupting exactly that frame (then self-healing).
    void FinishSnapshotCapture() {
        mainModelInfo.processLayer.clear();
        for (int l = 0; l < numLayers; ++l) {
            GPURenderUtils::waitForRenderCompletion(&mainBuffer->BufferForLayer(l, -1));
        }
        for (const auto& smi : subModelInfos) {
            for (int l = 0; l < smi->numLayers; ++l) {
                GPURenderUtils::waitForRenderCompletion(&smi->buffer->BufferForLayer(l, -1));
            }
        }
    }

    // Returns the slot even if the frame throws - otherwise an effect that
    // throws every frame would drop a slot each time and the pool would grow
    // without bound.
    struct ParSlotHold {
        RenderJob* job;
        ParSlot* slot;
        ParSlotHold(RenderJob* j) : job(j), slot(j->AcquireParSlot()) {}
        ~ParSlotHold() {
            if (slot != nullptr) {
                job->ReleaseParSlot(slot);
            }
        }
        ParSlotHold(const ParSlotHold&) = delete;
        ParSlotHold& operator=(const ParSlotHold&) = delete;
    };

    // Publishes "this thread is inside frame f of the running window" for the
    // hang diagnostic.  Scoped so an exception out of the frame still retires
    // the entry (the pool swallows those).
    struct ParFrameMark {
        RenderJob* job;
        int frame;
        ParFrameMark(RenderJob* j, int f) : job(j), frame(f) {
            std::unique_lock<std::mutex> lock(job->parWinLock);
            job->parWinInFlight[frame] = std::chrono::steady_clock::now();
        }
        ~ParFrameMark() {
            std::unique_lock<std::mutex> lock(job->parWinLock);
            job->parWinInFlight.erase(frame);
        }
        ParFrameMark(const ParFrameMark&) = delete;
        ParFrameMark& operator=(const ParFrameMark&) = delete;
    };

    // Render frames [a,e] FULLY (produce + blur/transitions + blend/CalcOutput +
    // the seqData write) concurrently, each into its own clone buffer.  Every
    // frame writes only seqData[frame] - a distinct row - so the whole per-frame
    // pipeline is independent, not just produce().  The caller must have already
    // satisfied the upstream gate through `e` (OutputFrame's CanOutputFrame is a
    // backstop).  FrameDone / currentFrame are emitted HERE, streamed per frame:
    // frames finish out of order, so a cursor advances over the contiguous
    // finished prefix and emits in frame order - downstream rows gated on frame
    // f start as soon as f lands instead of waiting for the whole window.
    // Each clone's EffectLayerInfo is reset so ProduceFrame re-initialises from
    // scratch (output-invariant for Pure effects).
    //
    // The frames are registered as ONE range on the round-robin RangeWorkPool
    // rather than submitted as a parallel_for.  Two consequences shape the rest
    // of this function: the window no longer has to be short to keep the pool
    // available to other rows (workers rotate between rows at frame
    // granularity), and it can GROW while it runs (Extend) instead of ending at
    // a barrier and being rebuilt.  So the window runs as long as the row's run
    // of parallel-safe frames and upstream allow, and the two things that used
    // to be implied by "short window" are now explicit: clone buffers come from
    // a concurrency-sized free list (AcquireParSlot), and an abort / structural
    // edit is checked per frame instead of only between windows.
    //
    // Returns the last frame rendered - a-1 if none, which happens only when the
    // window was cut short immediately; the caller resumes there and its
    // top-of-frame bail handles the abort.  May return MORE than `e` when the
    // window extended.
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
    int RenderParallelWindow(int a, int e, bool hasSnapshot) {
        int n = e - a + 1;
        // Build the first slot here, serially, so a submodel-lookup failure is
        // caught before any frame is farmed out - as the pre-sized clone pool
        // used to do.
        if (ParSlot* probe = AcquireParSlot()) {
            ReleaseParSlot(probe);
        } else {
            // Clone pool unavailable (a submodel lookup failed - the model
            // changed under us).  The caller already gated upstream through e,
            // so render the window serially on the real buffers and never try
            // to parallelise this row again.
            parEligible = false;
            for (int f = a; f <= e; ++f) {
                ProduceFrameAll(f);
                producedFrame = f;
                OutputFrameAll(f);
                currentFrame = f;
                if (HasNext()) {
                    FrameDone(f);
                }
            }
            return e;
        }
        // Re-price this row's frame concurrency against the memory governor now
        // that a clone exists to measure.  Done per window rather than once per
        // row because pressure is a property of the whole batch: a row that
        // opened its first window while memory was free must throttle when the
        // rest of the show has since filled it, and recover when it drains.
        parChunkFrames = ThrottledFrameConcurrency();
        parWinA = a;
        parWinE = e;
        parWinDone = 0;
        parWinActive = true;
        struct ParWinScope {
            RenderJob* job;
            ~ParWinScope() { job->parWinActive = false; }
        } parWinScope{ this };

        const int nUnits = 1 + (int)subModelInfos.size();
        if (xldbgParallelWindows && nUnits > 1) {
            fprintf(stderr, "XL_PARALLEL_WINDOWS SUB-WINDOW m='%s' a=%d e=%d units=%d snap=%d\n",
                    rowToRender->GetModelName().c_str(), a, e, nUnits, hasSnapshot ? 1 : 0);
        }
        if (hasSnapshot && xldbgParallelWindows) {
            fprintf(stderr, "XL_PARALLEL_WINDOWS SNAP-WINDOW m='%s' a=%d e=%d layers=%d\n",
                    rowToRender->GetModelName().c_str(), a, e, numLayers);
        }
        // Ring of captured frames for a Snapshottable window; slot (f-a)%snapRing
        // holds frame f.  The owner captures one frame ahead of the draws rather
        // than pre-passing the whole window, so the serial advance overlaps the
        // parallel draw instead of preceding all of it, and the window's length
        // stops being bounded by how many snapshots fit in memory.  The ring must
        // be larger than the row's frame concurrency: the finished PREFIX is what
        // says a slot is reusable, and it lags the claimed frames by up to that
        // many, so a ring of exactly parChunkFrames would never have free space.
        const int snapRing = hasSnapshot ? 2 * parChunkFrames : 0;
        std::vector<SnapFrame> snaps(snapRing);
        // Streamed completion state.  FrameDone must be exact-once per frame
        // (AggregatorRenderer counts one call per upstream per frame) and in
        // increasing order (done(N) promises every frame <= N is in seqData),
        // hence the contiguous-prefix cursor under a lock rather than emitting
        // from each worker directly.  Indices are claimed in order, so the
        // prefix advances close behind the workers.  A frame that throws (the
        // pool swallows it) or is skipped by a cut stalls the tail emissions;
        // the next serial FrameDone / END is monotonic and recovers the
        // watermark, and the caller resumes from the prefix.
        std::vector<bool> finished(n, false);
        int doneCursor = 0;
        std::mutex doneLock;
        std::condition_variable doneCv;
        bool capWaiting = false; // guarded by doneLock; see the capture drive
        const bool relay = HasNext();
        // Set when the window must stop early: an abort or a structural edit
        // landed, or a clone slot could not be built.  Windows now run long
        // enough that waiting for the whole window to drain before noticing is
        // not acceptable.  Already-claimed indices return immediately.
        std::atomic<bool> cut{ false };
        // Set by a worker; parEligible is the owner's to write (it is read on
        // the owner's frame loop without a lock).
        std::atomic<bool> slotFailed{ false };

        // Cutting must wake the capture owner: it can be parked on doneCv
        // waiting for ring space, and a cut means no frame will ever finish to
        // free some.  Taking doneLock to notify is what makes the owner's
        // check-then-wait airtight.
        auto signalCut = [&]() {
            cut.store(true, std::memory_order_relaxed);
            std::unique_lock<std::mutex> dl(doneLock);
            doneCv.notify_all();
        };

        auto renderOne = [&](int i) {
            if (cut.load(std::memory_order_relaxed)) {
                return;
            }
            if (abort || origChangeCount != rowToRender->getChangeCount()) {
                signalCut();
                return;
            }
            ParSlotHold hold(this);
            ParSlot* slot = hold.slot;
            if (slot == nullptr) {
                slotFailed.store(true, std::memory_order_relaxed);
                signalCut();
                return;
            }
            int f = a + i;
            ParFrameMark mark(this, f);
            slot->info->resetRenderState();
            for (auto& si : slot->subs) {
                si->resetRenderState();
            }
            // The owner published index i only after capturing it, and will not
            // reuse this ring slot until i is in the finished prefix.
            const SnapFrame* sf = hasSnapshot ? &snaps[i % snapRing] : nullptr;
            if (sf != nullptr) {
                for (int l = 0; l < numLayers; ++l) {
                    if (sf->byUnit[0][l]) slot->buffer->BufferForLayer(l, -1).pendingSnapshot = sf->byUnit[0][l].get();
                }
            }
            ProduceFrame(f, rowToRender, *slot->info, slot->buffer.get(), -1, supportsModelBlending);
            if (hasSnapshot) {
                for (int l = 0; l < numLayers; ++l) {
                    slot->buffer->BufferForLayer(l, -1).pendingSnapshot = nullptr;
                }
            }
            // Submodel/strand units, in subModelInfos order - the same produce
            // arguments and ordering as ProduceFrameAll on the serial path.
            if (!slot->subs.empty()) {
                const bool cleared = slot->info->produceEffectsToUpdate;
                const std::string inh = InheritedDuplicateSourceModel(f);
                for (size_t u = 0; u < slot->subs.size(); ++u) {
                    EffectLayerInfo* si = slot->subs[u].get();
                    if (sf != nullptr) {
                        const auto& lv = sf->byUnit[u + 1];
                        for (int l = 0; l < si->numLayers && l < (int)lv.size(); ++l) {
                            if (lv[l]) si->buffer->BufferForLayer(l, -1).pendingSnapshot = lv[l].get();
                        }
                    }
                    ProduceFrame(f, si->element, *si, si->buffer.get(), si->strand, supportsModelBlending ? true : cleared, inh);
                    if (hasSnapshot) {
                        for (int l = 0; l < si->numLayers; ++l) {
                            si->buffer->BufferForLayer(l, -1).pendingSnapshot = nullptr;
                        }
                    }
                }
            }
            OutputFrame(f, rowToRender, *slot->info, slot->buffer.get(), -1);
            for (auto& si : slot->subs) {
                OutputFrame(f, si->element, *si, si->buffer.get(), si->strand);
            }

            std::unique_lock<std::mutex> dl(doneLock);
            finished[i] = true;
            while (doneCursor < (int)finished.size() && finished[doneCursor]) {
                int df = a + doneCursor;
                currentFrame = df; // UI progress advances per frame, not per window
                if (relay) {
                    FrameDone(df);
                }
                ++doneCursor;
                parWinDone = doneCursor;
            }
            if (capWaiting) {
                doneCv.notify_all(); // the capture owner is parked on ring space
            }
        };

        // Grow [a,e] while the parallel-safe run continues, upstream allows, and
        // the frames keep matching the window's kind (NextParallelWindowEnd).
        auto growWindow = [&](bool wantSnapshot) {
            int e2 = NextParallelWindowEnd(e, wantSnapshot);
            if (e2 <= e) {
                return false;
            }
            {
                std::unique_lock<std::mutex> dl(doneLock);
                finished.resize(e2 - a + 1, false);
            }
            e = e2;
            parWinE = e2;
            return true;
        };

        // The item is live the moment it is registered - workers may be calling
        // renderOne before Register returns - so everything it captures is
        // declared above.  A Snapshottable window starts EMPTY and is published
        // one frame at a time by the capture below; a Pure window publishes the
        // whole run immediately.
        auto item = ParFramePool().Register(renderOne, 0, hasSnapshot ? 0 : n, parChunkFrames);
        if (hasSnapshot) {
            // Pipelined capture: capture a WAVE of frames on the serial buffers,
            // publish it, and go straight on to the next wave while workers draw
            // the last one.  The serial advance is then overlapped with the
            // parallel draw instead of being a barrier in front of it.
            //
            // Per-wave, not per-frame, because a Snapshottable effect's advance
            // is cheap by construction - that is what makes it worth snapshotting
            // at all - so publishing each frame as it is captured spends more on
            // synchronisation (~5us/frame of lock + notify) than the capture
            // latency it hides.  Measured on Alice's Restaurant: +1.0s of CPU
            // for 16505 frames x 13 rows, which per-wave publishing removes.  A
            // wave is one full concurrency's worth and the ring holds two, so
            // one wave draws while the next is captured.
            int cap = a; // next frame to capture
            while (!cut.load(std::memory_order_relaxed)) {
                if (cap > e && !growWindow(true)) {
                    break;
                }
                // Ring space: index idx may be captured while idx - snapRing is
                // in the finished prefix, so its slot is no longer being read.
                int space;
                {
                    std::unique_lock<std::mutex> dl(doneLock);
                    space = doneCursor + snapRing - (cap - a);
                    if (space <= 0) {
                        // Draw one ourselves rather than idle - if no worker has
                        // picked this row up, waiting would be waiting on
                        // nothing.
                        dl.unlock();
                        if (!item->RunOwnerOne()) {
                            dl.lock();
                            if (doneCursor + snapRing - (cap - a) <= 0 && !cut.load(std::memory_order_relaxed)) {
                                capWaiting = true;
                                doneCv.wait(dl);
                                capWaiting = false;
                            }
                        }
                        continue;
                    }
                }
                const int wave = std::min({ space, parChunkFrames, e - cap + 1 });
                for (int k = 0; k < wave; ++k) {
                    CaptureSnapshotFrame(cap, snaps[(cap - a) % snapRing]);
                    ++cap;
                }
                item->Extend(cap - a);
            }
            FinishSnapshotCapture();
            item->RunOwnerShare(); // help drain the frames still to be drawn
        } else {
            // RunOwnerShare returns as soon as no index is unclaimed, which is
            // the moment to try to grow the window: workers are still draining
            // the tail, so a successful Extend keeps them fed with no barrier in
            // between.
            while (true) {
                item->RunOwnerShare();
                if (cut.load(std::memory_order_relaxed) || !growWindow(false)) {
                    break;
                }
                item->Extend(e - a + 1);
            }
        }
        item.reset(); // closes the range and waits for the in-flight tail
        if (xldbgParallelWindows) {
            std::unique_lock<std::mutex> lock(parSlotLock);
            fprintf(stderr, "XL_PARALLEL_WINDOWS WINDOW m='%s' a=%d e=%d (asked %d) done=%d slots=%d\n",
                    rowToRender->GetModelName().c_str(), a, e, a + n - 1, doneCursor, (int)parSlots.size());
        }
        if (slotFailed.load(std::memory_order_relaxed)) {
            // A submodel lookup failed while building a clone - the model
            // changed under us.  Never try to parallelise this row again; the
            // caller resumes at the first unrendered frame and its top-of-frame
            // bail sets the dirty range.
            parEligible = false;
        }
        return a + doneCursor - 1;
    }

    // HOMOGENEOUS WINDOWS.  A window either carries a Snapshottable layer at
    // EVERY frame or at none - both window builders (here and the one in the
    // frame loop) refuse to extend across a change in a frame's snapshot-ness.
    // Why it matters in both directions:
    //   - Every frame of a capturing window costs a serial CaptureSnapshotFrame
    //     on the owner thread.  Letting one Snapshottable frame drag a long run
    //     of Pure frames into the window puts that cost on frames that would
    //     otherwise have had no serial component at all (measured: ~17% on
    //     Alice's Restaurant, whose Strobe effects sit in long Pure runs).
    //   - The capture advances the simulation on the REAL buffers and has to run
    //     from the window's first frame, so a window that did not start
    //     capturing must not grow into a frame that needs it.
    // It costs nothing: a Snapshottable effect covers a contiguous span, so the
    // boundary is where the window would have ended anyway.
    //
    // How far a RUNNING window may grow: the largest frame past e that keeps the
    // run of parallel-safe frames going, matches the window's kind, and is not
    // past what upstream has already delivered (the window is gated as a unit
    // before it starts, so an extension must not need a gate).
    int NextParallelWindowEnd(int e, bool wantSnapshot) {
        if (abort || origChangeCount != rowToRender->getChangeCount()) {
            return e;
        }
        // GetPreviousFrameDone is END_OF_RENDER_FRAME (INT_MAX) for a row with
        // no upstream, which is the case that gets the whole run in one window.
        const int cap = (int)std::min<long>((long)endFrame, (long)GetPreviousFrameDone());
        int e2 = e;
        while (e2 + 1 <= cap) {
            bool snap = false;
            if (!FrameIsParallelSafe(e2 + 1, snap) || snap != wantSnapshot) {
                break;
            }
            ++e2;
        }
        return e2;
    }


    // Where to stop growing a NEW window.  Long runs are the point - the pool
    // round-robins over rows at frame granularity, so one row holding a long
    // window no longer starves the others - but the window gates on upstream as
    // a unit before it starts, so growing past what upstream has delivered
    // means waiting for it.  Allow that only up to the classic chunk length,
    // which is exactly what the old fixed-chunk code waited for anyway.
    int ParWindowCap(int a) const {
        long cap = std::max<long>((long)a + parChunkFrames - 1, (long)GetPreviousFrameDone());
        return (int)std::min<long>(cap, (long)endFrame);
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

        // Frame-parallel eligibility (XL_PARALLEL_FRAMES).  Requires row-local
        // produce (no canvas-mix / canvas-Blend / per-model / per-node buffers);
        // submodel and strand effect rows are cloned per frame (CreateParSlot).
        // Per-frame Pure coverage - across main AND submodel layers - is checked
        // separately (FrameIsParallelSafe).
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
        // Item 03 step 3: rows carrying submodel/strand effect rows also qualify
        // - the clone slots replicate their buffers (CreateParSlot) and the
        // window produces/outputs every unit per frame.  Per-node buffers stay
        // excluded (per-node effect state lives on the job, not an
        // EffectLayerInfo, and the population is tiny).  Kill switch for
        // bisecting: XL_PARALLEL_SUBMODEL_ROWS=0.
        static const bool parSubmodelRows = []() {
            const char* e = getenv("XL_PARALLEL_SUBMODEL_ROWS");
            return e == nullptr || strtol(e, nullptr, 10) != 0;
        }();
        bool structurallyEligible = (isGroup || isBigModel)
            && !rowMustGateBeforeProduce
            && (subModelInfos.empty() || parSubmodelRows)
            && nodeBuffers.empty()
            && !ctorHasPerModelBuffers;
        parEligible = xldbgParallelFrames && structurallyEligible;
        parChunkFramesWanted = isGroup                 ? PAR_FRAME_GROUP_CHUNK
                               : !subModelInfos.empty() ? PAR_FRAME_SUBMODEL_CHUNK
                                                        : PAR_FRAME_MODEL_CHUNK;
        parChunkFrames = parChunkFramesWanted;

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
                        // streams FrameDone + currentFrame per frame; returns the
                        // last frame rendered (may run past e, or stop short)
                        resumeFrame = RenderParallelWindow(a, e, parWindowHasSnapshot) + 1;
                    }
                }
                while (!stopped && resumeFrame <= endFrame) {
                    // Frame-parallel fast path: render a contiguous run of frames
                    // whose every layer is Pure or Snapshottable in parallel clones.
                    // Snapshottable layers get a serial capture pre-pass first (see
                    // RenderParallelWindow).  The window gates once on upstream and
                    // streams FrameDone in frame order as frames complete, so
                    // downstream rows start before the window finishes.  Wins when
                    // the per-frame work is output/scatter-bound (serial per frame);
                    // effects whose internal parallel_for already saturates cores
                    // see no gain.
                    if (parEligible && !abort &&
                            origChangeCount == rowToRender->getChangeCount()) {
                        bool windowHasSnapshot = false;
                        if (FrameIsParallelSafe(resumeFrame, windowHasSnapshot)) {
                            int a = resumeFrame;
                            int cap = ParWindowCap(a);
                            int e = a;
                            while (e + 1 <= cap) {
                                bool frameSnap = false;
                                if (!FrameIsParallelSafe(e + 1, frameSnap) || frameSnap != windowHasSnapshot) {
                                    break; // homogeneous windows - see NextParallelWindowEnd
                                }
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
                                // Streams FrameDone + currentFrame per frame, and
                                // may render past e (it extends as upstream and
                                // the parallel-safe run allow) or stop short of it
                                // (abort / structural edit) - either way it
                                // returns the last frame actually rendered, and a
                                // short return lands on RenderFrame's top-of-frame
                                // bail, which sets the dirty range and stops.
                                resumeFrame = RenderParallelWindow(a, e, windowHasSnapshot) + 1;
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

public:
    // Approximate heap bytes this row's buffers hold, NOT counting clone slots.
    // This is also the price of one clone slot, since CreateParSlot replicates
    // exactly this shape (main buffer + one buffer per submodel/strand unit).
    uint64_t GetRowBufferMemoryBytes() const {
        uint64_t b = mainBuffer != nullptr ? mainBuffer->GetApproxMemoryBytes() : 0;
        for (const auto& smi : subModelInfos) {
            if (smi->buffer) {
                b += smi->buffer->GetApproxMemoryBytes();
            }
        }
        return b;
    }
    uint64_t GetNodeBufferMemoryBytes() const {
        uint64_t b = 0;
        for (const auto& nb : nodeBuffers) {
            if (nb.second) {
                b += nb.second->GetApproxMemoryBytes();
            }
        }
        return b;
    }
private:

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

    // Frame-parallel window state (see RenderParallelWindow).  parEligible and
    // parChunkFrames are computed once at InitializeRenderStates.  Clone slots
    // are built on demand (bounded by the pool's concurrency, NOT the window
    // length) and reused for the row's whole life.  parWindow* preserve a gated
    // window across an upstream suspend.
    bool parEligible = false;
    // ...Wanted is the tuned value for the row kind; parChunkFrames is what the
    // memory governor allows right now and is re-derived per window.
    int parChunkFramesWanted = PAR_FRAME_MAX_CHUNK;
    int parChunkFrames = PAR_FRAME_MAX_CHUNK;
    uint64_t parSlotCostBytes = 0;
    std::mutex parSlotLock;
    std::vector<std::unique_ptr<ParSlot>> parSlots;
    std::vector<ParSlot*> parFreeSlots;
    bool parWindowPending = false;   // a gated window is suspended awaiting upstream
    bool parWindowHasSnapshot = false;  // pending window has >=1 Snapshottable layer (needs the capture pre-pass)
    int parWindowStart = 0;
    int parWindowEnd = -1;

    // Telemetry for a RUNNING window, so a wedge inside one names the frames it
    // is stuck on.  A window holds the row's thread for its whole run, so the
    // job reports "holding a render thread" whether a frame is genuinely
    // rendering or the window will never finish - which is the one thing
    // GetHangStatus could not distinguish.  The scalars are atomics so the
    // per-frame updates never nest parWinLock inside the window's doneLock;
    // only the map needs a lock, and the diagnostic try_locks it.
    std::mutex parWinLock;
    std::atomic_bool parWinActive{ false };
    std::atomic_int parWinA{ 0 };
    std::atomic_int parWinE{ 0 };
    std::atomic_int parWinDone{ 0 };
    std::map<int, std::chrono::steady_clock::time_point> parWinInFlight; // frame -> claimed at

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

#ifdef __APPLE__
    // Precompute the largest size each video file is used at so the decoder can
    // emit pre-scaled frames (big cache-memory + scale savings). Apple-only: the
    // decode-time scaling is wired only for the AVFoundation reader, so on other
    // platforms (FFmpeg reader, which decodes native) this scan would be pure
    // overhead that nothing consumes. Run it on every render — full OR per-model
    // edit — always over the full model set, so a restricted render can't reuse a
    // stale entry and diverge from the next full render after a resize / crop /
    // effect change. Skip sequences with no video (the common case) so the scan
    // isn't paid on every dirty render while editing a non-video sequence.
    if (!seqElements.GetSequenceMedia().GetVideoFilePaths().empty()) {
        VideoEffect::PrepareDecodeSizes(seqElements, GetRenderTree().GetModels());
    }
#endif

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

    if (xldbgRenderMem) {
        std::vector<std::pair<uint64_t, std::string>> rowCost;
        uint64_t total = 0;
        for (size_t r = 0; r < (size_t)numRows; ++r) {
            if (jobs[r] == nullptr) {
                continue;
            }
            uint64_t b = jobs[r]->GetRowBufferMemoryBytes() + jobs[r]->GetNodeBufferMemoryBytes();
            total += b;
            rowCost.emplace_back(b, jobs[r]->GetName());
        }
        std::sort(rowCost.begin(), rowCost.end(), std::greater<>());
        RenderMemoryGovernor& gov = RenderMemoryGovernor::Get();
        fprintf(stderr, "XL_RENDER_MEM SETUP rows=%zu rowBuffers=%.1f MB seqData=%.1f MB footprint=%llu MB soft=%llu hard=%llu\n",
                rowCost.size(), total / (1024.0 * 1024.0),
                (double)seqData.NumFrames() * seqData.NumChannels() / (1024.0 * 1024.0),
                (unsigned long long)gov.FootprintMB(), (unsigned long long)gov.SoftLimitMB(),
                (unsigned long long)gov.HardLimitMB());
        for (size_t i = 0; i < rowCost.size() && i < 15; ++i) {
            fprintf(stderr, "XL_RENDER_MEM   row %6.1f MB  %s\n", rowCost[i].first / (1024.0 * 1024.0), rowCost[i].second.c_str());
        }
    }

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
                                    if (!effectObj->GetFrame(*rb, _renderCache, SettingsMap)) {
                                        // Serial advance+draw: a migrated Snapshottable
                                        // effect advances here, then Render draws the
                                        // returned snapshot - identical to the draw pass.
                                        auto snap = reff->AdvanceState(effectObj, SettingsMap, *rb);
                                        if (snap != nullptr) rb->pendingSnapshot = snap.get();
                                        reff->Render(effectObj, SettingsMap, *rb);
                                        rb->pendingSnapshot = nullptr;
                                        if (GPURenderUtils::HasPendingGPUWork(rb)) {
                                            // The effect rendered GPU-resident.  Caching it
                                            // means waitForRenderCompletion here to read the
                                            // pixels back, which drains + nils the command
                                            // buffer so the following blur/rotozoom/blend
                                            // can't append to it -- the GPU->CPU->GPU bounce
                                            // that broke the single-command-queue design for
                                            // every blurred layer.  A GPU re-render is cheap,
                                            // so skip storing this frame and let the layer
                                            // stay on one queue.  (CPU-resident effects fall
                                            // through: their wait is a no-op since there is no
                                            // queued GPU work, so caching them stays free --
                                            // and that is where the cache actually pays off.)
                                        } else {
                                            GPURenderUtils::waitForRenderCompletion(rb);
                                            effectObj->AddFrame(*rb, _renderCache);
                                        }
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
                        parallel_for(0, bufCnt, [&f](int x) {f(x); }, 1, &PerModelPool());
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

// How long a batch may go without ANY row advancing a frame before the log
// says so and names the outstanding rows.  Generous on purpose: one frame of a
// whole-house group on a slow box is seconds, not a minute, so a whole minute
// of nothing moving anywhere is pathological rather than merely slow.
static constexpr int STALL_REPORT_SECONDS = 60;

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
        // Frame progress on its own.  A batch whose frames have stopped moving
        // is worth reporting even when a job still holds a thread - that is
        // exactly the case the rescue below cannot see, and the case a user
        // experiences as a render that never finishes.  Report only; a job
        // inside an effect cannot be rescued, and guessing would be worse.
        if (sum != rpi->lastFrameSum || !anyUnfinished) {
            rpi->lastFrameSum = sum;
            rpi->lastFrameTime = now;
        } else if (std::chrono::duration_cast<std::chrono::seconds>(now - rpi->lastFrameTime).count() >= STALL_REPORT_SECONDS
                   && std::chrono::duration_cast<std::chrono::seconds>(now - rpi->lastStallReport).count() >= STALL_REPORT_SECONDS) {
            rpi->lastStallReport = now;
            spdlog::error("Render has made no frame progress for {}s.",
                          (long long)std::chrono::duration_cast<std::chrono::seconds>(now - rpi->lastFrameTime).count());
            LogUnfinishedRenderJobs("Stalled");
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

void RenderEngine::LogUnfinishedRenderJobs(const std::string& context) {
    // A wedged render usually has one or two rows outstanding; the cap only
    // exists so the first poll of a stuck full-sequence render doesn't dump
    // hundreds of lines every couple of seconds.
    constexpr int MAX_JOBS_LOGGED = 25;

    const auto now = std::chrono::steady_clock::now();
    const int numBatches = (int)_renderProgressInfo.size();
    int batch = 0;
    for (auto rpi : _renderProgressInfo) {
        ++batch;
        if (rpi->completed.load()) {
            spdlog::info("    {}: batch {}/{} has finished and is waiting to be drained.", context, batch, numBatches);
            continue;
        }
        // lastFrameTime, not lastProgressTime: the latter is the rescue timer and
        // resets while any job still holds a thread, so a permanently wedged
        // batch reported "no progress for 0s" - the opposite of the truth, and
        // the one number a reader trusts to tell slow from stuck.
        spdlog::info("    {}: batch {}/{}, {} of {} jobs outstanding, frames {}-{}, running for {}s, no frame progress for {}s.",
                     context, batch, numBatches, rpi->jobsRemaining.load(), rpi->totalJobs,
                     rpi->startFrame, rpi->endFrame,
                     (long long)std::chrono::duration_cast<std::chrono::seconds>(now - rpi->startTime).count(),
                     (long long)std::chrono::duration_cast<std::chrono::seconds>(now - rpi->lastFrameTime).count());

        int unfinished = 0;
        int logged = 0;
        for (int i = 0; i < rpi->numRows; ++i) {
            IRenderJobStatus* job = rpi->jobs[i];
            if (job == nullptr || job->IsFinished()) {
                continue;
            }
            ++unfinished;
            if (logged < MAX_JOBS_LOGGED) {
                ++logged;
                spdlog::info("        {}", job->GetHangStatus());
            }
        }
        if (unfinished > logged) {
            spdlog::info("        ... and {} more unfinished job(s).", unfinished - logged);
        }
        if (unfinished == 0) {
            // Every job took its terminal transition, so the batch is wedged
            // handing completion back to the caller, not inside an effect.
            spdlog::info("        No job is still running; the batch never signalled completion.");
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
    if (xldbgRenderMem) {
        RenderMemoryGovernor& gov = RenderMemoryGovernor::Get();
        fprintf(stderr, "XL_RENDER_MEM BATCH clones=%llu (%.1f MB) dropped=%llu peakFootprint=%llu MB soft=%llu MB\n",
                (unsigned long long)xldbgCloneCount.load(), xldbgCloneBytes.load() / (1024.0 * 1024.0),
                (unsigned long long)xldbgCloneDropped.load(),
                (unsigned long long)gov.PeakMB(), (unsigned long long)gov.SoftLimitMB());
    }

    bool expected = false;
    rpi->completed.compare_exchange_strong(expected, true);
}


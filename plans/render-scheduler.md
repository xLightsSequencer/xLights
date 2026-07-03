# Render Scheduler Redesign — Continuation-Based Render Scheduling

**Goal:** replace "one thread per model render job, blocking on dependent
models" with a small worker pool (~hardware cores) over resumable render jobs
that *suspend* instead of blocking. Threads only ever run jobs that have real
work to do.

Status of phases (update as work lands):

| Phase | Description | Status |
|---|---|---|
| 0 | Fix `waitForFrame` polling → predicate CV wait | ✅ done |
| 1 | Hoist `RenderJob::Process()` locals into members; extract per-frame slice | ✅ done |
| 2 | Collapse mid-frame waits into a single frame-entry gate (still blocking) | ✅ done |
| 3 | Gate → suspend/requeue; row ownership token; END-as-state; shrink pools | ✅ done |
| 4 | Remove oversubscription config; iPad sizing/comment cleanup; telemetry | ✅ done |

All phases complete. Pool sizing is GPU-aware (cpu + gpu + slack — see §4.4);
each batch logs jobs/suspensions/row-parks/elapsed on completion. Remaining
follow-on (not scheduled): lazy PixelBuffer allocation at a job's first slice,
released at Done, to cut peak render memory (§3).

Post-review hardening (2026-07): per-frame change-count bail extended to
downstream-having jobs + `ProcessFrame` layer-count clamp (structural edits
during a suspension can no longer index stale per-layer state — the risk §5.1
flagged); `~RenderJob` abandons row ownership (no dangling `ModelElement`
pointers on forced teardown); `previousFrameDone` updates are monotonic (a
stale lower-frame relay can't re-strand a job waiting on END); the stall
watchdog is per-batch (no longer masked by other batches keeping the pool
busy); `AbortRender` cancels parked jobs directly and aborted jobs short-cut
at Setup; the frame-entry gate uses an advancing per-layer effect cursor
instead of rescanning each layer's effect list every frame.

All of this is `src-core/` — desktop and iPad share it (parity auto-applied).

---

## 1. Problem statement

Each model row with effects is one `RenderJob`; `Process()` renders the whole
frame range on one pool thread and **blocks** whenever it needs a frame that an
overlapping (upstream) model hasn't produced yet. To keep progress under all
that blocking, the pools massively oversubscribe:

- Desktop: `cores × 10` (× 20 with >12 GB RAM), min 20, cap 250
  (`src-ui-wx/xLightsMain.cpp` around `jobPool.Start`).
- iPad: `max(24, 2 × hardware_concurrency)`
  (`src-iPad/Bridge/iPadRenderContext.cpp`, `EnsureRenderEngine`).
- Each Apple worker gets an explicit 1 MB stack (`JobPool.cpp`,
  `JobPoolWorker` ctor).

On a 200-model sequence, hundreds of threads exist and most sleep. Worse, the
"sleep" is a 100 Hz poll (see §2), so blocked threads burn CPU/battery. On
memory-constrained devices (iPad) the per-thread cost and the stalled pool
slots both hurt.

## 2. Current architecture (facts, `src-core/render/RenderEngine.cpp`)

- **Job per row.** `RenderEngine::Render()` builds one `RenderJob` per model
  row that has effects. `RenderJob::Process()` runs the entire
  startFrame→endFrame loop on one thread.
- **Dependency graph.** Built statically per `Render()` batch from channel
  overlap: each `RenderJob` is a `NextRenderer`; fan-in goes through an
  `AggregatorRenderer` (needs all N upstreams to finish frame F before
  relaying). Links (`addNext`) are wired before any job is pushed and never
  mutate during the render. Upstream is always an earlier row.
- **The wait is a poll.** `NextRenderer::waitForFrame` loops on
  `wait_for(lock, 10ms)` because `setPreviousFrameDone` writes
  `previousFrameDone` and notifies **without** holding `nextLock` — the 10 ms
  timeout papers over the missed-wakeup race. 150 blocked threads ≈ 15,000
  wakeups/sec during a render.
- **Wait points are mid-frame** (all funnel through `maybeWaitForFrame` /
  `waitForFrame`):
  1. canvas-mix preload in `ProcessFrame` — *inside* an effect-layer lock
     scope;
  2. before `CalcOutput`/seqData write when the frame produced output;
  3. before submodel processing; before node-buffer processing;
  4. `waitForFrame(END_OF_RENDER_FRAME)` at exit, so an upstream can't poke a
     deleted job (lifetime handshake).
- **Why it doesn't deadlock today:** jobs are pushed in two passes —
  zero-dependency jobs first, then dependents in row order — and the pool
  dequeues FIFO. A running job's upstreams are therefore always running or
  done, so someone always makes progress, whatever the pool size. **Any new
  design must preserve an equivalent progress guarantee.** Corollary: once the
  pool shrinks to ~cores, *no* dependency wait may block a thread — "all
  threads blocked on jobs still in the queue" becomes reachable otherwise.
- **Row render lock.** `Process()` holds `ModelElement::changeLock`
  (`recursive_timed_mutex`, via `GetRenderLock()`) for the row's **entire**
  render. Structural layer edits (`Element::AddEffectLayerInternal`,
  `InsertEffectLayer`, `RemoveEffectLayer`) `try_lock_for(500ms)` and abort
  the whole render on failure. Duplicate renders of a row serialize by
  blocking a pool thread on this mutex + the `IncWaitCount`/`DecWaitCount`
  early-bail dance. A mutex is thread-affine → cannot be held across a
  suspension that resumes on another thread. This is the piece that must be
  *redesigned*, not just refactored.
- **Already decoupled from threads (good news):** jobs live in
  `RenderProgressInfo` until the whole batch completes
  (`RenderUI.cpp::UpdateRenderStatus` deletes them after `completed` flips);
  the UI polls `IRenderJobStatus` (`GetCurrentFrame` etc.); completion is
  `jobsRemaining`/`completed` atomics drained on the main thread (desktop
  timer, iPad poll). The scheduler can change underneath without touching the
  progress dialog or the iPad bridge.

## 3. Cost analysis

- Threads: desktop up to 250, iPad ~24. Savings from shrinking to ~cores:
  1 MB stack each (virtual; real = touched pages, ~100–300 KB once a render
  ran on it), kernel objects, TLS, context-switch pressure — roughly tens of
  MB RSS on desktop, ~5–15 MB on iPad.
- CPU/battery: the 100 Hz poll per blocked thread goes away entirely (Phase 0
  fixes this independently of the rest).
- **Honest caveat:** the dominant render memory — every job's
  `PixelBufferClass` for every model/group/submodel, all allocated in the
  `RenderJob` ctor up front — is *unchanged* by this work. Follow-on enabled
  by the new scheduler: allocate buffers at a job's first slice, free at DONE
  (cuts peak when leaf models finish long before big groups).

## 4. Target design

### 4.1 Suspend at frame boundaries only

Restructure `Process()` into a resumable slice runner. At the top of each
frame, one gate: *"has my upstream finished frame N?"*
(`previousFrameDone >= N`).

- Gate satisfied → render the whole frame with **no** waits. The gate is
  strictly stronger than all four mid-frame waits, so they simply disappear.
- Gate not satisfied → register a continuation and return the thread to the
  pool.

Mid-frame suspension is ruled out deliberately: at those points the stack
holds an `AutoReleasePool` (ObjC, thread-affine), recursive effect-layer
locks, and frame-local state — exactly what breaks when resuming on a
different thread. Cost of the stricter gate: ~1 frame of upstream/downstream
pipelining per edge — noise when the pool is saturated with runnable work.

Keep today's sparse-effect sprint: a job only waits on frames where it
actually writes, so a group with effects only at the end races through the
rest. Preserve by applying the gate only on frames with effect coverage on
the row (computable from effect layers at slice start).

### 4.2 Suspension mechanics

Each job already *is* its own wait-state holder (its `previousFrameDone` is
fed by its single upstream job or aggregator), so continuations live on the
job itself:

- Job scheduling state: `QUEUED → RUNNING → (SUSPENDED(wantFrame) | AWAITING_END | DONE)`.
- `setPreviousFrameDone` takes `nextLock`, updates the frame, and if the job
  is `SUSPENDED` with `wantFrame <= done`, transitions it to `QUEUED` and
  pushes it to the pool. Suspending does the mirror under the same lock:
  re-check the frame; if it advanced, don't suspend — keep running. One lock,
  no missed-wakeup race, no double-queue.
- Batching: on resume, run every frame up to the upstream's current done
  frame before re-checking (existing `maxFrameBeforeCheck` logic maps over
  directly). Worst case is a per-frame requeue ping-pong — a queue op is
  microseconds against multi-ms frames.
- **END handshake becomes a state.** A job that finishes its frames enters
  `AWAITING_END`; when upstream delivers `END_OF_RENDER_FRAME` it resumes,
  propagates END downstream, then transitions to `DONE`. The
  `FinishNotifier`/`jobsRemaining` decrement moves from "Process() returned"
  to the DONE transition (critical: `Process()` now returns many times per
  job). Lifetime stays safe: jobs are only deleted after the whole batch
  completes, and all `NextRenderer` links are batch-internal.

### 4.3 Row ownership instead of the render lock

`ModelElement` gets an atomic *active render job* token plus a *pending job*
slot:

- A new job for a busy row parks in the pending slot instead of blocking a
  thread on the mutex. The old job already checks `GetWaitCount()` per frame
  and bails; its DONE transition schedules the pending job.
- `changeLock` shrinks back to guarding short structural ops, acquired
  per-slice by the engine rather than per-render. Side benefit: an
  `AddEffectLayer` during a render no longer stalls 500 ms and aborts the
  whole render — the edit slips between slices and dirty-range machinery
  re-renders.

### 4.4 Abort, watchdog, pool sizing

- `SignalAbort` additionally requeues every suspended job; each runs its
  existing bail path (`SetDirtyRange`, notify, DONE) immediately. Abort
  convergence gets faster and more reliable (relevant to the
  AbortRender-before-seqData-resize crash work).
- Watchdog in the existing render-status timer: batch hasn't progressed in N
  seconds while the pool is idle → log loudly, requeue all suspended jobs. A
  lost continuation must show up in logs, not as a silent hang on save/close.
- Pools drop to ~(CPU cores + GPU cores + small slack) everywhere. Many
  effects render on the GPU and park their thread in
  `waitForRenderCompletion`, freeing the CPU core — so the pool carries one
  extra thread per effect render the GPU can have in flight
  (`GPURenderUtils::GetGPUEffectConcurrency()`: IORegistry `gpu-core-count`
  on Apple Silicon macOS; CPU-count proxy on iOS/other Metal devices; 0 with
  no GPU backend). Remaining in-slice blocking (ShaderEffect's serial
  GL/ANGLE thread, render-cache I/O, nested `parallel_for` on the separate
  per-model pool) is bounded resource contention, not dependency waiting —
  safe with a small pool.

### 4.5 What doesn't change

Progress dialog and iPad polling (jobs stay alive and pollable in every
state; status 13 "Waiting to start frame" becomes "suspended, waiting for
\<upstream\>"), `RenderProgressInfo`, `completed`-flag drain, effects,
`PixelBuffer`, `JobPool` itself (re-push already supported).

## 5. Risks / audit list

1. **Effect/layer validity across suspension windows.** Today the full-render
   `changeLock` excludes structural layer mutation during a row's render.
   Per-slice locking opens windows: cached `numLayers`, `EffectLayerInfo`
   sizing, cached `Effect*` in `currentEffects[]` (pointer-compared across
   frames — dangling-compare ABA is survivable but re-validate on
   change-count movement at slice start). The per-frame bail check exists but
   only for `!HasNext()` jobs; jobs with downstreams need a defined bail
   story (send END, let the dirty-range re-render of the overlap chain fix
   it — `RenderEffectForModel` already re-renders the whole `renderOrder`).
2. **Determinism is the test oracle.** Frame output must be byte-identical
   before/after each phase ≥2 (gate is stricter than today's waits; blend
   order preserved). Validate: render real sequences with render cache
   disabled, byte-compare `.fseq`. Plus stress: edit-during-render,
   abort-during-render, close-during-render, batch render; Instruments on
   iPad for thread/memory counts.
3. **Debuggability.** Explicit member-state machine keeps native stacks
   readable for the crash-report pipeline. This is why C++20 coroutines and
   fibers were rejected (below).

## 6. Rejected alternatives

- **C++20 coroutines** (`co_await` at existing wait points): preserves code
  shape, but suspension points sit under thread-affine state (autorelease
  pools, recursive layer locks), and coroutine frames wreck crash-report
  stack readability. The frame-boundary state machine gets ~95% of the
  benefit with explicit state.
- **Fibers / boost.context:** keeps blocking code as-is, but per-job fiber
  stacks shrink the win, iOS support is fragile (ucontext deprecated), and
  crash handlers/ASAN get confused.
- **Per-frame task granularity** (frame N of all models as a task DAG):
  effects are stateful across frames within a model (buffers, effect state,
  render cache), so per-model sequencing is mandatory anyway; this adds
  scheduling overhead without removing the constraint.
- **Do nothing but fix the poll:** Phase 0 alone kills the wakeup storm but
  keeps hundreds of blocked threads holding pool slots.

## 7. Phases

- **Phase 0 — CV fix (independent quick win).** `setPreviousFrameDone`
  updates `previousFrameDone` under `nextLock` and notifies;
  `waitForFrame` uses a predicate wait. No behavior change other than wakeup
  mechanics. Ship-worthy alone.
- **Phase 1 — pure refactor.** Hoist `Process()` locals (`mainModelInfo`,
  node-effect maps, `origChangeCount`) into `RenderJob` members; extract
  `ComputeRenderRange()`, `InitializeRenderStates()`, `RenderFrame(frame)`
  (one full frame incl. submodels/nodes/FrameDone), `FinishRender()` (END
  handshake + completion callbacks). Still one blocking loop; diff
  reviewable; behavior identical.
- **Phase 2 — single gate.** Replace the four mid-frame waits with the
  frame-entry gate (still blocking). Only phase that can change output —
  run the `.fseq` byte-compare here.
- **Phase 3 — suspension.** Gate → suspend/requeue; render lock → ownership
  token (§4.3); END-as-state; abort requeue; watchdog; shrink pools to
  ~cores.
- **Phase 4 — cleanup.** Delete oversubscription multipliers
  (`xLightsMain.cpp`), update iPad `EnsureRenderEngine` sizing + comment,
  remove stale "oversubscribe or deadlock" comments, telemetry/logging of
  scheduler stats.

Scope estimate: ~80% inside `RenderEngine.cpp`
(`RenderJob`/`NextRenderer`/`AggregatorRenderer`), ~50 lines in
`Element.h/.cpp` for token ownership, a few lines each in `xLightsMain.cpp`
and `iPadRenderContext.cpp` for pool sizing. `JobPool` needs essentially
nothing.

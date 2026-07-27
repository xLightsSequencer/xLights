# Render Benchmarking & Profiling Guide

How to measure, profile and validate xLights render performance. Written for
developers and AI agents doing render work: everything here is copy-pasteable,
and every flag listed was read out of the source, not remembered.

If you change render code, **§2 (byte-identity) is not optional** — it is the
gate that makes a performance result meaningful. A faster render that changes
output is not a faster render.

Commands use `$SHOW`, `$OUT` and `$SEQS` — set them for your own show and
scratch dir (§2). Where a measurement is quoted, it comes from the **reference
corpus**: a 56-sequence residential Christmas show on a 16-core Apple M4 Max.
Those numbers are evidence for the claim next to them, not something to
reproduce — your show and machine will differ. §2 explains how to pick an
equivalent regression set from your own sequences.

---

## 1. Headless rendering — the measurement harness

`--headless` renders `.xsq` sequences to `.fseq` with no window and exits. It is
driven by `HeadlessRenderContext` (`src-core/render/`), a concrete wx-free
`xLightsShowContext` — the same show-state base `xLightsFrame` and the iPad
derive from — so it exercises the real render engine, effects and the shared
`LoadSequenceElements` open path. Nothing is stubbed.

```bash
xLights --headless -s <showdir> <seq.xsq> [<seq2.xsq> ...]
xLights --headless -s <showdir> "<showdir>/*.xsq"        # quote it; the app globs
xLights --headless -s <showdir> --outputdir <dir> "<showdir>/*.xsq"
```

Exit codes: `0` success · `1` render/write error · `2` bad args.

Per sequence it prints the render time to **stdout** as
`<outpath>     Updated in %7.3f seconds` (and the same figure to the log as
`--headless: wrote <path> in N seconds`). The stdout line is the one to parse —
it is a plain `printf`, unaffected by log sink configuration.

### Relevant CLI switches

| Switch | Long form | Meaning |
|---|---|---|
| `-hl` | `--headless` | render to fseq, no window, exit |
| `-fc` | `--fseqcmp` | compare two `.fseq` channel-for-channel, exit 0 iff identical |
| `-r` | `--render` | desktop render-and-exit (full GUI stack) |
| `-s` | `--show` | show directory (**required** for headless) |
| `-od` | `--outputdir` | output dir; applies to `-r` and `--headless`. Default: the show's configured fseq folder |
| `-m` | `--media` | media directory |
| `-cs` | `--checksequence` | run check-sequence and exit |

### Hard-won gotchas

- **Use Release.** `xcodebuild` with no `-configuration`. Debug is ~5–10× slower
  and skews every ratio.
- **Absolute paths.** Relative `.xsq` arguments silently no-op — exit 0, no fseq,
  no error. This has burned multiple sessions.
- **Keep the show and the output dir on local disk.** Both the `.xsq` read and
  the fseq write are inside the reported time. A network/FUSE mount (Parallels
  `psf`, SMB, NFS) costs a few percent on every measurement, and worse, some of
  that cost is served by resources outside the machine under test — which
  quietly biases a core-count or CPU-scaling study.
- **macOS sandbox.** The binary can only read paths it holds a security-scoped
  bookmark for (dirs opened in the GUI at some point) — **not** `/private/tmp`.
  Stage comparison fseqs somewhere the GUI has opened at least once - under the
  show dir, or a scratch dir you first pick in a file dialog. Pick one scratch
  dir and use it for everything (`$OUT` throughout this doc).
- **`-r` overwrites** fseqs in the configured folder. Use `--outputdir`.
- **Headless disables the render cache** (`HeadlessRenderContext.cpp`) — a
  one-shot render has nothing to reuse. `XL_HEADLESS_RENDERCACHE=1` opts back in.
  Numbers are therefore not comparable to pre-2026-07-24 baselines.

---

## 2. Byte-identity — the correctness gate

Almost all render performance work is required to be **output-invariant**. The
tool is `--fseqcmp`:

```bash
xLights --fseqcmp <a.fseq> <b.fseq>               # exit 0 iff identical
xLights --fseqcmp -s <showdir> <a.fseq> <b.fseq>  # + per-model diffs + frame-offset probe
```

Never `md5`/`cmp` two fseqs — the header carries a per-run `uniqueId`, so
byte-comparison of the files always differs. `--fseqcmp` compares channel data.

### The standard gate for a render change

Set these once; every recipe in this doc uses them.

```bash
BIN=macOS/build/Release/xLights.app/Contents/MacOS/xLights
SHOW=/path/to/your/show          # dir containing the .xsq files
OUT=/path/to/scratch             # must be GUI-bookmarked on macOS, see §1
SEQS=(SeqA SeqB SeqC SeqD)       # your regression set - see below
```

```bash
mkdir -p "$OUT"/{ser,par}
for s in "${SEQS[@]}"; do
  XL_NO_PARALLEL_FRAMES=1 "$BIN" --headless -s "$SHOW" -od "$OUT/ser" "$SHOW/$s.xsq" >/dev/null 2>&1
  "$BIN" --headless -s "$SHOW" -od "$OUT/par" "$SHOW/$s.xsq" >/dev/null 2>&1
  printf "%-32s " "$s"
  "$BIN" --fseqcmp -s "$SHOW" "$OUT/ser/$s.fseq" "$OUT/par/$s.fseq" >/dev/null 2>&1 \
    && echo IDENTICAL || echo "**DIFF**"
done
```

### Choosing your regression set

**Do not copy someone else's sequence names** — pick from your own show. What
matters is coverage of the five places frame-parallel bugs have actually lived.
Every one of the historical bugs needed a sequence with a specific property; a
set that misses a property will pass while broken.

| Must cover | Why | How to find one in your show |
|---|---|---|
| A large **group** row (whole-house or near) | The prime frame-parallel target; the output/scatter-bound case | `XL_PARALLEL_WINDOWS=1`, look for `[GROUP]` rows with big frame counts (§5) |
| A **multi-layer group** with a Snapshottable layer | The capture/draw split; multi-layer was where it broke | census below, cross-referenced with a group row |
| **Sparkles** on a large model | A stateful blend counter diverged per-clone; a *small* buffer masks it (it takes the ISPC path, while large buffers take Metal) | `grep -l 'SparkleFrequency\|MusicSparkles'`, prefer the biggest model |
| **Model blending** | Blend reads `seqData[frame]`, so ordering bugs surface here | `grep -l 'ModelBlending="true"'` |
| **Buffer continuity** (`OverlayBkg` / Freeze / Suppress) | A persistent effect relies on the previous frame having cleared the *serial* buffer | `grep -l 'OverlayBkg'` |

```bash
# effect census for one sequence, most-used first
grep -oE '<Effect [^>]*name="[^"]+"' "$SHOW/Seq.xsq" \
  | sed -E 's/.*name="([^"]+)".*/\1/' | sort | uniq -c | sort -rn | head -20

# rank sequences by Snapshottable-effect count (the capture path)
for f in "$SHOW"/*.xsq; do
  n=$(grep -oE '<Effect [^>]*name="(Strobe|Snowstorm|Fireworks|Meteors|VU Meter|Wave|SnowFlakes)"' "$f" | wc -l)
  [ "$n" -gt 0 ] && echo "$n ${f##*/}"
done | sort -rn | head

# sequences with sparkles / model blending / buffer continuity
grep -l 'SparkleFrequency\|MusicSparkles' "$SHOW"/*.xsq
grep -l 'ModelBlending="true"' "$SHOW"/*.xsq
grep -l 'OverlayBkg' "$SHOW"/*.xsq
```

Then favour **long** sequences (more frames = more chances to diverge) and ones
whose render time is a large share of process wall (§3). Four to seven is a
practical set; the whole corpus is the release gate, not the edit-loop gate.

### Same-binary diff — the core technique

Build ONE Release binary and run it both ways; compare with `--fseqcmp`. One
binary ⇒ build config is not a confound, and run-1-vs-run-2 of the same config is
your noise floor. A real bug shows as `variantA-vs-variantB` ≫ `run-to-run`.

### Expected (non-bug) differences

- **GPU vs CPU**: on the reference corpus 46 of 56 sequences differ between GPU
  and CPU compute. Legitimate float/shader path differences. Compare a GPU run
  only against a GPU baseline. **A GPU-vs-CPU diff proves nothing.**
- **Physics effects** (LiquidFun/Box2D).
- Everything else, including **Video**, **Shader** and **random/sparkle**
  effects, is deterministic. Video decode variance was a real bug (dropped frames
  + inconsistent frame selection in the AVFoundation bridge), fixed; sparkles are
  a pure function of (model, node, frame). If either regresses, that is a bug.
- **Shader/`DATE`** was a third such bug, fixed: the ISF `DATE` uniform was
  packed from the wall clock, so two runs seconds apart diverged from the first
  frame of any shader reading it — and a feedback shader then stayed perturbed
  for its whole span. One such shader in a 300-shader corpus was enough to make
  the entire sequence fail byte-identity on Metal *and* Vulkan. `DATE` now comes
  from the sequence timeline. The lesson generalises: **before concluding a
  change altered output, run the same config twice.** A single wall-clock-derived
  uniform is indistinguishable from a synchronisation bug if you only ever
  compare variant A against variant B.

---

## 3. Timing methodology — read this before quoting a number

The single biggest source of wrong conclusions in this codebase's performance
history is measuring wall-clock on a busy machine.

### Three metrics, three uses

| Metric | Where from | Use for |
|---|---|---|
| **Render-only time** | the binary's own `<path>     Updated in N seconds` line on **stdout** | the primary number. Spans `OpenSequence` → `RenderAndWait` → `WriteFseq` — see the scope note below |
| **User+sys CPU** | `/usr/bin/time -p` | detecting contention/regressions. Far less scheduling-sensitive than wall |
| **Process wall** | `/usr/bin/time -p` real | only when the render actually dominates the process |

**Know what the render-only number covers.** It is not the render alone: the
timer starts before the sequence is opened and stops after the fseq is written,
so the `.xsq` parse, its media load, and the fseq write are all inside it. What
it excludes is process start, the show-folder load (models, networks,
`xlights_rgbeffects.xml` — substantial on a large show) and teardown. Two
consequences: putting the show or the output dir on a slow filesystem moves cost
*into* the number you quote (measured: +3.7% just for reading the `.xsq` over a
Parallels psf/FUSE mount), and a change to sequence I/O will show up here even
though it changed no rendering.

**Process wall is often the wrong metric.** Anything short enough that sequence
load, audio decode and fseq save dominate will pin process wall regardless of
what the render does — on one reference sequence, wall sat at ~15.3s around a
~5.5s render, so a whole-process A/B reported "no change" through both a 17%
regression and its fix. Compare the two numbers on your own sequences before
trusting wall; if render-only is not most of wall, quote render-only.

Conversely, render-only time can *include waiting* (on upstream rows, on media),
so a change in it is not automatically a change in work done. When render-only
and CPU disagree, CPU is telling you about work and render-only about latency —
report both.

### Rules

1. **Idle machine.** `uptime` 1-minute load < 2 before starting. A concurrent
   build has been observed to spike load to 125 and make numbers unusable.
   Daytime numbers on a shared machine are typically worthless for anything
   under ~10%.
2. **Alternate A/B/A/B**, never all-A-then-all-B. Thermal and background drift
   are monotonic over minutes; interleaving cancels them.
3. **≥3 reps**, report the spread. On the reference machine, whole-run wall
   variance ran ±20% while render-only variance stayed under 1% — measure your
   own noise floor once (same binary twice) and treat anything smaller than it as
   no result.
4. **Build both binaries from the same tree state**, save them aside:
   ```bash
   SP=/tmp/bench            # or your scratchpad
   xcodebuild && cp -a macOS/build/Release/xLights.app $SP/xLights-new.app
   git stash push -u && xcodebuild && cp -a macOS/build/Release/xLights.app $SP/xLights-old.app
   git stash pop && xcodebuild   # restore
   ```
   `git stash -u` (untracked included) matters if the change adds files.
5. **Prefer deterministic gates over timing** where one exists: byte-identity and
   `XL_PARALLEL_BLOCKERS` coverage are exact and machine-state-independent.
   Coverage ≠ wall time, though — see §5.

### Harness

```bash
run() {  # $1=app bundle  $2=sequence name (no .xsq)
  local o=$( { /usr/bin/time -p "$1/Contents/MacOS/xLights" --headless \
      -s "$SHOW" -od "$OUT" "$SHOW/$2.xsq"; } 2>&1 )
  echo "$o" | awk '/Updated in/{for(i=1;i<=NF;i++) if($i=="in") rt=$(i+1)}
                   /^real/{r=$2} /^user/{u=$2} /^sys/{s=$2}
                   END{printf "render=%-8s wall=%-7s user=%-7s sys=%s", rt, r, u, s}'
}
for rep in 1 2 3; do
  echo "old rep$rep $(run $SP/xLights-old.app "${SEQS[1]}")"
  echo "new rep$rep $(run $SP/xLights-new.app "${SEQS[1]}")"
done
```

### Baselines

Keep corpus baselines outside the repo, one directory per capture
(`<scratch>/baseline_<date>/`), each with a `BASELINE.md` recording commit,
build config, machine, per-sequence times, determinism result and GPU-vs-CPU
comparison. If you are creating one, record all of that — a baseline without its
provenance is not usable six weeks later. Keep a GPU and a CPU-only baseline as a
pair and never cross-compare them.

---

## 4. Low-level effect timings — `XL_RENDER_PROFILE`

`XL_RENDER_PROFILE=1` accumulates per-row and per-effect timing and dumps two
tables to **stderr** when the batch completes. Zero cost when unset (the clock is
never read; see `StageTimer` in `src-core/render/RenderProfile.h`).

```bash
XL_RENDER_PROFILE=1 xLights --headless -s "$SHOW" -od /tmp/out "$SHOW/Seq.xsq" 2> prof.txt
```

### Table 1 — per row (model)

Columns: `model frames slices effect gpu blurZ trans blend getCol setCol gpuWait
suspend wall %gpu %sus` + top-3 effects by cpu+gpu. All times in ms.

| Column | Meaning |
|---|---|
| `effect` | CPU time in effect `Render()`. **For a GPU effect this is only the dispatch encode** |
| `gpu` | GPU execution window of the command buffers that effect opened |
| `blurZ` / `trans` | `HandleLayerBlurZoom` / `HandleLayerTransitions` |
| `blend` | `CalcOutput` |
| `getCol` | per-node scatter of final colors into seqData |
| `setCol` | reverse gather — preload of seqData back into a layer for "blend with existing" rows |
| `gpuWait` | parked in `GPURenderUtils::waitForRenderCompletion` |
| `suspend` | suspended awaiting an upstream frame |
| `wall` | this row's own timeline (`slice + suspended`) |

### Table 2 — per effect, all rows, ranked by cpu+gpu

Columns: `effect renders cpu-ms gpu-ms gpu-cbs cpu+gpu-ms`. This is the table to
pick optimisation targets from.

### Reading it correctly

- **Rank by `cpu+gpu`, never by `effect` alone.** A GPU effect's `effect` column
  is dispatch encode only and under-reports by up to 100×. The dump prints a
  loud warning if GPU is on but no GPU time could be attributed.
- **`gpu` totals can exceed the batch wall.** Command buffers from different rows
  overlap on the GPU. Use it to *rank*, not as a budget.
- **Per-effect totals under-report whenever frame-parallel is engaged.**
  `perEffectNs`/`perEffectCount` are only written by the thread carrying
  `tlsRenderProfile`, which `BeginSliceProfile` sets on the job's slice-owning
  thread alone — so effect renders that run on frame-pool workers (or nested
  `parallel_for` workers) are not counted at all. A sequence showing 58106 Text
  renders serially showed ~25000 with frame-parallel on, for identical output:
  the difference is accounting, not work. Consequences: never compare a
  serial per-effect total against a parallel one; when comparing two parallel
  runs, normalise to **ms per render** rather than the raw total, since the
  owner's share of frames varies between builds. The row-level stage timers are
  inclusive of worker time and do not have this problem.
- Rows named `(gpu blur)`, `(gpu rotozoom)`, `(gpu transition)`, `(gpu blend)`,
  `(gpu map)` are stage work no effect owns.
- If some GPU time ran on a command buffer shared between an effect and a later
  blur/rotozoom/transition stage, the dump says how much; it is charged to the
  effect and cannot be split out.
- High `suspend` across many rows means the render is **dependency-bound**, not
  compute-bound. Optimising effect kernels will not help; look at the scheduler
  (`plans/render-scheduler.md`).

### Sanity check on utilisation

```bash
# cores actually busy = (user+sys)/wall
/usr/bin/time -p xLights --headless -s "$SHOW" -od /tmp/out "$SHOW/Seq.xsq" 2>&1 \
  | awk '/^real/{r=$2}/^user/{u=$2}/^sys/{s=$2} END{printf "%.2f cores of %d\n",(u+s)/r,'"$(sysctl -n hw.ncpu)"'}'
```
On the reference corpus this typically lands at 4–6 of 16 — most sequences are
**not** core-bound, so a change that only improves intra-row parallelism measures
as neutral there. Run it on your own sequences first: which regime you are in
decides whether an optimisation can possibly show up.

---

## 5. Frame-parallel telemetry

Frame-parallel rendering renders several frames of one row concurrently on clone
buffers when every covering effect is `Pure` or `Snapshottable`. Default ON.

### `XL_PARALLEL_WINDOWS=1`

Per-row window analysis, plus a line per actual window:

```
XL_PARALLEL_WINDOWS WINDOW m='MegaTree' a=0 e=3015 (asked 3015) done=3016 slots=8
XL_PARALLEL_WINDOWS SNAP-WINDOW m='WH-NoTree' a=0 e=2951 layers=9
XL_PARALLEL_WINDOWS SUB-WINDOW m='Bushes' a=0 e=23 units=4 snap=0
```

- `a`/`e` — first/last frame of the window as it finished (may exceed `asked`
  because windows grow as upstream delivers)
- `done` — frames completed contiguously
- `slots` — the row's realised frame concurrency, and therefore its clone-buffer
  count. Bounded by that row kind's `parChunkFrames` (8 group / 24 model / 8
  submodel) **and** by the frame pool's worker count + 1 — so on a 16-thread box
  a model row tops out at 16, not 24. A value above the row's `parChunkFrames`
  is a bug.

Summarise:
```bash
XL_PARALLEL_WINDOWS=1 xLights --headless -s "$SHOW" -od /tmp/out "$SHOW/Seq.xsq" 2>&1 \
 | grep "^XL_PARALLEL_WINDOWS WINDOW" \
 | awk '{for(i=1;i<=NF;i++){if($i~/^m=/)m=$i;if($i~/^a=/){split($i,A,"=");a=A[2]}
         if($i~/^e=/){split($i,E,"=");e=E[2]};if($i~/^slots=/){split($i,S,"=");s=S[2]}}
         L=e-a+1;c[m]++;t[m]+=L;if(L>x[m])x[m]=L;if(s>k[m])k[m]=s}
   END{for(q in c)printf "%-24s windows=%-6d frames=%-7d maxlen=%-6d maxslots=%d\n",q,c[q],t[q],x[q],k[q]}' \
 | sort
```

Note the model name is whitespace-split, so a name containing a space is
truncated at it (`m='Garage Outline'` prints as `m='Garage`). Rows still
aggregate correctly as long as the first word is unique; if it is not, match on
`m='[^']*'` with a regex instead.

### `XL_PARALLEL_BLOCKERS=1`

Profile-only, deterministic. Walks every frame of every structurally-eligible row
and attributes blocked frames to the effect(s) that vetoed parallelism, then
dumps a ranked table at process exit. This is how you decide **which effect to
convert next** — it reordered a purely theoretical guess the first time it was
run. Distinguishes:

- inherently `Stateful` effects (the conversion targets), ranked by *sole*-blocker
  frames (converting it unlocks the frame immediately) vs *any*-blocker
- effects vetoed by a buffer-continuity **setting** (`OverlayBkg` / `Freeze` /
  `Suppress`) — not conversion targets
- `frame+1` continuity boundaries
- structurally ineligible rows, by reason

**Coverage is not wall time.** VU Meter's Snapshottable conversion took coverage
from 82% to 90% and was wall-time *flat*, because those sequences were not
output-stage-bound. Use coverage as a deterministic progress metric and time
separately.

### `XL_VERIFY_STATELESS=1`

For every effect declaring itself `Pure`, re-renders each frame from a cleared
`infoCache` and warns if the pixels differ. A mismatch means the effect secretly
carries cross-frame state and its `GetFrameParallelism()` override is wrong. Same
buffer and same GPU/CPU path both times, so there is no confound. **Run this
after any `GetFrameParallelism` change** — it has caught a real misclassification
(Fire marked Pure).

---

## 6. Determinism bisection

When two runs differ and they should not, work down this list. All are free when
unset and available in every build.

| Variable | Effect |
|---|---|
| `XL_SERIAL=1` | every `parallel_for` runs serially on the calling thread — isolates CPU thread-order races |
| `XL_SERIAL_PERMODEL=1` | per-model buffers render serially — isolates that pool specifically |
| `XL_NO_PARALLEL_FRAMES=1` | disable frame-parallel rendering (the serial oracle) |
| `XL_NO_GPU_COMPUTE=1` | disable GPU compute entirely |
| `XL_NO_GPU_BLEND` / `_BLUR` / `_BOXBLUR` / `_ROTO` / `_TRANS` / `_SPARKLES` | disable one GPU stage each |
| `XL_NO_METAL_FX=ALL` or `Name,Name` | force named effects to CPU, rest of the GPU pipeline stays active |
| `XL_NO_VULKAN_FX=ALL` or `Name,Name` | same, Vulkan backend |
| `XL_HEADLESS_NO_GL=1` | skip GL init; shaders fall back to solid colour |

**The 2×2 matrix** (`XL_SERIAL` × `XL_NO_GPU_COMPUTE`) splits a diff into
CPU-parallel vs GPU sources. Serial + noGPU byte-identical ⇒ the scheduler/blend
chain is not involved.

A worked example of the technique: a frame-parallel divergence was isolated by
observing that `XL_NO_GPU_BLEND=1` made two runs identical while the default
differed — which named a Metal-vs-ISPC blend discrepancy directly, and the bug
turned out to be a stateful sparkle counter that had been removed from the ISPC
kernel but not the Metal one.

### Stage checksums

`XL_EFFSUM=1` prints per-stage content hashes to stderr. Sort two runs' output
and diff to name the first divergent model/layer/stage:

| Tag | Stage |
|---|---|
| `SUM C` | canvas preload |
| `SUM L` | per-layer effect render |
| `SUM B` | blend input |
| `SUM N` | per-node post-blend |
| `SUM O` | post-blend seqData slice |

```bash
for v in a b; do XL_EFFSUM=1 xLights --headless -s "$SHOW" -od /tmp/$v "$SHOW/Seq.xsq" 2>&1 \
  | grep '^SUM ' | sort > /tmp/sum_$v.txt; done
diff /tmp/sum_a.txt /tmp/sum_b.txt | head
```
`SUM B`/`SUM L` matching while `SUM O` differs localises a bug to `CalcOutput`
rather than to effect rendering — that exact signature has appeared twice.

`XL_EFFSUM` is verbose: one 8300-frame sequence emits ~570k lines. Redirect to a
file, and narrow to the suspect model with `grep` before sorting on a big show.

`XL_BLENDSUM=1` gives per-layer node-color and final blend checksums from the
Metal/Vulkan blend kernels for the same purpose one level lower.

### Targeted dumps

| Variable | Format | Dumps |
|---|---|---|
| `XLDBG_LDUMP` | `"<model>:<layer>:<frame>:<outfile>"` | raw layer pixels (needs `XL_EFFSUM=1`) |
| `XLDBG_NDUMP` | `"<model>:<startFrame>:<endFrame>"` | every node's post-blend colour in the window |

---

## 7. Localising an fseq difference

`--fseqcmp -s <showdir>` already prints per-model diffs and a frame-offset probe.
Then narrow with:

| Variable | Effect |
|---|---|
| `XL_FSEQCMP_DUMPCH=<ch>` | dump one 1-based channel's A/B series across frames |
| `XL_FSEQCMP_RANGE=<first>[-<last>]` | frame window (inclusive) for the channel and PNG dumps. Without it the channel dump only covers the first 80 frames — it cannot reach a divergence thousands of frames in |
| `XL_FSEQCMP_FRAMES=1` | report which frame *ranges* differ — the temporal shape localises window/state bugs the way per-model localises rows |
| `XL_FSEQCMP_PNG=<model>` | rasterise that model's render buffer per frame as an `A \| B \| amplified-diff` strip. Needs `XL_FSEQCMP_RANGE` and `-s`. Capped at 64 frames |
| `XL_FSEQCMP_PNGDIR=<dir>` | where the strips go (default `.`) |

Numbers localise a divergence to a channel; the PNG strip shows its *shape* (a
rotation blowing up, a row off by one, one hot pixel), which is usually the
faster route to the mechanism.

---

## 8. Tuning knobs (frame-parallel scheduler)

Read `src-core/render/RenderEngine.cpp` and `src-core/utils/RangeWorkPool.h`
before using these; they change scheduling, not correctness.

| Variable | Default | Meaning |
|---|---|---|
| `XL_PARALLEL_FRAME_WORKERS` | `hardware_concurrency()-1` | frame-pool worker threads. Bounds how many ROWS render frames in parallel, not how fast one row goes |
| `XL_PARALLEL_GROUP_CHUNK` | 8 | per-row frame concurrency for **group** rows (also bounds their clone-buffer count) |
| `XL_PARALLEL_CHUNK` | 24 | same, for large **model** rows |
| `XL_PARALLEL_SUBMODEL_CHUNK` | 8 | same, for rows carrying submodel/strand effect rows |
| `XL_PARALLEL_MODEL_MIN` | 2048 | pixel floor for a plain model row to qualify; `0` disables model rows |
| `XL_PARALLEL_SUBMODEL_ROWS` | on | `0` excludes submodel/strand-carrying rows (bisect switch) |
| `XL_NO_PARALLEL_FRAMES` | unset | `1` forces fully serial frames |

All chunk values are clamped to `[2, 24]`.

Measured guidance: raising group-row concurrency from 8 to 16 cost ~10% more
system time on a video/group-heavy reference sequence for no wall gain —
concurrent frames contend on the shared `parallel_for` pool rather than adding
throughput. Higher is not better; re-measure before raising these.

---

## 9. Other flags

| Variable | Area | Meaning |
|---|---|---|
| `XL_HEADLESS_RENDERCACHE=1` | headless | re-enable the render cache (off by default in headless) |
| `XL_VIDEO_PARALLEL=1` | video | opt into frame-parallel Video effects (off pending decoder-lock rework) |
| `XL_NO_DECODE_SCALE=1` | video | decode at native size instead of render size (reverts decode-time scaling) |
| `XL_GPU_STATS=1` | Vulkan | GPU statistics dump |
| `XL_VULKAN_ALLOW_CPU=1` | Vulkan | use a software Vulkan device (lavapipe/llvmpipe) for COMPUTE effects too. Off by default because ISPC beats a software device by 3-4x; useful only to exercise the Vulkan compute kernels on a machine with no GPU. Shader effects use a software device regardless — they have no CPU implementation |
| `XL_VULKAN_DEVICE=<n>` | Vulkan | select physical device |
| `XL_VULKAN_CONCURRENCY=<n>` | Vulkan | override compute concurrency |
| `XL_VULKAN_VALIDATE` / `XL_VULKAN_GPUAV` | Vulkan | validation layers |
| `XL_NO_NATIVE_SHADER=1` | shaders | disable the native shader path |
| `XL_NATIVE_SHADER_DEBUG=1` | shaders | native shader debug output |
| `XL_SHADER_BUILD_STATS=1` | shaders | dumps, at exit, where the native Shader path's CPU time went: `ParseShader`, `SourceTransforms::Apply`, `nativeBuild` split into real glslang/pipeline misses vs process-wide program-cache hits, per-frame `nativeEncode`, and (Vulkan) a breakdown *inside* `nativeEncode` — input upload / record cb / queue submit / fence wait / readback memcpy. Use it before assuming "shader compiling is slow" — on a 302-effect / 235-distinct-shader sequence the compiles are ~3%, and of the remaining per-frame cost **81.6% is the CPU parked in `vkWaitForFences`**, with record/submit/readback ~3% each. Optimising anything but the wait is capped at a few percent |
| `XL_GRAPHICS_BACKEND=<name>` | UI | force a canvas backend (desktop UI only) |
| `XLDBG_BLURSYNC=1` | GPU | force synchronous GPU blur |

---

## 10. Checklist for a render performance change

1. Build **Release**, one binary, from a clean tree state.
2. **Byte-identity** vs `XL_NO_PARALLEL_FRAMES=1` (or vs the previous binary) on
   the four canonical sequences, plus the snapshot three if you touched that path.
3. If you touched `GetFrameParallelism` anywhere: `XL_VERIFY_STATELESS=1` clean.
4. `XL_PARALLEL_BLOCKERS=1` coverage delta, if the change is about eligibility.
5. Timing: idle machine, alternated A/B, ≥3 reps, **render-only time + CPU**,
   report the spread. State plainly if it is within noise.
6. `XL_RENDER_PROFILE=1` before/after if you claim a specific stage got faster.
7. Build the **iPad library** too (`src-core/` is shared) — see AGENTS.md §4.
8. Record the result honestly, including "no measurable change". Several changes
   in this area are byte-identical and neutral; that is a legitimate outcome and
   the reasons are usually informative.

---

## Related

- `AGENTS.md` §4 — build commands, headless summary
- `plans/render-scheduler.md` — the suspend/requeue render scheduler
- `src-core/render/RenderProfile.h` — profiling internals and GPU attribution
- `src-core/utils/RangeWorkPool.h` — the frame-parallel work pool

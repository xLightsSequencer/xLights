#ifdef _MSC_VER
// required so M_PI will be defined by MSC
#define _USE_MATH_DEFINES
#include <math.h>
#endif

#include "ChordDetector.h"

#include "AudioManager.h"
#include "kiss_fft/tools/kiss_fftr.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <vector>

namespace {

static const char* const kNoteNames[12] = {
    "C", "C#", "D", "D#", "E", "F",
    "F#", "G", "G#", "A", "A#", "B"
};

struct ChordTemplate {
    std::string name;
    std::array<float, 12> v;
};

std::vector<ChordTemplate> BuildTemplates() {
    std::vector<ChordTemplate> out;
    out.reserve(24);
    for (int root = 0; root < 12; root++) {
        // Major triad: root, major third, perfect fifth.
        ChordTemplate maj;
        maj.name = kNoteNames[root];
        maj.v.fill(0.0f);
        maj.v[root] = 1.0f;
        maj.v[(root + 4) % 12] = 1.0f;
        maj.v[(root + 7) % 12] = 1.0f;
        out.push_back(std::move(maj));
        // Minor triad: root, minor third, perfect fifth.
        ChordTemplate min;
        min.name = std::string(kNoteNames[root]) + "m";
        min.v.fill(0.0f);
        min.v[root] = 1.0f;
        min.v[(root + 3) % 12] = 1.0f;
        min.v[(root + 7) % 12] = 1.0f;
        out.push_back(std::move(min));
    }
    return out;
}

std::vector<float> MakeHannWindow(int n) {
    std::vector<float> w(n);
    if (n <= 1) { if (n == 1) w[0] = 1.0f; return w; }
    const float twoPi = 2.0f * float(M_PI);
    const float denom = float(n - 1);
    for (int i = 0; i < n; i++) {
        w[i] = 0.5f * (1.0f - std::cos(twoPi * float(i) / denom));
    }
    return w;
}

} // namespace

HarmonyAnalysis DetectChords(AudioManager* audio,
                              const ChordDetectorOptions& opts) {
    HarmonyAnalysis out;
    if (!audio || !audio->IsOk()) return out;

    const int N = opts.frameSize;
    const int hop = opts.hopSize;
    if (N < 32 || (N & 1) != 0 || hop <= 0) return out;

    long trackSize = audio->GetTrackSize();
    long rate = audio->GetRate();
    if (trackSize < long(N) || rate <= 0) return out;

    (void)audio->GetRawLeftDataPtr(trackSize - 1);
    float* src = audio->GetRawLeftDataPtr(0);
    if (!src) return out;

    kiss_fftr_cfg cfg = kiss_fftr_alloc(N, 0, nullptr, nullptr);
    if (!cfg) return out;

    const int nBins = N / 2 + 1;
    std::vector<float> window = MakeHannWindow(N);
    std::vector<float> input(N);
    std::vector<kiss_fft_cpx> spec(nBins);

    // Precompute per-bin pitch class within the usable frequency
    // range. Bins outside the range contribute nothing to the chroma.
    const int minBin = std::max(1, int(std::ceil(double(opts.minFreqHz) * N / double(rate))));
    const int maxBin = std::min(nBins - 1, int(std::floor(double(opts.maxFreqHz) * N / double(rate))));
    std::vector<int8_t> binToPC(nBins, -1);
    for (int k = minBin; k <= maxBin; k++) {
        double fHz = double(k) * double(rate) / double(N);
        if (fHz <= 0) continue;
        double midi = 69.0 + 12.0 * std::log2(fHz / 440.0);
        int pc = int(std::round(midi));
        pc = ((pc % 12) + 12) % 12;
        binToPC[k] = int8_t(pc);
    }

    const std::vector<ChordTemplate> templates = BuildTemplates();

    std::vector<int> perFrame;
    perFrame.reserve(size_t(trackSize / hop) + 1);
    std::array<double, 12> trackChroma{};
    trackChroma.fill(0.0);

    for (long pos = 0; pos + N <= trackSize; pos += hop) {
        for (int i = 0; i < N; i++) input[i] = src[pos + i] * window[i];
        kiss_fftr(cfg, input.data(), spec.data());

        std::array<float, 12> chroma{};
        chroma.fill(0.0f);
        for (int k = minBin; k <= maxBin; k++) {
            int8_t pc = binToPC[k];
            if (pc < 0) continue;
            float mag = std::sqrt(spec[k].r * spec[k].r + spec[k].i * spec[k].i);
            chroma[pc] += mag;
        }
        float total = 0.0f;
        for (float v : chroma) total += v;
        if (total > 1e-9f) {
            for (float& v : chroma) v /= total;
        }

        // Match against templates — dot product is fine since both
        // sides are non-negative and unit-summed.
        int bestIdx = 0;
        // -ffinite-math-only on Release can fold the infinity sentinel to 0
        // (see HitTest.cpp). Score >= 0 here so the bug is latent, but use a
        // finite sentinel to stay robust if templates ever go non-positive.
        float bestScore = std::numeric_limits<float>::lowest();
        for (size_t t = 0; t < templates.size(); t++) {
            float s = 0.0f;
            for (int i = 0; i < 12; i++) s += chroma[i] * templates[t].v[i];
            if (s > bestScore) { bestScore = s; bestIdx = int(t); }
        }
        perFrame.push_back(bestIdx);
        for (int i = 0; i < 12; i++) trackChroma[i] += chroma[i];
    }

    free(cfg);

    if (perFrame.empty()) return out;

    // Mode-filter smoothing — each frame's label is replaced by the
    // most common label in a ±W-frame window, where W is chosen so
    // the window spans `minSegmentMS`.
    const double hopMS = 1000.0 * double(hop) / double(rate);
    const int minFrames = std::max(1, int(std::ceil(double(opts.minSegmentMS) / hopMS)));
    const int W = std::max(1, minFrames / 2);

    std::vector<int> smoothed(perFrame.size());
    for (int i = 0; i < int(perFrame.size()); i++) {
        int lo = std::max(0, i - W);
        int hi = std::min(int(perFrame.size()) - 1, i + W);
        int counts[24] = {0};
        for (int j = lo; j <= hi; j++) counts[perFrame[j]]++;
        int bestC = 0;
        int bestIdx = perFrame[i];
        for (int c = 0; c < 24; c++) {
            if (counts[c] > bestC) { bestC = counts[c]; bestIdx = c; }
        }
        smoothed[i] = bestIdx;
    }

    // Emit segments via run-length consolidation.
    const long trackMS = audio->LengthMS();
    int cur = -1;
    long curStart = 0;
    for (int i = 0; i < int(smoothed.size()); i++) {
        long ms = long(double(i) * hopMS);
        if (smoothed[i] != cur) {
            if (cur >= 0) {
                ChordSegment seg;
                seg.startMS = curStart;
                seg.endMS = ms;
                seg.name = templates[size_t(cur)].name;
                out.chords.push_back(seg);
            }
            cur = smoothed[i];
            curStart = ms;
        }
    }
    if (cur >= 0) {
        ChordSegment seg;
        seg.startMS = curStart;
        seg.endMS = trackMS;
        seg.name = templates[size_t(cur)].name;
        out.chords.push_back(seg);
    }

    // Krumhansl–Schmuckler key estimation using the track-averaged
    // chroma. Profiles are the standard major/minor key-finding
    // profiles; we correlate via simple dot product which is
    // equivalent to K-S up to a constant bias when the chroma is
    // unit-normalised.
    static const float kMajor[12] = {
        6.35f, 2.23f, 3.48f, 2.33f, 4.38f, 4.09f,
        2.52f, 5.19f, 2.39f, 3.66f, 2.29f, 2.88f
    };
    static const float kMinor[12] = {
        6.33f, 2.68f, 3.52f, 5.38f, 2.60f, 3.53f,
        2.54f, 4.75f, 3.98f, 2.69f, 3.34f, 3.17f
    };
    std::array<float, 12> avgChroma{};
    double sum = 0;
    for (double v : trackChroma) sum += v;
    if (sum > 1e-9) {
        for (int i = 0; i < 12; i++) avgChroma[i] = float(trackChroma[i] / sum);
    }
    // Finite sentinel to dodge -ffinite-math-only folding `-inf` to 0
    // (see HitTest.cpp + the per-frame match above).
    float bestKeyScore = std::numeric_limits<float>::lowest();
    std::string bestKey;
    for (int root = 0; root < 12; root++) {
        float sMaj = 0, sMin = 0;
        for (int i = 0; i < 12; i++) {
            int idx = (i + 12 - root) % 12;
            sMaj += avgChroma[i] * kMajor[idx];
            sMin += avgChroma[i] * kMinor[idx];
        }
        if (sMaj > bestKeyScore) {
            bestKeyScore = sMaj;
            bestKey = std::string(kNoteNames[root]) + " major";
        }
        if (sMin > bestKeyScore) {
            bestKeyScore = sMin;
            bestKey = std::string(kNoteNames[root]) + " minor";
        }
    }
    out.key = bestKey;

    return out;
}

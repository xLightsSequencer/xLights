#ifdef _MSC_VER
// required so M_PI will be defined by MSC
#define _USE_MATH_DEFINES
#include <math.h>
#endif

#include "OnsetDetector.h"

#include "AudioManager.h"
#include "kiss_fft/tools/kiss_fftr.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

namespace {

std::vector<float> MakeHannWindow(int n) {
    std::vector<float> w(n);
    if (n <= 1) {
        if (n == 1) w[0] = 1.0f;
        return w;
    }
    const float twoPi = 2.0f * float(M_PI);
    const float denom = float(n - 1);
    for (int i = 0; i < n; i++) {
        w[i] = 0.5f * (1.0f - std::cos(twoPi * float(i) / denom));
    }
    return w;
}

} // namespace

OnsetEnvelope ComputeOnsetEnvelope(AudioManager* audio,
                                    const OnsetDetectorOptions& opts) {
    OnsetEnvelope env;
    if (!audio || !audio->IsOk()) return env;

    const int N = opts.frameSize;
    const int hop = opts.hopSize;
    if (N < 32 || (N & 1) != 0 || hop <= 0 || hop > N) return env;

    long trackSize = audio->GetTrackSize();
    long rate = audio->GetRate();
    if (trackSize < long(N) * 2 || rate <= 0) return env;

    // Wait for the full track to finish loading.
    (void)audio->GetRawLeftDataPtr(trackSize - 1);
    float* src = audio->GetRawLeftDataPtr(0);
    if (!src) return env;

    kiss_fftr_cfg cfg = kiss_fftr_alloc(N, 0, nullptr, nullptr);
    if (!cfg) return env;

    const int nBins = N / 2 + 1;
    std::vector<float> window = MakeHannWindow(N);
    std::vector<float> input(N);
    std::vector<kiss_fft_cpx> spec(nBins);
    std::vector<float> prevMag(nBins, 0.0f);
    std::vector<float> curMag(nBins, 0.0f);

    env.flux.reserve(size_t(trackSize / hop) + 1);
    bool firstFrame = true;
    for (long pos = 0; pos + N <= trackSize; pos += hop) {
        for (int i = 0; i < N; i++) {
            input[i] = src[pos + i] * window[i];
        }
        kiss_fftr(cfg, input.data(), spec.data());
        for (int k = 0; k < nBins; k++) {
            curMag[k] = std::sqrt(spec[k].r * spec[k].r + spec[k].i * spec[k].i);
        }
        if (firstFrame) {
            env.flux.push_back(0.0f);
            firstFrame = false;
        } else {
            float sum = 0.0f;
            for (int k = 0; k < nBins; k++) {
                float d = curMag[k] - prevMag[k];
                if (d > 0.0f) sum += d;
            }
            env.flux.push_back(sum);
        }
        std::swap(prevMag, curMag);
    }

    free(cfg);

    env.hopSize = hop;
    env.sampleRate = int(rate);
    env.lengthMS = audio->LengthMS();
    return env;
}

std::vector<long> DetectOnsets(AudioManager* audio,
                               const OnsetDetectorOptions& opts) {
    std::vector<long> onsets;
    OnsetEnvelope env = ComputeOnsetEnvelope(audio, opts);
    if (env.flux.empty() || env.sampleRate <= 0) return onsets;

    const std::vector<float>& flux = env.flux;
    const int hop = env.hopSize;
    const long rate = env.sampleRate;
    const int F = int(flux.size());
    if (F < 4) return onsets;

    // Adaptive threshold: flux[i] must exceed `sensitivity *
    // median(flux[i-W..i+W])` AND be a strict local max (> left
    // neighbour, >= right). Refractory period keeps peaks from
    // bunching on rolls / fast hi-hat patterns.
    const int W = std::max(1, opts.medianHalfWindow);
    std::vector<float> scratch;
    scratch.reserve(size_t(2 * W + 1));

    const double msPerFrame = 1000.0 * double(hop) / double(rate);
    const int minSpacingFrames = std::max(1, int(std::ceil(opts.minSpacingMS / msPerFrame)));
    int lastOnsetFrame = -minSpacingFrames;

    // Half-frame time offset so onset marks land at the centre of the
    // analysis window rather than its left edge.
    const double frameCenterMS = msPerFrame * 0.5;

    for (int i = 1; i < F - 1; i++) {
        const int lo = std::max(0, i - W);
        const int hi = std::min(F - 1, i + W);
        scratch.assign(flux.begin() + lo, flux.begin() + hi + 1);
        auto mid = scratch.begin() + scratch.size() / 2;
        std::nth_element(scratch.begin(), mid, scratch.end());
        const float median = *mid;
        const float threshold = median * opts.sensitivity;
        const float f = flux[i];
        if (f <= threshold || f <= 1e-6f) continue;
        if (!(f > flux[i - 1] && f >= flux[i + 1])) continue;
        if (i - lastOnsetFrame < minSpacingFrames) continue;
        const long ms = long(double(i) * msPerFrame + frameCenterMS);
        onsets.push_back(ms);
        lastOnsetFrame = i;
    }

    return onsets;
}

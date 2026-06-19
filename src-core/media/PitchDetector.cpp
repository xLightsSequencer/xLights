#ifdef _MSC_VER
// required so M_PI will be defined by MSC
#define _USE_MATH_DEFINES
#include <math.h>
#endif

#include "PitchDetector.h"

#include "AudioManager.h"
#include "kiss_fft/tools/kiss_fftr.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

namespace {

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

PitchContour DetectPitch(AudioManager* audio, const PitchDetectorOptions& opts) {
    PitchContour out;
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

    // Wiener–Khinchin: ACF = IFFT(|FFT|²) on a zero-padded 2N signal
    // so the "circular" IFFT result matches the linear autocorrelation
    // for the lags we care about.
    const int M = 2 * N;
    kiss_fftr_cfg cfgFwd = kiss_fftr_alloc(M, 0, nullptr, nullptr);
    kiss_fftr_cfg cfgInv = kiss_fftr_alloc(M, 1, nullptr, nullptr);
    if (!cfgFwd || !cfgInv) {
        if (cfgFwd) free(cfgFwd);
        if (cfgInv) free(cfgInv);
        return out;
    }

    const int nBins = M / 2 + 1;
    std::vector<float> window = MakeHannWindow(N);
    std::vector<float> input(M, 0.0f);
    std::vector<kiss_fft_cpx> spec(nBins);
    std::vector<float> acf(M, 0.0f);

    const int minTau = std::max(1, int(std::floor(double(rate) / double(opts.maxFreqHz))));
    const int maxTau = std::min(N - 2, int(std::ceil(double(rate) / double(opts.minFreqHz))));
    if (maxTau <= minTau + 2) {
        free(cfgFwd); free(cfgInv);
        return out;
    }

    out.frameHopMS = 1000.0f * float(hop) / float(rate);
    out.samples.reserve(size_t(trackSize / hop) + 1);
    const long halfFrameMS = long(500.0 * double(N) / double(rate));

    for (long pos = 0; pos + N <= trackSize; pos += hop) {
        for (int i = 0; i < N; i++) input[i] = src[pos + i] * window[i];
        for (int i = N; i < M; i++) input[i] = 0.0f;

        kiss_fftr(cfgFwd, input.data(), spec.data());
        for (int k = 0; k < nBins; k++) {
            float re = spec[k].r;
            float im = spec[k].i;
            spec[k].r = re * re + im * im;
            spec[k].i = 0.0f;
        }
        kiss_fftri(cfgInv, spec.data(), acf.data());

        float r0 = acf[0];
        PitchSample s;
        s.timeMS = long(double(pos) * 1000.0 / double(rate)) + halfFrameMS;

        if (r0 <= 1e-9f) {
            s.frequency = 0.0f;
            s.confidence = 0.0f;
            out.samples.push_back(s);
            continue;
        }

        int bestTau = minTau;
        float bestR = acf[minTau];
        for (int tau = minTau + 1; tau <= maxTau; tau++) {
            if (acf[tau] > bestR) {
                bestR = acf[tau];
                bestTau = tau;
            }
        }

        float confidence = bestR / r0;
        if (confidence < 0.0f) confidence = 0.0f;
        if (confidence > 1.0f) confidence = 1.0f;
        s.confidence = confidence;

        if (confidence >= opts.voicingThreshold) {
            double refinedTau = double(bestTau);
            if (bestTau > minTau && bestTau < maxTau) {
                double y1 = acf[bestTau - 1];
                double y2 = acf[bestTau];
                double y3 = acf[bestTau + 1];
                double denom = y1 - 2.0 * y2 + y3;
                if (std::fabs(denom) > 1e-12) {
                    double dx = 0.5 * (y1 - y3) / denom;
                    if (dx > -1.0 && dx < 1.0) refinedTau += dx;
                }
            }
            if (refinedTau > 0.5) {
                s.frequency = float(double(rate) / refinedTau);
            }
        }
        out.samples.push_back(s);
    }

    free(cfgFwd);
    free(cfgInv);
    return out;
}

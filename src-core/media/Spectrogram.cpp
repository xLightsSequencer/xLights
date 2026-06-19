#ifdef _MSC_VER
// required so M_PI will be defined by MSC
#define _USE_MATH_DEFINES
#include <math.h>
#endif

#include "Spectrogram.h"

#include "AudioManager.h"
#include "kiss_fft/tools/kiss_fftr.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

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

// 256-stop approximation of a magenta→yellow colormap (inferno-ish).
// Returns BGRA with A=255. `t` is clamped to [0, 1].
void MapColor(float t, uint8_t& b, uint8_t& g, uint8_t& r, uint8_t& a) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    // Piecewise-linear stops sampled from inferno:
    //   0.00  black
    //   0.25  purple  (80, 10, 100)
    //   0.50  red     (210, 60, 70)
    //   0.75  orange  (250, 150, 30)
    //   1.00  yellow  (255, 255, 180)
    struct Stop { float t; float r, g, b; };
    static const Stop stops[5] = {
        {0.00f,   0.0f,   0.0f,   0.0f},
        {0.25f,  80.0f,  10.0f, 100.0f},
        {0.50f, 210.0f,  60.0f,  70.0f},
        {0.75f, 250.0f, 150.0f,  30.0f},
        {1.00f, 255.0f, 255.0f, 180.0f},
    };
    int i = 0;
    while (i < 3 && t > stops[i + 1].t) i++;
    float span = stops[i + 1].t - stops[i].t;
    float f = span > 1e-6f ? (t - stops[i].t) / span : 0.0f;
    float R = stops[i].r + (stops[i + 1].r - stops[i].r) * f;
    float G = stops[i].g + (stops[i + 1].g - stops[i].g) * f;
    float B = stops[i].b + (stops[i + 1].b - stops[i].b) * f;
    r = uint8_t(R); g = uint8_t(G); b = uint8_t(B); a = 255;
}

} // namespace

Spectrogram ComputeSpectrogram(AudioManager* audio,
                                const SpectrogramOptions& opts) {
    Spectrogram out;
    if (!audio || !audio->IsOk()) return out;

    const int N = opts.frameSize;
    const int hop = opts.hopSize;
    if (N < 32 || (N & 1) != 0 || hop <= 0 || hop > N) return out;

    long trackSize = audio->GetTrackSize();
    long rate = audio->GetRate();
    if (trackSize < long(N) || rate <= 0) return out;

    // Wait for the full track to finish loading — we iterate to
    // `trackSize`, and reading uninitialised samples from the
    // unloaded tail turns into all-zero magnitudes (black stripes)
    // for the second half of the spectrogram.
    (void)audio->GetRawLeftDataPtr(trackSize - 1);
    float* src = audio->GetRawLeftDataPtr(0);
    if (!src) return out;

    kiss_fftr_cfg cfg = kiss_fftr_alloc(N, 0, nullptr, nullptr);
    if (!cfg) return out;

    const int nBins = N / 2 + 1;
    const std::vector<float> window = MakeHannWindow(N);
    std::vector<float> input(N);
    std::vector<kiss_fft_cpx> spec(nBins);

    const int totalFrames = int((trackSize - N) / hop) + 1;
    if (totalFrames <= 0) {
        free(cfg);
        return out;
    }
    out.magnitudes.resize(size_t(totalFrames) * size_t(nBins));
    out.frames = totalFrames;
    out.bins = nBins;
    out.hopMS = 1000.0f * float(hop) / float(rate);
    out.sampleRate = int(rate);
    out.frameSize = N;

    float peak = 0.0f;
    int frameIdx = 0;
    for (long pos = 0; pos + N <= trackSize && frameIdx < totalFrames; pos += hop) {
        for (int i = 0; i < N; i++) input[i] = src[pos + i] * window[i];
        kiss_fftr(cfg, input.data(), spec.data());
        float* row = &out.magnitudes[size_t(frameIdx) * size_t(nBins)];
        for (int k = 0; k < nBins; k++) {
            float m = std::sqrt(spec[k].r * spec[k].r + spec[k].i * spec[k].i);
            row[k] = m;
            if (m > peak) peak = m;
        }
        frameIdx++;
    }
    out.peakMag = peak;

    free(cfg);
    return out;
}

void RenderSpectrogramBGRA(const Spectrogram& sg,
                            long startMS, long endMS,
                            int outWidth, int outHeight,
                            std::vector<uint8_t>& outBGRA,
                            float dBFloor) {
    outBGRA.assign(size_t(outWidth) * size_t(outHeight) * 4, 0);
    if (sg.frames <= 0 || sg.bins <= 0 || outWidth <= 0 || outHeight <= 0) return;
    if (endMS <= startMS) return;
    const float peak = sg.peakMag > 1e-9f ? sg.peakMag : 1.0f;
    const float invDBRange = 1.0f / (-dBFloor);

    // Log-frequency mapping: bottom of the strip = minFreq, top =
    // maxFreq. Use the Nyquist as the top and 40 Hz as the low.
    const float sampleRate = float(sg.sampleRate > 0 ? sg.sampleRate : 44100);
    const float minFreq = 40.0f;
    const float maxFreq = 0.5f * sampleRate;
    const float logMin = std::log2(minFreq);
    const float logMax = std::log2(maxFreq);
    const float logRange = logMax - logMin;
    const float binHz = sampleRate / float(sg.frameSize);

    const double msPerFrame = sg.hopMS > 0 ? double(sg.hopMS) : 1.0;
    const double rangeMS = double(endMS - startMS);

    // Precompute per-row bin index (log freq → linear bin).
    std::vector<int> rowToBin;
    rowToBin.resize(size_t(outHeight));
    for (int y = 0; y < outHeight; y++) {
        float norm = 1.0f - (float(y) + 0.5f) / float(outHeight);
        float logF = logMin + norm * logRange;
        float freq = std::exp2(logF);
        int bin = int(std::round(freq / binHz));
        if (bin < 0) bin = 0;
        if (bin >= sg.bins) bin = sg.bins - 1;
        rowToBin[size_t(y)] = bin;
    }

    for (int x = 0; x < outWidth; x++) {
        double ms = double(startMS) + rangeMS * (double(x) + 0.5) / double(outWidth);
        int frameIdx = int(ms / msPerFrame);
        if (frameIdx < 0) frameIdx = 0;
        if (frameIdx >= sg.frames) frameIdx = sg.frames - 1;
        const float* col = &sg.magnitudes[size_t(frameIdx) * size_t(sg.bins)];
        for (int y = 0; y < outHeight; y++) {
            float mag = col[rowToBin[size_t(y)]];
            float dB = mag > 1e-9f ? 20.0f * std::log10(mag / peak) : dBFloor;
            if (dB < dBFloor) dB = dBFloor;
            if (dB > 0) dB = 0;
            float t = 1.0f + dB * invDBRange; // 0 at floor, 1 at peak
            uint8_t b, g, r, a;
            MapColor(t, b, g, r, a);
            uint8_t* px = &outBGRA[size_t(y) * size_t(outWidth) * 4 + size_t(x) * 4];
            px[0] = b; px[1] = g; px[2] = r; px[3] = a;
        }
    }
}

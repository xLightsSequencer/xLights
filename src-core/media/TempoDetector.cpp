#include "TempoDetector.h"

#include "AudioManager.h"
#include "OnsetDetector.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

TempoResult DetectTempo(AudioManager* audio,
                         const TempoDetectorOptions& opts) {
    TempoResult out;
    if (!audio || !audio->IsOk()) return out;

    OnsetEnvelope env = ComputeOnsetEnvelope(audio);
    if (env.flux.size() < 16 || env.sampleRate <= 0 || env.hopSize <= 0) {
        return out;
    }

    const double hopMS = 1000.0 * double(env.hopSize) / double(env.sampleRate);
    if (hopMS <= 0) return out;

    const int minTau = std::max(2, int(std::floor(60000.0 / std::max(opts.maxBPM, 1.0f) / hopMS)));
    const int maxTau = int(std::ceil(60000.0 / std::max(opts.minBPM, 1.0f) / hopMS));
    const int F = int(env.flux.size());
    if (maxTau <= minTau + 2 || F <= maxTau + 4) return out;

    // Zero-mean the flux so the autocorrelation peaks on periodicity
    // rather than on raw energy.
    double sumFlux = 0;
    for (float f : env.flux) sumFlux += f;
    const double meanFlux = sumFlux / double(F);
    std::vector<float> centered(F);
    for (int i = 0; i < F; i++) centered[i] = float(env.flux[i] - meanFlux);

    // Autocorrelation over the BPM search window. `denom` scales so
    // shorter lags don't get an unfair advantage from the larger
    // overlap.
    std::vector<float> autocorr(maxTau + 1, 0.0f);
    for (int tau = minTau; tau <= maxTau; tau++) {
        const int n = F - tau;
        if (n <= 0) continue;
        double acc = 0;
        for (int i = 0; i < n; i++) {
            acc += double(centered[i]) * double(centered[i + tau]);
        }
        autocorr[tau] = float(acc / double(n));
    }

    // Locate the peak. Enhance the harmonic at 2*tau a little — this
    // biases toward the fundamental instead of picking up half-tempo.
    int bestTau = minTau;
    // -ffast-math (desktop Release) implies -ffinite-math-only — the optimizer is
    // licensed to assume no operand is +/-inf and may fold the sentinel to 0,
    // which would make negative autocorrelation peaks fail `v > bestVal` and
    // silently pick the wrong tempo. Use a finite sentinel.
    float bestVal = std::numeric_limits<float>::lowest();
    for (int tau = minTau; tau <= maxTau; tau++) {
        float v = autocorr[tau];
        int tau2 = tau * 2;
        if (tau2 <= maxTau) v += 0.5f * autocorr[tau2];
        int tau3 = tau * 3;
        if (tau3 <= maxTau) v += 0.25f * autocorr[tau3];
        if (v > bestVal) {
            bestVal = v;
            bestTau = tau;
        }
    }

    // Parabolic interpolation between the three samples around the
    // peak for sub-frame accuracy.
    double refinedTau = double(bestTau);
    if (bestTau > minTau && bestTau < maxTau) {
        double y1 = autocorr[bestTau - 1];
        double y2 = autocorr[bestTau];
        double y3 = autocorr[bestTau + 1];
        double denom = (y1 - 2.0 * y2 + y3);
        if (std::fabs(denom) > 1e-12) {
            double dx = 0.5 * (y1 - y3) / denom;
            if (dx > -1 && dx < 1) refinedTau += dx;
        }
    }

    const double periodMS = refinedTau * hopMS;
    if (periodMS <= 0) return out;
    out.bpm = float(60000.0 / periodMS);

    // Confidence = peak value / mean of the search-range autocorr.
    double sum = 0;
    int count = 0;
    for (int tau = minTau; tau <= maxTau; tau++) {
        sum += std::fabs(autocorr[tau]);
        count++;
    }
    double meanAC = count > 0 ? sum / double(count) : 0.0;
    if (meanAC > 0) {
        double ratio = autocorr[bestTau] / (meanAC * 4.0);
        if (ratio < 0) ratio = 0;
        if (ratio > 1) ratio = 1;
        out.confidence = float(ratio);
    }

    // Phase detection: sweep a Dirac comb of period `periodFrames`
    // across the envelope and pick the phase that maximises total
    // flux under the comb's teeth.
    const int periodFrames = std::max(2, int(std::round(refinedTau)));
    double bestPhaseScore = -1;
    int bestPhase = 0;
    for (int phase = 0; phase < periodFrames; phase++) {
        double score = 0;
        for (int k = 0; phase + k * periodFrames < F; k++) {
            score += env.flux[phase + k * periodFrames];
        }
        if (score > bestPhaseScore) {
            bestPhaseScore = score;
            bestPhase = phase;
        }
    }

    // Emit beat positions at `refinedTau` spacing from the best phase
    // (use float-precision spacing so long tracks don't drift).
    const double firstBeatMS = double(bestPhase) * hopMS;
    const long endMS = env.lengthMS;
    for (double ms = firstBeatMS; ms < double(endMS); ms += periodMS) {
        out.beatMS.push_back(long(ms));
    }
    return out;
}

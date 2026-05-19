#pragma once

// A5: pitch contour extractor. Time-domain autocorrelation of windowed
// frames (computed via FFT so the per-frame cost stays O(N log N)),
// peak-picked in a configurable frequency range, parabolic-interpolated
// for sub-sample accuracy. Cross-platform via kiss_fft.

#include <vector>

class AudioManager;

struct PitchSample {
    // Absolute time of the analysis window's centre.
    long timeMS = 0;
    // Detected fundamental in Hz. 0 when the frame was classified as
    // unvoiced (confidence below threshold).
    float frequency = 0.0f;
    // 0..1 — the normalized autocorrelation peak relative to the
    // frame's zero-lag energy. Values >= the voicing threshold mark
    // the frame as pitched.
    float confidence = 0.0f;
};

struct PitchContour {
    std::vector<PitchSample> samples;
    float frameHopMS = 0.0f;
};

struct PitchDetectorOptions {
    int frameSize = 2048;      // analysis window (~46 ms @ 44.1 k)
    int hopSize = 441;         // ~10 ms @ 44.1 k
    float minFreqHz = 75.0f;   // low bound (bass voice)
    float maxFreqHz = 1200.0f; // upper bound (soprano / lead melody)
    // Normalized ACF peak threshold below which a frame is considered
    // unvoiced (the frame's frequency field is set to 0).
    float voicingThreshold = 0.4f;
};

// Blocking. Returns empty contour on failure.
PitchContour DetectPitch(AudioManager* audio,
                          const PitchDetectorOptions& opts = PitchDetectorOptions{});

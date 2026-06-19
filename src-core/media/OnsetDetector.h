#pragma once

// A2: percussive-onset detector. Runs spectral-flux peak-picking on the
// AudioManager's left channel; returns onset positions in milliseconds.
// Uses kiss_fft (already vendored under `src-core/media/kiss_fft/`) so
// the algorithm is cross-platform — iPad and desktop share the same
// path with no `#ifdef __APPLE__` branches.

#include <vector>

class AudioManager;

struct OnsetDetectorOptions {
    // FFT window length in samples. Must be even (kiss_fftr constraint).
    int frameSize = 1024;
    // Samples between successive frames. Typical values are `frameSize
    // / 2` or `frameSize / 4`.
    int hopSize = 512;
    // Adaptive-threshold multiplier: flux > `sensitivity * localMedian`
    // is required for a frame to be considered. Higher = fewer onsets.
    float sensitivity = 1.5f;
    // Refractory period — minimum interval between accepted onsets.
    int minSpacingMS = 60;
    // Half-window (in frames) used for the local median threshold.
    int medianHalfWindow = 20;
};

// Run the detector. Blocks until the required audio range is loaded.
// Returns ascending millisecond positions; empty vector on failure
// (e.g. no audio loaded, too-short track).
std::vector<long> DetectOnsets(AudioManager* audio,
                               const OnsetDetectorOptions& opts = OnsetDetectorOptions{});

// The raw spectral-flux curve that `DetectOnsets` peak-picks over.
// Exposed so that consumers like the tempo detector can autocorrelate
// the envelope directly. `flux[0]` is always 0 (first frame has no
// predecessor to difference against).
struct OnsetEnvelope {
    std::vector<float> flux;
    int hopSize = 512;
    int sampleRate = 44100;
    long lengthMS = 0;
};

OnsetEnvelope ComputeOnsetEnvelope(AudioManager* audio,
                                    const OnsetDetectorOptions& opts = OnsetDetectorOptions{});

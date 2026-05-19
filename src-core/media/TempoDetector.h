#pragma once

// A4: tempo / beat detection via autocorrelation of the onset
// envelope. Reuses `ComputeOnsetEnvelope` from OnsetDetector so there
// is only one FFT pass for both features on a typical track. Cross-
// platform (kiss_fft, no platform-specific dependencies).

#include <vector>

class AudioManager;

struct TempoResult {
    // Detected BPM, or 0 on failure.
    float bpm = 0.0f;
    // 0..1 confidence heuristic derived from the autocorrelation peak
    // height vs the mean autocorrelation inside the BPM search range.
    // < 0.2 is sketchy; > 0.5 is solid on rhythmic material.
    float confidence = 0.0f;
    // Ascending millisecond positions of detected beats, one per
    // period. Includes the implied downbeat phase offset.
    std::vector<long> beatMS;
};

struct TempoDetectorOptions {
    // BPM search window. 60..300 covers everything from ballads to
    // drum'n'bass; narrower search = more stable but misses outliers.
    float minBPM = 60.0f;
    float maxBPM = 300.0f;
};

// Blocking. Returns `{0, 0, {}}` on failure (no audio loaded,
// too-short track, or flat onset envelope).
TempoResult DetectTempo(AudioManager* audio,
                         const TempoDetectorOptions& opts = TempoDetectorOptions{});

#pragma once

// A7: Sound classification. Returns per-window confidence curves for each
// detected sound class (drums, vocals, guitar, applause, …).
//
// Two implementations:
//   SoundClassifier.mm  — Apple targets (macOS 12+, iOS 15+).
//                         Uses SNClassifySoundRequest from SoundAnalysis.framework.
//   SoundClassifier.cpp — Windows / Linux (requires HAVE_ORT).
//                         Uses an ONNX model at opts.modelPath (YAMNet-compatible).

#include <string>
#include <vector>

class AudioManager;

struct SoundClassResult {
    std::string name;
    // One confidence value per `timeStepSeconds` of audio; [0, 1] per window.
    // Classes are returned sorted by averageConfidence descending.
    std::vector<float> confidence;
    float averageConfidence = 0.0f;
};

struct SoundClassification {
    std::vector<SoundClassResult> classes;
    // Window size (seconds) used by the analyzer.
    float timeStepSeconds = 1.0f;
    long lengthMS = 0;
};

struct SoundClassifierOptions {
    // Classes whose averageConfidence falls below this are dropped
    // from the result before `maxClasses` is applied.
    float minAverageConfidence = 0.05f;
    // Upper bound on returned classes (for UI sanity).
    int maxClasses = 12;
    // Reporting window in seconds. Larger = smoother curves.
    float windowSeconds = 1.0f;
    // Path to .onnx model file. Non-Apple only; Apple uses the built-in
    // classifier and ignores this field.
    std::string modelPath;
};

// Runs synchronously. Apple's classifier is real-time on M-series
// Macs and ~real-time on modern iPads; a 4-minute track returns in
// well under 10 seconds. Returns an empty `SoundClassification` on
// failure (no audio loaded, framework unavailable, analyzer error).
SoundClassification ClassifySound(AudioManager* audio,
                                   const SoundClassifierOptions& opts = SoundClassifierOptions{});

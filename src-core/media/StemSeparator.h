#pragma once

// A8: HTDemucs-based 4-stem source separation. Splits the
// AudioManager's stereo signal into drums / bass / other / vocals
// streams via an on-device CoreML model (download-on-first-use, see
// `AIModelStore`). Apple-only; the `.mm` implementation isn't
// compiled on non-Apple targets and callers must guard with
// `#ifdef __APPLE__`.
//
// Model source: https://github.com/john-rocky/CoreML-Models/releases/tag/demucs-v1
// Filename:     HTDemucs_SourceSeparation_F32.mlpackage
// Input:        `mix` [1, 2, 343980]  stereo float @ 44.1 kHz (~7.8 s)
// Output:       `time_output` [1, 8, 343980]  (drums L/R, bass L/R, other L/R, vocals L/R)

#include <functional>
#include <string>
#include <vector>

class AudioManager;

struct StemOutput {
    std::vector<float> drumsL, drumsR;
    std::vector<float> bassL, bassR;
    std::vector<float> otherL, otherR;
    std::vector<float> vocalsL, vocalsR;
    // Number of samples in each vector. All vectors share the same
    // length and sample rate as the source audio.
    long sampleRate = 0;
};

struct StemSeparatorOptions {
    // Chunk size the HTDemucs F32 model expects (fixed by the model
    // author's conversion — don't change).
    int chunkSamples = 343980;
    // Overlap between chunks in samples. Small overlap crossfaded
    // with a linear taper keeps chunk boundaries from clicking.
    // 0 = pure concatenation (fastest, audible click at seams).
    int overlapSamples = 4410; // 100 ms
};

// Synchronous. Loads the model at `modelPath`, runs inference, and
// fills `out`. Returns false on model-load or inference failure.
// `progress` (optional) is called with a 0..100 integer as chunks
// complete.
bool SeparateStems(AudioManager* audio,
                    const std::string& modelPath,
                    StemOutput& out,
                    const StemSeparatorOptions& opts = StemSeparatorOptions{},
                    std::function<void(int pct)> progress = nullptr);

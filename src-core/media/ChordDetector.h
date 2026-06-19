#pragma once

// A9: chromagram-based chord and key detection. For each analysis
// frame, the magnitude spectrum is folded into 12 pitch classes; the
// resulting chroma vector is matched against 24 major/minor chord
// templates, and per-frame picks are smoothed via mode filtering into
// segments. Key detection uses Krumhansl–Schmuckler correlation on
// the track-averaged chroma. Cross-platform (kiss_fft).

#include <string>
#include <vector>

class AudioManager;

struct ChordSegment {
    long startMS = 0;
    long endMS = 0;
    std::string name; // "C", "Am", "F#m", …
};

struct HarmonyAnalysis {
    // e.g. "C major" / "A minor"; empty string on failure.
    std::string key;
    std::vector<ChordSegment> chords;
};

struct ChordDetectorOptions {
    int frameSize = 4096;
    int hopSize = 1024;
    float minFreqHz = 80.0f;
    float maxFreqHz = 3200.0f;
    // Chord runs shorter than this are merged into neighbours (via a
    // mode filter) so we don't emit a label every fraction of a beat.
    int minSegmentMS = 300;
};

HarmonyAnalysis DetectChords(AudioManager* audio,
                              const ChordDetectorOptions& opts = ChordDetectorOptions{});

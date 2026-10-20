#pragma once

#include <string>
#include <vector>

// Generates sparse, white Moving Head effects for a row of heads from a song's bars and sections.
// wx-free: produces plain data (and settings strings); the UI layer places them in the sequence.
namespace MovingHeadSong {

struct Section {
    long startMS = 0;
    long endMS = 0;
    bool chorus = false;
    bool peak = false;
};

enum class ChorusPairing {
    OddEven,  // odd heads on beats 1,3; even heads on beats 2,4
    LeftRight // left half on beats 1,3; right half on beats 2,4
};

struct Params {
    ChorusPairing chorusPairing = ChorusPairing::OddEven;
    int burstEveryBars = 4; // within a Peak section: burst on bars 1, 1+N, 1+2N... of the section (bar 0 is the sweep; 1 = every bar)
    int chorusBeats = 2; // beats per flash: 1, 2 or 4
    float poolTilt = 12.0f;
    int poolBars = 2;
    float sweepStartTilt = 55.0f;
    float sweepEndTilt = 10.0f;
    float chorusTilt = 20.0f;
    float chorusSwing = 18.0f;
    float outroTilt = 40.0f;
    long burstMS = 500;
    long leadInMS = 1000;
    std::vector<float> tiltSlewDegPerSec; // per head, 0 = unknown/unlimited -> use leadInMS
    float minBeamFraction = 0.4f; // a beam is never trimmed below this fraction of its length to make room for a link
    long minGapMS = 25;
    int outroBars = 2;
    int minBarsForOutro = 6;
};

struct HeadEffect {
    int head = 0; // index into the left-to-right ordered head list
    long startMS = 0;
    long endMS = 0;
    float tilt0 = 0.0f;
    float tilt1 = 0.0f;
    bool linked = false;           // dark pre-position effect with "Link end position to next"
    std::vector<float> dimmer;     // x,y pairs, x = fraction of effect duration (empty for linked)
};

// bars: bar start times with a trailing end point (bars[i]..bars[i+1] is bar i).
// sections: optional, any order; bars inside a chorus section become chorus bars.
// headCount: heads ordered left to right. Fan position and beat banks are computed over all of them.
// activeHeads: indices (into that ordering) to emit effects for; empty means all.
std::vector<HeadEffect> Generate(const std::vector<long>& bars,
                                 const std::vector<Section>& sections,
                                 int headCount,
                                 const Params& params = Params(),
                                 const std::vector<int>& activeHeads = {});

// Builds the per-effect settings (EffectDB style "key=value,..." with escaped commas) and the
// white palette for one generated effect. fixture is the model's fixture value (1..8).
std::string BuildSettings(const HeadEffect& e, int fixture);
std::string BuildPalette();

}

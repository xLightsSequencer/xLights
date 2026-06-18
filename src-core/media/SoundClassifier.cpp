/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Apple-only `ClassifySound` shell. The SoundAnalysis.framework calls
// live behind `AppleSoundClassifierBridge` in
// `macOS/src-apple-core/media/SoundClassifierBridge.{h,mm}`. This file
// pulls raw samples out of the AudioManager, calls the bridge, and
// applies `SoundClassifierOptions` filtering / sorting / truncation.

#ifdef __APPLE__

#include "SoundClassifier.h"
#include "AudioManager.h"
#include "media/SoundClassifierBridge.h"

#include <algorithm>
#include <utility>

SoundClassification ClassifySound(AudioManager* audio,
                                   const SoundClassifierOptions& opts) {
    SoundClassification out;
    if (!audio || !audio->IsOk()) return out;

    long trackSize = audio->GetTrackSize();
    long rate = audio->GetRate();
    if (trackSize <= 0 || rate <= 0) return out;

    // Wait for the audio to finish loading before pulling samples.
    (void)audio->GetRawLeftDataPtr(trackSize - 1);
    const float* mono = audio->GetRawLeftDataPtr(0);
    if (!mono) return out;

    auto bridgeResult = AppleSoundClassifierBridge::ClassifyMono(
        mono, trackSize, double(rate), double(opts.windowSeconds));
    if (bridgeResult.didError) return out;

    std::vector<SoundClassResult> all;
    all.reserve(bridgeResult.classes.size());
    for (auto& c : bridgeResult.classes) {
        SoundClassResult r;
        r.name = std::move(c.name);
        r.confidence = std::move(c.confidence);
        if (!r.confidence.empty()) {
            double sum = 0;
            for (float v : r.confidence) sum += v;
            r.averageConfidence = float(sum / double(r.confidence.size()));
        }
        if (r.averageConfidence >= opts.minAverageConfidence) {
            all.push_back(std::move(r));
        }
    }
    std::sort(all.begin(), all.end(),
              [](const SoundClassResult& a, const SoundClassResult& b) {
                  return a.averageConfidence > b.averageConfidence;
              });
    if ((int)all.size() > opts.maxClasses) {
        all.resize((size_t)opts.maxClasses);
    }

    out.classes = std::move(all);
    out.timeStepSeconds = opts.windowSeconds > 0 ? opts.windowSeconds : 1.0f;
    out.lengthMS = audio->LengthMS();
    return out;
}

#endif // __APPLE__

#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "media/MediaCompatibility.h"

class SequenceElements;

namespace seqmedia {

// Rewrites every Video effect whose source file appears in `gifIssues` into
// a Pictures effect (which plays animated GIFs natively). Returns the count
// of rewritten effects. Caller is responsible for any UI refresh after a
// non-zero return.
int ConvertGifVideoEffectsToPictures(SequenceElements& elements,
                                     const std::vector<MediaCompatibilityIssue>& gifIssues);

struct VideoConversionResult {
    int attempted = 0;              // files that needed an ffmpeg transcode
    int converted = 0;              // of those, how many succeeded
    int effectsUpdated = 0;         // Video effects repointed at the new file
    int gifEffectsConverted = 0;    // Video effects turned into Pictures effects
    bool cancelled = false;         // caller's progress callback returned false
    std::vector<std::string> failures;              // "<filename>: <reason>"
    std::map<std::string, std::string> convertedFiles;  // source -> target
};

// Progress hooks. `onFile` runs before each transcode with the 0-based index,
// the total count, and the source path; `onFrame` is forwarded to the
// transcoder. Either returning false cancels the batch. Both may be null.
using FileProgressCallback = std::function<bool(size_t index, size_t count, const std::string& source)>;
using FrameProgressCallback = std::function<bool(int frame, int totalFrames)>;

// Transcode every convertible video issue to a supported format and repoint
// the sequence's Video effects at the results. Animated GIFs are not
// transcoded - their effects become Pictures effects instead. Audio issues are
// ignored; they need re-encoding outside xLights.
//
// On cancel, files already written are left on disk but NO effect is rewritten
// - a sequence still pointing at its originals is the least-surprising outcome.
// The caller is responsible for saving the sequence and for any UI refresh.
VideoConversionResult ConvertIncompatibleVideos(SequenceElements& elements,
                                                const std::vector<MediaCompatibilityIssue>& issues,
                                                const FileProgressCallback& onFile = nullptr,
                                                const FrameProgressCallback& onFrame = nullptr);

}

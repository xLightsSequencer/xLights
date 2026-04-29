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

#include <string>
#include <vector>
#include <cstdint>

namespace xlights::wav {

// Writes a 32-bit IEEE float, interleaved stereo, RIFF/WAVE file at `path`.
// Returns true on success. Fails (returns false) if `left` and `right` differ
// in length, the file can't be opened, or any write fails. Designed for the
// "save AI-separated stems" path where buffers are already float [-1, 1].
bool WriteStereoFloatWav(const std::string& path,
                         const std::vector<float>& left,
                         const std::vector<float>& right,
                         uint32_t sampleRate);

}

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

}

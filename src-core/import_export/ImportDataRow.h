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

// Shared, wx-free per-frame colour-stream → effect conversion used by the
// legacy channel-data importers (HLS, Vixen 2, …). Ported from the desktop
// xLightsFrame::DoConvertDataRowToEffects path (src-ui-wx) so every core reader
// produces the same On / Color Wash synthesis the desktop emits.

#include "Color.h"

class EffectLayer;

// Convert a per-frame colour stream into On / Color Wash effects on `layer`.
// `colors` is consumed/mutated. `frameTime` is the ms-per-frame of the source.
// When `eraseExisting` is true the layer's existing effects are cleared first.
void ConvertDataRowToEffects(EffectLayer* layer, xlColorVector& colors, int frameTime, bool eraseExisting);

// Decode a base64-encoded string into raw bytes (wx-free analogue of the
// wxUtilities base64_decode the desktop importers call). Returns the number of
// trailing chars that couldn't be decoded.
int Base64Decode(const std::string& encoded, std::vector<unsigned char>& data);

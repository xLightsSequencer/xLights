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

// Cross-platform, framework-free registry of per-file maximum video decode
// sizes. Populated once before a full render by RenderEngine's pre-pass (which
// knows every video effect, its model's buffer size and its crop range) and
// consulted by the video decoder so it can emit frames already scaled to the
// largest size any effect actually needs — decoding a 4K clip at ~matrix size
// instead of native slashes cache memory and the per-frame scale cost.
//
// Keyed by the resolved absolute path (the same key the AVFoundation
// SharedDecoder is pooled under), so one entry governs all consumers of a file.
// Deliberately depends on nothing but the standard library so both src-core and
// the Apple-core bridge can use it across the include boundary.

#include <string>

namespace VideoDecodeSizeRegistry {

// Reset before each full render. Without this, an effect removed since the last
// render would keep pinning an oversized (or now-unneeded) decode.
void Clear();

// Record that the file at `resolvedPath` is rendered at up to (w,h) somewhere in
// the sequence; max-combined with any size already recorded for that path.
// No-op for empty paths or non-positive sizes.
void SetMaxDecodeSize(const std::string& resolvedPath, int w, int h);

// Returns the recorded max (w,h) for the file, or false if the pre-pass recorded
// nothing (decoder then decodes at native). Thread-safe.
bool GetMaxDecodeSize(const std::string& resolvedPath, int& w, int& h);

// True when nothing has been recorded — i.e. no pre-pass has run this session
// (e.g. a fresh sequence load whose first render is a per-model edit). Callers
// use it to trigger a populate before that first restricted render.
bool Empty();

} // namespace VideoDecodeSizeRegistry

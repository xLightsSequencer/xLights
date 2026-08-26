/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#pragma once

// Central declaration of the FFmpeg versions xLights supports.
//
// Supported range: FFmpeg 6.x, 7.x and 8.x
//
//   FFmpeg 6.x -> libavutil 58, libavcodec 60, libavformat 60
//   FFmpeg 7.x -> libavutil 59, libavcodec 61, libavformat 61
//   FFmpeg 8.x -> libavutil 60, libavcodec 62, libavformat 62
//
// The floor is 6.0 because that is what the oldest distribution xLights
// supports provides (Ubuntu 24.04 LTS ships 6.1); macOS and Windows take their
// FFmpeg from the xLights-dependencies bundle, which pins a single version for
// both. FFmpeg 5.x and earlier are NOT supported - they predate the
// AVChannelLayout API that this code uses unconditionally.
//
// Without this check, building against 5.x fails deep inside the media sources
// with confusing errors about ch_layout instead of saying what is wrong.

extern "C" {
#include <libavutil/version.h>
}

#if LIBAVUTIL_VERSION_MAJOR < 58
#error "xLights requires FFmpeg 6.0 or newer (libavutil >= 58). FFmpeg 5.x and earlier are no longer supported."
#endif

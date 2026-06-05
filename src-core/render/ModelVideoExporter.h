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

// wx-free model -> video export. Renders a model's channel data into RGBA
// frames (via FillXlImage) and feeds them to the backend-abstracted
// VideoWriter (AVFoundation on Apple, FFmpeg elsewhere). Shared by the
// desktop (xLightsFrame::WriteVideoModelFile) and available to the iPad app.

#include "media/VideoWriter.h"  // ProgressReportCb / QueryForCancelCb

#include <cstdint>
#include <string>

class Model;
class SequenceData;
class xlImage;

namespace ModelVideoExporter {

// Paint one frame of a model's render data into an RGBA xlImage. Handles
// model groups (each sub-model rendered at its own start channel) and the
// per-model RGB channel-order mapping. `invert` flips vertically.
void FillXlImage(xlImage& image, Model* model, uint8_t* framedata, int startAddr, bool invert);

// Export model frames [startFrame, endFrame) to a video file. Codec is chosen
// from the output extension plus the flags:
//   .mov forceProRes -> ProRes 4444 (4:4:4 near-lossless, decodes everywhere)
//   .mov             -> rawvideo when width%8==0 (bit-exact), else ProRes 4444
//   .avi             -> rawvideo (uncompressed; FFmpeg)
//   .mp4 highQuality -> HEVC constant-quality (near-visually-lossless)
//   .mp4 else        -> H.264 (compressed = lossy)
// Returns false on failure (also logged). `progress` (0..100) and `cancel`
// are optional hooks forwarded to the underlying VideoWriter — pass them to
// drive a progress dialog / allow aborting a long encode.
bool WriteModelVideo(const std::string& filename, SequenceData* dataBuf,
                     unsigned int startFrame, unsigned int endFrame,
                     Model* model, int startAddr,
                     bool compressed, bool highQuality, bool forceProRes,
                     ProgressReportCb progress = nullptr,
                     QueryForCancelCb cancel = nullptr);

} // namespace ModelVideoExporter

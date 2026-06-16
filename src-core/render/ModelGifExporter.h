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

class SequenceData;
class Model;

// wx-free animated-GIF export for a single model's rendered data. Shares the
// per-frame RGBA buffer building with ModelVideoExporter (FillXlImage) and
// encodes with the vendored public-domain gif-h, so both the desktop and the
// iPad app can export GIFs from the same core path.
namespace ModelGifExporter {

// Export model frames [startFrame, endFrame) to an animated GIF at `filename`.
// `startAddr` matches desktop DoExportModel (the model's absolute start
// channel); FillXlImage rebases the per-model buffer the same way the video
// exporter does. `frameTime` is the per-frame duration in milliseconds (the
// GIF delay is derived from it, rounded to the GIF's 1/100s resolution).
// Returns false on failure (also logged). The caller must ObtainAccessToURL on
// `filename` first (macOS / iPad sandbox).
bool WriteModelGif(const std::string& filename, SequenceData* dataBuf,
                   unsigned int startFrame, unsigned int endFrame,
                   Model* model, int startAddr, unsigned int frameTime);

} // namespace ModelGifExporter

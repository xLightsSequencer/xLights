/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "render/ModelGifExporter.h"

#include "render/ModelVideoExporter.h"
#include "render/SequenceData.h"
#include "models/Model.h"
#include "utils/xlImage.h"

#include <log.h>

#include <cstdint>

// gif-h defines its functions (non-static) in the header, so it must be
// included in exactly one translation unit. This is that unit.
#include "gif-h/gif.h"

namespace ModelGifExporter {

bool WriteModelGif(const std::string& filename, SequenceData* dataBuf,
                   unsigned int startFrame, unsigned int endFrame,
                   Model* model, int startAddr, unsigned int frameTime)
{
    spdlog::debug("Writing model GIF.");

    int width = 0;
    int height = 0;
    model->GetBufferSize("Default", "2D", "None", width, height, 0);
    if (width <= 0 || height <= 0) {
        spdlog::error("   Model GIF has invalid dimensions {}x{}.", width, height);
        return false;
    }
    if (endFrame <= startFrame) {
        spdlog::error("   Model GIF has no frames to write ({}..{}).", startFrame, endFrame);
        return false;
    }

    // GIF frame delay is in hundredths of a second; the sequence frame time is
    // in milliseconds. Round to the nearest centisecond, clamped to >= 1 so
    // viewers don't treat 0 as "as fast as possible".
    uint32_t delayCs = (frameTime + 5) / 10;
    if (delayCs == 0) {
        delayCs = 1;
    }

    spdlog::debug("   GIF dimensions {}x{}, {} frames, delay {}cs.", width, height, endFrame - startFrame, delayCs);

    GifWriter g{};
    if (!GifBegin(&g, filename.c_str(), (uint32_t)width, (uint32_t)height, delayCs)) {
        spdlog::error("   Failed to open GIF file for writing: {}", filename);
        return false;
    }

    const size_t pixelCount = (size_t)width * (size_t)height;
    bool ok = true;
    for (unsigned int f = startFrame; f < endFrame; ++f) {
        // Fresh (black, opaque-on-lit) RGBA frame, flipped to the same
        // convention the video exporter uses.
        xlImage image(width, height);
        ModelVideoExporter::FillXlImage(image, model, (uint8_t*)&(*dataBuf)[f][0], startAddr, /*invert*/ true);

        uint8_t* rgba = image.GetData();

        // Normalize near-blacks to true black so the GIF quantizer doesn't
        // spend palette slots on dark noise (matches the old wx exporter).
        constexpr uint8_t BLACK_THRESHOLD = 5;
        for (size_t p = 0; p < pixelCount; ++p) {
            uint8_t* px = rgba + p * 4;
            if (px[0] < BLACK_THRESHOLD && px[1] < BLACK_THRESHOLD && px[2] < BLACK_THRESHOLD) {
                px[0] = px[1] = px[2] = 0;
            }
        }

        if (!GifWriteFrame(&g, rgba, (uint32_t)width, (uint32_t)height, delayCs)) {
            spdlog::error("   Failed writing GIF frame {}.", f);
            ok = false;
            break;
        }
    }

    if (!GifEnd(&g)) {
        spdlog::error("   Failed finalizing GIF file: {}", filename);
        ok = false;
    }

    if (ok) {
        spdlog::debug("Model GIF written successfully.");
    }
    return ok;
}

} // namespace ModelGifExporter

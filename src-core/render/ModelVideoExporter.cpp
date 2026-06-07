/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "render/ModelVideoExporter.h"

#include "media/VideoWriter.h"
#include "render/SequenceData.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "utils/xlImage.h"
#include "utils/xlPoint.h"
#include "utils/Color.h"
#include "utils/string_utils.h"

#include <log.h>

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace ModelVideoExporter {

static void RenderModelOnXlImage(xlImage& image, Model* model, uint8_t* framedata, int startAddr, int x = 0, int y = 0, bool invert = false)
{
    int outheight = image.GetHeight();
    int outwidth = image.GetWidth();
    uint8_t* imagedata = image.GetData();

    int chs = model->GetChanCountPerNode();
    uint8_t* ps = framedata + startAddr;

    char r = model->GetChannelColorLetter(0);
    int rr = 0, gg = 1, bb = 2;
    if (r == 'G') gg = 0;
    else if (r == 'B') bb = 0;
    char g = model->GetChannelColorLetter(1);
    if (g == 'R') rr = 1;
    else if (g == 'B') bb = 1;
    char b = model->GetChannelColorLetter(2);
    if (b == 'R') rr = 2;
    else if (b == 'G') gg = 2;

    for (size_t i = 0; i < model->GetNodeCount(); i++) {
        xlColor c = model->GetNodeColor(i);
        std::vector<xlPoint> pts;
        model->GetNodeCoords(i, pts);

        for (const auto& it : pts) {
            int xx = x + it.x;
            int yy = y + it.y;
            if (invert) yy = outheight - yy - 1;

            if (xx >= 0 && xx < outwidth && yy >= 0 && yy < outheight) {
                uint8_t* p = imagedata + (yy * outwidth + xx) * 4; // RGBA
                if (chs == 1) {
                    p[0] = c.Red();
                    p[1] = c.Green();
                    p[2] = c.Blue();
                } else {
                    p[0] = *(ps + rr);
                    p[1] = *(ps + gg);
                    p[2] = *(ps + bb);
                }
                p[3] = 255; // fully opaque
            }
        }
        ps += chs;
    }
}

void FillXlImage(xlImage& image, Model* model, uint8_t* framedata, int startAddr, bool invert)
{
    if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
        ModelGroup* mg = static_cast<ModelGroup*>(model);
        for (auto m = mg->Models().begin(); m != mg->Models().end(); ++m) {
            int start = (*m)->GetFirstChannel() - startAddr;
            RenderModelOnXlImage(image, *m, framedata, start, 0, 0, invert);
        }
    } else {
        RenderModelOnXlImage(image, model, framedata, model->GetFirstChannel() - startAddr + 1, 0, 0, invert);
    }
}

bool WriteModelVideo(const std::string& filename, SequenceData* dataBuf,
                     unsigned int startFrame, unsigned int endFrame,
                     Model* model, int startAddr,
                     bool compressed, bool highQuality, bool forceProRes,
                     int exportWidth, int exportHeight,
                     ProgressReportCb progress, QueryForCancelCb cancel)
{
    spdlog::debug("Writing model video.");

    int origwidth = 0;
    int origheight = 0;
    model->GetBufferSize("Default", "2D", "None", origwidth, origheight, 0);
    if (origwidth <= 0 || origheight <= 0) {
        spdlog::error("   Model video has invalid dimensions {}x{}.", origwidth, origheight);
        return false;
    }

    // Compute output dimensions. When exportWidth/exportHeight are given,
    // scale up using the largest integer scale factor that fits the target
    // while preserving the model's aspect ratio; centre the result and fill
    // the remainder with black.
    int outW = origwidth;
    int outH = origheight;
    int scaledW = origwidth;
    int scaledH = origheight;
    int offsetX = 0;
    int offsetY = 0;
    bool needsScale = false;
    if (exportWidth > 0 && exportHeight > 0) {
        int scaleX = exportWidth / origwidth;
        int scaleY = exportHeight / origheight;
        int scale = std::max(1, std::min(scaleX, scaleY));
        outW = exportWidth;
        outH = exportHeight;
        scaledW = origwidth * scale;
        scaledH = origheight * scale;
        offsetX = (outW - scaledW) / 2;
        offsetY = (outH - scaledH) / 2;
        needsScale = (scale > 1 || outW != origwidth || outH != origheight);
        spdlog::debug("  4K upscale: {}x{} -> {}x{} (scale={}, offset={},{})",
                      origwidth, origheight, outW, outH, scale, offsetX, offsetY);
    }

    const bool isAvi = EndsWith(filename, ".avi");
    const bool isMov = EndsWith(filename, ".mov");

    // Codec policy:
    //   .mov forceProRes -> always ProRes 4444 (4:4:4, near-lossless, much
    //                    smaller than rawvideo, decodes everywhere; AVFoundation
    //                    on macOS / prores_ks on FFmpeg).
    //   .mov          -> rawvideo when width%8==0 (bit-exact, AVFoundation BGRA
    //                    passthrough), else ProRes 4444 — both AVFoundation-decodable.
    //   .avi          -> rawvideo (uncompressed; external tooling only, FFmpeg).
    //   .mp4 highQuality -> HEVC at a generous bitrate: AVFoundation-encodable,
    //                    near-visually-lossless, much higher quality than the
    //                    standard Compressed mode.
    //   .mp4 else     -> H.264 (compressed = lossy).
    VideoWriterParams params;
    params.width = outW;
    params.height = outH;
    params.fps = std::max(1u, 1000u / dataBuf->FrameTime());
    params.audioSampleRate = 0;       // video only
    params.inputChannels = 3;         // RGB24 extracted from the RGBA xlImage
    params.cpuFrames = true;          // frames are produced on the CPU
    params.lossless = !compressed;
    if (isMov && forceProRes) {
        params.videoCodec = "ProRes";
        params.lossless = false;      // ProRes is its own near-lossless codec
    } else if (isMov) {
        params.videoCodec = (outW % 8 == 0) ? "rawvideo" : "ProRes";
    } else if (isAvi) {
        params.videoCodec = "rawvideo";
    } else if (highQuality) {
        params.videoCodec = "H.265";
        params.lossless = false;
        // Constant-quality HEVC at the top of the scale (AVFoundation
        // AVVideoQualityKey). Crisper than any average-bitrate target, which
        // VideoToolbox undershoots on easy LED content. Not bit-exact (still
        // 4:2:0), but the best lossy quality — between H.264 and rawvideo.
        params.videoQuality = 1.0;
    } else {
        params.videoCodec = "H.264";
    }

    const unsigned lastFrame = (endFrame > startFrame) ? (endFrame - 1) : startFrame;

    // Capture scale variables for the lambda (origwidth/origheight always needed).
    const int captOrigW = origwidth;
    const int captOrigH = origheight;
    const int captOutW = outW;
    const int captOutH = outH;
    const int captScaledW = scaledW;
    const int captScaledH = scaledH;
    const int captOffX = offsetX;
    const int captOffY = offsetY;
    const bool captScale = needsScale;

    try {
        VideoWriter writer(filename, params, /*videoOnly*/ true);
        if (progress) {
            writer.setProgressReportCallback(progress);
        }
        if (cancel) {
            writer.setQueryForCancelCallback(cancel);
        }

        writer.setGetVideoCallback([&](VideoWriterFrame& vf, unsigned frameIndex) {
            // Clamp tail over-reads: the FFmpeg encoder may pull a few frames
            // past the requested count while flushing its pipeline.
            unsigned dataIdx = startFrame + frameIndex;
            if (dataIdx > lastFrame) {
                dataIdx = lastFrame;
            }
            // Fresh (black, opaque-on-lit) RGBA frame, flipped to model-video
            // convention via invert, then RGB extracted into the writer buffer.
            xlImage image(captOrigW, captOrigH);
            FillXlImage(image, model, (uint8_t*)&(*dataBuf)[dataIdx][0], startAddr, /*invert*/ true);
            const uint8_t* rgba = image.GetData();

            if (!captScale) {
                for (int p = 0; p < captOrigW * captOrigH; ++p) {
                    vf.rgbBuffer[p * 3 + 0] = rgba[p * 4 + 0];
                    vf.rgbBuffer[p * 3 + 1] = rgba[p * 4 + 1];
                    vf.rgbBuffer[p * 3 + 2] = rgba[p * 4 + 2];
                }
            } else {
                // Nearest-neighbour upscale: each source pixel expands to an
                // integer block; unused border pixels stay black.
                std::memset(vf.rgbBuffer, 0, captOutW * captOutH * 3);
                const int scaleX2 = captScaledW / captOrigW;
                const int scaleY2 = captScaledH / captOrigH;
                for (int sy = 0; sy < captOrigH; ++sy) {
                    for (int sx = 0; sx < captOrigW; ++sx) {
                        const uint8_t* src = rgba + (sy * captOrigW + sx) * 4;
                        for (int by = 0; by < scaleY2; ++by) {
                            int dy = captOffY + sy * scaleY2 + by;
                            uint8_t* dstRow = vf.rgbBuffer + (dy * captOutW + captOffX + sx * scaleX2) * 3;
                            for (int bx = 0; bx < scaleX2; ++bx) {
                                dstRow[bx * 3 + 0] = src[0];
                                dstRow[bx * 3 + 1] = src[1];
                                dstRow[bx * 3 + 2] = src[2];
                            }
                        }
                    }
                }
            }
            return true;
        });

        writer.initialize();
        writer.exportFrames(static_cast<int>(endFrame - startFrame));
        writer.completeExport();
        spdlog::debug("Model video written successfully (via {}).",
                      writer.usingAVFoundation() ? "AVFoundation" : "FFmpeg");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Model video export failed: {}", e.what());
        return false;
    }
}

} // namespace ModelVideoExporter

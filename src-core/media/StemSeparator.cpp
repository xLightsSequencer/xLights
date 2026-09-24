// HTDemucs 4-stem source separation. Three backends, selected by the
// build:
//   __APPLE__       → CoreML via macOS/src-apple-core bridge
//   HAVE_OPENVINO   → OpenVINO (cmake / Linux)
//   HAVE_ORT        → ONNX Runtime + DirectML (VS / Windows)
// STFT preprocessing and chunk crossfade are shared across all three
// backends (pure C++, no inference framework dependency).

#include "StemSeparator.h"
#include "AudioManager.h"
#include "kiss_fft/tools/kiss_fftr.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <optional>
#include <vector>

#ifdef __APPLE__
#include "media/StemSeparatorBridge.h"
#endif

// ─────────────────────────────────────────────────────────────────────────────
// STFT helpers — shared by all backends
// ─────────────────────────────────────────────────────────────────────────────

static constexpr int kSTFT_NFFT       = 4096;
static constexpr int kSTFT_HOP        = 1024;
static constexpr int kSTFT_BINS       = 2048;
static constexpr int kSTFT_FRAMES     = 336;
static constexpr int kSTFT_PAD_LEFT   = 1536;
static constexpr int kSTFT_PADDED_LEN = 347136;

namespace {

std::vector<float> MakeHannWindow(int n) {
    const float pi = 3.14159265358979f;
    std::vector<float> w(n);
    for (int i = 0; i < n; ++i)
        w[i] = 0.5f * (1.0f - std::cos(2.0f * pi * float(i) / float(n - 1)));
    return w;
}

void FillSpectralTensor(float* out,
                        const float* L, const float* R,
                        long start, long validCount,
                        kiss_fftr_cfg cfg,
                        const std::vector<float>& window,
                        std::vector<float>& padded,
                        std::vector<float>& frame,
                        std::vector<kiss_fft_cpx>& fftBuf) {
    const long strideC = (long)kSTFT_BINS * kSTFT_FRAMES;
    const long strideF = kSTFT_FRAMES;
    for (int ch = 0; ch < 2; ch++) {
        const float* src = (ch == 0) ? L : R;
        std::fill(padded.begin(), padded.end(), 0.0f);
        for (long i = 0; i < validCount; i++)
            padded[kSTFT_PAD_LEFT + i] = src[start + i];
        for (int t = 0; t < kSTFT_FRAMES; t++) {
            for (int k = 0; k < kSTFT_NFFT; k++)
                frame[k] = padded[t * kSTFT_HOP + k] * window[k];
            kiss_fftr(cfg, frame.data(), fftBuf.data());
            const long chReal = ch * 2;
            const long chImag = ch * 2 + 1;
            for (int k = 0; k < kSTFT_BINS; k++) {
                out[chReal * strideC + k * strideF + t] = fftBuf[k].r;
                out[chImag * strideC + k * strideF + t] = fftBuf[k].i;
            }
        }
    }
}

void AppendOutputs(const float* src,
                   long chunkFrames, long validCount,
                   long dstOffset, long overlap,
                   std::vector<float>& drumsL,  std::vector<float>& drumsR,
                   std::vector<float>& bassL,   std::vector<float>& bassR,
                   std::vector<float>& otherL,  std::vector<float>& otherR,
                   std::vector<float>& vocalsL, std::vector<float>& vocalsR) {
    std::vector<float>* targets[8] = {
        &drumsL, &drumsR, &bassL, &bassR, &vocalsL, &vocalsR, &otherL, &otherR
    };
    for (int ch = 0; ch < 8; ch++) {
        std::vector<float>& dst = *targets[ch];
        const float* row = src + (long)ch * chunkFrames;
        for (long i = 0; i < validCount; i++) {
            long idx = dstOffset + i;
            if (idx < 0 || idx >= (long)dst.size()) continue;
            float v = row[i];
            if (overlap > 0 && i < overlap) {
                float t = float(i) / float(overlap);
                dst[idx] = dst[idx] * (1.0f - t) + v * t;
            } else {
                dst[idx] = v;
            }
        }
    }
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Backend-specific includes
// ─────────────────────────────────────────────────────────────────────────────
#if !defined(__APPLE__) && defined(HAVE_OPENVINO)
#    include <openvino/openvino.hpp>
#endif

#if !defined(__APPLE__) && defined(HAVE_ORT)
#    include "../utils/SpecialOptions.h"
#    include <onnxruntime_cxx_api.h>
// Pragma only needed for the VS-native build; cmake links via target_link_libraries.
#    if defined(_MSC_VER) && !defined(XLIGHTS_CMAKE_BUILD)
#        pragma comment(lib, "onnxruntime.lib")
#    endif
// Forward-declare DirectML EP entry point — avoids pulling in dml_provider_factory.h
// (which requires d3d12.h + DirectML.h).
extern "C" OrtStatus* ORT_API_CALL OrtSessionOptionsAppendExecutionProvider_DML(
    OrtSessionOptions* options, int device_id);
#endif

// ─────────────────────────────────────────────────────────────────────────────
// SeparateStems — public API
// ─────────────────────────────────────────────────────────────────────────────
bool SeparateStems(AudioManager* audio,
                   const std::string& modelPath,
                   StemOutput& out,
                   const StemSeparatorOptions& opts,
                   std::function<void(int pct)> progress,
                   const std::atomic<bool>* cancel) {
    if (!audio || !audio->IsOk()) {
        spdlog::error("SeparateStems: no usable audio loaded");
        return false;
    }
    if (modelPath.empty()) {
        spdlog::error("SeparateStems: no model path");
        return false;
    }
    {
        std::error_code ec;
        auto modelBytes = std::filesystem::file_size(modelPath, ec);
        spdlog::info("SeparateStems: starting - model '{}' ({} bytes{}), audio {} frames at {} Hz, chunk {} overlap {}",
                     modelPath, ec ? 0 : (unsigned long long)modelBytes, ec ? ", size unreadable" : "",
                     audio->GetTrackSize(), audio->GetRate(), opts.chunkSamples, opts.overlapSamples);
    }

// ── CoreML (Apple) ───────────────────────────────────────────────────────────
#ifdef __APPLE__

    long trackSize = audio->GetTrackSize();
    long rate      = audio->GetRate();
    if (trackSize <= 0 || rate <= 0) return false;

    (void)audio->GetRawLeftDataPtr(trackSize - 1);
    const float* srcL = audio->GetRawLeftDataPtr(0);
    const float* srcR = audio->GetRawRightDataPtr(0);
    if (!srcL) return false;
    if (!srcR) srcR = srcL;

    auto* model = AppleStemSeparatorBridge::LoadModel(modelPath);
    if (!model) return false;

    const long chunkFrames = opts.chunkSamples;
    const long overlap = std::max<long>(0, std::min<long>(opts.overlapSamples, chunkFrames / 2));
    const long stride = chunkFrames - overlap;
    if (stride <= 0) {
        AppleStemSeparatorBridge::DestroyModel(model);
        return false;
    }

    out.drumsL.assign(trackSize, 0.0f);  out.drumsR.assign(trackSize, 0.0f);
    out.bassL.assign(trackSize, 0.0f);   out.bassR.assign(trackSize, 0.0f);
    out.otherL.assign(trackSize, 0.0f);  out.otherR.assign(trackSize, 0.0f);
    out.vocalsL.assign(trackSize, 0.0f); out.vocalsR.assign(trackSize, 0.0f);
    out.sampleRate = rate;

    kiss_fftr_cfg fftCfg = kiss_fftr_alloc(kSTFT_NFFT, 0, nullptr, nullptr);
    if (!fftCfg) {
        spdlog::error("SeparateStems: kiss_fftr_alloc failed");
        AppleStemSeparatorBridge::DestroyModel(model);
        return false;
    }
    auto hannWindow = MakeHannWindow(kSTFT_NFFT);
    std::vector<float>        padded(kSTFT_PADDED_LEN, 0.0f);
    std::vector<float>        frame(kSTFT_NFFT);
    std::vector<kiss_fft_cpx> fftBuf(kSTFT_NFFT / 2 + 1);

    std::vector<float> waveformBuf(2 * (size_t)chunkFrames);
    std::vector<float> spectralBuf(4 * (size_t)kSTFT_BINS * (size_t)kSTFT_FRAMES);
    std::vector<float> timeOutBuf(8 * (size_t)chunkFrames);

    const long totalChunks = (trackSize + stride - 1) / stride;
    long chunkIdx = 0;
    bool cancelled = false;

    for (long srcPos = 0; srcPos < trackSize; srcPos += stride) {
        if (cancel && cancel->load()) {
            cancelled = true;
            break;
        }
        long validCount = std::min<long>(chunkFrames, trackSize - srcPos);

        std::fill(waveformBuf.begin(), waveformBuf.end(), 0.0f);
        std::memcpy(waveformBuf.data(),               srcL + srcPos, validCount * sizeof(float));
        std::memcpy(waveformBuf.data() + chunkFrames, srcR + srcPos, validCount * sizeof(float));

        std::fill(spectralBuf.begin(), spectralBuf.end(), 0.0f);
        FillSpectralTensor(spectralBuf.data(), srcL, srcR, srcPos, validCount,
                           fftCfg, hannWindow, padded, frame, fftBuf);

        if (!AppleStemSeparatorBridge::RunChunk(
                model,
                waveformBuf.data(), (long)waveformBuf.size(),
                spectralBuf.data(), (long)spectralBuf.size(),
                timeOutBuf.data(), (long)timeOutBuf.size())) {
            spdlog::error("SeparateStems: bridge inference failed at chunk {}", chunkIdx);
            free(fftCfg);
            AppleStemSeparatorBridge::DestroyModel(model);
            return false;
        }

        AppendOutputs(timeOutBuf.data(), chunkFrames, validCount,
                      srcPos, chunkIdx == 0 ? 0 : overlap,
                      out.drumsL, out.drumsR, out.bassL, out.bassR,
                      out.otherL, out.otherR, out.vocalsL, out.vocalsR);

        chunkIdx++;
        if (progress) {
            int pct = (int)((chunkIdx * 100) / std::max<long>(1, totalChunks));
            progress(std::min(100, pct));
        }
        if (srcPos + chunkFrames >= trackSize) break;
    }

    free(fftCfg);
    AppleStemSeparatorBridge::DestroyModel(model);
    if (cancelled) {
        spdlog::info("SeparateStems: cancelled after {} of {} chunks", chunkIdx, totalChunks);
        return false;
    }
    spdlog::info("SeparateStems: completed {} chunks, {} frames", chunkIdx, trackSize);
    return true;

// ── OpenVINO ─────────────────────────────────────────────────────────────────
#elif defined(HAVE_OPENVINO)

    long trackSize = audio->GetTrackSize();
    long rate      = audio->GetRate();
    if (trackSize <= 0 || rate <= 0) return false;

    (void)audio->GetRawLeftDataPtr(trackSize - 1);
    const float* srcL = audio->GetRawLeftDataPtr(0);
    const float* srcR = audio->GetRawRightDataPtr(0);
    if (!srcL) return false;
    if (!srcR) srcR = srcL;

    ov::Core core;
    std::shared_ptr<ov::Model> model;
    try {
        model = core.read_model(modelPath);
    } catch (const std::exception& e) {
        spdlog::error("SeparateStems: OpenVINO read_model failed: {}", e.what());
        return false;
    }

    for (const auto& inp : model->inputs())
        spdlog::debug("SeparateStems: input '{}' {}", inp.get_any_name(),
                      inp.get_partial_shape().to_string());
    for (const auto& outp : model->outputs())
        spdlog::debug("SeparateStems: output '{}' {}", outp.get_any_name(),
                      outp.get_partial_shape().to_string());

    ov::CompiledModel compiled;
    try {
        compiled = core.compile_model(model, "AUTO");
    } catch (const std::exception& e) {
        spdlog::error("SeparateStems: compile_model failed: {}", e.what());
        return false;
    }

    ov::InferRequest req = compiled.create_infer_request();

    const long chunkFrames = opts.chunkSamples;
    const long overlap = std::max<long>(0, std::min<long>(opts.overlapSamples, chunkFrames / 2));
    const long stride  = chunkFrames - overlap;
    if (stride <= 0) return false;

    out.drumsL.assign(trackSize, 0.0f);  out.drumsR.assign(trackSize, 0.0f);
    out.bassL.assign(trackSize, 0.0f);   out.bassR.assign(trackSize, 0.0f);
    out.otherL.assign(trackSize, 0.0f);  out.otherR.assign(trackSize, 0.0f);
    out.vocalsL.assign(trackSize, 0.0f); out.vocalsR.assign(trackSize, 0.0f);
    out.sampleRate = rate;

    kiss_fftr_cfg fftCfg = kiss_fftr_alloc(kSTFT_NFFT, 0, nullptr, nullptr);
    if (!fftCfg) { spdlog::error("SeparateStems: kiss_fftr_alloc failed"); return false; }
    auto hannWindow = MakeHannWindow(kSTFT_NFFT);
    std::vector<float>        padded(kSTFT_PADDED_LEN, 0.0f);
    std::vector<float>        frame(kSTFT_NFFT);
    std::vector<kiss_fft_cpx> fftBuf(kSTFT_NFFT / 2 + 1);

    ov::Tensor waveformTensor(ov::element::f32, { 1, 2, (size_t)chunkFrames });
    ov::Tensor spectralTensor(ov::element::f32, { 1, 4, kSTFT_BINS, kSTFT_FRAMES });
    float* waveformData = waveformTensor.data<float>();
    float* spectralData = spectralTensor.data<float>();

    const long totalChunks = (trackSize + stride - 1) / stride;
    long chunkIdx = 0;
    bool cancelled = false;

    for (long srcPos = 0; srcPos < trackSize; srcPos += stride) {
        if (cancel && cancel->load()) {
            cancelled = true;
            break;
        }
        long validCount = std::min<long>(chunkFrames, trackSize - srcPos);

        std::memset(waveformData, 0, sizeof(float) * 2 * chunkFrames);
        std::memcpy(waveformData,               srcL + srcPos, validCount * sizeof(float));
        std::memcpy(waveformData + chunkFrames, srcR + srcPos, validCount * sizeof(float));

        std::memset(spectralData, 0, sizeof(float) * 4 * kSTFT_BINS * kSTFT_FRAMES);
        FillSpectralTensor(spectralData, srcL, srcR, srcPos, validCount,
                           fftCfg, hannWindow, padded, frame, fftBuf);

        try {
            req.set_tensor("audio_waveform",     waveformTensor);
            req.set_tensor("spectral_magnitude", spectralTensor);
            req.infer();
        } catch (const std::exception& e) {
            spdlog::error("SeparateStems: inference failed at chunk {}: {}", chunkIdx, e.what());
            free(fftCfg);
            return false;
        }

        ov::Tensor outTensor = req.get_tensor("time_output");
        AppendOutputs(outTensor.data<float>(), chunkFrames, validCount,
                      srcPos, chunkIdx == 0 ? 0 : overlap,
                      out.drumsL, out.drumsR, out.bassL, out.bassR,
                      out.otherL, out.otherR, out.vocalsL, out.vocalsR);

        chunkIdx++;
        if (progress) {
            int pct = (int)((chunkIdx * 100) / std::max<long>(1, totalChunks));
            progress(std::min(100, pct));
        }
        if (srcPos + chunkFrames >= trackSize) break;
    }

    free(fftCfg);
    if (cancelled) {
        spdlog::info("SeparateStems: cancelled after {} of {} chunks", chunkIdx, totalChunks);
        return false;
    }
    spdlog::info("SeparateStems: completed {} chunks, {} frames", chunkIdx, trackSize);
    return true;

// ── ONNX Runtime + DirectML ──────────────────────────────────────────────────
#elif defined(HAVE_ORT)

    long trackSize = audio->GetTrackSize();
    long rate      = audio->GetRate();
    if (trackSize <= 0 || rate <= 0) return false;

    (void)audio->GetRawLeftDataPtr(trackSize - 1);
    const float* srcL = audio->GetRawLeftDataPtr(0);
    const float* srcR = audio->GetRawRightDataPtr(0);
    if (!srcL) return false;
    if (!srcR) srcR = srcL;

    spdlog::info("SeparateStems: ONNX Runtime {}", Ort::GetVersionString());
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "xLights_demucs");
    std::wstring wpath(modelPath.begin(), modelPath.end());

    const long chunkFrames = opts.chunkSamples;
    const long overlap = std::max<long>(0, std::min<long>(opts.overlapSamples, chunkFrames / 2));
    const long stride  = chunkFrames - overlap;
    if (stride <= 0) return false;

    out.sampleRate = rate;

    std::vector<float> waveformBuf(2 * (size_t)chunkFrames);
    const int64_t waveformShape[] = {1, 2, (int64_t)chunkFrames};
    auto memInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    // Stem separation doesn't follow the GPU rendering preference; this lets a
    // user whose GPU grinds on HTDemucs fall back to CPU from special.options.
    const bool forceCPU = SpecialOptions::GetOption("StemSeparationCPU", "false") == "true";
    if (forceCPU) {
        spdlog::info("SeparateStems: StemSeparationCPU special option set, skipping DirectML");
    }

    auto makeSession = [&](bool withDML) {
        auto t0 = std::chrono::steady_clock::now();
        Ort::SessionOptions sopts;
        sopts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        sopts.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
        if (withDML) {
            try {
                Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_DML(sopts, 0));
                spdlog::info("SeparateStems: DirectML GPU provider enabled (device 0)");
            } catch (const Ort::Exception& e) {
                spdlog::warn("SeparateStems: DirectML unavailable ({}), using CPU", e.what());
            }
        } else {
            spdlog::info("SeparateStems: running on CPU");
        }
        Ort::Session s(env, wpath.c_str(), sopts);
        spdlog::info("SeparateStems: session created in {} ms",
                     std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count());
        return s;
    };

    std::optional<Ort::Session> sessionHolder;
    try {
        sessionHolder.emplace(makeSession(!forceCPU));
    } catch (const Ort::Exception& e) {
        spdlog::error("SeparateStems: session creation failed: {}", e.what());
        return false;
    }
    Ort::Session& session = *sessionHolder;

    Ort::AllocatorWithDefaultOptions allocator;
    auto inputName0  = session.GetInputNameAllocated(0, allocator);
    auto outputName0 = session.GetOutputNameAllocated(0, allocator);
    auto outShape    = session.GetOutputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    // [1, S, 2, N] where S = number of stems (4 or 6). We map first 4: drums/bass/other/vocals.
    const bool isFourDim = (outShape.size() == 4);
    std::string shapeStr;
    for (auto d : outShape) {
        shapeStr += (shapeStr.empty() ? "" : ",") + std::to_string(d);
    }
    spdlog::info("SeparateStems: output '{}' shape [{}] ({} dims)", outputName0.get(), shapeStr, outShape.size());

    const char* inputNames[]  = {inputName0.get()};
    const char* outputNames[] = {outputName0.get()};
    const long totalChunks = (trackSize + stride - 1) / stride;
    spdlog::info("SeparateStems: {} chunks of {} samples (stride {})", totalChunks, chunkFrames, stride);

    for (int attempt = 0; attempt < 2; attempt++) {
        const bool onGPU = attempt == 0 && !forceCPU;
        const auto runStart = std::chrono::steady_clock::now();
        out.drumsL.assign(trackSize, 0.0f);  out.drumsR.assign(trackSize, 0.0f);
        out.bassL.assign(trackSize, 0.0f);   out.bassR.assign(trackSize, 0.0f);
        out.otherL.assign(trackSize, 0.0f);  out.otherR.assign(trackSize, 0.0f);
        out.vocalsL.assign(trackSize, 0.0f); out.vocalsR.assign(trackSize, 0.0f);

        bool needRetry = false;
        bool cancelled = false;
        long chunkIdx = 0;

        try {
            for (long srcPos = 0; srcPos < trackSize; srcPos += stride) {
                if (cancel && cancel->load()) {
                    cancelled = true;
                    break;
                }
                long validCount = std::min<long>(chunkFrames, trackSize - srcPos);

                std::fill(waveformBuf.begin(), waveformBuf.end(), 0.0f);
                std::memcpy(waveformBuf.data(),               srcL + srcPos, validCount * sizeof(float));
                std::memcpy(waveformBuf.data() + chunkFrames, srcR + srcPos, validCount * sizeof(float));

                Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
                    memInfo, waveformBuf.data(), waveformBuf.size(), waveformShape, 3);

                const auto chunkStart = std::chrono::steady_clock::now();
                if (chunkIdx == 0) {
                    spdlog::info("SeparateStems: running first chunk on {}", onGPU ? "DirectML" : "CPU");
                }
                auto outputs = sessionHolder->Run(Ort::RunOptions{nullptr},
                                                 inputNames, &inputTensor, 1,
                                                 outputNames, 1);
                spdlog::info("SeparateStems: chunk {}/{} took {} ms", chunkIdx + 1, totalChunks,
                             std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - chunkStart).count());
                if (chunkIdx == 0) {
                    auto actual = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
                    std::string actualStr;
                    for (auto d : actual) {
                        actualStr += (actualStr.empty() ? "" : ",") + std::to_string(d);
                    }
                    spdlog::info("SeparateStems: first chunk output shape [{}]", actualStr);
                }

                const float* outData = outputs[0].GetTensorMutableData<float>();

                if (isFourDim) {
                    // [1, S, 2, N] — read first 4 stems: drums/bass/other/vocals
                    for (long i = 0; i < validCount; i++) {
                        long dstIdx = srcPos + i;
                        if (dstIdx >= trackSize) break;
                        auto get = [&](int stem, int ch) {
                            return outData[stem * 2 * chunkFrames + ch * chunkFrames + i];
                        };
                        auto blend = [&](std::vector<float>& dst, float v) {
                            if (chunkIdx > 0 && i < overlap) {
                                float t = float(i) / float(overlap);
                                dst[dstIdx] = dst[dstIdx] * (1.0f - t) + v * t;
                            } else {
                                dst[dstIdx] = v;
                            }
                        };
                        blend(out.drumsL,  get(0, 0)); blend(out.drumsR,  get(0, 1));
                        blend(out.bassL,   get(1, 0)); blend(out.bassR,   get(1, 1));
                        blend(out.otherL,  get(2, 0)); blend(out.otherR,  get(2, 1));
                        blend(out.vocalsL, get(3, 0)); blend(out.vocalsR, get(3, 1));
                    }
                } else {
                    // [1, 8, N] — interleaved channels
                    AppendOutputs(outData, chunkFrames, validCount,
                                  srcPos, chunkIdx == 0 ? 0 : overlap,
                                  out.drumsL, out.drumsR, out.bassL, out.bassR,
                                  out.otherL, out.otherR, out.vocalsL, out.vocalsR);
                }

                chunkIdx++;
                if (progress) {
                    int pct = (int)((chunkIdx * 100) / std::max<long>(1, totalChunks));
                    progress(std::min(100, pct));
                }
                if (srcPos + chunkFrames >= trackSize) break;
            }
        } catch (const Ort::Exception& e) {
            if (cancelled) {
                return false;
            }
            if (attempt == 0 && !forceCPU) {
                spdlog::warn("SeparateStems: DirectML inference failed at chunk {} ({}), retrying on CPU", chunkIdx, e.what());
                try {
                    sessionHolder.reset();
                    sessionHolder.emplace(makeSession(false));
                } catch (const Ort::Exception& e2) {
                    spdlog::error("SeparateStems: CPU session creation failed: {}", e2.what());
                    return false;
                }
                needRetry = true;
            } else {
                spdlog::error("SeparateStems: CPU inference failed: {}", e.what());
                return false;
            }
        }

        if (cancelled) {
            spdlog::info("SeparateStems: cancelled after {} of {} chunks", chunkIdx, totalChunks);
            return false;
        }
        if (!needRetry) {
            spdlog::info("SeparateStems: completed {} chunks, {} frames in {} ms on {}", chunkIdx, trackSize,
                         std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - runStart).count(),
                         onGPU ? "DirectML" : "CPU");
            return true;
        }
    }

    return false;

// ── No backend ───────────────────────────────────────────────────────────────
#else
    spdlog::warn("SeparateStems: no inference backend compiled");
    return false;
#endif
}

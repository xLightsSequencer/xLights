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
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
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
    if (!audio || !audio->IsOk()) return false;
    if (modelPath.empty()) return false;

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

    std::string requestedBackend;
    switch (opts.backend) {
    case StemSeparatorBackend::Cpu: requestedBackend = "cpu"; break;
    case StemSeparatorBackend::Gpu: requestedBackend = "gpu"; break;
    case StemSeparatorBackend::Auto: requestedBackend = "auto"; break;
    }
    if (const char* value = std::getenv("XLIGHTS_STEM_BACKEND")) {
        requestedBackend = value;
        std::transform(requestedBackend.begin(), requestedBackend.end(), requestedBackend.begin(),
                       [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    }

    bool useDML = requestedBackend != "cpu";
    bool allowCpuFallback = requestedBackend == "auto" || requestedBackend.empty();
    if (requestedBackend != "auto" && requestedBackend != "cpu" &&
        requestedBackend != "gpu" && requestedBackend != "directml" && !requestedBackend.empty()) {
        spdlog::warn("SeparateStems: unknown XLIGHTS_STEM_BACKEND='{}'; using auto", requestedBackend);
        requestedBackend = "auto";
        useDML = true;
        allowCpuFallback = true;
    }
    spdlog::info("SeparateStems: requested Windows backend '{}'", requestedBackend.empty() ? "auto" : requestedBackend);

    auto makeSession = [&](bool withDML) {
        Ort::SessionOptions sopts;
        sopts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        sopts.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
        if (withDML) {
            sopts.DisableMemPattern();
            Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_DML(sopts, 0));
            spdlog::info("SeparateStems: DirectML GPU provider enabled (device 0)");
        } else {
            spdlog::info("SeparateStems: running on CPU");
        }
        return Ort::Session(env, wpath.c_str(), sopts);
    };

    std::unique_ptr<Ort::Session> session;
    try {
        session = std::make_unique<Ort::Session>(makeSession(useDML));
    } catch (const Ort::Exception& e) {
        if (!useDML || !allowCpuFallback) {
            spdlog::error("SeparateStems: {} session creation failed: {}", useDML ? "DirectML" : "CPU", e.what());
            return false;
        }
        spdlog::warn("SeparateStems: DirectML session creation failed ({}), using CPU", e.what());
        useDML = false;
        try {
            session = std::make_unique<Ort::Session>(makeSession(false));
        } catch (const Ort::Exception& cpuError) {
            spdlog::error("SeparateStems: CPU session creation failed: {}", cpuError.what());
            return false;
        }
    }

    Ort::AllocatorWithDefaultOptions allocator;
    auto inputName0  = session->GetInputNameAllocated(0, allocator);
    auto outputName0 = session->GetOutputNameAllocated(0, allocator);
    auto outShape    = session->GetOutputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    // [1, S, 2, N] where S = number of stems (4 or 6). We map first 4: drums/bass/other/vocals.
    const bool isFourDim = (outShape.size() == 4);
    spdlog::info("SeparateStems: output '{}' {} stems, {} dims",
                 outputName0.get(), isFourDim ? outShape[1] : outShape[1] / 2, outShape.size());

    const char* inputNames[]  = {inputName0.get()};
    const char* outputNames[] = {outputName0.get()};
    const long totalChunks = (trackSize + stride - 1) / stride;

    const int attemptCount = useDML && allowCpuFallback ? 2 : 1;
    for (int attempt = 0; attempt < attemptCount; attempt++) {
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

                auto outputs = session->Run(Ort::RunOptions{nullptr},
                                            inputNames, &inputTensor, 1,
                                            outputNames, 1);

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
            if (useDML && allowCpuFallback && attempt == 0) {
                spdlog::warn("SeparateStems: DirectML inference failed ({}), retrying on CPU", e.what());
                try {
                    session = std::make_unique<Ort::Session>(makeSession(false));
                } catch (const Ort::Exception& cpuError) {
                    spdlog::error("SeparateStems: CPU session creation failed: {}", cpuError.what());
                    return false;
                }
                useDML = false;
                needRetry = true;
            } else {
                spdlog::error("SeparateStems: {} inference failed: {}", useDML ? "DirectML" : "CPU", e.what());
                return false;
            }
        }

        if (cancelled) {
            spdlog::info("SeparateStems: cancelled after {} of {} chunks", chunkIdx, totalChunks);
            return false;
        }
        if (!needRetry) {
            spdlog::info("SeparateStems: completed {} chunks, {} frames", chunkIdx, trackSize);
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

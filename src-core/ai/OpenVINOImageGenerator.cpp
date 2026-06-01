#ifdef HAVE_OPENVINO_GENAI

#ifdef _WIN32
#  define NOMINMAX
#endif

#include "ai/OpenVINOImageGenerator.h"
#include "utils/xlImage.h"

#include <openvino/genai/image_generation/text2image_pipeline.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstdint>

OpenVINOImageGenerator::OpenVINOImageGenerator(std::string modelDir, std::string device,
                                                int width, int height, int steps)
    : _modelDir(std::move(modelDir)), _device(std::move(device))
    , _width(width), _height(height), _steps(steps) {}

void OpenVINOImageGenerator::generateImage(const std::string& prompt,
                                            std::function<void(aiBase::AIImageResult)> callback) {
    aiBase::AIImageResult result;

    if (_modelDir.empty()) {
        result.error = "OpenVINO image model directory not configured";
        callback(std::move(result));
        return;
    }
    if (prompt.empty()) {
        result.error = "Prompt cannot be empty";
        callback(std::move(result));
        return;
    }

    try {
        ov::genai::Text2ImagePipeline pipe(_modelDir, _device);

        ov::Tensor image = pipe.generate(prompt,
            ov::genai::width(_width),
            ov::genai::height(_height),
            ov::genai::num_inference_steps(_steps));

        const auto& shape = image.get_shape();
        // Expected: [N=1, H, W, C=3] float32, values in [0, 255]
        // Some models emit [N=1, C=3, H, W] (NCHW) — detect by checking shape[3].
        if (shape.size() < 4) {
            result.error = "Unexpected output tensor rank from SD pipeline";
            callback(std::move(result));
            return;
        }

        const bool isNHWC = (shape[3] == 3 || shape[3] == 4);
        const size_t H = isNHWC ? shape[1] : shape[2];
        const size_t W = isNHWC ? shape[2] : shape[3];
        const size_t C = isNHWC ? shape[3] : shape[1];

        const float* data = image.data<float>();
        xlImage img(static_cast<int>(W), static_cast<int>(H));

        auto toU8 = [](float v) -> uint8_t {
            // OV GenAI SD pipeline post-processing scales to [0, 255] float.
            return static_cast<uint8_t>(std::clamp(v, 0.0f, 255.0f));
        };

        if (isNHWC) {
            for (size_t y = 0; y < H; ++y) {
                for (size_t x = 0; x < W; ++x) {
                    size_t base = (y * W + x) * C;
                    img.SetRGBA(static_cast<int>(x), static_cast<int>(y),
                                toU8(data[base]),
                                toU8(data[base + 1]),
                                toU8(data[base + 2]),
                                255);
                }
            }
        } else {
            // NCHW: separate R, G, B planes
            const float* rPlane = data;
            const float* gPlane = data + H * W;
            const float* bPlane = data + 2 * H * W;
            for (size_t y = 0; y < H; ++y) {
                for (size_t x = 0; x < W; ++x) {
                    size_t idx = y * W + x;
                    img.SetRGBA(static_cast<int>(x), static_cast<int>(y),
                                toU8(rPlane[idx]),
                                toU8(gPlane[idx]),
                                toU8(bPlane[idx]),
                                255);
                }
            }
        }

        if (!img.SaveAsPNG(result.pngBytes) || result.pngBytes.empty()) {
            result.error = "Failed to encode generated image as PNG";
        }

    } catch (const std::exception& e) {
        spdlog::error("OpenVINOImageGenerator: generation failed: {}", e.what());
        result.error = std::string("OpenVINO image error: ") + e.what();
    }

    callback(std::move(result));
}

std::vector<ServiceProperty> OpenVINOImageGenerator::GetProperties() const {
    std::vector<ServiceProperty> props;
    props.push_back({ ServiceProperty::Kind::Category, {}, "OpenVINO Image", "OpenVINO Image", {}, {}, {} });
    props.push_back({ ServiceProperty::Kind::Int, "OpenVINOImage.Width",  "Width (px)",  "OpenVINO Image",
                      "Output image width in pixels (must match the model's trained resolution)", {}, _width });
    props.push_back({ ServiceProperty::Kind::Int, "OpenVINOImage.Height", "Height (px)", "OpenVINO Image",
                      "Output image height in pixels", {}, _height });
    props.push_back({ ServiceProperty::Kind::Int, "OpenVINOImage.Steps",  "Inference Steps", "OpenVINO Image",
                      "Number of diffusion denoising steps (more = higher quality, slower)", {}, _steps });
    return props;
}

void OpenVINOImageGenerator::SetProperty(const std::string& id, int value) {
    if (id == "OpenVINOImage.Width")  _width  = std::max(64, value);
    else if (id == "OpenVINOImage.Height") _height = std::max(64, value);
    else if (id == "OpenVINOImage.Steps")  _steps  = std::max(1,  value);
}

void OpenVINOImageGenerator::SetProperty(const std::string& /*id*/, const std::string& /*value*/) {}

#endif // HAVE_OPENVINO_GENAI

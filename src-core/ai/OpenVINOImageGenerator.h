#pragma once
#ifdef HAVE_OPENVINO_GENAI

#include "aiBase.h"
#include "ServiceProperty.h"

#include <functional>
#include <string>
#include <vector>

class OpenVINOImageGenerator : public aiBase::AIImageGenerator {
public:
    OpenVINOImageGenerator(std::string modelDir, std::string device,
                            int width, int height, int steps);
    ~OpenVINOImageGenerator() override = default;

    void generateImage(const std::string& prompt,
                       std::function<void(aiBase::AIImageResult)> callback) override;

    [[nodiscard]] std::vector<ServiceProperty> GetProperties() const override;
    void SetProperty(const std::string& id, int value) override;
    void SetProperty(const std::string& id, const std::string& value) override;

private:
    std::string _modelDir;
    std::string _device;
    int _width;
    int _height;
    int _steps;
};

#endif // HAVE_OPENVINO_GENAI

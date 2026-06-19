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

#include "ai/IServiceSettingsStore.h"

#include <string>
#include <string_view>

// Desktop implementation of IServiceSettingsStore.
// Regular settings go through GetXLightsConfig() (JSON-backed app config);
// secrets go through wxSecretStore on platforms where it works, otherwise
// they fall back to plain config.
class WxServiceSettingsStore final : public IServiceSettingsStore {
public:
    WxServiceSettingsStore();
    ~WxServiceSettingsStore() override = default;

    [[nodiscard]] std::string getString(std::string_view key, const std::string& defaultValue) const override;
    [[nodiscard]] int         getInt   (std::string_view key, int defaultValue) const override;
    [[nodiscard]] bool        getBool  (std::string_view key, bool defaultValue) const override;

    void setString(std::string_view key, const std::string& value) override;
    void setInt   (std::string_view key, int value) override;
    void setBool  (std::string_view key, bool value) override;

    [[nodiscard]] std::string getSecret(std::string_view serviceName) const override;
    void                      setSecret(std::string_view serviceName, const std::string& token) override;
};

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

// iPad implementation of IServiceSettingsStore.
// - Plain settings (URLs, model names, toggles, integer counters) are stored
//   in NSUserDefaults under the app's bundle.
// - Secrets (API keys / tokens) are stored in the iOS Keychain as
//   kSecClassGenericPassword items, scoped to the app's bundle by default
//   (no kSecAttrAccessGroup). Items use kSecAttrAccessibleAfterFirstUnlock
//   so they're available to background tasks but not before the device
//   has been unlocked once after boot.
//
// This file is plain C++ so it can be included from src-core/. The
// implementation lives in the matching .mm file.
class XLiPadServiceSettingsStore final : public IServiceSettingsStore {
public:
    XLiPadServiceSettingsStore();
    ~XLiPadServiceSettingsStore() override = default;

    [[nodiscard]] std::string getString(std::string_view key, const std::string& defaultValue) const override;
    [[nodiscard]] int         getInt   (std::string_view key, int defaultValue) const override;
    [[nodiscard]] bool        getBool  (std::string_view key, bool defaultValue) const override;

    void setString(std::string_view key, const std::string& value) override;
    void setInt   (std::string_view key, int value) override;
    void setBool  (std::string_view key, bool value) override;

    [[nodiscard]] std::string getSecret(std::string_view serviceName) const override;
    void                      setSecret(std::string_view serviceName, const std::string& token) override;
};

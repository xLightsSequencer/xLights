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
#include <string_view>

// Settings + secret storage for AI services. The core layer never talks to
// wxSecretStore / wxConfig / NSUserDefaults / iOS Keychain directly; it goes
// through this interface. Host apps (desktop, iPad) supply their own concrete
// implementation when constructing ServiceManager.
//
// Regular settings (getString/getInt/getBool) are stored in plain config;
// secrets (getSecret/setSecret) should be stored in platform secure storage
// (Keychain on Apple, DPAPI/credential manager on Windows, libsecret on
// Linux). A trivial implementation may fall back to plain config for
// secrets, which is what the desktop build does when wxSecretStore is
// unavailable.
class IServiceSettingsStore {
public:
    IServiceSettingsStore() = default;
    virtual ~IServiceSettingsStore() = default;
    IServiceSettingsStore(const IServiceSettingsStore&) = delete;
    IServiceSettingsStore& operator=(const IServiceSettingsStore&) = delete;
    IServiceSettingsStore(IServiceSettingsStore&&) = delete;
    IServiceSettingsStore& operator=(IServiceSettingsStore&&) = delete;

    [[nodiscard]] virtual std::string getString(std::string_view key, const std::string& defaultValue) const = 0;
    [[nodiscard]] virtual int         getInt   (std::string_view key, int defaultValue) const = 0;
    [[nodiscard]] virtual bool        getBool  (std::string_view key, bool defaultValue) const = 0;

    virtual void setString(std::string_view key, const std::string& value) = 0;
    virtual void setInt   (std::string_view key, int value) = 0;
    virtual void setBool  (std::string_view key, bool value) = 0;

    [[nodiscard]] virtual std::string getSecret(std::string_view serviceName) const = 0;
    virtual void                      setSecret(std::string_view serviceName, const std::string& token) = 0;
};

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CloudStorageSettings.h"

#include "ai/WxServiceSettingsStore.h"
#include "settings/XLightsConfigAdapter.h"
#include "utils/string_utils.h"

#include <algorithm>

namespace {

const char* SECRET_NAME = "xLightsCloudStorage";

} // namespace

CloudStorageSettings CloudStorageSettings::Load(bool loadSecret) {
    CloudStorageSettings s;
    auto* config = GetXLightsConfig();
    s.provider = config->Read("CloudStorageProvider", std::string(PROVIDER_R2));
    s.r2AccountId = config->Read("CloudStorageR2AccountId", std::string());
    s.endpoint = config->Read("CloudStorageEndpoint", std::string());
    s.region = config->Read("CloudStorageRegion", std::string("auto"));
    s.bucket = config->Read("CloudStorageBucket", std::string());
    s.prefix = config->Read("CloudStoragePrefix", std::string("xlights"));
    s.accessKeyId = config->Read("CloudStorageAccessKeyId", std::string());
    s.pathStyle = config->ReadBool("CloudStoragePathStyle", true);
    s.includeFseq = config->ReadBool("CloudStorageIncludeFseq", true);
    s.includeThumbnail = config->ReadBool("CloudStorageIncludeThumbnail", true);
    s.keepBackups = std::clamp((int)config->ReadLong("CloudStorageKeepBackups", 0), 0, MAX_KEEP_BACKUPS);
    if (loadSecret) {
        WxServiceSettingsStore store;
        s.secretAccessKey = store.getSecret(SECRET_NAME);
    }
    return s;
}

void CloudStorageSettings::Save() const {
    auto* config = GetXLightsConfig();
    config->Write("CloudStorageProvider", provider);
    config->Write("CloudStorageR2AccountId", Trim(r2AccountId));
    config->Write("CloudStorageEndpoint", Trim(endpoint));
    config->Write("CloudStorageRegion", Trim(region));
    config->Write("CloudStorageBucket", Trim(bucket));
    config->Write("CloudStoragePrefix", Trim(prefix));
    config->Write("CloudStorageAccessKeyId", Trim(accessKeyId));
    config->Write("CloudStoragePathStyle", pathStyle);
    config->Write("CloudStorageIncludeFseq", includeFseq);
    config->Write("CloudStorageIncludeThumbnail", includeThumbnail);
    config->Write("CloudStorageKeepBackups", std::clamp(keepBackups, 0, MAX_KEEP_BACKUPS));
    config->Flush();
    WxServiceSettingsStore store;
    store.setSecret(SECRET_NAME, Trim(secretAccessKey));
}

std::string CloudStorageSettings::EffectiveEndpoint() const {
    if (IsR2()) {
        std::string id = Trim(r2AccountId);
        return id.empty() ? std::string() : S3Config::R2Endpoint(id);
    }
    return Trim(endpoint);
}

S3Config CloudStorageSettings::ToS3Config() const {
    S3Config c;
    c.endpoint = EffectiveEndpoint();
    c.region = IsR2() ? "auto" : Trim(region);
    c.bucket = Trim(bucket);
    c.accessKeyId = Trim(accessKeyId);
    c.secretAccessKey = Trim(secretAccessKey);
    c.pathStyle = IsR2() ? true : pathStyle;
    return c;
}

std::string CloudStorageSettings::Validate() const {
    if (IsR2() && Trim(r2AccountId).empty()) {
        return "Cloudflare account ID is missing";
    }
    if (!IsR2()) {
        std::string ep = Trim(endpoint);
        if (ep.empty()) {
            return "Endpoint URL is missing";
        }
        if (ep.rfind("https://", 0) != 0 && ep.rfind("http://", 0) != 0) {
            return "Endpoint URL must start with https://";
        }
    }
    if (Trim(bucket).empty()) {
        return "Bucket name is missing";
    }
    if (Trim(accessKeyId).empty()) {
        return "Access key ID is missing";
    }
    if (Trim(secretAccessKey).empty()) {
        return "Secret access key is missing";
    }
    return {};
}

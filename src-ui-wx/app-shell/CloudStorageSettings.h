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

#include "utils/S3Client.h"

// Per-machine cloud storage settings. Everything but the secret key lives in
// the xLights settings file; the secret goes to the OS secure store
// (wxSecretStore) through WxServiceSettingsStore.
struct CloudStorageSettings {
    static constexpr const char* PROVIDER_R2 = "r2";
    static constexpr const char* PROVIDER_S3 = "s3";
    static constexpr int MAX_KEEP_BACKUPS = 999;

    std::string provider = PROVIDER_R2;
    std::string r2AccountId;
    std::string endpoint;
    std::string region = "auto";
    std::string bucket;
    std::string prefix = "xlights";
    std::string accessKeyId;
    std::string secretAccessKey;
    bool pathStyle = true;
    // What File > Backup to Cloud adds besides the show folder.
    bool includeFseq = true;
    bool includeThumbnail = true;
    int keepBackups = 0; // backups of each show kept after a backup; 0 keeps them all

    // loadSecret = false avoids touching the secure store (which can prompt
    // on macOS) when only the plain settings are needed.
    static CloudStorageSettings Load(bool loadSecret = true);
    void Save() const;

    bool IsR2() const { return provider == PROVIDER_R2; }
    std::string EffectiveEndpoint() const;
    S3Config ToS3Config() const;

    // Empty when complete, otherwise what is missing.
    std::string Validate() const;
};

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
#include <vector>

#include "utils/CurlManager.h"


#include "RemoteFalconOptions.h"
#include "../../xLights/SpecialOptions.h"

#include "../../xLights/utils/string_utils.h"

#include <nlohmann/json.hpp>
#include <log.h>

#include <format>
#include <string>

class RemoteFalcon
{
    static std::string __token;
    std::string _URLBase;

    public:

        RemoteFalcon(const RemoteFalconOptions& options) {
            __token = options.GetToken();
            _URLBase = SpecialOptions::GetOption("RemoteFalconLocalIP", "https://" + SpecialOptions::GetOption("RemoteFalconURL", "remotefalcon.com")) + SpecialOptions::GetOption("RemoteFalconAPI", "/remote-falcon-plugins-api");
        }

        void SetToken(const std::string& token)
        {
            __token = token;
        }

        static std::string DeTokenfy(std::string s)
        {
            Replace(s, __token, "{token}");
            return s;
        }

        std::string FetchCurrentPlaylistFromQueue()
        {
            return CurlManager::HTTPSGet(_URLBase + "/nextPlaylistInQueue", "", "", 10, { {"remotetoken", __token} });
        }

        std::string FetchRemotePreferences()
        {
            return CurlManager::HTTPSGet(_URLBase + "/remotePreferences", "", "", 10, { {"remotetoken", __token} });
        }

        std::string FetchHighestVotedPlaylist()
        {
            return CurlManager::HTTPSGet(_URLBase + "/highestVotedPlaylist", "", "", 10, { {"remotetoken", __token} });
        }

        std::string UpdatePlaylistQueue()
        {
            nlohmann::json val;
            val["remoteToken"] = __token;
            return CurlManager::HTTPSPost(_URLBase + "/updatePlaylistQueue", val.dump(), "", "", "JSON", 10, { { "remotetoken", __token } });
        }

        std::string PurgeQueue() {
            nlohmann::json val;
            val["remoteToken"] = __token;
            return CurlManager::HTTPSDelete(_URLBase + "/purgeQueue", val.dump(), "", "", "JSON", 10, { { "remotetoken", __token } });
        }

        std::string EnableMangaedPSA(bool enable) {
            nlohmann::json val;
            val["remoteToken"] = __token;
            val["managedPsaEnabled"] = enable ? "Y" : "N";
            return CurlManager::HTTPSPost(_URLBase + "/updateManagedPsa", val.dump(), "", "", "JSON", 10, { { "remotetoken", __token } });
        }

        std::string EnableViewerControl(bool enable)
        {
            nlohmann::json val;
            val["remoteToken"] = __token;
            val["viewerControlEnabled"] = enable ? "Y" : "N";
            return CurlManager::HTTPSPost(_URLBase + "/updateViewerControl", val.dump(), "", "", "JSON", 10, { {"remotetoken", __token} });
        }

        std::string SendPlayingSong(const std::string& playing)
        {
            nlohmann::json val;
            val["remoteToken"] = __token;
            val["playlist"] = playing;
            return CurlManager::HTTPSPost(_URLBase + "/updateWhatsPlaying", val.dump(), "", "", "JSON", 10, { {"remotetoken", __token} });
        }

        std::string SyncPlayLists(const std::string& playlist, const std::string& steps)
        {
            nlohmann::json val;
            val["remoteToken"] = __token;
            val["playlists"] = nlohmann::json::array();

            try {
                nlohmann::json stepsJson = nlohmann::json::parse(steps);
                
                bool first = true;
                if (!stepsJson.is_null() && stepsJson.contains("steps") && stepsJson["steps"].is_array()) {
                    for (size_t i = 0; i < stepsJson["steps"].size(); i++) {
                        // filter out any everystep
                        if (stepsJson["steps"][i].contains("everystep") && stepsJson["steps"][i]["everystep"].get<std::string>() == "false") {
                            nlohmann::json pl_json;
                            pl_json["playlistName"] = stepsJson["steps"][i]["name"].get<std::string>();
                            pl_json["playlistIndex"] = (i + 1);
                            pl_json[ "playlistDuration"] = std::stoi(stepsJson["steps"][i]["lengthms"].get<std::string>()) / 1000;

                            val["playlists"].push_back(pl_json);
                        }
                    }
                }
            } catch (nlohmann::json::parse_error& ex) {
                spdlog::error("Failed to parse steps JSON for SyncPlayLists");
                spdlog::error(ex.what());
            } catch (std::exception& ex) {
                spdlog::error("Exception occurred while processing steps JSON for SyncPlayLists");
                spdlog::error(ex.what());
            }

            auto const url = _URLBase + "/syncPlaylists";
            spdlog::debug(RemoteFalcon::DeTokenfy(url));
            spdlog::debug(RemoteFalcon::DeTokenfy(val.dump()));
            return CurlManager::HTTPSPost(url, val.dump(), "", "", "JSON", 10, { {"remotetoken", __token} });
        }
};

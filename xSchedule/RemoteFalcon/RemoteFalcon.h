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

#include "utils/Curl.h"

#include <wx/wx.h>


#include "../wxJSON/jsonreader.h"
#include "RemoteFalconOptions.h"
#include "../../xLights/SpecialOptions.h"

#include "spdlog/spdlog.h"

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
            wxString ss(s);
            ss.Replace(__token, "{token}");
            return ss.ToStdString();
        }

        std::string FetchCurrentPlaylistFromQueue()
        {
            return Curl::HTTPSGet(_URLBase + "/nextPlaylistInQueue", "", "", 10, { {"remotetoken", __token} });
        }

        std::string FetchRemotePreferences()
        {
            return Curl::HTTPSGet(_URLBase + "/remotePreferences", "", "", 10, { {"remotetoken", __token} });
        }

        std::string FetchHighestVotedPlaylist()
        {
            return Curl::HTTPSGet(_URLBase + "/highestVotedPlaylist", "", "", 10, { {"remotetoken", __token} });
        }

        std::string UpdatePlaylistQueue()
        {
            std::string t = wxString::Format("{\"remoteToken\":\"%s\"}", __token);
            return Curl::HTTPSPost(_URLBase + "/updatePlaylistQueue", t, "", "", "JSON", 10, { {"remotetoken", __token} });
        }

        std::string PurgeQueue() {
            std::string t = wxString::Format("{\"remoteToken\":\"%s\"}", __token);
            return Curl::HTTPSDelete(_URLBase + "/purgeQueue", t, "", "", "JSON", 10, { { "remotetoken", __token } });
        }

        std::string EnableMangaedPSA(bool enable) {
            std::string t = wxString::Format("{\"remoteToken\":\"%s\",\"managedPsaEnabled\":\"%s\"}", __token, enable ? _("Y") : _("N"));
            return Curl::HTTPSPost(_URLBase + "/updateManagedPsa", t, "", "", "JSON", 10, { { "remotetoken", __token } });
        }

        std::string EnableViewerControl(bool enable)
        {
            std::string t = wxString::Format("{\"remoteToken\":\"%s\",\"viewerControlEnabled\":\"%s\"}", __token, enable ? _("Y") : _("N"));
            return Curl::HTTPSPost(_URLBase + "/updateViewerControl", t, "", "", "JSON", 10, { {"remotetoken", __token} });
        }

        std::string SendPlayingSong(const std::string& playing)
        {
            std::string t = wxString::Format("{\"remoteToken\":\"%s\",\"playlist\":\"%s\"}", __token, playing);
            return Curl::HTTPSPost(_URLBase + "/updateWhatsPlaying", t, "", "", "JSON", 10, { {"remotetoken", __token} });
        }

        std::string SyncPlayLists(const std::string& playlist, const std::string& steps)
        {
            std::string body = wxString::Format("{\"remoteToken\":\"%s\",\"playlists\":[", __token);

            wxJSONReader reader;
            wxJSONValue val;
            reader.Parse(steps, &val);
            bool first = true;
            if (!val.IsNull()) {
                for (int i = 0; i < val["steps"].AsArray()->Count(); i++) {
                    // filter out any everystep
                    if (val["steps"][i]["everystep"].AsString() == "false") {
                        if (first) {
                            first = false;
                        }
                        else {
                            body += ",";
                        }

                        body += wxString::Format("{\"playlistName\":\"" + val["steps"][i]["name"].AsString() + "\",\"playlistIndex\":%d, \"playlistDuration\":%d}",
                            (i+1), wxAtoi(val["steps"][i]["lengthms"].AsString()) / 1000);
                    }
                }
            }
        
            body += "]}";
            auto url = _URLBase + "/syncPlaylists";
            spdlog::debug(RemoteFalcon::DeTokenfy(url));
            spdlog::debug(RemoteFalcon::DeTokenfy(body));
            return Curl::HTTPSPost(url, body, "", "", "JSON", 10, { {"remotetoken", __token} });
        }
};

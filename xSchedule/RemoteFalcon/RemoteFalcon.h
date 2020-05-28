#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <vector>

#include "../xSMSDaemon/Curl.h"

#include <wx/wx.h>

#include "../../xLights/UtilFunctions.h"
#include "../wxJSON/jsonreader.h"
#include "RemoteFalconOptions.h"
#include "../../xLights/SpecialOptions.h"

#include <log4cpp/Category.hh>

class RemoteFalcon
{
    static std::string __token;
    std::string _URLBase;

    public:

        RemoteFalcon(const RemoteFalconOptions& options) {
            __token = options.GetToken();
            _URLBase = SpecialOptions::GetOption("RemoteFalconURL", "https://remotefalcon.com") + "/remotefalcon/api";
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

        std::string FetchCurrentPlayMode()
        {
            return Curl::HTTPSGet(_URLBase + "/viewerControlMode", "", "", 10, { {"remotetoken", __token} });
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

        std::string SendPlayingSong(const std::string& playing)
        {
            std::string t = wxString::Format("{\"remoteToken\":\"%s\",\"playlist\":\"%s\"}", __token, playing);
            return Curl::HTTPSPost(_URLBase + "/updateWhatsPlaying", t, "", "", "JSON", 10, { {"remotetoken", __token} });
        }

        std::string SyncPlayLists(const std::string& playlist, const std::string& steps)
        {
            static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
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

                        body += wxString::Format("{\"playlistName\":\"" + val["steps"][i]["name"].AsString() + "\",\"playlistDuration\":%d}",
                            wxAtoi(val["steps"][i]["lengthms"].AsString()) / 1000);
                    }
                }
            }
        
            body += "]}";
            auto url = _URLBase + "/syncPlaylists";
            logger_base.debug(RemoteFalcon::DeTokenfy(url));
            logger_base.debug(RemoteFalcon::DeTokenfy(body));
            return Curl::HTTPSPost(url, body, "", "", "JSON", 10, { {"remotetoken", __token} });
        }
};

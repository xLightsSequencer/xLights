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

#include <log4cpp/Category.hh>

class RemoteFalcon
{
    std::string token;
    const std::string URLBase = "https://remotefalcon.com/services/rmrghbsEvMhSH8LKuJydVn23pvsFKX/api/";

    public:

        RemoteFalcon(const RemoteFalconOptions& options) {
            token = options.GetToken();
        }

        std::string FetchCurrentPlaylistFromQueue()
        {
            std::string t = wxString::Format("{\"remoteToken\":\"%s\"}", token);
            return Curl::HTTPSPost(URLBase + "fetchNextPlaylistFromQueue.php", t, "JSON");
        }

        std::string UpdatePlaylistQueue()
        {
            std::string t = wxString::Format("{\"remoteToken\":\"%s\"}", token);
            return Curl::HTTPSPost(URLBase + "updatePlaylistQueue.php", t, "JSON");
        }

        std::string SendPlayingSong(const std::string& playing)
        {
            std::string t = wxString::Format("{\"remoteToken\":\"%s\",\"playlist\":\"%s\"}", token, playing);
            return Curl::HTTPSPost(URLBase + "updateWhatsPlaying.php", t, "JSON");
        }

        std::string SyncPlayLists(const std::string& playlist, const std::string& steps)
        {
            static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            std::string body = wxString::Format("{\"remoteToken\":\"%s\",\"playlists\":[", token);

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

                        body += "{\"playlistName\":\"" + val["steps"][i]["name"].AsString() + "\",\"playlistItems\":[";
                        body += wxString::Format("{\"playlistItemType\":\"both\",\"playlistItemEnabled\":1,\"playlistItemSequenceName\":\"%s.fseq\",\"playlistItemMediaName\":\"\",\"playlistItemDuration\":%d}", val["steps"][i]["name"].AsString(), wxAtoi(val["steps"][i]["lengthms"].AsString()) / 1000);
                        body += "]}";
                    }
                }
            }
        
            body += "]}";
            logger_base.debug(body);
            return Curl::HTTPSPost(URLBase + "syncPlaylists.php", body, "JSON");
        }
};

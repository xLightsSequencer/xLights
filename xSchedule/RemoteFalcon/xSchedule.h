#pragma once
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

#include "utils/Curl.h"
#include "RemoteFalconApp.h"
#include "../wxJSON/jsonreader.h"
#include "RemoteFalconOptions.h"

#include <log4cpp/Category.hh>

class xSchedule
{
	static p_xSchedule_Action __action;

	static std::string Action(const std::string& command, const std::string& parameters = "", const std::string& data = "")
	{
		size_t size = 1024 * 1024; // 1MB ... surely that is enough ... and on the heap so should be ok
		char* result = (char*)malloc(size);
        if (result != nullptr) {
            std::wstring p(parameters.begin(), parameters.end());
            __action(command.c_str(), (const wchar_t*)p.c_str(), data.c_str(), result, size);
            result[size - 1] = 0x00; // force null termination
            std::string s(result);
            free(result);
            return s;
        }
        return "";
	}

public:
	static void Initialise(p_xSchedule_Action action) { __action = action; }
	static std::string GetPlayingStatus()
	{
		return Action("GetPlayingStatus");
	}
	static std::string DecodePlayList(int plid)
	{
		auto pls = GetPlaylists();

		for (const auto& it : pls) {
			if (it.second == plid) return it.first;
		}

		return "Unknown playlist.";
	}
	static std::list<std::pair<std::string, int>> GetPlaylists()
	{
		std::list<std::pair<std::string, int>> res;

		auto json = Action("getplaylists");

		if (json != "") {

			wxJSONReader reader;
			wxJSONValue val;
			reader.Parse(json, &val);

			if (!val.IsNull()) {
				for (int i = 0; i < val["playlists"].AsArray()->Count(); i++) {
					auto pl = val["playlists"][i];
					res.push_back({ pl["name"].AsString(), wxAtoi(pl["id"].AsString()) });
				}
			}
		}

		return res;
	}

	static std::string GetPlayListSteps(const std::string& playlist)
	{
		return Action("GetPlayListSteps", playlist);
	}
	static std::string PlayPlayListStep(const std::string& playlist, const std::string& step)
	{
		return Action("Play playlist step", playlist + "," + step);
	}
	static std::string EnqueuePlaylistStep(const std::string& playlist, const std::string& step)
	{
		return Action("Enqueue playlist step", playlist + "," + step);
	}
    static std::string PlayEffect(const std::string& playlist, const std::string& step, EFFECT_MODE mode) {
		switch (mode) {
        case EFFECT_MODE::EM_PLAY_IMMEDIATELY:
            return Action("Run event playlist step unique", playlist + "," + step);
        case EFFECT_MODE::EM_PLAY_IMMEDIATELY_LOOPED:
            return Action("Run event playlist step unique looped", playlist + "," + step);
        case EFFECT_MODE::EM_PLAY_ONLY_IF_IDLE:
            return Action("Run event playlist step if idle", playlist + "," + step);
        }
        return "";
    }
    static std::list<std::string> GetPlayingEffects()
    {
        std::list<std::string> res;
        auto json = Action("getplayingeffects");

        if (json != "") {
            wxJSONReader reader;
            wxJSONValue val;
            reader.Parse(json, &val);

            if (!val.IsNull()) {
                for (int i = 0; i < val["playingeffects"].AsArray()->Count(); i++) {
                    auto pl = val["playingeffects"][i];
                    res.push_back(pl["name"].AsString());
                }
            }
        }

        return res;
    }
};

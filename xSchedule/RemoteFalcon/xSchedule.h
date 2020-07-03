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

#include "../xSMSDaemon/Curl.h"
#include "RemoteFalconApp.h"
#include "../wxJSON/jsonreader.h"

#include <log4cpp/Category.hh>

class xSchedule
{
	static p_xSchedule_Action __action;

	static std::string Action(const std::string& command, const std::string& parameters = "", const std::string& data = "")
	{
		char result[4096];
		std::wstring p(parameters.begin(), parameters.end());
		__action(command.c_str(), (const wchar_t*)p.c_str(), data.c_str(), result, sizeof(result));
		return std::string(result);
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
};
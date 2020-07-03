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

#include <list>
#include <string>
#include <vector>
#include <wx/wx.h>

class RemoteFalconOptions
{
    int _changeCount = 0;
    int _lastSavedChangeCount = 0;

    std::string _token = "";
    int _playlist = -1;

    public:

		std::string GetToken() const { return _token; }
        void SetToken(std::string token) { if (token != _token) { _token = token; _changeCount++; } }

        int GetPlaylist() const { return _playlist; }
        void SetPlaylist(int playlist) { if (playlist != _playlist) { _playlist = playlist; _changeCount++; } }

        bool IsDirty() const;
        void ClearDirty();
        RemoteFalconOptions();
        void Load();
        //virtual ~RemoteFalconOptions();
        void Save();
        bool IsValid() const;
};


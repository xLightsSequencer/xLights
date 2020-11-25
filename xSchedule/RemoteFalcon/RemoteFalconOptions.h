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
    std::string _playDuring = "";
    int _playlist = -1;
    int _leadTime = 5;
    bool _immediatelyInterrupt = true;
    bool _clearQueueOnStart = true;
    bool _sendEnableDisable = false;

    public:

		std::string GetToken() const { return _token; }
        void SetToken(std::string token) { if (token != _token) { _token = token; _changeCount++; } }

        int GetPlaylist() const { return _playlist; }
        void SetPlaylist(int playlist) { if (playlist != _playlist) { _playlist = playlist; _changeCount++; } }

        bool GetImmediatelyInterrupt() const { return _immediatelyInterrupt; }
        void SetImmediatelyInterrupt(bool interrupt) { if (interrupt != _immediatelyInterrupt) { _immediatelyInterrupt = interrupt; _changeCount++; } }

        bool GetClearQueueOnStart() const { return _clearQueueOnStart; }
        void SetClearQueueOnStart(bool clearQueue) { if (clearQueue != _clearQueueOnStart) { _clearQueueOnStart = clearQueue; _changeCount++; } }

        bool IsEnableDisable() const { return _sendEnableDisable; }
        void SetEnableDisable(bool sendEnableDisable) { if (_sendEnableDisable != sendEnableDisable) { _sendEnableDisable = sendEnableDisable; _changeCount++; } }

        int GetLeadTime() const { return _leadTime; }
        void SetLeadTime(int leadTime) { if (leadTime != _leadTime) { _leadTime = leadTime; _changeCount++; } }

        void SetPlayDuring(const std::string& playDuring) { if (playDuring != _playDuring) { _playDuring = playDuring; _changeCount++; } }
        std::string GetPlayDuring() const { return _playDuring; }
        bool IsPlayDuring(const std::string& playlist);

        bool IsDirty() const;
        void ClearDirty();
        RemoteFalconOptions();
        void Load();
        //virtual ~RemoteFalconOptions();
        void Save();
        bool IsValid() const;
};


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

enum class EFFECT_MODE {
    EM_PLAY_IMMEDIATELY,
    EM_PLAY_ONLY_IF_IDLE,
    EM_PLAY_IMMEDIATELY_LOOPED
};

enum class MESSAGE_LEVEL {
    ML_ERROR,
    ML_WARN,
    ML_INFO,
    ML_DEBUG
};

class RemoteFalconOptions
{
    public:


    protected:

    int _changeCount = 0;
    int _lastSavedChangeCount = 0;

    std::string _token = "";
    std::string _playDuring = "";
    int _playlist = -1;
    int _leadTime = 5;
    bool _immediatelyInterrupt = true;
    bool _clearQueueOnStart = true;
    bool _sendEnableDisable = false;
    bool _effectPlaylist = false;
    bool _sendPlayingEffect = false;
    EFFECT_MODE _effectMode = EFFECT_MODE::EM_PLAY_IMMEDIATELY;
    MESSAGE_LEVEL _messageLevel = MESSAGE_LEVEL::ML_WARN;

    public:

		std::string GetToken() const { return _token; }
        void SetToken(std::string token) { if (token != _token) { _token = token; _changeCount++; } }

        int GetPlaylist() const { return _playlist; }
        void SetPlaylist(int playlist) { if (playlist != _playlist) { _playlist = playlist; _changeCount++; } }

        bool GetImmediatelyInterrupt() const { return _immediatelyInterrupt; }
        void SetImmediatelyInterrupt(bool interrupt) { if (interrupt != _immediatelyInterrupt) { _immediatelyInterrupt = interrupt; _changeCount++; } }

        bool GetClearQueueOnStart() const { return _clearQueueOnStart; }
        void SetClearQueueOnStart(bool clearQueue) { if (clearQueue != _clearQueueOnStart) { _clearQueueOnStart = clearQueue; _changeCount++; } }

        MESSAGE_LEVEL GetMessageLevel() const
        {
            return _messageLevel;
        }
        void SetMessageLevel(MESSAGE_LEVEL messageLevel)
        {
            if (messageLevel != _messageLevel) {
                _messageLevel = messageLevel;
                _changeCount++;
            }
        }

        bool IsSendPlayingEffect() const
        {
            return _sendPlayingEffect;
        }
        void SetSendPlayignEffect(bool sendPlayingEffect)
        {
            if (_sendPlayingEffect != sendPlayingEffect) {
                _sendPlayingEffect = sendPlayingEffect;
                _changeCount++;
            }
        }

        bool IsEnableDisable() const { return _sendEnableDisable; }
        void SetEnableDisable(bool sendEnableDisable) { if (_sendEnableDisable != sendEnableDisable) { _sendEnableDisable = sendEnableDisable; _changeCount++; } }

        bool IsEffectPlaylist() const {
            return _effectPlaylist;
        }
        void SetEffectPlaylist(bool effectPlaylist) {
            if (_effectPlaylist != effectPlaylist) {
                _effectPlaylist = effectPlaylist;
                _changeCount++;
            }
        }

        EFFECT_MODE GetEffectMode() const {
            return _effectMode;
        }
        void SetEffectMode(EFFECT_MODE effectMode) {
            if (effectMode != _effectMode) {
                _effectMode = effectMode;
                _changeCount++;
            }
        }

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


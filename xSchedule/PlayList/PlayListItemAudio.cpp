/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/notebook.h>
#include <wx/xml/xml.h>

#include "PlayListItemAudio.h"
#include "PlayListItemAudioPanel.h"
#include "../../xLights/AudioManager.h"
#include "../../xLights/UtilFunctions.h"

#include <log4cpp/Category.hh>

PlayListItemAudio::PlayListItemAudio(wxXmlNode* node) :
    PlayListItem(node) {
    _fastStartAudio = false;
    _controlsTimingCache = false;
    _audioFile = "";
    _durationMS = 0;
    _audioManager = nullptr;
    _audioDevice = "";
    PlayListItemAudio::Load(node);
}

void PlayListItemAudio::Load(wxXmlNode* node) {
    PlayListItem::Load(node);
    _fastStartAudio = (node->GetAttribute("FastStartAudio", "FALSE") == "TRUE");
    _audioFile = node->GetAttribute("AudioFile", "");
    _audioFile = FixFile("", _audioFile);
    _audioDevice = node->GetAttribute("AudioDevice", "");

    // if (_fastStartAudio)
    //{
    //     LoadFiles();
    // }
    // else
    {
        FastSetDuration();
    }
}

void PlayListItemAudio::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) {
    if (outputframe) {
        if (ms == _delay && _delay != 0 && ControlsTiming() && _audioManager != nullptr) {
            _audioManager->Play(0, _audioManager->LengthMS());
        }
        _currentFrame++;
    }
}

bool PlayListItemAudio::SetPosition(size_t frame, size_t ms) {
    // wxASSERT(abs((long)frame * _msPerFrame - (long)ms) < _msPerFrame);
    bool res = false;

    _currentFrame = frame;
    if (_audioManager != nullptr) {
        _audioManager->Seek(frame * _msPerFrame);
        res = true;
    }

    return res;
}

void PlayListItemAudio::LoadFiles() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_audioManager != nullptr) {
        if (_audioManager->FileName() == _audioFile) {
            // already open
            return;
        } else {
            CloseFiles();
        }
    }

    _durationMS = 0;

    if (IsInSlaveMode() && IsSuppressAudioOnSlaves()) {
    } else if (wxFile::Exists(_audioFile)) {
        _audioManager = new AudioManager(_audioFile, -1, _audioDevice);

        if (_audioManager == nullptr || !_audioManager->IsOk()) {
            logger_base.error("Audio: Audio file '%s' has a problem opening.", (const char*)_audioFile.c_str());

            if (_audioManager != nullptr) {
                delete _audioManager;
                _audioManager = nullptr;
            }
        } else {
            if (_volume != -1) {
                _audioManager->SetVolume(_volume);
            }
            _durationMS = _audioManager->LengthMS();
            _controlsTimingCache = true;
        }
    } else {
        if (_audioFile != "") {
            logger_base.error("Audio: Audio file '%s' cannot be opened because it does not exist.", (const char*)_audioFile.c_str());
        }
    }
}

PlayListItemAudio::PlayListItemAudio() :
    PlayListItem() {
    _type = "PLIAudio";
    _fastStartAudio = false;
    _controlsTimingCache = false;
    _audioFile = "";
    _audioDevice = "";
    _durationMS = 0;
    _audioManager = nullptr;
}

PlayListItemAudio::~PlayListItemAudio() {
    CloseFiles();

    if (_audioManager != nullptr) {
        delete _audioManager;
        _audioManager = nullptr;
    }
}

PlayListItem* PlayListItemAudio::Copy(const bool isClone) const {
    PlayListItemAudio* res = new PlayListItemAudio();
    PlayListItem::Copy(res, isClone);
    res->_fastStartAudio = _fastStartAudio;
    res->_durationMS = _durationMS;
    res->_controlsTimingCache = _controlsTimingCache;
    res->SetAudioFile(_audioFile); // we set this way to trigger file loading
    res->SetAudioDevice(_audioDevice);

    if (_fastStartAudio) {
        res->LoadFiles();
    }

    return res;
}

wxXmlNode* PlayListItemAudio::Save() {
    wxXmlNode* node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("AudioFile", _audioFile);
    node->AddAttribute("AudioDevice", _audioDevice);
    if (_fastStartAudio) {
        node->AddAttribute("FastStartAudio", "TRUE");
    }

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemAudio::GetTitle() const {
    return "Audio";
}

void PlayListItemAudio::Configure(wxNotebook* notebook) {
    notebook->AddPage(new PlayListItemAudioPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemAudio::GetNameNoTime() const {
    // only recalc the name if the underlying file name has changed
    if (_audioFile != _lastNameFile) {
        _lastNameFile = _audioFile;

        wxFileName fn(_audioFile);
        if (fn.GetName() == "") {
            _lastName = "Audio";
        } else {
            _lastName = fn.GetName().ToStdString();
        }
    }

    return _lastName;
}

void PlayListItemAudio::SetAudioFile(const std::string& audioFile) {
    if (_audioFile != audioFile) {
        _audioFile = audioFile;
        if (_fastStartAudio) {
            LoadFiles();
        } else {
            FastSetDuration();
        }
        _changeCount++;
    }
}

void PlayListItemAudio::SetAudioDevice(const std::string& audioDevice) {
    if (_audioDevice != audioDevice) {
        _audioDevice = audioDevice;
        _changeCount++;
    }
}

void PlayListItemAudio::FastSetDuration() {
    _durationMS = 0;
    if (wxFile::Exists(_audioFile)) {
        _durationMS = AudioManager::GetAudioFileLength(_audioFile);
        _controlsTimingCache = true;
    }
}

bool PlayListItemAudio::Advance(int seconds) {
    int adjustFrames = seconds * 1000 / (int)GetFrameMS();
    _currentFrame += adjustFrames;
    if (_currentFrame < 0)
        _currentFrame = 0;
    if (_currentFrame > _stepLengthMS / GetFrameMS())
        _currentFrame = _stepLengthMS / GetFrameMS();

    if (ControlsTiming() && _audioManager != nullptr) {
        long newPos = _audioManager->Tell() + seconds * 1000;
        if (newPos < 0)
            newPos = 0;
        if (newPos > _audioManager->LengthMS())
            newPos = _audioManager->LengthMS() - 5;
        _audioManager->Seek(newPos);
    }

    return true;
}

size_t PlayListItemAudio::GetPositionMS() const {
    if (ControlsTiming() && _audioManager != nullptr) {
        if (_delay != 0) {
            if (_currentFrame * GetFrameMS() < _delay) {
                return _currentFrame * GetFrameMS();
            }
        }
        return _delay + _audioManager->Tell();
    }

    return 0;
}

void PlayListItemAudio::Restart() {
    if (ControlsTiming() && _audioManager != nullptr) {
        _audioManager->Stop();
        if (_delay == 0) {
            _audioManager->Play(0, _audioManager->LengthMS());
        } else {
            _audioManager->Seek(0);
        }
    }
}

void PlayListItemAudio::Start(long stepLengthMS) {
    PlayListItem::Start(stepLengthMS);

    // load the audio
    LoadFiles();

    if (ControlsTiming() && _audioManager != nullptr) {
        if (_delay == 0) {
            _audioManager->Play(0, _audioManager->LengthMS());
        } else {
            _audioManager->Seek(0);
        }
    }

    if (_audioManager != nullptr) {
        if (_volume != -1) {
            _audioManager->SetVolume(_volume);
        }
    }

    _currentFrame = 0;
}

void PlayListItemAudio::Suspend(bool suspend) {
    Pause(suspend);
}

void PlayListItemAudio::Pause(bool pause) {
    if (_audioManager != nullptr) {
        if (pause) {
            _audioManager->Pause();
        } else {
            _audioManager->Play();
        }
    }
}

void PlayListItemAudio::Stop() {
    if (_audioManager != nullptr) {
        _audioManager->Stop();
    }
    CloseFiles();
}

void PlayListItemAudio::CloseFiles() {
    if (_audioManager != nullptr) {
        if (!_fastStartAudio) {
            delete _audioManager;
            _audioManager = nullptr;
        } else {
            _audioManager->AbsoluteStop();
        }
    }
}

bool PlayListItemAudio::IsAudio(const std::string& ext) {
    if (ext == "mp3" ||
        ext == "ogg" ||
        ext == "mid" ||
        ext == "au" ||
        ext == "wav" ||
        ext == "m4a" ||
        ext == "wma" ||
        ext == "m4p") {
        return true;
    }

    return false;
}

void PlayListItemAudio::SetFastStartAudio(bool fastStartAudio) {
    if (_fastStartAudio != fastStartAudio) {
        _fastStartAudio = fastStartAudio;
        _changeCount++;
        if (_fastStartAudio) {
            LoadFiles();
        } else {
            CloseFiles();
        }
    }
}

std::list<std::string> PlayListItemAudio::GetMissingFiles() {
    std::list<std::string> res;
    if (!wxFile::Exists(GetAudioFile())) {
        res.push_back(GetAudioFile());
    }

    return res;
}

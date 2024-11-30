/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "wx/xml/xml.h"
#include <wx/mediactrl.h> //for wxMediaCtrl
#include <wx/notebook.h>

#include "PlayListItemFSEQVideo.h"
#include "PlayListItemFSEQVideoPanel.h"
#include "PlayerFrame.h"
#include "PlayerWindow.h"
#include "../../xLights/AudioManager.h"
#include "../../xLights/FSEQFile.h"
#include "../../xLights/UtilFunctions.h"
#include "../../xLights/VideoReader.h"
#include "../../xLights/outputs/OutputManager.h"
#include "../ScheduleManager.h"
#include "../ScheduleOptions.h"
#include "../VideoCache.h"
#include "../xScheduleApp.h"
#include "../xScheduleMain.h"

#include <log4cpp/Category.hh>

PlayListItemFSEQVideo::PlayListItemFSEQVideo(OutputManager* outputManager, wxXmlNode* node) :
    PlayListItem(node) {
    _outputManager = outputManager;
    _loopVideo = false;
    _fadeInMS = 0;
    _fadeOutMS = 0;
    _useMediaPlayer = false;
    _fastStartAudio = false;
    _cachedAudioFilename = "";
    _videoReader = nullptr;
    _cachedVideoReader = nullptr;
    _cacheVideo = false;
    _currentFrame = 0;
    _topMost = true;
    _suppressVirtualMatrix = false;
    _window = nullptr;
    _frame = nullptr;
    _videoFile = "";
    _audioDevice = "";
    _videoLength = 0;
    _sc = 0;
    _channels = 0;
    _startChannel = "1";
    _controlsTimingCache = false;
    _applyMethod = APPLYMETHOD::METHOD_OVERWRITE;
    _fseqFileName = "";
    _overrideAudio = false;
    _audioFile = "";
    _durationMS = 0;
    _fseqFile = nullptr;
    _audioManager = nullptr;
    PlayListItemFSEQVideo::Load(node);
}

void PlayListItemFSEQVideo::Load(wxXmlNode* node) {
    PlayListItem::Load(node);
    _fseqFileName = node->GetAttribute("FSEQFile", "");
    _fseqFileName = FixFile("", _fseqFileName);
    _audioFile = node->GetAttribute("AudioFile", "");
    _audioFile = FixFile("", _audioFile);
    _overrideAudio = (_audioFile != "");
    _applyMethod = (APPLYMETHOD)wxAtoi(node->GetAttribute("ApplyMethod", ""));
    _fadeInMS = wxAtoi(node->GetAttribute("FadeInMS", "0"));
    _fadeOutMS = wxAtoi(node->GetAttribute("FadeOutMS", "0"));
    _channels = wxAtol(node->GetAttribute("Channels", "0"));
    _startChannel = node->GetAttribute("StartChannel", "1").ToStdString();
    _fastStartAudio = (node->GetAttribute("FastStartAudio", "FALSE") == "TRUE");
    _cacheVideo = (node->GetAttribute("CacheVideo", "FALSE") == "TRUE");
    _loopVideo = (node->GetAttribute("LoopVideo", "FALSE") == "TRUE");
    _useMediaPlayer = (node->GetAttribute("UseMediaPlayer", "FALSE") == "TRUE");
    _videoFile = node->GetAttribute("VideoFile", "");
    _videoFile = FixFile("", _videoFile);
    _audioDevice = node->GetAttribute("AudioDevice", "");
    _origin = wxPoint(wxAtoi(node->GetAttribute("X", "0")), wxAtoi(node->GetAttribute("Y", "0")));
    _size = wxSize(wxAtoi(node->GetAttribute("W", "100")), wxAtoi(node->GetAttribute("H", "100")));
    _topMost = (node->GetAttribute("Topmost", "TRUE") == "TRUE");
    _suppressVirtualMatrix = (node->GetAttribute("SuppressVM", "FALSE") == "TRUE");

    // if (_fastStartAudio)
    //{
    //     LoadAudio();
    // }
    // else
    {
        FastSetDuration();
    }
}

std::string PlayListItemFSEQVideo::GetAudioFilename() {
    if (_overrideAudio) {
        return _audioFile;
    } else {
        if (_fseqFile != nullptr) {
            auto audio = FixFile("", _fseqFile->getMediaFilename());
            return audio;
        } else {
            if (_cachedAudioFilename != "") {
                return _cachedAudioFilename;
            }

            std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(_fseqFileName));
            if (f) {
                _cachedAudioFilename = FixFile("", f->getMediaFilename());
                return _cachedAudioFilename;
            }
        }
    }

    return "";
}

void PlayListItemFSEQVideo::LoadAudio() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    auto af = GetAudioFilename();

    if (_audioManager != nullptr) {
        if (_audioManager->FileName() == af) {
            // already open

            // If audio file is shorter than fseq override the duration
            if (_audioManager->LengthMS() < _durationMS) {
                logger_base.debug("FSEQ length %ld overridden by audio length %ld.", (long)_audioManager->LengthMS(), (long)_durationMS);
                _durationMS = _audioManager->LengthMS();
            }

            return;
        } else {
            delete _audioManager;
            _audioManager = nullptr;
        }
    }

    if (IsInSlaveMode() && IsSuppressAudioOnSlaves()) {
    } else if (wxFile::Exists(af)) {
        logger_base.debug("FSEQ Video: Loading audio file '%s'.", (const char*)af.c_str());
        _audioManager = new AudioManager(af, -1, _audioDevice);

        if (!_audioManager->IsOk()) {
            logger_base.error("FSEQ Video: Audio file '%s' has a problem opening.", (const char*)af.c_str());
            if (_fseqFile != nullptr)
                _durationMS = _fseqFile->getTotalTimeMS();
            delete _audioManager;
            _audioManager = nullptr;
        } else {
            logger_base.debug("    Loaded ok.");
            _durationMS = _audioManager->LengthMS();
        }

        if (_volume != -1 && _audioManager != nullptr)
            _audioManager->SetVolume(_volume);
        _controlsTimingCache = true;

        // If the FSEQ is shorter than the audio ... then override the length
        std::unique_ptr<FSEQFile> fseq(FSEQFile::openFSEQFile(_fseqFileName));
        size_t durationFSEQ = 0;
        if (fseq) {
            durationFSEQ = fseq->getTotalTimeMS();
        }
        if (durationFSEQ < _durationMS) {
            logger_base.debug("Audio length %ld overridden by FSEQ length %ld.", (long)_durationMS, (long)durationFSEQ);
            _durationMS = durationFSEQ;
        }
    } else {
        if (af != "") {
            logger_base.error("FSEQ Video: Audio file '%s' cannot be opened because it does not exist.", (const char*)af.c_str());
        }
    }
}

void PlayListItemFSEQVideo::LoadFiles(bool doCache) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    CloseFiles();

    if (wxFile::Exists(_fseqFileName)) {
        _fseqFile = FSEQFile::openFSEQFile(_fseqFileName);
        if (_fseqFile != nullptr) {
            _msPerFrame = _fseqFile->getStepTime();
            _durationMS = _fseqFile->getTotalTimeMS();
        } else {
            _msPerFrame = 50;
            _durationMS = 0;
        }
    } else {
        logger_base.error("FSEQ Video: File does not exist. %s", (const char*)_fseqFileName.c_str());
    }

    if (!_useMediaPlayer && _cacheVideo && doCache) {
        _cachedVideoReader = new CachedVideoReader(_videoFile, 0, GetFrameMS(), _size, false);
    } else {
        _videoReader = new VideoReader(_videoFile, _size.GetWidth(), _size.GetHeight(), false);
        if (_useMediaPlayer) {
            _videoLength = _videoReader->GetLengthMS();
            delete _videoReader;
            _videoReader = nullptr;
        }
    }

    LoadAudio();
}

PlayListItemFSEQVideo::PlayListItemFSEQVideo(OutputManager* outputManager, ScheduleOptions* options) :
    PlayListItem() {
    _type = "PLIFSEQVideo";
    _outputManager = outputManager;
    _fadeInMS = 0;
    _fadeOutMS = 0;
    _cachedAudioFilename = "";
    _currentFrame = 0;
    _topMost = true;
    _suppressVirtualMatrix = false;
    _fastStartAudio = false;
    _cacheVideo = false;
    _loopVideo = false;
    _useMediaPlayer = false;
    _videoReader = nullptr;
    _cachedVideoReader = nullptr;
    _videoLength = 0;
    _sc = 0;
    _channels = 0;
    _startChannel = "1";
    _controlsTimingCache = false;
    _applyMethod = APPLYMETHOD::METHOD_OVERWRITE;
    _fseqFileName = "";
    _overrideAudio = false;
    _audioFile = "";
    _durationMS = 0;
    _audioManager = nullptr;
    _fseqFile = nullptr;
    _window = nullptr;
    _frame = nullptr;
    _videoFile = "";
    _audioDevice = "";
    if (options != nullptr) {
        _origin = options->GetDefaultVideoPos();
        _size = options->GetDefaultVideoSize();
    }
}

PlayListItem* PlayListItemFSEQVideo::Copy(const bool isClone) const {
    PlayListItemFSEQVideo* res = new PlayListItemFSEQVideo(_outputManager, (ScheduleOptions*)nullptr);
    PlayListItem::Copy(res, isClone);
    res->_cachedAudioFilename = _cachedAudioFilename;
    res->_outputManager = _outputManager;
    res->_topMost = _topMost;
    res->_fadeInMS = _fadeInMS;
    res->_fadeOutMS = _fadeOutMS;
    res->_suppressVirtualMatrix = _suppressVirtualMatrix;
    res->_fseqFileName = _fseqFileName;
    res->_applyMethod = _applyMethod;
    res->_overrideAudio = _overrideAudio;
    res->_durationMS = _durationMS;
    res->_controlsTimingCache = _controlsTimingCache;
    res->_channels = _channels;
    res->_startChannel = _startChannel;
    res->_fastStartAudio = _fastStartAudio;
    res->_cacheVideo = _cacheVideo;
    res->_loopVideo = _loopVideo;
    res->_origin = _origin;
    res->_size = _size;
    res->_videoFile = _videoFile;
    res->_audioDevice = _audioDevice;
    res->SetAudioFile(_audioFile); // this will trigger file loading
    res->_useMediaPlayer = _useMediaPlayer;
    res->_videoLength = _videoLength;

    if (_fastStartAudio) {
        res->LoadAudio();
    }

    return res;
}

wxXmlNode* PlayListItemFSEQVideo::Save() {
    wxXmlNode* node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("FSEQFile", _fseqFileName);
    _fseqFileName = FixFile("", _fseqFileName);
    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_applyMethod));
    node->AddAttribute("VideoFile", _videoFile);
    node->AddAttribute("AudioDevice", _audioDevice);
    _videoFile = FixFile("", _videoFile);
    node->AddAttribute("X", wxString::Format(wxT("%i"), _origin.x));
    node->AddAttribute("Y", wxString::Format(wxT("%i"), _origin.y));
    node->AddAttribute("W", wxString::Format(wxT("%i"), _size.GetWidth()));
    node->AddAttribute("H", wxString::Format(wxT("%i"), _size.GetHeight()));
    node->AddAttribute("FadeInMS", wxString::Format(wxT("%i"), _fadeInMS));
    node->AddAttribute("FadeOutMS", wxString::Format(wxT("%i"), _fadeOutMS));
    node->AddAttribute("Channels", wxString::Format(wxT("%ld"), (long)_channels));
    node->AddAttribute("StartChannel", _startChannel);

    if (!_topMost) {
        node->AddAttribute("Topmost", "FALSE");
    }

    if (_suppressVirtualMatrix) {
        node->AddAttribute("SuppressVM", "TRUE");
    }

    if (_useMediaPlayer) {
        node->AddAttribute("UseMediaPlayer", "TRUE");
    }

    if (_fastStartAudio) {
        node->AddAttribute("FastStartAudio", "TRUE");
    }

    if (_cacheVideo) {
        node->AddAttribute("CacheVideo", "TRUE");
    }

    if (_loopVideo) {
        node->AddAttribute("LoopVideo", "TRUE");
    }

    if (_overrideAudio) {
        node->AddAttribute("AudioFile", _audioFile);
    }

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemFSEQVideo::GetTitle() const {
    return "FSEQ & Video";
}

size_t PlayListItemFSEQVideo::GetStartChannelAsNumber() {
    if (_sc == 0) {
        _sc = _outputManager->DecodeStartChannel(_startChannel);
    }

    return _sc;
}

void PlayListItemFSEQVideo::Configure(wxNotebook* notebook) {
    notebook->AddPage(new PlayListItemFSEQVideoPanel(notebook, _outputManager, this), GetTitle(), true);
}

std::string PlayListItemFSEQVideo::GetNameNoTime() const {
    wxFileName fn(_fseqFileName);
    if (fn.GetName() == "") {
        return "FSEQ & Video";
    } else {
        return fn.GetName().ToStdString();
    }
}

void PlayListItemFSEQVideo::SetFSEQFileName(const std::string& fseqFileName) {
    if (_fseqFileName != fseqFileName) {
        _fseqFileName = fseqFileName;
        if (_fastStartAudio) {
            LoadAudio();
        } else {
            FastSetDuration();
        }
        _changeCount++;
    }
}

void PlayListItemFSEQVideo::SetVideoFile(const std::string& videoFile) {
    if (_videoFile != videoFile) {
        _videoFile = videoFile;
        _changeCount++;
    }
}

void PlayListItemFSEQVideo::SetAudioFile(const std::string& audioFile) {
    if (_audioFile != audioFile) {
        _audioFile = audioFile;
        _changeCount++;
        if (_fastStartAudio) {
            LoadAudio();
        } else {
            FastSetDuration();
        }
    }
}

void PlayListItemFSEQVideo::SetAudioDevice(const std::string& audioDevice) {
    if (_audioDevice != audioDevice) {
        _audioDevice = audioDevice;
        _changeCount++;
    }
}

void PlayListItemFSEQVideo::SetOverrideAudio(bool overrideAudio) {
    if (_overrideAudio != overrideAudio) {
        _overrideAudio = overrideAudio;
        _changeCount++;
        if (_fastStartAudio) {
            LoadAudio();
        } else {
            FastSetDuration();
        }
    }
}

void PlayListItemFSEQVideo::SetFastStartAudio(bool fastStartAudio) {
    if (_fastStartAudio != fastStartAudio) {
        _fastStartAudio = fastStartAudio;
        _changeCount++;
        if (_fastStartAudio) {
            LoadAudio();
        } else {
            if (_audioManager != nullptr) {
                delete _audioManager;
                _audioManager = nullptr;
            }
        }
    }
}

void PlayListItemFSEQVideo::FastSetDuration() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _controlsTimingCache = false;
    std::string af = GetAudioFile();
    if (af == "") {
        std::unique_ptr<FSEQFile> fseq(FSEQFile::openFSEQFile(_fseqFileName));
        if (fseq) {
            af = FixFile("", fseq->getMediaFilename());

            if (!_overrideAudio && af != "" && wxFile::Exists(af)) {
                _durationMS = AudioManager::GetAudioFileLength(af);
                _controlsTimingCache = true;

                // If the FSEQ is shorter than the audio ... then override the length
                size_t durationFSEQ = fseq->getTotalTimeMS();
                if (_durationMS == 0) {
                    logger_base.debug("Audio length %ld overridden by FSEQ length %ld as zero just cant be right ... likely audio file load failed.", (long)_durationMS, (long)durationFSEQ);
                    _durationMS = durationFSEQ;
                } else if (durationFSEQ < _durationMS) {
                    logger_base.debug("Audio length %ld overridden by FSEQ length %ld.", (long)_durationMS, (long)durationFSEQ);
                    _durationMS = durationFSEQ;
                }
            } else {
                _durationMS = fseq->getTotalTimeMS();
            }
        }
    } else {
        _durationMS = AudioManager::GetAudioFileLength(GetAudioFilename());
        _controlsTimingCache = true;

        // If the FSEQ is shorter than the audio ... then override the length
        std::unique_ptr<FSEQFile> fseq(FSEQFile::openFSEQFile(_fseqFileName));
        size_t durationFSEQ = 0;
        if (fseq) {
            durationFSEQ = fseq->getTotalTimeMS();
        }
        if (_durationMS == 0) {
            logger_base.debug("Audio length %ld overridden by FSEQ length %ld as zero just cant be right ... likely audio file load failed.", (long)_durationMS, (long)durationFSEQ);
            _durationMS = durationFSEQ;
        } else if (durationFSEQ < _durationMS) {
            logger_base.debug("Audio length %ld overridden by FSEQ length %ld.", (long)_durationMS, (long)durationFSEQ);
            _durationMS = durationFSEQ;
        }
    }
}

bool PlayListItemFSEQVideo::Advance(int seconds) {
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

size_t PlayListItemFSEQVideo::GetPositionMS() const {
    if (ControlsTiming() && _audioManager != nullptr) {
        if (_delay != 0) {
            if (_currentFrame * GetFrameMS() < _delay) {
                return _currentFrame * GetFrameMS();
            }
        }
        return _delay + _audioManager->Tell();
    } else {
        return _currentFrame * GetFrameMS();
    }
}

// Maximum milliseconds a media player file can be out of sync with the sequence
#define MAXMEDIAJITTER (3 * framems)

void PlayListItemFSEQVideo::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    size_t adjustedMS = ms - _delay;

    wxStopWatch sw;
    if (outputframe) {
        if (ms < _delay) {
            // do nothing
        } else {
            if (ms == _delay && _delay != 0 && ControlsTiming() && _audioManager != nullptr) {
                _audioManager->Play(0, _audioManager->LengthMS());
            }

            if (_fseqFile != nullptr) {
                int frame = adjustedMS / framems;
                FSEQFile::FrameData* data = _fseqFile->getFrame(frame);
                if (data != nullptr) {
                    std::vector<uint8_t> buf(_fseqFile->getMaxChannel() + 1);
                    data->readFrame(&buf[0], buf.size());
                    size_t channelsPerFrame = (size_t)_fseqFile->getMaxChannel() + 1;
                    if (_channels > 0)
                        channelsPerFrame = std::min(_channels, (size_t)_fseqFile->getMaxChannel() + 1);
                    if (_channels > 0) {
                        long offset = GetStartChannelAsNumber() - 1;
                        Blend(buffer, size, &buf[offset], channelsPerFrame, _applyMethod, offset);
                    } else {
                        Blend(buffer, size, &buf[0], channelsPerFrame, _applyMethod, 0);
                    }
                    delete data;
                } else {
                    wxASSERT(false);
                }
            }
        }
        _currentFrame++;
    }

    if (ms < _delay) {
        // do nothing
    } else {
        long adjustedMS = ms - _delay;

        if (_useMediaPlayer) {
            bool videoOver = (!_loopVideo && adjustedMS > _videoLength);

            if (!videoOver && _frame->GetState() != wxMEDIASTATE_PLAYING) {
                _frame->Play();
            }

            long mediapos = adjustedMS;
            if (_loopVideo && _videoLength != 0) {
                mediapos = mediapos % _videoLength;
                // loop early to try and prevent black screen when video ends but before we can tell it to loop
                if (mediapos > _videoLength - framems) {
                    // logger_base.debug("Looping early");
                    mediapos = 0;
                }
            }

            if (videoOver) {
                _frame->Stop();
            } else {
                long videopos = _frame->Tell();
                long jitter = std::abs(videopos - mediapos);
                if (jitter > MAXMEDIAJITTER) {
                    _frame->Seek(mediapos);
                    // logger_base.debug("Sequence pos %ld, Desired video pos %ld, Video pos %ld, Jitter %ld, Length %ld, %s", adjustedMS, mediapos, videopos, jitter, (long)_durationMS, (const char*)_videoFile.c_str());
                }
            }
        } else {
            int brightness = 100;
            if (_fadeInMS != 0 && _fadeInMS < framems) {
                brightness = (float)framems * 100.0 / (float)_fadeInMS;
            } else if (_fadeOutMS != 0 && _stepLengthMS - framems < _fadeOutMS) {
                brightness = (float)(_stepLengthMS - framems) * 100.0 / (float)_fadeOutMS;
            }

            if (_cacheVideo) {
                if (_cachedVideoReader != nullptr) {
                    auto videoLength = _cachedVideoReader->GetLengthMS();
                    while (_loopVideo && adjustedMS > videoLength && videoLength > 0) {
                        adjustedMS -= videoLength;
                    }

                    _window->SetImage(CachedVideoReader::FadeImage(_cachedVideoReader->GetNextFrame(adjustedMS), brightness));
                }
            } else {
                if (_videoReader != nullptr) {
                    auto videoLength = _videoReader->GetLengthMS();
                    while (_loopVideo && videoLength > 0 && adjustedMS > videoLength) {
                        adjustedMS -= videoLength;
                    }

                    AVFrame* img = _videoReader->GetNextFrame(adjustedMS, framems);
                    _window->SetImage(CachedVideoReader::FadeImage(CachedVideoReader::CreateImageFromFrame(img, _size), brightness));
                }
            }
        }
    }

    if (sw.Time() > framems / 2) {
        logger_base.warn("   Getting frame %ld from FSEQvideo %s took more than half a frame: %ld.", (long)adjustedMS, (const char*)GetNameNoTime().c_str(), (long)sw.Time());
    }
}

void PlayListItemFSEQVideo::Restart() {
    if (ControlsTiming() && _audioManager != nullptr) {
        _audioManager->Stop();
        if (_delay == 0) {
            _audioManager->Play(0, _audioManager->LengthMS());
        } else {
            _audioManager->Seek(0);
        }
    }
    _currentFrame = 0;
}

void PlayListItemFSEQVideo::Start(long stepLengthMS) {
    PlayListItem::Start(stepLengthMS);

    if (_suppressVirtualMatrix) {
        xScheduleFrame::GetScheduleManager()->SuppressVM(true);
    }

    // load the FSEQ
    // load the audio
    LoadFiles(true);

    if (_fseqFile != nullptr) {
        _fseqFile->prepareRead({ { 0, _fseqFile->getMaxChannel() + 1 } });
    }

    _currentFrame = 0;
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

    // create the window
    if (_useMediaPlayer) {
        if (_frame == nullptr) {
            _frame = new PlayerFrame(wxGetApp().GetTopWindow(), _topMost, wxID_ANY, _origin, _size);
            auto res = _frame->Load(_videoFile);
            wxASSERT(res);
            _frame->Show();
        } else {
            _frame->Move(_origin);
            _frame->SetSize(_size);
            _frame->Show();
        }
    } else {
        if (_window == nullptr) {
            _window = new PlayerWindow(wxGetApp().GetTopWindow(), _topMost, wxIMAGE_QUALITY_BILINEAR /*wxIMAGE_QUALITY_HIGH*/, -1, wxID_ANY, _origin, _size);
        } else {
            _window->Move(_origin);
            _window->SetSize(_size);
        }
    }
}

void PlayListItemFSEQVideo::Suspend(bool suspend) {
    Pause(suspend);

    if (suspend) {
        if (_window != nullptr)
            _window->Hide();
        if (_frame != nullptr) {
            _frame->Pause();
            _frame->Hide();
        }
    } else {
        if (_window != nullptr)
            _window->Show();
        if (_frame != nullptr) {
            _frame->Play();
            _frame->Show();
        }
    }
}

void PlayListItemFSEQVideo::Pause(bool pause) {
    if (ControlsTiming() && _audioManager != nullptr) {
        if (pause) {
            _audioManager->Pause();
        } else {
            _audioManager->Play();
        }
    }

    if (_frame != nullptr) {
        if (pause) {
            _frame->Pause();
        } else {
            _frame->Play();
        }
    }
}

void PlayListItemFSEQVideo::Stop() {
    if (_suppressVirtualMatrix) {
        xScheduleFrame::GetScheduleManager()->SuppressVM(false);
    }

    if (_audioManager != nullptr) {
        _audioManager->Stop();
    }

    CloseFiles();

    // destroy the window
    if (_window != nullptr) {
        delete _window;
        _window = nullptr;
    }
    if (_frame != nullptr) {
        _frame->Stop();
        delete _frame;
        _frame = nullptr;
    }

    _currentFrame = 0;
}

void PlayListItemFSEQVideo::CloseFiles() {
    if (_fseqFile != nullptr) {
        delete _fseqFile;
        _fseqFile = nullptr;
    }

    if (_audioManager != nullptr) {
        if (!_fastStartAudio) {
            delete _audioManager;
            _audioManager = nullptr;
        } else {
            _audioManager->AbsoluteStop();
        }
    }

    if (_videoReader != nullptr) {
        delete _videoReader;
        _videoReader = nullptr;
    }

    if (_cachedVideoReader != nullptr) {
        delete _cachedVideoReader;
        _cachedVideoReader = nullptr;
    }
}

PlayListItemFSEQVideo::~PlayListItemFSEQVideo() {
    CloseFiles();

    if (_audioManager != nullptr) {
        delete _audioManager;
        _audioManager = nullptr;
    }

    if (_window != nullptr) {
        delete _window;
        _window = nullptr;
    }

    if (_frame != nullptr) {
        delete _frame;
        _frame = nullptr;
    }
}

std::list<std::string> PlayListItemFSEQVideo::GetMissingFiles() {
    std::list<std::string> res;
    if (!wxFile::Exists(GetFSEQFileName())) {
        res.push_back(GetFSEQFileName());
    }
    auto af = GetAudioFilename();
    if (af != "" && !wxFile::Exists(af)) {
        res.push_back(af);
    }
    if (!wxFile::Exists(GetVideoFile())) {
        res.push_back(GetVideoFile());
    }

    return res;
}

bool PlayListItemFSEQVideo::SetPosition(size_t frame, size_t ms) {
    // wxASSERT(abs((long)frame * _msPerFrame - (long)ms) < _msPerFrame);
    bool res = false;

    _currentFrame = frame;
    if (_audioManager != nullptr) {
        _audioManager->Seek(frame * _msPerFrame);
        res = true;
    }

    if (_frame != nullptr) {
        _frame->Seek(frame * _msPerFrame);
    }

    return res;
}

long PlayListItemFSEQVideo::GetFSEQChannels() const {
    if (_fseqFile != nullptr) {
        return _fseqFile->getMaxChannel(); // +1;
    } else {
        if (wxFile::Exists(_fseqFileName)) {
            std::unique_ptr<FSEQFile> fseqFile(FSEQFile::openFSEQFile(_fseqFileName));
            long ch = fseqFile->getMaxChannel(); // +1;
            return ch;
        } else {
            return 0;
        }
    }
}

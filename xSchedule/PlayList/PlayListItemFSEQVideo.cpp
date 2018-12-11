#include "PlayListItemFSEQVideo.h"
#include "wx/xml/xml.h"
#include <wx/notebook.h>
#include "PlayListItemFSEQVideoPanel.h"
#include "../../xLights/AudioManager.h"
#include "../../xLights/VideoReader.h"
#include <log4cpp/Category.hh>
#include "../VideoCache.h"
#include "PlayerWindow.h"
#include "../xScheduleApp.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../../xLights/UtilFunctions.h"
#include "../../xLights/outputs/OutputManager.h"

PlayListItemFSEQVideo::PlayListItemFSEQVideo(OutputManager* outputManager, wxXmlNode* node) : PlayListItem(node)
{
    _outputManager = outputManager;
    _loopVideo = false;
    _fadeInMS = 0;
    _fadeOutMS = 0;
    _fastStartAudio = false;
    _cachedAudioFilename = "";
    _videoReader = nullptr;
    _cachedVideoReader = nullptr;
    _cacheVideo = false;
    _currentFrame = 0;
    _topMost = true;
    _suppressVirtualMatrix = false;
    _window = nullptr;
    _videoFile = "";
    _origin.x = 0;
    _origin.y = 0;
    _size.SetWidth(300);
    _size.SetHeight(300);
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

void PlayListItemFSEQVideo::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _fseqFileName = node->GetAttribute("FSEQFile", "");
    _audioFile = node->GetAttribute("AudioFile", "");
    _overrideAudio = (_audioFile != "");
    _applyMethod = (APPLYMETHOD)wxAtoi(node->GetAttribute("ApplyMethod", ""));
    _fadeInMS = wxAtoi(node->GetAttribute("FadeInMS", "0"));
    _fadeOutMS = wxAtoi(node->GetAttribute("FadeOutMS", "0"));
    _channels = wxAtol(node->GetAttribute("Channels", "0"));
    _startChannel = node->GetAttribute("StartChannel", "1").ToStdString();
    _fastStartAudio = (node->GetAttribute("FastStartAudio", "FALSE") == "TRUE");
    _cacheVideo = (node->GetAttribute("CacheVideo", "FALSE") == "TRUE");
    _loopVideo = (node->GetAttribute("LoopVideo", "FALSE") == "TRUE");
    _videoFile = node->GetAttribute("VideoFile", "");
    _origin = wxPoint(wxAtoi(node->GetAttribute("X", "0")), wxAtoi(node->GetAttribute("Y", "0")));
    _size = wxSize(wxAtoi(node->GetAttribute("W", "100")), wxAtoi(node->GetAttribute("H", "100")));
    _topMost = (node->GetAttribute("Topmost", "TRUE") == "TRUE");
    _suppressVirtualMatrix = (node->GetAttribute("SuppressVM", "FALSE") == "TRUE");

    //if (_fastStartAudio)
    //{
    //    LoadAudio();
    //}
    //else
    {
        FastSetDuration();
    }
}

std::string PlayListItemFSEQVideo::GetAudioFilename() 
{
    if (_overrideAudio)
    {
        return _audioFile;
    }
    else
    {
        if (_fseqFile != nullptr)
        {
            return _fseqFile->GetAudioFileName();
        }
        else
        {
            if (_cachedAudioFilename != "")
            {
                return _cachedAudioFilename;
            }

            FSEQFile f(_fseqFileName);
            if (f.IsOk())
            {
                _cachedAudioFilename = f.GetAudioFileName();
                return f.GetAudioFileName();
            }
        }
    }

    return "";
}

void PlayListItemFSEQVideo::LoadAudio()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    auto af = GetAudioFilename();

    if (_audioManager != nullptr)
    {
        if (_audioManager->FileName() == af)
        {
            // already open

            // If audio file is shorter than fseq override the duration
            if (_audioManager->LengthMS() < _durationMS)
            {
                logger_base.debug("FSEQ length %ld overridden by audio length %ld.", (long)_audioManager->LengthMS(), (long)_durationMS);
                _durationMS = _audioManager->LengthMS();
            }

            return;
        }
        else
        {
            delete _audioManager;
            _audioManager = nullptr;
        }
    }

    if (IsInSlaveMode())
    {
    }
    else if (wxFile::Exists(af))
    {
        logger_base.debug("FSEQ Video: Loading audio file '%s'.", (const char *)af.c_str());
        _audioManager = new AudioManager(af);

        if (!_audioManager->IsOk())
        {
            logger_base.error("FSEQ Video: Audio file '%s' has a problem opening.", (const char *)af.c_str());
            if (_fseqFile != nullptr)
                _durationMS = _fseqFile->GetLengthMS();
            delete _audioManager;
            _audioManager = nullptr;
        }
        else
        {
            logger_base.debug("    Loaded ok.");
            _durationMS = _audioManager->LengthMS();
        }

        if (_volume != -1 && _audioManager != nullptr)
            _audioManager->SetVolume(_volume);
        _controlsTimingCache = true;

        // If the FSEQ is shorter than the audio ... then override the length
        FSEQFile fseq(_fseqFileName);
        size_t durationFSEQ = fseq.GetLengthMS();
        if (durationFSEQ < _durationMS)
        {
            logger_base.debug("Audio length %ld overridden by FSEQ length %ld.", (long)_durationMS, (long)durationFSEQ);
            _durationMS = durationFSEQ;
        }
    }
    else
    {
        if (af != "")
        {
            logger_base.error("FSEQ Video: Audio file '%s' cannot be opened because it does not exist.", (const char *)af.c_str());
        }
    }
}

void PlayListItemFSEQVideo::LoadFiles(bool doCache)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    CloseFiles();

    if (wxFile::Exists(_fseqFileName))
    {
        _fseqFile = new FSEQFile();
        _fseqFile->Load(_fseqFileName);
        _msPerFrame = _fseqFile->GetFrameMS();
        _durationMS = _fseqFile->GetLengthMS();
    }
    else
    {
        logger_base.error("FSEQ Video: File does not exist. %s", (const char *)_fseqFileName.c_str());
    }

    if (_cacheVideo && doCache)
    {
        _cachedVideoReader = new CachedVideoReader(_videoFile, 0, GetFrameMS(), _size, false);
    }
    else
    {
        _videoReader = new VideoReader(_videoFile, _size.GetWidth(), _size.GetHeight(), false);
    }

    LoadAudio();
}

PlayListItemFSEQVideo::PlayListItemFSEQVideo(OutputManager* outputManager) : PlayListItem()
{
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
    _videoReader = nullptr;
    _cachedVideoReader = nullptr;
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
    _videoFile = "";
    _origin.x = 0;
    _origin.y = 0;
    _size.SetWidth(300);
    _size.SetHeight(300);
}

PlayListItem* PlayListItemFSEQVideo::Copy() const
{
    PlayListItemFSEQVideo* res = new PlayListItemFSEQVideo(_outputManager);
    PlayListItem::Copy(res);
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
    res->SetAudioFile(_audioFile); // this will trigger file loading

    if (_fastStartAudio)
    {
        res->LoadAudio();
    }

    return res;
}

wxXmlNode* PlayListItemFSEQVideo::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIFSEQVideo");

    node->AddAttribute("FSEQFile", _fseqFileName);
    _fseqFileName = FixFile("", _fseqFileName);
    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_applyMethod));
    node->AddAttribute("VideoFile", _videoFile);
    _videoFile = FixFile("", _videoFile);
    node->AddAttribute("X", wxString::Format(wxT("%i"), _origin.x));
    node->AddAttribute("Y", wxString::Format(wxT("%i"), _origin.y));
    node->AddAttribute("W", wxString::Format(wxT("%i"), _size.GetWidth()));
    node->AddAttribute("H", wxString::Format(wxT("%i"), _size.GetHeight()));
    node->AddAttribute("FadeInMS", wxString::Format(wxT("%i"), _fadeInMS));
    node->AddAttribute("FadeOutMS", wxString::Format(wxT("%i"), _fadeOutMS));
    node->AddAttribute("Channels", wxString::Format(wxT("%ld"), (long)_channels));
    node->AddAttribute("StartChannel", _startChannel);

    if (!_topMost)
    {
        node->AddAttribute("Topmost", "FALSE");
    }

    if (_suppressVirtualMatrix)
    {
        node->AddAttribute("SuppressVM", "TRUE");
    }

    if (_fastStartAudio)
    {
        node->AddAttribute("FastStartAudio", "TRUE");
    }

    if (_cacheVideo)
    {
        node->AddAttribute("CacheVideo", "TRUE");
    }

    if (_loopVideo)
    {
        node->AddAttribute("LoopVideo", "TRUE");
    }

    if (_overrideAudio)
    {
        node->AddAttribute("AudioFile", _audioFile);
    }

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemFSEQVideo::GetTitle() const
{
    return "FSEQ & Video";
}

size_t PlayListItemFSEQVideo::GetStartChannelAsNumber()
{
    if (_sc == 0)
    {
        _sc = _outputManager->DecodeStartChannel(_startChannel);
    }

    return _sc;
}

void PlayListItemFSEQVideo::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemFSEQVideoPanel(notebook, _outputManager, this), GetTitle(), true);
}

std::string PlayListItemFSEQVideo::GetNameNoTime() const
{
    wxFileName fn(_fseqFileName);
    if (fn.GetName() == "")
    {
        return "FSEQ & Video";
    }
    else
    {
        return fn.GetName().ToStdString();
    }
}

void PlayListItemFSEQVideo::SetFSEQFileName(const std::string& fseqFileName)
{
    if (_fseqFileName != fseqFileName)
    {
        _fseqFileName = fseqFileName;
        if (_fastStartAudio)
        {
            LoadAudio();
        }
        else
        {
            FastSetDuration();
        }
        _changeCount++;
    }
}

void PlayListItemFSEQVideo::SetVideoFile(const std::string& videoFile) 
{ 
    if (_videoFile != videoFile)
    {
        _videoFile = videoFile;
        _changeCount++;
    }
}

void PlayListItemFSEQVideo::SetAudioFile(const std::string& audioFile)
{
    if (_audioFile != audioFile)
    {
        _audioFile = audioFile;
        _changeCount++;
        if (_fastStartAudio)
        {
            LoadAudio();
        }
        else
        {
            FastSetDuration();
        }
    }
}

void PlayListItemFSEQVideo::SetOverrideAudio(bool overrideAudio)
{
    if (_overrideAudio != overrideAudio)
    {
        _overrideAudio = overrideAudio;
        _changeCount++;
        if (_fastStartAudio)
        {
            LoadAudio();
        }
        else
        {
            FastSetDuration();
        }
    }
}

void PlayListItemFSEQVideo::SetFastStartAudio(bool fastStartAudio)
{
    if (_fastStartAudio != fastStartAudio)
    {
        _fastStartAudio = fastStartAudio;
        _changeCount++;
        if (_fastStartAudio)
        {
            LoadAudio();
        }
        else
        {
            if (_audioManager != nullptr)
            {
                delete _audioManager;
                _audioManager = nullptr;
            }
        }
    }
}

void PlayListItemFSEQVideo::FastSetDuration()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _controlsTimingCache = false;
    std::string af = GetAudioFile();
    if (af == "")
    {
        FSEQFile fseq(_fseqFileName);

        af = fseq.GetAudioFileName();

        if (!_overrideAudio && af != "" && wxFile::Exists(af))
        {
            _durationMS = AudioManager::GetAudioFileLength(fseq.GetAudioFileName());
            _controlsTimingCache = true;

            // If the FSEQ is shorter than the audio ... then override the length
            size_t durationFSEQ = fseq.GetLengthMS();
            if (_durationMS == 0)
            {
                logger_base.debug("Audio length %ld overridden by FSEQ length %ld as zero just cant be right ... likely audio file load failed.", (long)_durationMS, (long)durationFSEQ);
                _durationMS = durationFSEQ;
            }
            else if (durationFSEQ < _durationMS)
            {
                logger_base.debug("Audio length %ld overridden by FSEQ length %ld.", (long)_durationMS, (long)durationFSEQ);
                _durationMS = durationFSEQ;
            }
        }
        else
        {
            _durationMS = fseq.GetLengthMS();
        }
    }
    else
    {
        _durationMS = AudioManager::GetAudioFileLength(GetAudioFilename());
        _controlsTimingCache = true;

        // If the FSEQ is shorter than the audio ... then override the length
        FSEQFile fseq(_fseqFileName);
        size_t durationFSEQ = fseq.GetLengthMS();
        if (_durationMS == 0)
        {
            logger_base.debug("Audio length %ld overridden by FSEQ length %ld as zero just cant be right ... likely audio file load failed.", (long)_durationMS, (long)durationFSEQ);
            _durationMS = durationFSEQ;
        }
        else if (durationFSEQ < _durationMS)
        {
            logger_base.debug("Audio length %ld overridden by FSEQ length %ld.", (long)_durationMS, (long)durationFSEQ);
            _durationMS = durationFSEQ;
        }
    }
}

size_t PlayListItemFSEQVideo::GetPositionMS() const
{
    if (ControlsTiming() && _audioManager != nullptr)
    {
        if (_delay != 0)
        {
            if (_currentFrame * GetFrameMS() < _delay)
            {
                return _currentFrame * GetFrameMS();
            }
        }
        return _delay + _audioManager->Tell();
    }
    else
    {
        return _currentFrame * GetFrameMS();
    }
}

void PlayListItemFSEQVideo::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    size_t adjustedMS = ms - _delay;

    wxStopWatch sw;
    if (outputframe)
    {
        if (ms < _delay)
        {
            // do nothing
        }
        else
        {
            if (ms == _delay && _delay != 0 && ControlsTiming() && _audioManager != nullptr)
            {
                _audioManager->Play(0, _audioManager->LengthMS());
            }

            if (_fseqFile != nullptr)
            {
                if (_channels > 0)
                {
                    long sc = GetStartChannelAsNumber();
                    wxASSERT(sc > 0);
                    _fseqFile->ReadData(buffer, size, adjustedMS / framems, _applyMethod, sc - 1, _channels);
                }
                else
                {
                    _fseqFile->ReadData(buffer, size, adjustedMS / framems, _applyMethod, 0, 0);
                }
            }
        }
        _currentFrame++;
    }

    if (ms < _delay)
    {
        // do nothing
    }
    else
    {
        int brightness = 100;
        if (_fadeInMS != 0 && _fadeInMS < framems)
        {
            brightness = (float)framems * 100.0 / (float)_fadeInMS;
        }
        else if (_fadeOutMS != 0 && _stepLengthMS - framems < _fadeOutMS)
        {
            brightness = (float)(_stepLengthMS - framems) * 100.0 / (float)_fadeOutMS;
        }

        if (_cacheVideo)
        {
            if (_cachedVideoReader != nullptr)
            {
                while (_loopVideo && adjustedMS > _cachedVideoReader->GetLengthMS())
                {
                    adjustedMS -= _cachedVideoReader->GetLengthMS();
                }

                _window->SetImage(CachedVideoReader::FadeImage(_cachedVideoReader->GetNextFrame(adjustedMS), brightness));
            }
        }
        else
        {
            if (_videoReader != nullptr)
            {
                while (_loopVideo && adjustedMS > _videoReader->GetLengthMS())
                {
                    adjustedMS -= _videoReader->GetLengthMS();
                }

                AVFrame* img = _videoReader->GetNextFrame(adjustedMS, framems);
                _window->SetImage(CachedVideoReader::FadeImage(CachedVideoReader::CreateImageFromFrame(img, _size), brightness));
            }
        }
        if (sw.Time() > framems / 2)
        {
            logger_base.warn("   Getting frame %ld from FSEQvideo %s took more than half a frame: %ld.", (long)adjustedMS, (const char *)GetNameNoTime().c_str(), (long)sw.Time());
        }
    }
}

void PlayListItemFSEQVideo::Restart()
{
    if (ControlsTiming() && _audioManager != nullptr)
    {
        _audioManager->Stop();
        if (_delay == 0)
        {
            _audioManager->Play(0, _audioManager->LengthMS());
        }
        else
        {
            _audioManager->Seek(0);
        }
    }
    _currentFrame = 0;
}

void PlayListItemFSEQVideo::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    if (_suppressVirtualMatrix)
    {
        xScheduleFrame::GetScheduleManager()->SuppressVM(true);
    }

    // load the FSEQ
    // load the audio
    LoadFiles(true);

    _currentFrame = 0;
    if (ControlsTiming() && _audioManager != nullptr)
    {
        if (_delay == 0)
        {
            _audioManager->Play(0, _audioManager->LengthMS());
        }
        else
        {
            _audioManager->Seek(0);
        }
    }

    // create the window
    if (_window == nullptr)
    {
        _window = new PlayerWindow(wxGetApp().GetTopWindow(), _topMost, wxIMAGE_QUALITY_HIGH, wxID_ANY, _origin, _size);
    }
    else
    {
        _window->Move(_origin);
        _window->SetSize(_size);
    }
}

void PlayListItemFSEQVideo::Suspend(bool suspend)
{
    Pause(suspend);

    if (suspend)
    {
        if (_window != nullptr) _window->Hide();
    }
    else
    {
        if (_window != nullptr) _window->Show();
    }
}

void PlayListItemFSEQVideo::Pause(bool pause)
{
    if (ControlsTiming() && _audioManager != nullptr)
    {
        if (pause)
        {
            _audioManager->Pause();
        }
        else
        {
            _audioManager->Play();
        }
    }
}

void PlayListItemFSEQVideo::Stop()
{
    if (_suppressVirtualMatrix)
    {
        xScheduleFrame::GetScheduleManager()->SuppressVM(false);
    }

    if (_audioManager != nullptr)
    {
        _audioManager->Stop();
    }

    CloseFiles();

    // destroy the window
    if (_window != nullptr)
    {
        delete _window;
        _window = nullptr;
    }
    _currentFrame = 0;
}

void PlayListItemFSEQVideo::CloseFiles()
{
    if (_fseqFile != nullptr)
    {
        _fseqFile->Close();
        delete _fseqFile;
        _fseqFile = nullptr;
    }

    if (_audioManager != nullptr)
    {
        if (!_fastStartAudio)
        {
            delete _audioManager;
            _audioManager = nullptr;
        }
        else
        {
            _audioManager->AbsoluteStop();
        }
    }

    if (_videoReader != nullptr)
    {
        delete _videoReader;
        _videoReader = nullptr;
    }

    if (_cachedVideoReader != nullptr)
    {
        delete _cachedVideoReader;
        _cachedVideoReader = nullptr;
    }
}

PlayListItemFSEQVideo::~PlayListItemFSEQVideo()
{
    CloseFiles();
	
    if (_audioManager != nullptr)
    {
        delete _audioManager;
        _audioManager = nullptr;
    }

    if (_window != nullptr)
    {
        delete _window;
        _window = nullptr;
    }
}

std::list<std::string> PlayListItemFSEQVideo::GetMissingFiles()
{
    std::list<std::string> res;
    if (!wxFile::Exists(GetFSEQFileName()))
    {
        res.push_back(GetFSEQFileName());
    }
    auto af = GetAudioFilename();
    if (af != "" && !wxFile::Exists(af))
    {
        res.push_back(af);
    }
    if (!wxFile::Exists(GetVideoFile()))
    {
        res.push_back(GetVideoFile());
    }

    return res;
}

bool PlayListItemFSEQVideo::SetPosition(size_t frame, size_t ms)
{
    //wxASSERT(abs((long)frame * _msPerFrame - (long)ms) < _msPerFrame);

    _currentFrame = frame;
    if (_audioManager != nullptr)
    {
        _audioManager->Seek(frame * _msPerFrame);
        return true;
    }

    return false;
}

long PlayListItemFSEQVideo::GetFSEQChannels() const
{
    if (_fseqFile != nullptr)
    {
        return _fseqFile->GetChannels();
    }
    else
    {
        if (wxFile::Exists(_fseqFileName))
        {
            auto fseqFile = new FSEQFile();
            fseqFile->Load(_fseqFileName);
            long ch = fseqFile->GetChannels();
            fseqFile->Close();
            delete fseqFile;
            return ch;
        }
        else
        {
            return 0;
        }
    }
}

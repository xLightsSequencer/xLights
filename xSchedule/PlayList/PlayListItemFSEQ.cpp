#include "PlayListItemFSEQ.h"
#include "wx/xml/xml.h"
#include <wx/notebook.h>
#include "PlayListItemFSEQPanel.h"
#include "../../xLights/AudioManager.h"
#include <log4cpp/Category.hh>
#include "../../xLights/UtilFunctions.h"
#include "../../xLights/FSEQFile.h"
#include "../../xLights/outputs/OutputManager.h"

PlayListItemFSEQ::PlayListItemFSEQ(OutputManager* outputManager, wxXmlNode* node) : PlayListItem(node)
{
    _outputManager = outputManager;
    _fastStartAudio = false;
    _cachedAudioFilename = "";
    _currentFrame = 0;
    _channels = 0;
    _sc = 0;
    _startChannel = "1";
    _controlsTimingCache = false;
    _applyMethod = APPLYMETHOD::METHOD_OVERWRITE;
    _fseqFileName = "";
    _overrideAudio = false;
    _audioFile = "";
    _durationMS = 0;
    _fseqFile = nullptr;
    _audioManager = nullptr;
    PlayListItemFSEQ::Load(node);
}

void PlayListItemFSEQ::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _fseqFileName = node->GetAttribute("FSEQFile", "");
    _fseqFileName = FixFile("", _fseqFileName);
    _audioFile = node->GetAttribute("AudioFile", "");
    _startChannel = node->GetAttribute("StartChannel", "1").ToStdString();
    _audioFile = FixFile("", _audioFile);
    _overrideAudio = (_audioFile != "");
    _applyMethod = (APPLYMETHOD)wxAtoi(node->GetAttribute("ApplyMethod", ""));
    _channels = wxAtol(node->GetAttribute("Channels", "0"));
    _fastStartAudio = (node->GetAttribute("FastStartAudio", "FALSE") == "TRUE");
    _currentFrame = 0;

    //if (_fastStartAudio)
    //{
    //    LoadAudio();
    //}
    //else
    {
        FastSetDuration();
    }
}

std::string PlayListItemFSEQ::GetAudioFilename()
{
    if (_overrideAudio)
    {
        return _audioFile;
    }
    else
    {
        if (_fseqFile != nullptr)
        {
            return _fseqFile->getMediaFilename();
        }
        else
        {
            if (_cachedAudioFilename != "")
            {
                return _cachedAudioFilename;
            }
            _cachedAudioFilename = FSEQFile::getMediaFilename(_fseqFileName);
            return _cachedAudioFilename;
        }
    }

    return "";
}

void PlayListItemFSEQ::LoadAudio()
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
        logger_base.debug("FSEQ: Loading audio file '%s'.", (const char *)af.c_str());
        _audioManager = new AudioManager(af);

        if (!_audioManager->IsOk())
        {
            logger_base.error("FSEQ: Audio file '%s' has a problem opening.", (const char *)af.c_str());
            if (_fseqFile != nullptr)
                _durationMS = _fseqFile->getTotalTimeMS();
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
        std::unique_ptr<FSEQFile> fseq(FSEQFile::openFSEQFile(_fseqFileName));
        if (fseq) {
            size_t durationFSEQ = fseq->getTotalTimeMS();
            if (durationFSEQ < _durationMS)
            {
                logger_base.debug("Audio length %ld overridden by FSEQ length %ld.", (long)_durationMS, (long)durationFSEQ);
                _durationMS = durationFSEQ;
            }
        }
    }
    else
    {
        if (af != "")
        {
            logger_base.error("FSEQ: Audio file '%s' cannot be opened because it does not exist.", (const char *)af.c_str());
        }
    }
}

void PlayListItemFSEQ::LoadFiles()
{
    CloseFiles();

    if (wxFile::Exists(_fseqFileName))
    {
        _fseqFile = FSEQFile::openFSEQFile(_fseqFileName);
        if (_fseqFile != nullptr)
        {
            _msPerFrame = _fseqFile->getStepTime();
            _durationMS = _fseqFile->getTotalTimeMS();
        }
        else
        {
            _msPerFrame = 50;
            _durationMS = 0;
        }
    }

    LoadAudio();
}

long PlayListItemFSEQ::GetFSEQChannels() const
{
    if (_fseqFile != nullptr)
    {
        return _fseqFile->getMaxChannel() + 1;
    }
    else
    {
        if (wxFile::Exists(_fseqFileName))
        {
            std::unique_ptr<FSEQFile> fseqFile(FSEQFile::openFSEQFile(_fseqFileName));
            if (fseqFile) {
                long ch = fseqFile->getMaxChannel() + 1;
                return ch;
            }
        }
    }
    return 0;
}

PlayListItemFSEQ::PlayListItemFSEQ(OutputManager* outputManager) : PlayListItem()
{
    _outputManager = outputManager;
    _cachedAudioFilename = "";
    _fastStartAudio = false;
    _channels = 0;
    _sc = 0;
    _startChannel = "1";
    _controlsTimingCache = false;
    _applyMethod = APPLYMETHOD::METHOD_OVERWRITE;
    _fseqFileName = "";
    _overrideAudio = false;
    _audioFile = "";
    _durationMS = 0;
    _audioManager = nullptr;
    _fseqFile = nullptr;
    _currentFrame = 0;
}

PlayListItem* PlayListItemFSEQ::Copy() const
{
    PlayListItemFSEQ* res = new PlayListItemFSEQ(_outputManager);
    PlayListItem::Copy(res);
    res->_outputManager = _outputManager;
    res->_cachedAudioFilename = _cachedAudioFilename;
    res->_fseqFileName = _fseqFileName;
    res->_applyMethod = _applyMethod;
    res->_overrideAudio = _overrideAudio;
    res->_durationMS = _durationMS;
    res->_controlsTimingCache = _controlsTimingCache;
    res->_channels = _channels;
    res->_startChannel = _startChannel;
    res->_fastStartAudio = _fastStartAudio;
    res->SetAudioFile(_audioFile); // this will trigger file loading

    if (_fastStartAudio)
    {
        res->LoadFiles();
    }

    return res;
}

wxXmlNode* PlayListItemFSEQ::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIFSEQ");

    node->AddAttribute("FSEQFile", _fseqFileName);
    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_applyMethod));
    node->AddAttribute("StartChannel", _startChannel);
    node->AddAttribute("Channels", wxString::Format(wxT("%ld"), (long)_channels));
    if (_fastStartAudio)
    {
        node->AddAttribute("FastStartAudio", "TRUE");
    }

    if (_overrideAudio)
    {
        node->AddAttribute("AudioFile", _audioFile);
    }

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemFSEQ::GetTitle() const
{
    return "FSEQ";
}

size_t PlayListItemFSEQ::GetStartChannelAsNumber()
{
    if (_sc == 0)
    {
        _sc = _outputManager->DecodeStartChannel(_startChannel);
    }

    return _sc;
}

void PlayListItemFSEQ::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemFSEQPanel(notebook, _outputManager, this), GetTitle(), true);
}

std::string PlayListItemFSEQ::GetNameNoTime() const
{
    wxFileName fn(_fseqFileName);
    if (fn.GetName() == "")
    {
        return "FSEQ";
    }
    else
    {
        return fn.GetName().ToStdString();
    }
}

void PlayListItemFSEQ::SetFSEQFileName(const std::string& fseqFileName)
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

void PlayListItemFSEQ::SetAudioFile(const std::string& audioFile)
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

void PlayListItemFSEQ::SetOverrideAudio(bool overrideAudio)
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

void PlayListItemFSEQ::SetFastStartAudio(bool fastStartAudio)
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

void PlayListItemFSEQ::FastSetDuration()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _controlsTimingCache = false;
    std::string af = GetAudioFile();
    if (af == "")
    {
        std::unique_ptr<FSEQFile> fseq(FSEQFile::openFSEQFile(_fseqFileName));

        if (fseq) {
            af = fseq->getMediaFilename();

            if (!_overrideAudio && af != "" && wxFile::Exists(af))
            {
                _durationMS = AudioManager::GetAudioFileLength(fseq->getMediaFilename());
                _controlsTimingCache = true;

                // If the FSEQ is shorter than the audio ... then override the length
                size_t durationFSEQ = fseq->getTotalTimeMS();
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
                _durationMS = fseq->getTotalTimeMS();
            }
        }
    }
    else
    {
        _durationMS = AudioManager::GetAudioFileLength(GetAudioFilename());
        _controlsTimingCache = true;

        // If the FSEQ is shorter than the audio ... then override the length
        std::unique_ptr<FSEQFile> fseq(FSEQFile::openFSEQFile(_fseqFileName));
        size_t durationFSEQ = 0;
        if (fseq) {
            durationFSEQ = fseq->getTotalTimeMS();
        }
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

bool PlayListItemFSEQ::Advance(int seconds)
{
    int adjustFrames = seconds * 1000 / (int)GetFrameMS();
    _currentFrame += adjustFrames;
    if (_currentFrame < 0) _currentFrame = 0;
    if (_currentFrame > _stepLengthMS / GetFrameMS()) _currentFrame = _stepLengthMS / GetFrameMS();

    if (ControlsTiming() && _audioManager != nullptr)
    {
        long newPos = _audioManager->Tell() + seconds * 1000;
        if (newPos < 0) newPos = 0;
        if (newPos > _audioManager->LengthMS()) newPos = _audioManager->LengthMS() - 5;
        _audioManager->Seek(newPos);
        return true;
    }
    return false;
}

size_t PlayListItemFSEQ::GetPositionMS() const
{
    if (ControlsTiming() && _audioManager != nullptr)
    {
        if (_delay != 0)
        {
            if (_currentFrame * _msPerFrame < _delay)
            {
                return _currentFrame * _msPerFrame;
            }
        }
        return _delay + _audioManager->Tell();
    }
    else
    {
        return _currentFrame * _msPerFrame;
    }
}

bool PlayListItemFSEQ::Done() const
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //logger_base.debug("FSEQ Done %d <- %ld >= %ld - %ld", GetPositionMS() >= GetDurationMS() - _msPerFrame, GetPositionMS(), GetDurationMS(), _msPerFrame);
    return GetPositionMS() >= GetDurationMS() - _msPerFrame;
}

void PlayListItemFSEQ::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (outputframe)
    {
        if (_fseqFile != nullptr)
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

                ms -= _delay;
                
                int frame =  ms / framems;
                FSEQFile::FrameData *data = _fseqFile->getFrame(frame);
                std::vector<uint8_t> buf(_fseqFile->getMaxChannel() + 1);
                data->readFrame(&buf[0]);
                size_t channelsPerFrame = (size_t)_fseqFile->getMaxChannel() + 1;
                if (_channels > 0) channelsPerFrame = std::min(_channels, (size_t)_fseqFile->getMaxChannel() + 1);
                if (_channels > 0) {
                    long offset = GetStartChannelAsNumber() - 1;
                    Blend(buffer, size, &buf[offset], channelsPerFrame, _applyMethod, offset);
                } else {
                    Blend(buffer, size, &buf[0], channelsPerFrame, _applyMethod, 0);
                }
                delete data;
            }
        }
        _currentFrame++;
        //logger_base.debug("Current Frame %d", _currentFrame);
    }
}

void PlayListItemFSEQ::Restart()
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

void PlayListItemFSEQ::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    // load the FSEQ
    // load the audio
    LoadFiles();

    if (_fseqFile != nullptr)
    {
        _fseqFile->prepareRead({ { 0, _fseqFile->getMaxChannel() + 1 } });
    }

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

    _currentFrame = 0;
}

void PlayListItemFSEQ::Suspend(bool suspend)
{
    Pause(suspend);
}

void PlayListItemFSEQ::Pause(bool pause)
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

void PlayListItemFSEQ::Stop()
{
    if (_audioManager != nullptr)
    {
        _audioManager->Stop();
    }
    CloseFiles();
    _currentFrame = 0;
}

void PlayListItemFSEQ::CloseFiles()
{
    if (_fseqFile != nullptr)
    {
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
}

PlayListItemFSEQ::~PlayListItemFSEQ()
{
    CloseFiles();

    if (_audioManager != nullptr)
    {
        delete _audioManager;
        _audioManager = nullptr;
    }
}

std::list<std::string> PlayListItemFSEQ::GetMissingFiles() 
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

    return res;
}

bool PlayListItemFSEQ::SetPosition(size_t frame, size_t ms)
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

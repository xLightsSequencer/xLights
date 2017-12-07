#include "PlayListItemFSEQ.h"
#include "wx/xml/xml.h"
#include <wx/notebook.h>
#include "PlayListItemFSEQPanel.h"
#include "../../xLights/AudioManager.h"
#include <log4cpp/Category.hh>
#include "../../xLights/UtilFunctions.h"
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

void PlayListItemFSEQ::LoadAudio()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    auto af = GetAudioFilename();

    if (_audioManager != nullptr)
    {
        if (_audioManager->FileName() == af)
        {
            // already open
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
        _audioManager = new AudioManager(af);

        if (!_audioManager->IsOk())
        {
            logger_base.error("FSEQ: Audio file '%s' has a problem opening.", (const char *)af.c_str());
        }

        if (_volume != -1)
            _audioManager->SetVolume(_volume);
        _durationMS = _audioManager->LengthMS();
        _controlsTimingCache = true;
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
        _fseqFile = new FSEQFile();
        _fseqFile->Load(_fseqFileName);
        _msPerFrame = _fseqFile->GetFrameMS();
        _durationMS = _fseqFile->GetLengthMS();
    }

    LoadAudio();
}

long PlayListItemFSEQ::GetFSEQChannels() const
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
    }
}

size_t PlayListItemFSEQ::GetPositionMS() const
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

    return 0;
}

void PlayListItemFSEQ::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
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
                if (_channels > 0)
                {
                    long sc = GetStartChannelAsNumber();
                    wxASSERT(sc > 0);
                    _fseqFile->ReadData(buffer, size, ms / framems, _applyMethod, sc - 1, _channels);
                }
                else
                {
                    _fseqFile->ReadData(buffer, size, ms / framems, _applyMethod, 0, 0);
                }
            }
        }
        _currentFrame++;
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

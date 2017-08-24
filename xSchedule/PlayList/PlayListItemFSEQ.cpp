#include "PlayListItemFSEQ.h"
#include "wx/xml/xml.h"
#include <wx/notebook.h>
#include "PlayListItemFSEQPanel.h"
#include "../../xLights/AudioManager.h"
#include <log4cpp/Category.hh>

PlayListItemFSEQ::PlayListItemFSEQ(wxXmlNode* node) : PlayListItem(node)
{
    _channels = 0;
    _startChannel = 1;
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
    _audioFile = node->GetAttribute("AudioFile", "");
    _overrideAudio = (_audioFile != "");
    _applyMethod = (APPLYMETHOD)wxAtoi(node->GetAttribute("ApplyMethod", ""));
    _fastStartAudio = (node->GetAttribute("FastStartAudio", "FALSE") == "TRUE");

    if (_fastStartAudio)
    {
        LoadAudio();
    }
    else
    {
        FastSetDuration();
    }
}

std::string PlayListItemFSEQ::GetAudioFilename() const
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
            FSEQFile f(_fseqFileName);
            if (f.IsOk())
            {
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

    if (wxFile::Exists(af))
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

PlayListItemFSEQ::PlayListItemFSEQ() : PlayListItem()
{
    _fastStartAudio = false;
    _channels = 0;
    _startChannel = 1;
    _controlsTimingCache = false;
    _applyMethod = APPLYMETHOD::METHOD_OVERWRITE;
    _fseqFileName = "";
    _overrideAudio = false;
    _audioFile = "";
    _durationMS = 0;
    _audioManager = nullptr;
    _fseqFile = nullptr;
}

PlayListItem* PlayListItemFSEQ::Copy() const
{
    PlayListItemFSEQ* res = new PlayListItemFSEQ();
    res->_fseqFileName = _fseqFileName;
    res->_applyMethod = _applyMethod;
    res->_overrideAudio = _overrideAudio;
    res->_durationMS = _durationMS;
    res->_controlsTimingCache = _controlsTimingCache;
    res->_channels = _channels;
    res->_startChannel = _startChannel;
    res->_fastStartAudio = _fastStartAudio;
    res->SetAudioFile(_audioFile); // this will trigger file loading
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemFSEQ::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIFSEQ");

    node->AddAttribute("FSEQFile", _fseqFileName);
    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_applyMethod));
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

void PlayListItemFSEQ::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemFSEQPanel(notebook, this), GetTitle(), true);
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
        return _audioManager->Tell();
    }

    return 0;
}

void PlayListItemFSEQ::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (outputframe)
    {
        if (_fseqFile != nullptr)
        {
            if (_channels > 0)
            {
                wxASSERT(_startChannel > 0);
                _fseqFile->ReadData(buffer, size, ms / framems, _applyMethod, _startChannel - 1, _channels);
            }
            else
            {
                _fseqFile->ReadData(buffer, size, ms / framems, _applyMethod, 0, 0);
            }
        }
    }
}

void PlayListItemFSEQ::Restart()
{
    if (ControlsTiming() && _audioManager != nullptr)
    {
        _audioManager->Stop();
        _audioManager->Play(0, _audioManager->LengthMS());
    }
}

void PlayListItemFSEQ::Start()
{
    // load the FSEQ
    // load the audio
    LoadFiles();

    if (ControlsTiming() && _audioManager != nullptr)
    {
        _audioManager->Play(0, _audioManager->LengthMS());
    }
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
        delete _audioManager;
        _audioManager = nullptr;
    }
}

PlayListItemFSEQ::~PlayListItemFSEQ()
{
    CloseFiles();
}

std::list<std::string> PlayListItemFSEQ::GetMissingFiles() const
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
    wxASSERT(abs((long)frame * _msPerFrame - (long)ms) < _msPerFrame);

    if (_audioManager != nullptr)
    {
        _audioManager->Seek(frame * _msPerFrame);
        return true;
    }

    return false;
}

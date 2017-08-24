#include "PlayListItemAudio.h"
#include "wx/xml/xml.h"
#include <wx/notebook.h>
#include "PlayListItemAudioPanel.h"
#include "../../xLights/AudioManager.h"
#include <log4cpp/Category.hh>

PlayListItemAudio::PlayListItemAudio(wxXmlNode* node) : PlayListItem(node)
{
    _fastStartAudio = false;
    _controlsTimingCache = false;
    _audioFile = "";
    _durationMS = 0;
    _audioManager = nullptr;
    PlayListItemAudio::Load(node);
}

void PlayListItemAudio::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _fastStartAudio = (node->GetAttribute("FastStartAudio", "FALSE") == "TRUE");
    _audioFile = node->GetAttribute("AudioFile", "");

    if (_fastStartAudio)
    {
        LoadFiles();
    }
    else
    {
        FastSetDuration();
    }
}

void PlayListItemAudio::LoadFiles()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_audioManager != nullptr)
    {
        if (_audioManager->FileName() == _audioFile)
        {
            // already open
            return;
        }
        else
        {
            CloseFiles();
        }
    }

    if (IsInSlaveMode())
    {
        
    }
    else if (wxFile::Exists(_audioFile))
    {
        _audioManager = new AudioManager(_audioFile);

        if (!_audioManager->IsOk())
        {
            logger_base.error("Audio: Audio file '%s' has a problem opening.", (const char *)_audioFile.c_str());
        }

        if (_volume != -1)
            _audioManager->SetVolume(_volume);
        _durationMS = _audioManager->LengthMS();
        _controlsTimingCache = true;
    }
    else
    {
        if (_audioFile != "")
        {
            logger_base.error("Audio: Audio file '%s' cannot be opened because it does not exist.", (const char *)_audioFile.c_str());
        }
    }
}

PlayListItemAudio::PlayListItemAudio() : PlayListItem()
{
    _fastStartAudio = false;
    _controlsTimingCache = false;
    _audioFile = "";
    _durationMS = 0;
    _audioManager = nullptr;
}

PlayListItem* PlayListItemAudio::Copy() const
{
    PlayListItemAudio* res = new PlayListItemAudio();
    res->_fastStartAudio = _fastStartAudio;
    res->_durationMS = _durationMS;
    res->_controlsTimingCache = _controlsTimingCache;
    res->SetAudioFile(_audioFile); // we set this way to trigger file loading
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemAudio::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIAudio");

    node->AddAttribute("AudioFile", _audioFile);
    if (_fastStartAudio)
    {
        node->AddAttribute("FastStartAudio", "TRUE");
    }

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemAudio::GetTitle() const
{
    return "Audio";
}

void PlayListItemAudio::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemAudioPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemAudio::GetNameNoTime() const
{
    wxFileName fn(_audioFile);
    if (fn.GetName() == "")
    {
        return "Audio";
    }
    else
    {
        return fn.GetName().ToStdString();
    }
}

void PlayListItemAudio::SetAudioFile(const std::string& audioFile)
{
    if (_audioFile != audioFile)
    {
        _audioFile = audioFile;
        if (_fastStartAudio)
        {
            LoadFiles();
        }
        else
        {
            FastSetDuration();
        }
        _changeCount++;
    }
}

void PlayListItemAudio::FastSetDuration()
{
    _durationMS = 0;
    if (wxFile::Exists(_audioFile))
    {
        _durationMS = AudioManager::GetAudioFileLength(_audioFile);
        _controlsTimingCache = true;
    }
}

size_t PlayListItemAudio::GetPositionMS() const
{
    if (ControlsTiming() && _audioManager != nullptr)
    {
        return _audioManager->Tell();
    }
    
    return 0;
}

void PlayListItemAudio::Restart()
{
    if (ControlsTiming() && _audioManager != nullptr)
    {
        _audioManager->Stop();
        _audioManager->Play(0, _audioManager->LengthMS());
    }
}

void PlayListItemAudio::Start()
{
    // load the audio
    LoadFiles();

    if (ControlsTiming() && _audioManager != nullptr)
    {
        _audioManager->Play(0, _audioManager->LengthMS());
    }
}

void PlayListItemAudio::Suspend(bool suspend)
{
    Pause(suspend);
}

void PlayListItemAudio::Pause(bool pause)
{
    if (_audioManager != nullptr)
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

void PlayListItemAudio::Stop()
{
    if (_audioManager != nullptr)
    {
        _audioManager->Stop();
    }
    CloseFiles();
}

void PlayListItemAudio::CloseFiles()
{
    if (_audioManager != nullptr)
    {
        delete _audioManager;
        _audioManager = nullptr;
    }
}

bool PlayListItemAudio::IsAudio(const std::string& ext)
{
    if (ext == "mp3" ||
        ext == "ogg" ||
        ext == "mid" ||
        ext == "au" ||
        ext == "wav" ||
        ext == "m4a" ||
        ext == "wma" ||
        ext == "m4p"
        )
    {
        return true;
    }

    return false;
}

void PlayListItemAudio::SetFastStartAudio(bool fastStartAudio)
{
    if (_fastStartAudio != fastStartAudio)
    {
        _fastStartAudio = fastStartAudio;
        _changeCount++;
        if (_fastStartAudio)
        {
            LoadFiles();
        }
        else
        {
            CloseFiles();
        }
    }
}

std::list<std::string> PlayListItemAudio::GetMissingFiles() const
{
    std::list<std::string> res;
    if (!wxFile::Exists(GetAudioFile()))
    {
        res.push_back(GetAudioFile());
    }

    return res;
}
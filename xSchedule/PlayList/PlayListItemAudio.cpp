#include "PlayListItemAudio.h"
#include "wx/xml/xml.h"
#include <wx/notebook.h>
#include "PlayListItemAudioPanel.h"
#include "../../xLights/AudioManager.h"

PlayListItemAudio::PlayListItemAudio(wxXmlNode* node) : PlayListItem(node)
{
    _audioFile = "";
    _durationMS = 0;
    _audioManager = nullptr;
    PlayListItemAudio::Load(node);
}

void PlayListItemAudio::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _audioFile = node->GetAttribute("AudioFile", "");
    FastSetDuration();
}

void PlayListItemAudio::LoadFiles()
{
    CloseFiles();

    if (wxFile::Exists(_audioFile))
    {
        _audioManager = new AudioManager(_audioFile);
        _durationMS = _audioManager->LengthMS();
    }
}

PlayListItemAudio::PlayListItemAudio() : PlayListItem()
{
    _audioFile = "";
    _durationMS = 0;
    _audioManager = nullptr;
}

PlayListItem* PlayListItemAudio::Copy() const
{
    PlayListItemAudio* res = new PlayListItemAudio();
    res->_audioFile = _audioFile;
    res->_durationMS = _durationMS;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemAudio::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIAudio");

    node->AddAttribute("AudioFile", _audioFile);

    PlayListItem::Save(node);

    return node;
}

void PlayListItemAudio::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemAudioPanel(notebook, this), "Audio", true);
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
    _audioFile = audioFile;
    FastSetDuration();
    _dirty = true;
}

void PlayListItemAudio::FastSetDuration()
{
    _durationMS = 0;
    if (wxFile::Exists(_audioFile))
    {
        _durationMS = AudioManager::GetAudioFileLength(_audioFile);
    }
}

size_t PlayListItemAudio::GetPositionMS() const
{
    if (ControlsTiming())
    {
        return _audioManager->Tell();
    }
    
    return 0;
}

void PlayListItemAudio::Restart()
{
    if (ControlsTiming())
    {
        _audioManager->Stop();
        _audioManager->Play(0, _audioManager->LengthMS());
    }
}

void PlayListItemAudio::Start()
{
    // load the audio
    LoadFiles();

    if (ControlsTiming())
    {
        _audioManager->Play(0, _audioManager->LengthMS());
    }
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

#include "PlayListItemFSEQ.h"
#include "wx/xml/xml.h"
#include <wx/notebook.h>
#include "PlayListItemFSEQPanel.h"
#include "../../xLights/AudioManager.h"

PlayListItemFSEQ::PlayListItemFSEQ(wxXmlNode* node) : PlayListItem(node)
{
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
    FastSetDuration();
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
    }

    return "";
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
    auto af = GetAudioFilename();
    if (wxFile::Exists(af))
    {
        _audioManager = new AudioManager(af);
        _durationMS = _audioManager->LengthMS();
    }
}

PlayListItemFSEQ::PlayListItemFSEQ() : PlayListItem()
{
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
    res->_audioFile = _audioFile;
    res->_durationMS = _durationMS;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemFSEQ::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIFSEQ");

    node->AddAttribute("FSEQFile", _fseqFileName);
    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_applyMethod));

    if (_overrideAudio)
    {
        node->AddAttribute("AudioFile", _audioFile);
    }

    PlayListItem::Save(node);

    return node;
}

void PlayListItemFSEQ::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemFSEQPanel(notebook, this), "FSEQ", true);
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
    _fseqFileName = fseqFileName;
    FastSetDuration();
    _dirty = true;
}

void PlayListItemFSEQ::FastSetDuration()
{
    std::string af = GetAudioFile();
    if (af == "")
    {
        FSEQFile fseq(_fseqFileName);
        _durationMS = fseq.GetLengthMS();
    }
    else
    {
        _durationMS = AudioManager::GetAudioFileLength(GetAudioFilename());
    }

}

size_t PlayListItemFSEQ::GetPositionMS() const
{
    if (ControlsTiming())
    {
        return _audioManager->Tell();
    }
    
    return 0;
}

void PlayListItemFSEQ::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems)
{
    _fseqFile->ReadData(buffer, size, ms / framems, _applyMethod);
}

void PlayListItemFSEQ::Restart()
{
    if (ControlsTiming())
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

    if (ControlsTiming())
    {
        _audioManager->Play(0, _audioManager->LengthMS());
    }
}

void PlayListItemFSEQ::Pause(bool pause)
{
    if (ControlsTiming())
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
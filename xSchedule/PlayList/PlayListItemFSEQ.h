#ifndef PLAYLISTITEMFSEQ_H
#define PLAYLISTITEMFSEQ_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;

class PlayListItemFSEQ : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _fseqFile;
    std::string _audioFile;
    bool _overrideAudio;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemFSEQ(wxXmlNode* node);
    PlayListItemFSEQ();
    virtual ~PlayListItemFSEQ() {};
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetName() const override;
    std::string GetFSEQFile() const { return _fseqFile; }
    std::string GetAudioFile() const { return _audioFile; }
    bool GetOverrideAudio() const { return _overrideAudio; }
    void SetFSEQFile(const std::string& fseqFile) { _fseqFile = fseqFile; }
    void SetAudioFile(const std::string& audioFile) { _audioFile = audioFile; }
    void SetOverrideAudio(bool overrideAudio) { _overrideAudio = overrideAudio; }
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Play() override {};
    virtual void Stop() override {};
    virtual void PlayFrame(long frame) override {};
    virtual wxByte* GetFrameData(long frame) override { return nullptr; }
    #pragma endregion Playing

#pragma region UI
    // returns nullptr if cancelled
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif
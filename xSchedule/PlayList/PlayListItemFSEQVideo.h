#ifndef PLAYLISTITEMFSEQVIDEO_H
#define PLAYLISTITEMFSEQVIDEO_H

#include "PlayListItem.h"
#include "../FSEQFile.h"
#include <string>

class wxXmlNode;
class wxWindow;
class AudioManager;
class PlayerWindow;
class VideoReader;

class PlayListItemFSEQVideo : public PlayListItem
{
protected:

    #pragma region Member Variables
    APPLYMETHOD _applyMethod;
    std::string _fseqFileName;
    std::string _audioFile;
    bool _overrideAudio;
    bool _topMost;
    bool _suppressVirtualMatrix;
    FSEQFile* _fseqFile;
    AudioManager* _audioManager;
    size_t _durationMS;
    bool _controlsTimingCache;
    size_t _startChannel;
    size_t _channels;
    bool _fastStartAudio;
    bool _cacheVideo;
    VideoReader* _videoReader;

	std::string _videoFile;
	wxPoint _origin;
	wxSize _size;
	PlayerWindow* _window;
#pragma endregion Member Variables

    void LoadFiles();
    void CloseFiles(bool purgeCache);
    void FastSetDuration();
    void LoadAudio();

public:

    #pragma region Constructors and Destructors
    PlayListItemFSEQVideo(wxXmlNode* node);
    PlayListItemFSEQVideo();
    virtual ~PlayListItemFSEQVideo();
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    AudioManager* GetAudioManager() const { return _audioManager; }
    bool GetTopMost() const { return _topMost; }
    void SetTopmost(bool topmost) { if (_topMost != topmost) { _topMost = topmost; _changeCount++; } }
    bool GetSuppressVirtualMatrix() const { return _suppressVirtualMatrix; }
    void SetSuppressVirtualMatrix(bool suppressVirtualMatrix) { if (_suppressVirtualMatrix != suppressVirtualMatrix) { _suppressVirtualMatrix = suppressVirtualMatrix; _changeCount++; } }
    std::string GetAudioFilename() const;
    int GetBlendMode() const { return _applyMethod; }
    void SetBlendMode(const std::string& blendMode) { if (_applyMethod != EncodeBlendMode(blendMode)) { _applyMethod = EncodeBlendMode(blendMode); _changeCount++; } }
    virtual size_t GetDurationMS() const override { return _delay + _durationMS; }
    virtual std::string GetNameNoTime() const override;
    std::string GetFSEQFileName() const { return _fseqFileName; }
    std::string GetAudioFile() const { return _audioFile; }
    bool GetOverrideAudio() const { return _overrideAudio; }
    bool GetFastStartAudio() const { return _fastStartAudio; }
    bool GetCacheVideo() const { return _cacheVideo; }
    void SetFSEQFileName(const std::string& fseqFileName);
    void SetAudioFile(const std::string& audioFile);
    void SetOverrideAudio(bool overrideAudio);
    void SetFastStartAudio(bool fastStartAudio);
    virtual bool ControlsTiming() const override { return _controlsTimingCache || _audioManager != nullptr; }
    virtual size_t GetPositionMS() const override;
    virtual size_t GetFrameMS() const override { return _msPerFrame; }
    virtual bool Done() const override { return GetPositionMS() >= GetDurationMS() - GetFrameMS(); }
    virtual std::string GetSyncItemFSEQ() const override { return GetFSEQFileName(); }
    virtual std::string GetSyncItemMedia() const override { return GetAudioFilename(); }
    virtual std::string GetTitle() const override;
    long GetStartChannel() const { return _startChannel; }
    void SetStartChannel(long startChannel) { if (_startChannel != startChannel) { _startChannel = startChannel; _changeCount++; } }
    long GetChannels() const { return _channels; }
    void SetChannels(long channels) { if (_channels != channels) { _channels = channels; _changeCount++; } }
    void SetCacheVideo(bool cacheVideo) { if (_cacheVideo != cacheVideo) { _cacheVideo = cacheVideo; _changeCount++; } }
    bool SetPosition(size_t frame, size_t ms);
    void SetLocation(wxPoint pt, wxSize size) { if (_origin != pt || _size != size) { _origin = pt; _size = size; _changeCount++; } }
    void SetVideoFile(const std::string& videoFile);
    std::string GetVideoFile() const { return _videoFile; }
    wxPoint GetPosition() const { return _origin; }
    wxSize GetSize() const { return _size; }
    virtual std::list<std::string> GetMissingFiles() const override;
    virtual long GetFSEQChannels() const override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Start() override;
    virtual void Stop() override;
    virtual void Restart() override;
    virtual void Pause(bool pause) override;
    virtual void Suspend(bool suspend) override;
    #pragma endregion Playing

#pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif

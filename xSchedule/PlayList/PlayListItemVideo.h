#ifndef PLAYLISTITEMVIDEO_H
#define PLAYLISTITEMVIDEO_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;
class PlayerWindow;
class VideoReader;
class CachedVideoReader;

class PlayListItemVideo : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _videoFile;
	wxPoint _origin;
	wxSize _size;
    bool _suppressVirtualMatrix;
    bool _topMost;
    bool _cacheVideo;
    bool _loopVideo;
    VideoReader* _videoReader;
    CachedVideoReader* _cachedVideoReader;
    size_t _durationMS;
    PlayerWindow* _window;
    #pragma endregion Member Variables
    int _fadeInMS;
    int _fadeOutMS;

    void OpenFiles(bool doCache);
    void CloseFiles();

public:

    #pragma region Constructors and Destructors
    PlayListItemVideo(wxXmlNode* node);
    PlayListItemVideo();
    virtual ~PlayListItemVideo();
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    bool GetTopMost() const { return _topMost; }
    bool GetCacheVideo() const { return _cacheVideo; }
    bool GetLoopVideo() const { return _loopVideo; }
    void SetTopmost(bool topmost) { if (_topMost != topmost) { _topMost = topmost; _changeCount++; } }
    void SetCacheVideo(bool cacheVideo) { if (_cacheVideo != cacheVideo) { _cacheVideo = cacheVideo; _changeCount++; } }
    void SetLoopVideo(bool loopVideo) { if (_loopVideo != loopVideo) { _loopVideo = loopVideo; _changeCount++; } }
    bool GetSuppressVirtualMatrix() const { return _suppressVirtualMatrix; }
    void SetSuppressVirtualMatrix(bool suppressVirtualMatrix) { if (_suppressVirtualMatrix != suppressVirtualMatrix) { _suppressVirtualMatrix = suppressVirtualMatrix; _changeCount++; } }
    virtual size_t GetDurationMS() const override;
    virtual std::string GetNameNoTime() const override;
    void SetLocation(wxPoint pt, wxSize size) { if (_origin != pt || _size != size) { _origin = pt; _size = size; _changeCount++; } }
    void SetVideoFile(const std::string& videoFile);
    std::string GetVideoFile() const { return _videoFile; }
    wxPoint GetPosition() const { return _origin; }
    wxSize GetSize() const { return _size; }
    virtual std::string GetSyncItemMedia() override { return GetVideoFile(); }
    static bool IsVideo(const std::string& ext);
    virtual std::string GetTitle() const override;
    virtual std::list<std::string> GetMissingFiles() override;
    int GetFadeInMS() const { return _fadeInMS; }
    void SetFadeInMS(const int fadeInMS) { if (_fadeInMS != fadeInMS) { _fadeInMS = fadeInMS; _changeCount++; } }
    int GetFadeOutMS() const { return _fadeOutMS; }
    void SetFadeOutMS(const int fadeOutMS) { if (_fadeOutMS != fadeOutMS) { _fadeOutMS = fadeOutMS; _changeCount++; } }
#pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Start(long stepLengthMS) override;
    virtual void Stop() override;
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Suspend(bool suspend) override;
    #pragma endregion Playing

#pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif

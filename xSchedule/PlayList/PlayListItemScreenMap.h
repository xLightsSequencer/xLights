#ifndef PLAYLISTITEMSCREENMAP_H
#define PLAYLISTITEMSCREENMAP_H

#include "PlayListItem.h"
#include "Blend.h"
#include <string>

class wxXmlNode;
class MatrixMapper;

class PlayListItemScreenMap : public PlayListItem
{
protected:

    #pragma region Member Variables
    size_t _durationMS;
    APPLYMETHOD _blendMode;
    std::string _matrix;
    int _x;
    int _y;
	int _width;
	int _height;
    bool _rescale;
    std::string _quality;
    MatrixMapper* _matrixMapper;
    #pragma endregion Member Variables

    void SetPixel(wxByte* p, wxByte r, wxByte g, wxByte b, APPLYMETHOD blendMode);

public:

    #pragma region Constructors and Destructors
    PlayListItemScreenMap(wxXmlNode* node);
    PlayListItemScreenMap();
    virtual ~PlayListItemScreenMap();
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters

    void SetWidth(int width) { if (_width != width) { _width = width; _changeCount++; } }
    int GetWidth() const { return _width; }
    void SetHeight(int height) { if (_height != height) { _height = height; _changeCount++; } }
    int GetHeight() const { return _height; }
    void SetBlendMode(const std::string& blendMode) { if (_blendMode != EncodeBlendMode(blendMode)) { _blendMode = EncodeBlendMode(blendMode); _changeCount++; } }
    int GetBlendMode() const { return _blendMode; }
    void SetDuration(size_t duration) { if (_durationMS != duration) { _durationMS = duration; _changeCount++; } }
    size_t GetDuration() const { return _durationMS; }
    void SetMatrix(const std::string& matrix) { if (_matrix != matrix) { _matrix = matrix; _changeCount++; } }
    std::string GetMatrix() const { return _matrix; }
    void SetQuality(const std::string& quality) { if (_quality != quality) { _quality = quality; _changeCount++; } }
    std::string GetQuality() const { return _quality; }
    void SetRescale(bool rescale) { if (_rescale != rescale) { _rescale = rescale; _changeCount++; } }
    bool GetRescale() const { return _rescale; }
    void SetX(int x) { if (_x != x) { _x = x; _changeCount++; } }
    int GetX() const { return _x; }
    void SetY(int y) { if (_y != y) { _y = y; _changeCount++; } }
    int GetY() const { return _y; }

    virtual size_t GetDurationMS() const override;
    virtual std::string GetNameNoTime() const override;
    virtual std::string GetTitle() const override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Start(long stepLengthMS) override;
    virtual void Stop() override;
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    #pragma endregion Playing

#pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif

#ifndef PLAYLISTITEMTEXT_H
#define PLAYLISTITEMTEXT_H

#include "PlayListItem.h"
#include "../Blend.h"
#include <string>

class wxXmlNode;
class MatrixMapper;

class PlayListItemText : public PlayListItem
{
protected:

    #pragma region Member Variables
    size_t _durationMS;
    wxString _text;
    std::string _format;
    wxColour _colour;
    int _speed;
    APPLYMETHOD _blendMode;
    std::string _matrix;
    std::string _type;
    std::string _orientation;
    std::string _movement;
    wxFont* _font;
    int _x;
    int _y;
    bool _renderWhenBlank;
    wxSize _maxSize;
    MatrixMapper* _matrixMapper;
    #pragma endregion Member Variables

    wxString GetText(size_t ms);
    wxPoint GetLocation(size_t ms, wxSize size);
    void SetPixel(uint8_t* p, uint8_t r, uint8_t g, uint8_t b, APPLYMETHOD blendMode);

public:

    #pragma region Constructors and Destructors
    PlayListItemText(wxXmlNode* node);
    PlayListItemText();
    virtual ~PlayListItemText();
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    static std::string GetTooltip(const std::string& type);

    void SetSpeed(int speed) { if (_speed != speed) { _speed = speed; _changeCount++; } }
    int GetSpeed() const { return _speed; }
    void SetBlendMode(const std::string& blendMode) { if (_blendMode != EncodeBlendMode(blendMode)) { _blendMode = EncodeBlendMode(blendMode); _changeCount++; } }
    int GetBlendMode() const { return _blendMode; }
    void SetDuration(size_t duration) { if (_durationMS != duration) { _durationMS = duration; _changeCount++; } }
    size_t GetDuration() const { return _durationMS; }
    void SetText(const wxString& text) { if (_text != text) { _text = text; _changeCount++; } }
    wxString GetText() const { return _text; }
    void SetFormat(const std::string& format) { if (_format != format) { _format = format; _changeCount++; } }
    std::string GetFormat() const { return _format; }
    void SetColour(const wxColour& colour) { if (_colour != colour) { _colour = colour; _changeCount++; } }
    wxColour GetColour() const { return _colour; }
    void SetMatrix(const std::string& matrix) { if (_matrix != matrix) { _matrix = matrix; _changeCount++; } }
    std::string GetMatrix() const { return _matrix; }
    void SetType(const std::string& type) { if (_type != type) { _type = type; _changeCount++; } }
    std::string GetType() const { return _type; }
    void SetOrientation(const std::string& orientation) { if (_orientation != orientation) { _orientation = orientation; _changeCount++; } }
    std::string GetOrientation() const { return _orientation; }
    void SetMovement(const std::string& movement) { if (_movement != movement) { _movement = movement; _changeCount++; } }
    std::string GetMovement() const { return _movement; }
    void SetRenderWhenBlank(bool renderWhenBlank) { if (_renderWhenBlank != renderWhenBlank) { _renderWhenBlank = renderWhenBlank; _changeCount++; } }
    bool GetRenderWhenBlank() const { return _renderWhenBlank; }
    void SetFont(wxFont* font);
    wxFont* GetFont() const { return _font; }
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
    virtual void Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    #pragma endregion Playing

#pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif

#ifndef PLAYLISTITEMRDS_H
#define PLAYLISTITEMRDS_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;
class SerialPort;

class PlayListItemRDS : public PlayListItem
{
protected:

    #pragma region Member Variables
    bool _started;
    std::string _commPort;
    bool _highSpeed;
    int _mode;
    int _serialSpeed;
    std::string _stationName;
    std::string _text;
    int _stationDuration;
    int _lineDuration;
    bool _mrds;
    #pragma endregion Member Variables

    void Dump(unsigned char* buffer, int buflen);
    void Write(SerialPort* serial, unsigned char* buffer, int buflen);

public:

    #pragma region Constructors and Destructors
    PlayListItemRDS(wxXmlNode* node);
    PlayListItemRDS();
    virtual ~PlayListItemRDS() {};
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    static std::string GetTooltip();
    std::string GetNameNoTime() const override;
    std::string GetRawName() const { return _name; }
    void SetStationName(const std::string& stationName) { if (stationName != _stationName) { _stationName = stationName; _changeCount++; } }
    std::string GetStationName() const { return _stationName; }
    bool GetHighSpeed() const { return _highSpeed; }
    void SetHighSpeed(bool highSpeed) { if (_highSpeed != highSpeed) { _highSpeed = highSpeed; _changeCount++; } }
    bool GetMRDS() const { return _mrds; }
    void SetMRDS(bool mrds) { if (_mrds != mrds) { _mrds = mrds; _changeCount++; } }
    void SetCommPort(const std::string& commPort) { if (commPort != _commPort) { _commPort = commPort; _changeCount++; } }
    std::string GetCommPort() const { return _commPort; }
    std::string GetMode() const;
    void SetMode(const std::string& mode);
    int GetSerialSpeed() const { return _serialSpeed; }
    void SetSerialSpeed(int serialSpeed) { if (_serialSpeed != serialSpeed) { _serialSpeed = serialSpeed; _changeCount++; } }
    void SetText(const std::string& text) { if (text != _text) { _text = text; _changeCount++; } }
    std::string GetText() const { return _text; }
    int GetStationDuration() const { return _stationDuration; }
    void SetStationDuration(int stationDuration) { if (_stationDuration != stationDuration) { _stationDuration = stationDuration; _changeCount++; } }
    int GetLineDuration() const { return _lineDuration; }
    void SetLineDuration(int lineDuration) { if (_lineDuration != lineDuration) { _lineDuration = lineDuration; _changeCount++; } }
    virtual std::string GetTitle() const override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Start() override;
    #pragma endregion Playing

    #pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif
#ifndef PLAYLISTITEMSERIAL_H
#define PLAYLISTITEMSERIAL_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;

class PlayListItemSerial : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _commPort;
    std::string _configuration;
    std::string _data;
    int _speed;
    bool _started;
    #pragma endregion Member Variables

    unsigned char* PrepareData(const std::string s, int& size);

public:

    #pragma region Constructors and Destructors
    PlayListItemSerial(wxXmlNode* node);
    PlayListItemSerial();
    virtual ~PlayListItemSerial() {};
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    static std::string GetTooltip();
    std::string GetNameNoTime() const override;

    void SetCommPort(const std::string& commPort) { if (_commPort != commPort) { _commPort = commPort; _changeCount++; } }
    std::string GetCommPort() const { return _commPort; }
    void SetConfiguration(const std::string& configuration) { if (_configuration != configuration) { _configuration = configuration; _changeCount++; } }
    std::string GetConfiguration() const { return _configuration; }
    void SetSpeed(const int speed) { if (_speed != speed) { _speed = speed; _changeCount++; } }
    int GetSpeed() const { return _speed; }
    void SetData(const std::string& data) { if (_data != data) { _data = data; _changeCount++; } }
    std::string GetData() const { return _data; }
    virtual std::string GetTitle() const override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Start(long stepLengthMS) override;
    #pragma endregion Playing

    #pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif
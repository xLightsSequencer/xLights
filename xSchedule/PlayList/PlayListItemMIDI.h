#ifndef PLAYLISTITEMMIDI_H
#define PLAYLISTITEMMIDI_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;

class PlayListItemMIDI : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _device;
    std::string _status;
    std::string _data1;
    std::string _data2;
    std::string _channel;
    bool _started;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemMIDI(wxXmlNode* node);
    PlayListItemMIDI();
    virtual ~PlayListItemMIDI() {};
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    static std::list<std::string> GetDevices();

    #pragma region Getters and Setters
    std::string GetNameNoTime() const override;

    void SetDevice(const std::string& device) { if (_device != device) { _device = device; _changeCount++; } }
    std::string GetDevice() const { return _device; }
    void SetStatus(const std::string& status) { if (_status != status) { _status = status; _changeCount++; } }
    std::string GetStatus() const { return _status; }
    void SetChannel(const std::string& channel) { if (_channel != channel) { _channel = channel; _changeCount++; } }
    std::string GetChannel() const { return _channel; }
    void SetData1(const std::string& data1) { if (_data1 != data1) { _data1 = data1; _changeCount++; } }
    std::string GetData1() const { return _data1; }
    void SetData2(const std::string& data2) { if (_data2 != data2) { _data2 = data2; _changeCount++; } }
    std::string GetData2() const { return _data2; }
    virtual std::string GetTitle() const override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Start(long stepLengthMS) override;
    #pragma endregion Playing

    #pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif
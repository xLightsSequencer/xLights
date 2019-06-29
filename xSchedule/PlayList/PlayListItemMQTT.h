#ifndef PLAYLISTITEMMQTT_H
#define PLAYLISTITEMMQTT_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;

class PlayListItemMQTT : public PlayListItem
{
protected:

#pragma region Member Variables
    std::string _topic;
    std::string _brokerIP;
    std::string _username;
    std::string _password;
    int _port;
    bool _started;
    std::string _data;
#pragma endregion Member Variables

    unsigned char* PrepareData(const std::string s, int& size);

public:

#pragma region Constructors and Destructors
    PlayListItemMQTT(wxXmlNode* node);
    PlayListItemMQTT();
    virtual ~PlayListItemMQTT() {};
    virtual PlayListItem* Copy() const override;
#pragma endregion Constructors and Destructors

#pragma region Getters and Setters
    static std::string GetTooltip();
    std::string GetNameNoTime() const override;
    void SetIPAddress(const std::string& ip) { if (_brokerIP != ip) { _brokerIP = ip; _changeCount++; } }
    void SetTopic(const std::string& topic) { if (_topic != topic) { _topic = topic; _changeCount++; } }
    void SetUserName(const std::string& userName) { if (_username != userName) { _username = userName; _changeCount++; } }
    void SetPassword(const std::string& password) { if (_password != password) { _password = password; _changeCount++; } }
    void SetData(const std::string& data) { if (_data != data) { _data = data; _changeCount++; } }
    void SetPort(int port) { if (_port != port) { _port = port; _changeCount++; } }
    std::string GetIPAddress() const { return _brokerIP; }
    std::string GetTopic() const { return _topic; }
    int GetPort() const { return _port; }
    std::string GetData() const { return _data; }
    std::string GetUsername() const { return _username; }
    std::string GetPassword() const { return _password; }
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
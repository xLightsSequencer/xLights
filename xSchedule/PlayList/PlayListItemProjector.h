#ifndef PLAYLISTITEMProjector_H
#define PLAYLISTITEMProjector_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;
class AudioManager;
class wxSocketClient;

class PlayListItemProjector : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _command;
    std::string _commandData;
    std::string _parity;
    std::string _projectorProtocol;
    std::string _ipProtocol;
    std::string _ipAddress;
    std::string _password;
    std::string _commPort;
    std::string _parameter;
    int _port;
    int _baudRate;
    int _charBits;
    int _stopBits;

    bool _started;
    wxSocketClient* _socket;
    std::string _hash;

    #pragma endregion Member Variables

    void PJLinkLogout();
    bool PJLinkLogin();
    bool SendPJLinkCommand(const std::string& command);
    void ExecutePJLinkCommand();
    void ExecuteTCPCommand();
    void ExecuteSerialCommand();
    unsigned char* PrepareData(const std::string s, int& used);
    std::string BuildSerialConfiguration();

public:

    #pragma region Constructors and Destructors
    PlayListItemProjector(wxXmlNode* node);
    PlayListItemProjector();
    virtual ~PlayListItemProjector() { };
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    std::string GetCommand() const { return _command; }
    void SetCommand(std::string command) { if (_command != command) { _command = command; _changeCount++; } }
    std::string GetCommandData() const { return _commandData; }
    void SetCommandData(std::string commandData) { if (_commandData != commandData) { _commandData = commandData; _changeCount++; } }
    virtual std::string GetNameNoTime() const override;
    std::string GetPassword() const { return _password; }
    void SetPassword(const std::string& password) { if (_password != password) { _password = password; _changeCount++; } }
    std::string GetParameter() const { return _parameter; }
    void SetParameter(const std::string& parameter) { if (_parameter != parameter) { _parameter = parameter; _changeCount++; } }
    std::string GetIPProtocol() const { return _ipProtocol; }
    void SetIPProtocol(const std::string& ipProtocol) { if (_ipProtocol != ipProtocol) { _ipProtocol = ipProtocol; _changeCount++; } }
    std::string GetProjectorProtocol() const { return _projectorProtocol; }
    void SetProjectorProtocol(const std::string& protocol) { if (_projectorProtocol != protocol) { _projectorProtocol = protocol; _changeCount++; } }
    std::string GetIPAddress() const { return _ipAddress; }
    void SetIPAddress(const std::string& ipAddress) { if (_ipAddress != ipAddress) { _ipAddress = ipAddress; _changeCount++; } }
    std::string GetParity() const { return _parity; }
    void SetParity(const std::string& parity) { if (_parity != parity) { _parity = parity; _changeCount++; } }
    std::string GetCommPort() const { return _commPort; }
    void SetCommPort(const std::string& commPort) { if (_commPort != commPort) { _commPort = commPort; _changeCount++; } }
    int GetBaudRate() const { return _baudRate; }
    void SetBaudRate(int baudRate) { if (_baudRate != baudRate) { _baudRate = baudRate; _changeCount++; } }
    int GetCharBits() const { return _charBits; }
    void SetCharBits(int charBits) { if (_charBits != charBits) { _charBits = charBits; _changeCount++; } }
    int GetStopBits() const { return _stopBits; }
    void SetStopBits(int stopBits) { if (_stopBits != stopBits) { _stopBits = stopBits; _changeCount++; } }
    int GetPort() const { return _port; }
    void SetPort(int port) { if (_port != port) { _port = port; _changeCount++; } }
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
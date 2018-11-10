#ifndef PLAYLISTITEMRDS_H
#define PLAYLISTITEMRDS_H

#include "PlayListItem.h"
#include <string>
#include <atomic>

class wxXmlNode;
class wxWindow;
class SerialPort;

class PlayListItemRDS : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::atomic_bool _started;
    std::string _commPort = "COM1";
    std::string _stationName;
    std::string _text;
    #pragma endregion Member Variables

public:

    static void Dump(unsigned char* buffer, int buflen);
    static void Write(SerialPort* serial, unsigned char* buffer, int buflen);
    static void SendBitWithDTRCTS(SerialPort* serial, bool bit, int hold = 1);
    static void InitialiseDTRCTS(SerialPort* serial);
    static int SendWithDTRCTS(SerialPort* serial, char* buf, size_t len);

    #pragma region Constructors and Destructors
    PlayListItemRDS(wxXmlNode* node);
    PlayListItemRDS();
    virtual ~PlayListItemRDS();
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    static std::string GetTooltip();
    std::string GetNameNoTime() const override;
    void SetStationName(const std::string& stationName) { if (stationName != _stationName) { _stationName = stationName; _changeCount++; } }
    std::string GetStationName() const { return _stationName; }
    void SetCommPort(const std::string& commPort) { if (commPort != _commPort) { _commPort = commPort; _changeCount++; } }
    std::string GetCommPort() const { return _commPort; }
    void SetText(const std::string& text) { if (text != _text) { _text = text; _changeCount++; } }
    std::string GetText() const { return _text; }
    virtual std::string GetTitle() const override;
    void Do(std::string text, std::string stationName, std::string commPort);
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
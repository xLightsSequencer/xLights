#ifndef PLAYLISTITEMSERIALSEND_H
#define PLAYLISTITEMSERIALSEND_H
 
#include "PlayListItem.h"
#include <string>
 
class wxXmlNode;
class wxWindow;
 
class PlayListItemSerialSend : public PlayListItem
{
protected:
 
    #pragma region Member Variables
    std::string _commport;
    std::string _data;
    bool _started;
	int _stopBits;
	int _speed;
	int _bits;
	char _parity;
    #pragma endregion Member Variables
 
public:
 
    #pragma region Constructors and Destructors
    PlayListItemSerialSend(wxXmlNode* node);
    PlayListItemSerialSend();
    virtual ~PlayListItemSerialSend() {};
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors
 
    #pragma region Getters and Setters
    static std::string GetTooltip();
    std::string GetNameNoTime() const override;
    std::string GetRawName() const { return _name; }
    void SetData(const std::string& data) { if (_data != data) { _data = data; _changeCount++; } }
    std::string GetData() const { return _data; }
    int GetConfig(int& bits, int &stopBits, char& parity) const { bits = _bits; stopBits = _stopBits; parity = _parity; return _speed; }
    void SetConfig(int speed, int bits, int stopBits, char parity) {
        if (_speed != speed) { _speed = speed; _changeCount++; }
        if (_bits != bits) { _bits = bits; _changeCount++; }
        if (_stopBits != stopBits) { _stopBits = stopBits; _changeCount++; }
        if (_parity != parity) { _parity = parity; _changeCount++; }
    }
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
               
	static std::string Encode(const std::string data);
	static std::string Decode(const std::string data);
};
#endif
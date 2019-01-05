#ifndef PLAYLISTITEMFPPEVENT_H
#define PLAYLISTITEMFPPEVENT_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;

class PlayListItemFPPEvent : public PlayListItem
{
protected:

    #pragma region Member Variables
    int _major;
    int _minor;
    std::string _ip;
    bool _started;
    #pragma endregion Member Variables

    std::string GetEventString() const;

public:

    #pragma region Constructors and Destructors
    PlayListItemFPPEvent(wxXmlNode* node);
    PlayListItemFPPEvent();
    virtual ~PlayListItemFPPEvent() {};
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    static std::string GetTooltip();
    std::string GetNameNoTime() const override;
    void SetMajor(const int major) { if (_major != major) { _major = major; _changeCount++; } }
    void SetMinor(const int minor) { if (_minor != minor) { _minor = minor; _changeCount++; } }
    void SetIP(const std::string ip) { if (_ip != ip) { _ip = ip; _changeCount++; } }
    int GetMajor() const { return _major; }
    int GetMinor() const { return _minor; }
    std::string GetIP() const { return _ip; }
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
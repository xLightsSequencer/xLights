#ifndef PLAYLISTITEMCURL_H
#define PLAYLISTITEMCURL_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;

class PlayListItemCURL : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _url;
    std::string _curltype;
    std::string _body;
    bool _started;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemCURL(wxXmlNode* node);
    PlayListItemCURL();
    virtual ~PlayListItemCURL() {};
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    static std::string GetTooltip();
    std::string GetNameNoTime() const override;
    void SetURL(const std::string& url) { if (_url != url) { _url = url; _changeCount++; } }
    std::string GetURL() const { return _url; }
    void SetCURLType(const std::string& type) { if (_curltype != type) { _curltype = type; _changeCount++; } }
    std::string GetCURLType() const { return _curltype; }
    void SetBody(const std::string& body) { if (_body != body) { _body = body; _changeCount++; } }
    std::string GetBody() const { return _body; }
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
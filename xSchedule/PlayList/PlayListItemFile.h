#ifndef PLAYLISTITEMFILE_H
#define PLAYLISTITEMFILE_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;

class PlayListItemFile : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _content;
    std::string _fileName;
    bool _started;
    #pragma endregion Member Variables

    std::string FileReplaceTags(const std::string s);

public:

    #pragma region Constructors and Destructors
    PlayListItemFile(wxXmlNode* node);
    PlayListItemFile();
    virtual ~PlayListItemFile() {};
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    static std::string GetTooltip();
    std::string GetNameNoTime() const override;
    void SetContent(const std::string& content) { if (_content != content) { _content = content; _changeCount++; } }
    void SetFileName(const std::string& fileName) { if (_fileName != fileName) { _fileName = fileName; _changeCount++; } }
    std::string GetContent() const { return _content; }
    std::string GetFileName() const { return _fileName; }
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
#ifndef PLAYLISTITEMESEQ_H
#define PLAYLISTITEMESEQ_H

#include "PlayListItem.h"
#include "../ESEQFile.h"
#include <string>

class wxXmlNode;
class wxWindow;

class PlayListItemESEQ : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _ESEQFileName;
    ESEQFile* _ESEQFile;
    APPLYMETHOD _applyMethod;
    #pragma endregion Member Variables

    void LoadFiles();
    void CloseFiles();

public:

    #pragma region Constructors and Destructors
    PlayListItemESEQ(wxXmlNode* node);
    PlayListItemESEQ();
    virtual ~PlayListItemESEQ() { CloseFiles(); };
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetNameNoTime() const override;
    std::string GetESEQFileName() const { return _ESEQFileName; }
    void SetESEQFileName(const std::string& ESEQFileName);
    int GetBlendMode() const { return _applyMethod; }
    void SetBlendMode(const std::string& blendMode) { if (_applyMethod != EncodeBlendMode(blendMode)) { _applyMethod = EncodeBlendMode(blendMode); _changeCount++; } }
    virtual size_t GetDurationMS(size_t frameMS) const override;
    virtual std::string GetTitle() const override;
    virtual std::list<std::string> GetMissingFiles() override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Start(long stepLengthMS) override;
    virtual void Stop() override;
    #pragma endregion Playing

    #pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif
#pragma once

#include <list>
#include <string>

#include "Controller.h"

class wxXmlNode;
class Output;

// An serial controller sends data to a unique com port
class ControllerNull : public Controller
{
protected:

#pragma region Member Variables
#pragma endregion Member Variables

public:

#pragma region Constructors and Destructors
    ControllerNull(OutputManager* om, wxXmlNode* node, const std::string& showDir);
    ControllerNull(OutputManager* om);
    virtual ~ControllerNull() {}
    void Convert(wxXmlNode* node, std::string showDir); // loads a legacy networks node
#pragma endregion Constructors and Destructors

#pragma region Static Functions
#pragma endregion Static Functions

#pragma region Getters and Setters
    virtual bool NeedsControllerConfig() const override { return false; }
    virtual bool IsLookedUpByControllerName() const override { return false; }
    virtual bool IsAutoLayoutModels() const override { return false; }
    virtual std::string GetType() const override { return CONTROLLER_NULL; }
    virtual std::string GetChannelMapping(int32_t ch) const override;
    virtual std::string GetColumn1Label() const override { return "NULL"; }
    virtual bool SupportsSuppressDuplicateFrames() const override { return false; }
    virtual bool SupportsAutoStartChannels() const override { return true; }
    virtual bool SupportsAutoSize() const override { return true; }
    virtual std::string GetUniverseString() const override { return wxString::Format("%d", _id); }
    virtual std::string GetLongDescription() const override;
    virtual void SetId(int id) override;
#pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;

#pragma region UI
#ifndef EXCLUDENETWORKUI
    virtual void AddProperties(wxPropertyGrid* propertyGrid) override;
    virtual bool HandlePropertyEvent(wxPropertyGridEvent & event, OutputModelManager * outputModelManager) override;
#endif
#pragma endregion UI
};
#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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
#pragma endregion 

public:

#pragma region Constructors and Destructors
    ControllerNull(OutputManager* om, wxXmlNode* node, const std::string& showDir);
    ControllerNull(OutputManager* om);
    ControllerNull(OutputManager* om, const ControllerNull& from);
    virtual ~ControllerNull()
    {}
    virtual wxXmlNode* Save() override;
    virtual bool UpdateFrom(Controller* from) override;
    virtual Controller* Copy(OutputManager* om) override;
#pragma endregion Constructors and Destructors

#pragma region Virtual Functions
    virtual void SetId(int id) override;

    virtual bool SupportsSuppressDuplicateFrames() const override { return false; }
    virtual bool IsManaged() const override { return true; }
    virtual bool CanSendData() const override { return false; }

    virtual std::string GetLongDescription() const override;

    virtual std::string GetShortDescription() const override;

    virtual std::string GetType() const override { return CONTROLLER_NULL; }

    virtual void Convert(wxXmlNode* node, std::string showDir) override; // loads a legacy networks node

    virtual bool NeedsControllerConfig() const override { return false; }

    virtual bool IsLookedUpByControllerName() const override { return false; }

    virtual bool SupportsAutoLayout() const override { return true; }

    virtual bool SupportsAutoSize() const override { return IsAutoLayout(); }

    virtual std::string GetChannelMapping(int32_t ch) const override;
    virtual std::string GetUniverseString() const override { return wxString::Format("%d", _id); }

    virtual std::string GetColumn1Label() const override { return "NULL"; }

    virtual bool CanVisualise() const override { return false; }

    virtual std::string GetExport() const override;
    #pragma endregion 

    #pragma region UI
    #ifndef EXCLUDENETWORKUI
        virtual void AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
        virtual bool HandlePropertyEvent(wxPropertyGridEvent & event, OutputModelManager * outputModelManager) override;
    #endif
    #pragma endregion
};

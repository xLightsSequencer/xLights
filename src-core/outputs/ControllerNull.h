#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <list>
#include <string>

#include "Controller.h"

class Output;

// An serial controller sends data to a unique com port
class ControllerNull : public Controller
{
protected:

#pragma region Member Variables
#pragma endregion 

public:

#pragma region Constructors and Destructors
    ControllerNull(OutputManager* om, pugi::xml_node node, const std::string& showDir);
    ControllerNull(OutputManager* om);
    ControllerNull(OutputManager* om, const ControllerNull& from);
    virtual ~ControllerNull()
    {}
    virtual pugi::xml_node Save(pugi::xml_node parent) override;
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

    virtual void Convert(pugi::xml_node node, std::string showDir) override; // loads a legacy networks node

    virtual bool NeedsControllerConfig() const override { return false; }

    virtual bool IsLookedUpByControllerName() const override { return false; }

    virtual bool SupportsAutoLayout() const override { return true; }

    virtual bool SupportsAutoSize() const override { return IsAutoLayout(); }

    virtual std::string GetChannelMapping(int32_t ch) const override;
    virtual std::string GetUniverseString() const override { return std::to_string(_id); }

    virtual std::string GetColumn1Label() const override { return "NULL"; }

    virtual bool CanVisualise() const override { return false; }

    virtual std::string GetExport() const override;
    #pragma endregion 

    // UI property grid methods moved to ui/controllerproperties/ControllerNullPropertyAdapter
};

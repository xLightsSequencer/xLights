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
#include <memory>
#include <string>

class wxPropertyGrid;
class wxPropertyGridEvent;
class wxPGProperty;
class wxPGChoices;
class Controller;
class ModelManager;
class OutputManager;
class OutputModelManager;

class ControllerPropertyAdapter {
public:
    explicit ControllerPropertyAdapter(Controller& controller);
    virtual ~ControllerPropertyAdapter() = default;

    // Full property grid population
    virtual void AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties);
    virtual void UpdateProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties, OutputModelManager* outputModelManager);
    virtual bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager);
    virtual void ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const;
    virtual void HandleExpanded(wxPropertyGridEvent& event, bool expanded);

    // Helpers
    static int EncodeChoices(const wxPGChoices& choices, const std::string& choice);
    static std::string DecodeChoices(const wxPGChoices& choices, int choice);

protected:
    void AddModels(wxPGProperty* property, wxPGProperty* vp);
    void AddVariants(wxPGProperty* property);

    Controller& _controller;
};

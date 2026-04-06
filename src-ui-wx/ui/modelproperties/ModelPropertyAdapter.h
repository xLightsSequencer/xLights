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

#include <memory>

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class wxPGProperty;
class wxMenu;
class wxCommandEvent;
class Model;
class OutputManager;

class ModelPropertyAdapter {
public:
    explicit ModelPropertyAdapter(Model& model);
    virtual ~ModelPropertyAdapter() = default;

    // Full property grid population (replaces Model::AddProperties)
    virtual void AddProperties(wxPropertyGridInterface* grid, OutputManager* outputManager);
    virtual void UpdateProperties(wxPropertyGridInterface* grid, OutputManager* outputManager);

    // Type-specific properties (subclasses override for model-specific properties)
    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager);
    virtual void UpdateTypeProperties(wxPropertyGridInterface* grid);

    // Controller section
    void AddControllerProperties(wxPropertyGridInterface* grid);
    void UpdateControllerProperties(wxPropertyGridInterface* grid);

    // Size/location and dimensions
    void AddSizeLocationProperties(wxPropertyGridInterface* grid);
    virtual void AddDimensionProperties(wxPropertyGridInterface* grid);

    // Event handling
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event);
    virtual void OnPropertyGridChanging(wxPropertyGridInterface* grid, wxPropertyGridEvent& event);
    virtual int OnPropertyGridSelection(wxPropertyGridInterface* grid, wxPropertyGridEvent& event);
    virtual void OnPropertyGridItemCollapsed(wxPropertyGridInterface* grid, wxPropertyGridEvent& event);
    virtual void OnPropertyGridItemExpanded(wxPropertyGridInterface* grid, wxPropertyGridEvent& event);
    virtual void HandlePropertyGridRightClick(wxPropertyGridEvent& event, wxMenu& mnu);
    virtual void HandlePropertyGridContextMenu(wxCommandEvent& event);

    // Layer size property helpers
    void AddLayerSizeProperty(wxPropertyGridInterface* grid);
    bool HandleLayerSizePropertyChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event);

    // Helpers
    void AdjustStringProperties(wxPropertyGridInterface* grid, int newNum);
    void ColourClashingChains(wxPGProperty* p);
    virtual void DisableUnusedProperties(wxPropertyGridInterface* grid);

protected:
    Model& _model;
};

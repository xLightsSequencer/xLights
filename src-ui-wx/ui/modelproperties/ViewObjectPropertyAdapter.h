/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class ViewObject;
class OutputManager;

class ViewObjectPropertyAdapter {
public:
    explicit ViewObjectPropertyAdapter(ViewObject& obj);
    virtual ~ViewObjectPropertyAdapter() = default;

    void AddProperties(wxPropertyGridInterface* grid, OutputManager* outputManager);
    void UpdateProperties(wxPropertyGridInterface* grid, OutputManager* outputManager);

    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager);
    virtual void UpdateTypeProperties(wxPropertyGridInterface* grid);

    void AddSizeLocationProperties(wxPropertyGridInterface* grid);
    void AddDimensionProperties(wxPropertyGridInterface* grid);

    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event);

protected:
    ViewObject& _viewObject;
};

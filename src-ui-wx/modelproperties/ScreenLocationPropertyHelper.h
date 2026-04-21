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

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class ModelScreenLocation;
class BoxedScreenLocation;
class TwoPointScreenLocation;
class ThreePointScreenLocation;
class PolyPointScreenLocation;

// Helper that handles property grid UI for screen location classes.
// Keeps wxPropertyGrid code out of the screen location .cpp files.
class ScreenLocationPropertyHelper {
public:
    // Dispatch methods — accept base ModelScreenLocation& and dynamic_cast internally
    static void AddSizeLocationProperties(const ModelScreenLocation& loc, wxPropertyGridInterface* grid);
    static void AddDimensionProperties(const ModelScreenLocation& loc, wxPropertyGridInterface* grid, float factor = 1.0);
    static int OnPropertyGridChange(ModelScreenLocation& loc, wxPropertyGridInterface* grid, wxPropertyGridEvent& event);

private:
    // BoxedScreenLocation
    static void AddSizeLocationProperties(const BoxedScreenLocation& loc, wxPropertyGridInterface* grid);
    static void AddDimensionProperties(const BoxedScreenLocation& loc, wxPropertyGridInterface* grid, float factor);
    static int OnPropertyGridChange(BoxedScreenLocation& loc, wxPropertyGridInterface* grid, wxPropertyGridEvent& event);

    // TwoPointScreenLocation
    static void AddSizeLocationProperties(const TwoPointScreenLocation& loc, wxPropertyGridInterface* grid);
    static void AddDimensionProperties(const TwoPointScreenLocation& loc, wxPropertyGridInterface* grid, float factor);
    static int OnPropertyGridChange(TwoPointScreenLocation& loc, wxPropertyGridInterface* grid, wxPropertyGridEvent& event);

    // ThreePointScreenLocation
    static void AddSizeLocationProperties(const ThreePointScreenLocation& loc, wxPropertyGridInterface* grid);
    static void AddDimensionProperties(const ThreePointScreenLocation& loc, wxPropertyGridInterface* grid, float factor);
    static int OnPropertyGridChange(ThreePointScreenLocation& loc, wxPropertyGridInterface* grid, wxPropertyGridEvent& event);

    // PolyPointScreenLocation
    static void AddSizeLocationProperties(const PolyPointScreenLocation& loc, wxPropertyGridInterface* grid);
    static void AddDimensionProperties(const PolyPointScreenLocation& loc, wxPropertyGridInterface* grid, float factor);
    static int OnPropertyGridChange(PolyPointScreenLocation& loc, wxPropertyGridInterface* grid, wxPropertyGridEvent& event);
};

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

class ModelPropertyAdapter;
class Model;

class ModelPropertyManager {
public:
    // Creates the appropriate property adapter for a given model.
    // Returns a type-specific adapter if one is registered, otherwise
    // returns a default adapter that delegates to the model's own methods.
    static std::unique_ptr<ModelPropertyAdapter> CreateAdapter(Model& model);
};

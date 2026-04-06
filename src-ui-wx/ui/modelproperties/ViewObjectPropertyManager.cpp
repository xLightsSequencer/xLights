/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ViewObjectPropertyManager.h"
#include "ViewObjectPropertyAdapter.h"
#include "adapters/MeshObjectPropertyAdapter.h"
#include "adapters/ImageObjectPropertyAdapter.h"
#include "adapters/GridlinesObjectPropertyAdapter.h"
#include "adapters/TerrainObjectPropertyAdapter.h"
#include "adapters/RulerObjectPropertyAdapter.h"
#include "models/MeshObject.h"
#include "models/ImageObject.h"
#include "models/GridlinesObject.h"
#include "models/TerrainObject.h"
#include "models/RulerObject.h"

std::unique_ptr<ViewObjectPropertyAdapter> ViewObjectPropertyManager::CreateAdapter(ViewObject& obj) {
    if (auto* mesh = dynamic_cast<MeshObject*>(&obj)) {
        return std::make_unique<MeshObjectPropertyAdapter>(*mesh);
    } else if (auto* image = dynamic_cast<ImageObject*>(&obj)) {
        return std::make_unique<ImageObjectPropertyAdapter>(*image);
    } else if (auto* gridlines = dynamic_cast<GridlinesObject*>(&obj)) {
        return std::make_unique<GridlinesObjectPropertyAdapter>(*gridlines);
    } else if (auto* terrain = dynamic_cast<TerrainObject*>(&obj)) {
        return std::make_unique<TerrainObjectPropertyAdapter>(*terrain);
    } else if (auto* ruler = dynamic_cast<RulerObject*>(&obj)) {
        return std::make_unique<RulerObjectPropertyAdapter>(*ruler);
    }
    return std::make_unique<ViewObjectPropertyAdapter>(obj);
}

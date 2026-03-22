/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/


#include "ViewObject.h"
#include "Model.h"

ViewObject::ViewObject(const ObjectManager &manager)
{
}

ViewObject::~ViewObject()
{
}


void ViewObject::Setup() {
    layout_group = "Default"; // objects in 3d can only belong to default as only default is 3d

    GetObjectScreenLocation().Init();

    InitModel();

    IncrementChangeCount();
}


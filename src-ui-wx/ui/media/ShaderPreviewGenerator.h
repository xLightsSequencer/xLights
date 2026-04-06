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

class ShaderMediaCacheEntry;
class xLightsFrame;

// UI-layer function that renders a shader preview using the xLights render engine.
// Populates the entry's preview frame cache via SetPreviewFrames().
void GenerateShaderPreview(ShaderMediaCacheEntry* entry, xLightsFrame* xl);

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

// Internal (ObjC++) class extension handing XLLightTest the core managers,
// so the C++ types stay out of the Swift-visible XLLightTest.h.
// Implemented in XLLightTest.mm.

#import "XLLightTest.h"

class OutputManager;
class ModelManager;

@interface XLLightTest ()
- (instancetype)initWithOutputManager:(OutputManager*)outputManager
                         modelManager:(ModelManager*)modelManager;
@end

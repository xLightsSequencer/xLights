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

// Internal (ObjC++) class extension exposing XLAIServiceProperty's
// core-converting initializer to other bridge .mm files (e.g.
// XLAIImageSession) without leaking the C++ ServiceProperty type into the
// Swift-visible XLAIServices.h. Implemented in XLAIServices.mm.

#import "XLAIServices.h"

#include "ai/ServiceProperty.h"

@interface XLAIServiceProperty ()
- (instancetype)initFromCore:(const ServiceProperty&)p;
@end

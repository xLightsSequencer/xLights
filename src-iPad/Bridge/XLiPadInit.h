#pragma once

#import <Foundation/Foundation.h>

// Initialize xLights core systems for iPad.
// Must be called once before any other xLights API usage.
@interface XLiPadInit : NSObject
+ (void)initialize;
@end

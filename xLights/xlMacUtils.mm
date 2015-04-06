//
//  xlMacUtils.m
//  xLights
//
//

#import <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>

double xlOSXGetMainScreenContentScaleFactor()
{
    
    double displayScale = 1.0;
    if ( [ [NSScreen mainScreen] respondsToSelector:@selector(backingScaleFactor)] ) {
        NSArray *screens = [NSScreen screens];
        for (int i = 0; i < [screens count]; i++) {
            float s = [[screens objectAtIndex:i] backingScaleFactor];
            if (s > displayScale)
                displayScale = s;
        }
    }
    return displayScale;
}
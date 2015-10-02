//
//  xlMacUtils.m
//  xLights
//
//

#import <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>
#import <AppKit/NSOpenGL.h>
#import <AppKit/NSOpenGLView.h>
#include "wx/glcanvas.h"

#include "osxMacUtils.h"

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



void xlSetOpenGLRetina(wxGLCanvas &win) {
    NSOpenGLView *glView = (NSOpenGLView*)win.GetHandle();
    [glView setWantsBestResolutionOpenGLSurface:YES];
}

void xlSetRetinaCanvasViewport(wxGLCanvas &win, int &x, int &y, int &x2, int&y2) {
    NSOpenGLView *glView = (NSOpenGLView*)win.GetHandle();
    
    NSPoint pt;
    pt.x = x;
    pt.y = y;
    NSPoint pt2 = [glView convertPointToBacking: pt];
    x = pt2.x;
    y = -pt2.y;
    
    pt.x = x2;
    pt.y = y2;
    pt2 = [glView convertPointToBacking: pt];
    x2 = pt2.x;
    y2 = -pt2.y;
}

double xlTranslateToRetina(wxGLCanvas &win, double x) {
    NSOpenGLView *glView = (NSOpenGLView*)win.GetHandle();
    NSSize pt;
    pt.width = x;
    pt.height = 0;
    NSSize pt2 = [glView convertSizeToBacking: pt];
    return pt2.width;
}



class AppNapSuspenderPrivate
{
public:
    id<NSObject> activityId;
};

AppNapSuspender::AppNapSuspender() :
    p(new AppNapSuspenderPrivate)
{}
AppNapSuspender::~AppNapSuspender()
{
    delete p;
}

void AppNapSuspender::suspend()
{
    if ([[NSProcessInfo processInfo] respondsToSelector:@selector(beginActivityWithOptions:reason:)]) {
        p->activityId = [[NSProcessInfo processInfo ] beginActivityWithOptions: NSActivityUserInitiated | NSActivityLatencyCritical
                                                                        reason:@"Outputting to lights"];
        [p->activityId retain];
    }
}

void AppNapSuspender::resume()
{
    if ([[NSProcessInfo processInfo] respondsToSelector:@selector(beginActivityWithOptions:reason:)]) {
        [[NSProcessInfo processInfo ] endActivity:p->activityId];
        [p->activityId release];
    }
}


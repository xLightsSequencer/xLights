//
//  xlMacUtils.m
//  xLights
//
//

#import <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>
#import <AppKit/NSOpenGL.h>
#import <AppKit/NSOpenGLView.h>

#include "xlGLCanvas.h"
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



void xlSetOpenGLRetina(xlGLCanvas &win) {
    NSOpenGLView *glView = (NSOpenGLView*)win.GetHandle();
    [glView setWantsBestResolutionOpenGLSurface:YES];
}

void xlSetRetinaCanvasViewport(xlGLCanvas &win, int &x, int &y, int &x2, int&y2) {
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

double xlTranslateToRetina(xlGLCanvas &win, double x) {
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

wxString GetOSXFormattedClipboardData() {
    
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSArray *classArray = [NSArray arrayWithObject:[NSString class]];
    NSDictionary *options = [NSDictionary dictionary];
    
    BOOL ok = [pasteboard canReadObjectForClasses:classArray options:options];
    if (ok) {
        NSArray *objectsToPaste = [pasteboard readObjectsForClasses:classArray options:options];
        NSString *dts = [objectsToPaste objectAtIndex:0];
        return wxString([dts UTF8String], wxConvUTF8);
    }
    return "";
}


//
//  xlMacUtils.m
//  xLights
//
//

#import <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>
#import <AppKit/NSOpenGL.h>
#import <AppKit/NSOpenGLView.h>

#include <wx/config.h>

#include "xlGLCanvas.h"
#include "osxMacUtils.h"

#include <list>


void ObtainAccessToURL(const std::string &path) {
    if ("" == path) {
        return;
    }
    std::string pathurl = path;
    wxConfig *config = new wxConfig("xLights-Bookmarks");
    wxString data = config->Read(pathurl);
    NSError *error;
    if ("" == data) {
        NSString *filePath = [NSString stringWithCString:pathurl.c_str()
                                                encoding:[NSString defaultCStringEncoding]];
        NSURL *fileURL = [NSURL fileURLWithPath:filePath];
        
        NSData * newData = [fileURL bookmarkDataWithOptions: NSURLBookmarkCreationWithSecurityScope
                             includingResourceValuesForKeys: nil
                                              relativeToURL: nil
                                                      error: &error];
        NSString *base64 = [newData base64Encoding];
        const char *cstr = [base64 UTF8String];
        if (cstr != nullptr && *cstr) {
            data = cstr;
            config->Write(pathurl, data);
        }
    }
    
    if (data.length() > 0) {
        NSString* dstr = [NSString stringWithCString:data.ToStdString().c_str()
                                            encoding:[NSString defaultCStringEncoding]];
        NSData *nsdata = [[NSData alloc] initWithBase64Encoding:dstr];
        BOOL isStale = false;
    //options:(NSURLBookmarkResolutionOptions)options
    //relativeToURL:(NSURL *)relativeURL
        NSURL *fileURL = [NSURL URLByResolvingBookmarkData:nsdata
                                             options:NSURLBookmarkResolutionWithoutUI | NSURLBookmarkResolutionWithSecurityScope
                                             relativeToURL:nil
                                             bookmarkDataIsStale:&isStale
                                             error:&error];
        [fileURL startAccessingSecurityScopedResource];
    }
    delete config;
}

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

class AppNapSuspender {
public:
    AppNapSuspender();
    ~AppNapSuspender();
    
    void suspend();
    void resume();
private:
    AppNapSuspenderPrivate *p;
};


static AppNapSuspender sleepData;
void EnableSleepModes()
{
    sleepData.resume();
}
void DisableSleepModes()
{
    sleepData.suspend();
}


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


void AddWindowsMenu() {
    NSApplication *app = [NSApplication sharedApplication];
    NSMenu * mainMenu = app.mainMenu;
    
    NSMenu* windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    
    // MINIMIZE
    NSMenuItem* minimizeItem = [[NSMenuItem alloc] initWithTitle:@"Minimize"
                                                          action:@selector(performMiniaturize:)
                                                   keyEquivalent:@"m"];
    [windowMenu addItem:minimizeItem];
    [minimizeItem release];
    
    // ZOOM
    [windowMenu addItemWithTitle:@"Zoom"
                          action:@selector(performZoom:)
                   keyEquivalent:@""];
    
    // SEPARATOR
    [windowMenu addItem:[NSMenuItem separatorItem]];
    
    // BRING ALL TO FRONT
    [windowMenu addItemWithTitle:@"Bring All to Front"
                          action:@selector(arrangeInFront:)
                   keyEquivalent:@""];

    NSMenuItem* windowItem = [mainMenu insertItemWithTitle:@"" action:nil keyEquivalent:@"" atIndex: ([mainMenu numberOfItems] - 1)];
    [windowItem setSubmenu:windowMenu];
    [NSApp setWindowsMenu:windowMenu];
}


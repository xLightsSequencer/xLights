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
#include <wx/menu.h>

#include "xlGLCanvas.h"
#include "osxMacUtils.h"


#include <list>


#include <CoreAudio/CoreAudio.h>
#include <CoreServices/CoreServices.h>

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
        NSString *base64 = [newData base64EncodedStringWithOptions:0];
        const char *cstr = [base64 UTF8String];
        if (cstr != nullptr && *cstr) {
            data = cstr;
            config->Write(pathurl, data);
        }
    }
    
    if (data.length() > 0) {
        NSString* dstr = [NSString stringWithCString:data.c_str()
                                            encoding:[NSString defaultCStringEncoding]];
        NSData *nsdata = [[NSData alloc] initWithBase64EncodedString:dstr options:0];
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
    y = pt2.y;
    
    pt.x = x2;
    pt.y = y2;
    pt2 = [glView convertPointToBacking: pt];
    x2 = pt2.x;
    y2 = pt2.y;
}

double xlTranslateToRetina(xlGLCanvas &win, double x) {
    NSOpenGLView *glView = (NSOpenGLView*)win.GetHandle();
    NSSize pt;
    pt.width = x;
    pt.height = 0;
    NSSize pt2 = [glView convertSizeToBacking: pt];
    return pt2.width;
}

bool IsMouseEventFromTouchpad() {
    NSEvent *theEvent = (NSEvent*)wxTheApp->MacGetCurrentEvent();
    
    return (([theEvent momentumPhase] != NSEventPhaseNone) || ([theEvent phase] != NSEventPhaseNone));
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
#include "wx/osx/private.h"
void ModalPopup(wxWindow *w, wxMenu &menu) {
//    menu.GetHMenu();
    NSView *view = w->GetPeer()->GetWXWidget();
    wxPoint mouse = wxGetMousePosition();
    int x = mouse.x;
    int y = mouse.y;
    w->ScreenToClient( &x , &y ) ;
    
    if ([ view isFlipped ]) {
        y = w->GetSize().y - y;
    }
    
    NSPoint locationInWindow = NSMakePoint(x,y);

    //locationInWindow = [view.window convertBaseToScreen:locationInWindow];
    
    //[menu.GetHMenu() setAutoenablesItems:NO];


    
    NSEvent *event = [NSEvent mouseEventWithType:NSRightMouseDown
                                        location:locationInWindow
                                        modifierFlags:0
                                        timestamp:0
                                        windowNumber:view.window.windowNumber
                                        context:nil eventNumber:0 clickCount:1 pressure:1.0 ];
    [NSMenu popUpContextMenu:menu.GetHMenu() withEvent:event forView:view];
    
    //[menu.GetHMenu() popUpMenuPositioningItem:nil atLocation:NSMakePoint(x, y) inView:view];
    //w->PopupMenu(&menu);
}

#ifndef __NO_AUIDO__
#include "AudioManager.h"

static const AudioObjectPropertyAddress devlist_address = {
    kAudioHardwarePropertyDevices,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster
};

/* this is called when the system's list of available audio devices changes. */
static OSStatus
device_list_changed(AudioObjectID systemObj, UInt32 num_addr, const AudioObjectPropertyAddress *addrs, void *data)
{
    AudioManager *am = (AudioManager*)data;
    am->AudioDeviceChanged();
    return 0;
}

void AddAudioDeviceChangeListener(AudioManager *am) {
    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &devlist_address, device_list_changed, am);
}
void RemoveAudioDeviceChangeListener(AudioManager *am) {
    AudioObjectRemovePropertyListener(kAudioObjectSystemObject, &devlist_address, device_list_changed, am);
}
#endif

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
#include <wx/colour.h>

#include "xlGLCanvas.h"
#include "osxMacUtils.h"


#include <list>
#include <set>
#include <mutex>


#include <CoreAudio/CoreAudio.h>
#include <CoreServices/CoreServices.h>

static std::set<std::string> ACCESSIBLE_URLS;
static std::mutex URL_LOCK;


static void LoadGroupEntries(wxConfig *config, const wxString &grp, std::list<std::string> &removes, std::list<std::string> &grpRemoves) {
    wxString ent;
    long index = 0;
    bool cont = config->GetFirstEntry(ent, index);
    bool hasItem = false;
    while (cont) {
        hasItem = true;
        wxString f = grp + ent;
        if (wxFileExists(f) || wxDirExists(f)) {
            wxString data = config->Read(ent);
            NSString* dstr = [NSString stringWithCString:data.c_str()
                                                    encoding:[NSString defaultCStringEncoding]];
            NSData *nsdata = [[NSData alloc] initWithBase64EncodedString:dstr options:0];
            BOOL isStale = false;
            //options:(NSURLBookmarkResolutionOptions)options
            //relativeToURL:(NSURL *)relativeURL
            NSError *error;
            NSURL *fileURL = [NSURL URLByResolvingBookmarkData:nsdata
                                                     options:NSURLBookmarkResolutionWithoutUI | NSURLBookmarkResolutionWithSecurityScope
                                                     relativeToURL:nil
                                                     bookmarkDataIsStale:&isStale
                                                     error:&error];
            bool ok = [fileURL startAccessingSecurityScopedResource];
            [nsdata release];
            if (ok) {
                ACCESSIBLE_URLS.insert(f);
            } else {
                removes.push_back(f);
            }
        } else {
            removes.push_back(f);
        }
        cont = config->GetNextEntry(ent, index);
    }
    index = 0;
    ent = "";
    cont = config->GetFirstGroup(ent, index);
    while (cont) {
        hasItem = true;
        wxString p = config->GetPath();
        config->SetPath(ent + "/");
        LoadGroupEntries(config, p + "/" + ent + "/", removes, grpRemoves);
        config->SetPath(p);
        cont = config->GetNextGroup(ent, index);
    }
    if (!hasItem) {
        grpRemoves.push_back(grp);
    }
}


bool ObtainAccessToURL(const std::string &path) {
    if ("" == path) {
        return true;
    }
    
    std::unique_lock<std::mutex> lock(URL_LOCK);
    if (ACCESSIBLE_URLS.empty()) {
        std::list<std::string> removes;
        std::list<std::string> grpRemoves;
        wxConfig *config = new wxConfig("xLights-Bookmarks");
        LoadGroupEntries(config, "/", removes, grpRemoves);
        if (!removes.empty() || !grpRemoves.empty()) {
            for (auto &a : removes) {
                if (a.rfind("/Volumes/", 0) != 0) {
                    // don't remove entries that start with /Volumes as its likely just an SD card
                    // that isn't mounted right now.   It might be there later
                    config->DeleteEntry(a, true);
                }
            }
            for (auto &a : grpRemoves) {
                config->DeleteGroup(a);
            }
            config->Flush();
        }
        delete config;
    }
    if (ACCESSIBLE_URLS.find(path) != ACCESSIBLE_URLS.end()) {
        return true;
    }
    if (!wxFileName::Exists(path)) {
        return false;
    }
    wxFileName fn(path);
    if (!fn.IsDir()) {
        wxFileName parent(fn.GetPath());
        wxString ps = parent.GetPath();
        while (ps != "" && ps != "/" && ACCESSIBLE_URLS.find(ps) == ACCESSIBLE_URLS.end()) {
            parent.RemoveLastDir();
            ps = parent.GetPath();
        }
        
        if (ACCESSIBLE_URLS.find(ps) != ACCESSIBLE_URLS.end()) {
            // file is in a directory we already have access to, don't need to record it
            printf("Using dir %s for %s\n", (const char *)ps.c_str(), (const char *)path.c_str());
            ACCESSIBLE_URLS.insert(path);
            return true;
        }
    }
    
    std::string pathurl = path;
    wxConfig *config = new wxConfig("xLights-Bookmarks");
    wxString data = config->Read(pathurl);
    NSError *error = nil;
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
            ACCESSIBLE_URLS.insert(pathurl);
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
        [nsdata release];
    }
    delete config;
    return data.length() > 0;
}

double xlOSXGetMainScreenContentScaleFactor()
{
    
    double displayScale = 1.0;
    NSArray *screens = [NSScreen screens];
    for (int i = 0; i < [screens count]; i++) {
        float s = [[screens objectAtIndex:i] backingScaleFactor];
        if (s > displayScale)
            displayScale = s;
    }
    return displayScale;
}

#define WX_IS_MACOS_AVAILABLE(major, minor) \
       __builtin_available(macOS major ## . ## minor, *)
class xlOSXEffectiveAppearanceSetter
{
public:
    xlOSXEffectiveAppearanceSetter() {
        if (WX_IS_MACOS_AVAILABLE(10, 14)) {
            formerAppearance = NSAppearance.currentAppearance;
            NSAppearance.currentAppearance = NSApp.effectiveAppearance;
        }
    }
    ~xlOSXEffectiveAppearanceSetter() {
        if (WX_IS_MACOS_AVAILABLE(10, 14)) {
            NSAppearance.currentAppearance = (NSAppearance*) formerAppearance;
        }
    }
private:
    void * formerAppearance;
};

void AdjustColorToDeviceColorspace(const wxColor &c, xlColor &xlc) {
    xlOSXEffectiveAppearanceSetter helper;
    NSColor *nc = c.OSXGetNSColor();
    NSColor *ncrgbd = [nc colorUsingColorSpaceName:NSDeviceRGBColorSpace];

    float r = [ncrgbd redComponent] * 255;
    float g = [ncrgbd greenComponent] * 255;
    float b = [ncrgbd blueComponent] * 255;
    float a = [ncrgbd alphaComponent] * 255;

    xlc.Set(r, g, b, a);
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
    p->activityId = [[NSProcessInfo processInfo ] beginActivityWithOptions: NSActivityUserInitiated | NSActivityLatencyCritical
                                                                    reason:@"Outputting to lights"];
    [p->activityId retain];
}

void AppNapSuspender::resume()
{
    [[NSProcessInfo processInfo ] endActivity:p->activityId];
    [p->activityId release];
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



void WXGLUnsetCurrentContext()
{
    [NSOpenGLContext clearCurrentContext];
}




#ifndef __NO_AUIDO__
#include "AudioManager.h"

static const AudioObjectPropertyAddress devlist_address = {
    kAudioHardwarePropertyDevices,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster
};
static const AudioObjectPropertyAddress defaultdev_address = {
    kAudioHardwarePropertyDefaultOutputDevice,
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
    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &defaultdev_address, device_list_changed, am);
}
void RemoveAudioDeviceChangeListener(AudioManager *am) {
    AudioObjectRemovePropertyListener(kAudioObjectSystemObject, &devlist_address, device_list_changed, am);
    AudioObjectRemovePropertyListener(kAudioObjectSystemObject, &defaultdev_address, device_list_changed, am);
}
#endif






bool IsFromAppStore() {
    NSURL *bundleURL = [[NSBundle mainBundle] bundleURL];
       
    SecStaticCodeRef staticCode = NULL;
    OSStatus status = SecStaticCodeCreateWithPath((__bridge CFURLRef)bundleURL, kSecCSDefaultFlags, &staticCode);
    if (status != errSecSuccess) {
        return false;
    }
    NSString *requirementText = @"anchor apple generic";   // For code signed by Apple
    SecRequirementRef requirement = NULL;
    status = SecRequirementCreateWithString((__bridge CFStringRef)requirementText, kSecCSDefaultFlags, &requirement);
    if (status != errSecSuccess) {
        if (staticCode) {
            CFRelease(staticCode);
        }
        return false;
    }
    
    status = SecStaticCodeCheckValidity(staticCode, kSecCSDefaultFlags, requirement);
    if (status != errSecSuccess) {
        if (staticCode) {
            CFRelease(staticCode);
        }
        if (requirement) {
            CFRelease(requirement);
        }
        return false;
    }
    
    if (staticCode) CFRelease(staticCode);
    if (requirement) CFRelease(requirement);
    
    return true;
}


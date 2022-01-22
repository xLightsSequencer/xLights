
#include "Mouse3DManager.h"


#include <wx/app.h>
#include <stdint.h>
#include <stdio.h>
#include <algorithm>


#ifdef __WXOSX__

#include <dlfcn.h>

#define kConnexionClientModeTakeOver 1
#define kConnexionMaskAll 0x3fff
#define kConnexionMaskAllButtons 0xFFFFFFFF
#define kConnexionCmdHandleButtons 2
#define kConnexionCmdHandleAxis 3
#define kConnexionCmdAppSpecific 10
#define kConnexionMsgDeviceState '3dSR'
#define kConnexionCtlGetDeviceID '3did'
#define kConnexionMsgPrefsChanged '3dPC'

#pragma pack(push, 2)
struct ConnexionDeviceState {
    uint16_t version;
    uint16_t client;
    uint16_t command;
    int16_t param;
    int32_t value;
    uint64_t time;
    uint8_t report[8];
    uint16_t buttons8;
    int16_t axis[6];  // tx, ty, tz, rx, ry, rz
    uint16_t address;
    uint32_t buttons;
};
#pragma pack(pop)

// callback functions:
typedef void (*AddedHandler)(uint32_t);
typedef void (*RemovedHandler)(uint32_t);
typedef void (*MessageHandler)(uint32_t, uint32_t msg_type, void *msg_arg);

// driver functions:
typedef int16_t (*SetConnexionHandlers_ptr)(MessageHandler, AddedHandler, RemovedHandler, bool);
typedef int16_t (*InstallConnexionHandlers_ptr)(MessageHandler, AddedHandler, RemovedHandler);
typedef void (*CleanupConnexionHandlers_ptr)();
typedef uint16_t (*RegisterConnexionClient_ptr)(uint32_t signature,
                                                const char *name,
                                                uint16_t mode,
                                                uint32_t mask);
typedef void (*SetConnexionClientButtonMask_ptr)(uint16_t clientID, uint32_t buttonMask);
typedef void (*SetConnexionClientMask_ptr)(uint16_t clientID, uint32_t buttonMask);
typedef void (*UnregisterConnexionClient_ptr)(uint16_t clientID);
typedef int16_t (*ConnexionClientControl_ptr)(uint16_t clientID,
                                              uint32_t message,
                                              int32_t param,
                                              int32_t *result);
#define DECLARE_FUNC(name) name##_ptr name = nullptr

DECLARE_FUNC(SetConnexionHandlers);
DECLARE_FUNC(InstallConnexionHandlers);
DECLARE_FUNC(CleanupConnexionHandlers);
DECLARE_FUNC(RegisterConnexionClient);
DECLARE_FUNC(SetConnexionClientButtonMask);
DECLARE_FUNC(SetConnexionClientMask);
DECLARE_FUNC(UnregisterConnexionClient);
DECLARE_FUNC(ConnexionClientControl);


static void *load_func(void *module, const char *func_name) {
    void *func = dlsym(module, func_name);
    return func;
}
#define LOAD_FUNC(name) name = (name##_ptr)load_func(module, #name)

static void *module;  // handle to the whole driver
static bool driver_loaded = false;
static uint16_t clientID = 0;


static bool load_driver_functions() {
    if (driver_loaded) {
        return true;
    }

    module = dlopen("/Library/Frameworks/3DconnexionClient.framework/3DconnexionClient", RTLD_LAZY | RTLD_LOCAL);

    if (module) {
        LOAD_FUNC(SetConnexionHandlers);

        if (SetConnexionHandlers != NULL) {
            driver_loaded = true;
        } else {
            LOAD_FUNC(InstallConnexionHandlers);
            driver_loaded = (InstallConnexionHandlers != NULL);
        }

        if (driver_loaded) {
            LOAD_FUNC(CleanupConnexionHandlers);
            LOAD_FUNC(RegisterConnexionClient);
            LOAD_FUNC(SetConnexionClientButtonMask);
            LOAD_FUNC(SetConnexionClientMask);
            LOAD_FUNC(UnregisterConnexionClient);
            LOAD_FUNC(ConnexionClientControl);
        }
    }
    return driver_loaded;
}

static void DeviceRemoved(uint32_t unused) {
}

static void DeviceEvent(uint32_t unused, uint32_t msg_type, void *msg_arg) {
    if (msg_type == kConnexionMsgDeviceState) {
        ConnexionDeviceState *s = (ConnexionDeviceState *)msg_arg;
        if (s->client == clientID) {
            //printf("type: %d    device command %X    Param: %X   Value: %X\n", msg_type, s->command, s->param, s->value);
            switch (s->command) {
                case kConnexionCmdHandleAxis: {
                    //printf("%d  %d  %d\n", s->axis[3], s->axis[4], s->axis[5]);
                    glm::vec3 t((float)s->axis[0], (float)s->axis[1], (float)s->axis[2]);
                    glm::vec3 r((float)s->axis[3], (float)s->axis[4], (float)s->axis[5]);
                    for (int x = 0; x < 3; x++) {
                        //normalize to go from -1 to 1
                        t[x] /= 2500; //max speed is about 2500
                        r[x] /= 2500;
                        t[x] = std::clamp(t[x], -1.0f, 1.0f);
                        r[x] = std::clamp(r[x], -1.0f, 1.0f);
                    }
                    Mouse3DManager::INSTANCE.sendMotionEvents(t, r);
                    break;
                }
                case kConnexionCmdHandleButtons: {
                    Mouse3DManager::INSTANCE.sendButtonEvents(s->buttons);
                    break;
                }
                default:
                    //printf("Unknown command: %X\n", s->command);
                    break;
            }
        } else {
            //printf("Unknown client: %X\n", s->client);
        }
    }/* else {
        uint8_t c1 = msg_type & 0xFF;
        uint8_t c2 = (msg_type >> 8) & 0xFF;
        printf("Unknown state: %C%C\n", c2, c1 );
    } */
}

static uint32_t SUPPORTED_BUTTONS = 0b111000000000000000110001111111;
static void DeviceAdded(uint32_t unused) {
    int32_t result;
    ConnexionClientControl(clientID, kConnexionCtlGetDeviceID, 0, &result);
    int16_t vendorID = result >> 16;
    int16_t productID = result & 0xffff;
    Mouse3DManager::INSTANCE.deviceAdded(vendorID, productID);
    // Don't want ESC, ALT, SHIFT, CTRL, LOCK
    //SetConnexionClientButtonMask(clientID, 0x383FFFFF);
    SetConnexionClientButtonMask(clientID, SUPPORTED_BUTTONS);
}


Mouse3DManager::Mouse3DManager() {
    if (load_driver_functions()) {
        const bool separate_thread = true;
        uint16_t error = SetConnexionHandlers(DeviceEvent, DeviceAdded, DeviceRemoved, separate_thread);
        if (error) {
            return;
        }
        clientID = RegisterConnexionClient('xlts', "\007xLights", kConnexionClientModeTakeOver, kConnexionMaskAll);

        //SetConnexionClientMask(clientID, kConnexionMaskAllButtons);
        SetConnexionClientButtonMask(clientID, kConnexionMaskAllButtons);
        enabled = true;
    }
}
Mouse3DManager::~Mouse3DManager() {
    if (module) {
        dlclose(module);
    }
}

#else
Mouse3DManager::Mouse3DManager() {
    enabled = false;
}
Mouse3DManager::~Mouse3DManager() {
}

#endif

static const char *BUTTON_NAMES[] = {
    "BUTTON_MENU",     "BUTTON_FIT",      "BUTTON_TOP",    "BUTTON_LEFT",
    "BUTTON_RIGHT",    "BUTTON_FRONT",    "BUTTON_BOTTOM",
    "BUTTON_11",
    "BUTTON_ROLL",
    "BUTTON_12",
    "BUTTON_ISO1",   "BUTTON_ISO2",
    "BUTTON_1",        "BUTTON_2",        "BUTTON_3",      "BUTTON_4",
    "BUTTON_5",        "BUTTON_6",        "BUTTON_7",      "BUTTON_8",
    "BUTTON_9",        "BUTTON_10",
    "BUTTON_ESC",    "BUTTON_ALT",
    "BUTTON_SHIFT",    "BUTTON_CTRL",
    "BUTTON_LOCK",
    "BUTTON_V1",
    "BUTTON_V2",
    "BUTTON_V3",
    
    "BUTTON_???",    "BUTTON_???",  "BUTTON_???"
};

wxIMPLEMENT_DYNAMIC_CLASS(Motion3DEvent, wxEvent);
wxDEFINE_EVENT(EVT_MOTION3D, Motion3DEvent);
wxDEFINE_EVENT(EVT_MOTION3D_BUTTONCLICKED, wxCommandEvent);

Motion3DEvent::Motion3DEvent(const glm::vec3 &t, const glm::vec3 &r) : wxEvent(0, EVT_MOTION3D), translations(t), rotations(r) {
}
Motion3DEvent::Motion3DEvent() : wxEvent(0, EVT_MOTION3D) {
}


void Mouse3DManager::enableMotionEvents(wxWindow *w) {
    w->Bind(wxEVT_SET_FOCUS, &Mouse3DManager::focusEvent, this);
}
void Mouse3DManager::focusEvent(wxFocusEvent& event) {
    focusWindow = (wxWindow*)event.GetEventObject();
}

void Mouse3DManager::deviceAdded(int16_t vendorID, int16_t productID) {
    //printf("Device added %X  %X\n", vendorID, productID);
}
void Mouse3DManager::sendButtonEvents(uint32_t buttons) {
    //printf("Button:  %X\n", buttons);
    for (int x = 0; x < 32; x++) {
        if (buttons & 0x1) {
            wxCommandEvent *event = new wxCommandEvent(EVT_MOTION3D_BUTTONCLICKED);
            event->SetInt(x);
            event->SetString(BUTTON_NAMES[x]);
            sendEvent(event);
        }
        buttons >>= 1;
    }
}
void Mouse3DManager::sendMotionEvents(const glm::vec3 &t, const glm::vec3 &r) {
    //printf("Motion: %0.3f  %0.3f  %0.3f\n        %0.3f  %0.3f  %0.3f\n", t.x, t.y, t.z, r.x, r.y, r.z);
    Motion3DEvent *event = new Motion3DEvent(t, r);
    sendEvent(event);
}
void Mouse3DManager::sendEvent(wxEvent *event) {
    if (focusWindow && focusWindow->IsShownOnScreen()) {
        focusWindow->GetEventHandler()->QueueEvent(event);
        return;
    }
    wxWindow *w = nullptr;
    if (lastWindow && lastWindow->HasFocus()) {
        w = lastWindow;
    } else {
        wxPoint p = wxGetMousePosition();
        w = wxFindWindowAtPointer(p);
        if (w == nullptr) {
            w = lastWindow;
        }
    }
    if (w && w->IsShownOnScreen()) {
        w->GetEventHandler()->QueueEvent(event);
        lastWindow = w;
    } else {
        delete event;
    }
}

Mouse3DManager Mouse3DManager::INSTANCE;


#include "Mouse3DManager.h"


#include <wx/app.h>
#include <stdint.h>
#include <stdio.h>


#if defined(__WXOSX__)

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
                    uint32_t buttons = s->buttons;
                    for (int x = 0; x < 32; x++) {
                        if (buttons & 0x1) {
                            Mouse3DManager::INSTANCE.sendButtonEvent(x);
                        }
                        buttons >>= 1;
                    }
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
#endif

#if defined(__USE_HIDAPI__)
#include <algorithm>
#include <map>
#include <limits>
#include <condition_variable>


#pragma comment(lib, "hidapi.lib")


std::string Mouse3DManager::format_device_string(int vid, int pid)
{
    std::string ret;

    switch (vid) {
        case 0x046d: {
            ret = "LOGITECH";
            break;
        }
        case 0x256F: {
            ret = "3DCONNECTION";
            break;
        }
        default: {
            ret = "UNKNOWN";
            break;
        }
    }

    ret += "::";

    switch (pid) {
        case 0xc603: {
            ret += "spacemouse plus XT";
            break;
        }
        case 0xc605: {
            ret += "cadman";
            break;
        }
        case 0xc606: {
            ret += "spacemouse classic";
            break;
        }
        case 0xc621: {
            ret += "spaceball 5000";
            break;
        }
        case 0xc623: {
            ret += "space traveller";
            break;
        }
        case 0xc625: {
            ret += "space pilot";
            break;
        }
        case 0xc626: {
            ret += "space navigator";
            break;
        }
        case 0xc627: {
            ret += "space explorer";
            break;
        }
        case 0xc628: {
            ret += "space navigator for notebooks";
            break;
        }
        case 0xc629: {
            ret += "space pilot pro";
            break;
        }
        case 0xc62b: {
            ret += "space mouse pro";
            break;
        }
        case 0xc62e: {
            ret += "spacemouse wireless (USB cable)";
            break;
        }
        case 0xc62f: {
            ret += "spacemouse wireless receiver";
            break;
        }
        case 0xc631: {
            ret += "spacemouse pro wireless";
            break;
        }
        case 0xc632: {
            ret += "spacemouse pro wireless receiver";
            break;
        }
        case 0xc633: {
            ret += "spacemouse enterprise";
            break;
        }
        case 0xc635: {
            ret += "spacemouse compact";
            break;
        }
        case 0xc636: {
            ret += "spacemouse module";
            break;
        }
        case 0xc640: {
            ret += "nulooq";
            break;
        }
        case 0xc652: {
            ret += "3Dconnexion universal receiver";
            break;
        }
        default: {
            ret += "UNKNOWN";
            break;
        }
    }

    return ret;
}

DeviceData Mouse3DManager::detect_attached_device()
{
    DeviceData ret;

    // Initialize the hidapi library
    int res = hid_init();
    if (res != 0) {
       std::cout << "Unable to initialize hidapi library";
    }
    else {
        // Enumerates devices
        hid_device_info* devices = hid_enumerate(0, 0);
        if (devices == nullptr) {
            std::cout << "detect_attached_device() - no HID device enumerated.";
        } else {
            // Searches for 1st connected 3Dconnexion device
            struct DeviceData {
                unsigned short usage_page{ 0 };
                unsigned short usage{ 0 };

                DeviceData(unsigned short usage_page, unsigned short usage) :
                    usage_page(usage_page), usage(usage)
                {}

                // https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf
                // Usage page 1 - Generic Desktop Controls
                // Usage page 1, usage 8 - Multi-axis Controller
                [[nodiscard]]bool has_valid_usage() const
                {
                    return usage_page == 1 && usage == 8;
                }
            };

            // When using 3Dconnexion universal receiver, multiple devices are detected sharing the same vendor_id and product_id.
            // To choose from them the right one we use: usage_page == 1 and usage == 8
            // When only a single device is detected, as for wired connections, vendor_id and product_id are enough

            // First we count all the valid devices from the enumerated list,

            hid_device_info* current = devices;
            typedef std::tuple<unsigned short, unsigned short, std::string> DeviceIds;
            typedef std::vector<DeviceData> DeviceDataList;
            typedef std::map<DeviceIds, DeviceDataList> DetectedDevices;
            DetectedDevices detected_devices;
            while (current != nullptr) {
                unsigned short vendor_id{ 0 };
                unsigned short product_id{ 0 };
                std::string devicePath;

                for (auto const& vendor : _3DCONNEXION_VENDORS) {
                    if (vendor == current->vendor_id) {
                        vendor_id = current->vendor_id;
                        break;
                    }
                }

                if (vendor_id != 0) {
                    for (auto const& device : _3DCONNEXION_DEVICES) {
                        if (device == current->product_id) {
                            product_id = current->product_id;
                            devicePath = current->path;
                            DeviceIds detected_device(vendor_id, product_id, devicePath);
                            DetectedDevices::iterator it = detected_devices.find(detected_device);
                            if (it == detected_devices.end()) {
                                it = detected_devices.insert(DetectedDevices::value_type(detected_device, DeviceDataList())).first;
                            }
                            it->second.emplace_back(current->usage_page, current->usage);
                        }
                    }
                }

                current = current->next;
            }

            // Free enumerated devices
            hid_free_enumeration(devices);

            unsigned short vendor_id{ 0 };
            unsigned short product_id{ 0 };
            std::string devicePath;

            if (!detected_devices.empty()) {
                // Then we'll decide the choosing logic to apply in dependence of the device count and operating system
                for (const DetectedDevices::value_type& device : detected_devices) {
                    if (device.second.size() == 1) {
                        if (device.second.front().has_valid_usage()) {
                            vendor_id = std::get<0>(device.first);
                            product_id = std::get<1>(device.first);
                            devicePath = std::get<2>(device.first);
                            break;
                        }
                    } else {
                        bool found = false;
                        for (const DeviceData& data : device.second) {
                            if (data.has_valid_usage()) {
                                vendor_id = std::get<0>(device.first);
                                product_id = std::get<1>(device.first);
                                devicePath = std::get<2>(device.first);
                                found = true;
                                break;
                            }
                        }

                        if (found) 
                        {
                            break;
                        }
                    }
                }
            }

            if (vendor_id != 0 && product_id != 0) {
                ret.DeviceStr = format_device_string(static_cast<int>(vendor_id), static_cast<int>(product_id));
                ret.VID = vendor_id;
                ret.PID = product_id;
                ret.path = devicePath;
            }
        }

        // Finalize the hidapi library
        hid_exit();
    }

    return ret;
}

bool Mouse3DManager::handle_input(const DataPacketRaw& packet, const int packet_length)
{
    if (focusWindow == nullptr) {
        return false;
    }
#ifdef __WXMSW__
    if (!focusWindow->IsMouseInWindow()) {
        return false;
    }
#endif

    if (packet_length == 7 || packet_length == 13) {
        handle_packet(packet, packet_length);
    }
     return false;
}
// The following is called by handle_input() from the worker thread.
bool Mouse3DManager::handle_packet(const DataPacketRaw& packet, const int packet_length)
{
    //static int cnt = 0;
    //printf("Packet %d: %d   %d\n", ++cnt, (int)packet[0], packet_length);
    switch (packet[0]) {
        case 1: // Translation + Rotation
        {
            bool updated{false};
            if (packet_length == 13) {
                updated = handle_packet_translation_rotation(packet);
            } else {
                updated = handle_packet_translation(packet);
            }
            if (updated) {
                return true;
            }

            break;
        }
        case 2: // Rotation
        {
            if (handle_packet_rotation(packet, 1)) {
                return true;
            }

            break;
        }
        case 3: // Button
        {
            if (handle_packet_button(packet, packet.size() - 1)) {
                return true;
            }

            break;
        }
        case 28: // enterprise button
        {
            if (handle_packet_button28(packet, packet.size() - 1)) {
                return true;
            }
            break;
        }
        case 29: // enterprise button, long press
        {
            if (handle_packet_button29(packet, packet.size() - 1)) {
                return true;
            }
            break;
        }
        case 23: // Battery charge
        {
            std::cout << "3DConnexion - battery level: " << (int)packet[1] << " percent" << std::endl;
            break;
        }
        default: {
            std::cout << "3DConnexion - Got unknown data packet of code: " << (int)packet[0] << std::endl;
            break;
        }
    }

    return false;
}

static bool isZero(float value)
{
    constexpr auto threshold = std::numeric_limits<float>::epsilon();
    return value >= -threshold && value <= threshold;
}

static bool isZero(glm::vec3 val)
{
    return isZero(val.x) && isZero(val.y) && isZero(val.z);
}

static double convert_input(int coord_byte_low, int coord_byte_high, double deadzone)
{
    int value = coord_byte_low | (coord_byte_high << 8);
    if (value >= 32768) {
        value = value - 65536;
    }
    double ret = (double)value / 350.0;
    return (std::abs(ret) > deadzone) ? ret : 0.0;
}

bool Mouse3DManager::handle_packet_translation_rotation(const DataPacketRaw& packet)
{
    constexpr double deadzone{ 0 };
    glm::vec3 t(convert_input(packet[1], packet[2], deadzone),
                convert_input(packet[3], packet[4], deadzone),
                convert_input(packet[5], packet[6], deadzone));

    glm::vec3 r((float)convert_input(packet[7], packet[8], deadzone),
                (float)convert_input(packet[9], packet[10], deadzone),
                (float)convert_input(packet[11], packet[12], deadzone));
    if (isZero(t) && isZero(r)) {
        return false;
    }
    Mouse3DManager::INSTANCE.sendMotionEvents(t, r);
    return false;
}

bool Mouse3DManager::handle_packet_translation(const DataPacketRaw& packet)
{
    constexpr double deadzone{ 0 };
    glm::vec3 t(convert_input(packet[1], packet[2], deadzone),
                      convert_input(packet[3], packet[4], deadzone),
                      convert_input(packet[5], packet[6], deadzone));
    if (isZero(t)) {
        return false;
    }
    glm::vec3 r(0, 0, 0);
    Mouse3DManager::INSTANCE.sendMotionEvents(t, r);
    return false;
}
bool Mouse3DManager::handle_packet_rotation(const DataPacketRaw& packet, unsigned int first_byte)
{
    constexpr double deadzone{ 0 };
    glm::vec3 r((float)convert_input(packet[first_byte + 0], packet[first_byte + 1], deadzone),
                   (float)convert_input(packet[first_byte + 2], packet[first_byte + 3], deadzone),
                   (float)convert_input(packet[first_byte + 4], packet[first_byte + 5], deadzone));
    if (isZero(r)) {
        return false;
    }
    glm::vec3 t(0, 0, 0);
    Mouse3DManager::INSTANCE.sendMotionEvents(t, r);
    return false;
}
bool Mouse3DManager::handle_packet_button(const DataPacketRaw& packet, unsigned int packet_size)
{
    unsigned int curButton{ 0 };
    for (unsigned int i = 1; i < packet_size; ++i) {
        uint32_t buttons = packet[i];
        for (int x = 0; x < 8; x++) {
            if (buttons & 0x1) {
                Mouse3DManager::INSTANCE.sendButtonEvent(curButton);
            }
            buttons >>= 1;
            curButton++;
        }
    }
    return false;
}

inline uint32_t mapEnterpriseButton(uint32_t i) {
    if (i < 25) {
        return i;
    }
    switch (i) {
        case 0x4D: // BUTTON 11
            return 8;
        case 0x4E: // BUTTON 12
            return 10;
        case 0x19: // SHIFT
            return 25;
        case 0x1A: // CTRL
            return 26;
        case 0x1B: // LOCK
            return 27;
        case 0x67: // V1
            return 28;
        case 0x68: // V2
            return 29;
        case 0x69: // V3
            return 30;
            
        case 0x24: // ENTER
            return 31;
        case 0xAF: // TAB
            return 32;
        case 0x25: // DELETE
            return 33;
        case 0xB0: // SPACE
            return 34;
    }
    /*
    uint32_t n = i;
    for (uint32_t i2 = 1 << 7; i2 > 0; i = i2 / 2) {
          (n & i2) ? printf("1") : printf("0");
    }
    printf("   %X\n", (int)i);
    */
    return 0;
}

bool Mouse3DManager::handle_packet_button28(const DataPacketRaw& packet, unsigned int packet_size)
{
    for (unsigned int i = 1; i < packet_size; ++i) {
        uint32_t val = mapEnterpriseButton(packet[i]);
        if (val) {
            Mouse3DManager::INSTANCE.sendButtonEvent(val - 1);
        }
    }
    return false;
}
bool Mouse3DManager::handle_packet_button29(const DataPacketRaw& packet, unsigned int packet_size)
{
    for (unsigned int i = 1; i < packet_size; ++i) {
        uint32_t val = mapEnterpriseButton(packet[i]);
        if (val) {
            switch (val) {
                case 3:  // TOP -> BOTTOM
                    val += 4;
                    break;
                case 6:  // FRONT -> BACK
                    val = 35;
                    break;
                case 5:  // RIGHT -> LEFT
                    val -= 1;
                    break;
                case 11: // ISO1 -> ISO2
                    val += 1;
                    break;
                default:
                    break;
            }
            Mouse3DManager::INSTANCE.sendButtonEvent(val - 1);
        }
    }
    return false;
}
bool Mouse3DManager::connect_device(DeviceData const& dev)
{ 
    auto *device = hid_open_path(dev.path.c_str());
    if (device == nullptr) {
        device = hid_open(dev.VID, dev.PID, nullptr);
    }
    if (device == nullptr) {
        return false;
    }
    m_device_ptr = device;
    return true;
}
void Mouse3DManager::disconnect_device()
{
    m_device_ptr = nullptr;
}

void Mouse3DManager::run()
{
    int res = hid_init();
    if (res != 0) {
        return;
    }
    this->connect_device(m_device);
    if (m_device_ptr != nullptr) {
        for (;;) {
            if (m_stop) {
                break;
            }
            // Waits for 3DConnexion mouse input for maximum 100ms, then repeats.
            if (!this->collect_input()) {
                break;
            }
            wxMilliSleep(1);
        }
    }

    this->disconnect_device();

    // Finalize the hidapi library
    hid_exit();
}
bool Mouse3DManager::collect_input()
{
    DataPacketRaw packet = { 0 };
    // Read packet, block maximum 100 ms. That means when closing the application, closing the application will be delayed by 100 ms.
    int res = hid_read_timeout(m_device_ptr, packet.data(), packet.size(), 100);
    if (res < 0) {
        // An error occourred (device detached from pc ?). Close the 3Dconnexion device.
        this->disconnect_device();
        return false;
    }
    this->handle_input(packet, res);
    return true;
}
#endif

Mouse3DManager::Mouse3DManager()
{
    enabled = false;
#ifdef __WXOSX__
    if (load_driver_functions()) {
        const bool separate_thread = true;
        uint16_t error = SetConnexionHandlers(DeviceEvent, DeviceAdded, DeviceRemoved, separate_thread);
        if (!error) {
            clientID = RegisterConnexionClient('xlts', "\007xLights", kConnexionClientModeTakeOver, kConnexionMaskAll);

            //SetConnexionClientMask(clientID, kConnexionMaskAllButtons);
            SetConnexionClientButtonMask(clientID, kConnexionMaskAllButtons);
            enabled = true;
        }
    }
#endif
#if defined(__USE_HIDAPI__)
    if (!enabled) {
        m_device = detect_attached_device();
        if (!m_device.isEmpty()) {
            enabled = true;
        }

        assert(!m_thread.joinable());
        if (!m_thread.joinable()) {
            m_stop = false;
            m_thread = std::thread(&Mouse3DManager::run, this);
        }
    }
#endif
}

Mouse3DManager::~Mouse3DManager() {
#if defined(__USE_HIDAPI__)
    m_stop = true;
    if (m_thread.joinable()) {
        m_thread.join();
    }
#endif
#ifdef __WXOSX__
    if (module) {
        dlclose(module);
    }
#endif
}




static std::vector<const char *>BUTTON_NAMES = {
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
    
    "BUTTON_ENTER", "BUTTON_TAB", "BUTTON_DELETE", "BUTTON_SPACE", "BUTTON_BACK"
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
void Mouse3DManager::sendButtonEvent(uint32_t button) {
    std::string name = (button < BUTTON_NAMES.size()) ? BUTTON_NAMES[button] : "BUTTON_???";
    //printf("Button:  %d  %s\n", button, name.c_str());
    wxCommandEvent *event = new wxCommandEvent(EVT_MOTION3D_BUTTONCLICKED);
    event->SetInt(button);
    event->SetString(name);
    sendEvent(event);
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

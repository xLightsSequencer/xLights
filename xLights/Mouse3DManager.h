#pragma once


#include <wx/wx.h>
#include <glm/vec3.hpp>

#if defined(__WXOSX__) || defined(__WXMSW__)
#define __USE_HIDAPI__
#endif


#ifdef __USE_HIDAPI__

#include <thread>
#include <array>

extern "C" {
#include "hidapi/hidapi.h"
}

struct DeviceData 
{
    std::string DeviceStr;
    std::string path;
    unsigned short VID{ 0 };
    unsigned short PID{ 0 };

    void clear()
    {
        DeviceStr = std::string();
        VID=0;
        PID=0;
        path = "";
    }

    bool isEmpty()
    {
        return DeviceStr.empty();
    }
};

static const std::vector<int> _3DCONNEXION_VENDORS = {
    0x046d, // LOGITECH = 1133 // Logitech (3Dconnexion is made by Logitech)
    0x256F  // 3DCONNECTION = 9583 // 3Dconnexion
};

// See: https://github.com/FreeSpacenav/spacenavd/blob/a9eccf34e7cac969ee399f625aef827f4f4aaec6/src/dev.c#L202
static const std::vector<int> _3DCONNEXION_DEVICES = {
    0xc603, /* 50691 spacemouse plus XT */
    0xc605, /* 50693 cadman */
    0xc606, /* 50694 spacemouse classic */
    0xc621, /* 50721 spaceball 5000 */
    0xc623, /* 50723 space traveller */
    0xc625, /* 50725 space pilot */
    0xc626, /* 50726 space navigator *TESTED* */
    0xc627, /* 50727 space explorer */
    0xc628, /* 50728 space navigator for notebooks*/
    0xc629, /* 50729 space pilot pro*/
    0xc62b, /* 50731 space mouse pro*/
    0xc62e, /* 50734 spacemouse wireless (USB cable) *TESTED* */
    0xc62f, /* 50735 spacemouse wireless receiver */
    0xc631, /* 50737 spacemouse pro wireless *TESTED* */
    0xc632, /* 50738 spacemouse pro wireless receiver */
    0xc633, /* 50739 spacemouse enterprise */
    0xc635, /* 50741 spacemouse compact *TESTED* */
    0xc636, /* 50742 spacemouse module */
    0xc640, /* 50752 nulooq */
    0xc652, /* 50770 3Dconnexion universal receiver *TESTED* */
};
#endif

class Motion3DEvent;
wxDECLARE_EVENT(EVT_MOTION3D, Motion3DEvent);

class Motion3DEvent : public wxEvent {
public:
    Motion3DEvent(const glm::vec3 &t, const glm::vec3 &r);
    Motion3DEvent();
    wxEvent* Clone() const {return new Motion3DEvent(*this);}
    
    
    glm::vec3 translations;
    glm::vec3 rotations;

    wxDECLARE_DYNAMIC_CLASS( Motion3DEvent );
};


// A button press will send a wxCommandEvent with this ID
// with the cmd.GetInt() being the button
wxDECLARE_EVENT(EVT_MOTION3D_BUTTONCLICKED, wxCommandEvent);


class Mouse3DManager {
public:
    bool isEnabled() const { return enabled; }

    #ifdef __USE_HIDAPI__
    // Called by HID enumeration callback.
    void device_detached(const std::string& device);
    #endif
    
    
    static Mouse3DManager INSTANCE;
    
    
    void enableMotionEvents(wxWindow *);
    
    void deviceAdded(int16_t vendorID, int16_t productID);
    void sendButtonEvent(uint32_t button);
    void sendMotionEvents(const glm::vec3 &t, const glm::vec3 &r);
    
    void focusEvent(wxFocusEvent& event);
protected:
    Mouse3DManager();
    ~Mouse3DManager();
    
    void sendEvent(wxEvent *ev);
    
    bool enabled = false;
    wxWindow* lastWindow = nullptr;
    wxWindow* focusWindow = nullptr;

#ifdef __USE_HIDAPI__
private:
    std::thread m_thread;
    hid_device* m_device_ptr{ nullptr };
    std::atomic<bool> m_stop{ false };
    DeviceData m_device;

    bool connect_device(DeviceData const& dev);
    void disconnect_device();
    // secondary thread methods
    void run();
    bool collect_input();

    typedef std::array<unsigned char, 13> DataPacketRaw;

    static DeviceData detect_attached_device();
    static std::string format_device_string(int vid, int pid);

    // Unpack raw 3DConnexion HID packet of a wired 3D mouse into m_state. Called by the worker thread.
    bool handle_input(const DataPacketRaw& packet, const int packet_length);
    // The following is called by handle_input() from the worker thread.
    static bool handle_packet(const DataPacketRaw& packet, const int packet_length);
    static bool handle_packet_translation_rotation(const DataPacketRaw& packet);
    static bool handle_packet_translation(const DataPacketRaw& packet);
    static bool handle_packet_rotation(const DataPacketRaw& packet, unsigned int first_byte);
    static bool handle_packet_button(const DataPacketRaw& packet, unsigned int packet_size);
    static bool handle_packet_button28(const DataPacketRaw& packet, unsigned int packet_size);
    static bool handle_packet_button29(const DataPacketRaw& packet, unsigned int packet_size);
   #endif
};

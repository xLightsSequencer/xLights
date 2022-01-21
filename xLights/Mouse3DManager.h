#pragma once


#include <wx/wx.h>
#include <glm/vec3.hpp>

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
    
    
    static Mouse3DManager INSTANCE;
    
    
    void enableMotionEvents(wxWindow *);
    
    void deviceAdded(int16_t vendorID, int16_t productID);
    void sendButtonEvents(uint32_t buttons);
    void sendMotionEvents(const glm::vec3 &t, const glm::vec3 &r);
    
    void focusEvent(wxFocusEvent& event);
protected:
    Mouse3DManager();
    ~Mouse3DManager();
    
    void sendEvent(wxEvent *ev);
    
    bool enabled = false;
    wxWindow* lastWindow = nullptr;
    wxWindow* focusWindow = nullptr;
};

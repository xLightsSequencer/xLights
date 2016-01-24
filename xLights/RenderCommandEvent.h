//
//  RenderCommandEvent.h
//  xLights
//
//  Created by Daniel Kulp on 2/26/15.
//  Copyright (c) 2015 Daniel Kulp. All rights reserved.
//

#ifndef xLights_RenderCommandEvent_h
#define xLights_RenderCommandEvent_h

#include <string>

class RenderCommandEvent;
wxDECLARE_EVENT(EVT_RENDER_RANGE, RenderCommandEvent);

class RenderCommandEvent : public wxCommandEvent {
public:
    RenderCommandEvent() :  wxCommandEvent(EVT_RENDER_RANGE) {}
    RenderCommandEvent(const std::string &name,
                       int s,
                       int e,
                       bool c = true,
                       bool d = false)
        : wxCommandEvent(EVT_RENDER_RANGE),
          start(s), end(e), model(name),
          clear(c), deleted(d) {
        
    }
    RenderCommandEvent(const RenderCommandEvent &evt) : wxCommandEvent(evt), start(evt.start),
        end(evt.end),
        model(evt.model), clear(evt.clear), deleted(evt.deleted) {
        
    }
    
    virtual ~RenderCommandEvent() {}
    wxCommandEvent* Clone() const {return new RenderCommandEvent(*this);}
    
    
    int start, end;
    std::string model;
    bool clear;
    bool deleted;
    
    DECLARE_DYNAMIC_CLASS( RenderCommandEvent )

};

#endif

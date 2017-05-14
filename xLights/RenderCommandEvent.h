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
class SelectedEffectChangedEvent;

wxDECLARE_EVENT(EVT_RENDER_RANGE, RenderCommandEvent);
wxDECLARE_EVENT(EVT_SELECTED_EFFECT_CHANGED, SelectedEffectChangedEvent);

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

class SelectedEffectChangedEvent : public wxCommandEvent {
public:
    SelectedEffectChangedEvent(Effect *e, bool n, bool uui = true, bool ubtn = false)
        : wxCommandEvent(EVT_SELECTED_EFFECT_CHANGED),
            effect(e), isNew(n), updateUI(uui), updateBtn(ubtn) {

    }
    SelectedEffectChangedEvent(const SelectedEffectChangedEvent &evt)
    : wxCommandEvent(evt), effect(evt.effect), isNew(evt.isNew), updateUI(evt.updateUI), updateBtn(evt.updateBtn) {
    }

    virtual ~SelectedEffectChangedEvent() {}
    wxCommandEvent* Clone() const {return new SelectedEffectChangedEvent(*this);}


    Effect *effect;
    bool isNew;
    bool updateUI;
    bool updateBtn;

    DECLARE_DYNAMIC_CLASS( SelectedEffectChangedEvent )

protected:
    SelectedEffectChangedEvent()
        : wxCommandEvent(EVT_SELECTED_EFFECT_CHANGED), effect(nullptr), isNew(true), updateUI(true) {}

};


#endif

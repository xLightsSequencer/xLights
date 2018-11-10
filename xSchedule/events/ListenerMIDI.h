#ifndef LISTENERMIDI_H
#define LISTENERMIDI_H

#include "ListenerBase.h"
#include <string>
#include <wx/wx.h>

class wxMidiInDevice;

class ListenerMIDI : public ListenerBase
{
    wxMidiInDevice* _midiIn;
    int _deviceId;
    int _frameMS;

    void DoSync(int mode, int hours, int mins, int secs, int frames);

	public:
        ListenerMIDI(int deviceId, ListenerManager* _listenerManager);
		virtual ~ListenerMIDI() {}
		virtual void Start() override;
        virtual void Stop() override;
        virtual std::string GetType() const override { return "MIDI"; }
        virtual void StartProcess() override;
        virtual void StopProcess() override;
        virtual void Poll() override;
        int GetDeviceId() const { return _deviceId; }
};
#endif

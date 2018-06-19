#ifndef MIDILISTENER_H
#define MISLISTENER_H

#include <string>
#include <wx/wx.h>

wxDECLARE_EVENT(EVT_MIDI, wxCommandEvent);

class wxMidiInDevice;

class ListenerThread : public wxThread
{
    bool _stop;
    wxWindow* _target;
	wxWindow* _stashTarget;
    bool _running;
    wxMidiInDevice* _midiIn;
    int _deviceId;
    bool _isOk;

public:
    ListenerThread(int deviceId, wxWindow* win);
    virtual ~ListenerThread()
    {
        Stop();
    }
    bool IsOk() const { return _isOk; }
    void Stop()
    {
        _stop = true;
        // give it 10 ms to end ... this is not perfect but as the thread self deletes I cant wait for it.
        wxMilliSleep(10);
    }

    virtual void* Entry() override;
	void SetWindow(wxWindow* win) { _target = win; }
    int GetDeviceId() const { return _deviceId; }
    void SetTempWindow(wxWindow* win) { _stashTarget = _target; _target = win; }
    void ClearTempWindow() { _target = _stashTarget; _stashTarget = nullptr; }
};

class MIDIListener 
{
	ListenerThread* _thread;

    bool IsValidDeviceId(int deviceId);

	public:
        MIDIListener(int deviceId, wxWindow* win);
		virtual ~MIDIListener() {}
        void Stop();
        void SetTempWindow(wxWindow* win);
        void ClearTempWindow();
        bool IsOk() const;
		void SetDeviceId(int id, wxWindow* win);
};
#endif

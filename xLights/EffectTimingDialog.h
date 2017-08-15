#ifndef EffectTimingDialog_H
#define EffectTimingDialog_H

//(*Headers(EffectTimingDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class Effect;

class StepSpinCtrl : public wxSpinCtrl
{
    int _lastDir;
    int _step;
    int _lastValue;
    wxWindow* _parent;
public:
    
    void OnSpin(wxSpinEvent& event) {
        int delta = GetValue() - _lastValue;

        if (delta > 0)
        {
            if (GetValue() % _step != 0) SetValue(GetValue() - delta + _step);
            _lastDir = 1;
        }
        else if (delta < 0)
        {
            if (GetValue() % _step != 0) SetValue(GetValue() - delta - _step);
            _lastDir = -1;
        }
    }
    
    void SetValue(wxString value)
    {
        SetValue(wxAtoi(value));
    }

    virtual void SetValue(int value)
    {
        int oldValue = GetValue();
        int v = value;

        if (value % _step != 0)
        {
            if (_lastDir > 0)
            {
                v = (value / _step) * _step + _step;
                wxSpinCtrl::SetValue(v);
            }
            else if (_lastDir < 0)
            {
                v = (value / _step) * _step;
                wxSpinCtrl::SetValue(v);
            }
        }
        else
        {
            wxSpinCtrl::SetValue(v);
        }

        if (v != oldValue)
        {
            wxSpinEvent event(wxEVT_SPINCTRL, GetId());
            event.SetEventObject(this);
            event.SetInt(v);

            wxPostEvent(_parent, event);

            _lastValue = v;
        }
    }

    StepSpinCtrl(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString &value = wxEmptyString, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxSP_ARROW_KEYS | wxALIGN_RIGHT, int min = 0, int max = 100, int initial = 0, const wxString &name = "wxSpinCtrl") :
        wxSpinCtrl(parent, id, value, pos, size, style, min, max, initial, name)
    {
        _parent = parent;
        _lastValue = 0;
        _lastDir = 0;
        _step = 50;
        Bind(wxEVT_SPINCTRL, &StepSpinCtrl::OnSpin, this);
    }
    virtual ~StepSpinCtrl() {}
    void SetStep(int step) { _step = step; }
};

class EffectTimingDialog: public wxDialog
{
	public:

		EffectTimingDialog(wxWindow* parent, Effect* eff, int timingInterval,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EffectTimingDialog();
        int GetStartTime() const { return SpinCtrl_StartTime->GetValue(); }
        int GetEndTime() const { return SpinCtrl_EndTime->GetValue(); }

		//(*Declarations(EffectTimingDialog)
		wxButton* Button_Ok;
		wxStaticText* StaticText2;
		StepSpinCtrl* SpinCtrl_StartTime;
		StepSpinCtrl* SpinCtrl_Duration;
		StepSpinCtrl* SpinCtrl_EndTime;
		wxStaticText* StaticText3;
		wxButton* Button_Cancel;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(EffectTimingDialog)
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL3;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(EffectTimingDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnSpinCtrl_StartTimeChange(wxSpinEvent& event);
		void OnSpinCtrl_DurationChange(wxSpinEvent& event);
		void OnSpinCtrl_EndTimeChange(wxSpinEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

        int _timeInterval;
};

#endif

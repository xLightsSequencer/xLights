#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(EffectTimingDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

class Effect;
class EffectLayer;

class StepSpinCtrl : public wxSpinCtrl
{
    int _lastDir;
    int _step;
    int _lastValue;
    wxWindow* _parent;
public:

    void OnKillFocus(wxFocusEvent& event) {
        wxPostEvent(_parent, event);
        event.Skip();
    }

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
        Bind(wxEVT_KILL_FOCUS, &StepSpinCtrl::OnKillFocus, this);
    }
    virtual ~StepSpinCtrl() {}
    void SetStep(int step) { _step = step; }
};

class EffectTimingDialog: public wxDialog
{
	public:

		EffectTimingDialog(wxWindow* parent, Effect* eff, EffectLayer* el, int timingInterval,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EffectTimingDialog();
        int GetStartTime() const { return SpinCtrl_StartTime->GetValue(); }
        int GetEndTime() const { return SpinCtrl_EndTime->GetValue(); }

		//(*Declarations(EffectTimingDialog)
		StepSpinCtrl* SpinCtrl_Duration;
		StepSpinCtrl* SpinCtrl_EndTime;
		StepSpinCtrl* SpinCtrl_StartTime;
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText_Error;
		//*)

	protected:

		//(*Identifiers(EffectTimingDialog)
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL3;
		static const long ID_STATICTEXT1;
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
        EffectLayer* _effectLayer;
        int _effectId;

        void OnSpinCtrl_StartTimeLoseFocus(wxFocusEvent& event);
        void OnSpinCtrl_DurationLoseFocus(wxFocusEvent& event);
        void OnSpinCtrl_EndTimeLoseFocus(wxFocusEvent& event);

        void ValidateWindow();
        void StartTimeChange();
        void DurationChange();
        void EndTimeChange();
};

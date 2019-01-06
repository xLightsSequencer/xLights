#ifndef VALUECURVEDIALOG_H
#define VALUECURVEDIALOG_H

//(*Headers(ValueCurveDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include "xlCustomControl.h"
#include "ValueCurve.h"
#include <wx/dir.h>

class Element;
class wxAutoBufferedPaintDC;

class ValueCurvePanel : public wxWindow, public xlCustomControl
{
public:
    ValueCurvePanel(wxWindow* parent, Element* timingElement, int start, int end, wxWindowID id, const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize, long style = 0);
    virtual ~ValueCurvePanel() {};
    virtual void SetValue(const std::string &val) override {};
    void SetValue(ValueCurve* vc) { _vc = vc; }
    void SetType(std::string type) { _type = type; }
    void SetTimeOffset(int timeoffset) { _timeOffset = timeoffset; }
    void Delete();
    void Undo();
    void SaveUndo(float x, float y);
    void SaveUndoSelected();
    void ClearUndo() { _undo.clear(); }
    bool IsDirty() const { return _undo.size() > 0; }
    bool HasSelected() const { return _grabbedPoint != -1; }

protected:
    DECLARE_EVENT_TABLE()

    void mouseEnter(wxMouseEvent& event);
    void mouseLeave(wxMouseEvent& event);
    void mouseLeftDown(wxMouseEvent& event);
    void mouseLeftUp(wxMouseEvent& event);
    void mouseMoved(wxMouseEvent& event);
    void Paint(wxPaintEvent& event);
    void mouseCaptureLost(wxMouseCaptureLostEvent& event);
private:
    void Convert(float &x, float &y, wxMouseEvent& event);
    void DrawTiming(wxAutoBufferedPaintDC& pdc);
    void DrawTiming(wxAutoBufferedPaintDC& pdc, long timeMS);
    ValueCurve *_vc;
    float _originalGrabbedPoint;
    float _grabbedPoint;
    float _minGrabbedPoint;
    float _maxGrabbedPoint;
    std::string _type;
    int _timeOffset;
    std::list<wxRealPoint> _undo;
    int _start;
    int _end;
    Element* _timingElement;
};

class ValueCurveDialog: public wxDialog
{
    ValueCurve* _vc;
    ValueCurve _backup;
    bool _slideridd;
    void ValidateWindow();
    void SetParameter(int p, float v);
    void SetParameter100(int p, float v);
    ValueCurvePanel* _vcp;
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void PopulatePresets();
    void ProcessPresetDir(wxDir& directory, bool subdirs);
    void SetTextCtrlsFromSliders();
    void SetSlidersFromTextCtrls();
    void SetTextCtrlFromSlider(int parm, wxTextCtrl* text, int value);
    void SetSliderFromTextCtrl(int parm, wxSlider* slider, float value);
    void SetSliderMinMax();

    public:

		ValueCurveDialog(wxWindow* parent, ValueCurve* vc, bool slideridd, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ValueCurveDialog();

		//(*Declarations(ValueCurveDialog)
		wxButton* ButtonExport;
		wxButton* ButtonLoad;
		wxButton* Button_Cancel;
		wxButton* Button_Flip;
		wxButton* Button_Ok;
		wxButton* Button_Reverse;
		wxCheckBox* CheckBox_WrapValues;
		wxChoice* Choice1;
		wxFlexGridSizer* PresetSizer;
		wxSlider* Slider_Parameter1;
		wxSlider* Slider_Parameter2;
		wxSlider* Slider_Parameter3;
		wxSlider* Slider_Parameter4;
		wxSlider* Slider_TimeOffset;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText_BottomValue;
		wxStaticText* StaticText_P1;
		wxStaticText* StaticText_P2;
		wxStaticText* StaticText_P3;
		wxStaticText* StaticText_P4;
		wxStaticText* StaticText_TopValue;
		wxTextCtrl* TextCtrl_Parameter1;
		wxTextCtrl* TextCtrl_Parameter2;
		wxTextCtrl* TextCtrl_Parameter3;
		wxTextCtrl* TextCtrl_Parameter4;
		wxTextCtrl* TextCtrl_TimeOffset;
		//*)

	protected:

		//(*Identifiers(ValueCurveDialog)
		static const long ID_STATICTEXT3;
		static const long ID_STATICTEXT7;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_Parameter1;
		static const long IDD_TEXTCTRL_Parameter1;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_Parameter2;
		static const long IDD_TEXTCTRL_Parameter2;
		static const long ID_STATICTEXT5;
		static const long ID_SLIDER_Parameter3;
		static const long IDD_TEXTCTRL_Parameter3;
		static const long ID_STATICTEXT6;
		static const long ID_SLIDER_Parameter4;
		static const long IDD_TEXTCTRL_Parameter4;
		static const long ID_CHECKBOX_WrapValues;
		static const long ID_BUTTON5;
		static const long ID_BUTTON6;
		static const long ID_STATICTEXT8;
		static const long ID_SLIDER1;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ValueCurveDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnChoice1Select(wxCommandEvent& event);
		void OnTextCtrl_Parameter1Text(wxCommandEvent& event);
		void OnTextCtrl_Parameter2Text(wxCommandEvent& event);
		void OnSlider_Parameter1CmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_Parameter2CmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_Parameter3CmdSliderUpdated(wxScrollEvent& event);
		void OnTextCtrl_Parameter3Text(wxCommandEvent& event);
		void OnChar(wxKeyEvent& event);
		void OnSlider_Parameter4CmdSliderUpdated(wxScrollEvent& event);
		void OnTextCtrl_Parameter4Text(wxCommandEvent& event);
		void OnCheckBox_WrapValuesClick(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnButtonLoadClick(wxCommandEvent& event);
		void OnButtonExportClick(wxCommandEvent& event);
		void OnButtonPresetClick(wxCommandEvent& event);
		void OnButton_ReverseClick(wxCommandEvent& event);
		void OnSlider_TimeOffsetCmdSliderUpdated(wxScrollEvent& event);
		void OnTextCtrl_TimeOffsetText(wxCommandEvent& event);
		void OnButton_FlipClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

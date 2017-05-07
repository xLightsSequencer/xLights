#ifndef VALUECURVEDIALOG_H
#define VALUECURVEDIALOG_H

//(*Headers(ValueCurveDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include "xlCustomControl.h"
#include "ValueCurve.h"
#include <wx/dir.h>

class ValueCurvePanel : public wxWindow, public xlCustomControl
{
public:
    ValueCurvePanel(wxWindow* parent, wxWindowID id, const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize, long style = 0);
    virtual ~ValueCurvePanel() {};
    virtual void SetValue(const std::string &val) override {};
    void SetValue(ValueCurve* vc) { _vc = vc; }
    void SetType(std::string type) { _type = type; }
    void Delete();
    void Undo();
    void SaveUndo(float x, float y);
    void SaveUndoSelected();
    void ClearUndo() { _undo.clear(); }
    bool IsDirty() { return _undo.size() > 0; }

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
    ValueCurve *_vc;
    float _originalGrabbedPoint;
    float _grabbedPoint;
    float _minGrabbedPoint;
    float _maxGrabbedPoint;
    std::string _type;
    std::list<wxRealPoint> _undo;
};

class ValueCurveDialog: public wxDialog
{
    ValueCurve* _vc;
    ValueCurve _backup;
    void ValidateWindow();
    void SetParameter(int p, int v);
    ValueCurvePanel* _vcp;
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void PopulatePresets();
    void LoadXVC(ValueCurve* vc, const wxString& filename);
    void ProcessPresetDir(wxDir& directory, bool subdirs);
    void SetTextCtrlsFromSliders();
    void SetTextCtrlFromSlider(int parm, wxTextCtrl* text, int value);

    public:

		ValueCurveDialog(wxWindow* parent, ValueCurve* vc, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ValueCurveDialog();

		//(*Declarations(ValueCurveDialog)
		wxStaticText* StaticText_P3;
		wxButton* ButtonExport;
		wxTextCtrl* TextCtrl_Parameter3;
		wxSlider* Slider_Parameter2;
		wxButton* Button_Ok;
		wxStaticText* StaticText_P2;
		wxFlexGridSizer* PresetSizer;
		wxTextCtrl* TextCtrl_Parameter2;
		wxStaticText* StaticText_BottomValue;
		wxButton* Button_Cancel;
		wxSlider* Slider_Parameter3;
		wxStaticText* StaticText_P1;
		wxStaticText* StaticText_TopValue;
		wxStaticText* StaticText_P4;
		wxSlider* Slider_Parameter4;
		wxButton* ButtonLoad;
		wxCheckBox* CheckBox_WrapValues;
		wxChoice* Choice1;
		wxSlider* Slider_Parameter1;
		wxTextCtrl* TextCtrl_Parameter1;
		wxTextCtrl* TextCtrl_Parameter4;
		//*)

	protected:

		//(*Identifiers(ValueCurveDialog)
		static const long ID_STATICTEXT3;
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
		void OnPanel_GraphLeftDown(wxMouseEvent& event);
		void OnPanel_GraphLeftUp(wxMouseEvent& event);
		void OnPanel_GraphMouseMove(wxMouseEvent& event);
		void OnTextCtrl_Parameter1Text(wxCommandEvent& event);
		void OnTextCtrl_Parameter2Text(wxCommandEvent& event);
		void OnSlider_Parameter1CmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_Parameter2CmdSliderUpdated(wxScrollEvent& event);
		void OnPanel_GraphPaint(wxPaintEvent& event);
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
        //*)

		DECLARE_EVENT_TABLE()
};

#endif

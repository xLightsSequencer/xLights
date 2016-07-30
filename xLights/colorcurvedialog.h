#ifndef COLORCURVEDIALOG_H
#define COLORCURVEDIALOG_H

//(*Headers(ColorCurveDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/colordlg.h>
#include <wx/grid.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include "xlCustomControl.h"
#include "ColorCurve.h"

class ColorCurvePanel : public wxWindow, public xlCustomControl
{
public:
    ColorCurvePanel(wxWindow* parent, wxWindowID id, const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize, long style = 0);
    virtual ~ColorCurvePanel() {};
    virtual void SetValue(const std::string &val) override {};
    void SetValue(ColorCurve* cc) { _cc = cc; }
    void SetType(std::string type) { _type = type; }
    void Delete();
    void Undo();
    void SaveUndo(float x);
    void SaveUndoSelected();
    void Select(float x);

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
    void Convert(float &x, wxMouseEvent& event) const;
    ColorCurve *_cc;
    float _originalGrabbedPoint;
    float _grabbedPoint;
    std::string _type;
    std::list<float> _undo;
};

class ColorCurveDialog: public wxDialog
{
    ColorCurve* _cc;
    ColorCurve _backup;
    void ValidateWindow();
    void SetParameter(int p, int v);
    ColorCurvePanel* _ccp;

    public:

		ColorCurveDialog(wxWindow* parent, ColorCurve* cc, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ColorCurveDialog();

		//(*Declarations(ColorCurveDialog)
		wxButton* Button_Ok;
		wxColourDialog* ColourDialog1;
		wxStaticText* StaticText1;
		wxGrid* Grid1;
		wxButton* Button_Cancel;
		wxChoice* Choice1;
		//*)

	protected:

		//(*Identifiers(ColorCurveDialog)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_GRID1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ColorCurveDialog)
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
		void OnGrid1CellSelect(wxGridEvent& event);
		void OnGrid1CellLeftDClick(wxGridEvent& event);
		void OnGrid1CellLeftClick(wxGridEvent& event);
		void OnButtonAddClick(wxCommandEvent& event);
		void OnButtonDeleteClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

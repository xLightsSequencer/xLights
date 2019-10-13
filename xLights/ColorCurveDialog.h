#pragma once

//(*Headers(ColorCurveDialog)
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

#include <wx/colourdata.h>
#include <wx/dir.h>

#include "xlCustomControl.h"
#include "ColorCurve.h"

class wxAutoBufferedPaintDC;
class Element;

wxDECLARE_EVENT(EVT_CCP_CHANGED, wxCommandEvent);

class ColorCurvePanel : public wxWindow, public xlCustomControl
{
public:
    ColorCurvePanel(ColorCurve* cc, Element* timingElement, int start, int end , wxColourData &colorData, wxWindow* parent, wxWindowID id, const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize, long style = 0);
    virtual ~ColorCurvePanel() {};
    virtual void SetValue(const std::string &val) override {};
    void SetValue(ColorCurve* cc) { _cc = cc; }
    void SetType(std::string type) { _type = type; }
    void Delete();
    void Flip();
    void Undo();
    void SaveUndo(ccSortableColorPoint& point, bool del);
    void Select(float x);
    void Reloaded();
    void ClearUndo();
    bool IsDirty() { return _undo.size() > 0; }

protected:
    DECLARE_EVENT_TABLE()

    void mouseEnter(wxMouseEvent& event);
    void mouseLeave(wxMouseEvent& event);
    void mouseLeftDown(wxMouseEvent& event);
    void mouseLeftDClick(wxMouseEvent& event);
    void mouseLeftUp(wxMouseEvent& event);
    void mouseMoved(wxMouseEvent& event);
    void Paint(wxPaintEvent& event);
    void mouseCaptureLost(wxMouseCaptureLostEvent& event);
private:
    void Convert(float &x, wxMouseEvent& event) const;
    void NotifyChange()
    {
        wxCommandEvent eventCCPChange(EVT_CCP_CHANGED);
        eventCCPChange.SetEventObject(this);
        wxPostEvent(GetParent(), eventCCPChange);
    }
    void DrawStopsAsLines(wxAutoBufferedPaintDC& pdc);
    void DrawStopsAsHouses(wxAutoBufferedPaintDC& pdc);
    void DrawHouse(wxAutoBufferedPaintDC& pdc, int x, int height, bool selected, const wxColor& c, wxPointList& pl);
    void DrawTiming(wxAutoBufferedPaintDC& pdc);
    void DrawTiming(wxAutoBufferedPaintDC& pdc, long timeMS);
    ColorCurve *_cc;
    //float _originalGrabbedPoint;
    float _grabbedPoint;
    float _startPoint;
    float _minGrabbedPoint;
    float _maxGrabbedPoint;
    std::string _type;
    std::list<ccSortableColorPoint> _undo;
    int _start;
    int _end;
    wxColourData& _colorData;
    Element* _timingElement;
};

class ColorCurveDialog: public wxDialog
{
    ColorCurve* _cc;
    ColorCurve _backup;
    void ValidateWindow();
    ColorCurvePanel* _ccp;
    bool _exported = false;

    void PopulatePresets();
    void ProcessPresetDir(wxDir& directory, bool subdirs);

    public:

		ColorCurveDialog(wxWindow* parent, ColorCurve* cc, wxColourData& colorData, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ColorCurveDialog();
        void OnCCPChanged(wxCommandEvent& event);
        bool DidExport() const { return _exported; }

		//(*Declarations(ColorCurveDialog)
		wxButton* ButtonExport;
		wxButton* ButtonLoad;
		wxButton* Button_Cancel;
		wxButton* Button_Flip;
		wxButton* Button_Ok;
		wxChoice* Choice1;
		wxFlexGridSizer* PresetSizer;
		wxStaticText* StaticText1;
		//*)

	protected:

		//(*Identifiers(ColorCurveDialog)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_BUTTON5;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ColorCurveDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnChoice1Select(wxCommandEvent& event);
		void OnChar(wxKeyEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnButtonLoadClick(wxCommandEvent& event);
		void OnButtonExportClick(wxCommandEvent& event);
		void OnButtonPresetClick(wxCommandEvent& event);
		void OnButton_FlipClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

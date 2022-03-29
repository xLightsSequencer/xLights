#pragma once

#include "effects/SketchEffectDrawing.h"

#include <wx/dialog.h>
#include <wx/image.h>
#include <wx/geometry.h>

#include <memory>
#include <string>
#include <vector>

class wxBitmap;
class wxButton;
class wxFilePickerCtrl;
class wxListBox;
class wxPanel;
class wxSlider;

class SketchPathDialog: public wxDialog
{
public:
	SketchPathDialog(wxWindow* parent, wxWindowID id=wxID_ANY, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize);
    virtual ~SketchPathDialog() = default;

    void setSketch(const std::string& sketchStr);

    std::string sketchDefString() const;

private:
    DECLARE_EVENT_TABLE()

    enum PathState { Undefined,
                     DefineStartPoint,
                     LineToNewPoint,
                     QuadraticCurveToNewPoint,
                     CubicCurveToNewPoint };
    enum HandlePointType { Point,
                           QuadraticControlPt,
                           QuadraticCurveEnd,
                           CubicControlPt1,
                           CubicControlPt2,
                           CubicCurveEnd };
    enum BgUpdateType { ImageUpdate, AlphaUpdate };
    struct HandlePoint {
        HandlePoint(wxPoint2DDouble _pt, HandlePointType _handlePointType=Point) :
            pt(_pt),
            handlePointType(_handlePointType)
        {}
        wxPoint2DDouble pt;
        bool state = false;
        HandlePointType handlePointType = Point;
    };

    wxPanel* m_sketchPanel = nullptr;
    wxFilePickerCtrl* m_filePicker = nullptr;
    wxSlider* m_bgAlphaSlider = nullptr;
    wxButton* m_startPathBtn = nullptr;
    wxButton* m_endPathBtn = nullptr;
    wxButton* m_closePathBtn = nullptr;
    wxListBox* m_pathsListBox = nullptr;
    static long ID_MENU_Delete;

	void OnSketchPaint(wxPaintEvent& event);
    void OnSketchKeyDown(wxKeyEvent& event);
    void OnSketchLeftDown(wxMouseEvent& event);
    void OnSketchLeftUp(wxMouseEvent& event);
    void OnSketchMouseMove(wxMouseEvent& event);
    void OnSketchEnter(wxMouseEvent& event);

    void OnFilePickerCtrl_FileChanged(wxCommandEvent& event);
    void OnSlider_BgAlphaChanged(wxCommandEvent& event);
    void OnButton_StartPath(wxCommandEvent& event);
    void OnButton_EndPath(wxCommandEvent& event);
    void OnButton_ClosePath(wxCommandEvent& event);
    void OnButton_ClearSketch(wxCommandEvent& event);

    void OnListBox_PathSelected(wxCommandEvent& event);
    void OnListBox_ContextMenu(wxContextMenuEvent& event);
    void OnPopupCommand(wxCommandEvent& event);

    void OnButton_Ok(wxCommandEvent& event);
    void OnButton_Cancel(wxCommandEvent& event);

    wxPoint2DDouble UItoNormalized(const wxPoint2DDouble& pt) const;
    wxPoint2DDouble NormalizedToUI(const wxPoint2DDouble& pt) const;
    void UpdateHandlesForPath(long pathIndex);
    void UpdatePathFromHandles(long handleIndex);
    void UpdatePathFromHandles();
    void UpdatePathState(PathState state);
    static bool isControlPoint(const HandlePoint& handlePt);
    std::shared_ptr<SketchEffectPath> CreatePathFromHandles() const;
    void ResetHandlesState(PathState state = Undefined);
    void PopulatePathListBoxFromSketch();
    void UpdateBgBitmap(BgUpdateType updateType);

    // Handles and PathState are for the currently active path
    std::vector<HandlePoint> m_handles;
    std::vector<HandlePoint>::size_type m_grabbedHandleIndex = -1;
    PathState m_pathState = Undefined;
    bool m_pathClosed = false;

    SketchEffectSketch m_sketch;
    wxPoint2DDouble m_mousePos;
    int m_pathIndexToDelete = -1;

    wxImage m_bgImage;
    std::unique_ptr<wxBitmap> m_bgBitmap;
    unsigned char m_bitmapAlpha = 0x30;
};

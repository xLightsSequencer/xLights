#pragma once

#include "../SketchEffectDrawing.h"
#include "SketchCanvasPanel.h"      // for ISketchCanvasParent

#include <functional>
#include <string>

#include <wx/image.h>
#include <wx/panel.h>

class wxButton;
class wxListBox;

class SketchAssistPanel : public wxPanel,
                          public ISketchCanvasParent
{
public:
    // Effect panels are static (a single panel for the lifetime of the app) but
    // assist panels are not... so it seems like it should be safe for an assist
    // panel to reference an effect panel via a lambda 'this' capture
    typedef std::function<void(const std::string&, const std::string&, unsigned char)> SketchUpdateCallback;

    SketchAssistPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

    // Assist panels seem to be created/destroyed quite frequently when switching between effects
    virtual ~SketchAssistPanel() = default;

    void SetSketchDef(const std::string& sketchDef);
    void SetSketchUpdateCallback(SketchUpdateCallback cb)
    {
        m_sketchUpdateCB = cb;
    }

    void ForwardKeyEvent(wxKeyEvent& event);

    // ISketchCanvasParent impl
    SketchEffectSketch& GetSketch() override;
    int GetSelectedPathIndex() override;
    void NotifySketchUpdated() override;
    void NotifySketchPathsUpdated() override;
    void NotifyPathStateUpdated(SketchCanvasPathState state) override;
    void SelectLastPath() override;

    void UpdateSketchBackground(const wxString& imagePath, int opacity);

private:
    DECLARE_EVENT_TABLE()

    void OnButton_StartPath(wxCommandEvent& event);
    void OnButton_EndPath(wxCommandEvent& event);
    void OnButton_ClosePath(wxCommandEvent& event);
    void OnButton_ContinuePath(wxCommandEvent& event);
    void OnButton_ClearSketch(wxCommandEvent& event);
    void OnButton_ImportSketch(wxCommandEvent& event);
    void OnButton_ExportSketch(wxCommandEvent& event);
    void OnButton_ImportSVG(wxCommandEvent& event);

    void OnListBox_PathSelected(wxCommandEvent& event);
    void OnListBox_ContextMenu(wxContextMenuEvent& event);
    void OnPopupCommand(wxCommandEvent& event);

    void updateBgImage();
    void populatePathListBoxFromSketch();
    bool canContinuePath() const;

    bool areSame(double a, double b, float eps) const;
    bool areCollinear(const wxPoint2DDouble& a, const wxPoint2DDouble& b, const wxPoint2DDouble& c, double eps) const;

    std::string m_sketchDef;
    SketchEffectSketch m_sketch;
    SketchUpdateCallback m_sketchUpdateCB;

    SketchCanvasPanel* m_sketchCanvasPanel = nullptr;
    wxButton* m_startPathBtn = nullptr;
    wxButton* m_endPathBtn = nullptr;
    wxButton* m_closePathBtn = nullptr;
    wxButton* m_continuePathBtn = nullptr;
    wxButton* m_clearSketchBtn = nullptr;
    wxButton* m_importSketchBtn = nullptr;
    wxButton* m_exportSketchBtn = nullptr;
    wxButton* m_importSVGBtn = nullptr;

    wxListBox* m_pathsListBox = nullptr;
    static long ID_MENU_Delete;
    static long ID_MENU_Reverse;
    static long ID_MENU_MoveUp;
    static long ID_MENU_MoveDown;

    wxString m_bgImagePath;
    wxImage m_bgImage;
    unsigned char m_bitmapAlpha = 0x30;
    int m_pathIndexToDelete = -1;
};
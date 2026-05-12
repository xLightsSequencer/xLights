/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

 //(*InternalHeaders(LayoutPanel)
 #include <wx/button.h>
 #include <wx/checkbox.h>
 #include <wx/choice.h>
 #include <wx/font.h>
 #include <wx/intl.h>
 #include <wx/notebook.h>
 #include <wx/scrolbar.h>
 #include <wx/settings.h>
 #include <wx/sizer.h>
 #include <wx/splitter.h>
 #include <wx/stattext.h>
 #include <wx/string.h>
 //*)

#include <wx/stopwatch.h>
#include <wx/clipbrd.h>
#include <wx/progdlg.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/tglbtn.h>
#include <wx/srchctrl.h>
#include <pugixml.hpp>
#include <fstream>
#include <functional>
#include <spdlog/fmt/fmt.h>
#include <regex>
#include <sstream>
#include <wx/artprov.h>
#include <wx/dataview.h>
#include <wx/treebase.h>
#include <wx/colordlg.h>
#include <wx/numdlg.h>

#include "layout/LayoutPanel.h"
#include "layout/ModelPreview.h"
#include "xLightsMain.h"
#include "xLightsApp.h"
#include "settings/XLightsConfigAdapter.h"
#include "model/ChannelLayoutDialog.h"
#include "setup/ControllerConnectionDialog.h"
#include "layout/ModelGroupPanel.h"
#include "model/EditSubmodelAliasesDialog.h"
#include "layout/ViewObjectPanel.h"
#include "layout/LayoutGroup.h"
#include "models/ModelImages.h"
#include "models/SubModel.h"
#include "models/PolyLineModel.h"
#include "models/ModelGroup.h"
#include "models/ViewObject.h"
#include "models/RulerObject.h"
#include "models/CustomModel.h"
#include "models/handles/HitTest.h"

#include "XmlSerializer/FileSerializingVisitor.h"
#include "XmlSerializer/StringSerializingVisitor.h"
#include "XmlSerializer/XmlSerializer.h"
#include "model/WiringDialog.h"
#include "model/ModelDimmingCurveDialog.h"
#include "UtilFunctions.h"
#include "shared/utils/ExternalHooksUI.h"
#include "color/ColorManager.h"
#include "utils/VectorMath.h"
#include "../app-shell/KeyBindings.h"
#include "sequencer/MainSequencer.h"
#include "model/ImportPreviewsModelsDialog.h"
#include "layout/ViewsModelsPanel.h"
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "cad/ModelToCAD.h"
#include "layout/LORPreview.h"
#include "model/ModelFaceDialog.h"
#include "model/ModelStateDialog.h"
#include "model/CustomModelDialog.h"
#include "model/SubModelsDialog.h"
#include "color/xlColourData.h"
#include "shared/utils/xlPropertyGrid.h"
#include "modelproperties/ModelPropertyManager.h"
#include "modelproperties/ViewObjectPropertyManager.h"
#include "modelproperties/ViewObjectPropertyAdapter.h"
#include "modelproperties/ModelPropertyAdapter.h"

#include "layout/LayoutUtils.h"
#include "shared/utils/wxUtilities.h"
#include "import_export/VendorModelDialog.h"
#include "CachedFileDownloader.h"
#include "XmlSerializer/GdtfParser.h"

#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/mstream.h>
#include <wx/uri.h>

#include <log.h>

#include <wx/cursor.h>
#include <wx/aui/floatpane.h>
#include "utils/CursorType.h"

inline wxCursor CursorTypeToWx(CursorType ct) {
    switch (ct) {
        case CursorType::Hand: return wxCursor(wxCURSOR_HAND);
        case CursorType::Sizing: return wxCursor(wxCURSOR_SIZING);
        case CursorType::SizeWE: return wxCursor(wxCURSOR_SIZEWE);
        case CursorType::SizeNS: return wxCursor(wxCURSOR_SIZENS);
        case CursorType::SizeNWSE: return wxCursor(wxCURSOR_SIZENWSE);
        case CursorType::SizeNESW: return wxCursor(wxCURSOR_SIZENESW);
        case CursorType::Bullseye: return wxCursor(wxCURSOR_BULLSEYE);
        case CursorType::Wait: return wxCursor(wxCURSOR_WAIT);
        default: return wxCursor(wxCURSOR_DEFAULT);
    }
}

#include <set>

// Layout sizing constants
static constexpr int kPaneMinHeight      = 400; // minimum height for ModelList / ModelSettings panes
static constexpr int kListHeightFallback = 200; // fallback half-height used when container is hidden
static constexpr int kMinPaneWidth       = 150; // absolute floor for left-panel sash drag (px)

static inline handles::Modifier ModsFromEvent(const wxKeyboardState& event) {
    handles::Modifier mods = handles::Modifier::None;
    if (event.ShiftDown())   mods = mods | handles::Modifier::Shift;
    if (event.ControlDown()) mods = mods | handles::Modifier::Control;
    return mods;
}
// Left-panel target width is computed dynamically in UpdateLayoutSplitter() as
// 18% of the splitter width (floor kMinPaneWidth) so it scales with screen resolution.

// Custom AUI manager with two enhancements:
// 1. Floating frames always have wxCLOSE_BOX, regardless of pane CloseButton flag
//    (lets us suppress the AUI caption close button while keeping the OS X button).
// 2. Center-docked panes can be dragged to float — wxAUI hardcodes an early return
//    for center panes in OnLeftDown, which we work around by tracking the drag
//    ourselves and calling StartPaneDrag once the threshold is exceeded.
class LayoutAuiManager : public wxAuiManager {
public:
    // Called (via CallAfter) after a drag-initiated float or dock completes.
    std::function<void()> m_onPaneStateChanged;

    LayoutAuiManager(wxWindow* managed_wnd, unsigned int flags)
        : wxAuiManager(managed_wnd, flags) {}

    wxAuiFloatingFrame* CreateFloatingFrame(wxWindow* parent, const wxAuiPaneInfo& p) override {
        wxAuiFloatingFrame* frame = new wxAuiFloatingFrame(parent, this, p, wxID_ANY,
            wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION |
            wxFRAME_NO_TASKBAR | wxFRAME_FLOAT_ON_PARENT |
            wxCLIP_CHILDREN | wxCLOSE_BOX);
        // Prevent floating panels from being resized smaller than a usable area.
        frame->SetMinClientSize(wxSize(300, kPaneMinHeight));
        return frame;
    }

    // Track a pending center-pane drag until the system drag threshold is met.
    bool m_pendingCenterDrag = false;
    wxWindow* m_centerDragWindow = nullptr;
    wxPoint m_centerDragStart;
    wxPoint m_centerDragOffset;

    // Saved state for horizontal-dock sash clamping.
    // Set on the first OnMotion call of each resize drag (while m_actionPart is
    // still valid), then used for all subsequent calls so we never touch the
    // dangling m_actionPart or the potentially-stale m_currentDragItem again.
    // -1 means "not currently in a horizontal-dock resize".
    int m_horizResizeDockY = -1;
    int m_horizResizeActionOffsetY = 0;

    void OnLeftDown(wxMouseEvent& event) {
        // Reset per-drag clamping state before the base class sets m_actionPart.
        m_horizResizeDockY = -1;
        m_horizResizeActionOffsetY = 0;
        wxAuiDockUIPart* part = HitTest(event.GetX(), event.GetY());
        if (part &&
            (part->type == wxAuiDockUIPart::typeCaption ||
             part->type == wxAuiDockUIPart::typeGripper) &&
            part->dock &&
            part->dock->dock_direction == wxAUI_DOCK_CENTER &&
            part->pane && !part->pane->IsToolbar() &&
            (GetFlags() & wxAUI_MGR_ALLOW_FLOATING) &&
            part->pane->IsFloatable()) {
            m_pendingCenterDrag = true;
            m_centerDragWindow = part->pane->window;
            m_centerDragStart = wxPoint(event.GetX(), event.GetY());
            m_centerDragOffset = wxPoint(event.GetX() - part->rect.x,
                                         event.GetY() - part->rect.y);
        }
        event.Skip();
    }

    void OnMotion(wxMouseEvent& event) {
        if (m_pendingCenterDrag && m_centerDragWindow) {
            int dx = std::abs(event.GetX() - m_centerDragStart.x);
            int dy = std::abs(event.GetY() - m_centerDragStart.y);
            int tx = wxSystemSettings::GetMetric(wxSYS_DRAG_X, GetManagedWindow());
            int ty = wxSystemSettings::GetMetric(wxSYS_DRAG_Y, GetManagedWindow());
            static const int kDefaultDragThreshold = 4;
            if (tx < 0) tx = kDefaultDragThreshold;
            if (ty < 0) ty = kDefaultDragThreshold;
            if (dx > tx || dy > ty) {
                m_pendingCenterDrag = false;
                wxWindow* wnd = m_centerDragWindow;
                wxPoint offset = m_centerDragOffset;
                m_centerDragWindow = nullptr;
                wxAuiPaneInfo& pane = GetPane(wnd);
                if (pane.IsOk() && pane.IsFloatable()) {
                    pane.Float();
                    Update();
                    StartPaneDrag(wnd, offset);
                }
                return;  // don't let wxAuiManager::OnMotion run this cycle
            }
        }

        // Clamp the sash Y to enforce minimum heights for ModelList and the settings pane.
        // This runs before event.Skip() so the base class sees the clamped mouse position.
        //
        // Pointer/index stability problem: m_actionPart becomes a dangling pointer after
        // every live-resize Update() call (OnMotion→base Update() rebuilds m_uiParts).
        // m_currentDragItem can also go stale after a rebuild if GetActionPartIndex()
        // fails to locate the part (e.g. the sash disappears from m_uiParts when one pane
        // reaches its minimum size).  Stale/null lookups skip the clamp entirely, letting
        // the sash fly past the minimum to the raw mouse position.
        //
        // Fix: on the very FIRST OnMotion of each drag (before any Update() has fired,
        // while m_actionPart is still valid), detect whether we are resizing a horizontal
        // dock sizer and save the dock-rect Y and action-offset Y.  All subsequent calls
        // use those saved values — no pointer or index dereferenced again.
        if (m_action == actionResize) {
            if (m_horizResizeDockY < 0) {
                // First call: m_actionPart is still valid; m_currentDragItem may or may
                // not be set yet depending on the wxAUI version.
                wxAuiDockUIPart* part = nullptr;
                if (m_currentDragItem >= 0 && m_currentDragItem < (int)m_uiParts.GetCount())
                    part = &m_uiParts.Item(m_currentDragItem);
                else
                    part = m_actionPart; // valid before the first live-resize Update()
                if (part != nullptr &&
                    part->type == wxAuiDockUIPart::typeDockSizer &&
                    part->dock != nullptr &&
                    (part->dock->dock_direction == wxAUI_DOCK_TOP ||
                     part->dock->dock_direction == wxAUI_DOCK_BOTTOM)) {
                    m_horizResizeDockY = part->dock->rect.y;
                    m_horizResizeActionOffsetY = m_actionOffset.y;
                }
            }

            if (m_horizResizeDockY >= 0) {
                wxAuiPaneInfo& listPane = GetPane("ModelList");
                bool centerVisible = false;
                for (const char* nm : {"ModelSettings", "ModelGroupSettings"}) {
                    wxAuiPaneInfo& p = GetPane(nm);
                    if (p.IsOk() && p.IsShown() && !p.IsFloating()) {
                        centerVisible = true;
                        break;
                    }
                }
                if (listPane.IsOk() && listPane.IsShown() && !listPane.IsFloating() && centerVisible) {
                    // new_size = (event.m_y - m_horizResizeActionOffsetY) - m_horizResizeDockY
                    // Enforce new_size >= kPaneMinHeight and (containerH - new_size) >= kPaneMinHeight.
                    int containerH = GetManagedWindow()->GetClientSize().GetHeight();
                    int minY = kPaneMinHeight + m_horizResizeActionOffsetY + m_horizResizeDockY;
                    int maxY = (containerH - kPaneMinHeight) + m_horizResizeActionOffsetY + m_horizResizeDockY;
                    if (maxY < minY) maxY = minY; // degenerate: window too small for both minimums
                    event.m_y = std::clamp(event.m_y, minY, maxY);
                }
            }
        }

        event.Skip();
    }

    void OnLeftUp(wxMouseEvent& event) {
        // Capture interaction state before clearing — only trigger the post-layout
        // callback when an actual pane drag or resize took place, not on every click.
        bool wasDragging = m_action != actionNone;
        m_pendingCenterDrag    = false;
        m_centerDragWindow     = nullptr;
        // Reset per-drag clamping state.
        m_horizResizeDockY = -1;
        m_horizResizeActionOffsetY = 0;
        event.Skip();
        // After the base-class finishes processing the mouse-up (which may have
        // docked or floated a pane), update the splitter state.
        if (m_onPaneStateChanged && wasDragging) {
            GetManagedWindow()->CallAfter(m_onPaneStateChanged);
        }
    }

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(LayoutAuiManager, wxAuiManager)
    EVT_LEFT_DOWN(LayoutAuiManager::OnLeftDown)
    EVT_MOTION(LayoutAuiManager::OnMotion)
    EVT_LEFT_UP(LayoutAuiManager::OnLeftUp)
wxEND_EVENT_TABLE()

#define MODELCOLNAME "Model/Group"
#define STARTCHANCOLNAME "Start Chan"
#define ENDCHANCOLNAME "End Chan"
#define CONTCONNCOLNAME "Ctrlr Conn"

static wxRect scaledRect(int srcWidth, int srcHeight, int dstWidth, int dstHeight)
{
	wxRect r;
	float srcAspectRatio = float(srcWidth) / srcHeight;
	float dstAspectRatio = float(dstWidth) / dstHeight;

	if (srcAspectRatio > dstAspectRatio)
	{
		r.SetWidth(dstWidth);
		r.SetHeight(int(dstWidth / srcAspectRatio));
		r.SetTopLeft(wxPoint(0, (dstHeight - r.GetHeight()) / 2));
	}
	else
	{
		r.SetHeight(dstHeight);
		r.SetWidth(int(dstHeight * srcAspectRatio));
		r.SetTopLeft(wxPoint((dstWidth - r.GetWidth()) / 2, 0));
	}
	return r;
}

wxTreeListItem lastFoundItem = nullptr;

//(*IdInit(LayoutPanel)
const wxWindowID LayoutPanel::ID_PANEL4 = wxNewId();
const wxWindowID LayoutPanel::ID_PANEL_Objects = wxNewId();
const wxWindowID LayoutPanel::ID_NOTEBOOK_OBJECTS = wxNewId();
const wxWindowID LayoutPanel::ID_PANEL3 = wxNewId();
const wxWindowID LayoutPanel::ID_PANEL2 = wxNewId();
const wxWindowID LayoutPanel::ID_SPLITTERWINDOW1 = wxNewId();
const wxWindowID LayoutPanel::ID_CHECKBOX_3D = wxNewId();
const wxWindowID LayoutPanel::ID_CHECKBOXOVERLAP = wxNewId();
const wxWindowID LayoutPanel::ID_CHECKBOXSHOWNAMES = wxNewId();
const wxWindowID LayoutPanel::ID_CHECKBOXSHOWINFO = wxNewId();
const wxWindowID LayoutPanel::ID_BUTTON_SAVE_PREVIEW = wxNewId();
const wxWindowID LayoutPanel::ID_PANEL5 = wxNewId();
const wxWindowID LayoutPanel::ID_STATICTEXT1 = wxNewId();
const wxWindowID LayoutPanel::ID_CHOICE_PREVIEWS = wxNewId();
const wxWindowID LayoutPanel::ID_SCROLLBAR1 = wxNewId();
const wxWindowID LayoutPanel::ID_SCROLLBAR2 = wxNewId();
const wxWindowID LayoutPanel::ID_PANEL1 = wxNewId();
const wxWindowID LayoutPanel::ID_SPLITTERWINDOW2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(LayoutPanel,wxPanel)
	//(*EventTable(LayoutPanel)
	//*)
    EVT_TREELIST_SELECTION_CHANGED(wxID_ANY, LayoutPanel::OnSelectionChanged)
    EVT_TREELIST_ITEM_CONTEXT_MENU(wxID_ANY, LayoutPanel::OnItemContextMenu)
END_EVENT_TABLE()

const long LayoutPanel::ID_TREELISTVIEW_MODELS = wxNewId();
const long LayoutPanel::ID_PREVIEW_REPLACEMODEL = wxNewId();
const long LayoutPanel::ID_PREVIEW_RESET = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN = wxNewId();
const long LayoutPanel::ID_PREVIEW_RESIZE = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_NODELAYOUT = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_LOCK = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_UNLOCK = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_UNLINKFROMBASE = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_EXPORTASCUSTOM = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_EXPORTASCUSTOM3D = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_CREATEGROUP = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_WIRINGVIEW = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_ASPECTRATIO = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL = wxNewId();
const long LayoutPanel::ID_PREVIEW_RESIZE_SAMEWIDTH = wxNewId();
const long LayoutPanel::ID_PREVIEW_RESIZE_SAMEHEIGHT = wxNewId();
const long LayoutPanel::ID_PREVIEW_RESIZE_SAMESIZE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTION = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERNAME = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_SETACTIVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_SETINACTIVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_SMARTREMOTE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_TAGCOLOUR = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_PIXELSIZE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_PIXELSTYLE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_TRANSPARENCY = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_BLACKTRANSPARENCY = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_SHADOWMODELFOR = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERGAMMA = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERCOLOURORDER = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERBRIGHTNESS = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERSTARTNULLNODES = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERENDNULLNODES = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERDIRECTION = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERGROUPCOUNT = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERPROTOCOL = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTIONINCREMENT = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_SMARTREMOTETYPE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_PREVIEW = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_DIMMINGCURVES = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_TOP = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_GROUND = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_BOTTOM = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_LEFT = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_RIGHT = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_H_CENTER = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_V_CENTER = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_D_CENTER = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_FRONT = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_BACK = wxNewId();
const long LayoutPanel::ID_PREVIEW_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_PREVIEW_H_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_PREVIEW_V_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_PREVIEW_D_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_MNU_REMOVE_MODEL_FROM_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_EDIT_SUBMODEL_ALIAS = wxNewId();
const long LayoutPanel::ID_MNU_DELETE_MODEL = wxNewId();
const long LayoutPanel::ID_MNU_DELETE_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_DELETE_EMPTY_MODEL_GROUPS = wxNewId();
const long LayoutPanel::ID_MNU_DELETE_ALL_ALIASES = wxNewId();
const long LayoutPanel::ID_MNU_RENAME_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_CLONE_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_BULKEDIT_GROUP_TAGCOLOR = wxNewId();
const long LayoutPanel::ID_MNU_BULKEDIT_GROUP_PREVIEW = wxNewId();
const long LayoutPanel::ID_MNU_MAKESCVALID = wxNewId();
const long LayoutPanel::ID_MNU_MAKEALLSCVALID = wxNewId();
const long LayoutPanel::ID_MNU_MAKEALLSCNOTOVERLAPPING = wxNewId();
const long LayoutPanel::ID_MNU_ADD_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_ADD_TO_EXISTING_GROUPS = wxNewId();
const long LayoutPanel::ID_MNU_REMOVE_FROM_EXISTING_GROUPS = wxNewId();
const long LayoutPanel::ID_PREVIEW_DELETE_ACTIVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_RENAME_ACTIVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_ADDPOINT = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_DELETEPOINT = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_ADDCURVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_DELCURVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_SET_SEGMENTS = wxNewId();
const long LayoutPanel::ID_PREVIEW_SAVE_LAYOUT_IMAGE = wxNewId();
const long LayoutPanel::ID_PREVIEW_PRINT_LAYOUT_IMAGE = wxNewId();
const long LayoutPanel::ID_PREVIEW_SAVE_VIEWPOINT = wxNewId();
const long LayoutPanel::ID_PREVIEW_VIEWPOINT_DEFAULT = wxNewId();
const long LayoutPanel::ID_PREVIEW_VIEWPOINT_DEFAULT_RESTORE = wxNewId();
const long LayoutPanel::ID_PREVIEW_VIEWPOINT2D = wxNewId();
const long LayoutPanel::ID_PREVIEW_VIEWPOINT3D = wxNewId();
const long LayoutPanel::ID_PREVIEW_DELETEVIEWPOINT2D = wxNewId();
const long LayoutPanel::ID_PREVIEW_DELETEVIEWPOINT3D = wxNewId();
const long LayoutPanel::ID_PREVIEW_DELETEALLVIEWPOINTS3D = wxNewId();
const long LayoutPanel::ID_PREVIEW_REVERT_TO_2D = wxNewId();
const long LayoutPanel::ID_PREVIEW_IMPORTMODELSFROMRGBEFFECTS = wxNewId();
const long LayoutPanel::ID_PREVIEW_IMPORT_MODELS_FROM_LORS5 = wxNewId();
const long LayoutPanel::ID_ADD_OBJECT_IMAGE = wxNewId();
const long LayoutPanel::ID_ADD_OBJECT_GRIDLINES = wxNewId();
const long LayoutPanel::ID_ADD_OBJECT_TERRIAN = wxNewId();
const long LayoutPanel::ID_ADD_OBJECT_RULER = wxNewId();
const long LayoutPanel::ID_ADD_OBJECT_MESH = wxNewId();
const long LayoutPanel::ID_ADD_DMX_MOVING_HEAD = wxNewId();
const long LayoutPanel::ID_ADD_DMX_MOVING_HEAD_ADV = wxNewId();
const long LayoutPanel::ID_ADD_DMX_GENERAL = wxNewId();
const long LayoutPanel::ID_ADD_DMX_SKULL = wxNewId();
const long LayoutPanel::ID_ADD_DMX_SERVO = wxNewId();
const long LayoutPanel::ID_ADD_DMX_SERVO_3D = wxNewId();
const long LayoutPanel::ID_ADD_DMX_FLOODLIGHT = wxNewId();
const long LayoutPanel::ID_ADD_DMX_FLOODAREA = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_CAD_EXPORT = wxNewId();
const long LayoutPanel::ID_PREVIEW_LAYOUT_DXF_EXPORT = wxNewId();
const long LayoutPanel::ID_PREVIEW_EXPORT_FACESSTATESSUBMODELS = wxNewId();
const long LayoutPanel::ID_PREVIEW_FLIP_HORIZONTAL = wxNewId();
const long LayoutPanel::ID_PREVIEW_FLIP_VERTICAL = wxNewId();
const long LayoutPanel::ID_SET_CENTER_OFFSET = wxNewId();
const long LayoutPanel::ID_TEXTCTRL_MODEL_FILTER = wxNewId();

#define CHNUMWIDTH "10000000000000"

#define PlatformHandleSelectionChanged() HandleSelectionChanged()

class ModelTreeData : public wxTreeItemData {
public:
    ModelTreeData(Model *m, int NativeOrder, bool fullname) :wxTreeItemData(), fullname(fullname), model(m) {
        wxASSERT(m != nullptr);
        //a SetFromXML call on the parent (example: recalc start channels) will cause
        //submodel pointers to be deleted.  Need to not save them, but instead, use the parent
        //and query by name
        nativeOrder = NativeOrder;
        SubModel *s = dynamic_cast<SubModel*>(m);
        if (s != nullptr) {
            model = s->GetParent();
            subModel = s->GetName();
        }
        startingChannel = m->GetNumberFromChannelString(m->ModelStartChannel);
        endingChannel = m->GetLastChannel();
    };
    virtual ~ModelTreeData() {};

    Model *GetModel() const
    {
        if ("" != subModel) {
            return model->GetSubModel(subModel);
        }
        return model;
    }

    int startingChannel;
    int endingChannel;
    int nativeOrder;
    bool fullname;
private:
    Model *model;
    std::string subModel;
};

class NewModelBitmapButton : public wxBitmapButton
{
public:

    NewModelBitmapButton(wxWindow *parent, const wxImage &img, int iconSize, const std::string &type)
        : wxBitmapButton(parent, wxID_ANY, wxBitmapBundle()), modelType(type),
          originalImage(img), originalDisabled(img.ConvertToDisabled()), originalPressed(img.ConvertToDisabled(128)) {
        SetToolTip("Create new " + type);
        UpdateIconSize(iconSize);
    }
    virtual ~NewModelBitmapButton() {}

    void UpdateIconSize(int iconSize) {
#ifdef __WXMSW__
        const wxImageResizeQuality quality = wxIMAGE_QUALITY_BICUBIC;
#else
        const wxImageResizeQuality quality = wxIMAGE_QUALITY_HIGH;
#endif
        double sf = GetContentScaleFactor();
        wxImage imgScaled = originalImage.Scale(iconSize, iconSize, quality);
        wxImage disScaled = originalDisabled.Scale(iconSize, iconSize, quality);
        wxImage presScaled = originalPressed.Scale(iconSize, iconSize, quality);
        wxImage imgScaledSF = originalImage.Scale(iconSize * sf, iconSize * sf, quality);
        wxImage disScaledSF = originalDisabled.Scale(iconSize * sf, iconSize * sf, quality);
        wxImage presScaledSF = originalPressed.Scale(iconSize * sf, iconSize * sf, quality);

        bitmap = wxBitmapBundle::FromBitmaps(imgScaled, wxBitmap(imgScaledSF, sf));
        bitmapDisabled = wxBitmapBundle::FromBitmaps(disScaled, wxBitmap(disScaledSF, sf));
        pressedBitmap = wxBitmapBundle::FromBitmaps(presScaled, wxBitmap(presScaledSF, sf));

        SetState(state);
        InvalidateBestSize();
    }

    void SetState(unsigned int s) {
        if (s > 2) {
            s = 0;
        }
        state = s;
        if (state == 2) {
            SetBitmap(bitmapDisabled);
        } else if (state == 1) {
            SetBitmap(pressedBitmap);
        } else {
            SetBitmap(bitmap);
        }
    }
    unsigned int GetState() const
    {
        return state;
    }
    const std::string &GetModelType() const
    {
        return modelType;
    }
protected:
private:
    const std::string modelType;
    uint32_t state = 0;
    wxImage originalImage;
    wxImage originalDisabled;
    wxImage originalPressed;
    wxBitmapBundle bitmap;
    wxBitmapBundle bitmapDisabled;
    wxBitmapBundle pressedBitmap;
};

LayoutPanel::LayoutPanel(wxWindow* parent, xLightsFrame *xl, wxPanel* sequencer) : xlights(xl), main_sequencer(sequencer)
{
    

	//(*Initialize(LayoutPanel)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizerPreview;
	wxFlexGridSizer* LayoutGLSizer;
	wxFlexGridSizer* LeftPanelSizer;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizerPreview = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizerPreview->AddGrowableCol(0);
	FlexGridSizerPreview->AddGrowableRow(0);
	SplitterWindow2 = new wxSplitterWindow(this, ID_SPLITTERWINDOW2, wxDefaultPosition, wxDefaultSize, wxSP_3D|wxSP_LIVE_UPDATE, _T("ID_SPLITTERWINDOW2"));
	SplitterWindow2->SetMinimumPaneSize(10);
	SplitterWindow2->SetSashGravity(0.5);
	LeftPanel = new wxPanel(SplitterWindow2, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	LeftPanelSizer = new wxFlexGridSizer(0, 1, 0, 0);
	LeftPanelSizer->AddGrowableCol(0);
	LeftPanelSizer->AddGrowableRow(0);
	ModelSplitter = new wxSplitterWindow(LeftPanel, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D|wxSP_LIVE_UPDATE, _T("ID_SPLITTERWINDOW1"));
	ModelSplitter->SetMinimumPaneSize(0);
	ModelSplitter->SetSashGravity(0.5);
	FirstPanel = new wxPanel(ModelSplitter, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(0);
	Notebook_Objects = new wxNotebook(FirstPanel, ID_NOTEBOOK_OBJECTS, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_OBJECTS"));
	Notebook_Objects->SetMinSize(wxDLG_UNIT(FirstPanel,wxSize(-1,100)));
	PanelModels = new wxPanel(Notebook_Objects, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	PanelObjects = new wxPanel(Notebook_Objects, ID_PANEL_Objects, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Objects"));
	Notebook_Objects->AddPage(PanelModels, _("Models"), false);
	Notebook_Objects->AddPage(PanelObjects, _("3D Objects"), false);
	FlexGridSizer4->Add(Notebook_Objects, 1, wxALL|wxEXPAND, 1);
	FirstPanel->SetSizer(FlexGridSizer4);
	SecondPanel = new wxPanel(ModelSplitter, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	ModelSplitter->SplitHorizontally(FirstPanel, SecondPanel);
	LeftPanelSizer->Add(ModelSplitter, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	CheckBox_3D = new wxCheckBox(LeftPanel, ID_CHECKBOX_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_3D"));
	CheckBox_3D->SetValue(false);
	FlexGridSizer3->Add(CheckBox_3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBoxOverlap = new wxCheckBox(LeftPanel, ID_CHECKBOXOVERLAP, _("Overlap checks enabled"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOXOVERLAP"));
	CheckBoxOverlap->SetValue(false);
	FlexGridSizer3->Add(CheckBoxOverlap, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBoxShowNames = new wxCheckBox(LeftPanel, ID_CHECKBOXSHOWNAMES, _("Show Names"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOXSHOWNAMES"));
	CheckBoxShowNames->SetValue(false);
	FlexGridSizer3->Add(CheckBoxShowNames, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBoxShowInfo = new wxCheckBox(LeftPanel, ID_CHECKBOXSHOWINFO, _("Show Start Channel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOXSHOWINFO"));
	CheckBoxShowInfo->SetValue(false);
	FlexGridSizer3->Add(CheckBoxShowInfo, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	LeftPanelSizer->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	ButtonSavePreview = new wxButton(LeftPanel, ID_BUTTON_SAVE_PREVIEW, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_PREVIEW"));
	LeftPanelSizer->Add(ButtonSavePreview, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	LeftPanel->SetSizer(LeftPanelSizer);
	PreviewGLPanel = new wxPanel(SplitterWindow2, ID_PANEL1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_PANEL1"));
	PreviewGLSizer = new wxFlexGridSizer(3, 1, 0, 0);
	PreviewGLSizer->AddGrowableCol(0);
	PreviewGLSizer->AddGrowableRow(1);
	TopBarSizer = new wxFlexGridSizer(0, 6, 0, 0);
	TopBarSizer->AddGrowableCol(0);
	ToolSizer = new wxFlexGridSizer(0, 10, 0, 0);
	TopBarSizer->Add(ToolSizer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 3);
	StaticText1 = new wxStaticText(PreviewGLPanel, ID_STATICTEXT1, _("Preview:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	wxFont StaticText1Font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	if ( !StaticText1Font.Ok() ) StaticText1Font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	StaticText1Font.SetWeight(wxFONTWEIGHT_BOLD);
	StaticText1->SetFont(StaticText1Font);
	TopBarSizer->Add(StaticText1, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 40);
	ChoiceLayoutGroups = new wxChoice(PreviewGLPanel, ID_CHOICE_PREVIEWS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PREVIEWS"));
	TopBarSizer->Add(ChoiceLayoutGroups, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PreviewGLSizer->Add(TopBarSizer, 1, wxALL|wxALIGN_LEFT, 3);
	LayoutGLSizer = new wxFlexGridSizer(0, 2, 0, 0);
	LayoutGLSizer->AddGrowableCol(0);
	LayoutGLSizer->AddGrowableRow(0);
	ScrollBarLayoutVert = new wxScrollBar(PreviewGLPanel, ID_SCROLLBAR1, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL, wxDefaultValidator, _T("ID_SCROLLBAR1"));
	ScrollBarLayoutVert->SetScrollbar(0, 1, 10, 1);
	LayoutGLSizer->Add(ScrollBarLayoutVert, 1, wxALL|wxEXPAND, 0);
	ScrollBarLayoutHorz = new wxScrollBar(PreviewGLPanel, ID_SCROLLBAR2, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL, wxDefaultValidator, _T("ID_SCROLLBAR2"));
	ScrollBarLayoutHorz->SetScrollbar(0, 1, 10, 1);
	LayoutGLSizer->Add(ScrollBarLayoutHorz, 1, wxALL|wxEXPAND, 0);
	LayoutGLSizer->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	PreviewGLSizer->Add(LayoutGLSizer, 1, wxALL|wxEXPAND, 0);
	PreviewGLPanel->SetSizer(PreviewGLSizer);
	SplitterWindow2->SplitVertically(LeftPanel, PreviewGLPanel);
	FlexGridSizerPreview->Add(SplitterWindow2, 1, wxALL|wxEXPAND, 1);
	SetSizer(FlexGridSizerPreview);

	Connect(ID_NOTEBOOK_OBJECTS, wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, (wxObjectEventFunction)&LayoutPanel::OnNotebook_ObjectsPageChanged);
	Connect(ID_CHECKBOX_3D, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&LayoutPanel::OnCheckBox_3DClick);
	Connect(ID_CHECKBOXOVERLAP, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&LayoutPanel::OnCheckBoxOverlapClick);
	Connect(ID_CHECKBOXSHOWNAMES, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&LayoutPanel::OnCheckBoxShowNamesClick);
	Connect(ID_CHECKBOXSHOWINFO, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&LayoutPanel::OnCheckBoxShowInfoClick);
	Connect(ID_BUTTON_SAVE_PREVIEW, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LayoutPanel::OnButtonSavePreviewClick);
	Connect(ID_CHOICE_PREVIEWS, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&LayoutPanel::OnChoiceLayoutGroupsSelect);
	Connect(ID_SPLITTERWINDOW2, wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED, (wxObjectEventFunction)&LayoutPanel::OnSplitterWindowSashPosChanged);
	//*)

    ScrollBarLayoutHorz->Hide();
    ScrollBarLayoutVert->Hide();

    spdlog::debug("LayoutPanel basic setup complete");
    modelPreview = new ModelPreview( (wxPanel*) PreviewGLPanel, xlights, true, 0, false, true);
    LayoutGLSizer->Insert(0, modelPreview, 1, wxALL | wxEXPAND, 0);
    PreviewGLSizer->Fit(PreviewGLPanel);
    PreviewGLSizer->SetSizeHints(PreviewGLPanel);
    FlexGridSizerPreview->Fit(this);
    FlexGridSizerPreview->SetSizeHints(this);
    spdlog::debug("LayoutPanel ModelPreview created");

    modelPreview->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&LayoutPanel::OnPreviewLeftDown, nullptr,this);
    modelPreview->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&LayoutPanel::OnPreviewLeftUp, nullptr,this);
    modelPreview->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&LayoutPanel::OnPreviewRightDown, nullptr,this);
    modelPreview->Connect(wxEVT_MOTION,(wxObjectEventFunction)&LayoutPanel::OnPreviewMouseMove, nullptr,this);
    modelPreview->Connect(wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&LayoutPanel::OnPreviewMouseLeave, nullptr, this);
    modelPreview->Connect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)&LayoutPanel::OnPreviewLeftDClick, nullptr, this);
    modelPreview->Connect(wxEVT_MOUSEWHEEL, (wxObjectEventFunction)&LayoutPanel::OnPreviewMouseWheel, nullptr, this);
    modelPreview->Connect(wxEVT_MIDDLE_DOWN, (wxObjectEventFunction)&LayoutPanel::OnPreviewMouseWheelDown, nullptr, this);
    modelPreview->Connect(wxEVT_MIDDLE_UP, (wxObjectEventFunction)&LayoutPanel::OnPreviewMouseWheelUp, nullptr, this);
    modelPreview->Connect(wxEVT_MAGNIFY, (wxObjectEventFunction)&LayoutPanel::OnPreviewMagnify, nullptr, this);

    if (modelPreview->EnableTouchEvents(wxTOUCH_ROTATE_GESTURE | wxTOUCH_ZOOM_GESTURE)) {
        modelPreview->Connect(wxEVT_GESTURE_ROTATE, (wxObjectEventFunction)&LayoutPanel::OnPreviewRotateGesture, nullptr, this);
        modelPreview->Connect(wxEVT_GESTURE_ZOOM, (wxObjectEventFunction)&LayoutPanel::OnPreviewZoomGesture, nullptr, this);
    }
    modelPreview->Connect(EVT_MOTION3D, (wxObjectEventFunction)&LayoutPanel::OnPreviewMotion3D, nullptr, this);
    modelPreview->Connect(EVT_MOTION3D_BUTTONCLICKED, (wxObjectEventFunction)&LayoutPanel::OnPreviewMotion3DButtonEvent, nullptr, this);

    propertyEditor = new xlPropertyGrid(ModelSplitter,
                                        wxID_ANY, // id
                                        wxDefaultPosition, // position
                                        wxDefaultSize, // size
                                        // Here are just some of the supported window styles
                                        //wxPG_AUTO_SORT | // Automatic sorting after items added
                                        wxPG_SPLITTER_AUTO_CENTER | // Automatically center splitter until user manually adjusts it
                                        // Default style
                                        wxPG_DEFAULT_STYLE);
    propertyEditor->SetExtraStyle(wxWS_EX_PROCESS_IDLE | wxPG_EX_HELP_AS_TOOLTIPS);
    LayoutUtils::CreateImageList(m_imageList);

    wxFlexGridSizer* FlexGridSizerModels = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizerModels->AddGrowableCol(0);
	FlexGridSizerModels->AddGrowableRow(0);
	wxPanel* new_panel = new wxPanel(PanelModels, wxNewId(), wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_OBJECTS_PANEL"));
	FlexGridSizerModels->Add(new_panel, 1, wxALL|wxEXPAND, 0);
    PanelModels->SetSizer(FlexGridSizerModels);
    wxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* filterSizer = new wxBoxSizer(wxHORIZONTAL);
    ModelFilterCtrl = new wxSearchCtrl(new_panel, ID_TEXTCTRL_MODEL_FILTER,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    ModelFilterCtrl->SetDescriptiveText("Filter items...");
    ModelFilterCtrl->ShowCancelButton(true);
    ModelFilterCtrl->Bind(wxEVT_TEXT_ENTER, &LayoutPanel::OnModelFilterTextChanged, this);
    ModelFilterCtrl->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &LayoutPanel::OnModelFilterTextChanged, this);
    ModelFilterCtrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &LayoutPanel::OnModelFilterCancelBtn, this);
    ModelFilterCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent&) {
        _filterString = ModelFilterCtrl->GetValue().Trim();
        _filterRegex.Compile(_filterString, wxRE_ICASE);
        _filterRegexValid = _filterRegex.IsValid();
        if (_filterString.IsEmpty()) {
            UpdateModelList(true);
        }
        });
    filterSizer->Add(ModelFilterCtrl, 1, wxEXPAND | wxTOP, 2);
    sizer1->Add(filterSizer, 0, wxEXPAND);

    TreeListViewModels = CreateTreeListCtrl(wxTL_MULTIPLE, new_panel);
    sizer1->Add(TreeListViewModels, wxSizerFlags(2).Expand());
    new_panel->SetSizer(sizer1);
    sizer1->SetSizeHints(new_panel);

    comparator.SetFrame(xlights);
    TreeListViewModels->SetItemComparator(&comparator);

    ModelSplitter->ReplaceWindow(SecondPanel, propertyEditor);
    SecondPanel->Destroy();

    auto* config = GetXLightsConfig();
    int msp = config->Read("LayoutModelSplitterSash", -1);
    int sp = config->Read("LayoutMainSplitterSash", -1);
    is_3d = config->ReadBool("LayoutMode3D", false);
    bool showNames = config->ReadBool("LayoutShowNames", false);
    bool showInfo = config->ReadBool("LayoutShowStartChannel", false);

    CheckBox_3D->SetValue(is_3d);
    CheckBoxShowNames->SetValue(showNames);
    CheckBoxShowInfo->SetValue(showInfo);
    modelPreview->SetShowModelNames(showNames);
    modelPreview->SetShowModelInfo(showInfo);
    static_cast<ModelPreview*>(xlights->GetHousePreview())->Set3D(is_3d);

    ChoiceLayoutGroups->Enable();
    modelPreview->Set3D(is_3d);

    propertyEditor->Connect(wxEVT_PG_CHANGING, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridChanging,0,this);
    propertyEditor->Connect(wxEVT_PG_CHANGED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridChange,0,this);
    propertyEditor->Connect(wxEVT_PG_SELECTED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridSelection,0,this);
    propertyEditor->Connect(wxEVT_PG_ITEM_COLLAPSED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridItemCollapsed,0,this);
    propertyEditor->Connect(wxEVT_PG_ITEM_EXPANDED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridItemExpanded,0,this);
    propertyEditor->Connect(wxEVT_PG_RIGHT_CLICK, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridRightClick, 0, this);
    propertyEditor->SetValidationFailureBehavior(wxPGVFBFlags::MarkCell | wxPGVFBFlags::Beep);

    spdlog::debug("LayoutPanel property grid created");

    ToolSizer->SetCols(22);
    AddModelButton("Arches", arches);
    AddModelButton("Candy Canes", canes);
    AddModelButton("Channel Block", channelblock_xpm);
    AddModelButton("Circle", circles);
    AddModelButton("Cube", cube_xpm);
    AddModelButton("Custom", custom);
    AddModelButton("DMX", add_dmx_xpm);
    AddModelButton("Icicles", icicles_xpm);
    AddModelButton("Image", image_xpm);
    AddModelButton("Label", label_model_xpm);
    AddModelButton("Matrix", matrix);
    AddModelButton("Poly Line", polyline);
    AddModelButton("Single Line", singleline);
    AddModelButton("Sphere", sphere);
    AddModelButton("Spinner", spinner);
    AddModelButton("Star", star);
    AddModelButton("Tree", tree);
    AddModelButton("Window Frame", frame);
    AddModelButton("Download", download);
    AddModelButton("Import Custom", import);
    obj_button = AddModelButton("Add Object", object);
    obj_button->Enable(is_3d && ChoiceLayoutGroups->GetStringSelection() == "Default");

    spdlog::debug("LayoutPanel model buttons created");

    modelPreview->Connect(wxID_CUT, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoCut, nullptr,this);
    modelPreview->Connect(wxID_COPY, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoCopy, nullptr,this);
    modelPreview->Connect(wxID_PASTE, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoPaste, nullptr,this);
    modelPreview->Connect(wxID_UNDO, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoUndo, nullptr,this);
    modelPreview->Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(LayoutPanel::OnCharHook), nullptr,this);
    modelPreview->Connect(wxID_ANY, wxEVT_CHAR, wxKeyEventHandler(LayoutPanel::OnChar), nullptr,this);

    TreeListViewModels->GetView()->Connect(wxID_CUT, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoCut, nullptr,this);
    TreeListViewModels->GetView()->Connect(wxID_COPY, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoCopy, nullptr,this);
    TreeListViewModels->GetView()->Connect(wxID_PASTE, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoPaste, nullptr,this);
    TreeListViewModels->GetView()->Connect(wxID_UNDO, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoUndo, nullptr,this);
    TreeListViewModels->GetView()->Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(LayoutPanel::OnListCharHook), nullptr, this);
    wxScrolledWindow *sw = new wxScrolledWindow(ModelSplitter);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sw->SetSizer(sizer);
    model_grp_panel = new ModelGroupPanel(sw, xlights->AllModels, this);
    sizer->Add( model_grp_panel, 1, wxEXPAND | wxALL, 1 );
    sw->SetScrollRate(5,5);
    sw->Hide();

    spdlog::debug("LayoutPanel model group panel created");

    // Setup the Object List Panel
    //wxScrolledWindow *sw2 = new wxScrolledWindow(ModelSplitter);
    wxBoxSizer* sizer2 = new wxBoxSizer(wxVERTICAL);
    //->SetSizer(sizer2);
    objects_panel = new ViewObjectPanel(PanelObjects, xlights->AllObjects, this);
    sizer2->Add( objects_panel, 1, wxEXPAND | wxALL, 1 );
    sizer2->SetSizeHints(objects_panel);
    PanelObjects->SetSizer(sizer2);
    sizer1->SetSizeHints(PanelObjects);
    //sw2->SetScrollRate(5,5);
    //sw2->Hide();
    //ViewObjectWindow = sw2;
    spdlog::debug("LayoutPanel object panel created");

    LeftPanelSizer->Fit(LeftPanel);
    LeftPanelSizer->SetSizeHints(LeftPanel);
    FlexGridSizerPreview->Fit(this);
    FlexGridSizerPreview->SetSizeHints(this);

    ModelGroupWindow = sw;

    if( !is_3d ) {
        Notebook_Objects->RemovePage(1);
    }

    if (sp != -1) {
        SplitterWindow2->SetSashGravity(0.0);
        SplitterWindow2->SetSashPosition(sp);
    }
    // Replace ModelSplitter with an AUI-managed container.
    wxSizer* lps = LeftPanel->GetSizer();

    // Detach the old toolbar row from LeftPanelSizer — controls move to the preview.
    {
        wxSizer* fgs3 = CheckBox_3D->GetContainingSizer();
        if (fgs3) {
            fgs3->Detach(CheckBox_3D);
            fgs3->Detach(CheckBoxOverlap);
            fgs3->Detach(CheckBoxShowNames);
            fgs3->Detach(CheckBoxShowInfo);
            lps->Detach(fgs3);
            delete fgs3;
        }
        lps->Detach(ButtonSavePreview);
    }

    ModelPanelContainer = new wxPanel(LeftPanel, wxID_ANY);
    lps->Replace(ModelSplitter, ModelPanelContainer);
    lps->Layout();

    FirstPanel->Reparent(ModelPanelContainer);
    propertyEditor->Reparent(ModelPanelContainer);
    ModelGroupWindow->Reparent(ModelPanelContainer);

    ModelSplitter->Destroy();
    ModelSplitter = nullptr;

    LayoutAuiManager* new_layout_mgr = new LayoutAuiManager(ModelPanelContainer, wxAUI_MGR_ALLOW_FLOATING | wxAUI_MGR_DEFAULT);
    new_layout_mgr->m_onPaneStateChanged = [this]() {
        UpdateLayoutSplitter();
    };
    layout_mgr = new_layout_mgr;
    new_layout_mgr->Bind(wxEVT_AUI_PANE_CLOSE, &LayoutPanel::OnLayoutPaneClose, this);

    FirstPanel->SetMinSize(wxSize(0, kPaneMinHeight));
    int listHeight = (msp > 0) ? msp : kListHeightFallback;
    layout_mgr->AddPane(FirstPanel, wxAuiPaneInfo()
        .Name("ModelList")
        .Caption("Groups/Models List")
        .CaptionVisible(true)
        .CloseButton(false)
        .Floatable(true)
        .Dockable(true)
        .TopDockable(true)
        .BottomDockable(true)
        .LeftDockable(false)
        .RightDockable(false)
        .Top().Layer(0).Row(0)
        .BestSize(-1, listHeight)
        .FloatingSize(600, 1000)
        .MinSize(300, kPaneMinHeight));

    propertyEditor->SetMinSize(wxSize(0, kPaneMinHeight));
    layout_mgr->AddPane(propertyEditor, wxAuiPaneInfo()
        .Name("ModelSettings")
        .Caption("Background Properties")
        .CaptionVisible(true)
        .CloseButton(false)
        .Floatable(true)
        .TopDockable(false)
        .BottomDockable(false)
        .LeftDockable(false)
        .RightDockable(false)
        .Center()
        .FloatingSize(600, 1000)
        .MinSize(0, kPaneMinHeight));

    layout_mgr->AddPane(ModelGroupWindow, wxAuiPaneInfo()
        .Name("ModelGroupSettings")
        .Caption("Group Settings")
        .CaptionVisible(true)
        .CloseButton(false)
        .Floatable(true)
        .TopDockable(false)
        .BottomDockable(false)
        .LeftDockable(false)
        .RightDockable(false)
        .Center()
        .FloatingSize(600, 1000)
        .MinSize(-1, 50)
        .Hide());

    wxString auiPerspective;
    config->Read("LayoutAUIPerspective2", &auiPerspective, wxEmptyString);
    if (!auiPerspective.empty()) {
        layout_mgr->LoadPerspective(auiPerspective);
    }
    // Always reapply settings that LoadPerspective overwrites via SafeSet()
    layout_mgr->GetPane("ModelList").MinSize(300, kPaneMinHeight).CaptionVisible(true).Caption("Groups/Models List")
        .Floatable(true).CloseButton(false).TopDockable(true).BottomDockable(true).LeftDockable(false).RightDockable(false);
    layout_mgr->GetPane("ModelSettings").MinSize(0, kPaneMinHeight).CaptionVisible(true).Caption("Background Properties")
        .Floatable(true).CloseButton(false).TopDockable(false).BottomDockable(false).LeftDockable(false).RightDockable(false);
    layout_mgr->GetPane("ModelGroupSettings").CaptionVisible(true).Caption("Group Settings")
        .CloseButton(false).TopDockable(false).BottomDockable(false).LeftDockable(false).RightDockable(false);
    // Always start with the two main panels docked and visible regardless of
    // what was saved.  If the saved perspective had them floating or hidden
    // (e.g. closed while on another tab, or crashed), force them back to
    // docked+shown.  ModelGroupSettings stays hidden — it is only shown by
    // ShowPropGrid(false) when a model group is selected.
    layout_mgr->GetPane("ModelList").Top().Dock().Show();
    layout_mgr->GetPane("ModelSettings").Center().Dock().Show();
    {
        wxAuiPaneInfo& mgp = layout_mgr->GetPane("ModelGroupSettings");
        if (mgp.IsOk()) mgp.Center().Dock().Hide(); // always force-hide on startup
    }
    layout_mgr->Update();
    // Enable splitter auto-collapse / expand logic now that AUI is fully set up.
    _auiInitialized = true;
    UpdateLayoutSplitter();

    // Move 3D / Overlap / Save controls to a bar at the bottom-center of the
    // layout preview canvas, below the GL canvas.
    wxPanel* layoutControlsBar = new wxPanel(PreviewGLPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    CheckBox_3D->Reparent(layoutControlsBar);
    CheckBoxOverlap->Reparent(layoutControlsBar);
    CheckBoxShowNames->Reparent(layoutControlsBar);
    CheckBoxShowInfo->Reparent(layoutControlsBar);
    ButtonSavePreview->Reparent(layoutControlsBar);
    {
        wxBoxSizer* lcbSizer = new wxBoxSizer(wxHORIZONTAL);
        lcbSizer->AddStretchSpacer(1);
        lcbSizer->Add(CheckBox_3D, 0, wxALL|wxALIGN_CENTER_VERTICAL, 8);
        lcbSizer->Add(CheckBoxOverlap, 0, wxALL|wxALIGN_CENTER_VERTICAL, 8);
        lcbSizer->Add(CheckBoxShowNames, 0, wxALL|wxALIGN_CENTER_VERTICAL, 8);
        lcbSizer->Add(CheckBoxShowInfo, 0, wxALL|wxALIGN_CENTER_VERTICAL, 8);
        lcbSizer->Add(ButtonSavePreview, 0, wxALL|wxALIGN_CENTER_VERTICAL, 8);
        lcbSizer->AddStretchSpacer(1);
        layoutControlsBar->SetSizer(lcbSizer);
        layoutControlsBar->SetMinSize(wxSize(-1, 68));
    }
    PreviewGLPanel->GetSizer()->Add(layoutControlsBar, 0, wxEXPAND | wxALIGN_BOTTOM, 3);
    PreviewGLPanel->Layout();

    TreeListViewModels->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);
    TreeListViewModels->SetColumnWidth(1, TreeListViewModels->WidthFor(CHNUMWIDTH));
    TreeListViewModels->SetColumnWidth(2, TreeListViewModels->WidthFor(CHNUMWIDTH));
    TreeListViewModels->SetColumnWidth(3, wxCOL_WIDTH_AUTOSIZE);

}

wxTreeListCtrl* LayoutPanel::CreateTreeListCtrl(long style, wxPanel* panel)
{
    wxTreeListCtrl* const
        tree = new wxTreeListCtrl(panel, ID_TREELISTVIEW_MODELS,
                                  wxDefaultPosition, wxDefaultSize,
                                  style, "ID_TREELISTVIEW_MODELS");
    TreeListMiewInternalModel = tree->GetDataView()->GetModel();
    TreeListMiewInternalModel->IncRef();
    tree->SetImages(m_imageList);

    tree->AppendColumn(MODELCOLNAME,
                       wxCOL_WIDTH_AUTOSIZE,
                       wxALIGN_LEFT,
                       wxCOL_RESIZABLE | wxCOL_SORTABLE);

    // Because you cant programmatically reorder the columns we have to add them in the right order
    auto* config = GetXLightsConfig();
    auto colOrder = config->Read("LayoutModelListCols", "");

    int sortcol = 0;
    bool sortasc = true;

    wxArrayString cols;
    if (colOrder != "") {
        cols = wxSplit(colOrder, ',');
        int cc = 1;
        for (int i = 0; i < (int)cols.size(); i++) {
            if (cols[i] != "") {
                if (cols[i][0] == 'U') {
                    sortcol = cc++;
                    sortasc = true;
                }
                else if (cols[i][0] == 'D') {
                    sortcol = cc++;
                    sortasc = false;
                }
                else if (cols[i][0] == ' ') {
                    cc++;
                }
                cols[i] = cols[i].substr(1);
            }
        }

        // If cols are missing ... add them in
        if (std::find(begin(cols), end(cols), STARTCHANCOLNAME) ==  end(cols)) cols.push_back(STARTCHANCOLNAME);
        if (std::find(begin(cols), end(cols), ENDCHANCOLNAME) == end(cols)) cols.push_back(ENDCHANCOLNAME);
        if (std::find(begin(cols), end(cols), CONTCONNCOLNAME) == end(cols)) cols.push_back(CONTCONNCOLNAME);
    }
    else {
        cols.push_back(STARTCHANCOLNAME);
        cols.push_back(ENDCHANCOLNAME);
        cols.push_back(CONTCONNCOLNAME);
    }

    int i = 1;
    for (const auto& c : cols) {
        if (c == STARTCHANCOLNAME) {
            tree->AppendColumn(STARTCHANCOLNAME,
                tree->WidthFor(CHNUMWIDTH),
                wxALIGN_LEFT,
                wxCOL_RESIZABLE | wxCOL_SORTABLE | wxCOL_REORDERABLE);
            Col_StartChan = i++;
        }
        else if (c == ENDCHANCOLNAME) {
            tree->AppendColumn(ENDCHANCOLNAME,
                tree->WidthFor(CHNUMWIDTH),
                wxALIGN_LEFT,
                wxCOL_RESIZABLE | wxCOL_SORTABLE | wxCOL_REORDERABLE);
            Col_EndChan = i++;
        }
        else if (c == CONTCONNCOLNAME) {
            tree->AppendColumn(CONTCONNCOLNAME,
                wxCOL_WIDTH_AUTOSIZE,
                wxALIGN_LEFT,
                wxCOL_RESIZABLE | wxCOL_SORTABLE | wxCOL_REORDERABLE);
            Col_ControllerConnection = i++;
        }
    }

    tree->SetSortColumn(sortcol, sortasc);
    return tree;
}

void LayoutPanel::Reset()
{
    selectedBaseObject = nullptr;
    highlightedBaseObject = nullptr;
    SetCurrentLayoutGroup(xlights->GetStoredLayoutGroup());
    ChoiceLayoutGroups->Clear();
    ChoiceLayoutGroups->Append("Default");
    ChoiceLayoutGroups->Append("All Models");
    ChoiceLayoutGroups->Append("Unassigned");
    ChoiceLayoutGroups->SetToolTip("Select a preview or model group to display in the Layout Preview window. Choose 'Default' for models assigned to the default preview, 'All Models' to show all models, or a specific group to filter displayed models.");
    for (const auto& [name, grp] : xlights->LayoutGroups) {
        ChoiceLayoutGroups->Append(name);
    }
    ChoiceLayoutGroups->Append("<Create New Preview>");
    ChoiceLayoutGroups->SetSelection(0);
    for( int i = 0; i < (int)ChoiceLayoutGroups->GetCount(); i++ )
    {
        if( ChoiceLayoutGroups->GetString(i) == currentLayoutGroup )
        {
            ChoiceLayoutGroups->SetSelection(i);
            break;
        }
    }
    obj_button->Enable(is_3d && ChoiceLayoutGroups->GetStringSelection() == "Default");
}

void LayoutPanel::SetDirtyHiLight(bool dirty) {
    if (dirty) {
        xlights->UnsavedRgbEffectsChanges = true;
    }
//    if (dirty)
//    {
//        xlights->RebuildControllerConfig(xlights->GetOutputManager(), &xlights->AllModels);
//    }
    xlights->UpdateLayoutSave();
    xlights->UpdateControllerSave();
}

std::string LayoutPanel::GetCurrentPreview() const
{
    return ChoiceLayoutGroups->GetStringSelection().ToStdString();
}

NewModelBitmapButton* LayoutPanel::AddModelButton(const std::string &type, const char *data[]) {

    wxImage image(data);
    int iconSize = PreviewGLPanel->FromDIP(xlights->ToolIconSize());
    NewModelBitmapButton *button = new NewModelBitmapButton(PreviewGLPanel, image, iconSize, type);
    ToolSizer->Add(button, 1, wxALL, 0);
    buttons.push_back(button);
    Connect(button->GetId(), wxEVT_BUTTON, (wxObjectEventFunction)&LayoutPanel::OnNewModelTypeButtonClicked);
    return button;
}

void LayoutPanel::UpdateModelButtonSizes() {
    int iconSize = PreviewGLPanel->FromDIP(xlights->ToolIconSize());
    for (auto *btn : buttons) {
        btn->UpdateIconSize(iconSize);
    }
    ToolSizer->Layout();
    TopBarSizer->Layout();
    PreviewGLSizer->Layout();
}

int LayoutPanel::GetColumnIndex(const std::string& name) const
{
    for (size_t i = 0; i < TreeListViewModels->GetColumnCount(); i++) {
        auto col = TreeListViewModels->GetDataView()->GetColumn(i);
        if (col->GetTitle() == name) return i;
    }
    wxASSERT(false);
    return -1;
}

void LayoutPanel::SaveModelsListColumns()
{
    wxString colOrder;
    for (size_t i = 0; i < TreeListViewModels->GetColumnCount(); i++) {
        for (int j = 0; j < (int)TreeListViewModels->GetColumnCount(); j++) {
            auto col = TreeListViewModels->GetDataView()->GetColumn(j);
            auto p = TreeListViewModels->GetDataView()->GetColumnPosition(col);
            if (p == (int)i && col->GetTitle() != MODELCOLNAME) {
                if (col->IsSortKey()) {
                    if (col->IsSortOrderAscending()) {
                        colOrder += "U";
                    }
                    else {
                        colOrder += "D";
                    }
                }
                else {
                    colOrder += " ";
                }
                colOrder += col->GetTitle() + ",";
            }
        }
    }

    auto* config = GetXLightsConfig();
    config->Write("LayoutModelListCols", colOrder);
}

void LayoutPanel::SaveLayoutPerspective()
{
    if (layout_mgr == nullptr) {
        return;
    }
    // If panels were hidden by HideFloatingPanes() (tab-switch stash), restore
    // their info so we can inspect and dock them below.  Pass false to skip
    // the UI update — the frame may be tearing down and rendering could fail.
    if (!_savedFloatingPerspective.empty()) {
        layout_mgr->LoadPerspective(_savedFloatingPerspective, false);
        _savedFloatingPerspective.clear();
    }
    // Dock any floating panels before saving so the perspective always
    // starts with panels docked on the next launch.  GetAllPanes() returns
    // a reference to the internal m_panes array; mutations here are read
    // directly by SavePerspective() without needing Update() (which would
    // try to render to a partially-destroyed window).
    wxAuiPaneInfoArray& panes = layout_mgr->GetAllPanes();
    for (size_t i = 0; i < panes.GetCount(); i++) {
        if (panes[i].IsFloating()) {
            if (panes[i].name == "ModelList") {
                panes[i].Top().Dock();
            } else {
                panes[i].Center().Dock();
            }
        }
    }
    auto* config = GetXLightsConfig();
    auto perspective = layout_mgr->SavePerspective();
    config->Write("LayoutAUIPerspective2", perspective);
}

LayoutPanel::~LayoutPanel()
{
    if (layout_mgr != nullptr) {
        layout_mgr->UnInit();
        delete layout_mgr;
        layout_mgr = nullptr;
    }

    SaveModelsListColumns();
    if (background != nullptr) {
        delete background;
    }
    TreeListViewModels->SetItemComparator(nullptr);
    TreeListViewModels->DeleteAllItems();
    TreeListMiewInternalModel->DecRef();

	//(*Destroy(LayoutPanel)
	//*)
}

void LayoutPanel::OnPropertyGridChange(wxPropertyGridEvent& event) {
    wxString name = event.GetPropertyName();
    updatingProperty = true;
    if (name == "BkgBrightness") {
        if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
            xlights->SetPreviewBackgroundBrightness(event.GetValue().GetLong(), previewBackgroundAlpha);
        }
        else {
            pGrp->SetBackgroundBrightness(event.GetValue().GetLong(), previewBackgroundAlpha);
            modelPreview->SetBackgroundBrightness(event.GetValue().GetLong(), previewBackgroundAlpha);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "LayoutPanel::OnPropertyGridChange::Brightness");
        }
    }
    else if (name == "BkgTransparency") {
        if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
            xlights->SetPreviewBackgroundBrightness(previewBackgroundBrightness, 100 - event.GetValue().GetLong());
        }
        else {
            pGrp->SetBackgroundBrightness(previewBackgroundBrightness, 100 - event.GetValue().GetLong());
            modelPreview->SetBackgroundBrightness(previewBackgroundBrightness, 100 - event.GetValue().GetLong());
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "LayoutPanel::OnPropertyGridChange::BkgTransparency");
        }
    }
    else if (name == "BkgSizeWidth") {
        xlights->SetPreviewSize(event.GetValue().GetLong(), modelPreview->GetVirtualCanvasHeight());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "LayoutPanel::OnPropertyGridChange::BkgSizeWidth");
    }
    else if (name == "BkgSizeHeight") {
        xlights->SetPreviewSize(modelPreview->GetVirtualCanvasWidth(), event.GetValue().GetLong());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "LayoutPanel::OnPropertyGridChange::BkgSizeHeight");
    }
    else if (name == "BoundingBox") {
        modelPreview->SetDisplay2DBoundingBox(event.GetValue().GetBool());
        xlights->SetDisplay2DBoundingBox(event.GetValue().GetBool());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "LayoutPanel::OnPropertyGridChange::BoundingBox");
    }
    else if (name == "2DGrid") {
        modelPreview->SetDisplay2DGrid(event.GetValue().GetBool(), xlights->GetDisplay2DGridSpacing());
        xlights->SetDisplay2DGrid(event.GetValue().GetBool());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "LayoutPanel::OnPropertyGridChange::2DGrid");
    }
    else if (name == "2DGridSpacing") {
        modelPreview->SetDisplay2DGrid(xlights->GetDisplay2DGrid(), event.GetValue().GetLong());
        xlights->SetDisplay2DGridSpacing(event.GetValue().GetLong());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "LayoutPanel::OnPropertyGridChange::2DGridSpacing");
    } else if (name == "2DXZeroIsCenter") {
        modelPreview->SetDisplay2DCenter0(event.GetValue().GetBool());
        xlights->SetDisplay2DCenter0(event.GetValue().GetBool());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "LayoutPanel::OnPropertyGridChange::2DXZeroIsCenter");
    } else if (name == "BkgImage") {
        if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
            xlights->SetPreviewBackgroundImage(event.GetValue().GetString());
        }
        else {
            pGrp->SetBackgroundImage(event.GetValue().GetString());
            modelPreview->SetbackgroundImage(event.GetValue().GetString());
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "LayoutPanel::OnPropertyGridChange::BkgImage");
        }
    }
    else if (name == "BkgFill") {
        if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
            xlights->SetPreviewBackgroundScaled(event.GetValue().GetBool());
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "LayoutPanel::OnPropertyGridChange::BkgFill");
        } else {
            pGrp->SetBackgroundScaled(wxAtoi(event.GetValue().GetString()) > 0);
            modelPreview->SetScaleBackgroundImage(wxAtoi(event.GetValue().GetString()) > 0);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "LayoutPanel::OnPropertyGridChange::BkgFill");
        }
    }
    else {
        if (editing_models) {
            xlights->AbortRender();
            // Guard against a dangling selectedBaseObject. A modal dialog that
            // was cancelled mid-edit (ModelStateDialog / SubModelsDialog with an
            // active node-selection in progress, escape pressed before the
            // selection is released) can free the model the cache pointed at
            // without notifying us. The plain != nullptr check below would then
            // pass and the dynamic_cast / SaveDisplayDimensions call that
            // follows would crash on freed memory.
            if (selectedBaseObject != nullptr && !IsSelectedBaseObjectValid()) {
                spdlog::warn("LayoutPanel::OnPropertyGridChange: selectedBaseObject was stale; clearing cached selection.");
                selectedBaseObject = nullptr;
                highlightedBaseObject = nullptr;
                _propertyAdapter.reset();
                updatingProperty = false;
                return;
            }
            if (selectedBaseObject != nullptr) {
                Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
                //model property
                if ("ModelName" == name) {
                    std::string safename = Model::SafeModelName(event.GetValue().GetString().ToStdString());

                    if (safename != event.GetValue().GetString().ToStdString())
                    {
                        // need to update the property grid with the modified name
                        wxPGProperty* prop = propertyEditor->GetPropertyByName("ModelName");
                        if (prop != nullptr) {
                            prop->SetValue(safename);
                        }
                    }
                    std::string oldname = selectedModel->name;
                    if (oldname != safename) {
                        RenameModelInTree(selectedModel, safename);
                        selectedBaseObject = nullptr;
                        xlights->RenameModel(oldname, safename);
                        if (oldname == lastModelName) {
                            lastModelName = safename;
                        }
                    }
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                                  OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                                  OutputModelManager::WORK_CALCULATE_START_CHANNELS, "LayoutPanel::OnPropertyGridChange::ModelName", nullptr, nullptr, safename);
                }
                else {
                    if (selectedModel != nullptr) {
                        selectedModel->SaveDisplayDimensions();
                        int i = _propertyAdapter->OnPropertyGridChange(propertyEditor, event);
                        if ((i & GRIDCHANGE_SUPPRESS_HOLDSIZE) == 0 &&
                            (dynamic_cast<ModelWithScreenLocation<BoxedScreenLocation>*>(selectedModel) != nullptr ||
                                dynamic_cast<ModelWithScreenLocation<ThreePointScreenLocation>*>(selectedModel) != nullptr)) {
                            // only restore if not suppressed and if it is a boxed screen location
                            xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::OnPropertyGridChange", selectedModel);
                            selectedModel->RestoreDisplayDimensions();
                        }
                        wxASSERT(i == 0 || i == GRIDCHANGE_SUPPRESS_HOLDSIZE);
                    }
                }
            }
        }
        else {
            objects_panel->OnPropertyGridChange(propertyEditor, event);
        }
    }
    updatingProperty = false;
}

void LayoutPanel::SetDisplay2DBoundingBox(bool bb)
{
    modelPreview->SetDisplay2DBoundingBox(bb);
}

void LayoutPanel::SetDisplay2DGridSpacing(bool bb, long spacing)
{
    modelPreview->SetDisplay2DGrid(bb, spacing);
}

void LayoutPanel::SetDisplay2DCenter0(bool bb) {
    modelPreview->SetDisplay2DCenter0(bb);
}

void LayoutPanel::OnPropertyGridChanging(wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    xlights->AddTraceMessage("LayoutPanel::OnPropertyGridChanging  Property: " + name);
    // Same dangling-pointer guard as OnPropertyGridChange - see IsSelectedBaseObjectValid.
    if (selectedBaseObject != nullptr && !IsSelectedBaseObjectValid()) {
        spdlog::warn("LayoutPanel::OnPropertyGridChanging: selectedBaseObject was stale; clearing cached selection.");
        selectedBaseObject = nullptr;
        highlightedBaseObject = nullptr;
        _propertyAdapter.reset();
        return;
    }
    if (selectedBaseObject != nullptr) {
        if( editing_models ) {
            xlights->AbortRender();
            Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
            if (selectedModel == nullptr) {
                spdlog::warn("LayoutPanel::OnPropertyGridChanging: selectedBaseObject is not a Model; ignoring property change.");
                return;
            }
            if ("ModelName" == name) {
                std::string safename = Model::SafeModelName(event.GetValue().GetString().ToStdString());
                // refuse clashing names or names with unsafe characters
                if (xlights->AllModels[safename] != nullptr || safename != event.GetValue().GetString().ToStdString()) {
                    CreateUndoPoint("ModelName", selectedModel->name, safename);
                    event.Veto();
                }
                // todo do I need to do anything special here
            //} else if ("SubModels" == name) {
                // ignore the submodel changes for now.
            //    int a = 0;
            } else {
                CreateUndoPoint("ModelProperty", selectedModel->name, name, event.GetProperty()->GetValue().GetString().ToStdString());
                _propertyAdapter->OnPropertyGridChanging(propertyEditor, event);
            }
        } else {
            ViewObject* selectedObject = dynamic_cast<ViewObject*>(selectedBaseObject);
            if ("ObjectName" == name) {
                std::string safename = Model::SafeModelName(event.GetValue().GetString().ToStdString());
                // refuse clashing names or names with unsafe characters
                if (xlights->AllObjects[safename] != nullptr || safename != event.GetValue().GetString().ToStdString()) {
                    CreateUndoPoint("ObjectName", selectedObject->name, safename);
                    event.Veto();
                }
            } else {
                CreateUndoPoint("ObjectProperty", selectedObject->name, name, event.GetProperty()->GetValue().GetString().ToStdString());
                //objects_panel->GetSelectedObject()->OnPropertyGridChanging(propertyEditor, event);
            }
        }
    } else {
        CreateUndoPoint("Background", "", name, event.GetProperty()->GetValue().GetString().ToStdString());
    }
}

void LayoutPanel::OnPropertyGridSelection(wxPropertyGridEvent& event) {
    if (selectedBaseObject != nullptr) {
        if( editing_models ) {
            Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
            if( selectedModel != nullptr && selectedModel->GetDisplayAs() == DisplayAsType::PolyLine ) {
                int segment = _propertyAdapter->OnPropertyGridSelection(propertyEditor, event);
                selectedModel->GetBaseObjectScreenLocation().SelectSegment(segment);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPropertyGridSelection");
            }
        }
    }
}

void LayoutPanel::OnPropertyGridItemCollapsed(wxPropertyGridEvent& event) {
    if (selectedBaseObject != nullptr) {
        if( editing_models ) {
            Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
            if( selectedModel != nullptr && selectedModel->GetDisplayAs() == DisplayAsType::PolyLine ) {
                _propertyAdapter->OnPropertyGridItemCollapsed(propertyEditor, event);
            }
        }
    }
}

void LayoutPanel::OnPropertyGridItemExpanded(wxPropertyGridEvent& event) {
    if (selectedBaseObject != nullptr) {
        if( editing_models ) {
            Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
            if( selectedModel != nullptr && selectedModel->GetDisplayAs() == DisplayAsType::PolyLine ) {
                _propertyAdapter->OnPropertyGridItemExpanded(propertyEditor, event);
            }
        }
    }
}

void LayoutPanel::OnPropertyGridRightClick(wxPropertyGridEvent& event)
{
    if (selectedBaseObject != nullptr) {
        wxMenu mnu;
        if (_propertyAdapter) {
            _propertyAdapter->HandlePropertyGridRightClick(event, mnu);
        }
        if (mnu.GetMenuItemCount() != 0) {
            mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridContextMenu, nullptr, this);
            PopupMenu(&mnu);
        }
    }
}

void LayoutPanel::OnPropertyGridContextMenu(wxCommandEvent& event)
{
    if (selectedBaseObject != nullptr) {
        if (_propertyAdapter) {
            _propertyAdapter->HandlePropertyGridContextMenu(event);
        }
    }
}

void LayoutPanel::RefreshLayout()
{
    std::string selectedBaseObjectName = "";
    if (selectedBaseObject != nullptr) selectedBaseObjectName = selectedBaseObject->name;
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::RefreshLayout", nullptr, nullptr, selectedBaseObjectName);
    ShowPropGrid(true);
}

void LayoutPanel::RenderLayout()
{
    if (modelPreview == nullptr || !modelPreview->StartDrawing(mPointSize)) return;

    modelPreview->Render();
    if (m_creating_bound_rect)
    {
        if (!is_3d) {
            modelPreview->AddBoundingBoxToAccumulator(m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y);
        } else {
            modelPreview->AddScreenSpaceBoundingBoxToAccumulator(m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y);
        }
    }
    modelPreview->EndDrawing();
}

void LayoutPanel::UpdatePreview()
{
    spdlog::debug("        UpdatePreview.");
    SetDirtyHiLight(xlights->UnsavedRgbEffectsChanges);
    RenderLayout();
}

void LayoutPanel::updatePropertyGrid()
{
    if (selectedBaseObject == nullptr || propertyEditor == nullptr || ModelsSelectedCount() > 1) return;

    if (_propertyAdapter) {
        _propertyAdapter->UpdateProperties(propertyEditor, xlights->GetOutputManager());
    } else if (_viewObjectAdapter) {
        _viewObjectAdapter->UpdateProperties(propertyEditor, xlights->GetOutputManager());
    }
}

void LayoutPanel::ClearSelectedModelGroup()
{
    if (model_grp_panel != nullptr) model_grp_panel->UpdatePanel("");
}

void LayoutPanel::resetPropertyGrid() {
    spdlog::debug("        resetPropertyGrid.");

    if (selectedBaseObject != nullptr && selectedBaseObject->GetDisplayAs() == DisplayAsType::ModelGroup)
    {
        ShowPropGrid(false);
    }
    else
    {
        ShowPropGrid(true);
        propertyEditor->Freeze();
        auto save = propertyEditor->SaveEditableState();
        //auto scroll = propertyEditor->GetScrollPos(wxVERTICAL);
        //auto top = propertyEditor->GetItemAtY(0);
        wxString selProp = "";
        if (propertyEditor->GetSelection() != nullptr)
        {
            selProp = propertyEditor->GetSelection()->GetName();
        }
        clearPropGrid();
        if (selectedBaseObject != nullptr) {
            SetupPropGrid(selectedBaseObject);
            propertyEditor->RestoreEditableState(save);
            if (selProp != "")
            {
                auto p = propertyEditor->GetPropertyByName(selProp);
                if (p != nullptr) propertyEditor->EnsureVisible(p);
            }
            else
            {
                //propertyEditor->SetScrollPos(wxVERTICAL, scroll - 1, true);
                //wxScrollWinEvent e(wxEVT_SCROLLWIN_THUMBRELEASE);
                //e.SetOrientation(wxVERTICAL);
                //e.SetPosition(scroll);
                //propertyEditor->HandleOnScroll(e);
            }
        }
        propertyEditor->Thaw();
    }
}

void LayoutPanel::clearPropGrid() {
    // remember last selected item
    if (propertyEditor->GetSelection() != nullptr) {
        _lastSelProp = propertyEditor->GetSelection()->GetName();
    }
    propertyEditor->UnfocusEditor();
    propertyEditor->ClearSelection();

    wxPGProperty *p = propertyEditor->GetPropertyByName("ModelAppearance");
    if (p != nullptr) {
        appearanceVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("ModelSize");
    if (p != nullptr) {
        sizeVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("Dimensions");
    if (p != nullptr) {
        dimensionsVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("ModelStringProperties");
    if (p != nullptr) {
        stringPropsVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("ModelControllerConnectionProperties");
    if (p != nullptr) {
        controllerConnectionVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("Layers");
    if (p != nullptr) {
        layersVisible = propertyEditor->IsPropertyExpanded(p);
    }
    propertyEditor->Clear();
}

void LayoutPanel::refreshObjectList() {
    spdlog::debug("        refreshObjectList.");
    objects_panel->refreshObjectList();
}

std::string LayoutPanel::TreeModelName(const Model* model, bool fullname)
{
    std::string name = fullname ? model->GetFullName() : model->name;
    if (model->IsActive())
    {
        return name;
    }
    else
    {
        return "<" + name + ">";
    }
}

void LayoutPanel::FreezeTreeListView() {
    TreeListViewModels->Freeze();

    //turn off the column width auto-resize.  Makes it REALLY slow to populate the tree
    TreeListViewModels->SetColumnWidth(0, TreeListViewModels->GetColumnWidth(0));
    TreeListViewModels->SetColumnWidth(3, TreeListViewModels->GetColumnWidth(3));
    treeSorted = TreeListViewModels->GetSortColumn(&treeSortCol, &treeSortAscending);
    
    //turn off the sorting as that is ALSO really slow
    TreeListViewModels->SetItemComparator(nullptr);
    if (treeSorted) {
        //UnsetAsSortKey may be unimplemented on all  platforms so we'll set a
        //sort column to 0 which is faster due to straight string compare
        TreeListViewModels->SetSortColumn(0, true);
        //then turn it off again so platforms that DO support this can benefit
        TreeListViewModels->GetDataView()->GetSortingColumn()->UnsetAsSortKey();
    }
#ifdef __WXOSX__
    // dis-associate the model so the adds/removes will be a ton faster
    TreeListViewModels->GetDataView()->AssociateModel(nullptr);
#endif
}

void LayoutPanel::ThawTreeListView(const std::list<wxTreeListItem> &toExpand) {
#ifdef __WXOSX__
    // re-associate the model
    TreeListViewModels->GetDataView()->AssociateModel(TreeListMiewInternalModel);
#endif

    // Only set the column sizes the very first time we load it
    if (_firstTreeLoad) {
        _firstTreeLoad = false;

        TreeListViewModels->SetColumnWidth(1, wxCOL_WIDTH_AUTOSIZE);
        int width = TreeListViewModels->GetColumnWidth(1);
        if (width < 20) {
            width = TreeListViewModels->WidthFor(STARTCHANCOLNAME);
        }
        TreeListViewModels->SetColumnWidth(1, width);

        TreeListViewModels->SetColumnWidth(2, wxCOL_WIDTH_AUTOSIZE);
        width = TreeListViewModels->GetColumnWidth(2);
        if (width < 20) {
            width = TreeListViewModels->WidthFor(STARTCHANCOLNAME);
        }
        TreeListViewModels->SetColumnWidth(2, width);
    }
    //turn the sorting back on
    TreeListViewModels->SetItemComparator(&comparator);
    if (treeSorted) {
#ifdef __WXOSX__
        // if the sort direction doesn't acutally change from previous setting,
        // it won't actually sort for some reason so we'll double toggle to make sure
        TreeListViewModels->SetSortColumn(treeSortCol, !treeSortAscending);
#endif
        TreeListViewModels->SetSortColumn(treeSortCol, treeSortAscending);
        TreeListViewModels->GetDataView()->GetModel()->Resort();
    }
    
    for (auto &i : toExpand) {
        TreeListViewModels->Expand(i);
    }
    TreeListViewModels->Thaw();
    TreeListViewModels->Refresh();

    TreeListViewModels->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);
    {
        int w = TreeListViewModels->GetColumnWidth(0);
        if (w < 20) {
            w = TreeListViewModels->GetClientSize().GetWidth() / 3;
        }
        if (w < 20) {
            w = 150;
        }
        TreeListViewModels->SetColumnWidth(0, w);
    }
    TreeListViewModels->SetColumnWidth(3, wxCOL_WIDTH_AUTOSIZE);
    {
        int w = TreeListViewModels->GetColumnWidth(3);
        if (w < 20) {
            w = TreeListViewModels->WidthFor(CONTCONNCOLNAME);
        }
        if (w < 20) {
            w = 100;
        }
        TreeListViewModels->SetColumnWidth(3, w);
    }
}

void LayoutPanel::SetTreeListViewItemText(wxTreeListItem &item, int col, const wxString &txt) {
    wxString orig = TreeListViewModels->GetItemText(item, col);
    if (orig != txt) {
        TreeListViewModels->SetItemText(item, col, txt);
    }
}

void LayoutPanel::refreshModelList() {

    
    spdlog::debug("        refreshModelList.");
    wxStopWatch sw;

    std::list<wxTreeListItem> toExpand;
    FreezeTreeListView();

    for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
          item.IsOk();
          item = TreeListViewModels->GetNextItem(item) )
    {
        ModelTreeData *data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
        Model *model = data != nullptr ? data->GetModel() : nullptr;

        if (model != nullptr ) {

            if( model->GetDisplayAs() != DisplayAsType::ModelGroup ) {
                wxString cv = TreeListViewModels->GetItemText(item, Col_StartChan);
                wxString startStr = model->GetStartChannelInDisplayFormat(xlights->GetOutputManager());
                if (cv != startStr) {
                    data->startingChannel = model->GetNumberFromChannelString(model->ModelStartChannel);
                    if (model->GetDisplayAs() == DisplayAsType::SubModel || (model->CouldComputeStartChannel && model->IsValidStartChannelString())) {
                        SetTreeListViewItemText(item, Col_StartChan, startStr);
                    } else {
                        SetTreeListViewItemText(item, Col_StartChan, "*** " + model->ModelStartChannel);
                    }
                }
                cv = TreeListViewModels->GetItemText(item, Col_EndChan);
                wxString endStr = model->GetLastChannelInStartChannelFormat(xlights->GetOutputManager());
                if (cv != endStr) {
                    data->endingChannel = model->GetLastChannel()+1;

                    SetTreeListViewItemText(item, Col_EndChan, endStr);
                }
                cv = TreeListViewModels->GetItemText(item, Col_ControllerConnection);

                std::string cc = model->GetControllerConnectionRangeString();
                if (cv != cc) {
                    SetTreeListViewItemText(item, Col_ControllerConnection, cc);
                }
            }
        }
    }
    ThawTreeListView(toExpand);

    if (sw.Time() > 500)
        spdlog::debug("        LayoutPanel::refreshModelList took {}ms", sw.Time());
}

void LayoutPanel::RenameModelInTree(Model *model, const std::string& new_name)
{
    for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
          item.IsOk();
          item = TreeListViewModels->GetNextItem(item) ) {
        ModelTreeData *data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
        if (data != nullptr && data->GetModel() == model) {
            if (model->IsActive()) {
                SetTreeListViewItemText(item, 0, new_name);
            } else {
                SetTreeListViewItemText(item, 0, "<" + new_name + ">");
            }
        }
    }
}

int LayoutPanel::AddModelToTree(Model *model, wxTreeListItem* parent, bool expanded,
                                std::list<wxTreeListItem> &toExpand,
                                int nativeOrder, bool fullName) {
    

    if (model == nullptr) {
        spdlog::critical("LayoutPanel::AddModelToTree model is null ... this is going to crash.");
        wxASSERT(false);
    }

    wxTreeListItem item = TreeListViewModels->AppendItem(*parent, TreeModelName(model, fullName),
                                                         LayoutUtils::GetModelTreeIcon(DisplayAsTypeToString(model->GetDisplayAs()), LayoutUtils::GroupMode::Closed),
                                                         LayoutUtils::GetModelTreeIcon(DisplayAsTypeToString(model->GetDisplayAs()), LayoutUtils::GroupMode::Opened),
                                                         new ModelTreeData(model, nativeOrder, fullName));

    if (model->GetDisplayAs() != DisplayAsType::ModelGroup) {
        wxString startStr = model->GetStartChannelInDisplayFormat(xlights->GetOutputManager());
        wxString endStr = ((startStr[0] == '@' && model->HasIndividualStartChannels()) ? "" : model->GetLastChannelInStartChannelFormat(xlights->GetOutputManager()));
        if (model->GetDisplayAs() != DisplayAsType::SubModel) {
            if ((model->CouldComputeStartChannel || startStr[0] == '@') && model->IsValidStartChannelString()) {
                SetTreeListViewItemText(item, Col_StartChan, startStr);
            } else {
                SetTreeListViewItemText(item, Col_StartChan, "*** " + model->ModelStartChannel);
            }
            SetTreeListViewItemText(item, Col_EndChan, endStr);
        }
        std::string cc = model->GetControllerConnectionRangeString();
        SetTreeListViewItemText(item, Col_ControllerConnection, cc);
    }

    for (int x = 0; x < model->GetNumSubModels(); x++) {
        AddModelToTree(model->GetSubModel(x), &item, false, toExpand, x);
    }

    if( model->GetDisplayAs() == DisplayAsType::ModelGroup ) {
        ModelGroup *grp = (ModelGroup*)model;
        int i = 0;
        for (const auto& it : grp->ModelNames()) {
            Model *m = xlights->AllModels[it];
            if (m == nullptr) {
                spdlog::error("Model group {} thought it contained model. '{}' but it didnt. This would have crashed.", grp->GetName(), it);
            } else if (m == grp) {
                spdlog::error("Model group contains itself. '{}'", grp->GetName());
            } else {
                AddModelToTree(m, &item, false, toExpand, i, true);
                i++;
            }
        }
    }
    if (expanded) {
        toExpand.push_back(item);
    }
    return 0;
}

void LayoutPanel::ReloadModelList() {
    UpdateModelList(true);
}

void LayoutPanel::UpdateModelList(bool full_refresh) {
    std::vector<Model *> models;
    UpdateModelList(full_refresh, models);
    objects_panel->UpdateObjectList(full_refresh, currentLayoutGroup);
}

void LayoutPanel::UpdateModelList(bool full_refresh, std::vector<Model*> &models) {

    
    wxStopWatch sw;

    std::list<std::string> expanded;
    std::list<wxTreeListItem> toExpand;
    if (full_refresh) {
        // need to save the "expanded" state prior to freeze as freezing will disconnect the model from the view
        // and the query will fail/always return false
        wxTreeListItem item = TreeListViewModels->GetFirstChild(TreeListViewModels->GetRootItem());
        while (item.IsOk()) {
            if (TreeListViewModels->IsExpanded(item)) {
                expanded.push_back(TreeListViewModels->GetItemText(item));
            }
            item = TreeListViewModels->GetNextSibling(item);
        }
    }
    
    FreezeTreeListView();

    if (full_refresh) {
        UnSelectAllModels();
    }

    std::vector<Model *> dummy_models;

    // Update all the custom previews
    for (const auto& [gname, grp] : xlights->LayoutGroups) {
        dummy_models.clear();
        if (gname == currentLayoutGroup) {
            UpdateModelsForPreview(currentLayoutGroup, grp.get(), models, true);
        } else {
            UpdateModelsForPreview(gname, grp.get(), dummy_models, false);
        }
    }

    // update the Layout tab preview for default options
    if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
        UpdateModelsForPreview(currentLayoutGroup, nullptr, models, true);
    }

    //spdlog::debug("Layout tab preview models updated.");
    xlights->PreviewModels = models;

    if (full_refresh) {
        UnSelectAllModels();
        TreeListViewModels->DeleteAllItems();

        wxTreeListItem root = TreeListViewModels->GetRootItem();
        // add all the model groups
        for (const auto& it : xlights->AllModels) {
            Model *model = it.second;
            if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
                if (currentLayoutGroup == "All Models" || model->GetLayoutGroup() == currentLayoutGroup
                    || (model->GetLayoutGroup() == "All Previews" && currentLayoutGroup != "Unassigned")) {
                    if (!ModelMatchesFilter(model)) continue;
                    bool expand = (std::find(expanded.begin(), expanded.end(), model->GetName()) != expanded.end());
                    AddModelToTree(model, &root, expand, toExpand, 0);
                }
            }
        }

        // add all the models
        for (const auto& it : models) {
            Model *model = it;
            if (model->GetDisplayAs() != DisplayAsType::ModelGroup && model->GetDisplayAs() != DisplayAsType::SubModel) {
                if (!ModelMatchesFilter(model)) continue;
                bool expand = (std::find(expanded.begin(), expanded.end(), model->GetName()) != expanded.end());
                AddModelToTree(model, &root, expand, toExpand, 0);
            }
        }

    }
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::UpdateModelList");

    ThawTreeListView(toExpand);

    if (sw.Time() > 500)
        spdlog::debug("        LayoutPanel::UpdateModelList took {}ms", sw.Time());
}

void LayoutPanel::UpdateModelsForPreview(const std::string &group, LayoutGroup* layout_grp, std::vector<Model *> &prev_models, bool filtering)
{
    
    wxStopWatch sw;
    //spdlog::debug("Updated models for preview: {}.", (const char*)group.c_str());

    std::set<std::string> modelsAdded;

    for (const auto& it : xlights->AllModels) {
        Model *model = it.second;
        if (model->GetDisplayAs() != DisplayAsType::ModelGroup) {
            if (group == "All Models" ||
                model->GetLayoutGroup() == group ||
                (model->GetLayoutGroup() == "All Previews" && group != "Unassigned")) {
                prev_models.push_back(model);
                modelsAdded.insert(model->name);
            }
        }
    }

    // add in any models that were not in preview but belong to a group that is in the preview
    wxArrayString selectedGroupNames;

    if (selectedTreeGroups.size() > 0 && filtering) {
        for (int i = 0; i < (int)selectedTreeGroups.size(); i++) {
            wxString selectedName = TreeListViewModels->GetItemText(selectedTreeGroups[i]);
            selectedGroupNames.Add(selectedName);
        }
    }

    for (const auto& it : xlights->AllModels) {
        Model *model = it.second;
        bool mark_selected = false;
        if (selectedGroupNames.size() > 0 && filtering && (selectedGroupNames.Index(model->name) != -1)) {
            mark_selected = true;
        }
        if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
            ModelGroup *grp = (ModelGroup*)(model);
            if (group == "All Models" ||
                model->GetLayoutGroup() == group ||
                (model->GetLayoutGroup() == "All Previews" && group != "Unassigned")) {
                for (const auto& it2 : grp->ModelNames()) {
                    Model *m = xlights->AllModels[it2];
                    if (m != nullptr) {
                        if (mark_selected) {
                            if (selectedBaseObject == nullptr)
                            {
                                // This code here used to choose the first model in the group
                                // which was great if you wanted to align to that model but it causes
                                // problems when trying to display the selected submodels.
                                // Given the model selected to the user was essentially random it is
                                // of limited value (and they can always shift click to select a particular
                                // model for alignment so i am removing this and setting it to the group
                                // itself
                                SelectModel(model, false);
                            }
                            // DP - should not need these
                            // m->GroupSelected = true;
                            // m->Highlighted = true;
                        }
                        if (m->GetDisplayAs() == DisplayAsType::SubModel) {
                            if (mark_selected) {
                              //  prev_models.push_back(m);  // setting this causes exception when prev_models render finds a submodel
                            }
                        }
                        else if (m->GetDisplayAs() == DisplayAsType::ModelGroup) {
                            ModelGroup *mg = (ModelGroup*)m;
                            if (mark_selected) {
                                for (const auto& it3 : mg->Models()) {
                                    if (it3->GetDisplayAs() != DisplayAsType::ModelGroup) {
                                        if (selectedBaseObject == nullptr)
                                        {
                                            SelectModel(it3, false);
                                        }
                                        // DP - shouldn't need these
                                        // it3->GroupSelected = true;
                                        // it3->Highlighted = true;
                                        prev_models.push_back(it3);
                                    }
                                    else
                                    {
                                        // need to process groups within groups ... safely
                                        for (const auto& itm : xlights->AllModels)
                                        {
                                            if (itm.second->GetDisplayAs() == DisplayAsType::ModelGroup)
                                            {
                                                // ignore these
                                            }
                                            else
                                            {
                                                if (dynamic_cast<ModelGroup*>(it3)->ContainsModel(itm.second))
                                                {
                                                    if (std::find(prev_models.begin(), prev_models.end(), itm.second) == prev_models.end())
                                                    {
                                                        if (modelsAdded.find(itm.first) == modelsAdded.end()) {
                                                            modelsAdded.insert(itm.first);
                                                        }
                                                        prev_models.push_back(itm.second);
                                                    }
                                                    // DP - shouldn't need this
                                                    // itm.second->GroupSelected = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (modelsAdded.find(it2) == modelsAdded.end()) {
                            modelsAdded.insert(it2);
                            prev_models.push_back(m);
                        }
                    }
                }
            }
        }
    }

    // only run this for layout group previews
    if (layout_grp != nullptr) {
        layout_grp->SetModels(prev_models);
        ModelPreview* preview = layout_grp->GetModelPreview();
        if (layout_grp->GetPreviewCreated()) {
            preview->SetActiveLayoutGroup(layout_grp->GetName());
            if (preview->GetActive()) {
                preview->Refresh();
                preview->Update();
            }
        }
    }

    if (sw.Time() > 500)
        spdlog::debug("        LayoutPanel::UpdateModelsForPreview took {}ms", sw.Time());
}

void LayoutPanel::BulkEditDimmingCurves()
{
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();
    ModelDimmingCurveDialog dlg(this);
    std::map<std::string, std::map<std::string, std::string>> dimmingInfo;
    // Get the first non-empty dimming curve from selected models
    for (Model* model : modelsToEdit) {
        if (model != nullptr) {
            dimmingInfo = model->GetDimmingInfo();
            if (!dimmingInfo.empty()) {
                break;
            }
        }
    }
    // If no dimming curve found, use default values
    if (dimmingInfo.empty()) {
        dimmingInfo["all"]["gamma"] = "1.0";
        dimmingInfo["all"]["brightness"] = "0";
    }
    dlg.Init(dimmingInfo);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK) {
        dimmingInfo.clear();
        dlg.Update(dimmingInfo);
        // Remember selected tree models and paths for reselect
        std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();
        // Apply the dimming curve to all selected models
        for (Model* model : modelsToEdit) {
            if (model != nullptr) {
                model->SetDimmingInfo(dimmingInfo);
                model->IncrementChangeCount();
            }
        }
        // If we don't do these as ImmediateWork then the Model Tree is still frozen and models don't get reselected after refresh
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                      OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BulkEditDimmingCurves");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditDimmingCurves");
        ReselectTreeModels(selectedModelPaths);
    }
}

void LayoutPanel::BulkEditControllerConnection(int id)
{
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();

    // get the first controller connection
    Model *m = nullptr;

    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            std::string protocol = model->GetControllerProtocol();
            if (protocol != "") {
                m = model;
                break;
            }
        }
    }

    controller_connection_bulkedit ccbe = controller_connection_bulkedit::CEBE_CONTROLLERCONNECTION;
    if (id == ID_PREVIEW_BULKEDIT_CONTROLLERDIRECTION) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERDIRECTION;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERSTARTNULLNODES) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERSTARTNULLNODES;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERENDNULLNODES) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERENDNULLNODES;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERGAMMA) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERGAMMA;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERBRIGHTNESS) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERBRIGHTNESS;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERCOLOURORDER) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERCOLOURORDER;
    }
    else if (id == ID_PREVIEW_BULKEDIT_SMARTREMOTE) {
        ccbe = controller_connection_bulkedit::CEBE_SMARTREMOTE;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERGROUPCOUNT) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERGROUPCOUNT;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERPROTOCOL) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERPROTOCOL;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTIONINCREMENT) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERCONNECTIONINCREMENT;
    }
    else if (id == ID_PREVIEW_BULKEDIT_SMARTREMOTETYPE) {
        ccbe = controller_connection_bulkedit::CEBE_SMARTREMOTETYPE;
    }

    ControllerConnectionDialog dlg(this, ccbe);
    dlg.Set(m);
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK) {
        // remember selected tree models and paths for reselect
        std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

        for (Model* model: modelsToEdit) {
            if (model != nullptr) {
                dlg.Get(model->GetCtrlConn(), model->GetNumPhysicalStrings());
            }
        }

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                      OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnectionDialog::Get");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ControllerConnectionDialog::Get");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditControllerConnection");

        ReselectTreeModels(selectedModelPaths);
    }
}

void LayoutPanel::BulkEditActive(bool active)
{
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();

    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            std::string preChangeName = TreeModelName(model, false);
            model->SetActive(active);
            std::string postChangeName = TreeModelName(model, false);

            // fix name for reselect
            for (auto& path : selectedModelPaths) {
                if (path.back() == preChangeName) {
                    path.pop_back();
                    path.push_back(postChangeName);
                }
            }
        }
    }

    for (const auto& it : xlights->AllObjects) {

        if (it.second->Selected() || it.second->GroupSelected()) {
            it.second->SetActive(active);
        }

        ViewObject* view_object = it.second;

        if (selectedBaseObject == nullptr) {
            SelectBaseObject(view_object);
        }
    }

    if (selectedBaseObject != nullptr) {
        selectedBaseObject->SetActive(active);
    }

    if (editing_models) {
        // If we don't clear selection and bulk edits were made with models nested in groups selected then as part of the work
        // the model is also selected in the root of the tree even though it wasn't before.  I could not find any issues with
        // subsequent bulk edits/move/show wiring/export/etc but if this going to be a problem we can also force it to be unselected
        // in tree/preview after. BulkEditDimming was the only one this issue didn't appear and after the work there OMM selectedmodel is "".
        xlights->GetOutputModelManager()->ClearSelectedModel();
    }

    xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditActive");

    // reselect all the models
    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::BulkEditControllerName()
{
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();

    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    std::string name = "";
    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            name = model->GetControllerName();
            if (name != "") {
                break;
            }
        }
    }

    wxArrayString cn;
    int sel = 0;
    int i = 2;
    cn.push_back("");
    cn.push_back(NO_CONTROLLER);
    if (name == NO_CONTROLLER)
        sel = 1;
    for (const auto& it : xlights->GetOutputManager()->GetAutoLayoutControllerNames())
    {
        if (it == name) sel = i;
        cn.push_back(it);
        i++;
    }
    wxSingleChoiceDialog dlg(this, "Choose the controller name", "Controller Name", cn);
    dlg.SetSelection(sel);
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK) {
        name = dlg.GetStringSelection();

        for (Model* model : modelsToEdit) {
            model->SetControllerName(name, true); // skip work since we will command it below
        }

        std::string sm = xlights->GetOutputModelManager()->GetSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "BulkEditControllerName");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "BulkEditControllerName");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                      OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BulkEditControllerName");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_UPDATE_PROPERTYGRID, "BulkEditControllerName");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_MODELLIST, "BulkEditControllerName");
        // if we don't do this then we get stuck in a reselect loop on msw/linux in tree
        xlights->GetOutputModelManager()->ForceSelectedModel(sm);

        ReselectTreeModels(selectedModelPaths);
    }
}

void LayoutPanel::BulkEditPixelSize() {
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();
    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    int size = 0;
    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            size = std::max(model->GetPixelSize(), size);
        }
    }
    wxNumberEntryDialog dlg(this, "Choose the pixel size",  "Pixel Size:", "Pixel Size", size, 1, 500);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK) {
        size = dlg.GetValue();
        for (Model* model: modelsToEdit) {
            model->SetPixelSize(size);
        }

        // see comment in BulkEditActive()
        xlights->GetOutputModelManager()->ClearSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditPixelSize");
        // reselect all the models
        ReselectTreeModels(selectedModelPaths);
    }
}

void LayoutPanel::BulkEditPixelStyle() {
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();
    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    int style = 3;
    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            style = std::min(model->GetTransparency(), style);
        }
    }

    static const char *PIXEL_STYLES_VALUES[] = {"Square", "Smooth", "Solid Circle", "Blended Circle"};
    static wxArrayString PIXEL_STYLES(4, PIXEL_STYLES_VALUES);
    wxSingleChoiceDialog dlg(this, "Choose the Pixel Style",  "Pixel Style", PIXEL_STYLES);
    dlg.SetSelection(style);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK) {
        style = dlg.GetSelection();
        for (Model* model: modelsToEdit) {
            model->SetPixelStyle((Model::PIXEL_STYLE)style);
        }

        // see comment in BulkEditActive()
        xlights->GetOutputModelManager()->ClearSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditPixelStyle");
        // reselect all the models
        ReselectTreeModels(selectedModelPaths);
    }
}

void LayoutPanel::BulkEditTransparency() {
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();
    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    int trans = 100;
    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            trans = std::min(model->GetTransparency(), trans);
        }
    }
    wxNumberEntryDialog dlg(this, "Choose the transparency",  "Transparency:", "Transparency", trans, 0, 100);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK) {
        trans = dlg.GetValue();
        for (Model* model: modelsToEdit) {
            model->SetTransparency(trans);
        }

        // see comment in BulkEditActive()
        xlights->GetOutputModelManager()->ClearSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditTransparency");
        // reselect all the models
        ReselectTreeModels(selectedModelPaths);
    }
}

void LayoutPanel::BulkEditBlackTranparency() {
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();
    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    int trans = 100;
    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            trans = std::min(model->GetBlackTransparency(), trans);
        }
    }
    wxNumberEntryDialog dlg(this, "Choose the black transparency",  "Black Transparency:", "Black Transparency", trans, 0, 100);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK) {
        trans = dlg.GetValue();
        for (Model* model: modelsToEdit) {
            model->SetBlackTransparency(trans);
        }

        // see comment in BulkEditActive()
        xlights->GetOutputModelManager()->ClearSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditBlackTranparency");
        // reselect all the models
        ReselectTreeModels(selectedModelPaths);
    }
}

void LayoutPanel::BulkEditShadowModelFor()
{
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();
    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    wxArrayString choices;
    choices.Add("");

    for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
        if (modelPreview->GetModels()[i]->GetName() != selectedBaseObject->GetName()) {
            choices.Add(modelPreview->GetModels()[i]->GetName());
        }
    }

    wxSingleChoiceDialog dlg(this, "", "Select the model to shadow.", choices);
    dlg.SetSelection(0);
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK) {
        for (Model* model : modelsToEdit) {
            model->SetShadowModelFor(dlg.GetStringSelection());
        }

        // see comment in BulkEditActive()
        xlights->GetOutputModelManager()->ClearSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditBlackShadowModelFor");
        // reselect all the models
        ReselectTreeModels(selectedModelPaths);
    }
}

void LayoutPanel::BulkEditTagColour()
{
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();
    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    xlColor xlColour = xlBLACK;
    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            xlColour = model->GetTagColour();
            if (xlColour != xlBLACK) {
                break;
            }
        }
    }

    auto const& [res, color] = xlColourData::INSTANCE.ShowColorDialog(this, xlColorToWxColour(xlColour));
    if (res == wxID_OK) {
        xlColour = wxColourToXlColor(color);

        for (Model* model: modelsToEdit) {
            model->SetTagColour(xlColour);
        }

        // see comment in BulkEditActive()
        xlights->GetOutputModelManager()->ClearSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditTagColour");

        // reselect all the models
        ReselectTreeModels(selectedModelPaths);
    }
}

void LayoutPanel::BulkEditGroupTagColor()
{
    auto const& [res, color] = xlColourData::INSTANCE.ShowColorDialog(this, *wxBLACK);
    if (res == wxID_OK) {
        xlColor xlC = wxColourToXlColor(color);

        for (const auto& item : selectedTreeGroups) {
            Model* model = GetModelFromTreeItem(item);
            model->SetTagColour(xlC);
        }
    }
}

void LayoutPanel::BulkEditControllerPreview()
{
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();

    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    // get the first preview
    std::string p = "";
    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            p = model->GetLayoutGroup();
            if (p != "") break;
        }
    }

    auto groupList = Model::GetLayoutGroups(xlights->AllModels);
    wxArrayString choices;
    for (const auto& g : groupList) choices.push_back(g);
    int sel = 0;
    int j = 0;
    for (const auto& it : groupList)
    {
        if (it == p) {
            sel = j;
            break;
        }
        j++;
    }
    wxSingleChoiceDialog dlg(this, "Preview", "Preview", choices);
    dlg.SetSelection(sel);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK)
    {
        for (Model* model : modelsToEdit) {
            if (model != nullptr) {
                model->SetLayoutGroup(dlg.GetStringSelection().ToStdString());
            }
        }

        // see comment in BulkEditActive()
        xlights->GetOutputModelManager()->ClearSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditControllerPreview");

        // reselect all the models
        ReselectTreeModels(selectedModelPaths);

        RenderLayout();
    }
}

void LayoutPanel::BulkEditGroupControllerPreview() {
    auto groupList = Model::GetLayoutGroups(xlights->AllModels);
    wxArrayString choices;
    for (const auto& g : groupList) choices.push_back(g);
    int sel = 0;
    wxSingleChoiceDialog dlg(this, "Preview", "Preview", choices);
    dlg.SetSelection(sel);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK) {
        for (const auto& item : selectedTreeGroups) {
            Model* model = GetModelFromTreeItem(item);
            model->SetLayoutGroup(dlg.GetStringSelection().ToStdString());
        }
        xlights->GetOutputModelManager()->ClearSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditGroupControllerPreview");
        RenderLayout();
    }
}

void LayoutPanel::CreateModelGroupFromSelected()
{
    wxTextEntryDialog dlg(this, "Enter name for new group", "Enter name for new group");
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK) {
        wxString name = wxString(Model::SafeModelName(dlg.GetValue().ToStdString()));
        while (xlights->AllModels.GetModel(name.ToStdString()) != nullptr || name.IsEmpty()) {
            wxTextEntryDialog dlg2(this, "Model of name '" + name + "' already exists. Enter name for new group", "Enter name for new group");
            OptimiseDialogPosition(&dlg2);
            if (dlg2.ShowModal() == wxID_OK) {
                name = wxString(Model::SafeModelName(dlg2.GetValue().ToStdString()));
            }
            else {
                return;
            }
        }

        wxArrayString newGroupModels;

        // save selections so they can be added after create/reload
        // add selected groups
        for (const auto& group : selectedTreeGroups) {
            newGroupModels.Add(TreeListViewModels->GetItemText(group));
        }

        // add selected models
        for (const auto& model : selectedTreeModels) {
            newGroupModels.Add(TreeListViewModels->GetItemText(model));
        }

        // add selected submodels
        for (const auto& submodel : selectedTreeSubModels) {
            ModelTreeData *submodelData = (ModelTreeData*)TreeListViewModels->GetItemData(submodel);
            Model* subModel = ((submodelData != nullptr) ? submodelData->GetModel() : nullptr);

            if (subModel != nullptr) {
                newGroupModels.Add(subModel->GetFullName());
            }
        }

        wxString grp = currentLayoutGroup == "All Models" ? "Unassigned" : currentLayoutGroup;

        // Create the model group directly using setters
        ModelGroup* newGroup = new ModelGroup(xlights->AllModels);
        newGroup->SetName(name.ToStdString());
        newGroup->SetLayout("minimalGrid");
        newGroup->SetGridSize(400);
        newGroup->SetLayoutGroup(grp.ToStdString());

        // create group and reload before adding selected models. prior models were added before create and I was seeing frequent
        // crashes in Render() with invalid model pointers especially with mixed selections (groups, submodels & models)
        xlights->AllModels.AddModel(newGroup);
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "CreateModelGroupFromSelected");

        // now add the group models to the already created group
        std::vector<std::string> modelsList;
        for (const auto& m : newGroupModels) {
            modelsList.push_back(m.ToStdString());
        }
        newGroup->SetModels(modelsList);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                      OutputModelManager::WORK_RELOAD_ALLMODELS, "CreateModelGroupFromSelected", nullptr, nullptr, name.ToStdString());

        // we don't needs these, when group is selected after work is finished OnSelectionChange is fired and takes care of these
        //xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "CreateModelGroupFromSelected", nullptr, nullptr, name.ToStdString());
        //xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CreateModelGroupFromSelected", nullptr, nullptr, name.ToStdString());
    }
}

void LayoutPanel::AddSelectedToExistingGroups() {

    wxArrayString selectedModels;

    // add selected groups
    for (const auto& group : selectedTreeGroups) {
        selectedModels.Add(TreeListViewModels->GetItemText(group));
    }

    // add selected models
    for (const auto& model : selectedTreeModels) {
        selectedModels.Add(TreeListViewModels->GetItemText(model));
    }

    // add selected submodels
    for (const auto& submodel : selectedTreeSubModels) {
        ModelTreeData *submodelData = (ModelTreeData*)TreeListViewModels->GetItemData(submodel);
        Model* subModel = ((submodelData != nullptr) ? submodelData->GetModel() : nullptr);

        if (subModel != nullptr) {
            selectedModels.Add(subModel->GetFullName());
        }
    }

    wxArrayString choices;

    for (const auto& it : xlights->AllModels) {
        // Don't add selected groups to choices
        if (it.second->GetDisplayAs() == DisplayAsType::ModelGroup && selectedModels.Index(it.second->GetName()) == -1) {
            choices.Add(it.second->GetName());
        }
    }

    // User has selected all available groups, let them know and bail
    if (choices.size() == 0) {
        std::string userMsg = "You have selected all available groups, there must be at least one group that is not selected and available to add selections to.";
        wxMessageDialog msgDlg(this, userMsg, "No Available Groups To Add To", wxOK | wxCENTRE);
        msgDlg.ShowModal();
        return;
    }

    wxMultiChoiceDialog dlg(this, "Select existing groups to add selections to", "Existing Group", choices);
    OptimiseDialogPosition(&dlg);

    std::string selectgroupName;
    bool reload = false;

    if (dlg.ShowModal() == wxID_OK) {
        xlights->AbortRender();
        for (auto const& idx : dlg.GetSelections()) {
            std::string groupName = choices.at(idx).ToStdString();

            Model* addToGroupModel = xlights->GetModel(groupName);

            if (addToGroupModel != nullptr && addToGroupModel->GetDisplayAs() == DisplayAsType::ModelGroup) {
                ModelGroup* addToGroup = dynamic_cast<ModelGroup*>(addToGroupModel);
                
                if (addToGroup != nullptr) {
                    // Get current model names in the group
                    const std::vector<std::string>& groupModelNames = addToGroup->ModelNames();
                    
                    // Build a new list with existing models plus selected models
                    std::vector<std::string> updatedModelNames(groupModelNames);
                    bool groupChanged = false;
                    
                    for (const auto& selModel : selectedModels) {
                        std::string modelNameStr = selModel.ToStdString();
                        // only add if model doesn't already exist in group
                        if (std::find(updatedModelNames.begin(), updatedModelNames.end(), modelNameStr) == updatedModelNames.end()) {
                            groupChanged = true;
                            updatedModelNames.push_back(modelNameStr);
                        }
                    }

                    if (groupChanged) {
                        // Set the updated model list - this will update the internal state
                        addToGroup->SetModels(updatedModelNames);
                        
                        reload = true;
                        selectgroupName = groupName;
                    }
                }
            }
        }

        if (reload) {
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                          OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                          OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::AddSelectedToExistingGroups", nullptr, nullptr, selectgroupName);
        }
    }
}


void LayoutPanel::RemoveSelectedFromExistingGroups() {

    std::string selectedModel = this->selectedBaseObject->name;
    Model* sourceModel = xlights->GetModel(selectedModel);
    if (sourceModel != nullptr) {
        auto inModelGroups = sourceModel->GetModelManager().GetGroupsContainingModel(sourceModel);
        if (!inModelGroups.empty()) {
            wxArrayString choices;
            for (const auto& it : inModelGroups) {
                choices.Add(it);
            }
            wxMultiChoiceDialog dlg(this, "Select groups to remove model from", "Model in Groups", choices);
            OptimiseDialogPosition(&dlg);

            bool reload = false;
            if (dlg.ShowModal() == wxID_OK) {
                xlights->AbortRender();
                for (auto const& idx : dlg.GetSelections()) {
                    std::string groupName = choices.at(idx).ToStdString();
                    Model* grp = xlights->GetModel(groupName);
                    if (grp != nullptr && grp->GetDisplayAs() == DisplayAsType::ModelGroup) {
                        ModelGroup* modelGroup = dynamic_cast<ModelGroup*>(grp);

                        if (modelGroup != nullptr) {
                            // Get current model names in the group
                            const std::vector<std::string>& groupModelNames = modelGroup->ModelNames();

                            // Build a new list with the selected model removed
                            std::vector<std::string> updatedModelNames;
                            bool groupChanged = false;

                            for (const auto& modelName : groupModelNames) {
                                if (modelName != selectedModel) {
                                    updatedModelNames.push_back(modelName);
                                } else {
                                    groupChanged = true;
                                }
                            }

                            if (groupChanged) {
                                // Set the updated model list - this will update the internal state
                                modelGroup->SetModels(updatedModelNames);
                                reload = true;
                            }
                        }
                    }
                }
            }

            if (reload) {
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                              OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                              OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::RemoveSelectedFromExistingGroups", nullptr, nullptr, selectedModel);
            }
        }
    }
}

class xlImageProperty : public wxImageFileProperty {
public:
    xlImageProperty(const wxString& label,
                    const wxString& name,
                    const wxString& value,
                    const wxImage* img) :
        wxImageFileProperty(label, name, ""), lastFileName(value)
    {
        SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                         ";*.webp"
                                         "|All files (*.*)|*.*");
        SetValueFromString(value);
        if (img != nullptr) {
            setImage(*img);
        }
    }
    virtual ~xlImageProperty() {}

    virtual void OnSetValue() override {
        wxFileProperty::OnSetValue();
        wxFileName fn = GetFileName();
        ObtainAccessToURL(fn.GetFullPath());
        if (fn != lastFileName) {
            lastFileName = fn;
            if (FileExists(fn)) {
                setImage(wxImage(fn.GetFullPath()));
            } else {
                setImage(wxImage());
            }
        }
    }
    
    void setImage(const wxImage &img) {
#if (wxRELEASE_NUMBER > 6) || (wxMINOR_VERSION >= 2)
        m_image = img;
#else
        if (img.IsOk()) {
            m_pImage = new wxImage(img);
        } else {
            delete m_pImage;
            m_pImage = nullptr;
        }
#endif
    }

private:
    wxFileName lastFileName;
};

void LayoutPanel::UnSelectAllModels(bool addBkgProps)
{
    
    wxStopWatch sw;

    highlightedBaseObject = nullptr;
    selectedBaseObject = nullptr;
    _propertyAdapter.reset();
    selectionLatched = false;
    selectedPrimaryTreeItem = nullptr;
    selectedTreeGroups.clear();
    selectedTreeModels.clear();
    selectedTreeSubModels.clear();

    // process all models
    for (const auto& m : modelPreview->GetModels()) {
        if (!xlights->AllModels.IsModelValid(m) && m != _newModel) {
            spdlog::error("Really strange ... unselect all models returned an invalid model pointer");
        }
        else {
            xlights->AddTraceMessage("LayoutPanel::UnSelectAllModels Model " + m->GetName());
            if (m != nullptr) {
                m->Selected(false);
                m->Highlighted(false);
                m->GroupSelected(false);
                m->SelectHandle(-1);
                m->GetBaseObjectScreenLocation().SetActiveHandle(-1);

                for (const auto& sm : m->GetSubModels()) {
                    sm->Selected(false);
                    sm->Highlighted(false);
                    sm->GroupSelected(false);
                }
            }
            else {
                spdlog::error("Really strange ... unselect all models returned a null model pointer");
            }
        }
    }

    // process all view objects
    for (const auto& it : xlights->AllObjects) {
        ViewObject* view_object = it.second;
        xlights->AddTraceMessage("LayoutPanel::UnSelectAllModels Object " + view_object->GetName());
        if (view_object != nullptr) {
            view_object->Selected(false);
            view_object->Highlighted(false);
            view_object->GroupSelected(false);
            view_object->SelectHandle(-1);
            view_object->GetBaseObjectScreenLocation().SetActiveHandle(-1);
        }
        else {
            spdlog::error("Really strange ... unselect all models returned a null view object pointer");
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::UnselectAllModels");

    if (!updatingProperty && addBkgProps) {
        showBackgroundProperties();
    }

    if (sw.Time() > 500)
        spdlog::debug("        LayoutPanel::UnSelectAllModels took {}ms", sw.Time());
}

void LayoutPanel::showBackgroundProperties()
{
    // Ensure ModelSettings is visible with "Background Properties" caption,
    // and ModelGroupSettings is hidden — no model/group is selected.
    if (layout_mgr != nullptr) {
        bool needUpdate = false;
        wxAuiPaneInfo& ms = layout_mgr->GetPane("ModelSettings");
        wxAuiPaneInfo& mgs = layout_mgr->GetPane("ModelGroupSettings");
        if (ms.IsOk()) {
            ms.Caption("Background Properties");
            // If GroupSettings was floating, bring Background Properties up
            // at the same position so the "settings pane" doesn't jump.
            if (mgs.IsOk() && mgs.IsFloating() && mgs.IsShown()) {
                ms.Float()
                  .FloatingPosition(mgs.floating_pos)
                  .FloatingSize(mgs.floating_size);
            }
            if (!ms.IsShown()) { ms.Show(); needUpdate = true; }
            else needUpdate = true; // caption change still needs Update()
        }
        if (mgs.IsOk() && mgs.IsShown()) {
            mgs.Hide();
            needUpdate = true;
        }
        if (needUpdate) {
            layout_mgr->Update();
            // Ensure the splitter is restored if LeftPanel was unsplit while
            // all panes were floating — now that ModelSettings is shown (docked),
            // the left panel needs to be re-split to become visible.
            UpdateLayoutSplitter();
        }
        mPropGridActive = true;
    }

    propertyEditor->Freeze();
    clearPropGrid();

    GetBackgroundImageForSelectedPreview();       // don't need return value....just let it set local variable previewBackgroundFile
    GetBackgroundScaledForSelectedPreview();      // don't need return value....just let it set local variable previewBackgroundScaled
    GetBackgroundBrightnessForSelectedPreview();  // don't need return value....just let it set local variable previewBackgroundBrightness
    GetBackgroundAlphaForSelectedPreview();       // don't need return value....just let it set local variable previewBackgroundBrightness

    if (backgroundFile != previewBackgroundFile) {
        delete background;
        background = nullptr;
    }

    if (background == nullptr) {
        backgroundFile = previewBackgroundFile;
        if (backgroundFile != "" && FileExists(backgroundFile) && wxIsReadable(backgroundFile)) {
            background = new wxImage(backgroundFile);
            if (background->IsOk()) {
                int orientation = GetExifOrientation(backgroundFile);
                if (orientation != 1) { // 1 means no rotation needed
                    *background = ApplyOrientation(*background, orientation);
                }
            }
        }
    }
    wxPGProperty* prop = propertyEditor->Append(new xlImageProperty("Background Image",
        "BkgImage",
        previewBackgroundFile,
        background));

    propertyEditor->Append(new wxBoolProperty("Fill", "BkgFill", previewBackgroundScaled))->SetAttribute("UseCheckbox", 1);
    if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
        wxPGProperty* prop = propertyEditor->Append(new wxUIntProperty("Width", "BkgSizeWidth", modelPreview->GetVirtualCanvasWidth()));
        prop->SetAttribute("Min", 0);
        prop->SetAttribute("Max", 16384);
        prop->SetEditor("SpinCtrl");
        prop = propertyEditor->Append(new wxUIntProperty("Height", "BkgSizeHeight", modelPreview->GetVirtualCanvasHeight()));
        prop->SetAttribute("Min", 0);
        prop->SetAttribute("Max", 16384);
        prop->SetEditor("SpinCtrl");
    }
    prop = propertyEditor->Append(new wxUIntProperty("Brightness", "BkgBrightness", previewBackgroundBrightness));
    prop->SetAttribute("Min", 0);
    prop->SetAttribute("Max", 100);
    prop->SetEditor("SpinCtrl");

    prop = propertyEditor->Append(new wxUIntProperty("Transparency", "BkgTransparency", 100 - previewBackgroundAlpha));
    prop->SetAttribute("Min", 0);
    prop->SetAttribute("Max", 100);
    prop->SetEditor("SpinCtrl");

    prop = propertyEditor->Append(new wxBoolProperty("2D Bounding Box", "BoundingBox", xlights->GetDisplay2DBoundingBox()));
    prop->SetAttribute("UseCheckbox", true);

    prop = propertyEditor->Append(new wxBoolProperty("2D Grid", "2DGrid", xlights->GetDisplay2DGrid()));
    prop->SetAttribute("UseCheckbox", true);

    prop = propertyEditor->Append(new wxUIntProperty("2D Grid Spacing", "2DGridSpacing", xlights->GetDisplay2DGridSpacing()));
    prop->SetAttribute("Min", 5);
    prop->SetAttribute("Max", 200);
    prop->SetEditor("SpinCtrl");

    prop = propertyEditor->Append(new wxBoolProperty("X0 Is Center", "2DXZeroIsCenter", xlights->GetDisplay2DCenter0()));
    prop->SetAttribute("UseCheckbox", true);

    propertyEditor->Thaw();
}

void LayoutPanel::SelectAllModels()
{
    highlightedBaseObject = nullptr;
    selectedBaseObject = nullptr;
    selectionLatched = false;

    if (editing_models) {
        auto& models = modelPreview->GetModels();
        for (size_t i = 0; i < models.size(); i++)
        {
            Model* m = models[i];

            SelectModelInTree(m);
        }
    }
    else {
        for (const auto& it : xlights->AllObjects) {
            ViewObject* view_object = it.second;

            if (selectedBaseObject == nullptr)
            {
                SelectBaseObject(view_object);
            }

            view_object->Selected(false);
            view_object->Highlighted(true);
            view_object->GroupSelected(true);
            view_object->SelectHandle(-1);
            view_object->GetBaseObjectScreenLocation().SetActiveHandle(-1);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::SelectAllModels");
}

void LayoutPanel::SetupPropGrid(BaseObject *base_object) {

    // 

    if (base_object == nullptr || propertyEditor == nullptr) return;
    if (dynamic_cast<ModelGroup*>(base_object) != nullptr) {
        //groups don't use the property grid
        return;
    }

    if (propertyEditor->GetSelection() != nullptr) {
        _lastSelProp = propertyEditor->GetSelection()->GetName();
    }

    auto frozen = propertyEditor->IsFrozen();
    if (!frozen) propertyEditor->Freeze();
    clearPropGrid();

    // Create a property adapter for the model or view object
    Model* model = dynamic_cast<Model*>(base_object);
    if (model != nullptr) {
        _propertyAdapter = ModelPropertyManager::CreateAdapter(*model);
        _viewObjectAdapter.reset();
    } else if (auto* viewObj = dynamic_cast<ViewObject*>(base_object)) {
        _viewObjectAdapter = ViewObjectPropertyManager::CreateAdapter(*viewObj);
        _propertyAdapter.reset();
    } else {
        _propertyAdapter.reset();
        _viewObjectAdapter.reset();
    }

    //propertyEditor->Enable(true);

    if( editing_models ) {
        auto p = propertyEditor->Append(new wxStringProperty("Name", "ModelName", base_object->name));
        if (dynamic_cast<SubModel*>(base_object) != nullptr) {
            p->ChangeFlag(wxPGFlags::ReadOnly, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("SubModel names cannot be changed here.");
        }
    } else {
        propertyEditor->Append(new wxStringProperty("Name", "ObjectName", base_object->name));
    }

    if (_propertyAdapter) {
        _propertyAdapter->AddProperties(propertyEditor, xlights->GetOutputManager());
    } else if (_viewObjectAdapter) {
        _viewObjectAdapter->AddProperties(propertyEditor, xlights->GetOutputManager());
    }
    if (dynamic_cast<SubModel*>(base_object) == nullptr) {

        wxPGProperty* p2;

        if (RulerObject::GetRuler() != nullptr) {
            p2 = propertyEditor->Append(new wxPropertyCategory("Dimensions", "Dimensions"));

            if (_propertyAdapter) {
                _propertyAdapter->AddDimensionProperties(propertyEditor);
            } else if (_viewObjectAdapter) {
                _viewObjectAdapter->AddDimensionProperties(propertyEditor);
            }
            if (!dimensionsVisible) {
                propertyEditor->Collapse(p2);
            }

            if (p2->GetChildCount() == 0) {
                p2->Hide(true);
            }
        }

        p2 = propertyEditor->Append(new wxPropertyCategory("Size/Location", "ModelSize"));
        if (_propertyAdapter) {
            _propertyAdapter->AddSizeLocationProperties(propertyEditor);
        } else if (_viewObjectAdapter) {
            _viewObjectAdapter->AddSizeLocationProperties(propertyEditor);
        }
        if (!sizeVisible) {
            propertyEditor->Collapse(p2);
        }
        if (!appearanceVisible) {
            wxPGProperty *prop = propertyEditor->GetPropertyByName("ModelAppearance");
            if (prop != nullptr) {
                propertyEditor->Collapse(prop);
            }
        }
        if (!stringPropsVisible) {
            wxPGProperty *prop = propertyEditor->GetPropertyByName("ModelStringProperties");
            if (prop != nullptr) {
                propertyEditor->Collapse(prop);
            }
        }
        if (!controllerConnectionVisible) {
            wxPGProperty *prop = propertyEditor->GetPropertyByName("ModelControllerConnectionProperties");
            if (prop != nullptr) {
                propertyEditor->Collapse(prop);
            }
        }
        if (!layersVisible) {
            wxPGProperty* prop = propertyEditor->GetPropertyByName("Layers");
            if (prop != nullptr) {
                propertyEditor->Collapse(prop);
            }
        }
    }

    if (!frozen) propertyEditor->Thaw();

    if (_lastSelProp != "") {
        auto p = propertyEditor->GetPropertyByName(_lastSelProp);
        if (p != nullptr) propertyEditor->EnsureVisible(p);
    }

    if (base_object->IsFromBase()) {
        propertyEditor->SetToolTip("This model comes from the base folder and its properties cannot be edited.");
        auto it = propertyEditor->GetIterator(wxPG_ITERATE_ALL, nullptr);
        while (!it.AtEnd())
        {
            it.GetProperty()->Enable(false);
            it.Next(true);
        }

    } else {
        propertyEditor->UnsetToolTip();
    }
}

void LayoutPanel::SelectBaseObject3D()
{
    if (is_3d) {
        // latch center handle immediately
        if (selectedBaseObject != nullptr) {
            if (editing_models) {
                Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
                // I think the selected model might not be a model in some undo situations
                if (selectedModel != nullptr) {
                    selectedModel->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                    selectedModel->GetBaseObjectScreenLocation().SetActiveAxis(ModelScreenLocation::MSLAXIS::NO_AXIS);
                }
            }
            else {
                ViewObject* selectedViewObject = dynamic_cast<ViewObject*>(selectedBaseObject);
                // I think the selected model might not be a view object in some undo situations
                if (selectedViewObject != nullptr) {
                    selectedViewObject->GetObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                    selectedViewObject->GetObjectScreenLocation().SetActiveAxis(ModelScreenLocation::MSLAXIS::NO_AXIS);
                }
            }
            highlightedBaseObject = selectedBaseObject;
            selectionLatched = true;
        }
    }
}

void LayoutPanel::SelectBaseObject(const std::string & name, bool highlight_tree)
{
    
    if( editing_models ) {
        Model *m = xlights->AllModels[name];
        if (m == nullptr)
        {
            // Hmmm ... model doesnt exist ... maybe it is the additional model
            if (modelPreview->GetAdditionalModel() != nullptr && modelPreview->GetAdditionalModel()->GetName() == name)
            {
                m = modelPreview->GetAdditionalModel();
            }
            else
            {
                spdlog::warn("LayoutPanel:SelectBaseObject Unable to select model '{}'.", (const char*)name.c_str());
            }
        }
        if (m != nullptr && m != selectedBaseObject)
        {
            SelectModelInTree(m);
        }
    } else {
        ViewObject *v = xlights->AllObjects[name];
        if (v == nullptr)
        {
            spdlog::warn("LayoutPanel:SelectBaseObject Unable to select object '{}'.", (const char *)name.c_str());
        }
        if (v != selectedBaseObject)
        {
            for (const auto& it : xlights->AllObjects) {
                ViewObject* view_object = it.second;
                view_object->Selected(false);
                view_object->Highlighted(false);
                view_object->GroupSelected(false);
                view_object->SelectHandle(-1);
                view_object->GetBaseObjectScreenLocation().SetActiveHandle(-1);
            }
            SelectViewObject(v, highlight_tree);
        }
    }
}

void LayoutPanel::SelectBaseObject(BaseObject *obj, bool highlight_tree)
{
    if( editing_models ) {
        SelectBaseObjectInTree(obj);
    } else {
        SelectViewObject(dynamic_cast<ViewObject*>(obj), highlight_tree);
    }
}

void LayoutPanel::SelectModel(const std::string & name, bool highlight_tree)
{
    
    xlights->AddTraceMessage("LayoutPanel::SelectModel: " + name);
    Model *m = xlights->AllModels[name];
    if (m == nullptr)
    {
        spdlog::warn("LayoutPanel:SelectModel Unable to select model '{}'.", name);
    }
    else {
        SelectModelInTree(m);
    }
}

void LayoutPanel::SelectModelGroupModels(ModelGroup* m, std::list<ModelGroup*>& processed)
{
    processed.push_back(m);
    for (const auto& it : m->Models())
    {
        if (it->GetDisplayAs() == DisplayAsType::ModelGroup)
        {
            if (std::find(processed.begin(), processed.end(), it) == processed.end())
            {
                SelectModelGroupModels(dynamic_cast<ModelGroup*>(it), processed);
            }
        }
        else
        {
            SelectModelInTree(m);
        }
    }
}

void LayoutPanel::SelectModel(Model *m, bool highlight_tree) {
    SelectModelInTree(m);
    //SelectBaseObject3D();
}

void LayoutPanel::SelectViewObject(ViewObject *v, bool highlight_tree) {
    bool changed = false;
    if (v != selectedBaseObject) {
        changed = true;
    }

    modelPreview->SetFocus();

    if (v != nullptr) {
        v->Selected(true);

        if( highlight_tree ) {
            objects_panel->HighlightObject(v);
        }
        if (changed) {
            SetupPropGrid(v);
        }
    } else {
        propertyEditor->Freeze();
        clearPropGrid();
        propertyEditor->Thaw();
    }

    selectedBaseObject = v;
    if (selectedBaseObject != nullptr) {
        selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
        selectionLatched = true;
    }
    SelectBaseObject3D();

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::SelectViewObject");
}

bool LayoutPanel::Is3d() const
{
    return CheckBox_3D->IsChecked();
}

void LayoutPanel::Set3d(bool is3d)
{
    if (CheckBox_3D->IsChecked() != is3d)
    {
        CheckBox_3D->SetValue(is3d);
        wxCommandEvent e;
        OnCheckBox_3DClick(e);
    }
}

std::string LayoutPanel::GetSelectedModelName() const
{
    if (selectedBaseObject == nullptr) return "";
    return selectedBaseObject->GetName();
}

void LayoutPanel::OnCheckBoxOverlapClick(wxCommandEvent& event)
{
    if (CheckBoxOverlap->GetValue() == false) {
        for (wxTreeListItem item = TreeListViewModels->GetFirstItem();
            item.IsOk();
            item = TreeListViewModels->GetNextSibling(item)) {
            ModelTreeData* data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
            Model* model = data != nullptr ? data->GetModel() : nullptr;

            if (model != nullptr) {
                if (model->GetDisplayAs() != DisplayAsType::ModelGroup) {
                    model->Overlapping = false;
                }
            }
        }
    }
}

void LayoutPanel::OnCheckBoxShowNamesClick(wxCommandEvent& event)
{
    bool val = CheckBoxShowNames->GetValue();
    modelPreview->SetShowModelNames(val);
    auto* config = GetXLightsConfig();
    config->Write("LayoutShowNames", val);
}

void LayoutPanel::OnCheckBoxShowInfoClick(wxCommandEvent& event)
{
    bool val = CheckBoxShowInfo->GetValue();
    modelPreview->SetShowModelInfo(val);
    auto* config = GetXLightsConfig();
    config->Write("LayoutShowStartChannel", val);
}

bool LayoutPanel::SaveEffects()
{
    xlights->SaveEffectsFile();
    xlights->SetStatusText(_("Preview layout saved"));
    SetDirtyHiLight(false);

    return true;
}

void LayoutPanel::OnButtonSavePreviewClick(wxCommandEvent& event)
{
    // if we have auto layout make sure everything is up to date ... and update zcpp files
    if (xlights->GetOutputManager()->GetAutoLayoutControllerNames().size() > 0) {
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnButtonSavePreviewClick");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "LayoutPanel::OnButtonSavePreviewClick");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "LayoutPanel::OnButtonSavePreviewClick");
    }

    SaveEffects();
    xlights->SaveNetworksFile();
    xlights->UpdateLayoutSave(); // SaveEffects tried to do this, but if the saves are linked it is marked dirty til nets are saved.
}

int LayoutPanel::ModelListComparator::SortElementsFunction(wxTreeListCtrl* treelist, wxTreeListItem item1, wxTreeListItem item2, unsigned sortColumn)
{
    unsigned col;
    bool ascending;
    treelist->GetSortColumn(&col, &ascending);

    ModelTreeData* data1 = dynamic_cast<ModelTreeData*>(treelist->GetItemData(item1));
    ModelTreeData* data2 = dynamic_cast<ModelTreeData*>(treelist->GetItemData(item2));

    Model* a = data1->GetModel();
    Model* b = data2->GetModel();

    if (a == nullptr || b == nullptr) {
        return 0;
    }

    if (a->GetDisplayAs() == DisplayAsType::ModelGroup) {
        if (b->GetDisplayAs() == DisplayAsType::ModelGroup) {
            return NumberAwareStringCompare(a->name, b->name);
        }
        else {
            if (ascending)
                return -1;
            else
                return 1;
        }
    }
    else if (b->GetDisplayAs() == DisplayAsType::ModelGroup) {
        if (ascending)
            return 1;
        else
            return -1;
    }

    auto colobj = treelist->GetDataView()->GetColumn(sortColumn);

    if (colobj->GetTitle() == STARTCHANCOLNAME) {
        int ia = data1->startingChannel;
        int ib = data2->startingChannel;
        if (ia > ib)
            return 1;
        if (ia < ib)
            return -1;
        return NumberAwareStringCompare(a->name, b->name);
    }
    else if (colobj->GetTitle() == ENDCHANCOLNAME) {
        int ia = data1->endingChannel;
        int ib = data2->endingChannel;
        if (ia > ib)
            return 1;
        if (ia < ib)
            return -1;
        return NumberAwareStringCompare(a->name, b->name);
    }
    else if (colobj->GetTitle() == CONTCONNCOLNAME) {
        int32_t sc;
        // group controllers first
        Output* oa = xlights->GetOutputManager()->GetOutput(data1->startingChannel, sc);
        std::string sna = "";
        if (oa != nullptr) sna = oa->GetSortName();
        Output* ob = xlights->GetOutputManager()->GetOutput(data2->startingChannel, sc);
        std::string snb = "";
        if (ob != nullptr) snb = ob->GetSortName();

        if (sna == snb) {
            // then protocol
            std::string pra = data1->GetModel()->GetControllerProtocol();
            std::string prb = data2->GetModel()->GetControllerProtocol();
            if (pra > prb)
                return 1;
            if (pra < prb)
                return -1;

            // then start port
            int pa = data1->GetModel()->GetControllerPort();
            int pb = data2->GetModel()->GetControllerPort();

            if (pa > pb)
                return 1;
            if (pa < pb)
                return -1;

            // then start channel
            if (data1->startingChannel > data2->startingChannel)
                return 1;
            if (data1->startingChannel < data2->startingChannel)
                return -1;
            return 0;
        }
        else {
            return NumberAwareStringCompare(sna, snb);
        }
    }

    // Dont sort things with parents
    auto parent1 = treelist->GetItemParent(item1);
    auto parent2 = treelist->GetItemParent(item2);
    auto root = treelist->GetRootItem();
    if ((parent1 != root || parent2 != root) && parent1 == parent2) {
        int ia = data1->nativeOrder;
        int ib = data2->nativeOrder;
        if (ia > ib) {
            if (ascending) {
                return 1;
            }
            return -1;
        }
        if (ia < ib) {
            if (ascending) {
                return -1;
            }
            return 1;
        }
    }

    return NumberAwareStringCompare(a->name, b->name);
}

int LayoutPanel::ModelListComparator::Compare(wxTreeListCtrl* treelist, unsigned column, wxTreeListItem first, wxTreeListItem second)
{
    return SortElementsFunction(treelist, first, second, column);
}

void LayoutPanel::GetMouseLocation(int x, int y, glm::vec3& ray_origin, glm::vec3& ray_direction)
{
    VectorMath::ScreenPosToWorldRay(
        x, modelPreview->getHeight() - y,
        modelPreview->getWidth(), modelPreview->getHeight(),
        modelPreview->GetProjViewMatrix(),
        ray_origin,
        ray_direction
    );
}

int LayoutPanel::FindModelsClicked(int x, int y, std::vector<int>& found)
{
    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    GetMouseLocation(x, y, ray_origin, ray_direction);

    for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
        // This should not be necessary but i see enough crashes to think it is worthwhile
        if (xlights->AllModels.IsModelValid(modelPreview->GetModels()[i])) {
            if (modelPreview->GetModels()[i]->IsActive() ) {
                if (modelPreview->GetModels()[i]->HitTest(modelPreview, ray_origin, ray_direction)) {
                    found.push_back(i);
                }
            }
        }
    }
    return found.size();
}

Model* LayoutPanel::SelectSingleModel(int x, int y)
{
    UnSelectAllModelsInTree();
    std::vector<int> found;
    int modelCount = FindModelsClicked(x, y, found);

    if (modelCount == 0) {
        return nullptr;
    }
    else if (modelCount == 1) {
        mHitTestNextSelectModelIndex = 0;
        return modelPreview->GetModels()[found[0]];
    }
    else if (modelCount > 1) {
        for (int i = 0; i < modelCount; i++) {
            if (mHitTestNextSelectModelIndex == i) {
                mHitTestNextSelectModelIndex += 1;
                mHitTestNextSelectModelIndex %= modelCount;
                return modelPreview->GetModels()[found[i]];
            }
        }
    }
    return nullptr;
}

void LayoutPanel::SelectAllInBoundingRect(bool models_and_objects)
{
    if (editing_models || models_and_objects) {
        int count = 0;
        for (const auto& it : modelPreview->GetModels()) {
            if (xlights->AllModels.IsModelValid(it) || it == _newModel) {
                if (it->IsContained(modelPreview, m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y)) {
                    SelectModelInTree(it);
                    count++;
                }
            }
        }
        if (count > 1) showBackgroundProperties();
    }
    if (!editing_models || models_and_objects) {
        for (const auto& it : xlights->AllObjects) {
            ViewObject* view_object = it.second;
            {
                if (view_object->IsContained(modelPreview, m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y)) {
                    // if we dont have a selected model make the first one we find the selected model so alignment etc works
                    if (selectedBaseObject == nullptr) {
                        SelectBaseObject(view_object->GetName(), false);
                    }
                    view_object->GroupSelected(true);
                }
            }
        }
    }

    modelPreview->SetFocus();
}

void LayoutPanel::HighlightAllInBoundingRect(bool models_and_objects)
{
    if (editing_models || models_and_objects) {
        for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
            if (modelPreview->GetModels()[i]->IsContained(modelPreview, m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y)) {
                modelPreview->GetModels()[i]->Highlighted(true);
            } else if (!modelPreview->GetModels()[i]->Selected() &&
                !modelPreview->GetModels()[i]->GroupSelected()) {
                modelPreview->GetModels()[i]->Highlighted(false);
            }
        }
    }
    if (!editing_models || models_and_objects) {
        for (const auto& it : xlights->AllObjects) {
            ViewObject* view_object = it.second;
            if (view_object->GetBaseObjectScreenLocation().IsContained(modelPreview, m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y)) {
                view_object->Highlighted(true);
            } else if (!view_object->Selected() &&
                !view_object->GroupSelected()) {
                view_object->Highlighted(false);
            }
        }
    }
}

bool LayoutPanel::SelectMultipleModels(int x,int y)
{
    std::vector<int> found;
    int modelCount = FindModelsClicked(x, y, found);
    if (modelCount == 0)
        return false;

    propertyEditor->Freeze();
    clearPropGrid();
    propertyEditor->Thaw();
    bool mmWorkRequired = false;
    Model* clickedModel = modelPreview->GetModels()[found[0]];

    if (clickedModel->Selected()) {
        UnSelectModelInTree(clickedModel);
    } else if (clickedModel->GroupSelected()) {
        clickedModel->GroupSelected(false);
        clickedModel->Selected(true);
        if (selectedBaseObject != nullptr) {
            selectedBaseObject->GroupSelected(true);
            selectedBaseObject->Selected(false);
            selectedBaseObject->SelectHandle(-1);
            selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(-1);
        }

        selectedBaseObject = clickedModel;
        highlightedBaseObject = selectedBaseObject;
        selectedBaseObject->SelectHandle(-1);
        selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
        mmWorkRequired = true;
    } else {
        SelectModelInTree(clickedModel);
    }

    if (mmWorkRequired) {
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::SelectMultipleModels");
    }

    return true;
}

void LayoutPanel::SetMouseStateForModels(bool value)
{
    if ( mouse_state_set != value) {
        for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
            modelPreview->GetModels()[i]->GetModelScreenLocation().MouseDown(value);
        }
        mouse_state_set = value;
    }
}

void LayoutPanel::SetSelectedModelToGroupSelected()
{
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++) {
        if (modelPreview->GetModels()[i]->Selected()) {
            modelPreview->GetModels()[i]->Selected(false);
            modelPreview->GetModels()[i]->GroupSelected(true);
        }
    }
}

void LayoutPanel::OnPreviewLeftDClick(wxMouseEvent& event)
{
    // double-click ends polyline creation. This is the
    // touch-friendly equivalent of pressing ESC or RETURN. The
    // second click of a wxEVT_LEFT_DCLICK does NOT fire a separate
    // LEFT_DOWN (wx replaces it with the DCLICK event), so the
    // first click of the double-click placed the final vertex and
    // the DCLICK finalises immediately.
    if (m_polyline_active) {
        FinalizeModel();
        return;
    }
    if (!event.ControlDown()) {
        if (editing_models) {
            UnSelectAllModelsInTree();
        } else {
            UnSelectAllModels();
        }
        m_mouse_down = false;
        SetMouseStateForModels(m_mouse_down);
    }
}

void LayoutPanel::ProcessLeftMouseClick3D(wxMouseEvent& event)
{
    m_moving_handle = false;
    // don't mark mouse down if a selection is being made
    if (highlightedBaseObject != nullptr) {
        if (selectionLatched) {
            m_over_handle.reset();
            bool handledByNewApi = false;
            if (selectedBaseObject != nullptr) {
                glm::vec3 ray_origin;
                glm::vec3 ray_direction;
                GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);

                // Descriptor-based hit-test. CentreCycle / locked /
                // fromBase return nullptr from BeginDrag — those
                // hits route through the selectionOnly path below
                // instead of starting a drag session.
                Model* model = dynamic_cast<Model*>(selectedBaseObject);
                const auto currentTool =
                    selectedBaseObject->GetBaseObjectScreenLocation().GetAxisTool();
                handles::Tool newApiTool = handles::Tool::Translate;
                bool toolSupportedByNewApi = false;
                switch (currentTool) {
                    case ModelScreenLocation::MSLTOOL::TOOL_TRANSLATE:
                        newApiTool = handles::Tool::Translate;
                        toolSupportedByNewApi = true;
                        break;
                    case ModelScreenLocation::MSLTOOL::TOOL_SCALE:
                        newApiTool = handles::Tool::Scale;
                        toolSupportedByNewApi = true;
                        break;
                    case ModelScreenLocation::MSLTOOL::TOOL_ROTATE:
                        newApiTool = handles::Tool::Rotate;
                        toolSupportedByNewApi = true;
                        break;
                    case ModelScreenLocation::MSLTOOL::TOOL_XY_TRANS:
                        newApiTool = handles::Tool::XYTranslate;
                        toolSupportedByNewApi = true;
                        break;
                    case ModelScreenLocation::MSLTOOL::TOOL_ELEVATE:
                        newApiTool = handles::Tool::Elevate;
                        toolSupportedByNewApi = true;
                        break;
                    default:
                        break;
                }
                if (model != nullptr && toolSupportedByNewApi) {
                    const float zoom = modelPreview->GetCameraZoomForHandles();
                    const int hscale = modelPreview->GetHandleScale();
                    auto& sloc0 = selectedBaseObject->GetBaseObjectScreenLocation();
                    handles::ViewParams view;
                    view.axisArrowLength = sloc0.GetAxisArrowLength(zoom, hscale);
                    view.axisHeadLength  = sloc0.GetAxisHeadLength(zoom, hscale);
                    view.axisRadius      = sloc0.GetAxisRadius(zoom, hscale);
                    auto descriptors = model->GetHandles(
                        handles::ViewMode::ThreeD, newApiTool, view);
                    if (!descriptors.empty()) {
                        handles::ScreenProjection proj{
                            modelPreview->GetProjViewMatrix(),
                            modelPreview->getWidth(),
                            modelPreview->getHeight() };
                        handles::HitTestOptions opts;
                        // XY_TRANS / Elevate model the "single axis"
                        // as a wide drag area on top of the active
                        // handle (legacy AABB is ~half the arrow
                        // length on each side). Use a much looser
                        // screen tolerance so descriptors at the
                        // active handle catch clicks anywhere in
                        // that volume.
                        opts.handleTolerance = (newApiTool == handles::Tool::XYTranslate ||
                                                newApiTool == handles::Tool::Elevate)
                                                ? 28.0f : 8.0f;
                        glm::vec2 mousePx{ static_cast<float>(event.GetX()),
                                           static_cast<float>(event.GetY()) };
                        auto hit = handles::HitTest(descriptors, proj, mousePx, opts);

                        if (hit) {
                            // selection-only descriptors
                            // (3D spheres, etc.) translate the
                            // hit into a SetActiveHandle without
                            // creating a DragSession.
                            if (hit->selectionOnly) {
                                auto& sloc = selectedBaseObject->GetBaseObjectScreenLocation();
                                if (hit->id.role == handles::Role::Segment) {
                                    // segment click → SelectSegment
                                    // (curve / delete operations target it).
                                    sloc.SelectSegment(hit->id.index);
                                    xlights->GetOutputModelManager()->AddASAPWork(
                                        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW,
                                        "LayoutPanel::ProcessLeftMouseClick3D-NewAPI-Segment");
                                    handledByNewApi = true;
                                } else {
                                    if (sloc.GetActiveHandleId() == std::optional<handles::Id>(hit->id)) {
                                        sloc.AdvanceAxisTool();
                                    }
                                    sloc.SetActiveHandle(std::optional<handles::Id>(hit->id));
                                    xlights->GetOutputModelManager()->AddASAPWork(
                                        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW,
                                        "LayoutPanel::ProcessLeftMouseClick3D-NewAPI-Select");
                                    handledByNewApi = true;
                                }
                            } else {
                                handles::WorldRay startRay{ray_origin, ray_direction};
                                if (auto session = model->BeginDrag(hit->id, startRay)) {
                                    xlights->AbortRender();
                                    if (selectedBaseObject != _newModel) {
                                        CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject",
                                                        selectedBaseObject->name, "");
                                    }
                                    auto& sloc2 = selectedBaseObject->GetBaseObjectScreenLocation();
                                    const bool isAxisGizmo =
                                        hit->id.role == handles::Role::AxisArrow ||
                                        hit->id.role == handles::Role::AxisCube  ||
                                        hit->id.role == handles::Role::AxisRing;
                                    if (isAxisGizmo) {
                                        // Axis-gizmo drag: keep the body handle as
                                        // active (so GetHandles keeps emitting axis
                                        // descriptors and the gizmo stays anchored),
                                        // but sync the visual cues:
                                        //  • active_axis drives DrawActiveAxisIndicator's
                                        //    long red/green/blue line through the gizmo.
                                        //  • highlighted_handle drives the yellow tint
                                        //    on the dragged arrow/cube/ring in DrawAxisTool.
                                        sloc2.SetActiveAxis(static_cast<ModelScreenLocation::MSLAXIS>(hit->id.axis));
                                        sloc2.MouseOverHandle(hit->id);
                                    } else {
                                        // Vertex / CurveControl picks in XY_TRANS mode
                                        // skip the selectionOnly path; sync active_handle
                                        // here so the property panel and other legacy
                                        // readers reflect the dragged sub-handle.
                                        sloc2.SetActiveHandle(std::optional<handles::Id>(hit->id));
                                    }
                                    m_dragSession = std::move(session);
                                    m_moving_handle = true;
                                    m_mouse_down = true;
                                    last_centerpos = selectedBaseObject->GetBaseObjectScreenLocation().GetCenterPosition();
                                    last_worldrotate = selectedBaseObject->GetBaseObjectScreenLocation().GetRotationAngles();
                                    last_worldscale = selectedBaseObject->GetBaseObjectScreenLocation().GetScaleMatrix();
                                    xlights->GetOutputModelManager()->AddASAPWork(
                                        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW,
                                        "LayoutPanel::ProcessLeftMouseClick3D-NewAPI");
                                    handledByNewApi = true;
                                }
                            }
                        }
                    }
                }

            }
            if (!handledByNewApi && editing_models && !event.ControlDown() && !event.ShiftDown() && !event.AltDown()) {
                // click missed all handles on the currently
                // selected model. If the click landed on a different
                // model body, switch selection to it (avoids needing
                // to click empty space first to unlatch).
                glm::vec3 ray_origin;
                glm::vec3 ray_direction;
                GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
                BaseObject* which_object = nullptr;
                float distance = 1000000000.0f;
                for (const auto& it : modelPreview->GetModels()) {
                    if (it == selectedBaseObject) continue;
                    float intersection_distance = 1000000000.0f;
                    if (it->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                        if (intersection_distance < distance) {
                            distance = intersection_distance;
                            which_object = it;
                        }
                    }
                }
                if (which_object != nullptr) {
                    // Clear the tree selection first; otherwise
                    // SelectBaseObjectInTree's Select() call adds
                    // the new model to the existing tree selection
                    // (multi-select tree control), leaving the old
                    // model still group-selected.
                    UnSelectAllModelsInTree();
                    SelectBaseObject(which_object);
                    if (selectedBaseObject != nullptr) {
                        selectionLatched = true;
                        highlightedBaseObject = selectedBaseObject;
                        selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                    }
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::ProcessLeftMouseClick3D-SwitchModel");
                } else {
                    m_mouse_down = true;
                }
            }
            else {
                m_mouse_down = true;
            }
        }
        else {
            if (editing_models) {
                SelectBaseObjectInTree(highlightedBaseObject);
            } else {
                SelectBaseObject(highlightedBaseObject);
                selectionLatched = true;
                // latch center handle immediately
                selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::ProcessLeftMouseClick3D");
            }
        }
    }
    else {
        m_mouse_down = true;
    }

    m_last_mouse_x = event.GetX();
    m_last_mouse_y = event.GetY();

    if (selectedButton != nullptr) {
        //create a new model
        int wi, ht;
        modelPreview->GetVirtualCanvasSize(wi, ht);
        m_creating_bound_rect = false;
        const std::string& model_type = selectedButton->GetModelType();
        if (model_type != "DMX") {
            _newModel = CreateNewModel(model_type);
            _newModel->SetLayoutGroup(currentLayoutGroup);
        }
        else if (model_type == "DMX" && selectedDmxModelType != "") {
            _newModel = CreateNewModel(selectedDmxModelType);
            _newModel->SetLayoutGroup(currentLayoutGroup);
        }

        if (_newModel != nullptr) {
            m_moving_handle = true;
            if (model_type == "Poly Line") {
                m_polyline_active = true;
            }
            // also clear the tree control's selection.
            // UnSelectAllModels resets model-side flags but the
            // multi-select tree control still holds the previously-
            // selected model, so the new model is added to the
            // selection rather than replacing it.
            UnSelectAllModelsInTree();
            UnSelectAllModels();
            _newModel->Selected(true);
            _newModel->GetBaseObjectScreenLocation().SetActiveHandle(_newModel->GetBaseObjectScreenLocation().GetDefaultHandle());
            _newModel->GetBaseObjectScreenLocation().SetAxisTool(_newModel->GetBaseObjectScreenLocation().GetDefaultTool());
            selectionLatched = true;
            highlightedBaseObject = _newModel;
            selectedBaseObject = _newModel;
            // need to ensure the model stays selected
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::ProcessLeftMouseClick3D", _newModel, nullptr, _newModel->GetName());
            if (wi > 0 && ht > 0)
            {
                modelPreview->SetCursor(CursorTypeToWx(_newModel->InitializeLocation(m_polyline_create_handle, event.GetX(), event.GetY(), modelPreview)));
            }
            xlights->AbortRender();
            // open a placement DragSession. Session captures
            // the click intersection; mouse-move drives Update().
            handles::WorldRay clickRay{};
            GetMouseLocation(event.GetX(), event.GetY(), clickRay.origin, clickRay.direction);
            auto createSession = selectedBaseObject->GetBaseObjectScreenLocation().BeginCreate(
                _newModel->GetName(), clickRay, handles::ViewMode::ThreeD);
            if (createSession) {
                m_dragSession = std::move(createSession);
                m_mouse_down = true;
                // Setup() before the first redraw so Nodes reflect
                // the click point. Without this, the initial render
                // shows stale CreateDefaultModel-time positions and
                // the model appears to jump when the first
                // mouse-move fires Setup later.
                _newModel->Setup();
                _newModel->IncrementChangeCount();
            }
            lastModelName = _newModel->name;
            modelPreview->SetAdditionalModel(_newModel);
        }
    }

    ShowPropGrid(true);
    modelPreview->SetFocus();

    if (event.ControlDown() && !event.ShiftDown())
    {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        // if control key is down check to see if we are highlighting another model for group selection
        BaseObject* which_object = nullptr;
        float distance = 1000000000.0f;
        float intersection_distance = 1000000000.0f;
        if (editing_models) {
            for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
            {
                if (modelPreview->GetModels()[i]->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                    if (intersection_distance < distance) {
                        distance = intersection_distance;
                        which_object = modelPreview->GetModels()[i];
                    }
                }
            }
        }
        else {
            for (const auto& it : xlights->AllObjects) {
                ViewObject* view_object = it.second;
                if (view_object->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                    if (intersection_distance < distance) {
                        distance = intersection_distance;
                        which_object = view_object;
                    }
                }
            }
        }
        if (which_object != nullptr)
        {
            bool mmWorkRequired = false;
            if (which_object->Highlighted()) {
                if (!which_object->GroupSelected() && !which_object->Selected()) {
                    if (editing_models) {
                        SelectBaseObjectInTree(which_object);
                    } else {
                        which_object->GroupSelected(true);
                        mmWorkRequired = true;
                    }
                } else if (which_object->GroupSelected()) {
                    which_object->GroupSelected(false);
                    which_object->Selected(true);
                    if (selectedBaseObject != nullptr) {
                        selectedBaseObject->GroupSelected(true);
                        selectedBaseObject->Selected(false);
                        selectedBaseObject->SelectHandle(-1);
                        selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(-1);
                    }
                    selectedBaseObject = which_object;
                    highlightedBaseObject = selectedBaseObject;
                    selectedBaseObject->SelectHandle(-1);
                    selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                    mmWorkRequired = true;
                } else if (which_object->Selected()) {
                    if (editing_models) {
                        UnSelectBaseObjectInTree(which_object);
                    } else {
                        which_object->Selected(false);
                        which_object->Highlighted(false);
                        which_object->SelectHandle(-1);
                        which_object->GetBaseObjectScreenLocation().SetActiveHandle(-1);
                        selectedBaseObject = nullptr;

                        for (const auto& it : xlights->AllObjects) {
                            ViewObject* view_object = it.second;
                            if (view_object->GroupSelected()) {
                                selectedBaseObject = view_object;
                                break;
                            }
                        }
                        if (selectedBaseObject != nullptr) {
                            selectedBaseObject->GroupSelected(false);
                            selectedBaseObject->Selected(true);
                            selectedBaseObject->SelectHandle(-1);
                            selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                            highlightedBaseObject = selectedBaseObject;
                        }
                        mmWorkRequired = true;
                    }
                }

                if (mmWorkRequired) {
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::ProcessLeftMouseClick3D");
                }
            }
        }
    } else if (event.ShiftDown()) {
        m_3d_lasso_fresh_start = !m_3d_lasso_shift_continuous;
        if (m_3d_lasso_fresh_start) {
            UnSelectAllModels(false);
        }
        m_3d_lasso_shift_continuous = false;
        m_creating_bound_rect = true;
        m_bound_start_x = event.GetX();
        m_bound_start_y = event.GetY();
        m_bound_end_x = m_bound_start_x;
        m_bound_end_y = m_bound_start_y;
    } else {
        m_creating_bound_rect = false;
        m_dragging = false;
    }
}

void LayoutPanel::OnPreviewLeftDown(wxMouseEvent& event)
{
    if (m_polyline_active)
    {
        Model *m = _newModel;
        // close the prior vertex's drag session before
        // adding a new one, so the new vertex gets its own
        // session.
        if (m_dragSession) {
            m_dragSession->Commit();
            m_dragSession.reset();
        }
        m->AddHandle(modelPreview, event.GetX(), event.GetY());
        PolyLineModel* poly = dynamic_cast<PolyLineModel*>(m);
        poly->AddHandle();
        m->Reinitialize();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                      OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                      OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewLeftDown");
        m_polyline_create_handle++;
        // Advance active_handle to the next Vertex. If we're not on
        // a vertex yet (model just created), start at vertex 0.
        auto& sloc = m->GetBaseObjectScreenLocation();
        const auto curActive = sloc.GetActiveHandleId();
        const int nextVertexIdx =
            (curActive && curActive->role == handles::Role::Vertex)
                ? curActive->index + 1
                : 0;
        handles::Id nextVertexId;
        nextVertexId.role = handles::Role::Vertex;
        nextVertexId.index = nextVertexIdx;
        sloc.SetActiveHandle(std::optional<handles::Id>(nextVertexId));
        // Open a new session on the just-added vertex.
        auto polyLoc = dynamic_cast<PolyPointScreenLocation*>(&sloc);
        if (polyLoc) {
            handles::WorldRay polyRay{};
            GetMouseLocation(event.GetX(), event.GetY(), polyRay.origin, polyRay.direction);
            auto polySession = polyLoc->BeginExtend(
                m->GetName(), polyRay,
                is_3d ? handles::ViewMode::ThreeD : handles::ViewMode::TwoD,
                nextVertexIdx);
            if (polySession) {
                m_dragSession = std::move(polySession);
            }
        }
        return;
    }

    ShowPropGrid(true);
    modelPreview->SetFocus();

    if (is_3d) {
        ProcessLeftMouseClick3D(event);
        return;
    }

    // try descriptor-based 2D handle pick before any
    // Shift / Ctrl / Alt branches. Without this, Shift+click
    // routes to the marquee-select path and never reaches the
    // resize handle, which means aspect-ratio-locked resize
    // (which depends on Shift being held) is unreachable.
    {
        bool handledByNewApi = false;
        if (selectedBaseObject != nullptr) {
            Model* model = dynamic_cast<Model*>(selectedBaseObject);
            if (model != nullptr) {
                handles::ViewParams view2d;  // 2D handles are at
                // fixed positions (no camera-zoom scaling), so the
                // default 60/4 ViewParams are fine.
                auto descriptors = model->GetHandles(
                    handles::ViewMode::TwoD, handles::Tool::Translate, view2d);
                if (!descriptors.empty()) {
                    handles::ScreenProjection proj{
                        modelPreview->GetProjViewMatrix(),
                        modelPreview->getWidth(),
                        modelPreview->getHeight() };
                    handles::HitTestOptions opts;
                    opts.handleTolerance = 8.0f;
                    glm::vec2 mousePx{ static_cast<float>(event.GetX()),
                                       static_cast<float>(event.GetY()) };
                    if (auto hit = handles::HitTest(descriptors, proj, mousePx, opts)) {
                        // segment click → SelectSegment (2D).
                        if (hit->id.role == handles::Role::Segment) {
                            selectedBaseObject->GetBaseObjectScreenLocation().SelectSegment(hit->id.index);
                            modelPreview->SetCursor(wxCURSOR_DEFAULT);
                            xlights->GetOutputModelManager()->AddASAPWork(
                                OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW,
                                "LayoutPanel::OnPreviewLeftDown-NewAPI2D-Segment");
                            handledByNewApi = true;
                        } else {
                            handles::WorldRay startRay;
                            GetMouseLocation(event.GetX(), event.GetY(),
                                              startRay.origin, startRay.direction);
                            if (auto session = model->BeginDrag(hit->id, startRay)) {
                                xlights->AbortRender();
                                if (selectedBaseObject != _newModel) {
                                    CreateUndoPoint("SingleModel", selectedBaseObject->name, "");
                                }
                                m_dragSession = std::move(session);
                                m_moving_handle = true;
                                m_mouse_down = true;
                                xlights->GetOutputModelManager()->AddASAPWork(
                                    OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW,
                                    "LayoutPanel::OnPreviewLeftDown-NewAPI2D");
                                handledByNewApi = true;
                            }
                        }
                    }
                }
            }
        }
        if (handledByNewApi) {
            return;
        }
    }

    if (event.ControlDown())
    {
        SelectMultipleModels(event.GetX(), event.GetY());
        m_dragging = true;
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
    }
    else if (event.ShiftDown())
    {
        m_creating_bound_rect = true;
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        m_bound_start_x = ray_origin.x;
        m_bound_start_y = ray_origin.y;
        m_bound_end_x = m_bound_start_x;
        m_bound_end_y = m_bound_start_y;
    }
    else if (event.AltDown())
    {
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
        m_wheel_down = true;
    }
    else if (selectedButton != nullptr)
    {
        //create a new model
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
        m_creating_bound_rect = false;
        const std::string& model_type = selectedButton->GetModelType();

        if (model_type != "DMX") {
            _newModel = CreateNewModel(model_type);
        }
        else if (model_type == "DMX" && selectedDmxModelType != "") {
            _newModel = CreateNewModel(selectedDmxModelType);
        }

        if (_newModel != nullptr) {
            m_moving_handle = true;
            _newModel->SetLayoutGroup(currentLayoutGroup);
            _newModel->SetControllerName(NO_CONTROLLER);
            if (model_type == "Poly Line") {
                m_polyline_active = true;
            }
            UnSelectAllModelsInTree();
            UnSelectAllModels();
            _newModel->Selected(true);
            // Latch active_handle to the screen location's default
            // handle. The polyline-extend branch reads
            // `GetActiveHandleId()` to compute the next vertex
            // index — without this latch the 2nd click would drag
            // vertex 0 instead of the newly-added trailing vertex.
            _newModel->GetBaseObjectScreenLocation().SetActiveHandle(_newModel->GetBaseObjectScreenLocation().GetDefaultHandle());
            _newModel->GetBaseObjectScreenLocation().SetAxisTool(_newModel->GetBaseObjectScreenLocation().GetDefaultTool());
            modelPreview->SetCursor(CursorTypeToWx(_newModel->InitializeLocation(m_polyline_create_handle, event.GetX(), event.GetY(), modelPreview)));
            // Open a 2D placement DragSession — mouse-move drives
            // Update() until the session commits.
            handles::WorldRay clickRay2d{};
            GetMouseLocation(event.GetX(), event.GetY(), clickRay2d.origin, clickRay2d.direction);
            auto createSession2d = _newModel->GetBaseObjectScreenLocation().BeginCreate(
                _newModel->GetName(), clickRay2d, handles::ViewMode::TwoD);
            if (createSession2d) {
                m_dragSession = std::move(createSession2d);
                // See 3D path comment: rebuild Nodes immediately so
                // the first redraw after click reflects the click-
                // point position.
                _newModel->Setup();
                _newModel->IncrementChangeCount();
            }
            lastModelName = _newModel->name;
            modelPreview->SetAdditionalModel(_newModel);
        }
    }
    else
    {
        m_moving_handle = false;
        m_creating_bound_rect = false;

        if (!event.wxKeyboardState::ControlDown())
        {
            UnSelectAllModelsInTree();
        }

        Model* singleModel = SelectSingleModel(event.GetX(), event.GetY());
        if (singleModel != nullptr)
        {
            SelectModelInTree(singleModel);
            m_dragging = true;
            m_previous_mouse_x = event.GetX();
            m_previous_mouse_y = event.GetY();
            xlights->SetStatusText(wxString::Format("x=%d y=%d", m_previous_mouse_x, m_previous_mouse_y));
        }
        else
        {
            m_creating_bound_rect = true;
            glm::vec3 ray_origin;
            glm::vec3 ray_direction;
            GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
            m_bound_start_x = ray_origin.x;
            m_bound_start_y = ray_origin.y;
            m_bound_end_x = m_bound_start_x;
            m_bound_end_y = m_bound_start_y;
        }
    }
}

void LayoutPanel::OnPreviewLeftUp(wxMouseEvent& event)
{
    if (m_polyline_active) {
        m_mouse_down = false;
        SetMouseStateForModels(m_mouse_down);
        return;
    }

    if (m_wheel_down)
    {
        m_wheel_down = false;
        return;
    }

    if (m_mouse_down && m_dragSession) {
        auto commit = m_dragSession->Commit();
        const bool dirty = commit.dirty != handles::DirtyField::None;
        m_dragSession.reset();
        if (dirty) {
            xlights->GetOutputModelManager()->AddASAPWork(
                OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                    OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                    OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW,
                "LayoutPanel::OnPreviewLeftUp-NewAPI");
        }
    }

    if (is_3d && m_mouse_down) {
        if (selectedBaseObject != nullptr) {
            selectedBaseObject->GetBaseObjectScreenLocation().SetActiveAxis(ModelScreenLocation::MSLAXIS::NO_AXIS);
            selectedBaseObject->GetBaseObjectScreenLocation().SetActivePlane(ModelScreenLocation::MSLPLANE::NO_PLANE);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewLeftDown");
        }
        modelPreview->SetCameraView(0, 0, true);
    }

    m_mouse_down = false;
    SetMouseStateForModels(m_mouse_down);
    m_moving_handle = false;
    over_handle.reset();

    if (m_creating_bound_rect) {
        if (is_3d) {
            m_bound_end_x = event.GetX();
            m_bound_end_y = event.GetY();
            m_3d_lasso_shift_continuous = event.ShiftDown();
            if (m_3d_lasso_fresh_start) {
                // Fresh-start lasso: clear tree so only the new box items end up selected.
                TreeListViewModels->UnselectAll();
            }
        } else {
            glm::vec3 ray_origin;
            glm::vec3 ray_direction;
            GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
            m_bound_end_x = ray_origin.x;
            m_bound_end_y = ray_origin.y;
        }
        SelectAllInBoundingRect(event.ControlDown() && event.ShiftDown());
        m_creating_bound_rect = false;
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewLeftUp");
    }
    FinalizeModel();
}

static Model* GetXlightsModel(Model* model, std::string& last_model, xLightsFrame* xlights, bool& cancelled, bool download, wxProgressDialog* prog, int low, int high, ModelPreview* modelPreview, int& widthmm, int& heightmm, int& depthmm, std::vector<DownloadedModelInfo>* additionalModels = nullptr)
{
    pugi::xml_document doc;
    bool docLoaded = false;
    if (last_model.empty()) {
        if (download) {
            xlights->SuspendAutoSave(true);
            VendorModelDialog dlg(xlights, xlights->CurrentDir);
            xlights->SetCursor(wxCURSOR_WAIT);
            if (dlg.DlgInit(prog, low, high)) {
                if (prog != nullptr) {
                    prog->Update(100);
                }
                xlights->SetCursor(wxCURSOR_DEFAULT);
                if (dlg.ShowModal() == wxID_OK) {
                    xlights->SuspendAutoSave(false);
                    last_model = dlg.GetModelFile();
                    widthmm = dlg.GetModelWidthMM();
                    heightmm = dlg.GetModelHeightMM();
                    depthmm = dlg.GetModelDepthMM();

                    // Capture additional downloaded models (beyond the first) for batch placement.
                    // Always clear first so callers that reuse the vector don't see stale entries.
                    if (additionalModels != nullptr) {
                        additionalModels->clear();
                        const auto& allDownloaded = dlg.GetDownloadedModels();
                        if (allDownloaded.size() > 1) {
                            additionalModels->assign(allDownloaded.begin() + 1, allDownloaded.end());
                        }
                    }

                    if (last_model.empty()) {
                        DisplayError("Failed to download model file.");

                        cancelled = true;
                        return model;
                    }
                } else {
                    xlights->SuspendAutoSave(false);
                    cancelled = true;
                    return model;
                }
            } else {
                if (prog != nullptr)
                    prog->Hide();
                xlights->SetCursor(wxCURSOR_DEFAULT);
                xlights->SuspendAutoSave(false);
                cancelled = true;
                return model;
            }
        } else {
            std::string filename;
            if (auto* ui = xlights->GetUICallbacks()) {
                filename = ui->PromptForFile("Choose model file",
                    "xLights Model files (*.xmodel)|*.xmodel|LOR prop files (*.lff;*.lpf)|*.lff;*.lpf|General Device Type Format (*.gdtf)|*.gdtf");
            }
            if (filename.empty()) {
                cancelled = true;
                return model;
            }
            last_model = filename;

            std::string last_model_lower = last_model;
            std::transform(last_model_lower.begin(), last_model_lower.end(), last_model_lower.begin(), ::tolower);
            if (last_model_lower.ends_with(".xmodel")) {
                doc.load_file(last_model.c_str());
                if (doc.document_element() && !doc.document_element().attribute("name").empty()) {
                    docLoaded = true;
                    std::string modelName = doc.document_element().attribute("name").as_string("");

                    if (!doc.document_element().attribute("widthmm").empty()) {
                        widthmm = (int)std::strtol(doc.document_element().attribute("widthmm").as_string(""), nullptr, 10);
                    }
                    if (!doc.document_element().attribute("heightmm").empty()) {
                        heightmm = (int)std::strtol(doc.document_element().attribute("heightmm").as_string(""), nullptr, 10);
                    }
                    if (!doc.document_element().attribute("depthmm").empty()) {
                        depthmm = (int)std::strtol(doc.document_element().attribute("depthmm").as_string(""), nullptr, 10);
                    }

#ifdef __WXMSW__
                    if (!xlights->GetIgnoreVendorModelRecommendations()) {
#endif
                        std::string mappingJson = "https://raw.githubusercontent.com/xLightsSequencer/xLights/master/download/model_vendor_mapping.json";
                        std::string json = CachedFileDownloader::GetDefaultCache().GetFile(mappingJson, CACHETIME_DAY);
                        if (json == "") {
                            json = FileUtils::GetResourcesDir() + "/model_vendor_mapping.json";
                        }
                        if (json != "" && !FileExists(json)) {
                            json = "";
                        }
                        if (json != "") {
                            try {
                                std::ifstream file(json);
                                if (file.is_open()) {
                                    nlohmann::json origJson = nlohmann::json::parse(file);

                                    VendorModelDialog* dlg = nullptr;
#ifndef __WXMSW__
                                    bool block = false;
#endif
                                    std::string vendorBlock;
                                    for (auto& [name, v] : origJson["mappings"].items()) {
                                        bool matches = false;
                                        std::string newModelName = modelName;
                                        bool localBlock = false;
                                        if (v.contains("regex") && v["regex"].get<bool>()) {
                                            try {
                                                std::regex regex(name);
                                                std::string modelNameStr = modelName;
                                                if (std::regex_search(modelNameStr, regex)) {
                                                    std::string nmodel = v["model"].get<std::string>();
                                                    newModelName = std::regex_replace(modelNameStr, regex, nmodel);
                                                    matches = true;
                                                    if (v.contains("block")) {
                                                        localBlock = v["block"].get<bool>();
                                                    }
                                                }
                                            } catch (std::regex_error&) {
                                            }
                                        } else if (name == modelName) {
                                            matches = true;
                                            newModelName = v["model"].get<std::string>();
                                            if (v.contains("block")) {
                                                localBlock = v["block"].get<bool>();
                                            }
                                        }
                                        if (matches) {
                                            std::string vendor = v["vendor"].get<std::string>();
                                            if (dlg == nullptr) {
                                                dlg = new VendorModelDialog(xlights, xlights->CurrentDir);
                                                UNUSED(dlg->DlgInit(prog, low, high));
                                            }
                                            if (localBlock) {
                                                vendorBlock = vendor;
#ifndef __WXMSW__
                                                block = true;
#endif
                                            }
                                            if (dlg->FindModelFile(vendor, newModelName)) {
                                                if (localBlock) {
                                                    std::string msg = "'" + vendor + "' provides a certified model for '" + newModelName + "' in the xLights downloads.  The " + "vendor has requested that the model they provide be the model that is used." + "Use the Vendor provided model instead?";
                                                    if (auto* ui = xlights->GetUICallbacks()) {
                                                        if (ui->PromptYesNo(msg, "Use Vendor Certified Model?")) {
                                                            last_model = dlg->GetModelFile();
                                                        } else {
                                                            last_model = "";
                                                        }
                                                    }
                                                    docLoaded = false;
                                                    break;
                                                } else if (!xlights->GetIgnoreVendorModelRecommendations()) {
                                                    std::string msg = "xLights found a '" + vendor + "' provided and certified model for '" + newModelName + "' in the xLights downloads.  The " + "Vendor provided models are strongly recommended by the vendor due to their claimed quality and ease of use.\n\nWould you prefer to " + "use the Vendor provided model instead?";
                                                    if (auto* ui = xlights->GetUICallbacks()) {
                                                        if (ui->PromptYesNo(msg, "Use Vendor Certified Model?")) {
                                                            last_model = dlg->GetModelFile();
                                                            docLoaded = false;
                                                            break;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
#ifndef __WXMSW__
                                    if (block) {
                                        std::string msg = "'" + vendorBlock + "' has requested that the models they provide be the models that are used.";
                                        if (auto* ui = xlights->GetUICallbacks()) {
                                            ui->ShowMessage(msg, "Loading of Model Blocked");
                                        }
                                        last_model = "";
                                    }
#endif
                                    if (dlg) {
                                        delete dlg;
                                    }
                                }
                            }
                            catch (nlohmann::json::parse_error& ) {
                            }
                        }

#ifdef __WXMSW__
                    }
#endif

                }
            }
        }
    }
    std::string last_model_lc = last_model;
    std::transform(last_model_lc.begin(), last_model_lc.end(), last_model_lc.begin(), ::tolower);
    if (last_model_lc.ends_with(".gdtf")) {
        wxFileInputStream fin(last_model);
        wxZipInputStream zin(fin);
        wxZipEntry* ent = zin.GetNextEntry();

        while (ent != nullptr) {
            if (ent->GetName() == "description.xml") {
                pugi::xml_document gdtf_doc;
                wxMemoryOutputStream memStream;
                zin.Read(memStream);
                size_t sz = memStream.GetLength();
                std::vector<char> buf(sz);
                memStream.CopyTo(buf.data(), sz);
                gdtf_doc.load_buffer(buf.data(), sz);

                XmlSerialize::GdtfModelData gdtfData;
                if (XmlSerialize::ParseGdtfDescriptionXml(gdtf_doc, xlights->AllModels, xlights->GetUICallbacks(), cancelled, gdtfData)) {
                    model = XmlSerialize::CreateDmxModelFromGdtfData(model, gdtfData, xlights->AllModels);
                } else {
                    cancelled = true;
                }
                break;
            }
            ent = zin.GetNextEntry();
        }
        return model;
    } else if (!last_model_lc.ends_with(".xmodel")) {
        return model;
    }

    if (!docLoaded) {
        doc.load_file(last_model.c_str());
    }
    if (doc.document_element()) {
        pugi::xml_node root = doc.document_element();

        model->SetStartChannel("1");
        model = model->CreateDefaultModelFromSavedModelNode(model, root, xlights->AllModels, cancelled);

        if (!cancelled && model != nullptr) {
            // Reset controller name to NO_CONTROLLER so ReworkStartChannel auto-assigns
            // a correct start channel. Downloaded/imported xmodel files store vendor-specific
            // channel assignments that are not appropriate for the user's setup.
            // The deserialized model object has _controllerName="" (C++ default), not
            // NO_CONTROLLER, which causes ReworkStartChannel to treat it as a fixed reference
            // point rather than auto-assigning it, leaving it stuck at channel 1.
            model->SetControllerName(NO_CONTROLLER, true);
        }

        if (!cancelled)
            return model;
    }
    return model;
}

void LayoutPanel::FinalizeModel()
{
    xlights->AddTraceMessage("In LayoutPanel::FinalizeModel");
    // discard any active drag session before mutating mPos.
    // FinalizeModel's polyline DeleteHandle below shrinks mPos, so a
    // session pointing at the trailing vertex would crash on the
    // next mouse-move SetPoint.
    if (m_dragSession) {
        m_dragSession.reset();
    }
    if (m_polyline_active && m_polyline_create_handle > 1) {
        Model *m = _newModel;
        if (m != nullptr)
        {
            xlights->AddTraceMessage("LayoutPanel::FinalizeModel Polyline deleting handle.");
            m->DeleteHandle(m_polyline_create_handle);

            auto plm = dynamic_cast<PolyLineModel*>(m);
            plm->ClearPolyLineCreate(); // disable the auto-distribute node feature
            if (plm->GetNumHandles() < 2) {
                // If we end up with less than 2 points then we destroy the polyline
                highlightedBaseObject = nullptr;
                selectedBaseObject = nullptr;
                modelPreview->SetAdditionalModel(nullptr);
                delete _newModel;
                _newModel = nullptr;
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                              OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "FinalizeModel");
            }
        }
    }
    m_moving_handle = false;
    m_dragging = false;
    m_polyline_active = false;
    m_over_handle.reset();
    m_polyline_create_handle = NO_HANDLE;

    if (_newModel != nullptr) {
        xlights->AddTraceMessage("LayoutPanel::FinalizeModel New model is not null.");
        _newModel->GetBaseObjectScreenLocation().SetAxisTool(ModelScreenLocation::MSLTOOL::TOOL_TRANSLATE);
        // cache the selected button as it may change during a download or some such event
        auto b = selectedButton;
        std::vector<DownloadedModelInfo> additionalModels;
        glm::vec3 firstModelPos(0.0f);
        if (b != nullptr && (b->GetModelType() == "Import Custom" || b->GetModelType() == "Download"))
        {
            xlights->AddTraceMessage("LayoutPanel::FinalizeModel We were downloading or importing.");
            bool cancelled = false;
            auto pos = _newModel->GetBaseObjectScreenLocation().GetWorldPosition();
            firstModelPos = pos;

            wxProgressDialog* prog = nullptr;
            if (b->GetModelType() == "Download")
            {
                prog = new wxProgressDialog("Model download", "Downloading models ...", 100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
                prog->Show();
                prog->CenterOnParent();
            }
            auto oldNewModel = _newModel;
            auto oldam = modelPreview->GetAdditionalModel();
            modelPreview->SetAdditionalModel(nullptr); // just in case we delete the model

            int widthmm = -1;
            int heightmm = -1;
            int depthmm = -1;

            _newModel = GetXlightsModel(_newModel, _lastXlightsModel, xlights, cancelled, b->GetModelType() == "Download", prog, 0, 99, modelPreview, widthmm, heightmm, depthmm, &additionalModels);

            // These statements ensure the Additional model and _newModel pointers are all ok and any unnecessary models is cleaned up
            if (_newModel != oldNewModel) {
                // model was changed

                if (highlightedBaseObject == oldNewModel) {
                    highlightedBaseObject = _newModel;
                }
                if (selectedBaseObject == oldNewModel) {
                    selectedBaseObject = _newModel;
                }

                if (oldam == oldNewModel) {
                    modelPreview->SetAdditionalModel(_newModel);
                    oldam = nullptr;
                }
                else {
                    modelPreview->SetAdditionalModel(oldam);
                }
                // dont delete the oldNewModel as it should already be deleted
                //delete oldNewModel;
                oldNewModel = nullptr;
            }
            else {
                modelPreview->SetAdditionalModel(oldam);
            }

            if (cancelled || _newModel == nullptr) {
                // Clear the additional model BEFORE deleting the progress dialog,
                // since deleting the dialog can pump events that would try to render
                // the model we are about to delete
                modelPreview->SetAdditionalModel(nullptr);

                // Now safe to delete the progress dialog
                if (prog != nullptr) {
                    delete prog;
                }

                _lastXlightsModel = "";
                xlights->AddTraceMessage("LayoutPanel::FinalizeModel Downloading or importing cancelled.");
                xlights->GetOutputModelManager()->ClearSelectedModel();
                xlights->AddTraceMessage("LayoutPanel::FinalizeModel Additional model cleared.");
                if (_newModel != nullptr) {
                    if (highlightedBaseObject == _newModel) {
                        highlightedBaseObject = nullptr;
                    }
                    if (selectedBaseObject == _newModel) {
                        selectedBaseObject = nullptr;
                    }
                    xlights->AddTraceMessage("LayoutPanel::FinalizeModel About to delete newModel.");
                    auto* modelToDelete = _newModel;
                    _newModel = nullptr;
                    delete modelToDelete;
                    xlights->AddTraceMessage("LayoutPanel::FinalizeModel newModel successfully deleted.");
                }
                xlights->AddTraceMessage("LayoutPanel::Model deleted.");
                modelPreview->SetCursor(wxCURSOR_DEFAULT);
                b->SetState(0);
                selectedButton = nullptr;
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                              OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "FinalizeModel");
                return;
            }

            // we get rid of progress dialog here (non-cancel path)
            if (prog != nullptr) {
                delete prog;
            }

            if (_newModel->GetDisplayAs() == DisplayAsType::PolyLine) {
                _newModel->SetPosition(pos.x, pos.y);
            } else {
                _newModel->GetBaseObjectScreenLocation().SetWorldPosition(pos);
            }
            if (b->GetState() == 1)
            {
                _lastXlightsModel = "";
            }
        }
        xlights->AddTraceMessage("LayoutPanel::FinalizeModel Adding the model.");
        CreateUndoPoint("All", "", "");
        xlights->AllModels.AddModel(_newModel);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "FinalizeModel");

        _newModel->SetLayoutGroup(currentLayoutGroup == "All Models" ? "Default" : currentLayoutGroup);

        // Process additional models from multi-select download
        if (!additionalModels.empty()) {
            // Space each model by the actual width of its left neighbour (in layout units)
            // plus a small padding, so batch placement scales with model size rather than
            // using a single magic offset for every model.
            constexpr float BATCH_PLACEMENT_PADDING = 50.0f;
            constexpr float BATCH_PLACEMENT_MIN_OFFSET = 100.0f;

            float previousWidth = std::max(_newModel->GetRestorableMWidth(), BATCH_PLACEMENT_MIN_OFFSET);
            float currentX = firstModelPos.x + previousWidth + BATCH_PLACEMENT_PADDING;

            for (const auto& modelInfo : additionalModels) {
                std::string extraModelPath = modelInfo.modelFile;
                if (extraModelPath.empty()) continue;

                std::string extraModelLower = extraModelPath;
                std::transform(extraModelLower.begin(), extraModelLower.end(), extraModelLower.begin(),
                               [](unsigned char c) { return std::tolower(c); });
                if (!extraModelLower.ends_with(".xmodel")) continue;

                pugi::xml_document extraDoc;
                extraDoc.load_file(extraModelPath.c_str());
                if (!extraDoc.document_element()) continue;

                bool extraCancelled = false;
                xlights->GetOutputModelManager()->DisableASAPWork(true);
                Model* extraModel = xlights->AllModels.CreateDefaultModel("Custom", "1");
                xlights->GetOutputModelManager()->DisableASAPWork(false);

                if (extraModel == nullptr) continue;

                extraModel->SetStartChannel("1");
                // CreateDefaultModelFromSavedModelNode takes ownership of extraModel and deletes
                // it internally before returning a newly deserialized model. On failure it returns
                // nullptr having already freed the passed-in pointer, so we must NOT delete
                // extraModel here (would double-delete). Match the primary model path which also
                // just returns without manual cleanup.
                extraModel = extraModel->CreateDefaultModelFromSavedModelNode(extraModel, extraDoc.document_element(), xlights->AllModels, extraCancelled);

                if (extraCancelled || extraModel == nullptr) {
                    continue;
                }

                // Reset controller name to NO_CONTROLLER so ReworkStartChannel auto-assigns
                // a correct start channel (matches behavior in GetXlightsModel for the primary model)
                extraModel->SetControllerName(NO_CONTROLLER, true);

                extraModel->GetBaseObjectScreenLocation().SetWorldPosition(glm::vec3(currentX, firstModelPos.y, firstModelPos.z));
                extraModel->SetLayoutGroup(currentLayoutGroup == "All Models" ? "Default" : currentLayoutGroup);
                xlights->AllModels.AddModel(extraModel);

                // Advance past this model's own width so the next one sits beside it
                float thisWidth = std::max(extraModel->GetRestorableMWidth(), BATCH_PLACEMENT_MIN_OFFSET);
                currentX += thisWidth + BATCH_PLACEMENT_PADDING;
            }
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "FinalizeModel");
        }

        modelPreview->SetCursor(wxCURSOR_DEFAULT);
        modelPreview->SetAdditionalModel(nullptr);
        if ((b != nullptr && b->GetState() == 1) || selectedDmxModelType != "") {
            xlights->AddTraceMessage("LayoutPanel::FinalizeModel Exiting done.");
            std::string name = _newModel->name;
            _newModel = nullptr;
            if (b != nullptr) {  // needed if dmx model type
                b->SetState(0);
            }
            selectedButton = nullptr;
            selectedBaseObject = nullptr;
            selectedDmxModelType = "";
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "FinalizeModel", nullptr, nullptr, name);
            //SelectBaseObject(name);            SelectBaseObject3D();
        }
        else {
            xlights->AddTraceMessage("LayoutPanel::FinalizeModel Exiting but can draw another model.");
            _newModel = nullptr;
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "FinalizeModel");
        }
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST |
                                                      OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "FinalizeModel");
    }
}

void LayoutPanel::OnPreviewMouseLeave(wxMouseEvent& event)
{
    m_dragging = false;
    m_wheel_down = false;
}

void LayoutPanel::OnPreviewMouseWheelDown(wxMouseEvent& event)
{
    m_previous_mouse_x = event.GetX();
    m_previous_mouse_y = event.GetY();
    m_wheel_down = true;
}

void LayoutPanel::OnPreviewMouseWheelUp(wxMouseEvent& event)
{
    m_wheel_down = false;
}
void LayoutPanel::OnPreviewMotion3DButtonEvent(wxCommandEvent &event) {
    
    if (event.GetString() == "BUTTON_MENU") {
        wxMouseEvent evt;
        OnPreviewRightDown(evt);
    } else {
        modelPreview->OnMotion3DButtonEvent(event);
        /*
        int gSize = selectedTreeGroups.size();
        int smSize = selectedTreeSubModels.size();
        if (selectedBaseObject != nullptr && gSize == 0 && smSize == 0) {
        } else {
        }
        */
    }
}

void LayoutPanel::OnPreviewMotion3D(Motion3DEvent &event) {
    int gSize = selectedTreeGroups.size();
    int smSize = selectedTreeSubModels.size();
    if (selectedBaseObject != nullptr && gSize == 0 && smSize == 0 && !event.ControlDown() && !event.RawControlDown()) {
        int active_handle = selectedBaseObject->GetBaseObjectScreenLocation().GetActiveHandle();
        if (!xlights->AbortRender()) return;
        CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject", selectedBaseObject->name, std::to_string(active_handle));

        float scale = modelPreview->translateToBacking(1.0) * 20.0 * modelPreview->GetZoom(); //20 pixels at max speed, default zoom
        if (!modelPreview->Is3D()) {
            //moving/rotating the entire model
            constexpr float rscale = 10; //10 degrees per full 1.0 aka: max speed
            selectedBaseObject->Rotate(ModelScreenLocation::MSLAXIS::X_AXIS, event.rotations.x * rscale);
            selectedBaseObject->Rotate(ModelScreenLocation::MSLAXIS::Y_AXIS, -event.rotations.z * rscale);
            selectedBaseObject->Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS, event.rotations.y * rscale);

            selectedBaseObject->AddOffset(event.translations.x * scale,
                                          -event.translations.z * scale - event.translations.y * scale,
                                          0.0f);

            last_centerpos = selectedBaseObject->GetBaseObjectScreenLocation().GetCenterPosition();
            last_worldrotate = selectedBaseObject->GetBaseObjectScreenLocation().GetRotationAngles();
            last_worldscale = selectedBaseObject->GetBaseObjectScreenLocation().GetScaleMatrix();

            SetupPropGrid(selectedBaseObject);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                          OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                          OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMotion3D");
        } else if (modelPreview->Is3D()) {
            bool update_rgbeffects = false;
            selectedBaseObject->MoveHandle3D(scale, active_handle, event.rotations, event.translations, update_rgbeffects);

            last_centerpos = selectedBaseObject->GetBaseObjectScreenLocation().GetCenterPosition();
            last_worldrotate = selectedBaseObject->GetBaseObjectScreenLocation().GetRotationAngles();
            last_worldscale = selectedBaseObject->GetBaseObjectScreenLocation().GetScaleMatrix();
            
            SetupPropGrid(selectedBaseObject);
            if (update_rgbeffects) {
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPreviewMotion3D");
            }
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                          OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMotion3D");
        }
    } else {
        modelPreview->OnMotion3DEvent(event);
    }
}

void LayoutPanel::OnPreviewRotateGesture(wxRotateGestureEvent& event) {
    if (!rotate_gesture_active && !event.IsGestureStart()) {
        return;
    }
    if (event.IsGestureEnd()) {
        rotate_gesture_active = false;
    } else if (event.IsGestureStart()) {
        rotate_gesture_active = true;
    }
    int gSize = selectedTreeGroups.size();
    int smSize = selectedTreeSubModels.size();
    
    if (selectedBaseObject != nullptr && gSize == 0 && smSize == 0) {
        //groups and submodels shouldn't rotate
        float delta = (m_last_mouse_x - (event.GetRotationAngle() * 1000)) / 1000.0;
        if (!event.IsGestureStart()) {
            //convert to degrees
            delta = (delta/(2*M_PI))*360.0;
            if (delta > 90) {
                delta -= 360;
            }
            if (delta < -90) {
                delta += 360;
            }

            ModelScreenLocation::MSLAXIS axis = ModelScreenLocation::MSLAXIS::Z_AXIS; //default is around z axis
            if (wxGetKeyState(WXK_SHIFT)) {
                axis = ModelScreenLocation::MSLAXIS::X_AXIS;
            } else if (wxGetKeyState(WXK_CONTROL)) {
                axis = ModelScreenLocation::MSLAXIS::Y_AXIS;
            }
            if (selectedBaseObject->Rotate(axis, delta)) {
                SetupPropGrid(selectedBaseObject);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                              OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                              OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewRotateGesture");
            }
        } else {
            CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject", selectedBaseObject->name, "Zoom");
        }
    }
    m_last_mouse_x = (event.GetRotationAngle() * 1000);
}

void LayoutPanel::OnPreviewZoomGesture(wxZoomGestureEvent& event) {
    if (!zoom_gesture_active && !event.IsGestureStart()) {
        return;
    }
    if (event.IsGestureEnd()) {
        zoom_gesture_active = false;
    } else if (event.IsGestureStart()) {
        zoom_gesture_active = true;
    }
    float delta = (m_last_mouse_x - (event.GetZoomFactor() * 1000)) / 1000.0;
    
    int gSize = selectedTreeGroups.size();
    int smSize = selectedTreeSubModels.size();
    
    if (selectedBaseObject != nullptr && gSize == 0 && smSize == 0) {
        //groups and submodels shouldn't resize/scale
        if (!event.IsGestureStart()) {
            //resize model
            if (selectedBaseObject->Scale(glm::vec3(1.0f - delta))) {
                SetupPropGrid(selectedBaseObject);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                              OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                              OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewZoomGesture");
            }

            if (Is3d()) {
                auto pos = selectedBaseObject->GetBaseObjectScreenLocation().GetWorldPosition();
                xlights->SetStatusText(wxString::Format("x=%.2f y=%.2f z=%.2f %s", pos.x, pos.y, pos.z, selectedBaseObject->GetDimension()));
            }
        } else {
            CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject", selectedBaseObject->name, "Zoom");
        }
    } else if (!event.IsGestureStart()) {
        modelPreview->SetZoomDelta(delta);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewZoomGesture");
    }
    m_last_mouse_x = (event.GetZoomFactor() * 1000);
}

void LayoutPanel::OnPreviewMagnify(wxMouseEvent& event) {
    if (event.GetWheelRotation() == 0 || event.GetMagnification() == 0.0f) {
        //magnification of 0 is sometimes generated for other gestures (pinch/zoom), ignore
        return;
    }
    modelPreview->SetZoomDelta(event.GetMagnification() > 0 ? -0.1f : 0.1f);
}

void LayoutPanel::OnPreviewMouseWheel(wxMouseEvent& event)
{
    if (event.GetWheelRotation() == 0) {
        //rotation of 0 is sometimes generated for other gestures (pinch/zoom), ignore
        return;
    }
    // Don't allow zoom if panning
    if (!m_wheel_down) {
        bool fromTrackPad = IsMouseEventFromTouchpad();
        if (is_3d) {
            if (!fromTrackPad || event.ControlDown()) {
                int mouse_x = event.GetX();
                int mouse_y = event.GetY();
                float centerx = modelPreview->getWidth() / 2.0f;
                float centery = modelPreview->getHeight() / 2.0f;
                float deltax = mouse_x - centerx;
                float deltay = mouse_y - centery;
                if( !xlights->ZoomMethodToCursor() ) {
                    deltax = 0;
                    deltay = 0;
                }
                float zoom_delta = event.GetWheelRotation() > 0 ? -0.1f : 0.1f;
                if (fromTrackPad) {
                    float f = event.GetWheelRotation();
                    zoom_delta = -f / 100.0f;
                }
                modelPreview->SetZoomDelta(zoom_delta);
                float zoom = modelPreview->GetZoom();
                float new_x = deltax * zoom_delta / zoom;
                float new_y = deltay * zoom_delta / zoom;

                // account for grid rotation
                float angleX = glm::radians(modelPreview->GetCameraRotationX());
                float angleY = glm::radians(modelPreview->GetCameraRotationY());
                float delta_x = 0.0f;
                float delta_y = 0.0f;
                float delta_z = 0.0f;
                bool top_view = (angleX > glm::radians(45.0f)) && (angleX < glm::radians(135.0f));
                bool bottom_view = (angleX > glm::radians(225.0f)) && (angleX < glm::radians(315.0f));
                bool upside_down_view = (angleX >= glm::radians(135.0f)) && (angleX <= glm::radians(225.0f));
                if (top_view) {
                    delta_x = new_x * std::cos(angleY) - new_y * std::sin(angleY);
                    delta_z = new_y * std::cos(angleY) + new_x * std::sin(angleY);
                }
                else if (bottom_view) {
                    delta_x = new_x * std::cos(angleY) + new_y * std::sin(angleY);
                    delta_z = -new_y * std::cos(angleY) + new_x * std::sin(angleY);
                }
                else {
                    delta_x = new_x * std::cos(angleY);
                    delta_y = new_y;
                    delta_z = new_x * std::sin(angleY);
                    if (!upside_down_view) {
                        delta_y *= -1.0f;
                    }
                }
                delta_x *= modelPreview->GetZoom() * 2.0f;
                delta_y *= modelPreview->GetZoom() * 2.0f;
                delta_z *= modelPreview->GetZoom() * 2.0f;
                modelPreview->SetPan(delta_x, delta_y, delta_z);
            } else {
                if (event.ShiftDown()) {
                    float new_x = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? 0 : -event.GetWheelRotation();
                    float new_y = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? -event.GetWheelRotation() : 0;

                    // account for grid rotation
                    float angleX = glm::radians(modelPreview->GetCameraRotationX());
                    float angleY = glm::radians(modelPreview->GetCameraRotationY());
                    float delta_x = 0.0f;
                    float delta_y = 0.0f;
                    float delta_z = 0.0f;
                    bool top_view = (angleX > glm::radians(45.0f)) && (angleX < glm::radians(135.0f));
                    bool bottom_view = (angleX > glm::radians(225.0f)) && (angleX < glm::radians(315.0f));
                    bool upside_down_view = (angleX >= glm::radians(135.0f)) && (angleX <= glm::radians(225.0f));
                    if( top_view ) {
                        delta_x = new_x * std::cos(angleY) - new_y * std::sin(angleY);
                        delta_z = new_y * std::cos(angleY) + new_x * std::sin(angleY);
                    } else if( bottom_view ) {
                        delta_x = new_x * std::cos(angleY) + new_y * std::sin(angleY);
                        delta_z = -new_y * std::cos(angleY) + new_x * std::sin(angleY);
                    } else {
                        delta_x = new_x * std::cos(angleY);
                        delta_y = new_y;
                        delta_z = new_x * std::sin(angleY);
                        if( upside_down_view ) {
                            delta_y *= -1.0f;
                        }
                    }
                    delta_x *= modelPreview->GetZoom() * 2.0f;
                    delta_y *= modelPreview->GetZoom() * 2.0f;
                    delta_z *= modelPreview->GetZoom() * 2.0f;
                    modelPreview->SetPan(delta_x, delta_y, delta_z);
                } else {
                    float delta_x = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? 0 : -event.GetWheelRotation();
                    float delta_y = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? -event.GetWheelRotation() : 0;

                    modelPreview->SetCameraView(delta_x, delta_y, false);
                    modelPreview->SetCameraView(0, 0, true);
                }
            }
        }
        else {
            if (!fromTrackPad || event.ControlDown()) {
                int mouse_x = event.GetX();
                int mouse_y = event.GetY();
                float centerx = modelPreview->getWidth() / 2.0f;
                float centery = modelPreview->getHeight() / 2.0f;
                float deltax = mouse_x - centerx;
                float deltay = mouse_y - centery;
                float zoom_delta = event.GetWheelRotation() > 0 ? -0.1f : 0.1f;
                if (fromTrackPad) {
                    float f = event.GetWheelRotation();
                    zoom_delta = -f / 100.0f;
                }
                modelPreview->SetZoomDelta(zoom_delta);
                float zoom = modelPreview->GetZoom();
                float new_x = deltax * zoom_delta / zoom;
                float new_y = deltay * zoom_delta / zoom;
                modelPreview->SetPan(new_x, -new_y, 0.0f);
            }
            else {
                float delta_x = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? 0 : -event.GetWheelRotation();
                float delta_y = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? -event.GetWheelRotation() : 0;
                if (!fromTrackPad) {
                    delta_x *= modelPreview->GetZoom() * 2.0f;
                    delta_y *= modelPreview->GetZoom() * 2.0f;
                }
                modelPreview->SetPan(delta_x, delta_y, 0.0f);
                m_previous_mouse_x = event.GetX();
                m_previous_mouse_y = event.GetY();
            }
        }
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseWheel");
    }
}

void LayoutPanel::OnPreviewMouseMove3D(wxMouseEvent& event)
{
    

    xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D");

    if (!m_creating_bound_rect && !event.ShiftDown()) {
        m_3d_lasso_shift_continuous = false;
    }

    if (m_creating_bound_rect)
    {
        xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Creating bounding rectangle");
        m_bound_end_x = event.GetX();
        m_bound_end_y = event.GetY();
        HighlightAllInBoundingRect(event.ControlDown() && event.ShiftDown());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
        return;
    }
    else if (m_wheel_down)
    {
        xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Wheel down");
        float new_x = event.GetX() - m_previous_mouse_x;
        float new_y = event.GetY() - m_previous_mouse_y;
        // account for grid rotation
        float angleX = glm::radians(modelPreview->GetCameraRotationX());
        float angleY = glm::radians(modelPreview->GetCameraRotationY());
        float delta_x = 0.0f;
        float delta_y = 0.0f;
        float delta_z = 0.0f;
        bool top_view = (angleX > glm::radians(45.0f)) && (angleX < glm::radians(135.0f));
        bool bottom_view = (angleX > glm::radians(225.0f)) && (angleX < glm::radians(315.0f));
        bool upside_down_view = (angleX >= glm::radians(135.0f)) && (angleX <= glm::radians(225.0f));
        if( top_view ) {
            delta_x = new_x * std::cos(angleY) - new_y * std::sin(angleY);
            delta_z = new_y * std::cos(angleY) + new_x * std::sin(angleY);
        } else if( bottom_view ) {
            delta_x = new_x * std::cos(angleY) + new_y * std::sin(angleY);
            delta_z = -new_y * std::cos(angleY) + new_x * std::sin(angleY);
        } else {
            delta_x = new_x * std::cos(angleY);
            delta_y = new_y;
            delta_z = new_x * std::sin(angleY);
            if( !upside_down_view ) {
                delta_y *= -1.0f;
            }
        }
        delta_x *= modelPreview->GetZoom() * 2.0f;
        delta_y *= modelPreview->GetZoom() * 2.0f;
        delta_z *= modelPreview->GetZoom() * 2.0f;
        modelPreview->SetPan(delta_x, delta_y, delta_z);
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
    }
    else if (m_mouse_down) {
        if (m_moving_handle) {
            xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Mouse down moving handle");
            if (m_dragSession) {
                if (!xlights->AbortRender()) return;
                handles::WorldRay ray{};
                GetMouseLocation(event.GetX(), event.GetY(), ray.origin, ray.direction);
                auto result = m_dragSession->Update(ray, ModsFromEvent(event));
                // NeedsInit: rebuild Nodes when a vertex / curve move changes the
                // spline. Without this, polyline lights stay at their pre-drag
                // positions during creation and vertex moves.
                if (result == handles::UpdateResult::NeedsInit) {
                    if (Model* mModel = dynamic_cast<Model*>(selectedBaseObject)) {
                        mModel->Setup();
                        mModel->IncrementChangeCount();
                    }
                }
                if (result == handles::UpdateResult::Updated ||
                    result == handles::UpdateResult::NeedsInit) {
                    if (selectedBaseObject != nullptr) {
                        auto pos = selectedBaseObject->GetBaseObjectScreenLocation().GetCenterPosition();
                        xlights->SetStatusText(wxString::Format(
                            "x=%.2f y=%.2f z=%.2f %s",
                            pos.x, pos.y, pos.z,
                            selectedBaseObject->GetDimension()));

                        // Multi-select group-drag: routes by the dragged handle's role
                        // (arrow → translate, ring → rotate, cube → scale).
                        const int selectedModelCnt      = ModelsSelectedCount();
                        const int selectedViewObjectCnt = ViewObjectsSelectedCount();
                        const bool multiSel = (selectedModelCnt > 1 || selectedViewObjectCnt > 1);
                        if (multiSel) {
                            auto& sloc = selectedBaseObject->GetBaseObjectScreenLocation();
                            const handles::Role dragRole = m_dragSession->GetHandleId().role;
                            if (dragRole == handles::Role::AxisArrow) {
                                glm::vec3 new_centerpos = sloc.GetCenterPosition();
                                glm::vec3 pos_offset    = new_centerpos - last_centerpos;
                                for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
                                    Model* mm = modelPreview->GetModels()[i];
                                    if ((mm->GroupSelected() || mm->Selected()) && mm != selectedBaseObject) {
                                        mm->AddOffset(pos_offset.x, pos_offset.y, pos_offset.z);
                                    }
                                }
                                for (const auto& it : xlights->AllObjects) {
                                    ViewObject* vo = it.second;
                                    if ((vo->GroupSelected() || vo->Selected()) && vo != selectedBaseObject) {
                                        vo->AddOffset(pos_offset.x, pos_offset.y, pos_offset.z);
                                    }
                                }
                                last_centerpos = new_centerpos;
                            } else if (dragRole == handles::Role::AxisRing) {
                                // Wrap-aware rotation delta. X is negated to match
                                // RotateAboutPoint's handedness.
                                glm::vec3 new_worldrotate = sloc.GetRotationAngles();
                                glm::vec3 rotate_offset   = new_worldrotate - last_worldrotate;
                                if (rotate_offset.x >  180.0f) rotate_offset.x -= 360.0f;
                                if (rotate_offset.y >  180.0f) rotate_offset.y -= 360.0f;
                                if (rotate_offset.z >  180.0f) rotate_offset.z -= 360.0f;
                                if (rotate_offset.x < -180.0f) rotate_offset.x += 360.0f;
                                if (rotate_offset.y < -180.0f) rotate_offset.y += 360.0f;
                                if (rotate_offset.z < -180.0f) rotate_offset.z += 360.0f;
                                rotate_offset.x = -rotate_offset.x;
                                glm::vec3 active_handle_position = sloc.GetActiveHandlePosition();
                                for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
                                    Model* mm = modelPreview->GetModels()[i];
                                    if ((mm->GroupSelected() || mm->Selected()) && mm != selectedBaseObject) {
                                        xlights->AbortRender();
                                        mm->RotateAboutPoint(active_handle_position, rotate_offset);
                                    }
                                }
                                for (const auto& it : xlights->AllObjects) {
                                    ViewObject* vo = it.second;
                                    if ((vo->GroupSelected() || vo->Selected()) && vo != selectedBaseObject) {
                                        xlights->AbortRender();
                                        vo->RotateAboutPoint(active_handle_position, rotate_offset);
                                    }
                                }
                                last_worldrotate = new_worldrotate;
                            } else if (dragRole == handles::Role::AxisCube) {
                                glm::vec3 new_worldscale = sloc.GetScaleMatrix();
                                if (last_worldscale.x == 0 || last_worldscale.y == 0 || last_worldscale.z == 0) {
                                    spdlog::critical("11 multi-select scale: last_worldscale has a zero component");
                                } else {
                                    glm::vec3 scale_offset = new_worldscale / last_worldscale;
                                    for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
                                        Model* mm = modelPreview->GetModels()[i];
                                        if ((mm->GroupSelected() || mm->Selected()) && mm != selectedBaseObject) {
                                            mm->Scale(scale_offset);
                                        }
                                    }
                                    for (const auto& it : xlights->AllObjects) {
                                        ViewObject* vo = it.second;
                                        if ((vo->GroupSelected() || vo->Selected()) && vo != selectedBaseObject) {
                                            vo->Scale(scale_offset);
                                        }
                                    }
                                }
                                last_worldscale = new_worldscale;
                            }
                        }
                    }
                    xlights->GetOutputModelManager()->AddASAPWork(
                        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                            OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW,
                        "LayoutPanel::OnPreviewMouseMove3D-NewAPI");
                }
                m_last_mouse_x = event.GetX();
                m_last_mouse_y = event.GetY();
                return;
            }
        }
        else {
        xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Mouse down NOT moving handle");
            int delta_x = event.GetPosition().x - m_last_mouse_x;
            int delta_y = event.GetPosition().y - m_last_mouse_y;
            modelPreview->SetCameraView(delta_x, delta_y, false);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
        }
    }
    else {
        if (!selectionLatched) {
            xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Not selection latched");
            glm::vec3 ray_origin;
            glm::vec3 ray_direction;
            GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
            BaseObject* which_object = nullptr;
            float distance = 1000000000.0f;
            float intersection_distance = 1000000000.0f;
            if( editing_models ) {
                xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Not selection latched - Editing models");
                for (const auto& it : modelPreview->GetModels())
                {
                    if (it->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                        if (intersection_distance < distance) {
                            distance = intersection_distance;
                            which_object = it;
                        }
                    }
                }
            } else {
                xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Not selection latched - Not editing models");
                for (const auto& it : xlights->AllObjects) {
                    ViewObject *view_object = it.second;
                    if (view_object->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                        if (intersection_distance < distance) {
                            distance = intersection_distance;
                            which_object = view_object;
                        }
                    }
                }
            }
            if (which_object == nullptr)
            {
                xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Not selection latched - Not editing models - AAA");
                if (highlightedBaseObject != nullptr) {
                    highlightedBaseObject->Highlighted(false);
                    highlightedBaseObject = nullptr;
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
                }
            }
            else
            {
                xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Not selection latched - Not editing models - BBB");
                if (which_object != last_selection) {
                    UnSelectAllModels();
                    highlightedBaseObject = which_object;
                    highlightedBaseObject->Highlighted(true);
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
                }
            }
            last_selection = which_object;
        }
        xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D CCC");
        if (m_moving_handle)
        {
            xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Moving handle");
            BaseObject *obj = _newModel;
            if (obj == nullptr) {
                obj = selectedBaseObject;
                Model* m = dynamic_cast<Model*>(obj);
                if (obj == nullptr || (!xlights->AllModels.IsModelValid(m) && _newModel != obj)) return;
            }
            // descriptor session takes the move stream when
            // active. This branch fires for the polyline "trail vertex
            // with cursor" path between clicks (mouse-up cleared
            // m_mouse_down but m_moving_handle is still true).
            if (m_dragSession) {
                if (!xlights->AbortRender()) return;
                handles::WorldRay ray{};
                GetMouseLocation(event.GetX(), event.GetY(), ray.origin, ray.direction);
                auto result = m_dragSession->Update(ray, ModsFromEvent(event));
                if (result == handles::UpdateResult::NeedsInit) {
                    if (Model* mModel = dynamic_cast<Model*>(obj)) {
                        mModel->Setup();
                        mModel->IncrementChangeCount();
                    }
                }
                if (result == handles::UpdateResult::Updated ||
                    result == handles::UpdateResult::NeedsInit) {
                    auto pos = obj->GetBaseObjectScreenLocation().GetCenterPosition();
                    xlights->SetStatusText(wxString::Format(
                        "x=%.2f y=%.2f z=%.2f %s",
                        pos.x, pos.y, pos.z, obj->GetDimension()));
                    xlights->GetOutputModelManager()->AddASAPWork(
                        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                            OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW,
                        "LayoutPanel::OnPreviewMouseMove3D-NewAPI-trail");
                }
                return;
            }
        }
        else {
            Model* m = dynamic_cast<Model*>(selectedBaseObject);
            if (selectedBaseObject != nullptr && (_newModel == selectedBaseObject || xlights->AllModels.IsModelValid(m))) {
                xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Moving but no model selected");
                glm::vec3 ray_origin;
                glm::vec3 ray_direction;
                GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
                // hover hit-test through the descriptor system.
                // The descriptor hit's role drives the cursor; the Id
                // is also stored on `m_over_handle` so the click
                // handler can route by descriptor identity.
                CursorType hoverCursor = CursorType::Default;
                std::optional<handles::Id> hoverId;
                if (m != nullptr) {
                    auto& sloc = selectedBaseObject->GetBaseObjectScreenLocation();
                    const auto legacyToolHover = sloc.GetAxisTool();
                    handles::Tool hoverTool = handles::Tool::Translate;
                    bool hoverToolSupported = false;
                    switch (legacyToolHover) {
                        case ModelScreenLocation::MSLTOOL::TOOL_TRANSLATE: hoverTool = handles::Tool::Translate;   hoverToolSupported = true; break;
                        case ModelScreenLocation::MSLTOOL::TOOL_SCALE:     hoverTool = handles::Tool::Scale;       hoverToolSupported = true; break;
                        case ModelScreenLocation::MSLTOOL::TOOL_ROTATE:    hoverTool = handles::Tool::Rotate;      hoverToolSupported = true; break;
                        case ModelScreenLocation::MSLTOOL::TOOL_XY_TRANS:  hoverTool = handles::Tool::XYTranslate; hoverToolSupported = true; break;
                        case ModelScreenLocation::MSLTOOL::TOOL_ELEVATE:   hoverTool = handles::Tool::Elevate;     hoverToolSupported = true; break;
                        default: break;
                    }
                    if (hoverToolSupported) {
                        const float zoom = modelPreview->GetCameraZoomForHandles();
                        const int hscale = modelPreview->GetHandleScale();
                        handles::ViewParams hoverView;
                        hoverView.axisArrowLength = sloc.GetAxisArrowLength(zoom, hscale);
                        hoverView.axisHeadLength  = sloc.GetAxisHeadLength(zoom, hscale);
                        hoverView.axisRadius      = sloc.GetAxisRadius(zoom, hscale);
                        const auto hoverDescs = m->GetHandles(handles::ViewMode::ThreeD, hoverTool, hoverView);
                        if (!hoverDescs.empty()) {
                            handles::ScreenProjection hoverProj{
                                modelPreview->GetProjViewMatrix(),
                                modelPreview->getWidth(),
                                modelPreview->getHeight() };
                            handles::HitTestOptions hoverOpts;
                            hoverOpts.handleTolerance = (hoverTool == handles::Tool::XYTranslate ||
                                                          hoverTool == handles::Tool::Elevate)
                                                          ? 28.0f : 8.0f;
                            glm::vec2 hoverPx{ static_cast<float>(event.GetX()),
                                               static_cast<float>(event.GetY()) };
                            if (auto h = handles::HitTest(hoverDescs, hoverProj, hoverPx, hoverOpts)) {
                                hoverCursor = (h->id.role == handles::Role::Segment)
                                              ? CursorType::Default
                                              : CursorType::Hand;
                                hoverId = h->id;
                            }
                        }
                    }
                }
                m_over_handle = hoverId;
                modelPreview->SetCursor(CursorTypeToWx(hoverCursor));
                if (m_over_handle != over_handle) {
                    selectedBaseObject->GetBaseObjectScreenLocation().MouseOverHandle(m_over_handle);
                    over_handle = m_over_handle;
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
                } else if( event.ControlDown() ) {
                    // For now require control to be active before we start highlighting other models while a model is selected otherwise
                    // it gets hard to work on selected model with everything else highlighting.
                    // See if hovering over a model and if so highlight it or remove highlight as you leave it if it wasn't selected.
                    BaseObject* which_object = nullptr;
                    float distance = 1000000000.0f;
                    float intersection_distance = 1000000000.0f;
                    if( editing_models ) {
                        for (const auto& it : modelPreview->GetModels()) {
                            if (it != selectedBaseObject) {
                                if (it->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                                    if (intersection_distance < distance) {
                                        distance = intersection_distance;
                                        which_object = it;
                                    }
                                }
                            }
                        }
                    } else {
                        for (const auto& it : xlights->AllObjects) {
                            ViewObject *view_object = it.second;
                            if (view_object->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                                if (intersection_distance < distance) {
                                    distance = intersection_distance;
                                    which_object = view_object;
                                }
                            }
                        }
                    }
                    if (which_object != nullptr)
                    {
                        if (last_highlight != which_object) {
                            if (!which_object->Highlighted()) {
                                which_object->Highlighted(true);
                            }
                            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
                        }
                    }
                    if (last_highlight != nullptr && last_highlight != which_object) {
                        if (last_highlight->Highlighted() &&
                            !(last_highlight->Selected() || last_highlight->GroupSelected())) {
                            last_highlight->Highlighted(false);
                            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
                        }
                    }
                    last_highlight = which_object;
                }
            }
        }
    }
}

void LayoutPanel::OnPreviewMouseMove(wxMouseEvent& event)
{
    if (is_3d) {
        OnPreviewMouseMove3D(event);
        return;
    }

    /*/ FIXME:  Delete when not needed for debugging
    /////////////////////////////////////
    // temporary for debugging
    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        Model* whichModel = modelPreview->GetModels()[i];
        if (modelPreview->GetModels()[i]->GetBaseObjectScreenLocation().HitTest(ray_origin, ray_direction)) {
            whichModel->Highlighted(true);
        }
        else {
            whichModel->Highlighted(false);
        }
        UpdatePreview();
    }
    /////////////////////////////////////*/

    if (m_creating_bound_rect) {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        m_bound_end_x = ray_origin.x;
        m_bound_end_y = ray_origin.y;
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove");
        return;
    }
    else if (m_wheel_down) {
        float delta_x = event.GetX() - m_previous_mouse_x;
        float delta_y = event.GetY() - m_previous_mouse_y;
        delta_x /= modelPreview->GetZoom();
        delta_y /= modelPreview->GetZoom();
        modelPreview->SetPan(delta_x, -delta_y, 0.0f);
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove");
    }

    Model* m = _newModel;
    if (m == nullptr) {
        m = dynamic_cast<Model*>(selectedBaseObject);
        if (m == nullptr) return;
    }

    if (m_moving_handle) {
        if (!xlights->AbortRender()) return;
        if (m_dragSession) {
            handles::WorldRay ray{};
            GetMouseLocation(event.GetX(), event.GetY(), ray.origin, ray.direction);
            auto result = m_dragSession->Update(ray, ModsFromEvent(event));
            if (result == handles::UpdateResult::NeedsInit) {
                m->Setup();
                m->IncrementChangeCount();
            }
            if (result == handles::UpdateResult::Updated ||
                result == handles::UpdateResult::NeedsInit) {
                xlights->SetStatusText(wxString::Format("x=%.2f y=%.2f",
                    m->GetBaseObjectScreenLocation().GetCenterPosition().x,
                    m->GetBaseObjectScreenLocation().GetCenterPosition().y));
                xlights->GetOutputModelManager()->AddASAPWork(
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW,
                    "LayoutPanel::OnPreviewMouseMove-NewAPI");
            }
            return;
        }
    }
    else if (m_dragging && event.Dragging()) {
        double delta_x = (double)event.GetX() - m_previous_mouse_x;
        double delta_y = -((double)event.GetY() - m_previous_mouse_y);

        auto scale = modelPreview->GetCurrentScaleFactor();
        delta_x /= modelPreview->GetZoom() / scale;
        delta_y /= modelPreview->GetZoom() / scale;
        int wi, ht;
        modelPreview->GetVirtualCanvasSize(wi, ht);
        if (wi > 0 && ht > 0) {
            for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
                if (modelPreview->GetModels()[i]->Selected() || modelPreview->GetModels()[i]->GroupSelected()) {
                    if(!m->IsLocked()) {
                        CreateUndoPoint("SingleModel", m->name, "location");
                    }
                    modelPreview->GetModels()[i]->AddOffset(delta_x, delta_y, 0.0);
                    //SetupPropGrid(modelPreview->GetModels()[i]);
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewMouseMove");
                    // dont need these until finished moving
                    //xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPreviewMouseMove");
                    //xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnPreviewMouseMove");
                }
            }
        }
        m_previous_mouse_x = event.GetPosition().x;
        m_previous_mouse_y = event.GetPosition().y;
        xlights->SetStatusText(wxString::Format("x=%d y=%d", m_previous_mouse_x, m_previous_mouse_y));
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove");
    }
    else {
        if (m->Selected()) {
            extern CursorType GetResizeCursor(int cornerIndex, int PreviewRotation);
            handles::ViewParams view2d;
            const auto descs = m->GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate, view2d);
            CursorType hoverCur = CursorType::Default;
            std::optional<handles::Id> hoverId;
            if (!descs.empty()) {
                handles::ScreenProjection proj{
                    modelPreview->GetProjViewMatrix(),
                    modelPreview->getWidth(),
                    modelPreview->getHeight() };
                handles::HitTestOptions opts;
                opts.handleTolerance = 8.0f;
                glm::vec2 mousePx{ static_cast<float>(event.GetX()),
                                   static_cast<float>(event.GetY()) };
                if (auto h = handles::HitTest(descs, proj, mousePx, opts)) {
                    hoverId = h->id;
                    switch (h->id.role) {
                        case handles::Role::Segment:
                            hoverCur = CursorType::Default;
                            break;
                        case handles::Role::ResizeCorner: {
                            // id.index uses the L_TOP/R_TOP/etc. constants directly.
                            const int rotZ = static_cast<int>(m->GetBaseObjectScreenLocation().GetRotateZ());
                            hoverCur = GetResizeCursor(h->id.index, rotZ);
                            break;
                        }
                        default:
                            hoverCur = CursorType::Hand;
                            break;
                    }
                }
            }
            m_over_handle = hoverId;
            modelPreview->SetCursor(CursorTypeToWx(hoverCur));
        }
    }
}

bool LayoutPanel::IsAllSelectedModelsArePixelProtocol() const
{
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
        if (modelPreview->GetModels()[i]->Selected() || modelPreview->GetModels()[i]->GroupSelected()) {
            if (!modelPreview->GetModels()[i]->IsPixelProtocol()) {
                return false;
            }
        }
    }
    return true;
}

void LayoutPanel::AddSingleModelOptionsToBaseMenu(wxMenu &menu) {

    if (selectedBaseObject == nullptr)
        return;

    int selectedObjectCnt = editing_models ? ModelsSelectedCount() : ViewObjectsSelectedCount();

    if (!selectedBaseObject->GetBaseObjectScreenLocation().IsLocked() && !selectedBaseObject->IsFromBase())
    {
        bool need_sep = false;
        if( editing_models ) {
            Model* model = dynamic_cast<Model*>(selectedBaseObject);
            int sel_seg = model->GetSelectedSegment();
            if( sel_seg != -1 ) {
                menu.Append(ID_PREVIEW_MODEL_SET_SEGMENTS,"Enter Segment Size");
                if( !model->HasCurve(sel_seg) ) {
                    menu.Append(ID_PREVIEW_MODEL_ADDPOINT,"Add Point");
                    if( model->SupportsCurves() ) {
                        menu.Append(ID_PREVIEW_MODEL_ADDCURVE,"Define Curve");
                    }
                } else {
                    menu.Append(ID_PREVIEW_MODEL_DELCURVE,"Remove Curve");
                }
                need_sep = true;
            }
            // "Delete Point" only applies to vertex handles —
            // not the centre, not a curve control point, not a
            // segment hover.
            auto selectedId = model->GetSelectedHandleId();
            if (selectedId && selectedId->role == handles::Role::Vertex &&
                model->GetNumHandles() > 2) {
                menu.Append(ID_PREVIEW_MODEL_DELETEPOINT, "Delete Point");
                need_sep = true;
            }
        }
        if( need_sep ) {
            menu.AppendSeparator();
        }
        if( editing_models && (selectedBaseObject->GetDisplayAs() == DisplayAsType::Matrix ))
        {
            menu.Append(ID_PREVIEW_MODEL_ASPECTRATIO,"Correct Aspect Ratio");
        }
        if (is_3d && selectedObjectCnt == 1) {
            menu.Append(ID_PREVIEW_ALIGN_GROUND, "Align With Ground");
        }
    }
    if (editing_models)
    {
        bool anySelectedModelLocked = false;
        bool anySelectedModelUnlocked = false;
        bool allSelectedModelsFromBase = true;
        bool anySelectedModelFromBase = false;

        std::vector<Model*> selectedModels = GetSelectedModelsForEdit();
        for (const auto& it : selectedModels) {
            if (it->IsLocked())
                anySelectedModelLocked = true;
            else
                anySelectedModelUnlocked = true;

            if (!it->IsFromBase())
                allSelectedModelsFromBase = false;
            else
                anySelectedModelFromBase = true;
        }

        auto lm = menu.Append(ID_PREVIEW_MODEL_LOCK, "Lock");
        lm->Enable(anySelectedModelUnlocked && !allSelectedModelsFromBase);
        auto um = menu.Append(ID_PREVIEW_MODEL_UNLOCK, "Unlock");
        um->Enable(anySelectedModelLocked && !allSelectedModelsFromBase);
        auto ul = menu.Append(ID_PREVIEW_MODEL_UNLINKFROMBASE, "Unlink from base show folder");
        ul->Enable(anySelectedModelFromBase);
        
        Model* model = dynamic_cast<Model*>(selectedBaseObject);
        if (model != nullptr && model->GetDisplayAs() != DisplayAsType::ModelGroup && model->GetDisplayAs() != DisplayAsType::SubModel) {
            menu.Append(ID_PREVIEW_MODEL_NODELAYOUT, "Node Layout");
        }
        if (model->SupportsWiringView()) {
            menu.Append(ID_PREVIEW_MODEL_WIRINGVIEW, "Wiring View");
        }
        menu.AppendSeparator();
        if (model->SupportsExportAsCustom())
        {
            menu.Append(ID_PREVIEW_MODEL_EXPORTASCUSTOM, "Export as Custom xLights Model");
        }
        if (model->SupportsExportAsCustom3D()) {
            menu.Append(ID_PREVIEW_MODEL_EXPORTASCUSTOM3D, "Export as 3D Custom xLights Model");
        }
        menu.Append(ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL, "Export xLights Model");
#ifdef _DEBUG
        menu.Append(ID_PREVIEW_MODEL_CAD_EXPORT, "Export As DXF/STL/VRML");
#endif
        menu.Append(ID_PREVIEW_EXPORT_FACESSTATESSUBMODELS, "Export Faces/States/SubModels");
        menu.AppendSeparator();
        for (const auto& it : xlights->AllModels) {
            if (it.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
                menu.Append(ID_MNU_ADD_TO_EXISTING_GROUPS, "Add to Existing Groups");
                break;
            }
        }
        if (selectedTreeModels.size() == 1) {
            menu.Append(ID_MNU_REMOVE_FROM_EXISTING_GROUPS, "Remove from Existing Groups");
        }
        menu.Append(ID_PREVIEW_MODEL_CREATEGROUP, "Create Group");
        menu.AppendSeparator();
        menu.Append(ID_PREVIEW_FLIP_HORIZONTAL, "Flip Horizontal")->Enable(!selectedBaseObject->IsFromBase());
        menu.Append(ID_PREVIEW_FLIP_VERTICAL, "Flip Vertical")->Enable(!selectedBaseObject->IsFromBase());
        
        if ((selectedObjectCnt == 1) && (modelPreview->GetModels().size() > 1) && !selectedBaseObject->IsFromBase()) {
            menu.Append(ID_PREVIEW_REPLACEMODEL, "Replace A Model With This Model");
        }
    }
}

void LayoutPanel::AddBulkEditOptionsToMenu(wxMenu* mnuBulkEdit) {
    mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_SETACTIVE, "Active");
    mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_SETINACTIVE, "Inactive");
    if (editing_models) {
        if (xlights->GetOutputManager()->GetAutoLayoutControllerNames().size() > 0)
        {
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERNAME, "Controller Name");
        }
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_TAGCOLOUR, "Tag Color");
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_PIXELSIZE, "Pixel Size");
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_PIXELSTYLE, "Pixel Style");
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_TRANSPARENCY, "Transparency");
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_BLACKTRANSPARENCY, "Black Transparency");
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_SHADOWMODELFOR, "Shadow Model For");

        mnuBulkEdit->AppendSeparator();
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTION, "Controller Port");
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTIONINCREMENT, "Controller Port and Increment");
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERPROTOCOL, "Controller Protocol");
        if (IsAllSelectedModelsArePixelProtocol())
        {
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_SMARTREMOTE, "Controller Smart Remote");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_SMARTREMOTETYPE, "Controller Smart Remote Type");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERDIRECTION, "Controller Direction");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERBRIGHTNESS, "Controller Brightness");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERGAMMA, "Controller Gamma");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERCOLOURORDER, "Controller Colour Order");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERSTARTNULLNODES, "Controller Start Null Nodes");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERENDNULLNODES, "Controller End Null Nodes");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERGROUPCOUNT, "Controller Group Count");
            mnuBulkEdit->AppendSeparator();
        }
    }
    mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_PREVIEW, "Preview");
    if (editing_models) {
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_DIMMINGCURVES, "Dimming Curves");
    }
}

void LayoutPanel::AddAlignOptionsToMenu(wxMenu* mnuAlign) {
    mnuAlign->Append(ID_PREVIEW_ALIGN_TOP,"Top");
    mnuAlign->Append(ID_PREVIEW_ALIGN_BOTTOM,"Bottom");
    mnuAlign->Append(ID_PREVIEW_ALIGN_LEFT,"Left");
    mnuAlign->Append(ID_PREVIEW_ALIGN_RIGHT, "Right");
    if (is_3d) {
        mnuAlign->Append(ID_PREVIEW_ALIGN_FRONT, "Front");
        mnuAlign->Append(ID_PREVIEW_ALIGN_BACK, "Back");
        mnuAlign->Append(ID_PREVIEW_ALIGN_GROUND, "With Ground");
    }
    mnuAlign->Append(ID_PREVIEW_ALIGN_H_CENTER,"Horizontal Center");
    mnuAlign->Append(ID_PREVIEW_ALIGN_V_CENTER,"Vertical Center");
    if (is_3d) {
        mnuAlign->Append(ID_PREVIEW_ALIGN_D_CENTER, "Depth Center");
    }
}

void LayoutPanel::AddDistributeOptionsToMenu(wxMenu* mnuDistribute) {
    mnuDistribute->Append(ID_PREVIEW_H_DISTRIBUTE,"Horizontal");
    mnuDistribute->Append(ID_PREVIEW_V_DISTRIBUTE,"Vertical");
    if (is_3d) {
        mnuDistribute->Append(ID_PREVIEW_D_DISTRIBUTE, "Depth");
    }
}

void LayoutPanel::AddResizeOptionsToMenu(wxMenu* mnuResize) {
    mnuResize->Append(ID_PREVIEW_RESIZE_SAMEWIDTH, "Match Width");
    mnuResize->Append(ID_PREVIEW_RESIZE_SAMEHEIGHT, "Match Height");
    mnuResize->Append(ID_PREVIEW_RESIZE_SAMESIZE, "Match Size");
}

void LayoutPanel::OnPreviewRightDown(wxMouseEvent& event)
{
    modelPreview->SetFocus();
    wxMenu mnu;

    int selectedObjectCnt = editing_models ? ModelsSelectedCount() : ViewObjectsSelectedCount();

    if (selectedObjectCnt > 1)
    {
        wxMenu* mnuBulkEdit = new wxMenu();
        AddBulkEditOptionsToMenu(mnuBulkEdit);
        mnuBulkEdit->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

        wxMenu* mnuAlign = new wxMenu();
        AddAlignOptionsToMenu(mnuAlign);
        mnuAlign->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

        wxMenu* mnuDistribute = new wxMenu();
        AddDistributeOptionsToMenu(mnuDistribute);
        mnuDistribute->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

        wxMenu* mnuResize = new wxMenu();
        AddResizeOptionsToMenu(mnuResize);
        mnuResize->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

        mnu.Append(ID_PREVIEW_BULKEDIT, "Bulk Edit", mnuBulkEdit, "");
        mnu.Append(ID_PREVIEW_ALIGN, "Align", mnuAlign, "");
        mnu.Append(ID_PREVIEW_DISTRIBUTE, "Distribute", mnuDistribute, "");
        mnu.Append(ID_PREVIEW_RESIZE, "Resize", mnuResize, "");
        mnu.AppendSeparator();
    }
    if (!is_3d && selectedTreeGroups.size() == 1 && selectedTreeModels.size() == 0 && selectedTreeSubModels.size() == 0) {
        auto mg = GetSelectedModelGroup();
        if (xlights->AllModels.IsModelValid(mg)) {
            mnu.Append(ID_SET_CENTER_OFFSET, _("Set Center Offset Here"));
            mnu.AppendSeparator();
            m_previous_mouse_x = event.GetX();
            m_previous_mouse_y = event.GetY();
        }
    }

    if (selectedObjectCnt > 0) {
        AddSingleModelOptionsToBaseMenu(mnu);
    }

    if( currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned" ) {
        if (selectedObjectCnt > 0) {
            mnu.AppendSeparator();
        }
        mnu.Append(ID_PREVIEW_DELETE_ACTIVE,"Delete this Preview");
        mnu.Append(ID_PREVIEW_RENAME_ACTIVE, "Rename this Preview");
    }

    mnu.Append(ID_PREVIEW_SAVE_LAYOUT_IMAGE, _("Save Layout Image"));
    mnu.Append(ID_PREVIEW_PRINT_LAYOUT_IMAGE, _("Print Layout Image"));
    mnu.Append(ID_PREVIEW_IMPORTMODELSFROMRGBEFFECTS, _("Import Previews/Models/Groups"));
    mnu.Append(ID_PREVIEW_IMPORT_MODELS_FROM_LORS5, _("Import LOR S5 Models/Groups"));
    mnu.Append(ID_PREVIEW_LAYOUT_DXF_EXPORT, _("Export Layout As DXF"));

    // ViewPoint menus
    mnu.AppendSeparator();
    mnu.Append(ID_PREVIEW_RESET, "Reset");
    mnu.Append(ID_PREVIEW_VIEWPOINT_DEFAULT, _("Set Current ViewPoint as Default"));
    mnu.Append(ID_PREVIEW_VIEWPOINT_DEFAULT_RESTORE, _("Restore Default ViewPoint"));
    mnu.AppendSeparator();
    mnu.Append(ID_PREVIEW_SAVE_VIEWPOINT, _("Save Current ViewPoint"));
    if (is_3d) {
        if (xlights->viewpoint_mgr.GetNum3DCameras() > 0) {
            wxMenu* mnuViewPoint = new wxMenu();
            for (int i = 0; i < xlights->viewpoint_mgr.GetNum3DCameras(); ++i)
            {
                mnuViewPoint->Append(ID_PREVIEW_CAMERA_LOAD_BASE + i, xlights->viewpoint_mgr.GetCamera3D(i)->GetName());
            }
            mnu.Append(ID_PREVIEW_VIEWPOINT3D, "Load ViewPoint", mnuViewPoint, "");
            mnuViewPoint->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

            mnuViewPoint = new wxMenu();
            for (int i = 0; i < xlights->viewpoint_mgr.GetNum3DCameras(); ++i)
            {
                mnuViewPoint->Append(ID_PREVIEW_CAMERA_DELETE_BASE + i, xlights->viewpoint_mgr.GetCamera3D(i)->GetName());
            }
            mnuViewPoint->AppendSeparator();
            mnuViewPoint->Append(ID_PREVIEW_DELETEALLVIEWPOINTS3D, "Delete All");
            mnu.Append(ID_PREVIEW_DELETEVIEWPOINT3D, "Delete ViewPoint", mnuViewPoint, "");
            mnuViewPoint->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);
        }
    }
    else {
        if (xlights->viewpoint_mgr.GetNum2DCameras() > 0) {
            wxMenu* mnuViewPoint = new wxMenu();
            for (int i = 0; i < xlights->viewpoint_mgr.GetNum2DCameras(); ++i)
            {
                mnuViewPoint->Append(ID_PREVIEW_CAMERA_LOAD_BASE + i, xlights->viewpoint_mgr.GetCamera2D(i)->GetName());
            }
            mnu.Append(ID_PREVIEW_VIEWPOINT2D, "Load ViewPoint", mnuViewPoint, "");
            mnuViewPoint->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

            mnuViewPoint = new wxMenu();
            for (int i = 0; i < xlights->viewpoint_mgr.GetNum2DCameras(); ++i)
            {
                mnuViewPoint->Append(ID_PREVIEW_CAMERA_DELETE_BASE + i, xlights->viewpoint_mgr.GetCamera2D(i)->GetName());
            }
            mnu.Append(ID_PREVIEW_DELETEVIEWPOINT2D, "Delete ViewPoint", mnuViewPoint, "");
            mnuViewPoint->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);
        }
    }

    bool has3dState = is_3d
        || xlights->GetXmlSetting("LayoutMode3D", "0") == "1"
        || xlights->viewpoint_mgr.GetNum3DCameras() > 0
        || xlights->viewpoint_mgr.GetDefaultCamera3D() != nullptr;
    if (has3dState) {
        mnu.AppendSeparator();
        mnu.Append(ID_PREVIEW_REVERT_TO_2D, "Revert to 2D Layout");
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);
    PopupMenu(&mnu);
    modelPreview->SetFocus();
}

void LayoutPanel::OnPreviewModelPopup(wxCommandEvent& event)
{
    if (event.GetId() == ID_PREVIEW_RESET) {
        modelPreview->Reset();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_RESET");
    } else if (event.GetId() == ID_PREVIEW_REPLACEMODEL) {
        ReplaceModel();
    } else if (event.GetId() == ID_PREVIEW_ALIGN_TOP) {
        if (editing_models) {
            PreviewModelAlignTops();
        } else {
            objects_panel->PreviewObjectAlignTops();
        }
    } else if (event.GetId() == ID_PREVIEW_SAVE_LAYOUT_IMAGE) {
        PreviewSaveImage();
    } else if (event.GetId() == ID_PREVIEW_PRINT_LAYOUT_IMAGE) {
        PreviewPrintImage();
    } else if (event.GetId() == ID_PREVIEW_IMPORTMODELSFROMRGBEFFECTS) {
        ImportModelsFromRGBEffects();
    } else if (event.GetId() == ID_PREVIEW_IMPORT_MODELS_FROM_LORS5) {
        ImportModelsFromLORS5();
    } else if (event.GetId() == ID_PREVIEW_ALIGN_BOTTOM) {
        if (editing_models) {
            PreviewModelAlignBottoms();
        } else {
            objects_panel->PreviewObjectAlignBottoms();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_GROUND) {
        if (editing_models) {
            PreviewModelAlignWithGround();
        } else {
            objects_panel->PreviewObjectAlignWithGround();
        }
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTION ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERSTARTNULLNODES ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERENDNULLNODES ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERBRIGHTNESS ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERCOLOURORDER ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERGAMMA ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERGROUPCOUNT ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERDIRECTION ||
               event.GetId() == ID_PREVIEW_BULKEDIT_SMARTREMOTE ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERPROTOCOL ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTIONINCREMENT ||
               event.GetId() == ID_PREVIEW_BULKEDIT_SMARTREMOTETYPE) {
        BulkEditControllerConnection(event.GetId());
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERNAME) {
        BulkEditControllerName();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_SETACTIVE) {
        BulkEditActive(true);
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_SETINACTIVE) {
        BulkEditActive(false);
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_TAGCOLOUR) {
        BulkEditTagColour();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_PIXELSIZE) {
        BulkEditPixelSize();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_PIXELSTYLE) {
        BulkEditPixelStyle();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_TRANSPARENCY) {
        BulkEditTransparency();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_BLACKTRANSPARENCY) {
        BulkEditBlackTranparency();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_SHADOWMODELFOR) {
        BulkEditShadowModelFor();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_PREVIEW) {
        BulkEditControllerPreview();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_DIMMINGCURVES) {
        BulkEditDimmingCurves();
    } else if (event.GetId() == ID_PREVIEW_ALIGN_LEFT) {
        if (editing_models) {
            PreviewModelAlignLeft();
        } else {
            objects_panel->PreviewObjectAlignLeft();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_RIGHT) {
        if (editing_models) {
            PreviewModelAlignRight();
        } else {
            objects_panel->PreviewObjectAlignRight();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_FRONT) {
        if (editing_models) {
            PreviewModelAlignFronts();
        } else {
            objects_panel->PreviewObjectAlignFronts();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_BACK) {
        if (editing_models) {
            PreviewModelAlignBacks();
        } else {
            objects_panel->PreviewObjectAlignBacks();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_H_CENTER) {
        if (editing_models) {
            PreviewModelAlignHCenter();
        } else {
            objects_panel->PreviewObjectAlignHCenter();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_V_CENTER) {
        if (editing_models) {
            PreviewModelAlignVCenter();
        } else {
            objects_panel->PreviewObjectAlignVCenter();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_D_CENTER) {
        if (editing_models) {
            PreviewModelAlignDCenter();
        } else {
            objects_panel->PreviewObjectAlignDCenter();
        }
    } else if (event.GetId() == ID_PREVIEW_H_DISTRIBUTE) {
        if (editing_models) {
            PreviewModelHDistribute();
        } else {
            objects_panel->PreviewObjectHDistribute();
        }
    } else if (event.GetId() == ID_PREVIEW_V_DISTRIBUTE) {
        if (editing_models) {
            PreviewModelVDistribute();
        } else {
            objects_panel->PreviewObjectVDistribute();
        }
    } else if (event.GetId() == ID_PREVIEW_D_DISTRIBUTE) {
        if (editing_models) {
            PreviewModelDDistribute();
        } else {
            objects_panel->PreviewObjectDDistribute();
        }
    } else if (event.GetId() == ID_PREVIEW_RESIZE_SAMEWIDTH) {
        if (editing_models) {
            PreviewModelResize(true, false);
        } else {
            objects_panel->PreviewObjectResize(true, false);
        }
    } else if (event.GetId() == ID_PREVIEW_RESIZE_SAMEHEIGHT) {
        if (editing_models) {
            PreviewModelResize(false, true);
        } else {
            objects_panel->PreviewObjectResize(false, true);
        }
    } else if (event.GetId() == ID_PREVIEW_RESIZE_SAMESIZE) {
        if (editing_models) {
            PreviewModelResize(true, true);
        } else {
            objects_panel->PreviewObjectResize(true, true);
        }
    } else if (event.GetId() == ID_SET_CENTER_OFFSET) {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(m_previous_mouse_x, m_previous_mouse_y, ray_origin, ray_direction);
        auto mg = GetSelectedModelGroup();
        modelPreview->SetCenterOffset(mg, ray_origin.x, ray_origin.y);
        mg->RebuildBuffers();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                      OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                      OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnModelsPopup::ID_SET_CENTER_OFFSET", nullptr, nullptr, GetSelectedModelName());
    } else if (event.GetId() == ID_PREVIEW_MODEL_NODELAYOUT) {
        ShowNodeLayout();
    } else if (event.GetId() == ID_PREVIEW_MODEL_LOCK) {
        LockSelectedModels(true);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
                                                      OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_LOCK");
    } else if (event.GetId() == ID_PREVIEW_MODEL_UNLOCK) {
        LockSelectedModels(false);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
                                                      OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_UNLOCK");
    } else if (event.GetId() == ID_PREVIEW_MODEL_UNLINKFROMBASE) {
        UnlinkSelectedModels();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_UNLINKFROMBASE");
    } else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTASCUSTOM) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        {
            wxString name = wxString(md->GetName()).Trim(true).Trim(false);
            wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written
            wxString fn = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
            if (!fn.IsEmpty()) {
                FileSerializingVisitor visitor(ToStdString(fn), true /*exporting*/);
                if (!visitor.IsOpen())
                    DisplayError("Unable to create file " + ToStdString(fn));
                else
                    md->ExportAsCustomXModel(visitor);
            }
        }
    } else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTASCUSTOM3D) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        {
            wxString name = wxString(md->GetName()).Trim(true).Trim(false);
            wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written
            wxString fn = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
            if (!fn.IsEmpty()) {
                FileSerializingVisitor visitor(ToStdString(fn), true /*exporting*/);
                if (!visitor.IsOpen())
                    DisplayError("Unable to create file " + ToStdString(fn));
                else
                    md->ExportAsCustomXModel3D(visitor);
            }
        }
    } else if (event.GetId() == ID_PREVIEW_MODEL_CREATEGROUP) {
        CreateModelGroupFromSelected();
    } else if (event.GetId() == ID_MNU_ADD_TO_EXISTING_GROUPS) {
        AddSelectedToExistingGroups();
    } else if (event.GetId() == ID_MNU_REMOVE_FROM_EXISTING_GROUPS) {
        RemoveSelectedFromExistingGroups();
    } else if (event.GetId() == ID_PREVIEW_MODEL_WIRINGVIEW) {
        ShowWiring();
    } else if (event.GetId() == ID_PREVIEW_MODEL_CAD_EXPORT) {
        ExportModelAsCAD();
    } else if (event.GetId() == ID_PREVIEW_LAYOUT_DXF_EXPORT) {
        ExportLayoutDXF();
    } else if (event.GetId() == ID_PREVIEW_EXPORT_FACESSTATESSUBMODELS) {
        ExportFacesStatesSubModels();
    } else if (event.GetId() == ID_PREVIEW_MODEL_ASPECTRATIO) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int screen_wi = md->GetBaseObjectScreenLocation().GetMWidth();
        int screen_ht = md->GetBaseObjectScreenLocation().GetMHeight();
        float render_ht = md->GetBaseObjectScreenLocation().GetRenderHt();
        float render_wi = md->GetBaseObjectScreenLocation().GetRenderWi();
        float ht_ratio = render_ht / (float)screen_ht;
        float wi_ratio = render_wi / (float)screen_wi;
        if (ht_ratio > wi_ratio) {
            render_wi = render_wi / ht_ratio;
            md->GetBaseObjectScreenLocation().SetMWidth((int)render_wi);
        } else {
            render_ht = render_ht / wi_ratio;
            md->GetBaseObjectScreenLocation().SetMHeight((int)render_ht);
        }
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
                                                      OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                                                      OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ASPECTRATIO", nullptr, nullptr, GetSelectedModelName());
    } else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL) {
        const Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        XmlSerializer serializer;
        wxString name = md->GetName();
        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (!filename.IsEmpty()) {
            ObtainAccessToURL(filename);
            pugi::xml_document doc = serializer.SerializeModel(md, true);
            doc.save_file(ToStdString(filename).c_str());
        }
    } else if (event.GetId() == ID_PREVIEW_DELETE_ACTIVE) {
        DeleteCurrentPreview();
    } else if (event.GetId() == ID_PREVIEW_RENAME_ACTIVE) {
        RenameCurrentPreview();
    } else if (event.GetId() == ID_PREVIEW_MODEL_ADDPOINT) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int handle = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(handle + 0x8000));
        md->InsertHandle(handle, modelPreview->GetCameraZoomForHandles(), modelPreview->GetHandleScale());
        md->Reinitialize();
        // SetupPropGrid(md);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                                                      OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ADDPOINT");
    } else if (event.GetId() == ID_PREVIEW_MODEL_DELETEPOINT) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        auto selectedId = md->GetSelectedHandleId();
        if (selectedId && selectedId->role == handles::Role::Vertex &&
            md->GetNumHandles() > 2) {
            // PolyPoint vertex int convention is 1-based.
            const int legacyHandle = selectedId->index + 1;
            CreateUndoPoint("SingleModel", md->name, std::to_string(legacyHandle + 0x4000));
            md->DeleteHandle(legacyHandle);
            md->SelectHandle(-1);
            md->GetModelScreenLocation().SelectSegment(-1);
            md->Reinitialize();
            // SetupPropGrid(md);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                                                          OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_DELETEPOINT");
        }
    } else if (event.GetId() == ID_PREVIEW_MODEL_ADDCURVE) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int seg = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(seg + 0x2000));
        md->SetCurve(seg, true);
        md->Reinitialize();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ADDCURVE");
    } else if (event.GetId() == ID_PREVIEW_MODEL_DELCURVE) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int seg = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(seg + 0x1000));
        md->SetCurve(seg, false);
        md->Reinitialize();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_DELCURVE");
    } else if (event.GetId() == ID_PREVIEW_MODEL_SET_SEGMENTS) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        PolyLineModel* pmd = dynamic_cast<PolyLineModel*>(md);
        if (md == nullptr || pmd == nullptr)
            return;
        int seg = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(seg + 0x2000));
        wxTextEntryDialog dlg(this, "Enter New Segment Size:");
        OptimiseDialogPosition(&dlg);
        if (dlg.ShowModal() == wxID_OK) {
            int size = (int)std::strtol(dlg.GetValue().ToStdString().c_str(), nullptr, 10);
            pmd->SetSegmentSize(seg, size);
            md->Reinitialize();
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ADDCURVE");
        }
    } else if (event.GetId() == ID_PREVIEW_VIEWPOINT_DEFAULT) {
        modelPreview->SaveDefaultCameraPosition();
    } else if (event.GetId() == ID_PREVIEW_VIEWPOINT_DEFAULT_RESTORE) {
        modelPreview->RestoreDefaultCameraPosition();
    } else if (event.GetId() == ID_PREVIEW_SAVE_VIEWPOINT) {
        modelPreview->SaveCurrentCameraPosition();
        SetDirtyHiLight(true);
    } else if (event.GetId() == ID_PREVIEW_FLIP_HORIZONTAL) {
        if (editing_models) {
            PreviewModelFlipH();
        } else {
            objects_panel->PreviewObjectFlipH();
        }
    } else if (event.GetId() == ID_PREVIEW_FLIP_VERTICAL) {
        if (editing_models) {
            PreviewModelFlipV();
        } else {
            objects_panel->PreviewObjectFlipV();
        }
    } else if (event.GetId() == ID_PREVIEW_REVERT_TO_2D) {
        if (wxMessageBox("Reset to 2D layout? This will remove all 3D Viewpoints and reset all model positions. This cannot be undone. Are you sure?",
                         "Revert to 2D Layout", wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION, this) == wxYES) {
            while (xlights->viewpoint_mgr.GetNum3DCameras() > 0)
                xlights->viewpoint_mgr.DeleteCamera3D(0);
            xlights->viewpoint_mgr.ClearDefault3DCamera();

            for (auto& [name, model] : xlights->AllModels) {
                if (model->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
                model->GetModelScreenLocation().SetWorldPos_Z(0.0f);
                model->GetModelScreenLocation().SetRotateX(0.0f);
                model->GetModelScreenLocation().SetRotateY(0.0f);
            }

            std::vector<std::string> toDelete;
            for (const auto& [name, obj] : xlights->AllObjects) {
                if (obj->GetDisplayAs() != DisplayAsType::Gridlines)
                    toDelete.push_back(name);
            }
            for (const auto& name : toDelete)
                xlights->AllObjects.Delete(name);

            xlights->SetXmlSetting("LayoutMode3D", "0");
            GetXLightsConfig()->Write("LayoutMode3D", false);

            if (is_3d) {
                is_3d = false;
                CheckBox_3D->SetValue(false);
                modelPreview->Set3D(false);
                editing_models = true;
                if (dynamic_cast<Model*>(selectedBaseObject) == nullptr)
                    UnSelectAllModels();
                Notebook_Objects->RemovePage(1);
                obj_button->Enable(false);
            }

            SetDirtyHiLight(true);
            xlights->GetOutputModelManager()->AddASAPWork(
                OutputModelManager::WORK_VISUAL_CHANGE | OutputModelManager::WORK_RELOAD_OBJECTLIST,
                "LayoutPanel::RevertTo2D");
        }
    } else if (event.GetId() == ID_PREVIEW_DELETEALLVIEWPOINTS3D) {
        if (wxMessageBox("Are you sure you want to delete all 3D viewpoints?\n\nThis action cannot be undone.", "Confirm Delete All Viewpoints", wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION, this) == wxYES) {
            while (xlights->viewpoint_mgr.GetNum3DCameras() > 0)
                xlights->viewpoint_mgr.DeleteCamera3D(0);
            SetDirtyHiLight(true);
        }
    } else if (is_3d) {
        long loadIdx = event.GetId() - ID_PREVIEW_CAMERA_LOAD_BASE;
        long deleteIdx = event.GetId() - ID_PREVIEW_CAMERA_DELETE_BASE;
        if (loadIdx >= 0 && loadIdx < xlights->viewpoint_mgr.GetNum3DCameras()) {
            modelPreview->SetCamera3D(loadIdx);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::3dCamera");
        } else if (deleteIdx >= 0 && deleteIdx < xlights->viewpoint_mgr.GetNum3DCameras()) {
            std::string viewpointName = xlights->viewpoint_mgr.GetCamera3D(deleteIdx)->GetName();
            wxString message = wxString::Format("Are you sure you want to delete the 3D viewpoint '%s'?\n\nThis action cannot be undone.", viewpointName);
            if (wxMessageBox(message, "Confirm Delete Viewpoint", wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION, this) == wxYES) {
                xlights->viewpoint_mgr.DeleteCamera3D(deleteIdx);
            }
        }
    } else {
        long loadIdx = event.GetId() - ID_PREVIEW_CAMERA_LOAD_BASE;
        long deleteIdx = event.GetId() - ID_PREVIEW_CAMERA_DELETE_BASE;
        if (loadIdx >= 0 && loadIdx < xlights->viewpoint_mgr.GetNum2DCameras()) {
            modelPreview->SetCamera2D(loadIdx);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::2dCamera");
        } else if (deleteIdx >= 0 && deleteIdx < xlights->viewpoint_mgr.GetNum2DCameras()) {
            std::string viewpointName = xlights->viewpoint_mgr.GetCamera2D(deleteIdx)->GetName();
            wxString message = wxString::Format("Are you sure you want to delete the 2D viewpoint '%s'?\n\nThis action cannot be undone.", viewpointName);
            if (wxMessageBox(message, "Confirm Delete Viewpoint", wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION, this) == wxYES) {
                xlights->viewpoint_mgr.DeleteCamera2D(deleteIdx);
            }
        }
    }
}

void LayoutPanel::PreviewModelAlignWithGround()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GroupSelected() || modelPreview->GetModels()[i]->Selected())
        {
            modelPreview->GetModels()[i]->SetBottom(0.0F);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "LayoutPanel::PreviewModelAlignWithGround");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::EditSubmodels()
{
    Model* md = dynamic_cast<Model*>(selectedBaseObject);
    if (md == nullptr || md->GetDisplayAs() == DisplayAsType::ModelGroup || md->GetDisplayAs() == DisplayAsType::SubModel)
        return;

    SubModelsDialog dlg(this, &xlights->_outputManager);
    dlg.Setup(md);
    if (dlg.ShowModal() == wxID_OK) {
        dlg.Save();
        md->IncrementChangeCount();
        md->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::EditSubmodels");
    }
    if (dlg.ReloadLayout) { //force grid to reload
        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(xLightsApp::GetFrame(), eventForceRefresh);
        md->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                        OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::EditSubmodels");
    }
}

void LayoutPanel::EditFaces()
{
    Model* md = dynamic_cast<Model*>(selectedBaseObject);
    if (md == nullptr || md->GetDisplayAs() == DisplayAsType::ModelGroup || md->GetDisplayAs() == DisplayAsType::SubModel)
        return;

    ModelFaceDialog dlg(this, &xlights->_outputManager);
    auto oldFaceInfo = md->GetFaceInfo();
    dlg.SetFaceInfo(md, oldFaceInfo);
    if (dlg.ShowModal() == wxID_OK) {
        auto newFaceInfo = dlg.GetFaceInfo();
        if (newFaceInfo != oldFaceInfo) {
            md->SetFaceInfo(newFaceInfo);
            md->IncrementChangeCount();
            md->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::EditFaces");
        }
    }
}

void LayoutPanel::EditStates()
{
    Model* md = dynamic_cast<Model*>(selectedBaseObject);
    if (md == nullptr || md->GetDisplayAs() == DisplayAsType::ModelGroup || md->GetDisplayAs() == DisplayAsType::SubModel)
        return;

    ModelStateDialog dlg(this, &xlights->_outputManager);
    dlg.SetStateInfo(md, md->GetStateInfo());
    if (dlg.ShowModal() == wxID_OK) {
        md->SetStateInfo(dlg.GetStateInfo());
        md->IncrementChangeCount();
        md->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::EditStates");
    }
}

void LayoutPanel::EditModelData()
{
    CustomModel* md = dynamic_cast<CustomModel*>(selectedBaseObject);
    if (md == nullptr || md->GetDisplayAs() != DisplayAsType::Custom)
        return;

    md->SaveDisplayDimensions();
    auto oldAutoSave = xLightsApp::GetFrame()->_suspendAutoSave;
    xLightsApp::GetFrame()->_suspendAutoSave = true; // because we will tamper with model we need to suspend autosave
    CustomModelDialog dlg(this, &xlights->_outputManager);
    dlg.Setup(md);
    if (dlg.ShowModal() == wxID_OK) {
        dlg.Save(md);
        md->RestoreDisplayDimensions();
        md->IncrementChangeCount();
        md->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::EditModelData");
    } else {
        md->RestoreDisplayDimensions();
        xLightsApp::GetFrame()->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CustomModel::CancelCustomData");
    }
    xLightsApp::GetFrame()->_suspendAutoSave = oldAutoSave;
}

void LayoutPanel::ShowNodeLayout()
{
    Model* md = dynamic_cast<Model*>(selectedBaseObject);
    if (md == nullptr || md->GetDisplayAs() == DisplayAsType::ModelGroup || md->GetDisplayAs() == DisplayAsType::SubModel) return;
    wxString html = md->ChannelLayoutHtml(xlights->GetOutputManager(), IsDarkMode());
    ChannelLayoutDialog dialog(this);
    dialog.SetHtmlSource(html);
    dialog.ShowModal();
}

void LayoutPanel::ShowWiring()
{
    Model* md = dynamic_cast<Model*>(selectedBaseObject);
    if (md == nullptr || md->GetDisplayAs() == DisplayAsType::ModelGroup || md->GetDisplayAs() == DisplayAsType::SubModel) return;
    WiringDialog dlg(this, md->GetName());
    dlg.SetData(md);
    dlg.ShowModal();
}

void LayoutPanel::ExportModelAsCAD()
{
    Model* md = dynamic_cast<Model*>(selectedBaseObject);
    if (md == nullptr || md->GetDisplayAs() == DisplayAsType::ModelGroup || md->GetDisplayAs() == DisplayAsType::SubModel) return;

#ifdef __WXOSX__
    static std::string const filter = "DXF STL VRML File (*.dxf;*.stl;*.wrl)|*.dxf;*.stl;*.wrl";
#else
    static std::string const filter = "DXF File (*.dxf)|*.dxf|STL File (*.stl)|*.stl|VRML File (*.wrl)|*.wrl";
#endif

    wxString const filename = wxFileSelector(_("Choose Output File"), wxEmptyString, md->GetName(), wxEmptyString, filter, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename != "") {
        wxFileName file(filename);
        if (ModelToCAD::ExportCAD(md, filename, file.GetExt())) {
            xlights->SetStatusText(wxString::Format("Exported '%s' Successfully", filename));
        } else {
            xlights->SetStatusText(wxString::Format("Export Failed '%s'", filename));
        }
    }
}

void LayoutPanel::ExportLayoutDXF()
{
    wxString const filename = wxFileSelector(_("Choose output file"), wxEmptyString, currentLayoutGroup + "_Layout", wxEmptyString, "DXF File (*.dxf)|*.dxf", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename != "") {
        if (ModelToCAD::ExportCAD(&xlights->AllModels, filename, "dxf", currentLayoutGroup)) {
            xlights->SetStatusText(wxString::Format("Exported '%s' Successfully", filename));
        } else {
            xlights->SetStatusText(wxString::Format("Export Failed '%s'", filename));
        }
    }
}

void LayoutPanel::ExportFacesStatesSubModels() {
    Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
    if (selectedModel == nullptr || selectedModel->GetDisplayAs() == DisplayAsType::ModelGroup || selectedModel->GetDisplayAs() == DisplayAsType::SubModel)
        return;

    if (wxMessageBox("Are you sure you want to Export this model's Face/States/SubModels definitions to other models?\nThis will override all the other model's existing properties and there is no way to undo it.","Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
        return;
    }
    wxArrayString choices;
    
    for (const auto& model : modelPreview->GetModels()) {
        if (model->Name() == selectedBaseObject->Name() || model->GetDisplayAs() == DisplayAsType::Image || model->GetDisplayAs() == DisplayAsType::Label || model->GetDisplayAs() == DisplayAsType::ModelGroup)
            continue;
        choices.Add(model->Name());
    }

    wxMultiChoiceDialog dlg(this, "Export Face/States/SubModels to Other Models", "Choose Model(s)", choices);
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK) {
        std::map<std::string, std::map<std::string, std::string>> sourceFaces = selectedModel->GetFaceInfo();
        std::map<std::string, std::map<std::string, std::string>> sourceStates = selectedModel->GetStateInfo();

        for (auto const& idx : dlg.GetSelections()) {
            Model* targetModel = xlights->GetModel(choices.at(idx));
            targetModel->SetFaceInfo(sourceFaces);
            targetModel->SetStateInfo(sourceStates);

            // Copy submodels using copy constructor
            for (int i = 0; i < selectedModel->GetNumSubModels(); ++i) {
                const SubModel* sourceSubModel = dynamic_cast<const SubModel*>(selectedModel->GetSubModel(i));
                if (sourceSubModel != nullptr) {
                    // Create SubModel using copy constructor
                    SubModel* sm = new SubModel(targetModel, sourceSubModel);
                    targetModel->AddSubmodel(sm);                    
                }
            }
            targetModel->IncrementChangeCount();
        }
        xlights->MarkEffectsFileDirty();
    }
}

void LayoutPanel::PreviewModelAlignTops()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float top = modelPreview->GetModels()[selectedindex]->GetTop();
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected())
        {
            modelPreview->GetModels()[i]->SetTop(top);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignTops");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignBottoms()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float bottom = modelPreview->GetModels()[selectedindex]->GetBottom();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected())
        {
            modelPreview->GetModels()[i]->SetBottom(bottom);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignBottoms");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignLeft()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float left = modelPreview->GetModels()[selectedindex]->GetLeft();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected())
        {
            modelPreview->GetModels()[i]->SetLeft(left);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignLeft");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignFronts()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float front = modelPreview->GetModels()[selectedindex]->GetFront();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GroupSelected())
        {
            modelPreview->GetModels()[i]->SetFront(front);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignFronts");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignBacks()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float back = modelPreview->GetModels()[selectedindex]->GetBack();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GroupSelected())
        {
            modelPreview->GetModels()[i]->SetBack(back);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignBacks");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelResize(bool sameWidth, bool sameHeight)
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    if (!xlights->AbortRender()) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);

    Model* selectedModel = modelPreview->GetModels()[selectedindex];
    DisplayAsType selectedType = selectedModel->GetDisplayAs();
    float width = selectedModel->GetRestorableMWidth();
    float height = selectedModel->GetRestorableMHeight();
    float depth = selectedModel->GetRestorableMDepth();

    bool isBoxed = false;
    if ((dynamic_cast<ModelWithScreenLocation<BoxedScreenLocation>*>(selectedModel) != nullptr)) {
        isBoxed = true;
    }

    bool isCustom3d = false;
    std::string customFingerprint = "";

    // check if custom 3d and set model fingerprint
    if (selectedType == DisplayAsType::Custom) {
        CustomModel *cm = dynamic_cast<CustomModel*>(selectedModel);
        isCustom3d = cm && cm->GetCustomDepth() > 1;
        if (isCustom3d) {
            customFingerprint = cm->GetCustomData();
        }
    }

    for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
        if (modelPreview->GetModels()[i]->GroupSelected()) {
            Model* modelToResize = modelPreview->GetModels()[i];
            DisplayAsType modelType = modelToResize->GetDisplayAs();
            bool custom3dPrintsMatch = false;

            // if selected is Custom3d check if model fingerprints match
            if (isCustom3d && !customFingerprint.empty()) {
                CustomModel *cm = dynamic_cast<CustomModel*>(modelToResize);

                std::string mToResizeFingerprint = cm == nullptr ? "" : cm->GetCustomData();
                if (customFingerprint == mToResizeFingerprint) {
                    custom3dPrintsMatch = true;
                }
            }

            if ((isBoxed && selectedType == modelType && selectedType != DisplayAsType::Custom) || custom3dPrintsMatch) {
                // boxed model, types match and not a custom model OR custom 3d model and fingerprints matched so use scale matrix
                glm::vec3 matrixScale = selectedModel->GetModelScreenLocation().GetScaleMatrix();
                if (sameWidth && sameHeight) {
                    modelToResize->GetModelScreenLocation().SetScaleMatrix(matrixScale);
                } else if (sameWidth) {
                    float scaleY = ((BoxedScreenLocation&)modelToResize->GetModelScreenLocation()).GetScaleY();
                    ((BoxedScreenLocation&)modelToResize->GetModelScreenLocation()).SetScale(matrixScale.x, scaleY);
                    ((BoxedScreenLocation&)modelToResize->GetModelScreenLocation()).SetScaleZ(matrixScale.z);
                } else {
                    float scaleX = ((BoxedScreenLocation&)modelToResize->GetModelScreenLocation()).GetScaleX();
                    ((BoxedScreenLocation&)modelToResize->GetModelScreenLocation()).SetScale(scaleX, matrixScale.y);
                    ((BoxedScreenLocation&)modelToResize->GetModelScreenLocation()).SetScaleZ(matrixScale.z);
                }
            } else {
                // no special resizing, same as 2020.24 and prior
                bool z_scale = modelPreview->GetModels()[i]->GetBaseObjectScreenLocation().GetSupportsZScaling();

                if (sameWidth) {
                    modelPreview->GetModels()[i]->SetWidth(width);
                    if (z_scale) {
                        modelPreview->GetModels()[i]->GetBaseObjectScreenLocation().SetMDepth(depth);
                    }
                }

                if (sameHeight) {
                    modelToResize->SetHeight(height);
                }
            }
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignResize");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignRight()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float right = modelPreview->GetModels()[selectedindex]->GetRight();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected())
        {
            modelPreview->GetModels()[i]->SetRight(right);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignRight");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignHCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float center = modelPreview->GetModels()[selectedindex]->GetHcenterPos();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected())
        {
            modelPreview->GetModels()[i]->SetHcenterPos(center);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignHCenter");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignVCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float center = modelPreview->GetModels()[selectedindex]->GetVcenterPos();
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
        if (modelPreview->GetModels()[i]->GroupSelected())
        {
            modelPreview->GetModels()[i]->SetVcenterPos(center);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelVCenter");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignDCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float center = modelPreview->GetModels()[selectedindex]->GetDcenterPos();
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
        if (modelPreview->GetModels()[i]->GroupSelected())
        {
            modelPreview->GetModels()[i]->SetDcenterPos(center);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelDCenter");

    ReselectTreeModels(selectedModelPaths);
}

bool SortModelX(const Model* first, const Model* second)
{
    float firstmodelX = first->GetBaseObjectScreenLocation().GetHcenterPos();
    float secondmodelX = second->GetBaseObjectScreenLocation().GetHcenterPos();

    return firstmodelX < secondmodelX;
}

bool SortModelY(const Model* first, const Model* second)
{
    float firstmodelY = first->GetBaseObjectScreenLocation().GetVcenterPos();
    float secondmodelY = second->GetBaseObjectScreenLocation().GetVcenterPos();

    return firstmodelY < secondmodelY;
}

bool SortModelZ(const Model* first, const Model* second)
{
    float firstmodelZ = first->GetBaseObjectScreenLocation().GetDcenterPos();
    float secondmodelZ = second->GetBaseObjectScreenLocation().GetDcenterPos();

    return firstmodelZ < secondmodelZ;
}
void LayoutPanel::PreviewModelHDistribute()
{
    int count = 0;
    float minx = 999999;
    float maxx = -999999;

    std::list<Model*> models;

    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        Model* m = modelPreview->GetModels()[i];
        if (m->GroupSelected() || m->Selected())
        {
            count++;
            float x = m->GetHcenterPos();

            if (x < minx) minx = x;
            if (x > maxx) maxx = x;
            models.push_back(m);
        }
    }

    if (count <= 2) return;

    models.sort(SortModelX);

    float space = (maxx - minx) / (count - 1);

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", models.front()->name);

    float x = -1;
    for (const auto& it : models)
    {
        if (it == models.front())
        {
            x = it->GetHcenterPos() + space;
        }
        else if (it == models.back())
        {
            // do nothing
        }
        else
        {
            it->SetHcenterPos(x);
            x += space;
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelHDistribute");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelVDistribute()
{
    int count = 0;
    float miny = 999999;
    float maxy = -999999;

    std::list<Model*> models;

    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        Model* m = modelPreview->GetModels()[i];
        if (m->GroupSelected() || m->Selected())
        {
            count++;
            float y = m->GetVcenterPos();

            if (y < miny) miny = y;
            if (y > maxy) maxy = y;
            models.push_back(m);
        }
    }

    if (count <= 2) return;

    models.sort(SortModelY);

    float space = (maxy - miny) / (count - 1);

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", models.front()->name);

    float y = -1;
    for (const auto& it : models)
    {
        if (it == models.front())
        {
            y = it->GetVcenterPos() + space;
        }
        else if (it == models.back())
        {
            // do nothing
        }
        else
        {
            it->SetVcenterPos(y);
            y += space;
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelVDistribute");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelDDistribute()
{
    int count = 0;
    float minz = std::numeric_limits<float>::max();
    float maxz = std::numeric_limits<float>::lowest();

    std::list<Model*> models;

    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        Model* m = modelPreview->GetModels()[i];
        if (m->GroupSelected() || m->Selected())
        {
            count++;
            float z = m->GetDcenterPos();

            if (z < minz) minz = z;
            if (z > maxz) maxz = z;
            models.push_back(m);
        }
    }

    if (count <= 2) return;

    models.sort(SortModelZ);

    float space = (maxz - minz) / (count - 1);

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", models.front()->name);

    float z = -1;
    for (const auto& it : models)
    {
        if (it == models.front())
        {
            z = it->GetDcenterPos() + space;
        }
        else if (it == models.back())
        {
            // do nothing
        }
        else
        {
            it->SetDcenterPos(z);
            z += space;
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelDDistribute");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelFlipV() {
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) {
        return;
    }

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);

    for (auto model : modelPreview->GetModels()) {
        if (model->Selected()) {
            model->FlipVertical();
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelVCenter");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelFlipH() {
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) {
        return;
    }

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);

    for (auto model : modelPreview->GetModels()) {
        if (model->Selected()) {
            model->FlipHorizontal();
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelVCenter");

    ReselectTreeModels(selectedModelPaths);    
}

int LayoutPanel::GetSelectedModelIndex() const
{
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->Selected())
        {
            return i;
        }
    }
    return -1;
}

// Shortcut to get model pointer from tree item
Model* LayoutPanel::GetModelFromTreeItem(wxTreeListItem treeItem) {
    ModelTreeData *data = (ModelTreeData*)TreeListViewModels->GetItemData(treeItem);
    Model* model = ((data != nullptr) ? data->GetModel() : nullptr);
    return model;
}

// Select a Model in the tree, currently only selects top level model if found
void LayoutPanel::SelectModelInTree(Model* modelToSelect) {
    if (modelToSelect != nullptr && !_filterString.IsEmpty() && !ModelMatchesFilter(modelToSelect)) {
        wxCommandEvent dummy;
        OnModelFilterCancelBtn(dummy);
    }
    for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
          item.IsOk();
          item = TreeListViewModels->GetNextSibling(item) )
    {
        if (TreeListViewModels->GetItemData(item) != nullptr)
        {
            ModelTreeData *mitem = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
            if (mitem != nullptr && mitem->GetModel() == modelToSelect) {
                TreeListViewModels->Select(item);

                PlatformHandleSelectionChanged();
                TreeListViewModels->EnsureVisible(item);
                break;
            }
        }
    }
}

// Unselect a Model in the tree, currently only unselects top level model if found
void LayoutPanel::UnSelectModelInTree(Model* modelToUnSelect) {
    // clear any flags used for preview
    modelToUnSelect->Selected(false);
    modelToUnSelect->Highlighted(false);
    modelToUnSelect->GroupSelected(false);
    modelToUnSelect->SelectHandle(-1);
    modelToUnSelect->GetBaseObjectScreenLocation().SetActiveHandle(-1);

    for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
          item.IsOk();
          item = TreeListViewModels->GetNextSibling(item) )
    {
        if (TreeListViewModels->GetItemData(item) != nullptr)
        {
            ModelTreeData *mitem = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
            if (mitem != nullptr && mitem->GetModel() == modelToUnSelect) {
                TreeListViewModels->Unselect(item);
                PlatformHandleSelectionChanged();
                break;
            }
        }
    }
}

// Select a BaseObject in the model tree
void LayoutPanel::SelectBaseObjectInTree(BaseObject* baseObjectToSelect) {
    Model* modelToSelect = dynamic_cast<Model*>(baseObjectToSelect);
    SelectModelInTree(modelToSelect);
}

// Perform unselect of tree item by BaseObject
void LayoutPanel::UnSelectBaseObjectInTree(BaseObject* baseObjectToUnSelect) {
    Model* modelToUnSelect = dynamic_cast<Model*>(baseObjectToUnSelect);
    UnSelectModelInTree(modelToUnSelect);
}

// Get first tree item for a model, 'first' is the key word as a model pointer could exist in numerous
// places in model tree.
wxTreeListItem LayoutPanel::GetTreeItemFromModel(Model* model) {
    wxTreeListItem modelTreeItem = nullptr;
    for (wxTreeListItem item = TreeListViewModels->GetFirstItem(); item.IsOk(); item = TreeListViewModels->GetNextItem(item)) {
        ModelTreeData *data = (ModelTreeData*)TreeListViewModels->GetItemData(item);
        Model* itemModel = ((data != nullptr) ? data->GetModel() : nullptr);
        if (model == itemModel) {
            modelTreeItem = item;
            break;
        }
    }

    return modelTreeItem;
}

void LayoutPanel::UnSelectAllModelsInTree() {
    TreeListViewModels->UnselectAll();
    PlatformHandleSelectionChanged();
}

// Get unique models from selected tree model group included those deeply nested
std::vector<Model *> LayoutPanel::GetSelectedModelsFromGroup(wxTreeListItem groupItem, bool nested) {
    std::vector<Model *> groupModels;

    if (groupItem.IsOk()) {
        for (wxTreeListItem item = TreeListViewModels->GetFirstChild(groupItem);
              item.IsOk();
              item = TreeListViewModels->GetNextSibling(item) )
        {
            ModelTreeData *data = (ModelTreeData*)TreeListViewModels->GetItemData(item);
            Model* model = ((data != nullptr) ? data->GetModel() : nullptr);

            if (model->GetDisplayAs() == DisplayAsType::ModelGroup && nested == true) {
                std::vector<Model *> nestedModels = GetSelectedModelsFromGroup(item, true);
                for (Model* nestedModel: nestedModels) {
                    if (std::find(groupModels.begin(), groupModels.end(), nestedModel) == groupModels.end()) {
                        groupModels.push_back(nestedModel);
                    }
                }
            } else {
                if (std::find(groupModels.begin(), groupModels.end(), model) == groupModels.end()) {
                    groupModels.push_back(model);
                }
            }
        }

    }

    return groupModels;
}

// The will return unique selected models for edit, useful when groups are also selected in model tree
std::vector<Model*> LayoutPanel::GetSelectedModelsForEdit(bool incSubModels) {
    std::vector<Model*> modelsForEdit;

    for (const auto& groupItem : selectedTreeGroups) {
        std::vector<Model*> groupModels = GetSelectedModelsFromGroup(groupItem);
        for (Model* model: groupModels) {
            if (std::find(modelsForEdit.begin(), modelsForEdit.end(), model) == modelsForEdit.end()) {
                if (model->GetDisplayAs() != DisplayAsType::SubModel || incSubModels) {
                    modelsForEdit.push_back(model);
                }
            }
        }
    }

    for (const auto& modelItem : selectedTreeModels) {
        Model* model = GetModelFromTreeItem(modelItem);
        if (std::find(modelsForEdit.begin(), modelsForEdit.end(), model) == modelsForEdit.end()) {
            modelsForEdit.push_back(model);
        }
    }

    return modelsForEdit;
}

void LayoutPanel::SetTreeModelSelected(Model* model, bool isPrimary) {
    if (isPrimary) {
        model->Selected(true);
        model->GroupSelected(false);
        selectionLatched = true;
        selectedBaseObject = model;
        highlightedBaseObject = model;
        selectedBaseObject->Highlighted(true);
        selectedBaseObject->SelectHandle(-1);
        selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
        if (CheckBoxOverlap->GetValue() == true) {
            CheckModelForOverlaps(model);
        }
    } else {
        model->Selected(false);
        model->GroupSelected(true);
        model->Highlighted(true);
        model->SelectHandle(-1);
        model->GetBaseObjectScreenLocation().SetActiveHandle(-1);
    }
}

void LayoutPanel::SetTreeGroupModelsSelected(Model* model, bool isPrimary) {
    if (isPrimary) {
        selectedBaseObject = model;
        selectionLatched = true;
    }

    std::vector<Model *> groupModels = GetSelectedModelsFromGroup(GetTreeItemFromModel(model), true);

    for (const auto& m : groupModels) {
        m->GroupSelected(true);
        m->Highlighted(true);
        model->SelectHandle(-1);
        model->GetBaseObjectScreenLocation().SetActiveHandle(-1);
    }
}

void LayoutPanel::SetTreeSubModelSelected(Model* model, bool isPrimary) {
    if (isPrimary) {
        selectedBaseObject = model;
        highlightedBaseObject = model;
        selectionLatched = true;
    }
    model->GroupSelected(true);
    model->Highlighted(true);
    model->SelectHandle(-1);
    model->GetBaseObjectScreenLocation().SetActiveHandle(-1);
}

void LayoutPanel::CheckModelForOverlaps(Model* model) {
    // this is the channel range of the clicked on model
    int mStart = model->GetNumberFromChannelString(model->ModelStartChannel);
    int mEnd = model->GetLastChannel();

    for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
          item.IsOk();
          item = TreeListViewModels->GetNextSibling(item) ) {
        if (TreeListViewModels->GetItemData(item) != nullptr) {
            ModelTreeData *data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
            Model *mm = data != nullptr ? data->GetModel() : nullptr;
            if (mm != nullptr && mm != selectedBaseObject) {
                // this is the channel range of the model we are checking
                int startChan = mm->GetNumberFromChannelString(mm->ModelStartChannel);
                int endChan = mm->GetLastChannel();

                mm->Overlapping = ((mStart <= startChan && mEnd >= startChan) ||
                                   (mStart <= endChan && mEnd >= endChan) ||
                                   (mStart >= startChan && mEnd <= endChan));
            }
        }
    }
}

std::vector<std::list<std::string>> LayoutPanel::GetSelectedTreeModelPaths() {
    std::vector<std::list<std::string>> modelPaths;

    // Add selectedPrimaryTreeItem first if exists
    if (selectedPrimaryTreeItem != nullptr && selectedPrimaryTreeItem.IsOk()) {
        std::list<std::string> baseItemPath = GetTreeItemPath(selectedPrimaryTreeItem);
        modelPaths.push_back(baseItemPath);
    }

    for (const auto& item : selectedTreeGroups) {
        if (selectedPrimaryTreeItem != item) {
            std::list<std::string> modelPath = GetTreeItemPath(item);
            modelPaths.push_back(modelPath);
        }
    }

    for (const auto& item : selectedTreeModels) {
        if (selectedPrimaryTreeItem != item) {
            std::list<std::string> modelPath = GetTreeItemPath(item);
            modelPaths.push_back(modelPath);
        }
    }

    for (const auto& item : selectedTreeSubModels) {
        if (selectedPrimaryTreeItem != item) {
            std::list<std::string> modelPath = GetTreeItemPath(item);
            modelPaths.push_back(modelPath);
        }
    }

    return modelPaths;
}

std::list<std::string> LayoutPanel::GetTreeItemPath(wxTreeListItem item) {
    std::list<std::string> itemPath;
    wxTreeListItem root = TreeListViewModels->GetRootItem();
    itemPath.push_back(TreeListViewModels->GetItemText(item));
    wxTreeListItem parent = TreeListViewModels->GetItemParent(item);
    while (parent.IsOk() && parent != root) {
        itemPath.push_front(TreeListViewModels->GetItemText(parent));
        parent = TreeListViewModels->GetItemParent(parent);
    }

    return itemPath;
}

wxTreeListItem LayoutPanel::GetTreeItemBranch(wxTreeListItem parent, std::string branchName) {
    wxTreeListItem branch = TreeListViewModels->GetFirstChild(parent);
    bool branchFound = false;
    while(branch.IsOk() && !branchFound) {
        if (branchName == TreeListViewModels->GetItemText(branch)) {
            branchFound = true;
        } else {
            branch = TreeListViewModels->GetNextSibling(branch);
        }
    }

    return branch;
}

void LayoutPanel::ReselectTreeModels(std::vector<std::list<std::string>> modelPaths) {
    

    for (auto path : modelPaths) {
        // model name is last string in path
        std::string modelName = path.back();
        path.pop_back();
        wxTreeListItem branch = TreeListViewModels->GetRootItem();
        for (auto part : path) {
            branch = GetTreeItemBranch(branch, part);
        }

        if (branch != nullptr && branch.IsOk()) {
            for (wxTreeListItem child = TreeListViewModels->GetFirstChild(branch); child.IsOk(); child = TreeListViewModels->GetNextSibling(child)) {
                std::string childName = TreeListViewModels->GetItemText(child);
                if (TreeListViewModels->GetItemText(child) == modelName) {
                    TreeListViewModels->Select(child);
                    PlatformHandleSelectionChanged();
                    break;
                }
            }
        } else {
            spdlog::critical("LayoutPanel::ReselectTreeModels branch could not be found in tree... this shouldn't happen.");
        }
    }
}

int LayoutPanel::ModelsSelectedCount() const
{
    int selectedModelCount = 0;
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->Selected() || modelPreview->GetModels()[i]->GroupSelected())
        {
            selectedModelCount++;
        }
    }

    return selectedModelCount;
}

int LayoutPanel::ViewObjectsSelectedCount() const
{
    int selectedObjectCount = 0;
    for (const auto& it : xlights->AllObjects) {
        ViewObject *view_object = it.second;
        if(view_object->Selected() || view_object->GroupSelected())
        {
            selectedObjectCount++;
        }
    }
    return selectedObjectCount;
}


void LayoutPanel::OnSplitterWindowSashPosChanged(wxSplitterEvent& event)
{
    if (ModelGroupWindow == nullptr) {
        //event during creation
        return;
    }
    auto* config = GetXLightsConfig();
    config->Write("LayoutMainSplitterSash", event.GetSashPosition());
}

void LayoutPanel::OnNewModelTypeButtonClicked(wxCommandEvent& event) {
    for (const auto& it : buttons) {
        if (event.GetId() == it->GetId()) {
            if (it->GetModelType() == "Add Object") {
                DisplayAddObjectPopup();
            } else if (it->GetModelType() == "DMX") {
                selectedButton = it;
                DisplayAddDmxPopup();
            } else {
                int state = it->GetState();
                it->SetState(state + 1);
                if (it->GetState()) {
                    selectedButton = it;
                    UnSelectAllModels();
                    modelPreview->SetFocus();
                } else {
                    selectedButton = nullptr;
                    _lastXlightsModel = "";
                }
                Notebook_Objects->ChangeSelection(0);
                editing_models = true;
            }
        } else if (it->GetState()) {
            it->SetState(0);
        }
    }
}

void LayoutPanel::AddObjectButton(wxMenu& mnu, const long id, const std::string &name, const char *icon[]) {
    wxMenuItem* menu_item = mnu.Append(id, name);
    if (icon != nullptr) {
        wxImage image(icon);
        wxImage halfImg = image.Scale(24, 24, wxIMAGE_QUALITY_HIGH);
        menu_item->SetBitmap(wxBitmapBundle::FromBitmaps(halfImg, image));
    }
}

void LayoutPanel::DisplayAddObjectPopup() {
    wxMenu mnuObjects;
    AddObjectButton(mnuObjects, ID_ADD_OBJECT_IMAGE, "Image", add_object_image_xpm);
    AddObjectButton(mnuObjects, ID_ADD_OBJECT_GRIDLINES, "Gridlines", add_object_gridlines_xpm);
    AddObjectButton(mnuObjects, ID_ADD_OBJECT_TERRIAN, "Terrain", add_object_terrian_xpm);
    AddObjectButton(mnuObjects, ID_ADD_OBJECT_MESH, "Mesh", add_object_mesh_xpm);
    if (RulerObject::GetRuler() == nullptr) {
        AddObjectButton(mnuObjects, ID_ADD_OBJECT_RULER, "Ruler", add_object_ruler_xpm);
    }
    mnuObjects.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&LayoutPanel::OnAddObjectPopup, nullptr, this);
    PopupMenu(&mnuObjects);
}

void LayoutPanel::OnAddObjectPopup(wxCommandEvent& event)
{
    
    int id = event.GetId();
    ViewObject* vobj = nullptr;
    bool object_created = false;
    if (id == ID_ADD_OBJECT_IMAGE)
    {
        spdlog::debug("OnAddObjectPopup - ID_ADD_OBJECT_IMAGE");
        CreateUndoPoint("All", "", "");
        vobj = xlights->AllObjects.CreateAndAddObject("Image");
        vobj->SetLayoutGroup("Default"); // only Default supports 3D and hence objects
        objects_panel->UpdateObjectList(true, currentLayoutGroup);
        object_created = true;
    }
    else if (id == ID_ADD_OBJECT_GRIDLINES)
    {
        spdlog::debug("OnAddObjectPopup - ID_ADD_OBJECT_GRIDLINES");
        CreateUndoPoint("All", "", "");
        vobj = xlights->AllObjects.CreateAndAddObject("Gridlines");
        vobj->SetLayoutGroup("Default"); // only Default supports 3D and hence objects
        objects_panel->UpdateObjectList(true, currentLayoutGroup);
        object_created = true;
    }
    else if (id == ID_ADD_OBJECT_TERRIAN)
    {
        spdlog::debug("OnAddObjectPopup - ID_ADD_OBJECT_TERRIAN");
        CreateUndoPoint("All", "", "");
        vobj = xlights->AllObjects.CreateAndAddObject("Terrain");
        vobj->SetLayoutGroup("Default"); // only Default supports 3D and hence objects
        objects_panel->UpdateObjectList(true, currentLayoutGroup);
        object_created = true;
    }
    else if (id == ID_ADD_OBJECT_RULER) {
        spdlog::debug("OnAddObjectPopup - ID_ADD_OBJECT_RULER");
        CreateUndoPoint("All", "", "");
        vobj = xlights->AllObjects.CreateAndAddObject("Ruler");
        vobj->SetLayoutGroup("Default"); // only Default supports 3D and hence objects
        vobj->GetObjectScreenLocation().SetVcenterPos(100);
        vobj->GetObjectScreenLocation().SetLeft(-50);
        vobj->GetObjectScreenLocation().SetMWidth(100);
        objects_panel->UpdateObjectList(true, currentLayoutGroup);
        object_created = true;
    }
    else if (id == ID_ADD_OBJECT_MESH)
    {
        spdlog::debug("OnAddObjectPopup - ID_ADD_OBJECT_MESH");
        CreateUndoPoint("All", "", "");
        vobj = xlights->AllObjects.CreateAndAddObject("Mesh");
        vobj->SetLayoutGroup("Default"); // only Default supports 3D and hence objects
        objects_panel->UpdateObjectList(true, currentLayoutGroup);
        object_created = true;
    }

    if( object_created ) {
        Notebook_Objects->ChangeSelection(1);
        editing_models = false;
        SelectViewObject(vobj, true);
        //SetupPropGrid(vobj);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_ADD_OBJECT_MESH");
    }

    Refresh();
}

void LayoutPanel::DisplayAddDmxPopup() {
    wxMenu mnuObjects;
    AddObjectButton(mnuObjects, ID_ADD_DMX_GENERAL, "General", add_dmx_general_xpm);
    AddObjectButton(mnuObjects, ID_ADD_DMX_FLOODLIGHT, "Floodlight", add_dmx_floodlight_xpm);
    AddObjectButton(mnuObjects, ID_ADD_DMX_FLOODAREA, "Area Flood", add_dmx_floodlight_xpm);
    AddObjectButton(mnuObjects, ID_ADD_DMX_MOVING_HEAD_ADV, "Moving Head Adv", dmx_xpm);
    AddObjectButton(mnuObjects, ID_ADD_DMX_MOVING_HEAD, "Moving Head", add_dmx_moving_head_xpm);
    AddObjectButton(mnuObjects, ID_ADD_DMX_SERVO, "Servo", add_dmx_servo_xpm);
    if (is_3d) {
        AddObjectButton(mnuObjects, ID_ADD_DMX_SERVO_3D, "Servo 3D", add_dmx_servo3d_xpm);
    }
    AddObjectButton(mnuObjects, ID_ADD_DMX_SKULL, "Skull", add_dmx_skulltronix_xpm);
    mnuObjects.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&LayoutPanel::OnAddDmxPopup, nullptr, this);
    selectedDmxModelType = "";
    PopupMenu(&mnuObjects);
}

void LayoutPanel::OnAddDmxPopup(wxCommandEvent& event)
{
    
    int id = event.GetId();
    bool object_created = false;
    if (id == ID_ADD_DMX_FLOODLIGHT) {
        spdlog::debug("OnAddDmxPopup - ID_ADD_DMX_FLOODLIGHT");
        selectedDmxModelType = "DmxFloodlight";
        object_created = true;
    }
    else if (id == ID_ADD_DMX_GENERAL) {
        spdlog::debug("OnAddDmxPopup - ID_ADD_DMX_GENERAL");
        selectedDmxModelType = "DmxGeneral";
        object_created = true;
    }
    else if (id == ID_ADD_DMX_FLOODAREA) {
        spdlog::debug("OnAddDmxPopup - ID_ADD_DMX_FLOODAREA");
        selectedDmxModelType = "DmxFloodArea";
        object_created = true;
    }
    else if (id == ID_ADD_DMX_MOVING_HEAD) {
        spdlog::debug("OnAddDmxPopup - ID_ADD_DMX_MOVING_HEAD");
        selectedDmxModelType = "DmxMovingHead";
        object_created = true;
    }
    else if (id == ID_ADD_DMX_MOVING_HEAD_ADV) {
        spdlog::debug("OnAddDmxPopup - ID_ADD_DMX_MOVING_HEAD_ADV");
        selectedDmxModelType = "DmxMovingHeadAdv";
        object_created = true;
    }
    else if (id == ID_ADD_DMX_SERVO) {
        spdlog::debug("OnAddDmxPopup - ID_ADD_DMX_SERVO");
        selectedDmxModelType = "DmxServo";
        object_created = true;
    }
    else if (id == ID_ADD_DMX_SERVO_3D) {
        spdlog::debug("OnAddDmxPopup - ID_ADD_DMX_SERVO_3D");
        selectedDmxModelType = "DmxServo3d";
        object_created = true;
    }
    else if (id == ID_ADD_DMX_SKULL) {
        spdlog::debug("OnAddDmxPopup - ID_ADD_DMX_SKULL");
        selectedDmxModelType = "DmxSkull";
        object_created = true;
    }

    if (object_created) {
        UnSelectAllModels();
        modelPreview->SetFocus();
        Notebook_Objects->ChangeSelection(0);
        editing_models = true;
    }
}

Model *LayoutPanel::CreateNewModel(const std::string &type) const
{
    std::string t = type;
    if (t == "Import Custom" || t == "Download")
    {
        t = "Custom";
    }

    xlights->GetOutputModelManager()->DisableASAPWork(true);
    Model* m = xlights->AllModels.CreateDefaultModel(t, "1");
    xlights->GetOutputModelManager()->DisableASAPWork(false);

    return m;
}

std::list<BaseObject*> LayoutPanel::GetSelectedBaseObjects() const
{
    std::list<BaseObject*> res;

    if (selectedBaseObject != nullptr)
    {
        res.push_back(selectedBaseObject);
    }

    if (editing_models)
    {
        for (const auto& it : modelPreview->GetModels())
        {
            if (it != selectedBaseObject && (it->Selected() || it->GroupSelected()))
            {
                res.push_back(it);
            }
        }
    }

    return res;
}

void LayoutPanel::Nudge(int key)
{
    std::list<BaseObject*> selectedBaseObjects = GetSelectedBaseObjects();
    if (selectedBaseObjects.size() > 0)
    {
        if (selectedBaseObjects.size() == 1)
        {
            CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject", selectedBaseObjects.front()->name, "location");
        }
        else
        {
            CreateUndoPoint("All", "", "");
        }

        // this code speeds up the move the more times it is moved in the same direction in a short period of time
        static wxLongLong lastTime = 0;
        static float lastDelta = 0;
        if (key == WXK_UP || key == WXK_DOWN || key == WXK_LEFT || key == WXK_RIGHT) {
            static int repeats = 0;
            static int lastKey = 0;

            if (wxGetUTCTimeMillis() - lastTime > 500 || key != lastKey || xlights->IsDisableKeyAcceleration())
            {
                lastDelta = 1.0;
                repeats = 0;
            }
            else
            {
                repeats++;
                if (repeats > 5)
                {
                    lastDelta *= 2.0;
                    if (lastDelta > 30) lastDelta = 30;
                }
            }

            lastKey = key;
        }

        for (const auto& it : selectedBaseObjects)
        {
            float deltax = 0;
            float deltay = 0;

            if (key == WXK_UP) {
                deltay = 1.0;
            }
            else if (key == WXK_DOWN) {
                deltay = -1.0;
            }
            else if (key == WXK_LEFT) {
                deltax = -1.0;
            }
            else if (key == WXK_RIGHT) {
                deltax = 1.0;
            }

            if (deltax != 0 || deltay != 0)
            {
                deltax *= lastDelta;
                deltay *= lastDelta;

                if (is_3d)
                {
                    deltay *= -1;
                    int x = modelPreview->GetCameraRotationX();
                    int y = modelPreview->GetCameraRotationY();

                    // round to nearest 90 degrees
                    x += 45;
                    x /= 90;
                    x *= 90;
                    y += 45;
                    y /= 90;
                    y *= 90;

                    float xx = 0.0;
                    float yy = 0.0;
                    float zz = 0.0;

                    if (x == 0 || x == 360)
                    {
                        yy = -deltay;
                        if (y == 0 || y == 360)
                        {
                            xx = deltax;
                        }
                        else if (y == 90)
                        {
                            zz = deltax;
                        }
                        else if (y == 180)
                        {
                            xx = -deltax;
                        }
                        else if (y == 270)
                        {
                            zz = -deltax;
                        }
                    }
                    else if (x == 90)
                    {
                        if (y == 0 || y == 360)
                        {
                            xx = deltax;
                            zz = deltay;
                        }
                        else if (y == 90)
                        {
                            zz = deltax;
                            xx = -deltay;
                        }
                        else if (y == 180)
                        {
                            xx = -deltax;
                            zz = -deltay;
                        }
                        else if (y == 270)
                        {
                            zz = -deltax;
                            xx = deltay;
                        }
                    }
                    else if (x == 180)
                    {
                        yy = deltay;
                        if (y == 0 || y == 360)
                        {
                            xx = deltax;
                        }
                        else if (y == 90)
                        {
                            zz = deltax;
                        }
                        else if (y == 180)
                        {
                            xx = -deltax;
                        }
                        else if (y == 270)
                        {
                            zz = -deltax;
                        }
                    }
                    else if (x == 270)
                    {
                        if (y == 0 || y == 360)
                        {
                            xx = deltax;
                            zz = -deltay;
                        }
                        else if (y == 90)
                        {
                            zz = deltax;
                            xx = deltay;
                        }
                        else if (y == 180)
                        {
                            xx = -deltax;
                            zz = deltay;
                        }
                        else if (y == 270)
                        {
                            zz = -deltax;
                            xx = -deltay;
                        }
                    }

                    it->AddOffset(xx, yy, zz);
                }
                else
                {
                    it->AddOffset(deltax, deltay, 0.0);
                }
                //SetupPropGrid(it);
            }
        }

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "LayoutPanel::Nudge");

        // set last time after everything is done
        lastTime = wxGetUTCTimeMillis();
    }
}

void LayoutPanel::OnChar(wxKeyEvent& event) {

    if (HandleLayoutKeyBinding(event)) return;

    wxChar uc = event.GetKeyCode();
    switch (uc) {
        case WXK_UP:
        case WXK_DOWN:
        case WXK_LEFT:
        case WXK_RIGHT:
            Nudge(uc);
            break;
        default:
            break;
    }
}
void LayoutPanel::OnCharHook(wxKeyEvent& event) {

    if (HandleLayoutKeyBinding(event)) return;

  wxChar uc = event.GetKeyCode();

    switch(uc) {
#ifndef __WXOSX__
        case 'z':
        case 'Z':
            if (event.ControlDown()) {
                wxCommandEvent evt(wxEVT_MENU, wxID_UNDO);
                DoUndo(evt);
            }
            break;
        case 'x':
        case 'X':
            if (event.ControlDown()) {
                wxCommandEvent evt(wxEVT_MENU, wxID_CUT);
                DoCut(evt);
            }
            break;
        case 'c':
        case 'C':
            if (event.ControlDown()) {
                wxCommandEvent evt(wxEVT_MENU, wxID_COPY);
                DoCopy(evt);
            }
            break;
        case 'v':
        case 'V':
            if (event.ControlDown()) {
                wxCommandEvent evt(wxEVT_MENU, wxID_PASTE);
                DoPaste(evt);
            }
            break;
    case WXK_INSERT:
    case WXK_NUMPAD_INSERT:
        if (event.ControlDown()) // Copy
        {
            wxCommandEvent evt(wxEVT_MENU, wxID_COPY);
            DoCopy(evt);
            event.StopPropagation();
        }
        else if (event.ShiftDown()) // Paste
        {
            wxCommandEvent evt(wxEVT_MENU, wxID_PASTE);
            DoPaste(evt);
            event.StopPropagation();
        }
        break;
#endif
        case WXK_DELETE:
        case WXK_BACK:
#ifndef __WXOSX__
            if (event.ShiftDown()) // Cut
            {
                wxCommandEvent evt(wxEVT_MENU, wxID_CUT);
                DoCut(evt);
                event.StopPropagation();
            }
            else
#endif
                if (editing_models)
                {
                    if (selectedTreeGroups.size() > 0) {
                        DeleteSelectedGroups();
                    }
                    else {
                        DeleteSelectedModels();
                    }
                }
                else
                {
                    DeleteSelectedObject();
                }
            event.StopPropagation();
            break;
        case WXK_UP:
        case WXK_DOWN:
        case WXK_LEFT:
        case WXK_RIGHT:
            Nudge(uc);
            break;

        case WXK_ESCAPE:
            if (m_polyline_active) {
                FinalizeModel();
            } else if (is_3d) {
                if (editing_models) {
                    UnSelectAllModelsInTree();
                } else {
                    UnSelectAllModels();
                }
            }
            break;

        case WXK_RETURN:
            if (m_polyline_active) {
                FinalizeModel();
            }
            event.Skip();
            break;
        default:
            event.Skip();
            break;
    }
}

void LayoutPanel::OnListCharHook(wxKeyEvent& event)
{
    wxChar uc = event.GetKeyCode();
    switch (uc) {
    case WXK_UP:
    case WXK_DOWN:
    case WXK_LEFT:
    case WXK_RIGHT:
        event.DoAllowNextEvent();
        break;
    default:
        OnCharHook(event);
        break;
    }
}

ModelGroup* LayoutPanel::GetSelectedModelGroup() const
{
    ModelGroup* res = nullptr;

    // This is here because I am seeing crashes which i believe originate here
    xlights->AddTraceMessage("LayoutPanel::GetSelectedModelGroup");
    //if (mSelectedGroup.IsOk()) {
    //    ModelTreeData* data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(mSelectedGroup));
    //    Model* model = data != nullptr ? data->GetModel() : nullptr;
    //    if (model != nullptr) {
    //        if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
    //            res = dynamic_cast<ModelGroup*>(model);
    //        }
    //    }
    //}
    //else {
        if (ModelGroupWindow->IsShown()) {
            res = dynamic_cast<ModelGroup*>(xlights->AllModels[model_grp_panel->GetGroupName()]);
        }
    //}
    xlights->AddTraceMessage("LayoutPanel::GetSelectedModelGroup done");
    return res;
}

void LayoutPanel::RemoveSelectedModelsFromGroup() {
    if (selectedBaseObject != nullptr && !selectedBaseObject->GetBaseObjectScreenLocation().IsLocked() && !selectedBaseObject->IsFromBase()) {
        xlights->AddTraceMessage("LayoutPanel::Remove Selected Models From Group");

        wxArrayString modelsToRemove;
        wxString modelsToConfirm = "";
        wxString parentGroup = "";
        for (const auto& item : selectedTreeModels) {
            if (item.IsOk()) {
                parentGroup = TreeListViewModels->GetItemText(TreeListViewModels->GetItemParent(item));
                wxString modelName = TreeListViewModels->GetItemText(item);
                modelsToRemove.Add(modelName);
                modelsToConfirm = modelsToConfirm + wxString::Format("%s- %s\n", "    ", modelName);
            }
        }
        for (const auto& item : selectedTreeSubModels) {
            if (item.IsOk()) {
                parentGroup = TreeListViewModels->GetItemText(TreeListViewModels->GetItemParent(item));
                wxString modelName = TreeListViewModels->GetItemText(item);
                modelsToRemove.Add(modelName);
                modelsToConfirm = modelsToConfirm + wxString::Format("%s- %s\n", "    ", modelName);
            }
        }
        ModelGroup *grp = dynamic_cast<ModelGroup*>(xlights->GetModel(parentGroup));
        if (grp && wxMessageBox("Are you sure you want to remove the folowing model(s)?:\n\n" + modelsToConfirm, "Confirm Delete?", wxICON_QUESTION | wxYES_NO) == wxYES) {

            // we suspend deferred work because if the delete model pops a dialog then the ASAP work gets done prematurely
            xlights->GetOutputModelManager()->SuspendDeferredWork(true);
            xlights->UnselectEffect(); // we do this just in case the effect is on the model we are deleting

            CreateUndoPoint("All", wxJoin(modelsToRemove, ','));

            for (const auto& it: modelsToRemove) {
                grp->ModelRemoved(it);
            }
            selectedBaseObject = nullptr;

            xlights->GetOutputModelManager()->SuspendDeferredWork(false);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                          OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                          OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                          OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "LayoutPanel::RemoveSelectedModelsFromGroup");
        }
    }

}
void LayoutPanel::DeleteSelectedModels()
{
    // I deliberately allow objects that come from base to be deleted.
    if (selectedBaseObject != nullptr) {
        xlights->AddTraceMessage("LayoutPanel::Delete Selected Model");

        wxArrayString modelsToDelete;
        wxString modelsToConfirm = "";
        for (const auto& item : selectedTreeModels) {
            if (item.IsOk()) {
                wxString modelName = TreeListViewModels->GetItemText(item);
                modelName.Replace("<", "");
                modelName.Replace(">", "");
                modelsToDelete.Add(modelName);
                modelsToConfirm = modelsToConfirm + wxString::Format("%s- %s\n", "    ", modelName);
            }
        }

        if (wxMessageBox("Are you sure you want to delete the folowing model(s)?:\n\n" + modelsToConfirm, "Confirm Delete?", wxICON_QUESTION | wxYES_NO) == wxYES) {
            // we suspend deferred work because if the delete model pops a dialog then the ASAP work gets done prematurely
            xlights->GetOutputModelManager()->SuspendDeferredWork(true);
            xlights->UnselectEffect(); // we do this just in case the effect is on the model we are deleting
            xlights->AbortRender();    // stop any rendering as deleting models from under the renderer will crash xlights

            CreateUndoPoint("All", wxJoin(modelsToDelete, ','));

            bool allDeleted = true;

            for (const auto& it : modelsToDelete) {
                auto model = xlights->AllModels[it];
                if (model != nullptr) {
                    xlights->GetDisplayElementsPanel()->RemoveModelFromLists(it);
                    allDeleted = xlights->AllModels.Delete(it) && allDeleted;
                    xlights->AddTraceMessage(wxString::Format("LayoutPanel::Delete Selected Model : %s", it));
                }
                else {
                    allDeleted = false;
                }
            }

            if (!allDeleted) {
                wxBell();
                wxMessageBox("One or more models cannot be deleted. They may have effects on them.", "Delete failed", 5L, this);
            }

            selectedBaseObject = nullptr;

            xlights->GetOutputModelManager()->SuspendDeferredWork(false);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                          OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                          OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                          OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                                                          OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                                                          OutputModelManager::WORK_RELOAD_MODELLIST, "LayoutPanel::DeleteSelectedModels");
        }
    } else {
        wxBell();
    }
}

void LayoutPanel::DeleteSelectedObject()
{
    objects_panel->DeleteSelectedObject();
}

void LayoutPanel::DeleteSelectedGroups()
{
	wxArrayString groupsToDelete;
	wxString groupsToConfirm = "";
	for (const auto& item : selectedTreeGroups) {
		if (item.IsOk()) {
			wxString groupName = TreeListViewModels->GetItemText(item);
			groupsToDelete.Add(groupName);
			groupsToConfirm = groupsToConfirm + wxString::Format("%s- %s\n", "    ", groupName);
		}
	}

	if (wxMessageBox("Are you sure you want to delete the following group(s)?:\n\n" + groupsToConfirm, "Confirm Remove?", wxICON_QUESTION | wxYES_NO) == wxYES) {

		CreateUndoPoint("All", wxJoin(groupsToDelete, ','));

		xlights->UnselectEffect(); // we do this just in case the effect is on the model we are deleting
        xlights->AbortRender(); // stop rendering as deleting groups while rendering is not good

		for (const auto& it : groupsToDelete) {
			xlights->AllModels.Delete(it.ToStdString());
		}
		UnSelectAllModels();

		xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnModelsPopup::ID_MNU_DELETE_MODEL_GROUP");
	}
}

void LayoutPanel::EditSubModelAlias() {
    Model* subModel = GetModelFromTreeItem(selectedTreeSubModels[0]);
    Model* parent_info = dynamic_cast<SubModel*>(subModel)->GetParent();
    if (subModel == nullptr || parent_info == nullptr)
        return;

    EditSubmodelAliasesDialog dlg(this, parent_info, subModel->GetName());

    if (dlg.ShowModal() == wxID_OK) {
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ReplaceModel");
    }
}

void LayoutPanel::ReplaceModel()
{
    if (selectedBaseObject == nullptr) return;

    Model* modelToReplaceItWith = dynamic_cast<Model*>(selectedBaseObject);

    wxArrayString choices;

    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GetName() != selectedBaseObject->GetName())
        {
            choices.Add(modelPreview->GetModels()[i]->GetName());
        }
    }

    wxSingleChoiceDialog dlg(this, "", "Select the model to replace with this model.", choices);
    dlg.SetSelection(0);
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK)
    {
        xlights->UnselectEffect(); // we do this just in case the effect is on the model we are deleting
        xlights->AbortRender(); // we dont want to be rendering when we do this

        Model* replaceModel = nullptr;
        for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
        {
            if (modelPreview->GetModels()[i]->GetName() == dlg.GetStringSelection())
            {
                replaceModel = modelPreview->GetModels()[i];
            }
        }

        if (replaceModel == nullptr) return;

        CreateUndoPoint("All", "", "");

        // Prompt user to copy the target models start channel ...but only if
        // they are not already the same and the new model uses a chaining start
        // channel ... the theory being if you took time to set the start channel
        // you probably want to keep it and so a prompt will just be annoying
        if ((replaceModel->ModelStartChannel !=
            modelToReplaceItWith->ModelStartChannel &&
            wxString(modelToReplaceItWith->ModelStartChannel).StartsWith(">")) ||
            (replaceModel->GetControllerName() != modelToReplaceItWith->GetControllerName() && (modelToReplaceItWith->GetControllerName() == "" || modelToReplaceItWith->GetControllerName() == NO_CONTROLLER)))
        {
            auto msg = wxString::Format("Should I copy the replaced models start channel '%s' to the replacement model whose start channel is currently '%s'?", replaceModel->ModelStartChannel, modelToReplaceItWith->ModelStartChannel);
            if (wxMessageBox(msg, "Update Start Channel", wxYES_NO) == wxYES)
            {
                modelToReplaceItWith->SetStartChannel(replaceModel->ModelStartChannel);

                modelToReplaceItWith->SetControllerProtocol(replaceModel->GetControllerProtocol());
                modelToReplaceItWith->SetControllerPort(replaceModel->GetControllerPort());
                modelToReplaceItWith->SetControllerName(replaceModel->GetControllerName());
                modelToReplaceItWith->SetSmartRemote(replaceModel->GetSmartRemote());
                modelToReplaceItWith->SetSmartRemoteType(replaceModel->GetSmartRemoteType());
                modelToReplaceItWith->SetSRMaxCascade(replaceModel->GetSRMaxCascade());
                modelToReplaceItWith->SetSRCascadeOnPort(replaceModel->GetSRCascadeOnPort());
            }
        }

        if (replaceModel->GetNumSubModels() > 0 ) {
            if (wxMessageBox("Merge The Submodels", "Merge The Submodels", wxYES_NO) == wxYES) {
                for (int i = 0; i < replaceModel->GetNumSubModels(); ++i) {
                    auto name{ replaceModel->GetSubModel(i)->Name() };
                    if (modelToReplaceItWith->GetSubModel(name) != nullptr) {
                        continue;
                    }
                    
                    // Create SubModel using copy constructor
                    const SubModel* sourceSubModel = dynamic_cast<const SubModel*>(replaceModel->GetSubModel(i));
                    SubModel* sm = new SubModel(modelToReplaceItWith, sourceSubModel);
                    modelToReplaceItWith->AddSubmodel(sm);
                }
            }
        }

        auto rmn = replaceModel->GetName();
        auto riw = modelToReplaceItWith->GetName();

        if (wxMessageBox("Use original size and position of " + rmn, "Use original size and position", wxYES_NO) == wxYES) {
            modelToReplaceItWith->GetModelScreenLocation().SetRotation(replaceModel->GetModelScreenLocation().GetRotation());
            modelToReplaceItWith->SetHcenterPos(replaceModel->GetHcenterPos());
            modelToReplaceItWith->SetVcenterPos(replaceModel->GetVcenterPos());
            modelToReplaceItWith->SetDcenterPos(replaceModel->GetDcenterPos());       
            modelToReplaceItWith->SetHeight(replaceModel->GetHeight());       
            modelToReplaceItWith->SetWidth(replaceModel->GetWidth());    
            modelToReplaceItWith->SetDepth(replaceModel->GetDepth());    
        }

        xlights->AllModels.RenameInListOnly(dlg.GetStringSelection().ToStdString(), "Iamgoingtodeletethismodel");
        replaceModel->Rename("Iamgoingtodeletethismodel");
        xlights->AllModels.RenameInListOnly(modelToReplaceItWith->GetName(), dlg.GetStringSelection().ToStdString());
        modelToReplaceItWith->Rename(dlg.GetStringSelection().ToStdString());
        xlights->AllModels.Delete("Iamgoingtodeletethismodel");
        xlights->ReplaceModelWithModelFixGroups(rmn, riw);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                      OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                      OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                                                      OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ReplaceModel", nullptr, nullptr, dlg.GetStringSelection().ToStdString());
    }
}

void LayoutPanel::UnlinkSelectedModels()
{
    std::vector<Model*> modelsToLock = GetSelectedModelsForEdit();

    for (const auto& model : modelsToLock) {
        model->SetFromBase(false);
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::LockSelectedModels");
}

void LayoutPanel::LockSelectedModels(bool lock)
{
    std::vector<Model*> modelsToLock = GetSelectedModelsForEdit();

    for (const auto& model : modelsToLock) {
        model->Lock(lock);
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::LockSelectedModels");
}

void LayoutPanel::DoCopy(wxCommandEvent& event) {
    if (!modelPreview->HasFocus() && !TreeListViewModels->HasFocus() && !TreeListViewModels->GetView()->HasFocus() && !objects_panel->ObjectListHasFocus()) {
        event.Skip();
    } else if (selectedBaseObject != nullptr) {
        CopyPasteBaseObject copyData;

        copyData.SetBaseObject(selectedBaseObject);

        if (copyData.IsOk() && wxTheClipboard->Open()) {
            if (!wxTheClipboard->SetData(new wxTextDataObject(copyData.Serialise()))) {
                DisplayError("Unable to copy data to clipboard.", this);
            }
            wxTheClipboard->Close();
        }
    }
}

void LayoutPanel::DoCut(wxCommandEvent& event) {
    if (!modelPreview->HasFocus() && !TreeListViewModels->HasFocus() && !TreeListViewModels->GetView()->HasFocus() && !objects_panel->ObjectListHasFocus()) {
        event.Skip();
    } else if (selectedBaseObject != nullptr) {
        DoCopy(event);
		if (editing_models)
			DeleteSelectedModels();
		else
			DeleteSelectedObject();
    }
}

void LayoutPanel::DoPaste(wxCommandEvent& event) {
    if (!modelPreview->HasFocus() && !TreeListViewModels->HasFocus() && !TreeListViewModels->GetView()->HasFocus() && !objects_panel->ObjectListHasFocus()) {
        event.Skip();
    } else {
        if (wxTheClipboard->Open()) {
            CreateUndoPoint("All", selectedBaseObject == nullptr ? "" : selectedBaseObject->name);

            wxTextDataObject data;
            wxTheClipboard->GetData(data);

            CopyPasteBaseObject copyData(data.GetText().ToStdString());

            wxTheClipboard->Close();

            if (copyData.IsOk())
            {
                auto ownedNd = copyData.GetBaseObjectXml();
                pugi::xml_node nd = ownedNd;
                std::string source_model_name = xlEMPTY_STRING;

                if (nd)
                {
                    auto nda = DisplayAsTypeFromString(nd.attribute("DisplayAs").as_string());
                    auto nx = (int)nd.attribute("WorldPosX").as_double();
                    auto ny = (int)nd.attribute("WorldPosY").as_double();
                    auto nz = (int)nd.attribute("WorldPosZ").as_double();
                    source_model_name = nd.attribute("name").as_string();

                    bool moved = true;
                    while (moved)
                    {
                        moved = false;
                        // is there a model in the same location of the same type ... if so offset the pasting of the model
                        for (const auto& it : xlights->AllModels)
                        {
                            if (nda == it.second->GetDisplayAs())
                            {
                                auto pos = it.second->GetBaseObjectScreenLocation().GetWorldPosition();
                                auto x = (int)pos.x;
                                auto y = (int)pos.y;
                                auto z = (int)pos.z;
                                if (nx == x &&
                                    ny == y &&
                                    nz == z)
                                {
                                    nx += 40;
                                    SetXmlNodeAttribute(nd, "WorldPosX", fmt::format("{:6.4f}", (float)nx));
                                    moved = true;
                                    break;
                                }
                            }
                        }
                    }

                    std::string name = "";

                    if (!copyData.IsViewObject())
					{
						if (!editing_models)//dont paste model in View Object mode
							return;

						Model *newModel = xlights->AllModels.CreateModel(nd);

                        // Remove any existing controller port config
                        newModel->SetModelChain("");
                        nd.remove_attribute("Controller");
                        pugi::xml_node cc = nd.child("ControllerConnection");
                        if (cc) {
                            nd.remove_child(cc);
                        }

						name = xlights->AllModels.GenerateModelName(newModel->name);
                        newModel->SetControllerName(NO_CONTROLLER);
                        // If original model was already on NO_CONTROLLER, SetControllerName returns
                        // early without clearing the start channel. Force it empty so ReworkStartChannel
                        // assigns the next available channel rather than leaving it at the copied value.
                        newModel->SetStartChannel("");
						newModel->name = name;
						newModel->Lock(false);
						newModel->AddOffset(0.02, 0.02, 0.0);
						xlights->AllModels.AddModel(newModel);
						lastModelName = name;
					}
					else
					{
						if (editing_models)//dont paste view objects in model editing mode
							return;

						ViewObject *newViewObject = xlights->AllObjects.CreateObject(nd);
						name = xlights->AllObjects.GenerateObjectName(newViewObject->name);
						newViewObject->name = name;
						newViewObject->Lock(false);
						newViewObject->AddOffset(50.0, 0.0, 0.0);
						xlights->AllObjects.AddViewObject(newViewObject);
						lastModelName = name;
					}

                    Model* sourceModel = xlights->GetModel(source_model_name);
                    if (sourceModel != nullptr) {
                        auto inModelGroups = sourceModel->GetModelManager().GetGroupsContainingModel(sourceModel);
                        if (!inModelGroups.empty()) {
                            if (wxMessageBox("Should I add model to the same group(s) as the original?", "Add to groups?", wxICON_QUESTION | wxYES_NO) == wxYES) {
                                for (const auto& grp : inModelGroups) {
                                    Model* addToGroup = xlights->GetModel(grp);
                                    ModelGroup *g = dynamic_cast<ModelGroup *>(addToGroup);
                                    if (g && !g->IsFromBase()) {
                                        auto groupModels = g->ModelNames();
                                        int groupItems = groupModels.size(); // we'll keep adding items, keep inital count
                                        for (int i = 0; i < groupItems; i++) {
                                            if (StartsWith(groupModels[i], source_model_name)) {
                                                auto addnew = groupModels[i];
                                                Replace(addnew, source_model_name, name);
                                                g->AddModel(addnew);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    //SelectBaseObject(name);
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                                  OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                                  OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                                  OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                                                                  OutputModelManager::WORK_RELOAD_MODELLIST, "LayoutPanel::DoPaste", nullptr, nullptr, name);
                    modelPreview->SetCursor(wxCURSOR_DEFAULT);
                }
            } else {
                spdlog::warn("LayoutPanel: Error trying to parse XML for paste. Paste request ignored. {}.", data.GetText().ToStdString());
            }
        }
    }
}

void LayoutPanel::DoUndo(wxCommandEvent& event) {
    
    spdlog::debug("LayoutPanel::DoUndo");
    int sz = undoBuffer.size() - 1;
    if (sz >= 0) {
        UnSelectAllModels();
        xlights->AbortRender();

        if (undoBuffer[sz].type == "Background") {
            spdlog::debug("LayoutPanel::DoUndo Background");
            wxPropertyGridEvent pgEvent;
            pgEvent.SetPropertyGrid(propertyEditor);
            wxStringProperty wsp("Background", undoBuffer[sz].key, undoBuffer[sz].data);
            pgEvent.SetProperty(&wsp);
            wxVariant value(undoBuffer[sz].data);
            pgEvent.SetPropertyValue(value);
            OnPropertyGridChange(pgEvent);
            UnSelectAllModels();
        } else if (undoBuffer[sz].type == "ModelProperty") {
            spdlog::debug("LayoutPanel::DoUndo ModelProperty");
            SelectModel(undoBuffer[sz].model);
            wxPropertyGridEvent event2;
            event2.SetPropertyGrid(propertyEditor);
            wxStringProperty wsp("Model", undoBuffer[sz].key, undoBuffer[sz].data);
            event2.SetProperty(&wsp);
            wxVariant value(undoBuffer[sz].data);
            event2.SetPropertyValue(value);
            OnPropertyGridChange(event2);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                          OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                          OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::DoUndo");
        } else if (undoBuffer[sz].type == "ObjectProperty") {
            spdlog::debug("LayoutPanel::DoUndo ObjectProperty");
            ViewObject* vobj = xlights->AllObjects[undoBuffer[sz].model];
            SelectViewObject(vobj);
            wxPropertyGridEvent event2;
            event2.SetPropertyGrid(propertyEditor);
            wxStringProperty wsp("Object", undoBuffer[sz].key, undoBuffer[sz].data);
            event2.SetProperty(&wsp);
            wxVariant value(undoBuffer[sz].data);
            event2.SetPropertyValue(value);
            OnPropertyGridChange(event2);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                          OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                          OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::DoUndo");
        } else if (undoBuffer[sz].type == "SingleModel") {
            spdlog::debug("LayoutPanel::DoUndo SingleModel");
            Model *m = xlights->AllModels[undoBuffer[sz].model];
            if (m != nullptr) {
                pugi::xml_document mdoc;
                mdoc.load_string(undoBuffer[sz].data.c_str());
                pugi::xml_node root = mdoc.document_element();
                pugi::xml_node modelNode = root ? root.first_child() : pugi::xml_node();
                if (modelNode) {
                    Model* newModel = xlights->AllModels.CreateModel(modelNode,
                        modelPreview->GetVirtualCanvasWidth(),
                        modelPreview->GetVirtualCanvasHeight());
                    if (newModel != nullptr) {
                        xlights->AllModels.ReplaceModel(undoBuffer[sz].model, newModel);
                    }
                }
                SelectModel(undoBuffer[sz].model);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                              OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::DoUndo");
            }
        } else if (undoBuffer[sz].type == "SingleObject") {
            spdlog::debug("LayoutPanel::DoUndo SingleObject");
            ViewObject *m = xlights->AllObjects[undoBuffer[sz].model];
            if (m != nullptr) {
                pugi::xml_document mdoc;
                mdoc.load_string(undoBuffer[sz].data.c_str());
                pugi::xml_node root = mdoc.document_element();
                pugi::xml_node objectNode = root ? root.first_child() : pugi::xml_node();
                if (objectNode) {
                    xlights->AllObjects.Delete(undoBuffer[sz].model);
                    ViewObject* newObj = xlights->AllObjects.CreateObject(objectNode);
                    if (newObj != nullptr) {
                        xlights->AllObjects.AddViewObject(newObj);
                    }
                }
                SelectModel(undoBuffer[sz].model);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                              OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::DoUndo");
            }
        } else if (undoBuffer[sz].type == "All") {
            spdlog::debug("LayoutPanel::DoUndo All");
            UnSelectAllModels();

            // Restore models and groups from serialized XML
            pugi::xml_document mdoc;
            mdoc.load_string(undoBuffer[sz].models.c_str());
            pugi::xml_node mroot = mdoc.document_element();
            if (mroot) {
                pugi::xml_node modelsNode = mroot.child("models");
                pugi::xml_node groupsNode = mroot.child("modelGroups");
                if (modelsNode) {
                    xlights->AllModels.LoadModels(modelsNode,
                        modelPreview->GetVirtualCanvasWidth(),
                        modelPreview->GetVirtualCanvasHeight());
                }
                if (groupsNode) {
                    xlights->AllModels.LoadGroups(groupsNode,
                        modelPreview->GetVirtualCanvasWidth(),
                        modelPreview->GetVirtualCanvasHeight());
                }
            }

            // Restore view objects from serialized XML
            pugi::xml_document odoc;
            odoc.load_string(undoBuffer[sz].objects.c_str());
            pugi::xml_node oroot = odoc.document_element();
            if (oroot) {
                pugi::xml_node objectsNode = oroot.child("view_objects");
                if (objectsNode) {
                    xlights->AllObjects.LoadViewObjects(objectsNode);
                }
            }

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                          OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                          OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::DoUndo", nullptr, nullptr, undoBuffer[sz].model);
        } else if (undoBuffer[sz].type == "ModelName") {
            spdlog::debug("LayoutPanel::DoUndo ModelName");
            std::string origName = undoBuffer[sz].model;
            std::string newName = undoBuffer[sz].key;
            if (lastModelName == newName) {
                lastModelName = origName;
            }
            xlights->RenameModel(newName, origName);

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                          OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                          OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::DoUndo", nullptr, nullptr, origName);
        } else if (undoBuffer[sz].type == "ObjectName") {
            spdlog::debug("LayoutPanel::DoUndo ObjectName");
            std::string origName = undoBuffer[sz].model;
            std::string newName = undoBuffer[sz].key;
            xlights->RenameObject(newName, origName);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                          OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                          OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::DoUndo", nullptr, nullptr, origName);
        }
        else
        {
            wxASSERT(false);
        }
        modelPreview->SetFocus();

        undoBuffer.resize(sz);
    }
}

bool LayoutPanel::IsSelectedBaseObjectValid() const {
    // Pointer-address comparison only - never dereferences selectedBaseObject.
    // selectedBaseObject can be left pointing to freed memory if a modal dialog
    // (e.g. ModelStateDialog or SubModelsDialog) was cancelled mid-edit while
    // a node selection was in progress: the dialog tears down the model it was
    // editing without invalidating the LayoutPanel cache. Without this guard,
    // OnPropertyGridChange's null check passes (the pointer isn't literally
    // nullptr) and a subsequent dynamic_cast / dereference crashes on the
    // dangling pointer.
    if (selectedBaseObject == nullptr) {
        return false;
    }
    for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it) {
        if (static_cast<BaseObject*>(it->second) == selectedBaseObject) {
            return true;
        }
        for (auto* sm : it->second->GetSubModels()) {
            if (static_cast<BaseObject*>(sm) == selectedBaseObject) {
                return true;
            }
        }
    }
    for (auto it = xlights->AllObjects.begin(); it != xlights->AllObjects.end(); ++it) {
        if (static_cast<BaseObject*>(it->second) == selectedBaseObject) {
            return true;
        }
    }
    return false;
}

void LayoutPanel::CreateUndoPoint(const std::string &tp, const std::string &model, const std::string &key, const std::string &data) {
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::CreateUndoPoint");
    size_t idx = undoBuffer.size();

    std::string type = tp;

    //printf("%s   %s   %s  %s\n", type.c_str(), model.c_str(), key.c_str(), data.c_str());
    if (idx > 0 ) {
        if ((type == "SingleModel" || type == "ModelProperty" || type == "Background")
            && undoBuffer[idx - 1].model == model && undoBuffer[idx - 1].key == key) {
            //SingleModel - multi mouse movement, just record the original
            //Background/ModelProperty - multiple changes of the same property (like spinning spin button)
            return;
        }
        else if ((type == "SingleObject" || type == "ObjectProperty")
            && undoBuffer[idx - 1].model == model && undoBuffer[idx - 1].key == key) {
            return;
        }
    }
    
    // if we are doing a move/resize/etc... with multiple models selected, we
    // need to save everything so we can undo the entire operation
    int selectedModelCnt = ModelsSelectedCount();
    int selectedViewObjectCnt = ViewObjectsSelectedCount();
    if (type == "SingleModel" && selectedModelCnt > 1) {
        type = "All";
    }
    if (type == "SingleObject" && selectedViewObjectCnt > 1) {
        type = "All";
    }

    if (idx >= 100) {  //100 steps is more than enough IMO
        for (size_t x = 1; x < idx; x++) {
            undoBuffer[x-1] = undoBuffer[x];
        }
        idx--;
    }
    undoBuffer.resize(idx + 1);

    undoBuffer[idx].type = type;
    undoBuffer[idx].model = model;
    undoBuffer[idx].key = key;
    undoBuffer[idx].data = data;

    XmlSerializer serializer;

    if (type == "SingleModel") {
        const Model *m = _newModel;
        if (m == nullptr) {
            if( selectedBaseObject == nullptr ) {
                undoBuffer.resize(idx);
                return;
            }
            m=dynamic_cast<Model*>(selectedBaseObject);
            wxASSERT(m != nullptr);
        }
        
        // Use XmlSerializer to create the XML document
        pugi::xml_document doc = serializer.SerializeModel(m, false);

        std::ostringstream stream;
        doc.save(stream);
        undoBuffer[idx].data = stream.str();
    } else if (type == "SingleObject") {
        ViewObject *obj = nullptr;
        if( selectedBaseObject == nullptr ) {
            undoBuffer.resize(idx);
            return;
        }
        obj=dynamic_cast<ViewObject*>(selectedBaseObject);
        wxASSERT(obj != nullptr);
        
        StringSerializingVisitor visitor;
        serializer.SerializeObject(*obj, visitor);
        undoBuffer[idx].data = visitor.GetResult();
    } else if (type == "All") {
        // Serialize all models
        {
            StringSerializingVisitor visitor;
            visitor.WriteOpenTag("root");
            XmlSerializer::SerializeAllModels(xlights->AllModels, visitor);
            visitor.WriteCloseTag();

            undoBuffer[idx].models = visitor.GetResult();
        }
        
        // Serialize all view objects
        {
            StringSerializingVisitor visitor;
            visitor.WriteOpenTag("root");
            XmlSerializer::SerializeAllObjects(xlights->AllObjects, visitor);
            visitor.WriteCloseTag();

            undoBuffer[idx].objects = visitor.GetResult();
        }
    }
}

void LayoutPanel::OnModelsPopup(wxCommandEvent& event) {
    

    int id = event.GetId();
    if (id == ID_MNU_DELETE_MODEL) {
        spdlog::debug("LayoutPanel::OnModelsPopup DELETE_MODEL");
        DeleteSelectedModels();
    } else if (id == ID_MNU_REMOVE_MODEL_FROM_GROUP) {
        spdlog::debug("LayoutPanel::OnModelsPopup REMOVE_MODEL_FROM_GROUP");
        RemoveSelectedModelsFromGroup();
    } else if (id == ID_MNU_EDIT_SUBMODEL_ALIAS) {
        EditSubModelAlias();
    } else if (event.GetId() == ID_PREVIEW_REPLACEMODEL) {
        ReplaceModel();
    } else if (event.GetId() == ID_PREVIEW_MODEL_NODELAYOUT) {
        ShowNodeLayout();
    } else if (event.GetId() == ID_PREVIEW_MODEL_LOCK) {
        LockSelectedModels(true);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
                                                      OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_LOCK");
    } else if (event.GetId() == ID_PREVIEW_MODEL_UNLOCK) {
        LockSelectedModels(false);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
                                                      OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_UNLOCK");
    } else if (event.GetId() == ID_PREVIEW_MODEL_UNLINKFROMBASE) {
        UnlinkSelectedModels();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_UNLINKFROMBASE");
    } else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTASCUSTOM) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        {
            wxString name = wxString(md->GetName()).Trim(true).Trim(false);
            wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written
            wxString fn = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
            if (!fn.IsEmpty()) {
                FileSerializingVisitor visitor(ToStdString(fn), true /*exporting*/);
                if (!visitor.IsOpen())
                    DisplayError("Unable to create file " + ToStdString(fn));
                else
                    md->ExportAsCustomXModel(visitor);
            }
        }
    } else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTASCUSTOM3D) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        {
            wxString name = wxString(md->GetName()).Trim(true).Trim(false);
            wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written
            wxString fn = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
            if (!fn.IsEmpty()) {
                FileSerializingVisitor visitor(ToStdString(fn), true /*exporting*/);
                if (!visitor.IsOpen())
                    DisplayError("Unable to create file " + ToStdString(fn));
                else
                    md->ExportAsCustomXModel3D(visitor);
            }
        }
    } else if (event.GetId() == ID_PREVIEW_MODEL_CREATEGROUP) {
        CreateModelGroupFromSelected();
    } else if (event.GetId() == ID_MNU_ADD_TO_EXISTING_GROUPS) {
        AddSelectedToExistingGroups();
    } else if (event.GetId() == ID_MNU_REMOVE_FROM_EXISTING_GROUPS) {
        RemoveSelectedFromExistingGroups();
    } else if (event.GetId() == ID_PREVIEW_MODEL_WIRINGVIEW) {
        ShowWiring();
    } else if (event.GetId() == ID_PREVIEW_MODEL_CAD_EXPORT) {
        ExportModelAsCAD();
    } else if (event.GetId() == ID_PREVIEW_EXPORT_FACESSTATESSUBMODELS) {
        ExportFacesStatesSubModels();
    } else if (event.GetId() == ID_PREVIEW_MODEL_ASPECTRATIO) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int screen_wi = md->GetBaseObjectScreenLocation().GetMWidth();
        int screen_ht = md->GetBaseObjectScreenLocation().GetMHeight();
        float render_ht = md->GetBaseObjectScreenLocation().GetRenderHt();
        float render_wi = md->GetBaseObjectScreenLocation().GetRenderWi();
        float ht_ratio = render_ht / (float)screen_ht;
        float wi_ratio = render_wi / (float)screen_wi;
        if (ht_ratio > wi_ratio) {
            render_wi = render_wi / ht_ratio;
            md->GetBaseObjectScreenLocation().SetMWidth((int)render_wi);
        } else {
            render_ht = render_ht / wi_ratio;
            md->GetBaseObjectScreenLocation().SetMHeight((int)render_ht);
        }
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
                                                      OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                                                      OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ASPECTRATIO", nullptr, nullptr, GetSelectedModelName());
    } else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL) {
        const Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        XmlSerializer serializer;
        wxString name = md->GetName();
        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (!filename.IsEmpty()) {
            ObtainAccessToURL(filename);
            pugi::xml_document doc = serializer.SerializeModel(md, true);
            doc.save_file(ToStdString(filename).c_str());
        }
    } else if (event.GetId() == ID_PREVIEW_DELETE_ACTIVE) {
        DeleteCurrentPreview();
    } else if (event.GetId() == ID_PREVIEW_RENAME_ACTIVE) {
        RenameCurrentPreview();
    } else if (event.GetId() == ID_PREVIEW_MODEL_ADDPOINT) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int handle = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(handle + 0x8000));
        md->InsertHandle(handle, modelPreview->GetCameraZoomForHandles(), modelPreview->GetHandleScale());
        md->Reinitialize();
        //SetupPropGrid(md);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                                                      OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ADDPOINT");
    } else if (event.GetId() == ID_PREVIEW_MODEL_DELETEPOINT) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        auto selectedId = md->GetSelectedHandleId();
        if (selectedId && selectedId->role == handles::Role::Vertex &&
            md->GetNumHandles() > 2) {
            const int legacyHandle = selectedId->index + 1;
            CreateUndoPoint("SingleModel", md->name, std::to_string(legacyHandle + 0x4000));
            md->DeleteHandle(legacyHandle);
            md->SelectHandle(-1);
            md->GetModelScreenLocation().SelectSegment(-1);
            md->Reinitialize();
            //SetupPropGrid(md);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                                                          OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_DELETEPOINT");
        }
    } else if (event.GetId() == ID_PREVIEW_MODEL_ADDCURVE) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int seg = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(seg + 0x2000));
        md->SetCurve(seg, true);
        md->Reinitialize();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ADDCURVE");
    } else if (event.GetId() == ID_PREVIEW_MODEL_DELCURVE) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int seg = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(seg + 0x1000));
        md->SetCurve(seg, false);
        md->Reinitialize();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_DELCURVE");
    } else if (event.GetId() == ID_PREVIEW_ALIGN_BOTTOM) {
        if (editing_models) {
            PreviewModelAlignBottoms();
        } else {
            objects_panel->PreviewObjectAlignBottoms();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_GROUND) {
        if (editing_models) {
            PreviewModelAlignWithGround();
        } else {
            objects_panel->PreviewObjectAlignWithGround();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_TOP) {
        if (editing_models) {
            PreviewModelAlignTops();
        } else {
            objects_panel->PreviewObjectAlignTops();
        }
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTION ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERSTARTNULLNODES ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERENDNULLNODES ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERBRIGHTNESS ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERCOLOURORDER ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERGAMMA ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERGROUPCOUNT ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERDIRECTION ||
               event.GetId() == ID_PREVIEW_BULKEDIT_SMARTREMOTE ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERPROTOCOL ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTIONINCREMENT ||
               event.GetId() == ID_PREVIEW_BULKEDIT_SMARTREMOTETYPE) {
        BulkEditControllerConnection(event.GetId());
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERNAME) {
        BulkEditControllerName();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_SETACTIVE) {
        BulkEditActive(true);
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_SETINACTIVE) {
        BulkEditActive(false);
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_TAGCOLOUR) {
        BulkEditTagColour();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_PIXELSIZE) {
        BulkEditPixelSize();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_PIXELSTYLE) {
        BulkEditPixelStyle();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_TRANSPARENCY) {
        BulkEditTransparency();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_BLACKTRANSPARENCY) {
        BulkEditBlackTranparency();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_SHADOWMODELFOR) {
        BulkEditShadowModelFor();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_PREVIEW) {
        BulkEditControllerPreview();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_DIMMINGCURVES) {
        BulkEditDimmingCurves();
    } else if (event.GetId() == ID_PREVIEW_ALIGN_LEFT) {
        if (editing_models) {
            PreviewModelAlignLeft();
        } else {
            objects_panel->PreviewObjectAlignLeft();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_RIGHT) {
        if (editing_models) {
            PreviewModelAlignRight();
        } else {
            objects_panel->PreviewObjectAlignRight();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_FRONT) {
        if (editing_models) {
            PreviewModelAlignFronts();
        } else {
            objects_panel->PreviewObjectAlignFronts();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_BACK) {
        if (editing_models) {
            PreviewModelAlignBacks();
        } else {
            objects_panel->PreviewObjectAlignBacks();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_H_CENTER) {
        if (editing_models) {
            PreviewModelAlignHCenter();
        } else {
            objects_panel->PreviewObjectAlignHCenter();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_V_CENTER) {
        if (editing_models) {
            PreviewModelAlignVCenter();
        } else {
            objects_panel->PreviewObjectAlignVCenter();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_D_CENTER) {
        if (editing_models) {
            PreviewModelAlignDCenter();
        } else {
            objects_panel->PreviewObjectAlignDCenter();
        }
    } else if (event.GetId() == ID_PREVIEW_H_DISTRIBUTE) {
        if (editing_models) {
            PreviewModelHDistribute();
        } else {
            objects_panel->PreviewObjectHDistribute();
        }
    } else if (event.GetId() == ID_PREVIEW_V_DISTRIBUTE) {
        if (editing_models) {
            PreviewModelVDistribute();
        } else {
            objects_panel->PreviewObjectVDistribute();
        }
    } else if (event.GetId() == ID_PREVIEW_D_DISTRIBUTE) {
        if (editing_models) {
            PreviewModelDDistribute();
        } else {
            objects_panel->PreviewObjectDDistribute();
        }
    } else if (event.GetId() == ID_PREVIEW_RESIZE_SAMEWIDTH) {
        if (editing_models) {
            PreviewModelResize(true, false);
        } else {
            objects_panel->PreviewObjectResize(true, false);
        }
    } else if (event.GetId() == ID_PREVIEW_RESIZE_SAMEHEIGHT) {
        if (editing_models) {
            PreviewModelResize(false, true);
        } else {
            objects_panel->PreviewObjectResize(false, true);
        }
    } else if (event.GetId() == ID_PREVIEW_RESIZE_SAMESIZE) {
        if (editing_models) {
            PreviewModelResize(true, true);
        } else {
            objects_panel->PreviewObjectResize(true, true);
        }
    } else if (id == ID_MNU_DELETE_MODEL_GROUP) {
        spdlog::debug("LayoutPanel::OnModelsPopup DELETE_MODEL_GROUP");
        DeleteSelectedGroups();
    } else if (id == ID_MNU_DELETE_ALL_ALIASES) {
        spdlog::debug("LayoutPanel::Popup DELETE_ALL_ALIASES");
        if (wxMessageBox("This will remove aliases from *all* models, groups and submodels.\n Do you wish to continue?", "Delete all Aliases...", wxYES_NO, this) == wxYES) {
            bool deleted = false;
            bool rc = false;
            for (const auto& m : xlights->AllModels) {
                if (m.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
                    ModelGroup* mg = dynamic_cast<ModelGroup*>(m.second);
                    rc = mg->DeleteAllAliases();
                    deleted = deleted || rc;
                } else {
                    Model* mm = dynamic_cast<Model*>(m.second);
                    rc = mm->DeleteAllAliases();
                    deleted = deleted || rc;
                    for (auto sm : mm->GetSubModels()) {
                        SubModel* s = dynamic_cast<SubModel*>(sm);
                        if (s != nullptr) {
                            rc = s->DeleteAllAliases();
                            deleted = deleted || rc;
                        }
                    }
                }
            }
            if (deleted) {
                xlights->MarkEffectsFileDirty();
            }
        }
    } else if (id == ID_MNU_DELETE_EMPTY_MODEL_GROUPS) {
        spdlog::debug("LayoutPanel::OnModelsPopup DELETE_EMPTY_MODEL_GROUPS");

        bool deleted = true;

        while (deleted) {
            deleted = false;
            auto it = xlights->AllModels.begin();
            while (it != xlights->AllModels.end()) {
                if (it->second->GetDisplayAs() == DisplayAsType::ModelGroup) {
                    ModelGroup* mg = dynamic_cast<ModelGroup*>(it->second);
                    ++it;
                    if (mg->GetModelCount() == 0) {
                        xlights->UnselectEffect(); // we do this just in case the effect is on the model we are deleting
                        xlights->AbortRender();

                        bool response = xlights->AllModels.Delete(mg->GetName());
                        if (response) {
                            deleted = true;
                        }
                    }
                } else {
                    ++it;
                }
            }
        }

        UnSelectAllModels();

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                      OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                      OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnModelsPopup::ID_MNU_DELETE_EMPTY_MODEL_GROUPS");
    } else if (id == ID_MNU_MAKEALLSCVALID) {
        if (wxMessageBox("While this will make all your start channels valid it will not magically make your start channel right for your show. It will however solve strange nodes lighting up in the sequencer.\nAre you ok with this?", "WARNING", wxYES_NO | wxCENTRE) == wxYES) {
            for (const auto& it : xlights->AllModels) {
                if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup && (!it.second->CouldComputeStartChannel || !it.second->IsValidStartChannelString())) {
                    it.second->SetControllerName(NO_CONTROLLER);
                }
            }

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                          OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                          OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                          OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "LayoutPanel::OnModelsPopup::ID_MNU_MAKEALLSCVALID");
        }
    } else if (id == ID_MNU_MAKEALLSCNOTOVERLAPPING) {
        if (wxMessageBox("While this will make all your start channels not overlapping it will not magically make your start channel right for your show. It will however solve strange nodes lighting up in the sequencer.\nAre you ok with this?", "WARNING", wxYES_NO | wxCENTRE) == wxYES) {
            for (const auto& it : xlights->AllModels) {
                if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup && xlights->AllModels.IsModelOverlapping(it.second)) {
                    it.second->SetControllerName(NO_CONTROLLER);
                }
            }

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                          OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                          OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                          OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "LayoutPanel::OnModelsPopup::ID_MNU_MAKEALLSCNOTOVERLAPPING");
        }
    } else if (id == ID_MNU_MAKESCVALID) {
        if (wxMessageBox("While this will make your start channel valid and not overlapping it will not magically make your start channel right for your show. It will however solve strange nodes lighting up in the sequencer.\nAre you ok with this?", "WARNING", wxYES_NO | wxCENTRE) == wxYES) {
            if (selectedBaseObject != nullptr) {
                Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
                selectedModel->SetControllerName(NO_CONTROLLER);

                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                              OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                              OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                              OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "LayoutPanel::OnModelsPopup::ID_MNU_MAKESCVALID");
            }
        }
    } else if (id == ID_MNU_RENAME_MODEL_GROUP) {
        spdlog::debug("LayoutPanel::OnModelsPopup RENAME_MODEL_GROUP");
        if (selectedTreeGroups[0].IsOk()) {
            wxString sel = TreeListViewModels->GetItemText(selectedTreeGroups[0]);
            wxTextEntryDialog dlg(this, "Enter new name for group " + sel, "Rename " + sel, sel);
            OptimiseDialogPosition(&dlg);
            if (dlg.ShowModal() == wxID_OK) {
                wxString name = wxString(Model::SafeModelName(dlg.GetValue().ToStdString()));

                while (xlights->AllModels.GetModel(name.ToStdString()) != nullptr || name.IsEmpty()) {
                    wxTextEntryDialog dlg2(this, "Model or Group of name '" + name + "' already exists. Enter new name for group", "Enter new name for group");
                    OptimiseDialogPosition(&dlg2);
                    if (dlg2.ShowModal() == wxID_OK) {
                        name = wxString(Model::SafeModelName(dlg2.GetValue().ToStdString()));
                    } else {
                        return;
                    }
                }

                xlights->RenameModel(sel.ToStdString(), name.ToStdString());
                model_grp_panel->UpdatePanel(name.ToStdString());

                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                              OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                              OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnModelsPopup::ID_MNU_RENAME_MODEL_GROUP", nullptr, nullptr, name.ToStdString());
            }
        }
    } else if (event.GetId() == ID_PREVIEW_MODEL_CREATEGROUP) {
        CreateModelGroupFromSelected();
    } else if (id == ID_MNU_ADD_MODEL_GROUP) {
        spdlog::debug("LayoutPanel::OnModelsPopup ADD_MODEL_GROUP");
        wxTextEntryDialog dlg(this, "Enter name for new group", "Enter name for new group");
        OptimiseDialogPosition(&dlg);
        if (dlg.ShowModal() == wxID_OK) {
            wxString name = wxString(Model::SafeModelName(dlg.GetValue().ToStdString()));
            while (xlights->AllModels.GetModel(name.ToStdString()) != nullptr || name.IsEmpty()) {
                wxTextEntryDialog dlg2(this, "Model of name '" + name + "' already exists. Enter name for new group", "Enter name for new group");
                OptimiseDialogPosition(&dlg2);
                if (dlg2.ShowModal() == wxID_OK) {
                    name = wxString(Model::SafeModelName(dlg2.GetValue().ToStdString()));
                } else {
                    return;
                }
            }
            wxString grp = currentLayoutGroup == "All Models" ? "Unassigned" : currentLayoutGroup;

            // Create the model group directly using setters
            ModelGroup* newModelGroup = new ModelGroup(xlights->AllModels);
            newModelGroup->SetName(name.ToStdString());
            newModelGroup->SetLayout("minimalGrid");
            newModelGroup->SetGridSize(400);
            newModelGroup->SetLayoutGroup(grp.ToStdString());
            xlights->AllModels.AddModel(newModelGroup);

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                          OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                          OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                                                          OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnModelsPopup::ID_MNU_ADD_MODEL_GROUP", nullptr, nullptr, name.ToStdString());
        }
    } else if (id == ID_MNU_CLONE_MODEL_GROUP) {
        spdlog::debug("LayoutPanel::OnModelsPopup CLONE_MODEL_GROUP");

        wxString sel = TreeListViewModels->GetItemText(selectedTreeGroups[0]);
        ModelGroup* mg = dynamic_cast<ModelGroup*>(xlights->AllModels.GetModel(sel));
        if (mg == nullptr)
            return;
        std::string name = xlights->AllModels.GenerateModelName(sel);

        // Serialize the existing group to a temporary XML node, then rename it
        pugi::xml_document groupsDoc;
        pugi::xml_node groupsRoot = groupsDoc.append_child("modelGroups");
        XmlSerializingVisitor visitor{ groupsRoot };
        mg->Accept(visitor);
        pugi::xml_node node = groupsRoot.first_child();
        if (node) {
            node.remove_attribute("name");
            node.append_attribute("name") = name;
        }

        if (node) {
            xlights->AllModels.AddModel(xlights->AllModels.CreateModel(node));
        }
        //model_grp_panel->UpdatePanel(name);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                      OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                      OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                                      OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                                                      OutputModelManager::WORK_RELOAD_MODELLIST, "LayoutPanel::OnModelsPopup::ID_MNU_CLONE_MODEL_GROUP", nullptr, nullptr, name);
    } else if (event.GetId() == ID_MNU_BULKEDIT_GROUP_TAGCOLOR) {
        BulkEditGroupTagColor();
    } else if (event.GetId() == ID_MNU_BULKEDIT_GROUP_PREVIEW) {
        BulkEditGroupControllerPreview();
    } else if (event.GetId() == ID_PREVIEW_FLIP_HORIZONTAL) {
        if (editing_models) {
            PreviewModelFlipH();
        } else {
            objects_panel->PreviewObjectFlipH();
        }
    }
    else if (event.GetId() == ID_PREVIEW_FLIP_VERTICAL) {
        if (editing_models) {
            PreviewModelFlipV();
        } else {
            objects_panel->PreviewObjectFlipV();
        }
    }
}

LayoutGroup* LayoutPanel::GetLayoutGroup(const std::string &name)
{
    auto it = xlights->LayoutGroups.find(name);
    if (it != xlights->LayoutGroups.end()) {
        return it->second.get();
    }
    return nullptr;
}

void LayoutPanel::OnChoiceLayoutGroupsSelect(wxCommandEvent& event)
{
    UnSelectAllModels();

    //for (const auto& it : xlights->AllModels) {
    //    Model *model = it.second;
    //    model->Selected = false;
    //    model->GroupSelected = false;
    //    model->Highlighted = false;
    //}

    std::string choice_layout = std::string(ChoiceLayoutGroups->GetStringSelection().c_str());
    if( choice_layout == "<Create New Preview>" ) {
        wxTextEntryDialog dlg(this, "Enter name for new preview", "Create New Preview");
        OptimiseDialogPosition(&dlg);
        if (dlg.ShowModal() == wxID_OK) {
            wxString name = dlg.GetValue();
            while (GetLayoutGroup(name.ToStdString()) != nullptr || name == "Default" || name == "All Models" || name == "Unassigned") {
                wxTextEntryDialog dlg2(this, "Preview of name " + name + " already exists. Enter name for new preview", "Create New Preview");
                OptimiseDialogPosition(&dlg2);
                if (dlg2.ShowModal() == wxID_OK) {
                    name = dlg2.GetValue();
                } else {
                    SwitchChoiceToCurrentLayoutGroup();
                    return;
                }
            }
            //mSelectedGroup = nullptr;
            auto grp = std::make_unique<LayoutGroup>(name.ToStdString(), xlights);
            grp->SetBackgroundImage(xlights->GetDefaultPreviewBackgroundImage());
            xlights->AddPreviewOption(grp.get());
            xlights->LayoutGroups.emplace(name.ToStdString(), std::move(grp));
            SetCurrentLayoutGroup(name.ToStdString());
            AddPreviewChoice(name.ToStdString());
            ChoiceLayoutGroups->SetSelection(ChoiceLayoutGroups->GetCount() - 2);

            //xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnChoiceLayoutGroupsSelect");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnChoiceLayoutGroupsSelect");
            //ShowPropGrid(true);
        } else {
            SwitchChoiceToCurrentLayoutGroup();
            return;
        }
    } else {
        SetCurrentLayoutGroup(choice_layout);
        UpdateModelList(true);
    }
    // When switching to a main group, restore the show's saved 3D mode so that
    // temporary 2D viewing of non-default groups doesn't leave the layout in 2D.
    if (choice_layout == "Default" || choice_layout == "All Models" || choice_layout == "Unassigned") {
        auto* config = GetXLightsConfig();
        bool saved_3d = config->ReadBool("LayoutMode3D", false);
        bool xml_3d = xlights->GetXmlSetting("LayoutMode3D", saved_3d ? "1" : "0") == "1";
        Set3d(xml_3d);
    }
    modelPreview->SetDisplay2DBoundingBox(xlights->GetDisplay2DBoundingBox());
    modelPreview->SetDisplay2DGrid(xlights->GetDisplay2DGrid(), xlights->GetDisplay2DGridSpacing());
    modelPreview->SetDisplay2DCenter0(xlights->GetDisplay2DCenter0());
    modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
    modelPreview->SetScaleBackgroundImage(GetBackgroundScaledForSelectedPreview());
    modelPreview->SetBackgroundBrightness(GetBackgroundBrightnessForSelectedPreview(), GetBackgroundAlphaForSelectedPreview());
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnChoiceLayoutGroupsSelect");

    xlights->SetStoredLayoutGroup(currentLayoutGroup);
    obj_button->Enable(is_3d && currentLayoutGroup == "Default");
}

void LayoutPanel::PreviewSaveImage()
{
	wxImage *image = modelPreview->GrabImage();
	if (image == nullptr)
	{
		
		spdlog::error("SavePreviewImage() - problem grabbing ModelPreview image");

		wxMessageDialog msgDlg(this, _("Error capturing preview image"), _("Image Capture Error"), wxOK | wxCENTRE);
		msgDlg.ShowModal();
		return;
	}

	const char wildcard[] = "JPG files (*.jpg;*.jpeg)|*.jpg;*.jpeg|PNG files (*.png)|*.png";
	wxFileDialog saveDlg(this, _("Save Image"), wxEmptyString, wxEmptyString,
		wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (saveDlg.ShowModal() == wxID_OK)
	{
		wxString path = saveDlg.GetPath();
		wxBitmapType bitmapType = path.EndsWith(".png") ? wxBITMAP_TYPE_PNG : wxBITMAP_TYPE_JPEG;
		image->SaveFile(path, bitmapType);
	}

	delete image;
}

void LayoutPanel::ImportModelsFromPreview(std::list<impTreeItemData*> models, wxString const& layoutGroup, bool includeEmptyGroups, float srcPerUnit)
{
    float scaleFactor = 1.0f;
    if (srcPerUnit > 0.0f && RulerObject::GetRuler() != nullptr) {
        // srcPerUnit is already in metres/pixel (normalised in GetSourceRulerPerUnit).
        // Convert destination perUnit to metres/pixel using the destination ruler's units.
        float dstPerUnit_native = RulerObject::Measure(1.0f);  // dest native-units/pixel
        int   dstUnits          = RulerObject::GetUnits();
        float dstPerUnit_m      = RulerObject::Convert(dstUnits, std::string("m"), dstPerUnit_native);
        if (dstPerUnit_m > 0.0f) {
            scaleFactor = srcPerUnit / dstPerUnit_m;
            spdlog::debug("ImportModelsFromPreview: ruler scale factor={:.6f} (src={:.6f} m/px, dst={:.6f} m/px)",
                          scaleFactor, srcPerUnit, dstPerUnit_m);
        }
    }

    auto scaleNodeAttr = [scaleFactor](pugi::xml_node node, const char* attrName) {
        if (scaleFactor == 1.0f) return;
        auto attr = node.attribute(attrName);
        if (attr) attr.set_value(attr.as_float() * scaleFactor);
    };

    auto scaleModelNode = [&scaleNodeAttr](pugi::xml_node node) {
        scaleNodeAttr(node, "ScaleX");
        scaleNodeAttr(node, "ScaleY");
        scaleNodeAttr(node, "ScaleZ");
        scaleNodeAttr(node, "X2");
        scaleNodeAttr(node, "Y2");
        scaleNodeAttr(node, "Z2");
    };

    //add models first
    for (auto const& it2 : models)
    {
        if (!it2->IsModelGroup())
        {
            std::string newName = it2->GetName();
            if (xlights->AllModels.GetModel(newName) != nullptr) {
                newName = xlights->AllModels.GenerateModelName(it2->GetName());
            }
            it2->GetModelNode().remove_attribute("name");
            it2->GetModelNode().remove_attribute("LayoutGroup");
            it2->GetModelNode().append_attribute("name") = newName;
            it2->GetModelNode().append_attribute("LayoutGroup") = layoutGroup.ToStdString();
            scaleModelNode(it2->GetModelNode());
            xlights->AllModels.createAndAddModel(it2->GetModelNode(), modelPreview->getWidth(), modelPreview->getHeight());
            spdlog::debug("Imported model '{}' as '{}'.", (const char*)it2->GetName().c_str(), (const char*)newName.c_str());
        }
    }

    //add model groups second, skip adding duplicates, just add models to existing group
    for (auto const& it2 : models)
    {
        if (it2->IsModelGroup())//if a group, try to add models if exist
        {
            wxString const smodels = it2->GetModelNode().attribute("models").as_string();
            auto models = wxSplit(smodels, ',');

            models.erase(std::remove_if(models.begin(), models.end(), [&](std::string const& s)
                {
                    return (xlights->AllModels.GetModel(s) == nullptr);
                }), models.end());

            if (models.empty() && !includeEmptyGroups) {
                spdlog::debug("Skipping empty model group '{}'.", (const char*)it2->GetName().c_str());
                continue;
            }

            wxString const name = it2->GetName();
            Model* model = xlights->AllModels.GetModel(name);
            if (model == nullptr) {//if group doesnt exist, create it
                it2->GetModelNode().remove_attribute("LayoutGroup");
                it2->GetModelNode().append_attribute("LayoutGroup") = layoutGroup.ToStdString();
                model = xlights->AllModels.createAndAddModel(it2->GetModelNode(), modelPreview->getWidth(), modelPreview->getHeight());
                spdlog::debug("Imported model group '{}'.", (const char*)name.c_str());
            }

            if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
                ModelGroup *group = (ModelGroup*)model;
                for (const auto& m : models) {
                    // only add model to group if it doesn't already exist
                    if (group->GetModel(m) == nullptr) {
                        group->AddModel(m);
                        spdlog::debug("    Models model group '{}' added model '{}'.", (const char*)name.c_str(), (const char*)m.c_str());
                    }
                }
            }
        }
    }
}

void LayoutPanel::ImportModelsFromRGBEffects()
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
#ifdef __WXOSX__
    wxString wildcard = "*.xml";
#else
    wxString wildcard = "xlights_rgbeffects.xml";
#endif
    wxString filename = wxFileSelector(_("Choose RGB Effects file to import from"), wxEmptyString,
                                       XLIGHTS_RGBEFFECTS_FILE, wxEmptyString,
                                       "RGB Effects Files (xlights_rgbeffects.xml)|" + wildcard,
                                       wxFD_FILE_MUST_EXIST | wxFD_OPEN);
    if (filename.IsEmpty()) return;

    ImportPreviewsModelsDialog dlg(this, filename, xlights->AllModels, xlights->LayoutGroups);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString lg = ChoiceLayoutGroups->GetStringSelection();
        if (lg == "All Models") lg = "Default";

        float srcPerUnit = dlg.GetSourceRulerPerUnit();
        ImportModelsFromPreview(dlg.GetModelsInPreview(""), lg, dlg.GetIncludeEmptyGroups(), srcPerUnit);

        for (const auto& it : dlg.GetPreviews())
        {
            bool found = false;
            for (size_t i = 0; i < ChoiceLayoutGroups->GetCount(); i++)
            {
                if (ChoiceLayoutGroups->GetString(i) == it)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                auto grp = std::make_unique<LayoutGroup>(it.ToStdString(), xlights);
                grp->SetBackgroundImage(xlights->GetDefaultPreviewBackgroundImage());
                xlights->AddPreviewOption(grp.get());
                xlights->LayoutGroups.emplace(it.ToStdString(), std::move(grp));
                AddPreviewChoice(it.ToStdString());
            }
            ImportModelsFromPreview(dlg.GetModelsInPreview(it), it, dlg.GetIncludeEmptyGroups(), srcPerUnit);
        }
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                      OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                      OutputModelManager::WORK_RELOAD_MODELLIST, "LayoutPanel::ImportModelsFromRGBEffects");
    }
}

void LayoutPanel::ImportModelsFromLORS5()
{
    wxString lg = ChoiceLayoutGroups->GetStringSelection();
    if (lg == "All Models") lg = "Default";
    LORPreview lorPreview(xlights, lg);

    if (lorPreview.LoadPreviewFile())
    {
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                                      OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::ImportModelsFromLORS5");
    }
}

void LayoutPanel::PreviewPrintImage()
{
	class Printout : public wxPrintout
	{
	public:
		Printout(ModelPreview *canvas, bool invert) : m_canvas(canvas), _invert(invert) {}
		virtual ~Printout() {
			clearImage();
		}

		void clearImage() {
			if (m_image != nullptr) {
				delete m_image;
				m_image = nullptr;
			}
		}

		virtual bool GrabImage() {
			clearImage();

			wxRect rect = GetLogicalPageRect();
			rect.Deflate(rect.GetWidth() / 20, rect.GetHeight() / 20);
			wxRect adjustedRect = scaledRect(m_canvas->getWidth(), m_canvas->getHeight(), rect.GetWidth(), rect.GetHeight());

			m_image = m_canvas->GrabImage(wxSize(adjustedRect.GetWidth(), adjustedRect.GetHeight()));

            // invert the image for printing
            if (_invert && m_image != nullptr) {
                unsigned char* imgdata = m_image->GetData();
                unsigned int ch = m_image->HasAlpha() ? 4 : 3;
                const int imgdata_size = m_image->GetWidth() * m_image->GetHeight() * ch;
                for (auto i = 0; i < imgdata_size; i += ch) {
                    imgdata[i] = 255 - imgdata[i];
                    imgdata[i + 1] = 255 - imgdata[i + 1];
                    imgdata[i + 2] = 255 - imgdata[i + 2];
                }
            }

			m_grabbedImage = (m_image != nullptr);
			return m_grabbedImage;
		}

		virtual bool OnPrintPage(int page) override {
			if ( GrabImage() == false )
				return false;

			wxDC* dc = GetDC();
			wxRect rect = GetLogicalPageRect();


            wxRect r = scaledRect(m_image->GetWidth(), m_image->GetHeight(), rect.GetWidth(), rect.GetHeight());
            wxAffineMatrix2D mtx;
            double xScale = r.GetWidth() / double(m_image->GetWidth());
            double yScale = r.GetHeight() / double(m_image->GetHeight());
            mtx.Scale(xScale, yScale);
            dc->SetTransformMatrix(mtx);
            dc->DrawBitmap(*m_image, rect.GetTopLeft());
			return true;
		}

		bool grabbedImage() const { return m_grabbedImage; }
	protected:
        ModelPreview *m_canvas = nullptr;
		wxImage *m_image = nullptr;
		bool m_grabbedImage = false;
        bool _invert = false;
	};

	Printout printout(modelPreview, true);

	wxPrintData printdata;
    static wxPrintDialogData printDialogData(printdata);
	wxPrinter printer(&printDialogData);

	if (!printer.Print(this, &printout, true)) {
		if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
			DisplayError(wxString::Format("Problem printing. %d", wxPrinter::GetLastError()).ToStdString());
		}
    }
	else {
		printDialogData = printer.GetPrintDialogData();
        if (!printout.grabbedImage()) {
            DisplayError("Problem grabbing ModelPreview image for printing", this);
        }
    }
}

void LayoutPanel::AddPreviewChoice(const std::string& name)
{
    ChoiceLayoutGroups->Insert(name, ChoiceLayoutGroups->GetCount() - 1);
    model_grp_panel->AddPreviewChoice(name);

    // see if we need to switch to this one
    const std::string& storedLayoutGroup = xlights->GetStoredLayoutGroup();
    if (storedLayoutGroup == name) {
        for (int i = 0; i < (int)ChoiceLayoutGroups->GetCount(); i++) {
            if (ChoiceLayoutGroups->GetString(i) == storedLayoutGroup) {
                SetCurrentLayoutGroup(storedLayoutGroup);
                ChoiceLayoutGroups->SetSelection(i);
                modelPreview->SetDisplay2DBoundingBox(xlights->GetDisplay2DBoundingBox());
                modelPreview->SetDisplay2DGrid(xlights->GetDisplay2DGrid(), xlights->GetDisplay2DGridSpacing());
                modelPreview->SetDisplay2DCenter0(xlights->GetDisplay2DCenter0());
                modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
                modelPreview->SetScaleBackgroundImage(GetBackgroundScaledForSelectedPreview());
                modelPreview->SetBackgroundBrightness(GetBackgroundBrightnessForSelectedPreview(), GetBackgroundAlphaForSelectedPreview());
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::AddPreview");
                break;
            }
        }
    }
}

const wxString& LayoutPanel::GetBackgroundImageForSelectedPreview() {
    previewBackgroundFile = xlights->GetDefaultPreviewBackgroundImage();
    if (pGrp != nullptr && currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned") {
        previewBackgroundFile = pGrp->GetBackgroundImage();
    }
    return previewBackgroundFile;
}

bool LayoutPanel::GetBackgroundScaledForSelectedPreview() {
    previewBackgroundScaled = xlights->GetDefaultPreviewBackgroundScaled();
    if (pGrp != nullptr && currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned") {
        previewBackgroundScaled = pGrp->GetBackgroundScaled();
    }
    return previewBackgroundScaled;
}

int LayoutPanel::GetBackgroundBrightnessForSelectedPreview() {
    previewBackgroundBrightness = xlights->GetDefaultPreviewBackgroundBrightness();
    if (pGrp != nullptr && currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned") {
        previewBackgroundBrightness = pGrp->GetBackgroundBrightness();
    }
    return previewBackgroundBrightness;
}

int LayoutPanel::GetBackgroundAlphaForSelectedPreview()
{
    previewBackgroundAlpha = xlights->GetDefaultPreviewBackgroundAlpha();
    if (pGrp != nullptr && currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned") {
        previewBackgroundAlpha = pGrp->GetBackgroundAlpha();
    }
    return previewBackgroundAlpha;
}

void LayoutPanel::SwitchChoiceToCurrentLayoutGroup() {
    ChoiceLayoutGroups->SetSelection(0);
    for (int i = 0; i < (int)ChoiceLayoutGroups->GetCount(); ++i) {
        if (ChoiceLayoutGroups->GetString(i) == currentLayoutGroup) {
            ChoiceLayoutGroups->SetSelection(i);
            break;
        }
    }
}

void LayoutPanel::DeleteCurrentPreview() {
    if (wxMessageBox("Are you sure you want to delete the " + currentLayoutGroup + " preview?", "Confirm Delete?", wxICON_QUESTION | wxYES_NO) == wxYES) {
        auto it = xlights->LayoutGroups.find(currentLayoutGroup);
        if (it != xlights->LayoutGroups.end()) {
            xlights->RemovePreviewOption(it->second.get());
            xlights->LayoutGroups.erase(it);
        }
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::DeleteCurrentPreview");

        for (size_t i = 0; i < ChoiceLayoutGroups->GetCount(); ++i) {
            if (ChoiceLayoutGroups->GetString(i) == currentLayoutGroup) {
                ChoiceLayoutGroups->Delete(i);
                break;
            }
        }
        // change any existing assignments to this preview to be unassigned
        for (const auto& it : xlights->AllModels) {
            Model* model = it.second;
            if (model->GetLayoutGroup() == currentLayoutGroup) {
                model->SetLayoutGroup("Unassigned");
            }
        }

        SetCurrentLayoutGroup("Default");
        ChoiceLayoutGroups->SetSelection(0);
        xlights->SetStoredLayoutGroup(currentLayoutGroup);

        UpdateModelList(true);
        modelPreview->SetDisplay2DBoundingBox(xlights->GetDisplay2DBoundingBox());
        modelPreview->SetDisplay2DGrid(xlights->GetDisplay2DGrid(), xlights->GetDisplay2DGridSpacing());
        modelPreview->SetDisplay2DCenter0(xlights->GetDisplay2DCenter0());
        modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
        modelPreview->SetScaleBackgroundImage(GetBackgroundScaledForSelectedPreview());
        modelPreview->SetBackgroundBrightness(GetBackgroundBrightnessForSelectedPreview(), GetBackgroundAlphaForSelectedPreview());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::DeleteCurrentPreview");
    }
}

void LayoutPanel::RenameCurrentPreview()
{
    wxTextEntryDialog dlg(this, "Change preview name", "Enter the new preview name:", currentLayoutGroup);
    do {
        if (dlg.ShowModal() == wxCANCEL)
            return;
    } while (::Lower(currentLayoutGroup) != ::Lower(dlg.GetValue()) && GetLayoutGroup(dlg.GetValue().ToStdString()) != nullptr);

    for (const auto& it : xlights->AllModels) {
        Model* model = it.second;
        if (model->GetLayoutGroup() == currentLayoutGroup) {
            model->SetLayoutGroup(dlg.GetValue());
        }
    }

    {
        auto node = xlights->LayoutGroups.extract(currentLayoutGroup);
        if (!node.empty()) {
            node.mapped()->SetName(dlg.GetValue());
            node.key() = dlg.GetValue();
            xlights->LayoutGroups.insert(std::move(node));
        }
    }

     for (size_t i = 0; i < ChoiceLayoutGroups->GetCount(); ++i) {
        if (ChoiceLayoutGroups->GetString(i) == currentLayoutGroup) {
            ChoiceLayoutGroups->SetString(i, dlg.GetValue());
            break;
        }
    }

    currentLayoutGroup = dlg.GetValue();

    SetCurrentLayoutGroup(currentLayoutGroup);
    SwitchChoiceToCurrentLayoutGroup();
    xlights->SetStoredLayoutGroup(currentLayoutGroup);
    UpdateModelList(true);

    modelPreview->SetDisplay2DBoundingBox(xlights->GetDisplay2DBoundingBox());
    modelPreview->SetDisplay2DGrid(xlights->GetDisplay2DGrid(), xlights->GetDisplay2DGridSpacing());
    modelPreview->SetDisplay2DCenter0(xlights->GetDisplay2DCenter0());
    modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
    modelPreview->SetScaleBackgroundImage(GetBackgroundScaledForSelectedPreview());
    modelPreview->SetBackgroundBrightness(GetBackgroundBrightnessForSelectedPreview(), GetBackgroundAlphaForSelectedPreview());

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
                                                  OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                                                  OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::RenameCurrentPreview");
}

void LayoutPanel::DockAndRefresh(bool setModelListHeight) {
    // Shared post-dock sequence used by DockAll() and OnLayoutPaneClose():
    // restore splitter visibility, optionally resize ModelList, commit the layout,
    // then defer sash-minimum enforcement until pending size events have settled.
    UpdateLayoutSplitter();
    if (setModelListHeight) {
        int halfHeight = ModelPanelContainer->GetSize().GetHeight() / 2;
        if (halfHeight < kListHeightFallback) halfHeight = kListHeightFallback;
        layout_mgr->GetPane("ModelList").BestSize(-1, halfHeight);
    }
    layout_mgr->Update();
    // Defer minimum-size enforcement until after pending size events from
    // SplitVertically / layout_mgr->Update() have been processed.  Do NOT
    // force the sash to the target width — that would leave no room to drag left.
    CallAfter([this]() {
        if (SplitterWindow2->IsSplit()) {
            if (SplitterWindow2->GetSashPosition() < kMinPaneWidth)
                SplitterWindow2->SetSashPosition(kMinPaneWidth);
            SplitterWindow2->SetMinimumPaneSize(kMinPaneWidth);
        }
    });
}

void LayoutPanel::DockAll() {
    if (layout_mgr == nullptr) return;
    // If floating panes were stashed by HideFloatingPanes() (e.g. startup with
    // Layout tab already active, or called from a non-Layout context), restore
    // them now so the loop below sees them as floating+shown and can dock them.
    if (!_savedFloatingPerspective.empty()) {
        RestoreFloatingPanes();
    }
    wxAuiPaneInfoArray& panes = layout_mgr->GetAllPanes();
    bool hasModelList = false;
    bool update = false;
    for (size_t i = 0; i < panes.GetCount(); i++) {
        if (panes[i].IsFloating()) {
            if (panes[i].name == "ModelList") {
                panes[i].Top().Dock();
                hasModelList = true;
            } else {
                panes[i].Center().Dock();
            }
            update = true;
        }
    }
    if (update) {
        DockAndRefresh(hasModelList);
    }
}

void LayoutPanel::ResetToDefaults() {
    if (!_auiInitialized || layout_mgr == nullptr) return;

    // If panes were stashed as floating+hidden by HideFloatingPanes() (tab-switch),
    // restore them first so the AUI manager sees them in their actual floating state
    // before we force-dock them below.
    if (!_savedFloatingPerspective.empty()) {
        layout_mgr->LoadPerspective(_savedFloatingPerspective);
        _savedFloatingPerspective.clear();
    }

    // Dock all panels to their default positions: ModelList at top, ModelSettings
    // in the center, ModelGroupSettings docked but hidden.
    layout_mgr->GetPane("ModelList").Top().Layer(0).Row(0).Dock().Show();
    layout_mgr->GetPane("ModelSettings").Center().Dock().Show();
    {
        wxAuiPaneInfo& mgp = layout_mgr->GetPane("ModelGroupSettings");
        if (mgp.IsOk()) mgp.Center().Dock().Hide();
    }

    // Split ModelList and ModelSettings evenly (50/50).
    int halfHeight = ModelPanelContainer->GetSize().GetHeight() / 2;
    if (halfHeight < kListHeightFallback) halfHeight = kListHeightFallback;
    layout_mgr->GetPane("ModelList").BestSize(-1, halfHeight);

    // Ensure the left panel is visible in the splitter, then commit the AUI layout.
    // Also discard any saved sash position so the default 18% width is used on the
    // next float/dock cycle (UpdateLayoutSplitter reads _savedSashPos).
    _savedSashPos = -1;
    UpdateLayoutSplitter();
    layout_mgr->Update();

    // Reset the vertical sash to the default 18% position with proportional
    // gravity (same behaviour as first launch with no saved config).
    int targetW = LeftPanelMinWidth();
    SplitterWindow2->SetSashGravity(0.5);
    SplitterWindow2->SetSashPosition(targetW);
    SplitterWindow2->SetMinimumPaneSize(kMinPaneWidth);

    // Clear saved state so the defaults persist across restarts.
    auto* config = GetXLightsConfig();
    config->DeleteEntry("LayoutAUIPerspective2");
    config->DeleteEntry("LayoutMainSplitterSash");
    config->DeleteEntry("LayoutModelSplitterSash");

    // Re-populate the property editor so the Background Properties panel
    // isn't empty after the reset.  Defer via CallAfter so any pending
    // window-resize / sash-minimum events from DockAndRefresh settle first;
    // interacting with the property grid before those events flush can cause
    // re-entrant property changes and a lockup.
    CallAfter([this]() {
        showBackgroundProperties();
    });
}


void LayoutPanel::OnLayoutPaneClose(wxAuiManagerEvent& event) {
    event.Veto();
    wxAuiPaneInfo* pane = event.GetPane();
    if (pane == nullptr) return;
    wxString name = pane->name;
    CallAfter([this, name]() {
        if (layout_mgr == nullptr) return;
        wxAuiPaneInfo& p = layout_mgr->GetPane(name);
        if (!p.IsOk()) return;
        if (name == "ModelList") {
            p.Top().Dock();
        } else {
            p.Center().Dock();
        }
        DockAndRefresh(name == "ModelList");
    });
}

void LayoutPanel::HideFloatingPanes() {
    if (layout_mgr == nullptr) return;
    wxAuiPaneInfoArray& panes = layout_mgr->GetAllPanes();
    bool hasFloating = false;
    for (size_t i = 0; i < panes.GetCount(); i++) {
        if (panes[i].IsFloating() && panes[i].IsShown()) {
            hasFloating = true;
            break;
        }
    }
    if (!hasFloating) return;
    _savedFloatingPerspective = layout_mgr->SavePerspective();
    for (size_t i = 0; i < panes.GetCount(); i++) {
        if (panes[i].IsFloating() && panes[i].IsShown()) {
            panes[i].Hide();
        }
    }
    layout_mgr->Update();
}

void LayoutPanel::RestoreFloatingPanes() {
    if (layout_mgr == nullptr || _savedFloatingPerspective.empty()) return;
    layout_mgr->LoadPerspective(_savedFloatingPerspective);
    // Reapply pane configuration that LoadPerspective may overwrite via SafeSet(),
    // but preserve the visibility restored from the saved perspective.
    wxAuiPaneInfo& modelListPane = layout_mgr->GetPane("ModelList");
    if (modelListPane.IsOk()) {
        modelListPane.MinSize(300, kPaneMinHeight).CaptionVisible(true).Caption("Groups/Models List")
            .Floatable(true).CloseButton(false).TopDockable(true).BottomDockable(true).LeftDockable(false).RightDockable(false);
    }
    wxAuiPaneInfo& modelSettingsPane = layout_mgr->GetPane("ModelSettings");
    if (modelSettingsPane.IsOk()) {
        wxString modelSettingsCaption = modelSettingsPane.caption;
        if (modelSettingsCaption.IsEmpty()) {
            modelSettingsCaption = "Model Settings";
        }
        modelSettingsPane.MinSize(0, kPaneMinHeight).CaptionVisible(true)
            .Caption(modelSettingsCaption)
            .Floatable(true).CloseButton(false).TopDockable(false).BottomDockable(false).LeftDockable(false).RightDockable(false);
    }
    wxAuiPaneInfo& modelGroupSettingsPane = layout_mgr->GetPane("ModelGroupSettings");
    if (modelGroupSettingsPane.IsOk()) {
        modelGroupSettingsPane.MinSize(0, kPaneMinHeight).CaptionVisible(true).Caption("Group Settings")
            .CloseButton(false).TopDockable(false).BottomDockable(false).LeftDockable(false).RightDockable(false);
    }
    layout_mgr->Update();
    _savedFloatingPerspective.clear();
    UpdateLayoutSplitter();
}

int LayoutPanel::LeftPanelMinWidth() const
{
    int totalW = SplitterWindow2->GetClientSize().GetWidth();
    return std::max(totalW * 18 / 100, kMinPaneWidth);
}

void LayoutPanel::UpdateLayoutSplitter() {
    if (!_auiInitialized || layout_mgr == nullptr) return;

    // Check if any relevant pane is docked (shown and not floating).
    bool anyDocked = false;
    for (const char* nm : {"ModelList", "ModelSettings", "ModelGroupSettings"}) {
        wxAuiPaneInfo& p = layout_mgr->GetPane(nm);
        if (p.IsOk() && p.IsShown() && !p.IsFloating()) {
            anyDocked = true;
            break;
        }
    }

    if (!anyDocked) {
        // All panes are floating or hidden — collapse the left panel so the
        // preview canvas expands to fill the full available width.
        if (SplitterWindow2->IsSplit()) {
            _savedSashPos = SplitterWindow2->GetSashPosition();
            SplitterWindow2->SetMinimumPaneSize(0);
            SplitterWindow2->Unsplit(LeftPanel);
            // The GL canvas got a new (larger) size from the sizer but the GL
            // viewport isn't updated until the next paint.  Force a repaint now
            // so the newly-exposed area to the right doesn't stay black.
            if (modelPreview) modelPreview->Refresh();
        }
    } else {
        // Target width: 18% of splitter width, floor kMinPaneWidth.  This is where the
        // left panel is placed when (re-)docking.  The hard minimum below is
        // intentionally smaller so the user can drag the sash further left.
        // If the user had previously adjusted the sash, restore their position.
        int targetW = (_savedSashPos >= kMinPaneWidth) ? _savedSashPos : LeftPanelMinWidth();

        if (!SplitterWindow2->IsSplit()) {
            SplitterWindow2->SplitVertically(LeftPanel, PreviewGLPanel, targetW);
        } else {
            int sash = SplitterWindow2->GetSashPosition();
            if (sash < kMinPaneWidth) {
                SplitterWindow2->SetSashPosition(kMinPaneWidth);
            }
        }
        SplitterWindow2->SetMinimumPaneSize(kMinPaneWidth);
    }
}

void LayoutPanel::ShowPropGrid(bool show) {
    if (show) {
        wxAuiPaneInfo& mgs = layout_mgr->GetPane("ModelGroupSettings");
        wxAuiPaneInfo& ms  = layout_mgr->GetPane("ModelSettings");
        // If GroupSettings was floating, bring ModelSettings up in the same
        // position so the "settings pane" doesn't jump to the dock.
        if (mgs.IsOk() && ms.IsOk() && mgs.IsFloating() && mgs.IsShown()) {
            ms.Float()
              .FloatingPosition(mgs.floating_pos)
              .FloatingSize(mgs.floating_size);
        }
        mgs.Hide();
        ms.Caption("Model Settings").Show();
        layout_mgr->Update();
        mPropGridActive = true;
    } else {
        wxAuiPaneInfo& ms  = layout_mgr->GetPane("ModelSettings");
        wxAuiPaneInfo& mgs = layout_mgr->GetPane("ModelGroupSettings");
        // If ModelSettings was floating, bring GroupSettings up in the same
        // position so the "settings pane" doesn't jump to the dock.
        if (ms.IsOk() && mgs.IsOk() && ms.IsFloating() && ms.IsShown()) {
            mgs.Float()
              .FloatingPosition(ms.floating_pos)
              .FloatingSize(ms.floating_size);
        }
        ms.Caption("Background Properties").Hide();
        mgs.Caption("Group Settings").Show();
        layout_mgr->Update();
        mPropGridActive = false;
    }
    // If panels are floating, SplitterWindow2 may have LeftPanel unsplit (zero
    // size). Showing or hiding a docked pane needs the splitter restored so the
    // newly-shown panel is actually visible.
    UpdateLayoutSplitter();
}

void LayoutPanel::SetCurrentLayoutGroup(const std::string& group)
{
    currentLayoutGroup = group;
    auto it = xlights->LayoutGroups.find(group);
    if (it != xlights->LayoutGroups.end()) {
        pGrp = it->second.get();
        modelPreview->SetActiveLayoutGroup(group);
        return;
    }
    modelPreview->SetActiveLayoutGroup(group);
}

void LayoutPanel::OnItemContextMenu(wxTreeListEvent& event)
{
    if (!editing_models) {
        objects_panel->OnItemContextMenu(event);
        return;
    }

    wxMenu mnuContext;

    if (selectedTreeGroups.size() == 0) {
        if (selectedTreeSubModels.size() == 0) {
            if (selectedTreeModels.size() == 1) {
                auto par = TreeListViewModels->GetItemParent(selectedTreeModels[0]);
                if (par != TreeListViewModels->GetRootItem()) {
                    mnuContext.Append(ID_MNU_REMOVE_MODEL_FROM_GROUP, "Remove Model From Group");
                    mnuContext.AppendSeparator();
                }
            }
            else 
            if (selectedTreeModels.size() > 1) {
                auto parent = TreeListViewModels->GetItemParent(selectedTreeModels[0]);
                bool allSameParent = true;
                bool allLocked = true;
                bool allUnlocked = true;
                bool allFromBase = true;
                for (auto &i : selectedTreeModels) {
                    if (parent != TreeListViewModels->GetItemParent(i)) {
                        allSameParent = false;
                    }

                    ModelTreeData* data = (ModelTreeData*)TreeListViewModels->GetItemData(i);
                    Model* model = ((data != nullptr) ? data->GetModel() : nullptr);
                    if (model != nullptr) {
                        if (model->IsLocked()) {
                            allUnlocked = false;
                        } else {
                            allLocked = false;
                        }
                        allFromBase = allFromBase && model->IsFromBase();
                    }
                }
                auto lm = mnuContext.Append(ID_PREVIEW_MODEL_LOCK, "Lock Models");
                lm->Enable(!allLocked);
                auto um = mnuContext.Append(ID_PREVIEW_MODEL_UNLOCK, "Unlock Models");
                um->Enable(!allUnlocked);
                auto ul = mnuContext.Append(ID_PREVIEW_MODEL_UNLINKFROMBASE, "Unlink Models from Base Show Folder");
                ul->Enable(allFromBase);

                if (allSameParent && parent != TreeListViewModels->GetRootItem()) {
                    mnuContext.Append(ID_MNU_REMOVE_MODEL_FROM_GROUP, "Remove Models From Group");
                }
                auto dm = mnuContext.Append(ID_MNU_DELETE_MODEL, "Delete Models");
                dm->Enable(!allLocked);

                mnuContext.AppendSeparator();
            }
        } else {
            auto par = TreeListViewModels->GetItemParent(selectedTreeSubModels[0]);
            if (par != TreeListViewModels->GetRootItem()) {
                if ((selectedTreeSubModels.size() + selectedTreeModels.size()) == 1) {
                    mnuContext.Append(ID_MNU_REMOVE_MODEL_FROM_GROUP, "Remove Model From Group");
                } else {
                    mnuContext.Append(ID_MNU_REMOVE_MODEL_FROM_GROUP, "Remove Models From Group");
                }
                mnuContext.AppendSeparator();
                if (selectedTreeSubModels.size() == 1) {
                    mnuContext.Append(ID_MNU_EDIT_SUBMODEL_ALIAS, "Add/Edit SubModel Alias");
                    mnuContext.AppendSeparator();
                }
            }
        }
    }

    if (selectedTreeModels.size() == 1 && selectedTreeGroups.size() + selectedTreeSubModels.size() == 0) {
        AddSingleModelOptionsToBaseMenu(mnuContext);
        ModelTreeData* data = (ModelTreeData*)TreeListViewModels->GetItemData(selectedTreeModels[0]);
        Model* model = ((data != nullptr) ? data->GetModel() : nullptr);
        auto dm = mnuContext.Append(ID_MNU_DELETE_MODEL, "Delete Model");
        if (model != nullptr) {
            dm->Enable(!model->IsLocked());
        }
        // Remove preview 'Create Group' option as it may be confusing with tree list 'Create Group from Selections'
        mnuContext.Remove(ID_PREVIEW_MODEL_CREATEGROUP);
        // Remove preview option 'Add to Existing Group' as it is added with the other group options below
        wxMenuItem* addToExisting = mnuContext.FindItem(ID_MNU_ADD_TO_EXISTING_GROUPS);
        if (addToExisting != nullptr) {
            mnuContext.Remove(ID_MNU_ADD_TO_EXISTING_GROUPS);
        }
        mnuContext.AppendSeparator();
    }

    // add model menu options if only models selected and selected > 1n
    if (selectedTreeModels.size() > 1 && selectedTreeGroups.size() + selectedTreeSubModels.size() == 0) {
        wxMenu* mnuBulkEdit = new wxMenu();
        AddBulkEditOptionsToMenu(mnuBulkEdit);
        mnuBulkEdit->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnModelsPopup, nullptr, this);

        wxMenu* mnuAlign = new wxMenu();
        AddAlignOptionsToMenu(mnuAlign);
        mnuAlign->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnModelsPopup, nullptr, this);

        wxMenu* mnuDistribute = new wxMenu();
        AddDistributeOptionsToMenu(mnuDistribute);
        mnuDistribute->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnModelsPopup, nullptr, this);

        wxMenu* mnuResize = new wxMenu();
        AddResizeOptionsToMenu(mnuResize);
        mnuResize->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnModelsPopup, nullptr, this);

        mnuContext.Append(ID_PREVIEW_BULKEDIT, "Bulk Edit", mnuBulkEdit, "");
        mnuContext.Append(ID_PREVIEW_ALIGN, "Align", mnuAlign, "");
        mnuContext.Append(ID_PREVIEW_DISTRIBUTE, "Distribute", mnuDistribute, "");
        mnuContext.Append(ID_PREVIEW_RESIZE, "Resize", mnuResize, "");

        mnuContext.AppendSeparator();
    }

    mnuContext.Append(ID_MNU_ADD_MODEL_GROUP, "Add Empty Group");
    if ((selectedTreeModels.size() + selectedTreeSubModels.size() + selectedTreeGroups.size()) > 0) {
        for (const auto& m : xlights->AllModels) {
            if (m.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
                // adjust label for tree
                mnuContext.Append(ID_MNU_ADD_TO_EXISTING_GROUPS, "Add Selections to Existing Groups");
                break;
            }
        }
        if (selectedTreeModels.size() == 1) {
            mnuContext.Append(ID_MNU_REMOVE_FROM_EXISTING_GROUPS, "Remove from Existing Groups");
        }
        mnuContext.Append(ID_PREVIEW_MODEL_CREATEGROUP, "Create Group from Selections");
    }

    if (selectedTreeModels.size() == 0 && selectedTreeSubModels.size() == 0) {
        if (selectedTreeGroups.size() > 1) {
            mnuContext.Append(ID_MNU_DELETE_MODEL_GROUP, "Delete Groups");
        }

        if (selectedTreeGroups.size() == 1) {
            ModelTreeData* data = (ModelTreeData*)TreeListViewModels->GetItemData(selectedTreeGroups[0]);
            Model* model = ((data != nullptr) ? data->GetModel() : nullptr);
            mnuContext.Append(ID_MNU_DELETE_MODEL_GROUP, "Delete Group");
            mnuContext.Append(ID_MNU_RENAME_MODEL_GROUP, "Rename Group")->Enable(!model->IsFromBase());
            mnuContext.Append(ID_MNU_CLONE_MODEL_GROUP, "Clone Group");
            auto ul = mnuContext.Append(ID_PREVIEW_MODEL_UNLINKFROMBASE, "Unlink Group from Base Show Folder");
            ul->Enable(model->IsFromBase());
        }
    }

    mnuContext.Append(ID_MNU_DELETE_EMPTY_MODEL_GROUPS, "Delete Empty Groups");

    mnuContext.Append(ID_MNU_DELETE_ALL_ALIASES, "Delete All Aliases");

    if (selectedTreeGroups.size() > 1) {
        mnuContext.AppendSeparator();
        mnuContext.Append(ID_MNU_BULKEDIT_GROUP_TAGCOLOR, "Bulk Edit Tag Color");
        mnuContext.Append(ID_MNU_BULKEDIT_GROUP_PREVIEW, "Bulk Edit Preview");
    }

    bool foundInvalid = false;
    bool foundOverlapping = false;

    if (selectedTreeModels.size() == 1 && selectedTreeSubModels.size() == 0 && selectedTreeGroups.size() == 0) {
        ModelTreeData* data = (ModelTreeData*)TreeListViewModels->GetItemData(selectedTreeModels[0]);
        Model* model = ((data != nullptr) ? data->GetModel() : nullptr);
        if (model) {
            if (!model->CouldComputeStartChannel || !model->IsValidStartChannelString()) {
                mnuContext.Append(ID_MNU_MAKESCVALID, "Make Start Channel Valid");
                foundInvalid = true;
            }
            if (xlights->AllModels.IsModelOverlapping(model)) {
                foundOverlapping = true;
                mnuContext.Append(ID_MNU_MAKESCVALID, "Make Start Channel Not Overlapping");
            }
        }
    }

    if (!foundOverlapping && !foundInvalid) { // no point looking again if we already know we have an issue
        for (const auto& it : xlights->AllModels) {
            if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup) {
                if (!foundInvalid && (!it.second->CouldComputeStartChannel || !it.second->IsValidStartChannelString())) {
                    foundInvalid = true;
                    if (foundOverlapping) break;
                }
                if (!foundOverlapping && xlights->AllModels.IsModelOverlapping(it.second)) {
                    foundOverlapping = true;
                    if (foundInvalid) break;
                }
            }
        }
    }

    if (foundInvalid) {
        mnuContext.Append(ID_MNU_MAKEALLSCVALID, "Make All Start Channels Valid");
    }
    if (foundOverlapping) {
        mnuContext.Append(ID_MNU_MAKEALLSCNOTOVERLAPPING, "Make All Start Channels Not Overlapping");
    }

    mnuContext.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&LayoutPanel::OnModelsPopup, nullptr, this);
    PopupMenu(&mnuContext);
}

static inline void SetToolTipForTreeList(wxTreeListCtrl *tv, const std::string &tip) {
#ifdef __WXMSW__
    if (tip == "") {
        tv->GetView()->UnsetToolTip();
    } else {
        tv->GetView()->SetToolTip(tip);
    }
#else
    if (tip == "") {
        tv->UnsetToolTip();
    } else {
        tv->SetToolTip(tip);
    }
#endif
}

void LayoutPanel::OnSelectionChanged(wxTreeListEvent& event)
{
    if (editing_models) {
        HandleSelectionChanged();
    } else {
        UnSelectAllModels(false);
        ViewObject* view_object = nullptr;
        bool show_prop_grid = objects_panel->OnSelectionChanged(event, &view_object, currentLayoutGroup);
        SelectViewObject(view_object, false);
        ShowPropGrid(show_prop_grid);
        selectedBaseObject = view_object;
    }
}

void LayoutPanel::HandleSelectionChanged() {

    

    // Even when Tree is Frozen which happens during full refresh this event is still fired on DeleteItem()/DeleteItems()
    // and randomly causes crash when model is nullptr, so bail when Frozen.  Also make sure tooltip is empty and property
    // grid is shown so background props show after full refresh when nothing is selected.
    if (TreeListViewModels->IsFrozen()) {
        //ShowPropGrid(true);
        showBackgroundProperties();
        SetToolTipForTreeList(TreeListViewModels, "");
        return;
    }

    wxStopWatch sw;

    BaseObject* lastSelectedBaseObject = selectedBaseObject;
    Model* lastSelectedModel = dynamic_cast<Model*>(lastSelectedBaseObject);
    wxTreeListItems selectedItems;
    TreeListViewModels->GetSelections(selectedItems);

    UnSelectAllModels(false);
    resetPropertyGrid();

    if (sw.Time() > 500)
        spdlog::debug("        LayoutPanel::HandleSelectionChanged after reset of property grid {}ms", sw.Time());

    if (selectedItems.size() > 0) {
        bool isPrimary = false;
        if (selectedItems.size() == 1) {
            isPrimary = true;
        }

        for (const auto& item : selectedItems) {
            Model* model = GetModelFromTreeItem(item);
            if (model != nullptr) {
                #ifdef __LINUX__
                                // This seems to happen only on Linux so prevent the crash
                                if (!xlights->AllModels.IsModelValid(model)) {
                                    spdlog::debug("LINUX ONLY Error: LayoutPanel::OnSelectionChanged Model is Not Valid pointer. This would have crashed. Ignoring.");
                                    return;
                                }
                #elif defined(__WXOSX__)
                                // Given I am seeing these crashes on OSX but not windows I suspect like LINUX these crashes occur
                                // If is likely due to differences in the order messages arrive on the different platforms that results in invalid pointers
                                // This code will prove that theory
                                if (!xlights->AllModels.IsModelValid(model)) {
                                    spdlog::critical("LayoutPanel::OnSelectionChanged model was not valid ... this is going to crash.");
                                }
                #else
                                wxASSERT(xlights->AllModels.IsModelValid(model));
                #endif
                if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
                    selectedTreeGroups.push_back(item);
                    SetTreeGroupModelsSelected(model, isPrimary);
                } else if (model->GetDisplayAs() == DisplayAsType::SubModel) {
                    selectedTreeSubModels.push_back(item);
                    SetTreeSubModelSelected(model, isPrimary);
                } else {
                    selectedTreeModels.push_back(item);
                    if (model == lastSelectedModel || lastSelectedBaseObject == nullptr || isPrimary) {
                        selectedPrimaryTreeItem = item;
                        SetTreeModelSelected(model, true);
                    } else {
                        SetTreeModelSelected(model, false);
                    }
                }
            }
        }
        if (sw.Time() > 500)
            spdlog::debug("        LayoutPanel::HandleSelectionChanged after select in tree {}ms", sw.Time());

        // if we still don't have a primary model selected then force one if we can
        if (selectedPrimaryTreeItem == nullptr) {
            if (selectedTreeModels.size() > 0) {
                Model* model = GetModelFromTreeItem(selectedTreeModels[0]);
                SetTreeModelSelected(model, true);
                selectedPrimaryTreeItem = selectedTreeModels[0];
            } else if (selectedTreeSubModels.size() > 0) {
                Model* model = GetModelFromTreeItem(selectedTreeSubModels[0]);
                SetTreeSubModelSelected(model, true);
            } else if (selectedTreeGroups.size() > 0){
                Model* model = GetModelFromTreeItem(selectedTreeGroups[0]);
                SetTreeGroupModelsSelected(model, true);
            }
        }

        if (sw.Time() > 500)
            spdlog::debug("        LayoutPanel::HandleSelectionChanged after force select {}ms", sw.Time());

        // determine which panel and tooltip to show if any
        int mSize = selectedTreeModels.size();
        int gSize = selectedTreeGroups.size();
        int smSize = selectedTreeSubModels.size();
        int totalSelections = mSize + smSize + gSize;

        std::string tooltip = "";

        if (totalSelections > 1) {
            showBackgroundProperties();
            tooltip = wxString::Format("Selected Items:\n -Groups: %d\n -Models: %d\n -SubModels: %d\n\nTotal Nodes: %d", gSize, mSize, smSize, calculateNodeCountOfSelected());
        } else if (gSize == 1) {
            Model* model = GetModelFromTreeItem(selectedTreeGroups[0]);
            if (model->IsFromBase()) {
                tooltip = "From Base Show Folder";
            } else {
                tooltip = wxString::Format("Total Nodes in Group: %d", calculateNodeCountOfSelected());
            }
            ShowPropGrid(false);
            model_grp_panel->UpdatePanel(TreeListViewModels->GetItemText(selectedTreeGroups[0]));
            model_grp_panel->Show();
        } else if (smSize == 1) {
            Model* subModel = GetModelFromTreeItem(selectedTreeSubModels[0]);
            SetupPropGrid(subModel);
            ShowPropGrid(true);
        } else if (mSize == 1) {
            Model* model = GetModelFromTreeItem(selectedTreeModels[0]);
            if (model != nullptr) {
                tooltip = xlights->GetChannelToControllerMapping(model->GetNumberFromChannelString(model->ModelStartChannel)) + "Nodes: " + wxString::Format("%d", (int)model->GetNodeCount()).ToStdString();
                if (model->IsFromBase()) {
                    tooltip += "\nFrom Base Show Folder";
                }
            } else {
                spdlog::critical("LayoutPanel::HandleSelectionChanged Model was selected and now is null, this should not have happened.");
            }
            if (selectedBaseObject->GetBaseObjectScreenLocation().hasX2()) {
                const TwoPointScreenLocation& screenLoc = dynamic_cast<const TwoPointScreenLocation&>(selectedBaseObject->GetBaseObjectScreenLocation());
                glm::vec3 loc = screenLoc.GetWorldPosition();
                float x1 = loc.x;
                float y1 = loc.y;
                float x2 = screenLoc.GetX2();
                float y2 = screenLoc.GetY2();
                if (x2 < x1 && std::abs(x2 - x1) > 30.0) {
                    if (!tooltip.empty()) {
                        tooltip += "\n";
                    }
                    tooltip += "Warning: Model is perhaps flipped left to right.";
                }
                if (y2 < y1 && std::abs(x2 - x1) < 30.0) {
                    if (!tooltip.empty()) {
                        tooltip += "\n";
                    }
                    tooltip += "Warning: Model is perhaps flipped top to bottom.";
                }
            }
            SetupPropGrid(model);
            ShowPropGrid(true);
        } else {
            spdlog::critical("LayoutPanel::HandleSelectionChanged No models selected after processing, this should not have happen, when we started there were {} selections.", selectedItems.size());
            showBackgroundProperties();
        }

        SetToolTipForTreeList(TreeListViewModels, tooltip);

        if (sw.Time() > 500)
            spdlog::debug("        LayoutPanel::HandleSelectionChanged after tooltip {}ms", sw.Time());

        // removing below or Keyboard Cut/Copy/Paste/etc will not fire when making selections in preview
        // #ifndef LINUX
        // TreeListViewModels->SetFocus();
        // #endif

        auto pos = selectedBaseObject->GetBaseObjectScreenLocation().GetWorldPosition();
        if (Is3d()) {
            auto pos = selectedBaseObject->GetBaseObjectScreenLocation().GetWorldPosition();
            xlights->SetStatusText(wxString::Format("x=%.2f y=%.2f z=%.2f %s", pos.x, pos.y, pos.z, selectedBaseObject->GetDimension()));
        } else {
            xlights->SetStatusText(wxString::Format("x=%.2f y=%.2f", pos.x, pos.y));
        }

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::HandleSelectionChanged");

    } else {
        selectedBaseObject = nullptr;
        UnSelectAllModels(true);
        showBackgroundProperties();
        SetToolTipForTreeList(TreeListViewModels, "");
        xlights->SetStatusText("");
    }

    if (sw.Time() > 500)
        spdlog::debug("        LayoutPanel::HandleSelectionChanged took {}ms", sw.Time());
}

void LayoutPanel::ModelGroupUpdated(ModelGroup *grp) {

    if (grp == nullptr) return;

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
                                                  OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                                                  OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::ModelGroupUpdated", nullptr, nullptr, grp->GetName());

    std::vector<Model *> models;
    UpdateModelList(true, models);
}

CopyPasteBaseObject::CopyPasteBaseObject() {
}

CopyPasteBaseObject::CopyPasteBaseObject(const std::string& in)
{
    // check it looks like xml ... to stop parser errors
    wxString xml = in;
    if (!xml.StartsWith("<?xml") || (!xml.Contains("<model ") && !xml.Contains("<view_object "))) {
        // not valid
        return;
    }

	if (xml.Contains("<view_object ")) {
		_viewObject = true;
	}

    _xmlDoc = std::make_shared<pugi::xml_document>();
    pugi::xml_parse_result result = _xmlDoc->load_string(in.c_str());

    if (!result || !_xmlDoc->document_element()) {
        // not valid
        _xmlDoc.reset();
        return;
    }

    _ok = true;
}

void CopyPasteBaseObject::SetBaseObject(BaseObject* model)
{
    _xmlDoc.reset();
    _ok = false;
    if (model != nullptr) {
        _xmlDoc = std::make_shared<pugi::xml_document>();
        XmlSerializingVisitor visitor{ _xmlDoc.get() };
        model->Accept(visitor);

        if (_xmlDoc->document_element()) {
            _ok = true;
        } else {
            _xmlDoc.reset();
        }
    }
}

std::string CopyPasteBaseObject::Serialise() const
{
    if (!_xmlDoc) {
        return "";
    } else {
        std::ostringstream stream;
        _xmlDoc->save(stream);
        return stream.str();
    }
}

void LayoutPanel::OnCheckBox_3DClick(wxCommandEvent& event)
{
    is_3d = CheckBox_3D->GetValue();

    modelPreview->Set3D(is_3d);
    if (is_3d) {
        if (selectedBaseObject != nullptr) {
            selectionLatched = true;
            highlightedBaseObject = selectedBaseObject;
            selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
        } else {
            UnSelectAllModels();
        }
	    Notebook_Objects->AddPage(PanelObjects, _("3D Objects"), false);
    } else {
        editing_models = true;
        Model *m = dynamic_cast<Model*>(selectedBaseObject);
        if (m == nullptr) {
            UnSelectAllModels();
        }
        Notebook_Objects->RemovePage(1);
    }
    obj_button->Enable(is_3d && ChoiceLayoutGroups->GetStringSelection() == "Default");

    GetXLightsConfig()->Write("LayoutMode3D", is_3d);
    Refresh();
}

bool LayoutPanel::HandleLayoutKeyBinding(wxKeyEvent& event) {
    

    auto k = event.GetKeyCode();

    if ((event.ControlDown() || event.CmdDown()) && event.ShiftDown() && k == 'F') { // consume ctrl+shift+F and ctrl+F keybinding and observe the next key press
        ctrlshiftFPressed = true;
        return true;
    } else if ((event.ControlDown() || event.CmdDown()) && k == 'F') {
        ctrlFPressed = true;
        return true;
    }

    if (k == WXK_SHIFT || k == WXK_CONTROL || k == WXK_ALT)
        return false;

    if ((!event.ControlDown() && !event.CmdDown() && !event.AltDown()) ||
        (k == 'A' && (event.ControlDown() || event.CmdDown()) && !event.AltDown())) {
        // Let Control + A through
        // Just a regular key ... If current focus is a control then we need to not process this
        if (dynamic_cast<wxControl*>(event.GetEventObject()) != nullptr &&
            (k < 128 || k == WXK_NUMPAD_END || k == WXK_NUMPAD_HOME || k == WXK_NUMPAD_INSERT || k == WXK_HOME || k == WXK_END || k == WXK_NUMPAD_SUBTRACT || k == WXK_NUMPAD_DECIMAL)) {
            return false;
        }
    }

    if ((ctrlFPressed || ctrlshiftFPressed) && wxIsalpha(k)) {
        wxTreeListItems currentItems;
        TreeListViewModels->GetSelections(currentItems);

        if (currentItems.empty()) return false;

        wxChar letter = event.GetUnicodeKey();
        if (!wxIsalpha(letter)) return false;

        wxTreeListItem startItem = lastFoundItem.IsOk() ? lastFoundItem : currentItems[0];
        wxTreeListItem nextItem = TreeListViewModels->GetNextItem(startItem);
        bool found = false;

        while (nextItem.IsOk() && !found) {
            if (TreeListViewModels->GetItemParent(nextItem) == TreeListViewModels->GetRootItem() || ctrlshiftFPressed) {
                wxString itemName = TreeListViewModels->GetItemText(nextItem, 0);
                if (wxToupper(itemName.GetChar(0)) == wxToupper(letter)) {
                    TreeListViewModels->UnselectAll();
                    TreeListViewModels->Select(nextItem);
                    TreeListViewModels->EnsureVisible(nextItem);
                    lastFoundItem = nextItem;
                    found = true;
                    HandleSelectionChanged();
                    break;
                }
            }
            nextItem = TreeListViewModels->GetNextItem(nextItem);
        }
        if (!found) {       // If not found, wrap around to start
            nextItem = TreeListViewModels->GetFirstItem();
            while (nextItem.IsOk() && nextItem != startItem && !found) {
                if (TreeListViewModels->GetItemParent(nextItem) == TreeListViewModels->GetRootItem() || ctrlshiftFPressed) {
                    wxString itemName = TreeListViewModels->GetItemText(nextItem, 0);
                    if (wxToupper(itemName.GetChar(0)) == wxToupper(letter)) {
                        TreeListViewModels->UnselectAll();
                        TreeListViewModels->Select(nextItem);
                        TreeListViewModels->EnsureVisible(nextItem);
                        lastFoundItem = nextItem;
                        break;
                    }
                }
                nextItem = TreeListViewModels->GetNextItem(nextItem);
            }
        }
        ctrlFPressed = ctrlshiftFPressed = false;
        return true;
    }

    // Handle other keybindings
    auto binding = xlights->GetMainSequencer()->keyBindings.Find(event, KBSCOPE::Layout);
    if (binding != nullptr) {
        std::string type = binding->GetType();
        if (type == "LOCK_MODEL") {
            LockSelectedModels(true);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
                                                          OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::HandleLayoutKey::LOCK_MODEL");
        } else if (type == "UNLOCK_MODEL") {
            LockSelectedModels(false);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
                                                          OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::HandleLayoutKey::UNLOCK_MODEL");
        } else if (type == "GROUP_MODELS") {
            CreateModelGroupFromSelected();
        } else if (type == "WIRING_VIEW") {
            ShowWiring();
        } else if (type == "EXPORT_MODEL_CAD") {
            ExportModelAsCAD();
        } else if (type == "EXPORT_LAYOUT_DXF") {
            ExportLayoutDXF();
        } else if (type == "NODE_LAYOUT") {
            ShowNodeLayout();
        } else if (type == "MODEL_SUBMODELS") {
            EditSubmodels();
        } else if (type == "MODEL_FACES") {
            EditFaces();
        } else if (type == "MODEL_STATES") {
            EditStates();
        } else if (type == "MODEL_MODELDATA") {
            EditModelData();
        } else if (type == "SAVE_LAYOUT") {
            SaveEffects();
            xlights->SaveNetworksFile();
        } else if (type == "MODEL_ALIGN_TOP") {
            PreviewModelAlignTops();
        } else if (type == "MODEL_ALIGN_BOTTOM") {
            PreviewModelAlignBottoms();
        } else if (type == "MODEL_ALIGN_LEFT") {
            PreviewModelAlignLeft();
        } else if (type == "MODEL_ALIGN_RIGHT") {
            PreviewModelAlignRight();
        } else if (type == "MODEL_ALIGN_CENTER_VERT") {
            PreviewModelAlignVCenter();
        } else if (type == "MODEL_ALIGN_CENTER_HORIZ") {
            PreviewModelAlignHCenter();
        } else if (type == "MODEL_ALIGN_BACKS") {
            PreviewModelAlignBacks();
        } else if (type == "MODEL_ALIGN_FRONTS") {
            PreviewModelAlignFronts();
        } else if (type == "MODEL_ALIGN_GROUND") {
            PreviewModelAlignWithGround();
        } else if (type == "MODEL_DISTRIBUTE_HORIZ") {
            PreviewModelHDistribute();
        } else if (type == "SELECT_ALL_MODELS") {
            SelectAllModels();
        } else if (type == "MODEL_DISTRIBUTE_VERT") {
            PreviewModelVDistribute();
        } else if (type == "MODEL_FLIP_VERT") {
            PreviewModelFlipV();
        } else if (type == "MODEL_FLIP_HORIZ") {
            PreviewModelFlipH();
        } else {
            spdlog::warn("Keybinding '{}' not recognised.", (const char*)type.c_str());
            wxASSERT(false);
            return false;
        }
        event.StopPropagation();
        return true;
    }

    return xlights->HandleAllKeyBinding(event);
}

void LayoutPanel::OnNotebook_ObjectsPageChanged(wxNotebookEvent& event)
{
#ifdef __WXOSX__
    UnSelectAllModels();
#else
    UnSelectAllModelsInTree();
#endif
    if (Notebook_Objects->GetPageText(Notebook_Objects->GetSelection()) == "Models") {
        editing_models = true;
    } else {
        editing_models = false;
    }
}

bool LayoutPanel::IsNewModel(Model* m) const
{
    // if nullptr is passed then it is true if the new model pointer is valid
    if (m == nullptr) return _newModel != nullptr;

    return m == _newModel;
}

//Calculate the total node count of selected items in the panel. Handles calculations of models, submodels and groups
int LayoutPanel::calculateNodeCountOfSelected()
{
    int totalNodeCount = 0;
    std::vector<Model*> modelsProcessed;
    //We can break the selected groups into their components for processing. GetSelectedModelsForEdit already does this, even though we aren't editing. We can reuse that logic. This gives us all models, so I want to split this back up into models and submodels
    std::vector<Model*> selectedModels;
    std::vector<Model*> selectedSubModels;
    
    for (const auto& item : selectedTreeSubModels){
        ModelTreeData *submodelData = (ModelTreeData*)TreeListViewModels->GetItemData(item);
        Model* subModel = ((submodelData != nullptr) ? submodelData->GetModel() : nullptr);
        if( subModel )
            selectedSubModels.push_back(subModel);
    }
    
    //Now parse the group elements into their perspective groups for processing
    std::vector<Model*> sgModels = GetSelectedModelsForEdit(true);
    for (const auto& item : sgModels){
        if (item->GetDisplayAs() == DisplayAsType::SubModel) {
            selectedSubModels.push_back(item);
        } else {
            selectedModels.push_back(item);
        }
    }
    
    //Process the core models first. Save their pointer addresses for use in submodel and group processing
    for (const auto& model : selectedModels) {
        //If any of this models submodels are already counted, we shouldn't count the nodes from that submodel twice
        if (model != nullptr) {
            if(std::find(modelsProcessed.begin(), modelsProcessed.end(), model) == modelsProcessed.end()){
                totalNodeCount += model->GetNodeCount();
                modelsProcessed.push_back(model);
            }
        }
    }
    // Now process submodels. Submodels might already be counted from the above parent models, so dont process a submodel if it's parent is already processed and accounted for.
    for (const auto& subModel : selectedSubModels) {
        Model* parent_info = dynamic_cast<SubModel*>(subModel)->GetParent();
        if (subModel != nullptr) {
            //if this submodel is already in the count, dont do it
            if(std::find(modelsProcessed.begin(), modelsProcessed.end(), parent_info) == modelsProcessed.end()
               && std::find(modelsProcessed.begin(), modelsProcessed.end(), subModel) == modelsProcessed.end()
               )
            {
                totalNodeCount += subModel->GetNodeCount();
                modelsProcessed.push_back(subModel);
            }
        }
    }
    
    return totalNodeCount;
}

void LayoutPanel::OnModelFilterCancelBtn(wxCommandEvent& event) {
    ModelFilterCtrl->SetValue("");
    _filterString = "";
    _filterRegexValid = false;
    UpdateModelList(true);
}

void LayoutPanel::OnModelFilterTextChanged(wxCommandEvent& event) {
    _filterString = ModelFilterCtrl->GetValue().Trim();
    _filterRegex.Compile(_filterString, wxRE_ICASE);
    _filterRegexValid = _filterRegex.IsValid();
    UpdateModelList(true);
}

bool LayoutPanel::ModelMatchesFilter(Model* model) const {
    if (ModelFilterCtrl == nullptr || _filterString.IsEmpty()) return true;

    if (_filterRegexValid)
        return _filterRegex.Matches(model->GetName());

    return wxString(model->GetName()).Lower().Contains(_filterString.Lower());
}

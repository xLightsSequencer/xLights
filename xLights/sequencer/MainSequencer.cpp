/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/
 
//(*InternalHeaders(MainSequencer)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dcbuffer.h>
#include <wx/event.h>
#include <wx/clipbrd.h>
#include <wx/artprov.h>

#include "MainSequencer.h"
#include "SequenceElements.h"
#include "../xLightsMain.h"
#include "TimeLine.h"
#include "../UtilFunctions.h"
#include "../xLightsVersion.h"
#include "../EffectsPanel.h"
#include "../ExternalHooks.h"
#include "../effects/RenderableEffect.h"
#include "../graphics/xlGraphicsBase.h"

#include <log4cpp/Category.hh>

//(*IdInit(MainSequencer)
const long MainSequencer::ID_CHOICE_VIEW_CHOICE = wxNewId();
const long MainSequencer::ID_PANEL1 = wxNewId();
const long MainSequencer::ID_PANEL3 = wxNewId();
const long MainSequencer::ID_PANEL6 = wxNewId();
const long MainSequencer::ID_PANEL2 = wxNewId();
const long MainSequencer::ID_SCROLLBAR_EFFECTS_VERTICAL = wxNewId();
const long MainSequencer::ID_CHECKBOX1 = wxNewId();
const long MainSequencer::ID_SCROLLBAR_EFFECT_GRID_HORZ = wxNewId();
//*)

wxDEFINE_EVENT(EVT_HORIZ_SCROLL, wxCommandEvent);
wxDEFINE_EVENT(EVT_SCROLL_RIGHT, wxCommandEvent);

BEGIN_EVENT_TABLE(MainSequencer,wxPanel)
    EVT_MOUSEWHEEL(MainSequencer::mouseWheelMoved)

    EVT_COMMAND(wxID_ANY, EVT_HORIZ_SCROLL, MainSequencer::HorizontalScrollChanged)
    EVT_COMMAND(wxID_ANY, EVT_SCROLL_RIGHT, MainSequencer::ScrollRight)
    EVT_COMMAND(wxID_ANY, EVT_TIME_LINE_CHANGED, MainSequencer::TimelineChanged)
    EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_CHANGED, MainSequencer::SequenceChanged)
    EVT_COMMAND(wxID_ANY, EVT_WAVE_FORM_HIGHLIGHT, MainSequencer::TimeLineSelectionChanged)

END_EVENT_TABLE()

void MainSequencer::SetHandlers(wxWindow *window)
{
    if (window) {
        window->Connect(wxID_ANY,
                        wxEVT_CHAR,
                        wxKeyEventHandler(MainSequencer::OnChar),
                        (wxObject*) nullptr,
                        this);
        window->Connect(wxID_ANY,
                        wxEVT_CHAR_HOOK,
                        wxKeyEventHandler(MainSequencer::OnCharHook),
                        (wxObject*) nullptr,
                        this);
        window->Connect(wxID_ANY,
                        wxEVT_KEY_DOWN,
                        wxKeyEventHandler(MainSequencer::OnKeyDown),
                        (wxObject*) nullptr,
                        this);

        wxWindowList &list = window->GetChildren();
        for (wxWindowList::iterator it = list.begin(); it != list.end(); ++it) {
            wxWindow* pclChild = *it;
            SetHandlers(pclChild);
        }

        window->Connect(wxID_CUT, wxEVT_MENU, (wxObjectEventFunction)&MainSequencer::DoCut,0,this);
        window->Connect(wxID_COPY, wxEVT_MENU, (wxObjectEventFunction)&MainSequencer::DoCopy,0,this);
        window->Connect(wxID_PASTE, wxEVT_MENU, (wxObjectEventFunction)&MainSequencer::DoPaste,0,this);
        window->Connect(wxID_UNDO, wxEVT_MENU, (wxObjectEventFunction)&MainSequencer::DoUndo,0,this);
        window->Connect(wxID_REDO, wxEVT_MENU, (wxObjectEventFunction)&MainSequencer::DoRedo,0,this);
    }
}

class TimeDisplayControl : public GRAPHICS_BASE_CLASS
{
public:
    TimeDisplayControl(wxPanel* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                       const wxSize &size=wxDefaultSize, long style=0)
    : GRAPHICS_BASE_CLASS(parent, id, pos, size, style, "TimeDisplay") {
        _time = "Time: 00:00:00";
        _selected = "";
        _fps = "";
        newFontSize = 14;
        fontSize = 0;
        SetColors();
    }
    virtual ~TimeDisplayControl(){};

    void SetGLSize(int w, int h) {
        SetMinSize(wxSize(w, h));

        wxSize size = GetSize();
        if (w == -1) w = size.GetWidth();
        if (h == -1) h = size.GetHeight();

        SetSize(w, h);
        if (h > 50) {
            newFontSize = 14;
        } else if (h > 36) {
            newFontSize = 10;
        } else if (h > 25) {
            newFontSize = 8;
        } else {
            newFontSize = 6;
        }
        Refresh();
    }

    void SetSelected(const wxString &sel) {
        _selected = sel;
        Refresh();
    }
    void SetLabels(const wxString &time, const wxString &fps) {
        _fps = fps;
        _time = time;
        render();
    }

    protected:
    DECLARE_EVENT_TABLE()
    void Paint( wxPaintEvent& event ) {
        wxPaintDC dc(this);
        render();
    }

    virtual xlColor ClearBackgroundColor() const override {
        return bgColor;
    }

    void SetColors() {
        wxColor c = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
        AdjustColorToDeviceColorspace(c, bgColor.red, bgColor.green, bgColor.blue, bgColor.alpha);
        fgColor = ColorManager::instance()->GetColor(ColorManager::COLOR_ROW_HEADER_TEXT);
    }
    void OnSysColourChanged(wxSysColourChangedEvent& event) {
        event.Skip();
        SetColors();
        Refresh();
    }
public:
    void render() override {
        if(!IsShownOnScreen()) return;
        if(!mIsInitialized) {
            PrepareCanvas();
        }

        xlGraphicsContext *ctx = PrepareContextForDrawing();
        if (ctx == nullptr) {
            return;
        }
        ctx->SetViewport(0, 0, mWindowWidth, mWindowHeight);
        
        float fs = translateToBacking(fontSize);
        if (newFontSize != fontSize || backedFontSize != fs) {
            if (fontTexture) {
                delete fontTexture;
                fontTexture = nullptr;
            }
            fontSize = newFontSize;
            fs = translateToBacking(fontSize);
            backedFontSize = fs;
            const xlFontInfo &fi = xlFontInfo::FindFont((int)fs);
            fontTexture = ctx->createTextureForFont(fi);
        }
        const xlFontInfo &fi = xlFontInfo::FindFont(fs);
#define LINEGAP 1.2
        xlVertexTextureAccumulator *vta = ctx->createVertexTextureAccumulator();
        float x = mapLogicalToAbsolute(5);
        float perLine = mapLogicalToAbsolute(fontSize) * LINEGAP;
        float y = perLine;
        float factor = drawingUsingLogicalSize() ? translateToBacking(1.0) : 1.0;
        fi.populate(*vta, x, y, _time, factor);
        y += perLine;
        // only display FPS if we have room
        if ((y + perLine <= mWindowHeight)
            || (_selected == "" && y <= mWindowHeight)) {
            fi.populate(*vta, x, y, _fps, factor);
            y += perLine;
        }
        if (y <= mWindowHeight) {
            fi.populate(*vta, x, y, _selected, factor);
        }
        ctx->enableBlending();
        ctx->drawTexture(vta, fontTexture, fgColor);
        delete vta;
        FinishDrawing(ctx);
    }

private:
    xlColor bgColor;
    xlColor fgColor;
    xlTexture *fontTexture = nullptr;

    std::string _time;
    std::string _fps;
    std::string _selected;

    int fontSize = 0;
    int backedFontSize = 0;
    int newFontSize = 14;
};

BEGIN_EVENT_TABLE(TimeDisplayControl, GRAPHICS_BASE_CLASS)
EVT_PAINT(TimeDisplayControl::Paint)
EVT_SYS_COLOUR_CHANGED(TimeDisplayControl::OnSysColourChanged)
END_EVENT_TABLE()

MainSequencer::MainSequencer(wxWindow* parent, bool smallWaveform, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("                Creating main sequencer");

    //(*Initialize(MainSequencer)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer4;
    wxStaticText* StaticText1;

    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxWANTS_CHARS, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(3, 3, 0, 0);
    FlexGridSizer1->AddGrowableCol(1);
    FlexGridSizer1->AddGrowableRow(1);
    FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    StaticText1 = new wxStaticText(this, wxID_ANY, _("View:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText1, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ViewChoice = new wxChoice(this, ID_CHOICE_VIEW_CHOICE, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_VIEW_CHOICE"));
    FlexGridSizer2->Add(ViewChoice, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 0, wxEXPAND, 0);
    FlexGridSizer4 = new wxFlexGridSizer(2, 0, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    FlexGridSizer4->AddGrowableRow(1);
    PanelTimeLine = new TimeLine(this, ID_PANEL1, wxDefaultPosition, wxDLG_UNIT(this,wxSize(-1,15)), wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    PanelTimeLine->SetMinSize(wxDLG_UNIT(this,wxSize(-1,15)));
    PanelTimeLine->SetMaxSize(wxDLG_UNIT(this,wxSize(-1,15)));
    FlexGridSizer4->Add(PanelTimeLine, 1, wxALL|wxEXPAND, 0);
    PanelWaveForm = new Waveform(this, ID_PANEL3, wxDefaultPosition, wxDLG_UNIT(this,wxSize(-1,40)), wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    PanelWaveForm->SetMinSize(wxDLG_UNIT(this,wxSize(-1,40)));
    PanelWaveForm->SetMaxSize(wxDLG_UNIT(this,wxSize(-1,40)));
    FlexGridSizer4->Add(PanelWaveForm, 1, wxALL|wxEXPAND, 0);
    FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 0);
    FlexGridSizer1->Add(-1,-1,1, wxALL|wxEXPAND, 5);
    PanelRowHeadings = new RowHeading(this, ID_PANEL6, wxDefaultPosition, wxDLG_UNIT(this,wxSize(90,-1)), wxTAB_TRAVERSAL, _T("ID_PANEL6"));
    PanelRowHeadings->SetMinSize(wxDLG_UNIT(this,wxSize(90,-1)));
    PanelRowHeadings->SetMaxSize(wxDLG_UNIT(this,wxSize(90,-1)));
    FlexGridSizer1->Add(PanelRowHeadings, 1, wxALL|wxEXPAND, 0);
    PanelEffectGrid = new EffectsGrid(this, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("ID_PANEL2"));
    FlexGridSizer1->Add(PanelEffectGrid, 1, wxALL|wxEXPAND, 0);
    ScrollBarEffectsVertical = new wxScrollBar(this, ID_SCROLLBAR_EFFECTS_VERTICAL, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL|wxALWAYS_SHOW_SB, wxDefaultValidator, _T("ID_SCROLLBAR_EFFECTS_VERTICAL"));
    ScrollBarEffectsVertical->SetScrollbar(0, 1, 10, 1);
    FlexGridSizer1->Add(ScrollBarEffectsVertical, 1, wxALL|wxEXPAND, 0);
    CheckBox_SuspendRender = new wxCheckBox(this, ID_CHECKBOX1, _("Suspend Render"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_SuspendRender->SetValue(false);
    FlexGridSizer1->Add(CheckBox_SuspendRender, 1, wxALL|wxEXPAND, 0);
    ScrollBarEffectsHorizontal = new wxScrollBar(this, ID_SCROLLBAR_EFFECT_GRID_HORZ, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL|wxALWAYS_SHOW_SB, wxDefaultValidator, _T("ID_SCROLLBAR_EFFECT_GRID_HORZ"));
    ScrollBarEffectsHorizontal->SetScrollbar(0, 1, 100, 1);
    FlexGridSizer1->Add(ScrollBarEffectsHorizontal, 1, wxALL|wxEXPAND, 0);
    SetSizer(FlexGridSizer1);

    Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
    Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_TOP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
    Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_BOTTOM,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
    Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_LINEUP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
    Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_LINEDOWN,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
    Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_PAGEUP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
    Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_PAGEDOWN,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
    Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
    Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
    Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&MainSequencer::OnCheckBox_SuspendRenderClick);
    Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
    Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_TOP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
    Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_BOTTOM,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
    Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_LINEUP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsHorizontalScrollLineUp);
    Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_LINEDOWN,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsHorizontalScrollLineDown);
    Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_PAGEUP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
    Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_PAGEDOWN,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
    Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
    Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
    //*)

#ifdef __WXOSX__
    wxFont fnt = CheckBox_SuspendRender->GetFont();
    fnt.SetFractionalPointSize(10.0);
    CheckBox_SuspendRender->SetFont(fnt);
#endif

    logger_base.debug("                Create time display control");
    timeDisplay = new TimeDisplayControl(this, wxID_ANY);
    FlexGridSizer2->Add(timeDisplay, 1, wxALL |wxEXPAND, 0);
    //FlexGridSizer2->AddGrowableRow(3);

    FlexGridSizer2->Fit(this);
    FlexGridSizer2->SetSizeHints(this);

    if (smallWaveform)
    {
        SetSmallWaveform();
    }
    else
    {
        SetLargeWaveform();
    }

    _savedTopModel = "";
    mParent = parent;

    logger_base.debug("                Set handlers");
    SetHandlers(this);

    logger_base.debug("                Load key bindings");
    keyBindings.LoadDefaults();
    mCanUndo = false;
    mPasteByCell = false;
    // ReSharper disable CppVirtualFunctionCallInsideCtor
    SetName("MainSequencer");
    // ReSharper restore CppVirtualFunctionCallInsideCtor

    logger_base.debug("                Initialise touch bar");
#ifdef __XLIGHTS_HAS_TOUCHBARS__
    touchBarSupport.Init(this);
#endif
}

MainSequencer::~MainSequencer()
{
    mSequenceElements->SetTimeLine(nullptr);
    timeDisplay = nullptr; // wxWidgets will delete it

	//(*Destroy(MainSequencer)
	//*)
}

void MainSequencer::SetSequenceElements(SequenceElements* elements)
{
    mSequenceElements = elements;
}

void MainSequencer::UpdateEffectGridVerticalScrollBar()
{
    int position = mSequenceElements->GetFirstVisibleModelRow();
    int range = mSequenceElements->GetTotalNumberOfModelRows();
    int pageSize = mSequenceElements->GetMaxModelsDisplayed();
    int thumbSize = mSequenceElements->GetMaxModelsDisplayed();
    ScrollBarEffectsVertical->SetScrollbar(position,thumbSize,range,pageSize);
    ScrollBarEffectsVertical->Refresh();
    PanelEffectGrid->Draw();
    PanelRowHeadings->Refresh();
}

void MainSequencer::UpdateTimeDisplay(int time_ms, float fps)
{
    int time = time_ms >= 0 ? time_ms : 0;
    int msec = time % 1000;
    int seconds = time / 1000;
    int minutes = seconds / 60;
    seconds = seconds % 60;
    wxString play_time = wxString::Format("Time: %d:%02d.%02d", minutes, seconds, msec);
    wxString fpsStr;
    if (fps >= 0) {
        fpsStr = wxString::Format("FPS: %5.1f", fps);
    }
    if (timeDisplay != nullptr) {
        timeDisplay->SetLabels(play_time, fpsStr);
    }
}

void MainSequencer::UpdateSelectedDisplay(int selected)
{
    if (selected == 0) {
        timeDisplay->SetSelected("");
    } else {
        timeDisplay->SetSelected(wxString::Format("Selected: %s", FORMATTIME(selected)));
    }
}
void MainSequencer::OnScrollBarEffectGridHorzScroll(wxScrollEvent& event)
{
    int position = ScrollBarEffectsHorizontal->GetThumbPosition();
    int timeLength = PanelTimeLine->GetTimeLength();

    int startTime = (int)(((double)position/(double)timeLength) * (double)timeLength);
    PanelTimeLine->SetStartTimeMS(startTime);
    UpdateEffectGridHorizontalScrollBar();
}

void MainSequencer::OnScrollBarEffectsVerticalScrollChanged(wxScrollEvent& event)
{
    int position = ScrollBarEffectsVertical->GetThumbPosition();
    int scroll = mSequenceElements->SetFirstVisibleModelRow(position);
    PanelEffectGrid->ScrollBy(scroll);
    UpdateEffectGridVerticalScrollBar();
}

void MainSequencer::mouseWheelMoved(wxMouseEvent& event)
{
    bool fromTrackPad = IsMouseEventFromTouchpad();
    int i = event.GetWheelRotation();
    if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL) {
        int position = ScrollBarEffectsHorizontal->GetThumbPosition();
        int ts = ScrollBarEffectsHorizontal->GetThumbSize() / 10;
        if (fromTrackPad) {
            ts = PanelTimeLine->GetTimeMSfromPosition(std::abs(event.GetWheelRotation()));
        }
        if (ts ==0) {
            ts = 1;
        }
        if (i < 0) {
            if (position > 0) {
                position -= ts;
            }
        } else if (i > 0) {
            position += ts;
        }
        if (position >= ScrollBarEffectsHorizontal->GetRange()) {
            position = ScrollBarEffectsHorizontal->GetRange() - 1;
        }
        ScrollBarEffectsHorizontal->SetThumbPosition(position);
        wxCommandEvent eventScroll(EVT_HORIZ_SCROLL);
        HorizontalScrollChanged(eventScroll);
    } else {
        int position = ScrollBarEffectsVertical->GetThumbPosition();
        int positionAdj = 1;
        if (fromTrackPad) {
            static int accumulated = 0;
            static long long lastEventTime = 0;
            long long eventTime = event.GetTimestamp();
            long long diffTime = eventTime - lastEventTime;
            lastEventTime = eventTime;
            if (diffTime > 500) {
                accumulated = 0;
            }
            if (accumulated < 0 && event.GetWheelRotation() > 0) {
                if (std::abs(accumulated) > DEFAULT_ROW_HEADING_HEIGHT) {
                    positionAdj = std::abs(accumulated) / DEFAULT_ROW_HEADING_HEIGHT;
                }
                accumulated = i;
                i = -1;
            } else if (accumulated > 0 && event.GetWheelRotation() < 0) {
                if (std::abs(accumulated) > DEFAULT_ROW_HEADING_HEIGHT) {
                    positionAdj = std::abs(accumulated) / DEFAULT_ROW_HEADING_HEIGHT;
                }
                accumulated = i;
                i = 1;
            } else {
                accumulated += event.GetWheelRotation();
                if (std::abs(accumulated) > DEFAULT_ROW_HEADING_HEIGHT) {
                    positionAdj = std::abs(accumulated) / DEFAULT_ROW_HEADING_HEIGHT;
                    if (accumulated > 0) {
                        accumulated -= positionAdj * DEFAULT_ROW_HEADING_HEIGHT;
                    } else {
                        accumulated += positionAdj * DEFAULT_ROW_HEADING_HEIGHT;
                    }
                } else {
                    positionAdj = 0;
                }
                if (event.GetWheelRotation() == 0) {
                    accumulated = 0;
                }
            }
        }
        
        if (positionAdj) {
            if (i < 0) {
                if (position < ScrollBarEffectsVertical->GetRange()-1) {
                    position += positionAdj;
                    ScrollBarEffectsVertical->SetThumbPosition(position);
                    int scroll = mSequenceElements->SetFirstVisibleModelRow(position);
                    PanelEffectGrid->ScrollBy(scroll);
                }
            } else if (i > 0) {
                if (position > 0) {
                    position -= positionAdj;
                    ScrollBarEffectsVertical->SetThumbPosition(position);
                    int scroll = mSequenceElements->SetFirstVisibleModelRow(position);
                    PanelEffectGrid->ScrollBy(scroll);
                }
            }
        }
        mSequenceElements->PopulateVisibleRowInformation();
        PanelEffectGrid->ForceRefresh();  // call this so we can check if we need to update which effects are selected
        PanelRowHeadings->Refresh();
    }
}

bool MainSequencer::HandleSequencerKeyBinding(wxKeyEvent& event)
{
    log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (mSequenceElements != nullptr) {

        auto k = event.GetKeyCode();
        if (k == WXK_SHIFT || k == WXK_CONTROL || k == WXK_ALT || k == WXK_RAW_CONTROL) return false;

        if ((!event.ControlDown() && !event.CmdDown() && !event.AltDown() && !event.RawControlDown()) ||
            (k == 'A' && (event.ControlDown() || event.CmdDown() || event.RawControlDown()) && !event.AltDown())) {
            // Just a regular key ... If current focus is a control then we need to not process this
            if (dynamic_cast<wxControl*>(event.GetEventObject()) != nullptr &&
                (k < 128 || k == WXK_NUMPAD_END || k == WXK_NUMPAD_HOME || k == WXK_NUMPAD_INSERT || k == WXK_HOME || k == WXK_END || k == WXK_NUMPAD_SUBTRACT || k == WXK_NUMPAD_DECIMAL)) {
                return false;
            }
        }

        auto binding = keyBindings.Find(event, KBSCOPE::Sequence);
        if (binding != nullptr) {
            std::string type = binding->GetType();
            if (type == "TIMING_ADD") {
                InsertTimingMarkFromRange();
            }
            else if (type == "TIMING_SPLIT") {
                SplitTimingMark();
            }
            else if (type == "ZOOM_IN") {
                PanelTimeLine->ZoomIn();
            }
            else if (type == "ZOOM_OUT") {
                PanelTimeLine->ZoomOut();
            }
            else if (type == "ZOOM_SEL") {
                PanelTimeLine->ZoomSelection();
            }
            else if (type == "RANDOM") {
                Effect* ef = PanelEffectGrid->Paste("Random\t\t\n", xlights_version_string);
                SelectEffect(ef);
            }
            else if (type == "EFFECT") {
                if (PanelEffectGrid->GetSelectedEffect() == nullptr) {
                    // reset default settings if appropriate
                    if (binding->GetEffectName() != mSequenceElements->GetXLightsFrame()->GetEffectsPanel()->EffectChoicebook->GetChoiceCtrl()->GetStringSelection()) {
                        mSequenceElements->GetXLightsFrame()->ResetPanelDefaultSettings(binding->GetEffectName(), nullptr, true);
                    }

                    Effect* ef = PanelEffectGrid->Paste(binding->GetEffectName() + "\t" + binding->GetEffectString() + _("\t\n"), binding->GetEffectDataVersion());
                    if (ef != nullptr) {
                        SelectEffect(ef);
                    }
                }
            }
            else if (type == "APPLYSETTING") {
                SettingsMap newSetting = SettingsMap();
                newSetting.Parse(nullptr, binding->GetEffectString(), "");

                // Apply setting on the UI
                mSequenceElements->GetXLightsFrame()->SetEffectControls(newSetting);

                // Now apply it to all selected effects
                for (const auto& s : newSetting) {
                    ApplyEffectSettingToSelected("", s.first, s.second, nullptr, "");
                }
            }
            else if (type == "PRESET") {
                Effect* ef = mSequenceElements->GetXLightsFrame()->ApplyEffectsPreset(binding->GetEffectName());
                PanelEffectGrid->SelectEffect(ef);
            }
            else if (type == "EFFECT_SETTINGS_TOGGLE") {
                wxCommandEvent e;
                mSequenceElements->GetXLightsFrame()->ShowHideEffectSettingsWindow(e);
            }
            else if (type == "SAVE_SEQUENCE") {
                mSequenceElements->GetXLightsFrame()->SaveSequence();
            }
            else if (type == "SAVEAS_SEQUENCE") {
                mSequenceElements->GetXLightsFrame()->SaveAsSequence();
            }
            else if (type == "EFFECT_ASSIST_TOGGLE") {
                wxCommandEvent e;
                mSequenceElements->GetXLightsFrame()->ShowHideEffectAssistWindow(e);
            }
            else if (type == "COLOR_TOGGLE") {
                wxCommandEvent e;
                mSequenceElements->GetXLightsFrame()->ShowHideColorWindow(e);
            }
            else if (type == "LAYER_SETTING_TOGGLE") {
                wxCommandEvent e;
                mSequenceElements->GetXLightsFrame()->ShowHideBufferSettingsWindow(e);
            }
            else if (type == "LAYER_BLENDING_TOGGLE") {
                wxCommandEvent e;
                mSequenceElements->GetXLightsFrame()->ShowHideLayerTimingWindow(e);
            }
            else if (type == "MODEL_PREVIEW_TOGGLE") {
                ToggleModelPreview();
            }
            else if (type == "HOUSE_PREVIEW_TOGGLE") {
                ToggleHousePreview();
            }
            else if (type == "EFFECTS_TOGGLE") {
                wxCommandEvent e;
                mSequenceElements->GetXLightsFrame()->ShowHideEffectDropper(e);
            }
            else if (type == "DISPLAY_ELEMENTS_TOGGLE") {
                wxCommandEvent e;
                mSequenceElements->GetXLightsFrame()->ShowHideDisplayElementsWindow(e);
            }
            else if (type == "JUKEBOX_TOGGLE") {
                wxCommandEvent e;
                mSequenceElements->GetXLightsFrame()->OnMenuItem_JukeboxSelected(e);
            }
            else if (type == "LOCK_EFFECT") {
                PanelEffectGrid->LockEffects(true);
            }
            else if (type == "CANCEL_RENDER") {
                CancelRender();
            }
            else if (type == "TOGGLE_RENDER") {
                CheckBox_SuspendRender->SetValue(!CheckBox_SuspendRender->GetValue());
                ToggleRender(CheckBox_SuspendRender->GetValue());
            }
            else if (type == "UNLOCK_EFFECT") {
                PanelEffectGrid->LockEffects(false);
            }
            else if (type == "MARK_SPOT") {
                SavePosition();
            }
            else if (type == "RETURN_TO_SPOT") {
                RestorePosition();
            }
            else if (type == "PRIOR_TAG") {
                PanelTimeLine->GoToPriorTag();
            }
            else if (type == "NEXT_TAG") {
                PanelTimeLine->GoToNextTag();
            }
            else if (type == "EFFECT_DESCRIPTION") {
                PanelEffectGrid->SetEffectsDescription();
            }
            else if (type == "EFFECT_ALIGN_START") {
                PanelEffectGrid->AlignSelectedEffects(EFF_ALIGN_MODE::ALIGN_START_TIMES);
            }
            else if (type == "EFFECT_ALIGN_END") {
                PanelEffectGrid->AlignSelectedEffects(EFF_ALIGN_MODE::ALIGN_END_TIMES);
            }
            else if (type == "EFFECT_ALIGN_BOTH") {
                PanelEffectGrid->AlignSelectedEffects(EFF_ALIGN_MODE::ALIGN_BOTH_TIMES);
            }
            else if (type == "INSERT_LAYER_ABOVE") {
                PanelEffectGrid->InsertEffectLayerAbove();
            }
            else if (type == "AUDIO_FULL_SPEED") {
                mSequenceElements->GetXLightsFrame()->SetPlaySpeedTo(1.0);
            }
            else if (type == "AUDIO_F_1_5_SPEED") {
                mSequenceElements->GetXLightsFrame()->SetPlaySpeedTo(1.5);
            }
            else if (type == "AUDIO_F_2_SPEED") {
                mSequenceElements->GetXLightsFrame()->SetPlaySpeedTo(2.0);
            }
            else if (type == "AUDIO_F_3_SPEED") {
                mSequenceElements->GetXLightsFrame()->SetPlaySpeedTo(3.0);
            }
            else if (type == "AUDIO_F_4_SPEED") {
                mSequenceElements->GetXLightsFrame()->SetPlaySpeedTo(4.0);
            }
            else if (type == "AUDIO_S_3_4_SPEED") {
                mSequenceElements->GetXLightsFrame()->SetPlaySpeedTo(0.75);
            }
            else if (type == "AUDIO_S_1_2_SPEED") {
                mSequenceElements->GetXLightsFrame()->SetPlaySpeedTo(0.5);
            }
            else if (type == "AUDIO_S_1_4_SPEED") {
                mSequenceElements->GetXLightsFrame()->SetPlaySpeedTo(0.25);
            }
            else if (type == "SELECT_ALL") {
                mSequenceElements->SelectAllEffects();
                PanelEffectGrid->Refresh();
            }
            else if (type == "SELECT_ALL_NO_TIMING") {
                mSequenceElements->SelectAllEffectsNoTiming();
                PanelEffectGrid->Refresh();
            }
            else if (type == "INSERT_LAYER_BELOW") {
                PanelEffectGrid->InsertEffectLayerBelow();
            }
            else if (type == "TOGGLE_ELEMENT_EXPAND") {
                PanelEffectGrid->ToggleExpandElement(PanelRowHeadings);
            }
            else if (type == "SHOW_PRESETS") {
                mSequenceElements->GetXLightsFrame()->ShowPresetsPanel();
            }
            else if (type == "PRESETS_TOGGLE") {
                mSequenceElements->GetXLightsFrame()->TogglePresetsPanel();
            }
            else if (type == "VALUECURVES_TOGGLE") {
                wxCommandEvent e;
                mSequenceElements->GetXLightsFrame()->OnMenuItem_ValueCurvesSelected(e);
            }
            else if (type == "COLOR_DROPPER_TOGGLE") {
                wxCommandEvent e;
                mSequenceElements->GetXLightsFrame()->OnMenuItem_ColourDropperSelected(e);
            }
            else if (type == "SEARCH_TOGGLE") {
                wxCommandEvent e;
                mSequenceElements->GetXLightsFrame()->OnMenuItemSelectEffectSelected(e);
            }
            else if (type == "PERSPECTIVES_TOGGLE") {
                wxCommandEvent e;
                mSequenceElements->GetXLightsFrame()->ShowHidePerspectivesWindow(e);
            }
            else if (type == "EFFECT_UPDATE") {
                wxCommandEvent eventEffectUpdated(EVT_EFFECT_UPDATED);
                wxPostEvent(GetParent(), eventEffectUpdated);
            }
            else if (type == "COLOR_UPDATE") {
                wxCommandEvent eventEffectUpdated(EVT_EFFECT_PALETTE_UPDATED);
                wxPostEvent(GetParent(), eventEffectUpdated);
            }
            else if (type == "MODEL_TOGGLE") {
                
                PanelEffectGrid->EnDisableSelectedModelWithRefresh();
            
            } else if (type == "MODEL_DISABLE") {
                
                PanelEffectGrid->EnDisableSelectedModelWithRefresh(1);

            }
            else if (type == "MODEL_ENABLE") {
                
                PanelEffectGrid->EnDisableSelectedModelWithRefresh(0);

            } 
            else if (type == "EFFECT_TOGGLE") {

                PanelEffectGrid->EnDisableRenderEffectsWithRefresh();                

            } 
            else if (type == "EFFECT_DISABLE") {

                PanelEffectGrid->EnDisableRenderEffectsWithRefresh(1);

            } 
            else if (type == "EFFECT_ENABLE") {

                PanelEffectGrid->EnDisableRenderEffectsWithRefresh(0);

            } 
            else if (type == "MODEL_EFFECT_TOGGLE") {

                PanelEffectGrid->EnDisableSelectedModelOrEffectsWithRefresh();

            }
            else {
                logger_base.warn("Keybinding '%s' not recognised.", (const char*)type.c_str());
                wxASSERT(false);
                return false;
            }
            event.StopPropagation();
            return true;
        }
    }

    return mSequenceElements->GetXLightsFrame()->HandleAllKeyBinding(event);
}

void MainSequencer::OnCharHook(wxKeyEvent& event)
{
    wxChar uc = event.GetKeyCode();

    if (mSequenceElements != nullptr && mSequenceElements->GetXLightsFrame() != nullptr && mSequenceElements->GetXLightsFrame()->IsACActive())
    {
        if (PanelEffectGrid->HandleACKey(uc, event.ShiftDown()))
        {
            event.StopPropagation();
            return;
        }
    }

    if (HandleSequencerKeyBinding(event)) return;

    //printf("OnCharHook %d   %c\n", uc, uc);
    switch(uc)
    {
        case WXK_BACK:
			PanelEffectGrid->DeleteSelectedEffects();
			event.StopPropagation();
			break;
		case WXK_INSERT:
		case WXK_NUMPAD_INSERT:
#ifdef __WXMSW__
			if (event.ControlDown())
			{
                if (mSequenceElements != nullptr) {
                    Copy();
                }
				event.StopPropagation();
			}
			else if (event.ShiftDown())
			{
                if (mSequenceElements != nullptr) {
                    Paste();
                }
				event.StopPropagation();
			}
#endif
			break;
		case WXK_DELETE:
#ifdef __WXMSW__
			if (!event.ShiftDown())
#endif
			{
				// Delete
                if (mSequenceElements != nullptr) {
                    PanelEffectGrid->DeleteSelectedEffects();
                }
				event.StopPropagation();
			}
#ifdef __WXMSW__
			else
			{
				// Cut - windows only
                if (mSequenceElements != nullptr) {
                    Cut();
                }
				event.StopPropagation();
			}
#endif
			break;
        case WXK_UP:
            PanelEffectGrid->MoveSelectedEffectUp(event.ShiftDown());
            event.StopPropagation();
            break;
        case WXK_DOWN:
            PanelEffectGrid->MoveSelectedEffectDown(event.ShiftDown());
            event.StopPropagation();
            break;
        case WXK_LEFT:
            PanelEffectGrid->MoveSelectedEffectLeft(event.ShiftDown(), event.ControlDown() || event.RawControlDown(), event.AltDown());
            event.StopPropagation();
            break;
        case WXK_RIGHT:
            PanelEffectGrid->MoveSelectedEffectRight(event.ShiftDown(), event.ControlDown() || event.RawControlDown(), event.AltDown());
            event.StopPropagation();
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case WXK_NUMPAD0:
        case WXK_NUMPAD1:
        case WXK_NUMPAD2:
        case WXK_NUMPAD3:
        case WXK_NUMPAD4:
        case WXK_NUMPAD5:
        case WXK_NUMPAD6:
        case WXK_NUMPAD7:
        case WXK_NUMPAD8:
        case WXK_NUMPAD9:
        {
            int number = wxAtoi(uc);

            if (number > 9) number -= WXK_NUMPAD0;

            if (event.ControlDown() || event.RawControlDown()) {
                if (event.ShiftDown()) {
                    PanelTimeLine->SetStartTimeMS(number * 10 * PanelTimeLine->GetTimeLength() / 100);
                    UpdateEffectGridHorizontalScrollBar();
                } else {
                    PanelTimeLine->GoToTag(number);
                }
            }
        }
            break;
        case WXK_PAGEUP:
            if (event.ControlDown() || event.RawControlDown()) {
                ScrollToRow(0);
            } else {
                ScrollToRow(std::max(0, mSequenceElements->GetFirstVisibleModelRow() - mSequenceElements->GetMaxModelsDisplayed()));
            }
            break;
        case WXK_PAGEDOWN:
            if (event.ControlDown() || event.RawControlDown()) {
                ScrollToRow(mSequenceElements->GetTotalNumberOfModelRows());
            } else {
                ScrollToRow(std::min(mSequenceElements->GetTotalNumberOfModelRows() - mSequenceElements->GetMaxModelsDisplayed(), mSequenceElements->GetFirstVisibleModelRow() + mSequenceElements->GetMaxModelsDisplayed()));
            }
            break;
        case WXK_ESCAPE:
            CancelRender();
            break;
        default:
            event.Skip();
            break;
    }
}

void MainSequencer::CancelRender()
{
    static bool escapeReenter = false;

    if (!escapeReenter) {
        escapeReenter = true;
        if (mSequenceElements != nullptr && mSequenceElements->GetXLightsFrame() != nullptr) {
            mSequenceElements->GetXLightsFrame()->AbortRender();
        }
        escapeReenter = false;
    }
}

void MainSequencer::ToggleRender(bool off)
{
    if (mSequenceElements == nullptr) return;
    if (off) CancelRender();
    mSequenceElements->GetXLightsFrame()->SuspendRender(off);
}

void MainSequencer::OnKeyDown(wxKeyEvent& event)
{
    //wxChar uc = event.GetUnicodeKey();
    //printf("OnKeyDown %d   %c\n", uc, uc);
    event.Skip();
}

void MainSequencer::OnChar(wxKeyEvent& event)
{
    wxChar uc = event.GetUnicodeKey();

    if (mSequenceElements != nullptr && mSequenceElements->GetXLightsFrame() != nullptr && mSequenceElements->GetXLightsFrame()->IsACActive())
    {
        if (PanelEffectGrid->HandleACKey(uc))
        {
            event.StopPropagation();
            return;
        }
    }

    if (HandleSequencerKeyBinding(event)) return;

    //printf("OnChar %d   %c\n", uc, uc);
    switch(uc)
    {
        case 'c':
        case 'C':
        case WXK_CONTROL_C:
            if (event.CmdDown() || event.ControlDown()) {
                if (mSequenceElements != nullptr) {
                    Copy();
                    event.StopPropagation();
                }
            }
            break;
        case 'x':
        case 'X':
        case WXK_CONTROL_X:
            if (event.CmdDown() || event.ControlDown()) {
                if (mSequenceElements != nullptr) {
                    Cut();
                    event.StopPropagation();
                }
            }
            break;
        case 'v':
        case 'V':
        case WXK_CONTROL_V:
            if (event.CmdDown() || event.ControlDown()) {
                Paste();
                event.StopPropagation();
            }
            break;
        case 'z':
        case 'Z':
        case WXK_CONTROL_Z:
            if (event.CmdDown() || event.ControlDown()) {
                if( mSequenceElements != nullptr &&
                   mSequenceElements->get_undo_mgr().CanUndo() ) {
                    mSequenceElements->get_undo_mgr().UndoLastStep();
                    PanelEffectGrid->ClearSelection();
                    PanelEffectGrid->Draw();
                    PanelEffectGrid->sendRenderDirtyEvent();
                }
                event.StopPropagation();
            }
            break;
        case 'y':
        case 'Y':
        case WXK_CONTROL_Y:
            if (event.CmdDown() || event.ControlDown()) {
                if( mSequenceElements != nullptr &&
                   mSequenceElements->get_undo_mgr().CanRedo() ) {
                    mSequenceElements->get_undo_mgr().RedoLastStep();
                    PanelEffectGrid->ClearSelection();
                    PanelEffectGrid->Draw();
                    PanelEffectGrid->sendRenderDirtyEvent();
                }
                event.StopPropagation();
            }
            break;
        case WXK_ESCAPE: {
            static bool escapeReenter = false;

            if (!escapeReenter) {
                escapeReenter = true;
                if (mSequenceElements != nullptr && mSequenceElements->GetXLightsFrame() != nullptr) {
                    mSequenceElements->GetXLightsFrame()->AbortRender();
                }
                escapeReenter = false;
            }
        }
        break;
    }
}

void MainSequencer::ToggleHousePreview() {
    if (mSequenceElements != nullptr && mSequenceElements->GetXLightsFrame() != nullptr) {
        wxCommandEvent event;
        mSequenceElements->GetXLightsFrame()->ShowHideHousePreview(event);
    }
}

void MainSequencer::ToggleModelPreview() {
    if (mSequenceElements != nullptr && mSequenceElements->GetXLightsFrame() != nullptr) {
        wxCommandEvent event;
        mSequenceElements->GetXLightsFrame()->ShowHideModelPreview(event);
    }
}

void MainSequencer::TouchPlayControl(const std::string &evt) {
    wxCommandEvent e;
    if (evt == "Play") {
        mSequenceElements->GetXLightsFrame()->OnAuiToolBarItemPlayButtonClick(e);
    } else if (evt == "Pause") {
        mSequenceElements->GetXLightsFrame()->OnAuiToolBarItemPauseButtonClick(e);
    } else if (evt == "Stop") {
        mSequenceElements->GetXLightsFrame()->OnAuiToolBarItemStopClick(e);
    } else if (evt == "Back") {
        mSequenceElements->GetXLightsFrame()->OnAuiToolBarFirstFrameClick(e);
    } else if (evt == "Forward") {
        mSequenceElements->GetXLightsFrame()->OnAuiToolBarLastFrameClick(e);
    }
}

void MainSequencer::TouchButtonEvent(wxCommandEvent &event) {
    if (mSequenceElements != nullptr) {
        wxString effect = ((wxWindow*)event.GetEventObject())->GetName();
        Effect* ef = PanelEffectGrid->Paste(effect + "\t\t\n", xlights_version_string);
        SelectEffect(ef);
    }
}

#ifdef __XLIGHTS_HAS_TOUCHBARS__
void MainSequencer::SetupTouchBar(EffectManager &effectManager, ColorPanelTouchBar *colorBar) {
    if (effectGridTouchbar == nullptr && touchBarSupport.HasTouchBar()) {
        std::vector<TouchBarItem*> items;
        std::vector<ButtonTouchBarItem*> pvitems;
        
        pvitems.push_back(new ButtonTouchBarItem([this]() {
            ToggleHousePreview();
        },
                                                 "Toggle House Preview",
                                                 wxArtProvider::GetBitmapBundle("xlART_HOUSE_PREVIEW", wxART_TOOLBAR)));
        pvitems.push_back(new ButtonTouchBarItem([this]() {
            ToggleModelPreview();
        },
                                                 "Toggle Model Preview",
                                                 wxArtProvider::GetBitmapBundle("xlART_MODEL_PREVIEW", wxART_TOOLBAR)));
        items.push_back(new GroupTouchBarItem("Previews", pvitems));
        
        
        std::vector<ButtonTouchBarItem*> pbitems;
        
        pbitems.push_back(new ButtonTouchBarItem([this]() { TouchPlayControl("Play"); },
                                                 "Play",
                                                 wxArtProvider::GetBitmapBundle("xlART_PLAY", wxART_TOOLBAR)));
        pbitems.push_back(new ButtonTouchBarItem([this]() { TouchPlayControl("Pause"); },
                                                 "Pause",
                                                 wxArtProvider::GetBitmapBundle("xlART_PAUSE", wxART_TOOLBAR)));
        pbitems.push_back(new ButtonTouchBarItem([this]() { TouchPlayControl("Stop"); },
                                                 "Stop",
                                                 wxArtProvider::GetBitmapBundle("xlART_STOP", wxART_TOOLBAR)));
        pbitems.push_back(new ButtonTouchBarItem([this]() { TouchPlayControl("Back"); },
                                                 "Backward",
                                                 wxArtProvider::GetBitmapBundle("xlART_BACKWARD", wxART_TOOLBAR)));
        pbitems.push_back(new ButtonTouchBarItem([this]() { TouchPlayControl("Forward"); },
                                                 "Forward",
                                                 wxArtProvider::GetBitmapBundle("xlART_FORWARD", wxART_TOOLBAR)));
        
        items.push_back(new GroupTouchBarItem("Playback Controls", pbitems));
        
        std::vector<ButtonTouchBarItem*> zitems;
        
        zitems.push_back(new ButtonTouchBarItem([this]() {
            PanelTimeLine->ZoomIn();
        },
                                                 "Zoom In",
                                                 wxArtProvider::GetBitmapBundle("xlART_ZOOM_IN", wxART_TOOLBAR)));
        zitems.push_back(new ButtonTouchBarItem([this]() {
            PanelTimeLine->ZoomOut();
        },
                                                 "Zoom Out",
                                                 wxArtProvider::GetBitmapBundle("xlART_ZOOM_OUT", wxART_TOOLBAR)));
        items.push_back(new GroupTouchBarItem("Zoom", zitems));
        

        
        ButtonTouchBarItem *colorsButton = new ButtonTouchBarItem([colorBar]() {
            colorBar->SetActive();
        }, "NSTouchBarColorPickerFill", "Colors");
        items.push_back(colorsButton);
        
        for (auto it = effectManager.begin(); it != effectManager.end(); ++it) {
            wxButton *b = new wxButton(touchBarSupport.GetControlParent(), wxID_ANY,
                                       "",
                                       wxDefaultPosition,
                                       wxDefaultSize,
                                       0,
                                       wxDefaultValidator,
                                       (*it)->Name());
            b->SetBitmap((*it)->GetEffectIcon());
            b->Connect(wxEVT_BUTTON, (wxObjectEventFunction)&MainSequencer::TouchButtonEvent, nullptr, this);
            
            items.push_back(new wxControlTouchBarItem(b));
        }
        
        
        effectGridTouchbar = std::unique_ptr<EffectGridTouchBar>(new EffectGridTouchBar(touchBarSupport, items));
        effectGridTouchbar->SetActive();
    }
}
#endif

void MainSequencer::DoCopy(wxCommandEvent& event) {
    if (mSequenceElements != nullptr) {
        Copy();
    }
}
void MainSequencer::DoCut(wxCommandEvent& event) {
    if (mSequenceElements != nullptr) {
        Cut();
    }
}
void MainSequencer::DoPaste(wxCommandEvent& event) {
    if (mSequenceElements != nullptr) {
        Paste();
    }
}

void MainSequencer::DoUndo(wxCommandEvent& event) {

    if (PanelEffectGrid == nullptr) return;

    if (mSequenceElements != nullptr && mSequenceElements->get_undo_mgr().CanUndo() ) {
        mSequenceElements->get_undo_mgr().UndoLastStep();
        PanelEffectGrid->ClearSelection();
        PanelEffectGrid->Draw();
        PanelEffectGrid->sendRenderDirtyEvent();
    }
}

void MainSequencer::DoRedo(wxCommandEvent& event) {
    if (PanelEffectGrid == nullptr) return;

    if (mSequenceElements != nullptr && mSequenceElements->get_undo_mgr().CanRedo() ) {
        mSequenceElements->get_undo_mgr().RedoLastStep();
        PanelEffectGrid->ClearSelection();
        PanelEffectGrid->Draw();
        PanelEffectGrid->sendRenderDirtyEvent();
    }
}

void MainSequencer::SetLargeWaveform()
{
    PanelWaveForm->SetWaveFormSize(Waveform::GetLargeSize());
    timeDisplay->SetGLSize(-1, Waveform::GetLargeSize() - 22);
    Layout();
    PanelWaveForm->Refresh();
    timeDisplay->Refresh();
}

void MainSequencer::SetSmallWaveform()
{
    PanelWaveForm->SetWaveFormSize(Waveform::GetSmallSize());
    timeDisplay->SetGLSize(-1, Waveform::GetSmallSize() - 22);
    Layout();
    PanelWaveForm->Refresh();
    timeDisplay->Refresh();
}

void MainSequencer::GetPresetData(wxString& copy_data)
{
    if (PanelEffectGrid->IsACActive()) {
        GetACEffectsData(copy_data);
    } else {
        GetSelectedEffectsData(copy_data);
    }
}

void MainSequencer::GetSelectedEffectsData(wxString& copy_data) {
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int start_column = PanelEffectGrid->GetStartColumn();
    int column_start_time = -1000;
    int number_of_timings = 0;
    int number_of_effects = 0;
    int number_of_timing_rows = mSequenceElements->GetNumberOfTimingRows();
    int first_timing_row = -1;
    int last_timing_row = 0;
    wxString effect_data;
    EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
    if( tel != nullptr && start_column != -1) {
        Effect* eff = tel->GetEffect(start_column);
        if( eff != nullptr ) {
            column_start_time = eff->GetStartTimeMS();
        }
    }

    for(int i=0;i<mSequenceElements->GetRowInformationSize();i++)
    {
        int row_number;
        if( i < number_of_timing_rows )
        {
            row_number = mSequenceElements->GetRowInformation(i)->Index;
        }
        else
        {
            row_number = mSequenceElements->GetRowInformation(i)->Index-mSequenceElements->GetFirstVisibleModelRow();
        }
        EffectLayer* el = mSequenceElements->GetEffectLayer(i);
        for (int x = 0; x < el->GetEffectCount(); x++) {
            Effect *ef = el->GetEffect(x);
            if( ef == nullptr ) break;
            if (ef->GetSelected() != EFFECT_NOT_SELECTED && !ef->GetTagged()) {
                ef->SetTagged(true);
                wxString start_time = wxString::Format("%d",ef->GetStartTimeMS());
                wxString end_time = wxString::Format("%d",ef->GetEndTimeMS());
                wxString row = wxString::Format("%d",row_number);
                wxString column_start = wxString::Format("%d",column_start_time);
                effect_data += ef->GetEffectName() + "\t" + ef->GetSettingsAsString() + "\t" + ef->GetPaletteAsString() +
                               "\t" + start_time + "\t" + end_time + "\t" + row + "\t" + column_start;
                if( i < number_of_timing_rows )
                {
                    number_of_timings++;
                    last_timing_row = row_number;
                    if( first_timing_row < 0 )
                    {
                        first_timing_row = row_number;
                    }
                    effect_data += "\tTIMING_EFFECT\n";
                }
                else
                {
                    number_of_effects++;
                    if( column_start_time == -1000 && mSequenceElements->GetSelectedTimingRow() >= 0 )
                    {
                        if (tel == nullptr)
                        {
                            logger_base.crit("MainSequencer::GetSelectedEffectsData tel is nullptr ... this is going to crash.");
                        }

                        if( tel->HitTestEffectByTime(ef->GetStartTimeMS()+1,start_column) )
                        {
                            column_start_time = tel->GetEffect(start_column)->GetStartTimeMS();
                        }
                    }
                    if( column_start_time != -1000 )  // add paste by cell info
                    {
                        if( mSequenceElements->GetSelectedTimingRow() >= 0 )
                        {
                            Effect* te_start = tel->GetEffectByTime(ef->GetStartTimeMS() + 1); // if we don't add 1ms, it picks up the end of the previous timing instead of the start of this one
                            Effect* te_end = tel->GetEffectByTime(ef->GetEndTimeMS());
                            if( te_start != nullptr && te_end != nullptr )
                            {
                                if (tel == nullptr)
                                {
                                    logger_base.crit("MainSequencer::GetSelectedEffectsData tel is nullptr ... this is going to crash.");
                                }

                                if (te_start->GetEndTimeMS() == te_start->GetStartTimeMS()) {
                                    logger_base.crit("MainSequencer::GetSelectedEffectsData start effect start and end time is the same ... this is going to crash.");
                                }
                                int start_pct = ((ef->GetStartTimeMS() - te_start->GetStartTimeMS()) * 100) / (te_start->GetEndTimeMS() - te_start->GetStartTimeMS());

                                if (te_end->GetEndTimeMS() == te_end->GetStartTimeMS()) {
                                    logger_base.crit("MainSequencer::GetSelectedEffectsData end effect start and end time is the same ... this is going to crash.");
                                }
                                int end_pct = ((ef->GetEndTimeMS() - te_end->GetStartTimeMS()) * 100) / (te_end->GetEndTimeMS() - te_end->GetStartTimeMS());
                                int start_index;
                                int end_index;
                                tel->HitTestEffectByTime(te_start->GetStartTimeMS()+1,start_index);
                                tel->HitTestEffectByTime(te_end->GetStartTimeMS()+1,end_index);
                                wxString start_pct_str = wxString::Format("%d",start_pct);
                                wxString end_pct_str = wxString::Format("%d",end_pct);
                                wxString start_index_str = wxString::Format("%d",start_index);
                                wxString end_index_str = wxString::Format("%d",end_index);
                                effect_data += "\t" + start_index_str + "\t" + end_index_str + "\t" + start_pct_str + "\t" + end_pct_str + "\n";
                            } else {effect_data += "\tNO_PASTE_BY_CELL\n";}
                        } else {effect_data += "\tNO_PASTE_BY_CELL\n";}
                    } else {effect_data += "\tNO_PASTE_BY_CELL\n";}
                }
            }
        }
    }
    if( first_timing_row >= 0 )
    {
        last_timing_row -= first_timing_row;  // calculate the total number of timing rows
    }

    wxString num_timings = wxString::Format("%d",number_of_timings);
    wxString num_effects = wxString::Format("%d",number_of_effects);
    wxString num_timing_rows = wxString::Format("%d",number_of_timing_rows);
    wxString last_row = wxString::Format("%d",last_timing_row);
    wxString starting_column = wxString::Format("%d",start_column);
    copy_data = "CopyFormat1\t" + num_timings + "\t" + num_effects + "\t" + num_timing_rows + "\t" + last_row + "\t" + starting_column;
    if( column_start_time != -1000 ) {
        copy_data += "\tPASTE_BY_CELL\n" + effect_data;
    }
    else {
        copy_data += "\tNO_PASTE_BY_CELL\n" + effect_data;
    }
    UnTagAllEffects();
}

void MainSequencer::GetACEffectsData(wxString& copy_data) {
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int start_column = PanelEffectGrid->GetStartColumn();
    int end_column = PanelEffectGrid->GetEndColumn();
    int start_row = PanelEffectGrid->GetStartRow();
    int end_row = PanelEffectGrid->GetEndRow();
    int column_start_time = -1000;
    int column_end_time = -1000;
    int number_of_timings = 0;
    int number_of_effects = 0;
    int number_of_timing_rows = mSequenceElements->GetNumberOfTimingRows();
    int last_timing_row = 0;
    wxString effect_data;
    EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
    if (tel != nullptr && start_column != -1) {
        Effect* eff1 = tel->GetEffect(start_column);
        Effect* eff2 = tel->GetEffect(end_column);
        if (eff1 != nullptr && eff2 != nullptr) {
            column_start_time = eff1->GetStartTimeMS();
            column_end_time = eff2->GetEndTimeMS();
        }
        else {
            return;
        }
    }
    else {
        return;  // there should always be a range selection in AC copy mode
    }

    for (int i = 0; i < mSequenceElements->GetRowInformationSize(); i++)
    {
        int row_number;
        if (i < number_of_timing_rows)
        {
            row_number = mSequenceElements->GetRowInformation(i)->Index;
        }
        else
        {
            row_number = mSequenceElements->GetRowInformation(i)->Index;// -mSequenceElements->GetFirstVisibleModelRow();
        }
        if (row_number >= start_row && row_number <= end_row)
        {
            EffectLayer* el = mSequenceElements->GetEffectLayer(i);
            for (int x = 0; x < el->GetEffectCount(); x++) {
                Effect *ef = el->GetEffect(x);
                if (ef == nullptr) break;
                if (!ef->GetTagged()) {
                    ef->SetTagged(true);
                    if (ef->GetStartTimeMS() < column_end_time && ef->GetEndTimeMS() > column_start_time) {
                        int start_offset = 0;
                        int end_offset = 0;
                        if (ef->GetStartTimeMS() < column_start_time) {
                            start_offset = column_start_time - ef->GetStartTimeMS();
                        }
                        if (ef->GetEndTimeMS() > column_end_time) {
                            end_offset = ef->GetEndTimeMS() - column_end_time;
                        }
                        int adj_start_time = ef->GetStartTimeMS() + start_offset;
                        int adj_end_time = ef->GetEndTimeMS() - end_offset;
                        wxString start_time = wxString::Format("%d", adj_start_time);
                        wxString end_time = wxString::Format("%d", adj_end_time);
                        wxString row = wxString::Format("%d", row_number);
                        wxString column_start = wxString::Format("%d", column_start_time);
                        std::string settings = PanelEffectGrid->TruncateEffectSettings(ef->GetSettings(), ef->GetEffectName(), ef->GetStartTimeMS(), ef->GetEndTimeMS(), adj_start_time, adj_end_time);

                        effect_data += ef->GetEffectName() + "\t" + settings + "\t" + ef->GetPaletteAsString() +
                            "\t" + start_time + "\t" + end_time + "\t" + row + "\t" + column_start;
                        number_of_effects++;
                        Effect* te_start = tel->GetEffectByTime(adj_start_time + 1); // if we don't add 1ms, it picks up the end of the previous timing instead of the start of this one
                        Effect* te_end = tel->GetEffectByTime(adj_end_time);
                        if (te_start != nullptr && te_end != nullptr)
                        {
                            if (tel == nullptr)
                            {
                                logger_base.crit("MainSequencer::GetSelectedEffectsData tel is nullptr ... this is going to crash.");
                            }

                            int start_pct = ((adj_start_time - te_start->GetStartTimeMS()) * 100) / (te_start->GetEndTimeMS() - te_start->GetStartTimeMS());
                            int end_pct = ((adj_end_time - te_end->GetStartTimeMS()) * 100) / (te_end->GetEndTimeMS() - te_end->GetStartTimeMS());
                            int start_index;
                            int end_index;
                            tel->HitTestEffectByTime(te_start->GetStartTimeMS() + 1, start_index);
                            tel->HitTestEffectByTime(te_end->GetStartTimeMS() + 1, end_index);
                            wxString start_pct_str = wxString::Format("%d", start_pct);
                            wxString end_pct_str = wxString::Format("%d", end_pct);
                            wxString start_index_str = wxString::Format("%d", start_index);
                            wxString end_index_str = wxString::Format("%d", end_index);
                            effect_data += "\t" + start_index_str + "\t" + end_index_str + "\t" + start_pct_str + "\t" + end_pct_str + "\n";
                        }
                    }
                }
            }
        }
    }
    wxString num_timings = wxString::Format("%d", number_of_timings);
    wxString num_effects = wxString::Format("%d", number_of_effects);
    wxString num_timing_rows = wxString::Format("%d", number_of_timing_rows);
    wxString last_row = wxString::Format("%d", last_timing_row);
    wxString starting_column = wxString::Format("%d", start_column);
    wxString ending_column = wxString::Format("%d", end_column);
    wxString starting_row = wxString::Format("%d", start_row);
    wxString ending_row = wxString::Format("%d", end_row);
    wxString starting_time = wxString::Format("%d", column_start_time);
    wxString ending_time = wxString::Format("%d", column_end_time);
    copy_data = "CopyFormatAC\t" + num_timings + "\t" + num_effects + "\t" + num_timing_rows + "\t" + last_row + "\t" + starting_column + "\t" + ending_column + "\t" + starting_row + "\t" + ending_row + "\t" + starting_time + "\t" + ending_time;
    copy_data += "\tPASTE_BY_CELL\n" + effect_data;
    UnTagAllEffects();
}

bool MainSequencer::CopySelectedEffects() {
    wxString copy_data;
    if (PanelEffectGrid->IsACActive()) {
        GetACEffectsData(copy_data);
    }
    else {
        GetSelectedEffectsData(copy_data);
    }
    if (!copy_data.IsEmpty() && wxTheClipboard != nullptr && wxTheClipboard->Open()) {
        if (!wxTheClipboard->SetData(new wxTextDataObject(copy_data))) {
            DisplayError("Unable to copy data to clipboard.", this);
        }
        wxTheClipboard->Close();
        return true;
    }
    return false;
}

void MainSequencer::Copy() {
    CopySelectedEffects();
}

void MainSequencer::Cut() {
    if (CopySelectedEffects()) {
        if (PanelEffectGrid->IsACActive())
        {
            PanelEffectGrid->DoACDraw(false, ACTYPE::OFF, ACSTYLE::INTENSITY, ACTOOL::TOOLNIL, ACMODE::MODENIL);
        }
        else
        {
            PanelEffectGrid->DeleteSelectedEffects();
        }
    }
}

Effect* MainSequencer::GetSelectedEffect()
{
    return PanelEffectGrid->GetSelectedEffect();
}

int MainSequencer::GetSelectedEffectCount(const std::string effectName) const
{
    return PanelEffectGrid->GetSelectedEffectCount(effectName);
}

bool MainSequencer::AreAllSelectedEffectsOnTheSameElement() const
{
    return PanelEffectGrid->AreAllSelectedEffectsOnTheSameElement();
}

void MainSequencer::ApplyEffectSettingToSelected(const std::string& effectName, const std::string id, const std::string value, ValueCurve* vc, const std::string& vcid)
{
    return PanelEffectGrid->ApplyEffectSettingToSelected(effectName, id, value, vc, vcid);
}

void MainSequencer::ApplyButtonPressToSelected(const std::string& effectName, const std::string id)
{
    return PanelEffectGrid->ApplyButtonPressToSelected(effectName, id);
}

void MainSequencer::RemapSelectedDMXEffectValues(const std::vector<std::tuple<int, int, float, int>>& dmxmappings)
{
    return PanelEffectGrid->RemapSelectedDMXEffectValues(dmxmappings);
}

void MainSequencer::ConvertSelectedEffectsTo(const std::string& effectName)
{
    return PanelEffectGrid->ConvertSelectedEffectsTo(effectName);
}

void MainSequencer::UnselectAllEffects()
{
    mSequenceElements->UnSelectAllEffects();
}

void MainSequencer::SelectEffect(Effect* ef)
{
    if (!mSequenceElements->GetXLightsFrame()->IsACActive())
    {
        PanelEffectGrid->SelectEffect(ef);
    }
}

Effect* MainSequencer::SelectEffectUsingDescription(std::string description)
{
    mSequenceElements->UnSelectAllEffects();
    return mSequenceElements->SelectEffectUsingDescription(description);
}

Effect* MainSequencer::SelectEffectUsingElementLayerTime(std::string element, int layer, int time)
{
    mSequenceElements->UnSelectAllEffects();
    return mSequenceElements->SelectEffectUsingElementLayerTime(element, layer, time);
}

void MainSequencer::SetChanged()
{
    mSequenceElements->IncrementChangeCount(nullptr);
}

std::list<std::string> MainSequencer::GetAllElementNamesWithEffects()
{
    return mSequenceElements->GetAllElementNamesWithEffects();
}

int MainSequencer::GetElementLayerCount(std::string elementName, std::list<int>* layers)
{
    return mSequenceElements->GetElementLayerCount(elementName, layers);
}

std::list<Effect*> MainSequencer::GetElementLayerEffects(std::string elementName, int layer)
{
    return mSequenceElements->GetElementLayerEffects(elementName, layer);
}

std::list<std::string> MainSequencer::GetUniqueEffectPropertyValues(const std::string& id)
{
    return mSequenceElements->GetUniqueEffectPropertyValues(id);
}

std::list<std::string> MainSequencer::GetAllEffectDescriptions()
{
    return mSequenceElements->GetAllEffectDescriptions();
}

void MainSequencer::Paste(bool row_paste) {
    wxTextDataObject data;
    wxClipboard *cbd = wxClipboard::Get();
    if (cbd && cbd->Open()) {
        if ((cbd->IsSupported(wxDF_TEXT) || cbd->IsSupported(wxDF_UNICODETEXT))
            && cbd->GetData(data)) {
            //assume clipboard always has data from same version of xLights
            Effect* ef = PanelEffectGrid->Paste(data.GetText(), xlights_version_string, row_paste);
            SelectEffect(ef);
        }
        cbd->Close();
    }
}

void MainSequencer::InsertTimingMarkFromRange()
{
    bool is_range = true;
    int x1;
    int x2;
    if (mSequenceElements->GetXLightsFrame()->GetPlayStatus() == PLAY_TYPE_MODEL) {
        x1 = PanelTimeLine->GetPlayMarker();
        x2 = x1;
    }
    else {
        x1 = PanelTimeLine->GetSelectedPositionStart();
        x2 = PanelTimeLine->GetSelectedPositionEnd();

        int pm = PanelTimeLine->GetPlayMarker();
        if ((x1 == -1 || x2 == -1 || x1 == x2) && pm != -1) {
            x1 = pm;
            x2 = x1;
        }
    }
    if (x2 == -1) x2 = x1;
    if (x1 == x2) is_range = false;

    int selectedTiming = mSequenceElements->GetSelectedTimingRow();
    if (selectedTiming >= 0) {
        int t1 = PanelTimeLine->GetAbsoluteTimeMSfromPosition(x1);
        int t2 = PanelTimeLine->GetAbsoluteTimeMSfromPosition(x2);
        if (t2 > PanelTimeLine->GetTimeLength()) {
            t2 = PanelTimeLine->GetTimeLength();
        }
        if (is_range) {
            Element* e = mSequenceElements->GetVisibleRowInformation(selectedTiming)->element;
            if (e != nullptr) {
                EffectLayer* el = e->GetEffectLayer(mSequenceElements->GetVisibleRowInformation(selectedTiming)->layerIndex);
                if (el != nullptr) {
                    int i1 = -1;
                    int i2 = -1;
                    el->HitTestEffectByTime(t1, i1);
                    el->HitTestEffectByTime(t2, i2);

                    if ((!el->HitTestEffectByTime(t1, i1) && !el->HitTestEffectByTime(t2, i2) && !el->HitTestEffectBetweenTime(t1, t2)) ||
                        (!el->HitTestEffectBetweenTime(t1, t2) && i1 != i2)) {
                        std::string name, settings;
                        el->AddEffect(0, name, settings, "", t1, t2, false, false);
                        PanelEffectGrid->ForceRefresh();
                    }
                    else
                    {
                        DisplayError("Timing placement error: Timing exists already in the selected region", this);
                    }
                }
            }
        }
        else
        {
            // x1 and x2 are the same. Insert from end time of timing to the left to x2
            Element* e = mSequenceElements->GetVisibleRowInformation(selectedTiming)->element;
            if (e != nullptr) {
                EffectLayer* el = e->GetEffectLayer(mSequenceElements->GetVisibleRowInformation(selectedTiming)->layerIndex);
                if (el != nullptr) {
                    int index = 0;
                    if (!el->HitTestEffectByTime(t2, index)) {
                        // get effect to left and right
                        Effect* lefteffect = nullptr;
                        Effect* righteffect = nullptr;
                        for (int x = 0; x < el->GetEffectCount(); x++) {
                            Effect* eff = el->GetEffect(x);
                            if (eff->GetEndTimeMS() < t2 && (lefteffect == nullptr || lefteffect->GetEndTimeMS() < eff->GetEndTimeMS())) {
                                lefteffect = eff;
                            }
                            if (righteffect == nullptr && eff->GetStartTimeMS() > t2) {
                                righteffect = eff;
                                break;
                            }
                        }

                        std::string name;
                        std::string settings;
                        if (lefteffect != nullptr && righteffect != nullptr) {
                            // fill to left and right
                            el->AddEffect(0, name, settings, "", lefteffect->GetEndTimeMS(), t2, false, false);
                            el->AddEffect(0, name, settings, "", t2, righteffect->GetStartTimeMS(), false, false);
                        }
                        else if (lefteffect != nullptr) {
                            el->AddEffect(0, name, settings, "", lefteffect->GetEndTimeMS(), t2, false, false);
                        }
                        else if (righteffect != nullptr) {
                            el->AddEffect(0, name, settings, "", t2, righteffect->GetStartTimeMS(), false, false);
                        }
                        else {
                            el->AddEffect(0, name, settings, "", 0, t2, false, false);
                        }

                        PanelEffectGrid->ForceRefresh();
                    }
                    else {
                        SplitTimingMark();  // inserting a timing mark inside a timing mark same as a split
                    }
                }
            }
        }
    }
}

void MainSequencer::SplitTimingMark()
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int x1;
    int x2;
    if (mSequenceElements->GetXLightsFrame()->GetPlayStatus() == PLAY_TYPE_MODEL)
    {
        x1 = PanelTimeLine->GetPlayMarker();
        x2 = x1;
    }
    else
    {
        x1 = PanelTimeLine->GetSelectedPositionStart();
        x2 = PanelTimeLine->GetSelectedPositionEnd();

        int pm = PanelTimeLine->GetPlayMarker();
        if ((x1 == -1 || x2 == -1 || x1 == x2) && pm != -1)
        {
            x1 = pm;
            x2 = x1;
        }
    }
    if (x2 == -1) x2 = x1;
    int selectedTiming = mSequenceElements->GetSelectedTimingRow();
    if (selectedTiming >= 0)
    {
        Element* e = mSequenceElements->GetVisibleRowInformation(selectedTiming)->element;
        EffectLayer* el = e->GetEffectLayer(mSequenceElements->GetVisibleRowInformation(selectedTiming)->layerIndex);

        if (el == nullptr)
        {
            logger_base.crit("MainSequencer::SplitTimingMark el is nullptr ... this is going to crash.");
        }

        int index1, index2;
        int t1 = PanelTimeLine->GetAbsoluteTimeMSfromPosition(x1);
        int t2 = PanelTimeLine->GetAbsoluteTimeMSfromPosition(x2);
        if (el->HitTestEffectByTime(t1, index1) && el->HitTestEffectByTime(t2, index2))
        {
            if (index1 == index2)
            {
                Effect* eff1 = el->GetEffect(index1);
                int old_end_time = eff1->GetEndTimeMS();
                if (t1 != t2 || ((t1 == t2) && t1 != eff1->GetStartTimeMS() && t1 != eff1->GetEndTimeMS()))
                {
                    eff1->SetEndTimeMS(t1);
                    std::string name, settings;
                    el->AddEffect(0, name, settings, "", t2, old_end_time, false, false);
                    PanelEffectGrid->ForceRefresh();
                }
            }
            else
            {
                DisplayError("Timing placement error: Timing cannot be split across timing marks.");
            }
        }
    }
}

void MainSequencer::OnScrollBarEffectsHorizontalScrollLineUp(wxScrollEvent& event)
{
    int position = ScrollBarEffectsHorizontal->GetThumbPosition();
    if( position > 0 )
    {
        int ts = ScrollBarEffectsHorizontal->GetThumbSize() / 10;
        if (ts == 0) {
            ts = 1;
        }
        position -= ts;
        if (position < 0) {
            position = 0;
        }
        ScrollBarEffectsHorizontal->SetThumbPosition(position);
    }
}

void MainSequencer::OnScrollBarEffectsHorizontalScrollLineDown(wxScrollEvent& event)
{
    int position = ScrollBarEffectsHorizontal->GetThumbPosition();
    int limit = ScrollBarEffectsHorizontal->GetRange();
    if( position < limit-1 )
    {
        int ts = ScrollBarEffectsHorizontal->GetThumbSize() / 10;
        if (ts == 0) {
            ts = 1;
        }
        position += ts;
        if (position >= limit) {
            position = limit - 1;
        }
        ScrollBarEffectsHorizontal->SetThumbPosition(position);
    }
}


void MainSequencer::HorizontalScrollChanged( wxCommandEvent& event)
{
    int position = ScrollBarEffectsHorizontal->IsEnabled() ? ScrollBarEffectsHorizontal->GetThumbPosition() : 0;
    int timeLength = PanelTimeLine->GetTimeLength();
    int startTime = (int)(((double)position/(double)timeLength) * (double)timeLength);
    PanelTimeLine->SetStartTimeMS(startTime);
    UpdateEffectGridHorizontalScrollBar();
}

void MainSequencer::ScrollRight(wxCommandEvent& event)
{
    int position = ScrollBarEffectsHorizontal->GetThumbPosition();
    int limit = ScrollBarEffectsHorizontal->GetRange();
    if( position < limit-1 )
    {
        int ts = ScrollBarEffectsHorizontal->GetThumbSize();
        if (ts == 0) {
            ts = 1;
        }
        position += ts;
        if (position >= limit) {
            position = limit - 1;
        }
        ScrollBarEffectsHorizontal->SetThumbPosition(position);
        wxCommandEvent eventScroll(EVT_HORIZ_SCROLL);
        HorizontalScrollChanged(eventScroll);
    }
}

void MainSequencer::TimeLineSelectionChanged(wxCommandEvent& event)
{
    UpdateSelectedDisplay(event.GetInt());
}

void MainSequencer::SequenceChanged(wxCommandEvent& event)
{
    mSequenceElements->IncrementChangeCount(nullptr);
}

void MainSequencer::TimelineChanged( wxCommandEvent& event)
{
    TimelineChangeArguments *tla = (TimelineChangeArguments*)(event.GetClientData());
    PanelWaveForm->SetZoomLevel(tla->ZoomLevel);
    PanelWaveForm->SetStartPixelOffset(tla->StartPixelOffset);
    UpdateTimeDisplay(tla->CurrentTimeMS, -1);
    PanelTimeLine->Refresh();
    PanelTimeLine->Update();
    PanelWaveForm->render();
    PanelEffectGrid->SetStartPixelOffset(tla->StartPixelOffset);
    PanelEffectGrid->Draw();
    UpdateEffectGridHorizontalScrollBar();
    delete tla;
}

void MainSequencer::UpdateEffectGridHorizontalScrollBar()
{
    PanelWaveForm->SetZoomLevel(PanelTimeLine->GetZoomLevel());
    PanelWaveForm->SetStartPixelOffset(PanelTimeLine->GetStartPixelOffset());
    UpdateTimeDisplay(PanelTimeLine->GetCurrentPlayMarkerMS(), -1);

    //printf("%d\n", PanelTimeLine->GetStartPixelOffset());
    PanelTimeLine->Refresh();
    PanelTimeLine->Update();
    PanelWaveForm->render();
    PanelEffectGrid->SetStartPixelOffset(PanelTimeLine->GetStartPixelOffset());
    PanelEffectGrid->Draw();

    int zoomLevel = PanelTimeLine->GetZoomLevel();
    int maxZoomLevel = PanelTimeLine->GetMaxZoomLevel(); 
    if (zoomLevel >= maxZoomLevel) {
        // Max Zoom so scrollbar is same size as window.
        int range = PanelTimeLine->GetSize().x;
        int pageSize =range;
        int thumbSize = range;
        ScrollBarEffectsHorizontal->SetScrollbar(0,thumbSize,range,pageSize);
        ScrollBarEffectsHorizontal->Disable();
    } else {
        int startTime;
        int endTime;
        int range = PanelTimeLine->GetTimeLength();
        PanelTimeLine->GetViewableTimeRange(startTime,endTime);

        int diff = endTime - startTime;
        int thumbSize = diff;
        int pageSize = thumbSize;
        int position = startTime;
        ScrollBarEffectsHorizontal->SetScrollbar(position,thumbSize,range,pageSize);
        ScrollBarEffectsHorizontal->Enable();
    }
    ScrollBarEffectsHorizontal->Refresh();
}

void MainSequencer::SetEffectDuration(const std::string& effectType, const uint32_t durationMS)
{
    // find the selected effect
    auto eff = GetSelectedEffect();
    if (eff != nullptr) {
        // check it matches the effect type
        if (eff->GetEffectName() == effectType) {
            auto start = eff->GetStartTimeMS();
            if (start < PanelTimeLine->GetSequenceEnd()) {
                // get the end of the sequence
                uint32_t seqLeft = PanelTimeLine->GetSequenceEnd() - start;
                // get its start time
                // get the next effect after that effect
                uint32_t nextEffectTime = 0xFFFFFFFF;
                auto nextEff = eff->GetParentEffectLayer()->GetEffectAfterTime(eff->GetEndTimeMS());
                if (nextEff != nullptr) {
                    nextEffectTime = nextEff->GetStartTimeMS() - start;
                }
                // set the effect duration to the minimum of (video length, space up until next effect, space up until sequence end)
                uint32_t duration = TimeLine::RoundToMultipleOfPeriod(std::min(durationMS, std::min(nextEffectTime, seqLeft)), PanelTimeLine->GetFrameMS());
                eff->SetEndTimeMS(start + duration);
                PanelEffectGrid->ForceRefresh(); // update the display of effects
            }
        }
    }
}

void MainSequencer::TagAllSelectedEffects()
{
    for(int row=0;row<mSequenceElements->GetRowInformationSize();row++) {
        EffectLayer* el = mSequenceElements->GetEffectLayer(row);
        el->TagAllSelectedEffects();
    }
}

void MainSequencer::UnTagAllEffects()
{
    for (int i = 0; i < mSequenceElements->GetRowInformationSize(); i++) {
        EffectLayer* el = mSequenceElements->GetEffectLayer(i);
        if (el != nullptr) {
            el->UnTagAllEffects();
        }
    }
}

void MainSequencer::RestorePosition()
{
    if (mSequenceElements == nullptr) return;

    if (_savedTopModel != "") {
        PanelTimeLine->RestorePosition();
        Element* elem = mSequenceElements->GetElement(_savedTopModel);
        if (elem != nullptr) {
            for (int row = 0; row < mSequenceElements->GetRowInformationSize(); row++) {
                EffectLayer* el = mSequenceElements->GetEffectLayer(row);
                if (el->GetParentElement()->GetModelName() == elem->GetName()) {
                    ScrollToRow(row - mSequenceElements->GetNumberOfTimingRows());
                }
            }
        }
    }
}

void MainSequencer::SavePosition()
{
    if (mSequenceElements == nullptr || mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetNumberOfTimingRows()) == nullptr)
    {
        _savedTopModel = "";
    }
    else
    {
        PanelTimeLine->SavePosition();
        Element* elem = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetNumberOfTimingRows())->GetParentElement();
        _savedTopModel = elem->GetName();
    }
}

void MainSequencer::ScrollToRow(int row)
{
    if (mSequenceElements == nullptr) return;

    mSequenceElements->SetFirstVisibleModelRow(row);
    UpdateEffectGridVerticalScrollBar();
}

void MainSequencer::OnCheckBox_SuspendRenderClick(wxCommandEvent& event)
{
    ToggleRender(CheckBox_SuspendRender->IsChecked());
}

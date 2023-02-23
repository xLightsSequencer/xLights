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

#ifndef _glpane_
#define _glpane_
#endif

#include "wx/wx.h"
#include "wx/glcanvas.h"
#include "../graphics/xlGraphicsBase.h"
#include "../Color.h"
#include "SequenceElements.h"
#include "TimeLine.h"
#include "EffectLayer.h"
#include "Effect.h"
#include "Element.h"
#include "../EffectTreeDialog.h"
#include "../ValueCurve.h"
#include "RowHeading.h"

#include <map>
#include <array>

#define MINIMUM_EFFECT_WIDTH_FOR_SMALL_RECT 4

enum class ACTYPE { SELECT, ON, OFF, SHIMMER, TWINKLE, NILTYPEOVERRIDE };
enum class ACSTYLE { INTENSITY, RAMPUP, RAMPDOWN, RAMPUPDOWN, NILSTYLEOVERRIDE };
enum class ACMODE { FOREGROUND, BACKGROUND, MODENIL, NILMODEOVERRIDE};
enum class ACTOOL { FILL, CASCADE, TOOLNIL, NILTOOLOVERRIDE };

enum class HitLocation {
    NONE,
    LEFT_EDGE,
    LEFT_EDGE_DISCONNECT,
    LEFT,
    CENTER,
    RIGHT,
    RIGHT_EDGE_DISCONNECT,
    RIGHT_EDGE
};

enum EFF_ALIGN_MODE {
    ALIGN_START_TIMES,
    ALIGN_END_TIMES,
    ALIGN_BOTH_TIMES,
    ALIGN_CENTERPOINTS,
    ALIGN_MATCH_DURATION,
    ALIGN_START_TIMES_SHIFT,
    ALIGN_END_TIMES_SHIFT
};

wxDECLARE_EVENT(EVT_ZOOM, wxCommandEvent);
wxDECLARE_EVENT(EVT_GSCROLL, wxCommandEvent);
wxDECLARE_EVENT(EVT_PLAY_MODEL_EFFECT, wxCommandEvent);
wxDECLARE_EVENT(EVT_EFFECT_DROPPED, wxCommandEvent);
wxDECLARE_EVENT(EVT_EFFECTFILE_DROPPED, wxCommandEvent);

struct EffectDropData;

class MainSequencer;
class PixelBufferClass;
class SequenceData;
class DataLayer;

// this is used to keep track of effect found which might be generating data on a node
typedef struct findDataEffect {
    Element* e = nullptr;
    EffectLayer* el = nullptr;
    NodeLayer* nl = nullptr;
    Effect* ef = nullptr;
    DataLayer* dl = nullptr;

    // helper functions
    int GetStrand() const;
    int GetNode() const;
    std::string GetTypeDescription() const;
    std::string GetName() const;
    std::string GetStrandSubmodel() const;
} findDataEffect;

class EffectsGrid : public GRAPHICS_BASE_CLASS
{

public:
	EffectsGrid(MainSequencer* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);

	virtual ~EffectsGrid();

    virtual bool drawingUsingLogicalSize() const {
        return true;
    }

    // Public Methods
    void SetStartPixelOffset(int offset);
    void SetSequenceElements(SequenceElements* elements);
    void SetTimeline(TimeLine* timeline);
    bool DragOver(int x, int y);
    void OnDrop(int x, int y);
    void OnDropFiles(int x, int y, const wxArrayString& files);
    bool IsDroppingBetweenTimingMarks() const;
    void ForceRefresh();
    void SetTimingClickPlayMode(bool mode) {mTimingPlayOnDClick = mode;}
    void SetEffectIconBackground(bool mode) {mGridIconBackgrounds = mode;}
    void SetEffectNodeValues(bool mode) {mGridNodeValues = mode;}
    void MoveSelectedEffectUp(bool shift);
    void MoveSelectedEffectDown(bool shift);
    void MoveSelectedEffectLeft(bool shift, bool control, bool alt);
    void MoveSelectedEffectRight(bool shift, bool control, bool alt);
    void SetEffectStatusText(Effect* eff) const;
    void InsertEffectLayerAbove();
    void InsertEffectLayerBelow();
    void ToggleExpandElement(RowHeading* rowHeading);
    void PlayLoopedEffect(Effect* eff, bool loop);

    void DeleteSelectedEffects();
    void SetEffectsDescription();
    void SetEffectsTiming();
    void ProcessDroppedEffect(Effect* effect);
    void CutModelEffects(int row_number, bool allLayers);
    void CopyModelEffects(int row_number, bool allLayers);
    void PasteModelEffects(int row_number, bool allLayers);
    Effect* GetSelectedEffect() const;
    int GetSelectedEffectCount(const std::string& effectName) const;
    bool AreAllSelectedEffectsOnTheSameElement() const;
    void ApplyEffectSettingToSelected(const std::string& effectName, const std::string id, const std::string value, ValueCurve* vc, const std::string& vcid);
    void ApplyButtonPressToSelected(const std::string& effectName, const std::string id);
    void RemapSelectedDMXEffectValues(const std::vector<std::tuple<int, int, float, int>>& dmxmappings);
    void ConvertSelectedEffectsTo(const std::string& effectName);
    void DuplicateSelectedEffects();

    bool HandleACKey(wxChar key, bool shift = false);
    bool IsACActive();
    void MakeRowVisible(int row);
    void MakeColVisible(int col);
    std::string TruncateEffectSettings(SettingsMap settings, std::string name, int originalStartMS, int originalEndMS, int startMS, int endMS);
    bool DoACDraw(bool keyboard = false, ACTYPE typeOverride = ACTYPE::NILTYPEOVERRIDE, ACSTYLE styleOverride = ACSTYLE::NILSTYLEOVERRIDE, ACTOOL toolOverride = ACTOOL::NILTOOLOVERRIDE, ACMODE modeOverride = ACMODE::NILMODEOVERRIDE);

    void AlignSelectedEffects(EFF_ALIGN_MODE align_mode);

    int GetEffectRow(Effect* ef);
    Effect* OldPaste(const wxString &data, const wxString &pasteDataVer);
    Effect* Paste(const wxString &data, const wxString &pasteDataVer, bool row_paste = false);
    int GetStartColumn() { return mRangeStartCol < mRangeEndCol ? mRangeStartCol : mRangeEndCol; }
    int GetStartRow() { return mRangeStartRow < mRangeEndRow ? mRangeStartRow : mRangeEndRow; }
    int GetEndColumn() { return mRangeStartCol < mRangeEndCol ? mRangeEndCol : mRangeStartCol; }
    int GetEndRow() { return mRangeStartRow < mRangeEndRow ? mRangeEndRow : mRangeStartRow; }
    int GetMSFromColumn(int col) const;
    Element* GetActiveTimingElement() const;
    void RaiseSelectedEffectChanged(Effect* effect, bool isNew, bool updateUI = true, bool async = false) const;
    void LockEffects(bool lock);
    void DisableRenderEffects(bool disable);
    int GetDropRow() const { 
        if (mDragDropping) return mDropRow; 
        return -1;
    }

    void SetRenderDataSources(xLightsFrame *xl, const SequenceData *data) {
        seqData = data;
        xlights = xl;
    }

    void ClearSelection();

    static EffectLayer* FindOpenLayer(Element* elem, int startTimeMS, int endTimeMS);

    void sendRenderEvent(const std::string &model, int start, int end, bool clear = true);
    void sendRenderDirtyEvent();
    void UnselectEffect(bool force = false);
    void SelectEffect(Effect* ef);
    void ScrollBy(int by);
    void Draw();
    void UpdateMousePosition(int time) const;
    int GetDropStartMS() const
    {
        return mDropStartTimeMS;
    }
    bool CanDropEffect() const;
    SequenceElements* GetSequenceElements() const
    {
        return mSequenceElements;
    }

protected:

private:
    Effect* GetEffectAtRowAndTime(int row, int ms,int &index, HitLocation &selectionType);
    int GetClippedPositionFromTimeMS(int ms) const;

    void DrawFadeHints(Effect* e, int x1, int y1, int x2, int y2, xlVertexColorAccumulator *backgrounds) const;
    void CreateEffectForFile(int x, int y, const std::string& effectName, const std::string& filename);
    void render(wxPaintEvent& evt);
    void magnify(wxMouseEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftDClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
    void OnLostMouseCapture(wxMouseCaptureLostEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);

    void CreateEffectIconTextures(xlGraphicsContext *ctx);
    void SetRCToolTip();

    void DrawLines(xlGraphicsContext *ctx) const;
    void DrawSelectedCells(xlGraphicsContext *ctx);
    int DrawEffectBackground(const Row_Information_Struct* ri, const Effect *effect,
                             int x1, int y1, int x2, int y2,
                             xlVertexColorAccumulator *backgrounds) const;

    void DrawTimingEffects(int row);
    void DrawEffects(xlGraphicsContext *ctx);
    void DrawPlayMarker(xlGraphicsContext *ctx) const;
    bool AdjustDropLocations(int x, EffectLayer* el);
    void Resize(int position, bool offset, bool control);
    void RunMouseOverHitTests(int rowIndex, int x,int y);
    void UpdateTimePosition(int time) const;
    void UpdateZoomPosition(int time) const;
    void EstablishSelectionRectangle();
    void UpdateSelectionRectangle();
    void UpdateSelectedEffects();
    void SetFirstEffectSelected();
    void CheckForPartialCell(int x_pos);
    void RaiseEffectDropped(int x, int y) const;
    void RaisePlayModelEffect(Element* element, Effect* effect,bool renderEffect) const;

    std::set<EffectLayer *> GetLayersWithSelectedEffects() const;
    bool MultipleEffectsSelected() const;
    std::list<Effect*> GetSelectedEffects() const;
    bool PapagayoEffectsSelected() const;
    bool AtLeastOneEffectSelected() const;
    void ResizeSingleEffectMS(int time);
    void ResizeSingleEffect(int position);
    void ResizeMoveMultipleEffectsMS(int time, bool offset);
    void ResizeMoveMultipleEffects(int position, bool offset);
    void ResizeMoveMultipleEffectsByTime(int delta, bool force);
    void ButtUpResizeMoveMultipleEffects(bool right);
    void StretchMultipleEffectsByTime(int delta);
    void ButtUpStretchMultipleEffects(bool right);
    void GetRangeOfMovementForSelectedEffects(int &toLeft, int &toRight) const;
    void MoveAllSelectedEffects(int deltaMS, bool offset) const;
    void StretchAllSelectedEffects(int deltaMS, bool offset) const;
    int GetRow(int y) const;
    void OnGridPopup(wxCommandEvent& event);
    Effect* FillRandomEffects();
    bool OneCellSelected();
    Effect* ACDraw(ACTYPE type, ACSTYLE style, ACMODE mode, int intensity, int a, int b, int startMS, int endMS, int startRow, int endRow);
    void ACCascade(int startMS, int endMS, int startCol, int endCol, int startRow, int endRow);
    void ACFill(ACTYPE type, int startMS, int endMS, int startRow, int endRow);
    Effect* CreateACEffect(EffectLayer* el, ACTYPE type, int startMS, int endMS, int startBrightness, int midBrightness, int endBrightness, bool select);
    Effect* CreateACEffect(EffectLayer* el, std::string name, std::string settings, int startMS, int endMS, bool select, std::string pal = "");
    Effect* CreatePartialACEffect(EffectLayer* el, ACTYPE type, int startMS, int endMS, int partialStart, int partialEnd, int startBrightness, int midBrightness, int endBrightness, bool select);
    void TruncateEffect(EffectLayer* el, Effect* eff, int startMS, int endMS);
    int GetEffectBrightnessAt(std::string effName, SettingsMap settings, float pos, long startMS, long endMS);
    Effect* DuplicateAndTruncateEffect(EffectLayer* el, SettingsMap settings, std::string palette, std::string name, int originalStartMS, int originalEndMS, int startMS, int endMS, int offsetMS = 0);
    void TruncateBrightnessValueCurve(ValueCurve& vc, double startPos, double endPos, int startMS, int endMS, int originalLength);
    uint32_t FindChannel(Element* element, int strandIndex, int nodeIndex, uint8_t& channelsPerNode) const;
    void FindEffectsForData(uint32_t channel, uint8_t chans, uint32_t _findDataMS) const;

    SequenceElements* mSequenceElements;
    bool mIsDrawing = false;
    bool mGridIconBackgrounds;
    bool mTimingPlayOnDClick;
    bool mGridNodeValues = true;
    bool _doubleClick = false;

    int mStartPixelOffset;

    wxWindow* mParent;
    TimeLine* mTimeline;

    int mSelectedRow;
    Effect* mSelectedEffect;

    xlVertexAccumulator *lines = nullptr;
    xlVertexAccumulator *selectedLinesFixed = nullptr;
    xlVertexAccumulator *selectedLinesLocked = nullptr;
    xlVertexAccumulator *timingEffLines = nullptr;
    xlVertexAccumulator *selectedLines = nullptr;
    xlVertexAccumulator *selectFocusLines = nullptr;
    xlVertexAccumulator *selectFocusLinesLocked = nullptr;
    xlVertexAccumulator *selectFocusLinesDisabled = nullptr;
    xlVertexAccumulator *selectedLinesDisabled = nullptr;
    xlVertexAccumulator *linesDisabled = nullptr;
    xlVertexAccumulator *linesLocked = nullptr;
    xlVertexColorAccumulator *timingLines = nullptr;
    xlVertexColorAccumulator *backgrounds = nullptr;
    xlVertexColorAccumulator *selectedBoxes = nullptr;

    xlVertexTextureAccumulator *texts = nullptr;
    xlTexture *fontTexture = nullptr;
    int curFontSize = 0;

    std::array<float[2], EffectManager::eff_LASTEFFECT> effectIconLocations;
    xlTexture *effectIconTexture;
    xlVertexTextureAccumulator *effectIcons;

    int mResizingMode;
    int mStartResizeTimeMS;
    bool mResizing;
    bool mDragging;
    int mDragStartRow;
    int mDragStartX;
    int mDragStartY;
    int mDragEndX;
    int mDragEndY;
    float magSinceLast;

    EffectLayer* mEffectLayer;
    int mResizeEffectIndex;

    // Drag And Drop
    bool mDragDropping;
    int mDropStartX;
    int mDropEndX;
    int mDropRow;
    int mDropStartTimeMS;
    int mDropEndTimeMS;

    bool mCellRangeSelected;
    bool mPartialCellSelected;
    int mRangeStartCol;
    int mRangeEndCol;
    int mRangeStartRow;
    int mRangeEndRow;
    int mRangeCursorRow;
    int mRangeCursorCol;

    static const long ID_GRID_MNU_CUT;
    static const long ID_GRID_MNU_COPY;
    static const long ID_GRID_MNU_PASTE;
    static const long ID_GRID_MNU_DELETE;
    static const long ID_GRID_MNU_RANDOM_EFFECTS;
    static const long ID_GRID_MNU_DESCRIPTION;
    static const long ID_GRID_MNU_LOCK;
    static const long ID_GRID_MNU_UNLOCK;
    static const long ID_GRID_MNU_RENDERDISABLE;
    static const long ID_GRID_MNU_RENDERENABLE;
    static const long ID_GRID_MNU_FINDEFFECTFORDATA;
    static const long ID_GRID_MNU_TIMING;
    static const long ID_GRID_MNU_UNDO;
    static const long ID_GRID_MNU_REDO;
    static const long ID_GRID_MNU_PRESETS;
    static const long ID_GRID_MNU_BREAKDOWN_PHRASE;
    static const long ID_GRID_MNU_HALVETIMINGS;
    static const long ID_GRID_MNU_BREAKDOWN_WORD;
    static const long ID_GRID_MNU_BREAKDOWN_WORDS;
    static const long ID_GRID_MNU_ALIGN_START_TIMES;
    static const long ID_GRID_MNU_ALIGN_END_TIMES;
    static const long ID_GRID_MNU_ALIGN_BOTH_TIMES;
    static const long ID_GRID_MNU_ALIGN_CENTERPOINTS;
    static const long ID_GRID_MNU_ALIGN_MATCH_DURATION;
    static const long ID_GRID_MNU_ALIGN_START_TIMES_SHIFT;
    static const long ID_GRID_MNU_ALIGN_END_TIMES_SHIFT;
    static const long ID_GRID_MNU_SPLIT_EFFECT;
    static const long ID_GRID_MNU_DUPLICATE_EFFECT;
    EventPlayEffectArgs* playArgs = nullptr;

    const SequenceData *seqData = nullptr;
    xLightsFrame *xlights = nullptr;
    Row_Information_Struct* _findDataRI = nullptr;
    uint32_t _findDataMS = 0xFFFFFFFF;

	DECLARE_EVENT_TABLE()
};

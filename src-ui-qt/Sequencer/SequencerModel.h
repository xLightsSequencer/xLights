#pragma once

#include "../Bridge/QtSequenceDoc.h"
#include <QColor>
#include <QList>
#include <QObject>
#include <QString>

struct EffectBlock {
    int     startFrame = 0;
    int     endFrame   = 0;
    QString effectName;
    QColor  color;
    bool    selected = false;
    QString settings;   // raw "key=val,..." from xsq effectDB
    QString palette;    // raw "key=val,..." from xsq ColorPalettes
};

struct SequencerRow {
    QString            name;
    QString            modelName;    // parent model (same as name for single-layer rows)
    int                layerIndex = 0;   // 0-based layer within the model
    int                layerCount = 1;   // total layers this model has
    QList<EffectBlock> blocks;

    bool isLayerRow()  const { return layerIndex > 0; }
    bool isModelRow()  const { return layerIndex == 0; }
};

// Owns all sequencer data. Phase 4: standalone with generated mock data.
// Phase 5+ will populate from src-core SequenceElements.
class SequencerModel : public QObject {
    Q_OBJECT

public:
    explicit SequencerModel(QObject* parent = nullptr);

    // ── Data ─────────────────────────────────────────────────────────────
    int rowCount()    const { return _rows.size(); }
    int totalFrames() const { return _totalFrames; }
    int fps()         const { return _fps; }

    const SequencerRow& row(int r) const { return _rows.at(r); }
    SequencerRow&       row(int r)       { return _rows[r]; }

    // ── View geometry ────────────────────────────────────────────────────
    double pixelsPerFrame() const { return _pxPerFrame; }
    void   setPixelsPerFrame(double v);

    int rowHeight()  const { return _rowHeight; }
    int gridWidth()  const { return int(_totalFrames * _pxPerFrame); }
    int gridHeight() const { return _rows.size() * _rowHeight; }

    // ── Playhead ─────────────────────────────────────────────────────────────
    int  playheadFrame() const { return _playheadFrame; }
    void setPlayhead(int frame);

    // ── Selection ────────────────────────────────────────────────────────
    // Returns {rowIndex, blockIndex} or {-1,-1} if none.
    int selectedRow()   const { return _selRow; }
    int selectedBlock() const { return _selBlock; }
    void select(int row, int block);
    void clearSelection();

    // ── Block manipulation ────────────────────────────────────────────────
    void moveBlock(int row, int block, int newStart);
    void resizeBlock(int row, int block, int newStart, int newEnd);

    // ── Coordinate helpers ────────────────────────────────────────────────
    int   frameAt(int x) const;         // pixel x → frame number (clamped)
    int   xAt(int frame) const;         // frame → pixel x
    int   rowAt(int y)   const;         // pixel y → row index (-1 = none)
    int   yAt(int row)   const;         // row index → pixel y (top edge)

    // Hit-test a point; sets out parameters. Returns false if no block hit.
    enum HitZone { None, Body, ResizeLeft, ResizeRight };
    bool hitTest(QPoint pt, int& outRow, int& outBlock, HitZone& outZone) const;

    void loadFromSequence(const QtSequenceInfo& seq); // Phase 2: real xsq data

signals:
    void modelChanged();
    void selectionChanged();
    void geometryChanged();

private:
    QList<SequencerRow> _rows;
    int    _totalFrames = 0;
    int    _fps         = 20;
    double _pxPerFrame  = 3.0;
    int    _rowHeight   = 28;
    int    _selRow        = -1;
    int    _selBlock      = -1;
    int    _playheadFrame = 0;

    static constexpr int kResizeHandleW = 8;
};

#pragma once

#include "SequencerModel.h"
#include <QObject>
#include <QPoint>

class SequencerController : public QObject {
    Q_OBJECT

public:
    explicit SequencerController(SequencerModel* model, QObject* parent = nullptr);

    // Pass mouse events from SequencerGridCanvas.
    void mousePress(QPoint pos, Qt::KeyboardModifiers mods);
    void mouseMove(QPoint pos);
    void mouseRelease(QPoint pos);

    // Ghost state — read by the canvas during paintEvent.
    bool  hasGhost()       const { return _dragging; }
    int   ghostRow()       const { return _dragRow; }
    int   ghostStartFrame() const { return _ghostStart; }
    int   ghostEndFrame()   const { return _ghostEnd; }
    SequencerModel::HitZone dragZone() const { return _zone; }

signals:
    void repaintNeeded();

private:
    int  snap(int frame) const;   // snap to nearest frame (extend for timing marks later)

    SequencerModel*           _model;
    bool                      _dragging   = false;
    int                       _dragRow    = -1;
    int                       _dragBlock  = -1;
    SequencerModel::HitZone   _zone       = SequencerModel::None;
    int                       _dragOriginX= 0;
    int                       _origStart  = 0;
    int                       _origEnd    = 0;
    int                       _ghostStart = 0;
    int                       _ghostEnd   = 0;
};

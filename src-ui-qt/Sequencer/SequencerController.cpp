#include "SequencerController.h"

SequencerController::SequencerController(SequencerModel* model, QObject* parent)
    : QObject(parent), _model(model) {}

int SequencerController::snap(int frame) const {
    // Phase 4: snap to frame boundary only.
    // Phase 6+: also snap to timing marks when Alt held.
    return qBound(0, frame, _model->totalFrames());
}

void SequencerController::mousePress(QPoint pos, Qt::KeyboardModifiers /*mods*/) {
    int row, block;
    SequencerModel::HitZone zone;

    if (!_model->hitTest(pos, row, block, zone)) {
        _model->clearSelection();
        emit repaintNeeded();
        return;
    }

    _model->select(row, block);

    _dragging    = true;
    _dragRow     = row;
    _ghostRow    = row;   // starts on the source row
    _dragBlock   = block;
    _zone        = zone;
    _dragOriginX = pos.x();
    _origStart   = _model->row(row).blocks[block].startFrame;
    _origEnd     = _model->row(row).blocks[block].endFrame;
    _ghostStart  = _origStart;
    _ghostEnd    = _origEnd;

    emit repaintNeeded();
}

void SequencerController::mouseMove(QPoint pos) {
    if (!_dragging) return;

    int dx     = pos.x() - _dragOriginX;
    int dFrame = int(dx / _model->pixelsPerFrame());

    switch (_zone) {
    case SequencerModel::Body: {
        _ghostStart = snap(_origStart + dFrame);
        _ghostEnd   = _ghostStart + (_origEnd - _origStart);
        // Track target row: follow mouse Y, fall back to source row.
        int targetRow = _model->rowAt(pos.y());
        if (targetRow >= 0 && _model->isRowVisible(targetRow))
            _ghostRow = targetRow;
        break;
    }
    case SequencerModel::ResizeLeft:
        _ghostStart = snap(qMin(_origStart + dFrame, _origEnd - 1));
        _ghostEnd   = _origEnd;
        break;
    case SequencerModel::ResizeRight:
        _ghostStart = _origStart;
        _ghostEnd   = snap(qMax(_origEnd + dFrame, _origStart + 1));
        break;
    default:
        break;
    }

    emit repaintNeeded();
}

void SequencerController::mouseRelease(QPoint /*pos*/) {
    if (!_dragging) return;

    if (_zone == SequencerModel::Body) {
        if (_ghostRow != _dragRow)
            _model->moveBlockToRow(_dragRow, _dragBlock, _ghostRow, _ghostStart);
        else
            _model->moveBlock(_dragRow, _dragBlock, _ghostStart);
    } else if (_zone == SequencerModel::ResizeLeft || _zone == SequencerModel::ResizeRight) {
        _model->resizeBlock(_dragRow, _dragBlock, _ghostStart, _ghostEnd);
    }

    _dragging = false;
    emit repaintNeeded();
}

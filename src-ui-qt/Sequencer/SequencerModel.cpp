#include "SequencerModel.h"
#include "../Bridge/QtSequenceDoc.h"
#include <QPoint>
#include <spdlog/spdlog.h>

SequencerModel::SequencerModel(QObject* parent) : QObject(parent) {
}

// ── Geometry ──────────────────────────────────────────────────────────────────
void SequencerModel::setPixelsPerFrame(double v) {
    _pxPerFrame = qBound(0.5, v, 20.0);
    emit geometryChanged();
}

int SequencerModel::frameAt(int x) const {
    return qBound(0, int(x / _pxPerFrame), _totalFrames - 1);
}
int SequencerModel::xAt(int frame) const {
    return int(frame * _pxPerFrame);
}

// y coordinate helpers work in VISUAL space (visible rows only).
int SequencerModel::rowAt(int y) const {
    int vis = y / _rowHeight;
    return visualToData(vis);
}
int SequencerModel::yAt(int dataRow) const {
    // Convert data row index to visual y coordinate.
    int vis = 0;
    for (int r = 0; r < dataRow && r < _rows.size(); ++r)
        if (isRowVisible(r)) ++vis;
    return vis * _rowHeight;
}

// ── Collapse / visibility ──────────────────────────────────────────────────────

void SequencerModel::toggleCollapse(int r) {
    if (r < 0 || r >= _rows.size() || !_rows[r].isModelRow()) return;
    _rows[r].collapsed = !_rows[r].collapsed;
    emit geometryChanged();
}

bool SequencerModel::isRowVisible(int r) const {
    if (r < 0 || r >= _rows.size()) return false;
    if (_rows[r].isModelRow()) return true;
    // Layer rows: visible only when the model row is NOT collapsed.
    // Walk back to find the parent model row.
    for (int p = r - 1; p >= 0; --p) {
        if (_rows[p].isModelRow() && _rows[p].modelName == _rows[r].modelName)
            return !_rows[p].collapsed;
    }
    return true;
}

int SequencerModel::visibleRowCount() const {
    int v = 0;
    for (int r = 0; r < _rows.size(); ++r)
        if (isRowVisible(r)) ++v;
    return v;
}

int SequencerModel::visualToData(int visualRow) const {
    int vis = 0;
    for (int r = 0; r < _rows.size(); ++r) {
        if (!isRowVisible(r)) continue;
        if (vis == visualRow) return r;
        ++vis;
    }
    return -1;
}

// ── Hit-test ──────────────────────────────────────────────────────────────────
bool SequencerModel::hitTest(QPoint pt, int& outRow, int& outBlock, HitZone& outZone) const {
    int r = rowAt(pt.y());
    if (r < 0) return false;
    int x = pt.x();
    for (int b = 0; b < _rows[r].blocks.size(); ++b) {
        const EffectBlock& blk = _rows[r].blocks[b];
        int x0 = xAt(blk.startFrame);
        int x1 = xAt(blk.endFrame);
        if (x < x0 || x > x1) continue;
        outRow   = r;
        outBlock = b;
        if (x <= x0 + kResizeHandleW)      outZone = ResizeLeft;
        else if (x >= x1 - kResizeHandleW) outZone = ResizeRight;
        else                                outZone = Body;
        return true;
    }
    return false;
}

// ── Selection ─────────────────────────────────────────────────────────────────
void SequencerModel::select(int row, int block) {
    if (row == _selRow && block == _selBlock) return;
    if (_selRow >= 0 && _selBlock >= 0)
        _rows[_selRow].blocks[_selBlock].selected = false;
    _selRow   = row;
    _selBlock = block;
    if (_selRow >= 0 && _selBlock >= 0)
        _rows[_selRow].blocks[_selBlock].selected = true;
    emit selectionChanged();
}

void SequencerModel::clearSelection() { select(-1, -1); }

// ── Playhead ──────────────────────────────────────────────────────────────────
void SequencerModel::setPlayhead(int frame) {
    int clamped = qBound(0, frame, _totalFrames);
    if (clamped == _playheadFrame) return;
    _playheadFrame = clamped;
    emit geometryChanged();
}

// ── Block manipulation ────────────────────────────────────────────────────────
void SequencerModel::moveBlock(int row, int block, int newStart) {
    auto& blk  = _rows[row].blocks[block];
    int   len  = blk.endFrame - blk.startFrame;
    blk.startFrame = qBound(0, newStart, _totalFrames - len);
    blk.endFrame   = blk.startFrame + len;
    emit modelChanged();
}

void SequencerModel::moveBlockToRow(int fromRow, int fromBlock, int toRow, int newStart) {
    if (fromRow == toRow) { moveBlock(fromRow, fromBlock, newStart); return; }
    if (fromRow < 0 || fromRow >= _rows.size()) return;
    if (toRow   < 0 || toRow   >= _rows.size()) return;
    if (fromBlock < 0 || fromBlock >= _rows[fromRow].blocks.size()) return;

    // Deselect old location before moving.
    _rows[fromRow].blocks[fromBlock].selected = false;

    EffectBlock blk = _rows[fromRow].blocks[fromBlock];
    const int   len = blk.endFrame - blk.startFrame;
    blk.startFrame  = qBound(0, newStart, _totalFrames - len);
    blk.endFrame    = blk.startFrame + len;
    blk.selected    = true;

    _rows[fromRow].blocks.removeAt(fromBlock);
    _rows[toRow].blocks.append(blk);

    // Update selection to the new location.
    _selRow   = toRow;
    _selBlock = _rows[toRow].blocks.size() - 1;

    emit selectionChanged();
    emit modelChanged();
}

void SequencerModel::resizeBlock(int row, int block, int newStart, int newEnd) {
    auto& blk      = _rows[row].blocks[block];
    blk.startFrame = qBound(0, newStart, newEnd - 1);
    blk.endFrame   = qBound(blk.startFrame + 1, newEnd, _totalFrames);
    emit modelChanged();
}

// ── Block color (shared by load and create) ───────────────────────────────────
static QColor effectColor(const QString& name) {
    uint h = qHash(name);
    return QColor::fromHsv(int((h * 137u) % 360), 160, 200);
}

void SequencerModel::createBlock(int row, int startFrame, int endFrame, const QString& effectName) {
    if (row < 0 || row >= _rows.size()) return;
    EffectBlock blk;
    blk.startFrame = qBound(0, startFrame, _totalFrames);
    blk.endFrame   = qBound(blk.startFrame + 1, endFrame, _totalFrames);
    blk.effectName = effectName;
    blk.color      = effectColor(effectName);
    _rows[row].blocks.append(blk);
    emit modelChanged();
}

// ── Real xsq data ─────────────────────────────────────────────────────────────
void SequencerModel::loadFromSequence(const QtSequenceInfo& seq) {
    if (!seq.isValid()) return;

    clearSelection();
    _rows.clear();
    _fps         = seq.fps();
    _totalFrames = seq.totalFrames();

    spdlog::info("SequencerModel: loading {} elements, {} frames @ {}fps",
                 seq.elements.size(), _totalFrames, _fps);

    const int frameMs = _fps > 0 ? 1000 / _fps : 50;

    for (const auto& elem : seq.elements) {
        const int nLayers = elem.layers.size();

        for (int li = 0; li < qMax(1, nLayers); ++li) {
            SequencerRow row;
            row.modelName  = elem.name;
            row.layerIndex = li;
            row.layerCount = nLayers;

            if (nLayers <= 1 || li == 0) {
                row.name = elem.name;
            } else {
                row.name = QString("  └ Layer %1").arg(li + 1);
            }

            if (li < nLayers) {
                for (const auto& blk : elem.layers[li].blocks) {
                    EffectBlock b;
                    b.startFrame = blk.startMs / frameMs;
                    b.endFrame   = blk.endMs   / frameMs;
                    b.effectName = blk.name;
                    b.color      = effectColor(blk.name);
                    b.settings   = blk.settings;
                    b.palette    = blk.palette;
                    if (b.endFrame > b.startFrame)
                        row.blocks.append(b);
                }
            }

            // Always include the first layer row; skip empty subsequent layers.
            if (li == 0 || !row.blocks.isEmpty())
                _rows.append(row);
        }
    }

    // If the xsq had no sequenceDuration, derive it from the max effect end.
    if (_totalFrames == 0) {
        for (const auto& row : _rows)
            for (const auto& blk : row.blocks)
                _totalFrames = qMax(_totalFrames, blk.endFrame + _fps);
        if (_totalFrames == 0) _totalFrames = 600;
    }

    emit geometryChanged();
    emit modelChanged();
}

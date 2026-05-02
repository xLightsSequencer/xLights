#include "SequencerWidget.h"
#include "SequencerModel.h"
#include "SequencerController.h"
#include "SequencerGridCanvas.h"
#include "RowHeaderWidget.h"
#include "TimingRulerCanvas.h"

#include <QGridLayout>
#include <QScrollArea>
#include <QScrollBar>

SequencerWidget::SequencerWidget(QWidget* parent) : QWidget(parent) {
    _model = new SequencerModel(this);
    _ctrl  = new SequencerController(_model, this);

    // ── Widgets ───────────────────────────────────────────────────────────
    _headers = new RowHeaderWidget(_model, this);
    _ruler   = new TimingRulerCanvas(_model, this);
    _grid    = new SequencerGridCanvas(_model, _ctrl, this);

    // Wrap the grid in a scroll area so it scrolls both axes.
    _scrollArea = new QScrollArea(this);
    _scrollArea->setWidget(_grid);
    _scrollArea->setWidgetResizable(false);
    _scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    _scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    _scrollArea->setFrameShape(QFrame::NoFrame);

    _hbar = _scrollArea->horizontalScrollBar();
    _vbar = _scrollArea->verticalScrollBar();

    // ── Layout ────────────────────────────────────────────────────────────
    // Row 0: [corner] | [timing ruler]
    // Row 1: [headers] | [grid scroll area]
    auto* grid = new QGridLayout(this);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(0);

    auto* corner = new QWidget(this);
    corner->setFixedSize(_headers->width(), _ruler->height());
    corner->setStyleSheet("background: #1e1e1e;");
    grid->addWidget(corner,      0, 0);
    grid->addWidget(_ruler,      0, 1);
    grid->addWidget(_headers,    1, 0);
    grid->addWidget(_scrollArea, 1, 1);
    grid->setColumnStretch(1, 1);
    grid->setRowStretch(1, 1);

    // ── Sync scrollbars ───────────────────────────────────────────────────
    connect(_hbar, &QScrollBar::valueChanged, this, &SequencerWidget::onHScroll);
    connect(_vbar, &QScrollBar::valueChanged, this, &SequencerWidget::onVScroll);

    // ── Forward block selection ───────────────────────────────────────────
    connect(_grid, &SequencerGridCanvas::blockSelected,
            this,  &SequencerWidget::blockSelected);

    // Collapse triangle click → grid resize is handled by geometryChanged signal above.

    // Resize grid canvas when model geometry changes
    connect(_model, &SequencerModel::geometryChanged, this, [this]() {
        _grid->resize(_model->gridWidth(), _model->gridHeight());
    });
    connect(_model, &SequencerModel::modelChanged, this, [this]() {
        _grid->resize(_model->gridWidth(), _model->gridHeight());
    });
}

void SequencerWidget::setPlayhead(int frame) {
    _model->setPlayhead(frame);
    _ruler->setPlayhead(frame);
}

void SequencerWidget::onHScroll(int value) {
    _ruler->setScrollOffset(value);
}

void SequencerWidget::onVScroll(int value) {
    _headers->setScrollOffset(value);
}

#pragma once
#include "SequencerModel.h"
#include <QWidget>

class SequencerController;

class SequencerGridCanvas : public QWidget {
    Q_OBJECT

public:
    SequencerGridCanvas(SequencerModel* model,
                        SequencerController* ctrl,
                        QWidget* parent = nullptr);

    QSize sizeHint() const override {
        return QSize(_model->gridWidth(), _model->gridHeight());
    }

signals:
    void blockSelected(int row, int block);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void dragEnterEvent(QDragEnterEvent*) override;
    void dragMoveEvent(QDragMoveEvent*) override;
    void dragLeaveEvent(QDragLeaveEvent*) override;
    void dropEvent(QDropEvent*) override;

private:
    void drawBlock(QPainter& p, int x0, int x1, int y,
                   int h, const QColor& col, const QString& name,
                   bool selected, float opacity = 1.0f);

    SequencerModel*      _model;
    SequencerController* _ctrl;

    // Drop-ghost state (separate from the move/resize ghost in SequencerController)
    bool    _hasDrop        = false;
    int     _dropRow        = -1;
    int     _dropFrame      = 0;
    int     _dropDuration   = 0;   // frames
    QString _dropEffectName;
};

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

private:
    void drawBlock(QPainter& p, int x0, int x1, int y,
                   int h, const QColor& col, const QString& name,
                   bool selected, float opacity = 1.0f);

    SequencerModel*      _model;
    SequencerController* _ctrl;
};

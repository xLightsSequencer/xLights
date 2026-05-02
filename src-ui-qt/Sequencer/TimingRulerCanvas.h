#pragma once
#include "SequencerModel.h"
#include <QWidget>

class TimingRulerCanvas : public QWidget {
    Q_OBJECT
public:
    explicit TimingRulerCanvas(SequencerModel* model, QWidget* parent = nullptr);
    void setScrollOffset(int xOffset);
    void setPlayhead(int frame);
    QSize sizeHint() const override { return QSize(_model->gridWidth(), 24); }

protected:
    void paintEvent(QPaintEvent*) override;

private:
    SequencerModel* _model;
    int             _xOff     = 0;
    int             _playhead = 0;
};

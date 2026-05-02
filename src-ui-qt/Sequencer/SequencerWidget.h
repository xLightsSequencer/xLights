#pragma once
#include <QWidget>

class SequencerModel;
class SequencerController;
class SequencerGridCanvas;
class RowHeaderWidget;
class TimingRulerCanvas;
class QScrollArea;
class QScrollBar;

class SequencerWidget : public QWidget {
    Q_OBJECT

public:
    explicit SequencerWidget(QWidget* parent = nullptr);

    SequencerModel* model() const { return _model; }

    void setPlayhead(int frame);

signals:
    void blockSelected(int row, int block);

private slots:
    void onHScroll(int value);
    void onVScroll(int value);

private:
    SequencerModel*      _model;
    SequencerController* _ctrl;
    SequencerGridCanvas* _grid;
    RowHeaderWidget*     _headers;
    TimingRulerCanvas*   _ruler;
    QScrollArea*         _scrollArea;
    QScrollBar*          _hbar;
    QScrollBar*          _vbar;
};

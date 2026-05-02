#pragma once
#include <QWidget>
class SequencerModel;

class RowHeaderWidget : public QWidget {
    Q_OBJECT
public:
    explicit RowHeaderWidget(SequencerModel* model, QWidget* parent = nullptr);
    void setScrollOffset(int yOffset);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent*) override;

private:
    SequencerModel* _model;
    int             _yOff = 0;
};

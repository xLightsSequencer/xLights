#pragma once
#include <QWidget>
class SequencerModel;

class RowHeaderWidget : public QWidget {
    Q_OBJECT
public:
    explicit RowHeaderWidget(SequencerModel* model, QWidget* parent = nullptr);
    void setScrollOffset(int yOffset);
    QSize sizeHint() const override;

signals:
    void collapseToggled(int dataRow);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* ev) override;

private:
    SequencerModel* _model;
    int             _yOff = 0;
};

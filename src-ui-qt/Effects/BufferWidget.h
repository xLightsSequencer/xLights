#pragma once
#include <QColor>
#include <QComboBox>
#include <QList>
#include <QWidget>

// Shows the raw 2-D render buffer (effect output before model mapping).
// Also hosts the layer blend-mode selector and buffer-style combo.
class BufferWidget : public QWidget {
    Q_OBJECT
public:
    explicit BufferWidget(QWidget* parent = nullptr);
    QSize sizeHint() const override { return {260, 200}; }

    // Push real pixel data here once src-core rendering is wired up.
    void setPixels(int w, int h, const QList<QColor>& pixels);

    // Clears to "no data" state.
    void clear();

    QString blendMode()  const;
    QString bufferStyle() const;

signals:
    void blendModeChanged(const QString& mode);
    void bufferStyleChanged(const QString& style);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QComboBox* _blendCombo;
    QComboBox* _bufferCombo;

    int           _bufW = 0;
    int           _bufH = 0;
    QList<QColor> _pixels;
};

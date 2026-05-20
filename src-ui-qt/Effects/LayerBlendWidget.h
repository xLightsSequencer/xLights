#pragma once
#include <QWidget>

class QComboBox;

// Tab 3 — Layer blending settings.
// Owns the blend-mode selector.  Future: opacity slider, layer priority.
class LayerBlendWidget : public QWidget {
    Q_OBJECT
public:
    explicit LayerBlendWidget(QWidget* parent = nullptr);

    QString blendMode() const;
    void    setBlendMode(const QString& mode);

signals:
    void blendModeChanged(const QString& mode);

private:
    QComboBox* _blendCombo;
};

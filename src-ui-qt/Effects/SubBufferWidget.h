#pragma once
#include <QWidget>
#include <QVariantMap>

class QCheckBox;
class QComboBox;
class QLabel;
class QSlider;
class SubBufferCanvas;

// Panel that mirrors xLights' "Buffer" tab on every effect:
//   - Buffer Style   (B_CHOICE_BufferStyle  → key "CHOICE_BufferStyle")
//   - Transform      (B_CHOICE_BufferTransform → key "CHOICE_BufferTransform")
//   - Overlay Bkg    (B_CHECKBOX_OverlayBkg  → key "CHECKBOX_OverlayBkg")
//   - Sub-Buffer     (B_CUSTOM_SubBuffer     → key "CUSTOM_SubBuffer")
//     encoded as "left%xleft%bright%xtop%x0x0" using 'x' as delimiter.
class SubBufferWidget : public QWidget {
    Q_OBJECT
public:
    explicit SubBufferWidget(QWidget* parent = nullptr);

    // Load from the flat settings map (keys with CHOICE_/CHECKBOX_/CUSTOM_ prefix).
    void loadSettings(const QVariantMap& settings);

    // Write current values back into the settings map.
    void writeSettings(QVariantMap& settings) const;

signals:
    void changed();

private:
    void connectSignals();
    void updateSubBufferLabel();

    static QString encodeSubBuffer(int left, int bottom, int right, int top);
    static void    decodeSubBuffer(const QString& sb,
                                   int& left, int& bottom, int& right, int& top);

    QComboBox* _style     = nullptr;
    QComboBox* _transform = nullptr;
    QCheckBox* _overlay   = nullptr;

    QSlider* _left   = nullptr;
    QSlider* _right  = nullptr;
    QSlider* _bottom = nullptr;
    QSlider* _top    = nullptr;

    QLabel*          _sbLabel   = nullptr;
    SubBufferCanvas* _canvas    = nullptr;
    QCheckBox*       _oversized = nullptr;

    void setSliderRanges(bool oversized);
};

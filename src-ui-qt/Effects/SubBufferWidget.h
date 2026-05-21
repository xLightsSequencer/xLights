#pragma once
#include <QVariantMap>
#include <QWidget>
#include <nlohmann/json.hpp>

class QCheckBox;
class QScrollArea;
class QTabWidget;
class SubBufferCanvas;

// Buffer tab: ECB-driven controls from Buffer.json (BufferStyle, BufferStagger,
// PerPreviewCamera, BufferTransform, Blur, OverlayBkg) + interactive SubBufferCanvas.
// Roto-Zoom tab: ECB-driven controls from Buffer.json Roto-Zoom tab.
// All settings keys are bare IDs matching Buffer.json property ids.
class SubBufferWidget : public QWidget {
    Q_OBJECT
public:
    explicit SubBufferWidget(QWidget* parent = nullptr);

    void loadSettings(const QVariantMap& settings);
    void writeSettings(QVariantMap& settings) const;

signals:
    void changed();

private:
    void buildBufferEcb();
    void buildRotoZoomEcb();
    nlohmann::json tabProps(const std::string& tabLabel) const;

    static QString encodeSubBuffer(int left, int bottom, int right, int top);
    static void    decodeSubBuffer(const QString& s, int& l, int& b, int& r, int& t);

    nlohmann::json   _json;
    QVariantMap      _settings;

    QTabWidget*      _tabs      = nullptr;
    QScrollArea*     _bufScroll = nullptr;
    QScrollArea*     _rzScroll  = nullptr;
    SubBufferCanvas* _canvas    = nullptr;
    QCheckBox*       _oversized = nullptr;
};

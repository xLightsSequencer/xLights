#pragma once

#include <QRhiWidget>
#include <rhi/qrhi.h>

// Base class for all Qt-side rendering surfaces.
// Subclasses override paintCanvas() to issue draw calls via the command buffer.
// The RHI backend is selected automatically by Qt: D3D11 on Windows,
// Metal on macOS, Vulkan/OpenGL on Linux.
class RhiCanvas : public QRhiWidget {
    Q_OBJECT

public:
    explicit RhiCanvas(QWidget* parent = nullptr);
    ~RhiCanvas() override = default;

    QSize sizeHint() const override { return {400, 300}; }

protected:
    // Override to issue draw calls. Called every frame inside a render pass
    // that clears to backgroundColor(). Default: empty (clear only).
    virtual void paintCanvas(QRhiCommandBuffer* cb) { Q_UNUSED(cb) }

    // Background colour used for the render pass clear. Default: #1a1a1a.
    virtual QColor backgroundColor() const { return QColor(0x1a, 0x1a, 0x1a); }

    // QRhiWidget interface
    void initialize(QRhiCommandBuffer* cb) override;
    void render(QRhiCommandBuffer* cb) override;
};

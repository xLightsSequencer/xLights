#pragma once
#include "../Bridge/QtSequenceDoc.h"
#include <QWidget>
#include <QString>

class QComboBox;
class QLabel;
class QListWidget;
class QScrollArea;
class QSplitter;
class QVBoxLayout;

// Phase 16 — Controller Visualizer.
// Shows one row per port for the selected controller with model boxes
// arranged by start channel. Supports drag-and-drop wiring.
class ControllerVisualizerWindow : public QWidget {
    Q_OBJECT
public:
    explicit ControllerVisualizerWindow(QWidget* parent = nullptr);

    // Reload from the current sequence / show file.
    void refresh();

    // Open the window and pre-select a specific controller.
    void openForController(const QString& controllerName);

private slots:
    void onControllerChanged(const QString& name);

private:
    void buildPortView(const QString& controllerName);
    void clearPortView();

    // Save a model's wiring change to xlights_rgbeffects.xml.
    void saveWiring(const QString& modelName,
                    const QString& controllerName, int port);

    bool eventFilter(QObject* obj, QEvent* ev) override;

    QtSequenceInfo _data;   // effective sequence (live or show-file fallback)

    QComboBox*   _ctrlCombo    = nullptr;
    QScrollArea* _portScroll   = nullptr;
    QWidget*     _portContainer = nullptr;
    QVBoxLayout* _portLayout   = nullptr;
    QListWidget* _availList    = nullptr;
    QSplitter*   _split        = nullptr;
};

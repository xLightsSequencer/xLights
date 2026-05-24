#pragma once
#include "../Bridge/QtSequenceDoc.h"
#include <QWidget>
#include <QString>
#include <QStringList>

class QComboBox;
class QLabel;
class QScrollArea;
class QSplitter;
class QVBoxLayout;
class QtRenderBridge;
class AvailableModelsList;

// Phase 16 — Controller Visualizer.
// Shows one row per port for the selected controller with model boxes
// arranged by start channel. Supports drag-and-drop wiring.
class ControllerVisualizerWindow : public QWidget {
    Q_OBJECT
public:
    explicit ControllerVisualizerWindow(QWidget* parent = nullptr);

    // Reload from the current sequence / show file.
    void refresh();

    // Provide the render bridge so buildPortView() can call GetNumStrings()
    // on the real src-core Model objects via ModelManager.
    void setRenderBridge(QtRenderBridge* bridge);

    // Open the window and pre-select a specific controller.
    void openForController(const QString& controllerName);

private slots:
    void onControllerChanged(const QString& name);

private:
    void buildPortView(const QString& controllerName);
    void clearPortView();

    // Apply a drop: update wiring of the moved model and re-stitch ModelChain
    // attributes for the source and target ports based on the dropped position.
    void applyDrop(const QString& modelName, int fromPort, int toPort,
                   int insertIndex);

    // Names of models currently displayed on a given port row, in display order.
    QStringList portModelNames(int port) const;

    // Persist controller assignment and re-write ModelChain attributes for the
    // target port (in targetOrder) and the source port (in sourceOrder).
    // Pass empty controllerName + port=0 to clear the model's wiring.
    void saveWiring(const QString& modelName,
                    const QString& controllerName, int port,
                    const QStringList& targetOrder,
                    const QStringList& sourceOrder);

    QtSequenceInfo  _data;   // effective sequence (live or show-file fallback)
    QtRenderBridge* _bridge = nullptr;

    QComboBox*           _ctrlCombo     = nullptr;
    QScrollArea*         _portScroll    = nullptr;
    QWidget*             _portContainer = nullptr;
    QVBoxLayout*         _portLayout    = nullptr;
    AvailableModelsList* _availList     = nullptr;
    QSplitter*           _split         = nullptr;
};

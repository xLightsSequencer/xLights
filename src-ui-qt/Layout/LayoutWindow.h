#pragma once
#include "../Bridge/QtSequenceDoc.h"
#include <QString>
#include <QWidget>

class ModelEditDialog;
class ModelLayoutCanvas;
class LayoutPropertyTree;
class QtRenderBridge;
class QListWidget;
class QListWidgetItem;
class QSplitter;
class QTabWidget;

// Phase 11 layout editor window.
// Non-modal, stays open alongside the sequencer.
// Left panel: tabbed list (Models / Groups / Controllers) + LayoutPropertyTree.
// Right panel: ModelLayoutCanvas for interactive 2D yard view.
class LayoutWindow : public QWidget {
    Q_OBJECT
public:
    explicit LayoutWindow(QWidget* parent = nullptr);

    // Provide the render bridge so refresh() can pull ModelManager geometry.
    // Must be called once before the first refresh().
    void setRenderBridge(QtRenderBridge* bridge);

    // Reload all lists and canvas from the current sequence.
    void refresh();

signals:
    void visualizerRequested(const QString& controllerName);
    void uploadRequested(const QString& controllerName);

private slots:
    void onModelListClicked(QListWidgetItem* item);
    void onGroupListClicked(QListWidgetItem* item);
    void onControllerListClicked(QListWidgetItem* item);
    void onCanvasModelClicked(const QString& modelName);

private:
    void buildModelProps(const QString& name);
    void buildGroupProps(const QString& name);
    void buildControllerProps(const QString& name);
    void clearProps();

    // Left side
    QSplitter*          _leftSplit      = nullptr;
    QTabWidget*         _tabs           = nullptr;
    QListWidget*        _modelList      = nullptr;
    QListWidget*        _groupList      = nullptr;
    QListWidget*        _controllerList = nullptr;
    LayoutPropertyTree* _props          = nullptr;

    // Right side
    ModelLayoutCanvas*  _canvas         = nullptr;

    ModelEditDialog*    _editDialog     = nullptr;
    QtRenderBridge*     _bridge         = nullptr;

    // Effective sequence data — either the live sequence or the show-file fallback.
    // All property lookups use this instead of currentSequence() directly.
    QtSequenceInfo      _data;
};

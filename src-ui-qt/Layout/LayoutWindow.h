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

    // Phase 20 — add / delete entities.
    void onAddModel();
    void onAddGroup();
    void onAddController();
    void onDeleteModel();
    void onDeleteGroup();
    void onDeleteController();

    // Phase 20f — finalise an in-progress click-to-drop placement at the
    // canvas click point (world-space coords).  Reads _pendingModel for the
    // dialog values that were captured when the user accepted Add Model.
    void onPlacementClicked(double wx, double wy);
    void onPlacementCancelled();

private:
    void buildModelProps(const QString& name);
    void buildGroupProps(const QString& name);
    void buildControllerProps(const QString& name);
    void clearProps();

    // After an entity is added or removed, rebuild list widgets + canvas
    // from live src-core managers.  Used by the Add / Delete flows; cheaper
    // than the full refresh() which also pulls from QtXLightsApp's snapshot.
    void rebuildModelLists();
    void rebuildControllerList();

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

    // Phase 20f — dialog values captured by onAddModel, waiting for the user
    // to click on the canvas to place the model.  active=false when no
    // placement is in flight.
    struct PendingModel {
        bool    active = false;
        QString type;
        QString name;
        QString layoutGroup;
        QString startChannel;
    } _pendingModel;
};

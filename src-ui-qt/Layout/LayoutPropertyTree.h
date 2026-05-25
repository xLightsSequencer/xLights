#pragma once
#include <QString>
#include <QTreeWidget>

class Controller;
class Model;
class ModelGroup;
class ModelManager;
class OutputManager;
class QTreeWidgetItem;

// Property-grid-style tree used by the Layout window for all three left-tab
// selections (Models, Groups, Controllers).  Mirrors the wx LayoutPanel's
// wxPropertyGrid layout — bold top-level "category" rows followed by indented
// "Property : Value" rows.
//
// Built read-only in phase 1/2/7; phases 3+ add the controller-connection
// sub-tree on models, per-type properties, and inline editors.
//
// Data comes from live src-core objects (`Model*`, `ModelGroup*`, `Controller*`)
// reached through `QtRenderBridge::modelManager()` and
// `ModelManager::GetOutputManager()`.
class LayoutPropertyTree : public QTreeWidget {
    Q_OBJECT
public:
    explicit LayoutPropertyTree(QWidget* parent = nullptr);

    // Provide the live ModelManager.  May be nullptr early on; the tree
    // simply clears itself in that case.
    void setModelManager(ModelManager* mm);

    // Replace contents with properties for the named model.
    // No-op if the model can't be found.
    void showModel(const QString& name);

    // Replace contents with properties for the named model group.
    // No-op if the group can't be found.
    void showGroup(const QString& name);

    // Replace contents with properties for the named controller.
    // No-op if the controller can't be found.
    void showController(const QString& name);

    // Clear all rows.
    void clearAll();

private:
    QTreeWidgetItem* addCategory(const QString& label);
    QTreeWidgetItem* addRow(QTreeWidgetItem* category,
                            const QString& label,
                            const QString& value);

    // Model population helpers (phase 1/2).
    void populateModelIdentity(Model* m);
    void populateModelSizingChannels(Model* m);
    void populateModelLayout(Model* m);
    void populateModelAppearance(Model* m);
    void populateModelStringProperties(Model* m);
    void populateModelControllerConnection(Model* m);
    void populateModelAuxiliary(Model* m);

    // Per-model-type properties (phase 5).  Dispatches on m->GetDisplayAs().
    // Currently covers: Matrix, Arches, Tree, Star, Cube, CandyCanes,
    // WindowFrame, PolyLine, plus a generic "# Strings" row for the rest.
    void populateModelTypeProperties(Model* m);

    // Group population helpers (phase 7).
    void populateGroupIdentity(ModelGroup* g);
    void populateGroupBuffer(ModelGroup* g);
    void populateGroupBounds(ModelGroup* g);
    void populateGroupMembers(ModelGroup* g);
    void populateGroupAppearance(ModelGroup* g);

    // Controller population helpers (phase 7).
    void populateControllerIdentity(Controller* c);
    void populateControllerNetwork(Controller* c);
    void populateControllerOutput(Controller* c);
    void populateControllerCapabilities(Controller* c);
    void populateControllerOutputs(Controller* c);

    ModelManager*  _mm = nullptr;
    OutputManager* _om = nullptr;
};

#pragma once
#include "../Bridge/QtSequenceDoc.h"   // for QtModelGroupInfo fallback
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
// Built read-only in phase 1/2/3/5/6; phase 8 adds inline editors on a small
// set of "safe" fields (Description / Active / Tag Color / Pixel Size /
// Transparency / Black Transparency).  Further fields will be enabled once a
// persistence path to xlights_rgbeffects.xml is wired up.
//
// Data comes from live src-core objects (`Model*`, `ModelGroup*`, `Controller*`)
// reached through `QtRenderBridge::modelManager()` and
// `ModelManager::GetOutputManager()`.
class LayoutPropertyTree : public QTreeWidget {
    Q_OBJECT
public:
    // Per-row editor kind.  Stored in Qt::UserRole on the value column so the
    // delegate (LayoutPropertyDelegate) can pick the right editor widget.
    // Rows with kind == None are read-only.
    enum class Kind {
        None,
        String,         // QLineEdit
        Int,            // QSpinBox
        IntPercent,     // QSpinBox with " %" suffix, 0..100
        Double,         // QDoubleSpinBox
        Bool,           // QComboBox yes/no
        Color,          // QColorDialog
        Enum,           // QComboBox with options stored in Qt::UserRole+12 as QStringList
    };

    explicit LayoutPropertyTree(QWidget* parent = nullptr);

    // Provide the live ModelManager.  May be nullptr early on; the tree
    // simply clears itself in that case.
    void setModelManager(ModelManager* mm);

    // Replace contents with properties for the named model.
    // No-op if the model can't be found.
    void showModel(const QString& name);

    // Replace contents with properties for the named model group.
    // Tries the live ModelGroup* in ModelManager first; if that's missing
    // (bridge not initialised, group with unresolved members, etc.) falls
    // back to the supplied QtModelGroupInfo snapshot so the tree always
    // shows something for a known group name.
    void showGroup(const QString& name, const QtModelGroupInfo& fallback = {});

    // Replace contents with properties for the named controller.
    void showController(const QString& name);

    // Clear all rows.
    void clearAll();

signals:
    void editModelRequested(const QString& modelName, int tab);

    // Emitted after a successful edit so the caller can refresh the canvas /
    // preview AND persist to disk.  Models + groups both serialise into
    // xlights_rgbeffects.xml; controllers go to xlights_networks.xml.
    void modelChanged(const QString& modelName);
    void groupChanged(const QString& groupName);
    void controllerChanged(const QString& controllerName);

private:
    QTreeWidgetItem* addCategory(const QString& label);
    QTreeWidgetItem* addRow(QTreeWidgetItem* category,
                            const QString& label,
                            const QString& value);

    // Like addRow, but marks the value cell editable and stashes the kind
    // tag + field id on Qt::UserRole / UserRole+2.  `fieldId` is a free-form
    // string that the commit handler dispatches on.
    QTreeWidgetItem* addEditableRow(QTreeWidgetItem* category,
                                    const QString& label,
                                    const QString& value,
                                    Kind kind,
                                    const QString& fieldId,
                                    const QStringList& enumOptions = {});

    // Apply an editor commit to the currently-shown entity.  Returns true if
    // the change was understood (and the entity mutated).
    bool commitModelField(const QString& fieldId, const QVariant& value);
    bool commitGroupField(const QString& fieldId, const QVariant& value);
    bool commitControllerField(const QString& fieldId, const QVariant& value);

    // Model population helpers.
    void populateModelIdentity(Model* m);
    void populateModelSizingChannels(Model* m);
    void populateModelLayout(Model* m);
    void populateModelAppearance(Model* m);
    void populateModelStringProperties(Model* m);
    void populateModelControllerConnection(Model* m);
    void populateModelAuxiliary(Model* m);

    // Per-model-type properties (phase 5).
    void populateModelTypeProperties(Model* m);

    // Group population helpers (phase 7).
    void populateGroupIdentity(ModelGroup* g);
    void populateGroupBuffer(ModelGroup* g);
    void populateGroupBounds(ModelGroup* g);
    void populateGroupMembers(ModelGroup* g);
    void populateGroupAppearance(ModelGroup* g);

    // Fallback path when ModelManager doesn't have the live ModelGroup*
    // (e.g. unresolved members, bridge not yet initialised).  Populates a
    // smaller set of rows directly from the QtSequenceDoc snapshot.
    void populateGroupFromInfo(const QtModelGroupInfo& gi);

    // Controller population helpers (phase 7).
    void populateControllerIdentity(Controller* c);
    void populateControllerNetwork(Controller* c);
    void populateControllerOutput(Controller* c);
    void populateControllerCapabilities(Controller* c);
    void populateControllerOutputs(Controller* c);

    ModelManager*  _mm = nullptr;
    OutputManager* _om = nullptr;

    // True while a show*() call is building the tree.  The itemChanged
    // handler checks this and ignores changes during population — otherwise
    // a setText/setData/setBackground on an editable row fires itemChanged,
    // which would run a spurious commit + modelChanged → re-entrant
    // showModel → clear(), destroying the tree mid-build.
    bool _populating = false;

    // The currently displayed entity — used by the delegate-commit path to
    // know what to mutate without searching the tree each time.
    QString _currentEntity;
    enum class EntityKind { None, Model, Group, Controller } _currentKind = EntityKind::None;
};

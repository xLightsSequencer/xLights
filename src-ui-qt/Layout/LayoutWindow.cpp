#include "LayoutWindow.h"
#include "AddEntityDialogs.h"
#include "ModelEditDialog.h"
#include "ModelLayoutCanvas.h"
#include "LayoutPropertyTree.h"
#include "../App/QtXLightsApp.h"
#include "../Bridge/QtSequenceDoc.h"
#include "../Bridge/QtRenderBridge.h"
#include "../../src-core/models/ModelManager.h"
#include "../../src-core/models/Model.h"
#include "../../src-core/models/ModelGroup.h"
#include "../../src-core/outputs/OutputManager.h"
#include "../../src-core/outputs/Controller.h"
#include "../../src-core/outputs/ControllerEthernet.h"
#include "../../src-core/outputs/ControllerSerial.h"
#include "../../src-core/outputs/ControllerNull.h"

#include <QHeaderView>
#include <QListWidget>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QSplitter>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

LayoutWindow::LayoutWindow(QWidget* parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle("Layout");
    setMinimumSize(900, 600);
    resize(1200, 750);

    // ── Left panel ────────────────────────────────────────────────────────
    _modelList      = new QListWidget;
    _groupList      = new QListWidget;
    _controllerList = new QListWidget;

    // Helper: wrap a list widget in a vertical layout with a + / − toolbar
    // above.  Returns the wrapper QWidget; `addBtnOut` / `delBtnOut` receive
    // the toolbar buttons so callers can wire them.  The − button is
    // auto-disabled until a row is selected.
    auto makeTabWidget = [](QListWidget* list,
                            QToolButton** addBtnOut,
                            QToolButton** delBtnOut) -> QWidget* {
        auto* w = new QWidget;
        auto* v = new QVBoxLayout(w);
        v->setContentsMargins(0, 0, 0, 0);
        v->setSpacing(2);

        auto* tb = new QHBoxLayout;
        tb->setContentsMargins(2, 2, 2, 0);
        tb->setSpacing(4);
        auto* addBtn = new QToolButton; addBtn->setText("+"); addBtn->setToolTip("Add… (Insert)");
        auto* delBtn = new QToolButton; delBtn->setText(QString::fromUtf8("−")); delBtn->setToolTip("Delete (Delete)");
        delBtn->setEnabled(false);
        tb->addWidget(addBtn);
        tb->addWidget(delBtn);
        tb->addStretch(1);
        v->addLayout(tb);
        v->addWidget(list, 1);

        QObject::connect(list, &QListWidget::currentRowChanged, list,
                         [delBtn](int row) { delBtn->setEnabled(row >= 0); });

        *addBtnOut = addBtn;
        *delBtnOut = delBtn;
        return w;
    };

    QToolButton *modelAdd, *modelDel, *groupAdd, *groupDel, *ctrlAdd, *ctrlDel;
    QWidget* modelsTab      = makeTabWidget(_modelList,      &modelAdd, &modelDel);
    QWidget* groupsTab      = makeTabWidget(_groupList,      &groupAdd, &groupDel);
    QWidget* controllersTab = makeTabWidget(_controllerList, &ctrlAdd,  &ctrlDel);

    // Model list supports multi-select (Ctrl/Shift+click) — kept in sync
    // with the canvas's selection set so users can drag from either side.
    _modelList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    _tabs = new QTabWidget;
    _tabs->addTab(modelsTab,      "Models");
    _tabs->addTab(groupsTab,      "Groups");
    _tabs->addTab(controllersTab, "Controllers");

    connect(modelAdd, &QToolButton::clicked, this, &LayoutWindow::onAddModel);
    connect(modelDel, &QToolButton::clicked, this, &LayoutWindow::onDeleteModel);
    connect(groupAdd, &QToolButton::clicked, this, &LayoutWindow::onAddGroup);
    connect(groupDel, &QToolButton::clicked, this, &LayoutWindow::onDeleteGroup);
    connect(ctrlAdd,  &QToolButton::clicked, this, &LayoutWindow::onAddController);
    connect(ctrlDel,  &QToolButton::clicked, this, &LayoutWindow::onDeleteController);

    // Keyboard: Insert / Delete trigger the active tab's add / delete.
    auto* insShortcut = new QShortcut(QKeySequence(Qt::Key_Insert), this);
    connect(insShortcut, &QShortcut::activated, this, [this]() {
        switch (_tabs->currentIndex()) {
            case 0: onAddModel();      break;
            case 1: onAddGroup();      break;
            case 2: onAddController(); break;
        }
    });
    auto* delShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(delShortcut, &QShortcut::activated, this, [this]() {
        switch (_tabs->currentIndex()) {
            case 0: onDeleteModel();      break;
            case 1: onDeleteGroup();      break;
            case 2: onDeleteController(); break;
        }
    });

    // Single property tree for all three tabs.  Population branches on the
    // selected item type (model / group / controller); see show* methods.
    _props = new LayoutPropertyTree;
    _props->setMinimumHeight(160);

    auto* editBtn = new QPushButton("Edit Sub-Models / Faces / States…");
    editBtn->setEnabled(false);

    auto* vizBtn    = new QPushButton("Visualize");
    auto* uploadBtn = new QPushButton("Upload");
    vizBtn->setEnabled(false);
    uploadBtn->setEnabled(false);

    // Row of contextual buttons under the property panel.
    // editBtn: enabled when a model is selected.
    // vizBtn / uploadBtn: enabled when a controller is selected.
    auto* btnRow = new QHBoxLayout;
    btnRow->setContentsMargins(0, 0, 0, 0);
    btnRow->addWidget(editBtn);
    btnRow->addWidget(vizBtn);
    btnRow->addWidget(uploadBtn);

    // Wrap the property panel + buttons together in the left splitter.
    auto* propWidget = new QWidget;
    auto* propVL = new QVBoxLayout(propWidget);
    propVL->setContentsMargins(0, 0, 0, 0);
    propVL->setSpacing(2);
    propVL->addWidget(_props, 1);
    propVL->addLayout(btnRow);

    _leftSplit = new QSplitter(Qt::Vertical);
    _leftSplit->addWidget(_tabs);
    _leftSplit->addWidget(propWidget);
    _leftSplit->setSizes({380, 220});
    _leftSplit->setMinimumWidth(260);
    _leftSplit->setMaximumWidth(400);

    // ── Right panel: 2D canvas ────────────────────────────────────────────
    _canvas = new ModelLayoutCanvas;

    // ── Main horizontal splitter ──────────────────────────────────────────
    auto* mainSplit = new QSplitter(Qt::Horizontal, this);
    mainSplit->addWidget(_leftSplit);
    mainSplit->addWidget(_canvas);
    mainSplit->setSizes({300, 900});
    mainSplit->setStretchFactor(1, 1);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(mainSplit);

    // ── Connections ───────────────────────────────────────────────────────
    _editDialog = new ModelEditDialog(this);

    // Disable all contextual buttons and clear props when the tab changes.
    connect(_tabs, &QTabWidget::currentChanged, this,
            [editBtn, vizBtn, uploadBtn, this](int /*tab*/) {
        editBtn->setEnabled(false);
        vizBtn->setEnabled(false);
        uploadBtn->setEnabled(false);
        clearProps();
        _canvas->clearSelection();
        _canvas->setGroupHighlight({}, {});
    });

    // Enable edit button only when a model is selected (Models tab only).
    connect(_modelList, &QListWidget::currentRowChanged, this, [editBtn, this](int row) {
        editBtn->setEnabled(row >= 0 && _tabs->currentIndex() == 0);
    });
    connect(_modelList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item) _editDialog->openForModel(item->text(), _data);
    });
    connect(editBtn, &QPushButton::clicked, this, [this]() {
        auto* item = _modelList->currentItem();
        if (item) _editDialog->openForModel(item->text(), _data);
    });

    connect(_modelList,      &QListWidget::itemClicked,
            this, &LayoutWindow::onModelListClicked);
    connect(_groupList,      &QListWidget::itemClicked,
            this, &LayoutWindow::onGroupListClicked);
    connect(_controllerList, &QListWidget::itemClicked,
            this, &LayoutWindow::onControllerListClicked);

    // Enable controller buttons only when a controller is selected.
    connect(_controllerList, &QListWidget::currentRowChanged,
            this, [vizBtn, uploadBtn](int row) {
        vizBtn->setEnabled(row >= 0);
        uploadBtn->setEnabled(row >= 0);
    });
    connect(vizBtn, &QPushButton::clicked, this, [this]() {
        auto* item = _controllerList->currentItem();
        if (item) emit visualizerRequested(item->text());
    });
    connect(uploadBtn, &QPushButton::clicked, this, [this]() {
        auto* item = _controllerList->currentItem();
        if (item) emit uploadRequested(item->text());
    });
    connect(_canvas, &ModelLayoutCanvas::modelClicked,
            this, &LayoutWindow::onCanvasModelClicked);
    connect(_canvas, &ModelLayoutCanvas::placementClicked,
            this, &LayoutWindow::onPlacementClicked);
    connect(_canvas, &ModelLayoutCanvas::placementCancelled,
            this, &LayoutWindow::onPlacementCancelled);

    // Drag-to-move: ModelLayoutCanvas already mutated the dragged model's
    // world position; we just persist + refresh.  modelsDragged also fires
    // for multi-drags — we use it for the multi-persist path and let the
    // single-model modelDragged signal trigger nothing on its own.
    connect(_canvas, &ModelLayoutCanvas::modelsDragged,
            this, [this](const QStringList& names) {
        if (names.isEmpty() || !_bridge) return;
        for (const QString& n : names)
            _bridge->saveModelToShowFile(n);
        if (_bridge->modelManager())
            _canvas->loadLayoutFromManager(_bridge->modelManager());
        // Restore the multi-selection that was active during the drag.
        _canvas->setSelectedModels(names);
        // Sync the model-list widget too.
        _modelList->blockSignals(true);
        for (int i = 0; i < _modelList->count(); ++i) {
            auto* it = _modelList->item(i);
            it->setSelected(names.contains(it->text()));
        }
        _modelList->blockSignals(false);
        if (_props && !names.isEmpty()) _props->showModel(names.last());
        emit layoutChanged();
    });

    // Canvas-driven selection (Ctrl/Shift+click on the canvas) → mirror in
    // the model list widget.
    connect(_canvas, &ModelLayoutCanvas::selectionChanged,
            this, [this](const QStringList& names) {
        if (_tabs->currentIndex() != 0) _tabs->setCurrentIndex(0);
        _modelList->blockSignals(true);
        for (int i = 0; i < _modelList->count(); ++i) {
            auto* it = _modelList->item(i);
            it->setSelected(names.contains(it->text()));
        }
        _modelList->blockSignals(false);
        if (_props && !names.isEmpty()) _props->showModel(names.last());
    });

    // Model-list driven selection (Ctrl/Shift+click in the list) → mirror
    // in the canvas.
    connect(_modelList, &QListWidget::itemSelectionChanged, this, [this]() {
        if (_tabs->currentIndex() != 0) return;
        QStringList names;
        for (auto* item : _modelList->selectedItems()) names << item->text();
        _canvas->setSelectedModels(names);
    });

    // Property-tree → ModelEditDialog: open the right tab when the user
    // double-clicks the Sub-Models / Faces / States rows.
    connect(_props, &LayoutPropertyTree::editModelRequested,
            this, [this](const QString& name, int tab) {
        if (!_editDialog || name.isEmpty()) return;
        _editDialog->openForModel(name, _data, tab);
    });

    // Property-tree commit → persist to xlights_rgbeffects.xml + refresh
    // canvas + re-show props.  Save happens synchronously; if the file is
    // huge this could feel sluggish on slow disks — at that point we can
    // debounce on a QTimer like the renderer does.
    connect(_props, &LayoutPropertyTree::modelChanged,
            this, [this](const QString& name) {
        if (_bridge) _bridge->saveModelToShowFile(name);
        if (_bridge && _bridge->modelManager())
            _canvas->loadLayoutFromManager(_bridge->modelManager());
        _props->showModel(name);
        emit layoutChanged();
    });

    // Group edits persist to xlights_rgbeffects.xml exactly like models
    // (saveModelToShowFile handles both — it routes <modelGroup> by dynamic-
    // casting the live object).
    connect(_props, &LayoutPropertyTree::groupChanged,
            this, [this](const QString& name) {
        if (_bridge) _bridge->saveModelToShowFile(name);
        if (_bridge && _bridge->modelManager())
            _canvas->loadLayoutFromManager(_bridge->modelManager());
        _props->showGroup(name);
        emit layoutChanged();
    });

    // Controller edits persist to xlights_networks.xml via OutputManager::Save.
    // For renames the controller-list widget still shows the old name, so we
    // rebuild it from the live OutputManager and reselect the (possibly
    // renamed) entry by its current name.
    connect(_props, &LayoutPropertyTree::controllerChanged,
            this, [this](const QString& name) {
        if (_bridge) _bridge->saveControllersToShowFile();
        if (_bridge && _bridge->modelManager()) {
            if (auto* om = _bridge->modelManager()->GetOutputManager()) {
                _controllerList->clear();
                for (auto* c : om->GetControllers())
                    _controllerList->addItem(QString::fromStdString(c->GetName()));
                const auto matches = _controllerList->findItems(name, Qt::MatchExactly);
                if (!matches.isEmpty()) {
                    _controllerList->setCurrentItem(matches.first());
                    _controllerList->scrollToItem(matches.first());
                }
            }
        }
        _props->showController(name);
        emit layoutChanged();
    });
}

void LayoutWindow::setRenderBridge(QtRenderBridge* bridge) {
    _bridge = bridge;
    if (_props)
        _props->setModelManager(bridge ? bridge->modelManager() : nullptr);
}

// ── Refresh ───────────────────────────────────────────────────────────────────

void LayoutWindow::refresh() {
    // Prefer the live sequence; fall back to loading model definitions
    // directly from xlights_rgbeffects.xml when no sequence is open.
    const QtSequenceInfo& live = QtXLightsApp::instance().currentSequence();
    if (!live.models.isEmpty()) {
        _data = live;
    } else {
        _data = QtSequenceInfo{};
        const QString sf = QtXLightsApp::instance().showFolder();
        if (!sf.isEmpty())
            QtSequenceDoc::loadModels(sf + "/xlights_rgbeffects.xml", _data);
    }

    if (_data.models.isEmpty() && _data.controllers.isEmpty()) {
        _modelList->clear();
        _groupList->clear();
        _controllerList->clear();
        _canvas->loadLayoutFromManager(_bridge ? _bridge->modelManager() : nullptr);
        clearProps();
        return;
    }

    _modelList->clear();
    for (auto it = _data.models.constBegin(); it != _data.models.constEnd(); ++it)
        _modelList->addItem(it->name);
    _modelList->sortItems();

    _groupList->clear();
    for (auto it = _data.groups.constBegin(); it != _data.groups.constEnd(); ++it)
        _groupList->addItem(it->name);
    _groupList->sortItems();

    _controllerList->clear();
    for (const QtControllerInfo& c : _data.controllers)
        _controllerList->addItem(c.name);

    // Use src-core ModelManager for bounding-box geometry (Phase 18b).
    // Falls back to a dot-based layout from _data if the bridge is unavailable.
    if (_bridge && _bridge->modelManager())
        _canvas->loadLayoutFromManager(_bridge->modelManager());
    else
        _canvas->loadLayout(_data);

    // LayoutPropertyTree pulls live src-core objects via the bridge — refresh
    // its pointer in case the bridge initialised after our last
    // setRenderBridge() call.
    if (_props && _bridge)
        _props->setModelManager(_bridge->modelManager());

    clearProps();
}

// ── List clicks ───────────────────────────────────────────────────────────────

void LayoutWindow::onModelListClicked(QListWidgetItem* item) {
    if (!item) return;
    const QString name = item->text();
    _canvas->setSelectedModel(name);
    _canvas->setGroupHighlight({}, {});
    buildModelProps(name);
}

void LayoutWindow::onGroupListClicked(QListWidgetItem* item) {
    if (!item) return;
    const QString name = item->text();
    const QtModelGroupInfo& gi = _data.groups.value(name);
    _canvas->setGroupHighlight(name, gi.modelNames);
    _canvas->clearSelection();
    buildGroupProps(name);
}

void LayoutWindow::onControllerListClicked(QListWidgetItem* item) {
    if (!item) return;
    _canvas->clearSelection();
    _canvas->setGroupHighlight({}, {});
    buildControllerProps(item->text());
}

// ── Canvas click ──────────────────────────────────────────────────────────────

void LayoutWindow::onCanvasModelClicked(const QString& modelName) {
    _canvas->setSelectedModel(modelName);
    _canvas->setGroupHighlight({}, {});

    // Sync the list selection.
    const QList<QListWidgetItem*> matches =
        _modelList->findItems(modelName, Qt::MatchExactly);
    _modelList->clearSelection();
    if (!matches.isEmpty()) {
        _tabs->setCurrentIndex(0);   // switch to Models tab
        matches.first()->setSelected(true);
        _modelList->scrollToItem(matches.first());
    }

    buildModelProps(modelName);
}

// ── Property panel ────────────────────────────────────────────────────────────

void LayoutWindow::clearProps() {
    if (_props) _props->clearAll();
}

void LayoutWindow::buildModelProps(const QString& name) {
    if (_props) _props->showModel(name);
}

void LayoutWindow::buildGroupProps(const QString& name) {
    if (_props) _props->showGroup(name);
}

void LayoutWindow::buildControllerProps(const QString& name) {
    if (_props) _props->showController(name);
}

// ── Add / delete entities (Phase 20) ──────────────────────────────────────────

void LayoutWindow::rebuildModelLists() {
    ModelManager* mm = _bridge ? _bridge->modelManager() : nullptr;
    if (!mm) return;
    _modelList->clear();
    _groupList->clear();
    for (const auto& [name, m] : *mm) {
        const QString qname = QString::fromStdString(name);
        if (dynamic_cast<ModelGroup*>(m))
            _groupList->addItem(qname);
        else
            _modelList->addItem(qname);
    }
    _modelList->sortItems();
    _groupList->sortItems();
    _canvas->loadLayoutFromManager(mm);
}

void LayoutWindow::rebuildControllerList() {
    if (!_bridge || !_bridge->modelManager()) return;
    auto* om = _bridge->modelManager()->GetOutputManager();
    if (!om) return;
    _controllerList->clear();
    for (auto* c : om->GetControllers())
        _controllerList->addItem(QString::fromStdString(c->GetName()));
}

void LayoutWindow::onAddModel() {
    ModelManager* mm = _bridge ? _bridge->modelManager() : nullptr;
    if (!mm) {
        QMessageBox::warning(this, "Add Model",
                             "Render bridge not initialised — open a show folder first.");
        return;
    }
    AddModelDialog dlg(mm, this);
    if (dlg.exec() != QDialog::Accepted) return;

    const QString name = dlg.modelName();
    if (name.isEmpty()) return;
    if (mm->GetModel(name.toStdString()) != nullptr) {
        QMessageBox::warning(this, "Add Model",
                             "A model named '" + name + "' already exists.");
        return;
    }

    // Defer creation until the user clicks on the canvas so the new model
    // lands where they want it (Phase 20f).  Escape cancels.
    _pendingModel.active       = true;
    _pendingModel.type         = dlg.typeName();
    _pendingModel.name         = name;
    _pendingModel.layoutGroup  = dlg.layoutGroup();
    _pendingModel.startChannel = dlg.startChannel();
    _canvas->setPlacementMode(true);
    setWindowTitle("Layout — click to place '" + name + "'  (Esc cancels)");
}

void LayoutWindow::onPlacementClicked(double wx, double wy) {
    if (!_pendingModel.active) return;
    PendingModel p = _pendingModel;
    _pendingModel = {};                     // consume — single-shot
    setWindowTitle("Layout");

    ModelManager* mm = _bridge ? _bridge->modelManager() : nullptr;
    if (!mm) return;

    Model* m = mm->CreateDefaultModel(p.type.toStdString(),
                                      p.startChannel.toStdString());
    if (!m) {
        QMessageBox::warning(this, "Add Model",
                             "src-core couldn't create a default '" + p.type + "' model.");
        return;
    }
    m->Rename(p.name.toStdString());
    if (!p.layoutGroup.isEmpty())
        m->SetLayoutGroup(p.layoutGroup.toStdString());
    // Position the new model at the click point.  Z stays at the default 0.
    m->GetModelScreenLocation().SetWorldPos(
        static_cast<float>(wx), static_cast<float>(wy), 0.0f);
    mm->AddModel(m);

    if (_bridge) _bridge->saveModelToShowFile(p.name);
    rebuildModelLists();
    _tabs->setCurrentIndex(0);
    const auto matches = _modelList->findItems(p.name, Qt::MatchExactly);
    if (!matches.isEmpty()) {
        _modelList->setCurrentItem(matches.first());
        _modelList->scrollToItem(matches.first());
    }
    buildModelProps(p.name);
    emit layoutChanged();
}

void LayoutWindow::onPlacementCancelled() {
    _pendingModel = {};
    setWindowTitle("Layout");
}

void LayoutWindow::onAddGroup() {
    ModelManager* mm = _bridge ? _bridge->modelManager() : nullptr;
    if (!mm) {
        QMessageBox::warning(this, "Add Group",
                             "Render bridge not initialised — open a show folder first.");
        return;
    }
    AddGroupDialog dlg(mm, this);
    if (dlg.exec() != QDialog::Accepted) return;

    const QString name = dlg.groupName();
    if (name.isEmpty()) return;
    if (mm->GetModel(name.toStdString()) != nullptr) {
        QMessageBox::warning(this, "Add Group",
                             "A model or group named '" + name + "' already exists.");
        return;
    }

    auto* g = new ModelGroup(*mm);
    g->Rename(name.toStdString());
    g->SetLayout(dlg.layout().toStdString());
    if (!dlg.layoutGroup().isEmpty())
        g->SetLayoutGroup(dlg.layoutGroup().toStdString());
    std::vector<std::string> members;
    for (const auto& n : dlg.members()) members.push_back(n.toStdString());
    g->SetModels(members);
    mm->AddModel(g);

    if (_bridge) _bridge->saveModelToShowFile(name);
    rebuildModelLists();
    _tabs->setCurrentIndex(1);
    const auto matches = _groupList->findItems(name, Qt::MatchExactly);
    if (!matches.isEmpty()) {
        _groupList->setCurrentItem(matches.first());
        _groupList->scrollToItem(matches.first());
    }
    buildGroupProps(name);
    emit layoutChanged();
}

void LayoutWindow::onAddController() {
    if (!_bridge || !_bridge->modelManager()) {
        QMessageBox::warning(this, "Add Controller",
                             "Render bridge not initialised — open a show folder first.");
        return;
    }
    auto* om = _bridge->modelManager()->GetOutputManager();
    if (!om) return;

    AddControllerDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    const QString name = dlg.controllerName();
    if (name.isEmpty()) return;
    if (om->GetController(name.toStdString()) != nullptr) {
        QMessageBox::warning(this, "Add Controller",
                             "A controller named '" + name + "' already exists.");
        return;
    }

    Controller* c = nullptr;
    switch (dlg.controllerType()) {
        case AddControllerDialog::Type::Ethernet: {
            auto* e = new ControllerEthernet(om, /*acceptDuplicates*/false);
            if (!dlg.protocol().isEmpty())
                e->SetProtocol(dlg.protocol().toStdString());
            if (!dlg.ip().isEmpty())
                e->SetIP(dlg.ip().toStdString());
            c = e;
            break;
        }
        case AddControllerDialog::Type::Serial: {
            auto* s = new ControllerSerial(om);
            if (!dlg.port().isEmpty())
                s->SetPort(dlg.port().toStdString());
            if (dlg.speed() > 0)
                s->SetSpeed(dlg.speed());
            c = s;
            break;
        }
        case AddControllerDialog::Type::Null:
            c = new ControllerNull(om);
            break;
    }
    if (!c) return;

    c->SetName(name.toStdString());
    if (!dlg.vendor().isEmpty())  c->SetVendor(dlg.vendor().toStdString());
    if (!dlg.model().isEmpty())   c->SetModel(dlg.model().toStdString());
    if (!dlg.variant().isEmpty()) c->SetVariant(dlg.variant().toStdString());
    om->AddController(c, /*pos=*/-1);

    if (_bridge) _bridge->saveControllersToShowFile();
    rebuildControllerList();
    _tabs->setCurrentIndex(2);
    const auto matches = _controllerList->findItems(name, Qt::MatchExactly);
    if (!matches.isEmpty()) {
        _controllerList->setCurrentItem(matches.first());
        _controllerList->scrollToItem(matches.first());
    }
    buildControllerProps(name);
    emit layoutChanged();
}

void LayoutWindow::onDeleteModel() {
    auto* item = _modelList->currentItem();
    if (!item) return;
    const QString name = item->text();
    if (QMessageBox::question(this, "Delete Model",
            "Delete model '" + name + "'?  This cannot be undone.",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;
    auto* mm = _bridge ? _bridge->modelManager() : nullptr;
    if (!mm) return;
    mm->Delete(name.toStdString());
    if (_bridge) _bridge->removeModelFromShowFile(name);
    rebuildModelLists();
    clearProps();
    emit layoutChanged();
}

void LayoutWindow::onDeleteGroup() {
    auto* item = _groupList->currentItem();
    if (!item) return;
    const QString name = item->text();
    if (QMessageBox::question(this, "Delete Group",
            "Delete group '" + name + "'?  Members will be unaffected.",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;
    auto* mm = _bridge ? _bridge->modelManager() : nullptr;
    if (!mm) return;
    mm->Delete(name.toStdString());
    if (_bridge) _bridge->removeModelFromShowFile(name);
    rebuildModelLists();
    clearProps();
    emit layoutChanged();
}

void LayoutWindow::onDeleteController() {
    auto* item = _controllerList->currentItem();
    if (!item) return;
    const QString name = item->text();
    if (QMessageBox::question(this, "Delete Controller",
            "Delete controller '" + name +
            "'?  Models referencing it will have an empty Controller field.",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;
    auto* om = (_bridge && _bridge->modelManager())
                 ? _bridge->modelManager()->GetOutputManager() : nullptr;
    if (!om) return;
    om->DeleteController(name.toStdString());
    if (_bridge) _bridge->saveControllersToShowFile();
    rebuildControllerList();
    clearProps();
    emit layoutChanged();
}

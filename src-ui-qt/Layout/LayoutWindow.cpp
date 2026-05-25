#include "LayoutWindow.h"
#include "ModelEditDialog.h"
#include "ModelLayoutCanvas.h"
#include "LayoutPropertyTree.h"
#include "../App/QtXLightsApp.h"
#include "../Bridge/QtSequenceDoc.h"
#include "../Bridge/QtRenderBridge.h"
#include "../../src-core/models/ModelManager.h"

#include <QHeaderView>
#include <QListWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSplitter>
#include <QTabWidget>
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

    _tabs = new QTabWidget;
    _tabs->addTab(_modelList,      "Models");
    _tabs->addTab(_groupList,      "Groups");
    _tabs->addTab(_controllerList, "Controllers");

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

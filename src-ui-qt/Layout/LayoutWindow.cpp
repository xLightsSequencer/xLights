#include "LayoutWindow.h"
#include "ModelEditDialog.h"
#include "ModelLayoutCanvas.h"
#include "../App/QtXLightsApp.h"
#include "../Bridge/QtSequenceDoc.h"

#include <QHeaderView>
#include <QListWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSplitter>
#include <QTabWidget>
#include <QTableWidget>
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

    _propTable = new QTableWidget(0, 2);
    _propTable->setHorizontalHeaderLabels({"Property", "Value"});
    _propTable->horizontalHeader()->setStretchLastSection(true);
    _propTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    _propTable->verticalHeader()->hide();
    _propTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _propTable->setSelectionMode(QAbstractItemView::NoSelection);
    _propTable->setMinimumHeight(160);

    auto* editBtn = new QPushButton("Edit Sub-Models / Faces / States…");
    editBtn->setEnabled(false);

    auto* vizBtn    = new QPushButton("Visualize");
    auto* uploadBtn = new QPushButton("Upload");
    vizBtn->setEnabled(false);
    uploadBtn->setEnabled(false);

    // Row of contextual buttons under the properties table.
    // editBtn: enabled when a model is selected.
    // vizBtn / uploadBtn: enabled when a controller is selected.
    auto* btnRow = new QHBoxLayout;
    btnRow->setContentsMargins(0, 0, 0, 0);
    btnRow->addWidget(editBtn);
    btnRow->addWidget(vizBtn);
    btnRow->addWidget(uploadBtn);

    // Wrap the properties table and buttons together in the left splitter.
    auto* propWidget = new QWidget;
    auto* propVL = new QVBoxLayout(propWidget);
    propVL->setContentsMargins(0, 0, 0, 0);
    propVL->setSpacing(2);
    propVL->addWidget(_propTable, 1);
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
    connect(_tabs, &QTabWidget::currentChanged, this, [editBtn, vizBtn, uploadBtn, this](int tab) {
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
        _canvas->loadLayout(QtSequenceInfo{});
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

    _canvas->loadLayout(_data);
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
    _propTable->setRowCount(0);
}

void LayoutWindow::setProps(const QList<QPair<QString,QString>>& rows) {
    _propTable->setRowCount(rows.size());
    for (int i = 0; i < rows.size(); ++i) {
        _propTable->setItem(i, 0, new QTableWidgetItem(rows[i].first));
        _propTable->setItem(i, 1, new QTableWidgetItem(rows[i].second));
    }
}

void LayoutWindow::buildModelProps(const QString& name) {
    const QtModelInfo& m = _data.models.value(name);
    if (m.name.isEmpty()) { clearProps(); return; }

    QList<QPair<QString,QString>> rows = {
        { "Name",          m.name },
        { "Type",          m.type },
        { "Nodes",         QString::number(m.nodeCount) },
        { "Parm1",         QString::number(m.parm1) },
        { "Parm2",         QString::number(m.parm2) },
        { "Buffer W",      QString::number(m.bufferW) },
        { "Buffer H",      QString::number(m.bufferH) },
        { "Start Channel", QString::number(m.startChannel) },
        { "World X",       QString::number(m.worldPosX, 'f', 2) },
        { "World Y",       QString::number(m.worldPosY, 'f', 2) },
        { "Scale X",       QString::number(m.scaleX,    'f', 3) },
        { "Scale Y",       QString::number(m.scaleY,    'f', 3) },
    };
    if (!m.controllerName.isEmpty()) {
        rows.append({ "Controller", m.controllerName });
        if (m.controllerPort > 0)
            rows.append({ "Port", QString::number(m.controllerPort) });
    }
    setProps(rows);
}

void LayoutWindow::buildGroupProps(const QString& name) {
    const QtModelGroupInfo& g = _data.groups.value(name);
    if (g.name.isEmpty()) { clearProps(); return; }

    setProps({
        { "Name",     g.name },
        { "Layout",   g.layout },
        { "Members",  QString::number(g.modelNames.size()) },
        { "Buffer W", QString::number(g.bufferW) },
        { "Buffer H", QString::number(g.bufferH) },
        { "Bounds X", QString("%1 – %2").arg(g.minX, 0, 'f', 1).arg(g.maxX, 0, 'f', 1) },
        { "Bounds Y", QString("%1 – %2").arg(g.minY, 0, 'f', 1).arg(g.maxY, 0, 'f', 1) },
        { "Models",   g.modelNames.join(", ") },
    });
}

void LayoutWindow::buildControllerProps(const QString& name) {
    for (const QtControllerInfo& c : _data.controllers) {
        if (c.name != name) continue;
        QList<QPair<QString,QString>> rows = {
            { "Name",          c.name },
            { "Type",          c.type },
            { "IP / Port",     c.ip },
            { "Protocol",      c.protocol },
            { "Start Channel", QString::number(c.startChannel) },
            { "Channels",      QString::number(c.channelCount) },
        };
        if (c.pixelPortCount > 0)
            rows.append({ "Pixel Ports", QString::number(c.pixelPortCount) });
        if (c.serialPortCount > 0)
            rows.append({ "Serial Ports", QString::number(c.serialPortCount) });
        setProps(rows);
        return;
    }
    clearProps();
}

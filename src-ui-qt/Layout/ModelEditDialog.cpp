#include "ModelEditDialog.h"
#include "ModelNodePreview.h"
#include "../App/QtXLightsApp.h"

#include <pugixml.hpp>
#include <spdlog/spdlog.h>

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QTableWidget>
#include <QTabWidget>
#include <QVBoxLayout>

// ── Constants ─────────────────────────────────────────────────────────────────

static const QStringList kPhonemeKeys = {
    "FaceOutline",
    "Mouth_AI", "Mouth_E", "Mouth_etc", "Mouth_FV", "Mouth_L",
    "Mouth_MBP", "Mouth_O", "Mouth_rest", "Mouth_U", "Mouth_WQ",
    "Eyes_Open", "Eyes_Closed"
};
static const QStringList kMatrixImageKeys = {
    "Eyes_Open", "Eyes_Closed", "Mouth"
};
static const QString kColorSuffix = "Color";

// ── Color-swatch helpers ──────────────────────────────────────────────────────

static QColor parseColor(const QString& s) {
    if (s.isEmpty()) return {};
    QColor c(s); return c.isValid() ? c : QColor();
}

static QPushButton* makeSwatchBtn(const QString& colorStr, QWidget* parent) {
    auto* btn = new QPushButton(parent);
    btn->setFlat(true); btn->setFixedSize(40, 20);
    const QColor c   = parseColor(colorStr);
    const QString hex = c.isValid() ? c.name() : "#000000";
    btn->setProperty("colorHex", hex);
    btn->setStyleSheet(QString("background-color:%1;border:1px solid #666;").arg(hex));
    QObject::connect(btn, &QPushButton::clicked, [btn, parent]() {
        const QColor cur(btn->property("colorHex").toString());
        const QColor chosen = QColorDialog::getColor(cur.isValid() ? cur : Qt::black, parent);
        if (chosen.isValid()) {
            btn->setProperty("colorHex", chosen.name());
            btn->setStyleSheet(QString("background-color:%1;border:1px solid #666;").arg(chosen.name()));
        }
    });
    return btn;
}

static QString swatchColor(QWidget* w) {
    return w ? w->property("colorHex").toString() : QString();
}

// ── Node-range utilities ──────────────────────────────────────────────────────

QList<int> ModelEditDialog::parseRangeStr(const QString& s) const {
    QList<int> result;
    if (s.isEmpty()) return result;
    for (const QString& part : s.split(',', Qt::SkipEmptyParts)) {
        const int dash = part.indexOf('-');
        if (dash > 0) {
            const int from = part.left(dash).trimmed().toInt() - 1;
            const int to   = part.mid(dash + 1).trimmed().toInt() - 1;
            for (int i = from; i <= to; ++i)
                if (i >= 0 && i < _nodeCount) result.append(i);
        } else {
            const int n = part.trimmed().toInt() - 1;
            if (n >= 0 && n < _nodeCount) result.append(n);
        }
    }
    return result;
}

QString ModelEditDialog::indicesToRangeStr(const QList<int>& idx) const {
    if (idx.isEmpty()) return {};
    QList<int> sorted = idx;
    std::sort(sorted.begin(), sorted.end());
    QString result;
    int start = sorted[0], end = sorted[0];
    auto flush = [&] {
        if (!result.isEmpty()) result += ',';
        if (start == end) result += QString::number(start + 1);
        else result += QString("%1-%2").arg(start + 1).arg(end + 1);
    };
    for (int i = 1; i < sorted.size(); ++i) {
        if (sorted[i] == end + 1) { end = sorted[i]; }
        else { flush(); start = end = sorted[i]; }
    }
    flush();
    return result;
}

QString ModelEditDialog::indicesToSingleNodeStr(const QList<int>& idx) const {
    return idx.isEmpty() ? QString() : QString::number(idx.first() + 1);
}

// Parse the key part of a state entry (e.g. "s001" → 0, "s001-s005" → 0..4).
static QList<int> parseStateKey(const QString& key, int nodeCount) {
    QList<int> result;
    // Strip leading 's' and possible second 's' in range.
    QString k = key;
    if (k.startsWith('s') || k.startsWith('S')) k = k.mid(1);
    const int dash = k.indexOf('-');
    if (dash > 0) {
        QString right = k.mid(dash + 1);
        if (right.startsWith('s') || right.startsWith('S')) right = right.mid(1);
        const int from = k.left(dash).toInt() - 1;
        const int to   = right.toInt() - 1;
        for (int i = from; i <= to && i < nodeCount; ++i)
            if (i >= 0) result.append(i);
    } else {
        const int n = k.toInt() - 1;
        if (n >= 0 && n < nodeCount) result.append(n);
    }
    return result;
}

// ── Constructor ───────────────────────────────────────────────────────────────

ModelEditDialog::ModelEditDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Model Editor");
    setMinimumSize(900, 580);
    resize(1100, 680);

    _titleLabel = new QLabel("No model selected");
    QFont f = _titleLabel->font(); f.setBold(true); f.setPointSize(f.pointSize() + 1);
    _titleLabel->setFont(f);

    _tabs = new QTabWidget;
    auto* smTab    = new QWidget; setupSubModelsTab(smTab);
    auto* faceTab  = new QWidget; setupFacesTab(faceTab);
    auto* stateTab = new QWidget; setupStatesTab(stateTab);
    _tabs->addTab(smTab,    "Sub-Models");
    _tabs->addTab(faceTab,  "Faces");
    _tabs->addTab(stateTab, "States");

    connect(_tabs, &QTabWidget::currentChanged, this, [this](int idx) {
        _activeTab = idx;
        _preview->clearHighlight();
    });

    // ── Preview panel ─────────────────────────────────────────────────────
    _preview = new ModelNodePreview;
    auto* prevLabel = new QLabel("Model Preview");
    prevLabel->setStyleSheet("font-weight:bold;color:#aaa;");
    auto* prevHint  = new QLabel("Drag to lasso-select nodes");
    prevHint->setStyleSheet("color:#666;font-size:10px;");
    auto* prevPanel = new QWidget;
    auto* prevVL    = new QVBoxLayout(prevPanel);
    prevVL->setContentsMargins(0,0,0,0);
    prevVL->addWidget(prevLabel);
    prevVL->addWidget(_preview, 1);
    prevVL->addWidget(prevHint);
    prevPanel->setMinimumWidth(180);

    connect(_preview, &ModelNodePreview::nodesLassoed,
            this, &ModelEditDialog::onNodesLassoed);

    // ── Main splitter: tabs | preview ─────────────────────────────────────
    auto* mainSplit = new QSplitter(Qt::Horizontal);
    mainSplit->addWidget(_tabs);
    mainSplit->addWidget(prevPanel);
    mainSplit->setSizes({700, 240});
    mainSplit->setStretchFactor(0, 1);

    auto* buttons  = new QDialogButtonBox;
    auto* saveBtn  = buttons->addButton("Save",  QDialogButtonBox::AcceptRole);
    auto* closeBtn = buttons->addButton("Close", QDialogButtonBox::RejectRole);
    connect(saveBtn,  &QPushButton::clicked, this, &ModelEditDialog::onSave);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    root->addWidget(_titleLabel);
    root->addWidget(mainSplit, 1);
    root->addWidget(buttons);
}

// ── Tab setup ─────────────────────────────────────────────────────────────────

static QWidget* makeListPanel(QListWidget*& list, QObject* recv,
                               const char* addSlot, const char* delSlot) {
    list = new QListWidget;
    auto* addBtn = new QPushButton("+");
    auto* delBtn = new QPushButton("–");
    addBtn->setFixedWidth(28); delBtn->setFixedWidth(28);
    QObject::connect(addBtn, SIGNAL(clicked()), recv, addSlot);
    QObject::connect(delBtn, SIGNAL(clicked()), recv, delSlot);
    auto* btnRow = new QHBoxLayout;
    btnRow->addWidget(addBtn); btnRow->addWidget(delBtn); btnRow->addStretch();
    auto* w  = new QWidget;
    auto* vl = new QVBoxLayout(w);
    vl->setContentsMargins(0,0,0,0);
    vl->addWidget(list, 1);
    vl->addLayout(btnRow);
    return w;
}

static QSplitter* makeTabSplit(QWidget* listPanel, QWidget* editor) {
    auto* s = new QSplitter(Qt::Horizontal);
    s->addWidget(listPanel); s->addWidget(editor);
    s->setSizes({200, 600}); s->setStretchFactor(1, 1);
    return s;
}

void ModelEditDialog::setupSubModelsTab(QWidget* tab) {
    auto* listPanel = makeListPanel(_smList, this,
                                    SLOT(onSmAdd()), SLOT(onSmDelete()));

    _smNameEdit    = new QLineEdit;
    _smLayout      = new QComboBox;  _smLayout->addItems({"horizontal","vertical"});
    _smType        = new QComboBox;  _smType->addItems({"ranges","subbuffer"});
    _smBufferStyle = new QComboBox;  _smBufferStyle->addItems({"Default","Keep XY","Stacked Strands"});

    auto* form = new QFormLayout;
    form->addRow("Name:",         _smNameEdit);
    form->addRow("Layout:",       _smLayout);
    form->addRow("Type:",         _smType);
    form->addRow("Buffer style:", _smBufferStyle);

    _smRanges = new QTableWidget(0, 1);
    _smRanges->setHorizontalHeaderLabels({"Node Ranges (e.g. 1-10)"});
    _smRanges->horizontalHeader()->setStretchLastSection(true);
    _smRanges->verticalHeader()->hide();

    auto* rAdd = new QPushButton("+ Row");
    auto* rDel = new QPushButton("– Row");
    connect(rAdd, &QPushButton::clicked, this, &ModelEditDialog::onSmRangeAdd);
    connect(rDel, &QPushButton::clicked, this, &ModelEditDialog::onSmRangeDelete);
    auto* rBtns = new QHBoxLayout;
    rBtns->addWidget(rAdd); rBtns->addWidget(rDel); rBtns->addStretch();

    auto* ed  = new QWidget;
    auto* edVL = new QVBoxLayout(ed);
    edVL->addLayout(form);
    edVL->addWidget(new QLabel("Ranges (one per row):"));
    edVL->addWidget(_smRanges, 1);
    edVL->addLayout(rBtns);

    auto* vl = new QVBoxLayout(tab);
    vl->setContentsMargins(4,4,4,4);
    vl->addWidget(makeTabSplit(listPanel, ed));

    connect(_smList, &QListWidget::currentRowChanged, this, [this](int) { onSmSelectionChanged(); });
    connect(_smRanges, &QTableWidget::currentCellChanged,
            this, [this](int row, int col, int, int) { onSmRangeCellChanged(row, col); });
    connect(_smRanges, &QTableWidget::itemChanged,
            this, [this](QTableWidgetItem*) {
        previewFromSmRow(_smRanges->currentRow());
    });
    connect(_smNameEdit,    &QLineEdit::textEdited, this, &ModelEditDialog::onSmNameEdited);
    connect(_smBufferStyle, &QComboBox::currentTextChanged, this, &ModelEditDialog::onSmBufferStyleChanged);
}

void ModelEditDialog::setupFacesTab(QWidget* tab) {
    auto* listPanel = makeListPanel(_faceList, this,
                                    SLOT(onFaceAdd()), SLOT(onFaceDelete()));

    _faceType = new QComboBox;
    _faceType->addItems({"NodeRange", "SingleNode", "Matrix"});

    _faceTable = new QTableWidget(0, 3);
    _faceTable->setHorizontalHeaderLabels({"Feature / Phoneme", "Nodes", "Color"});
    _faceTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    _faceTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    _faceTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    _faceTable->verticalHeader()->hide();

    auto* ed  = new QWidget;
    auto* edVL = new QVBoxLayout(ed);
    auto* typeRow = new QHBoxLayout;
    typeRow->addWidget(new QLabel("Type:")); typeRow->addWidget(_faceType); typeRow->addStretch();
    edVL->addLayout(typeRow);
    edVL->addWidget(_faceTable, 1);

    auto* vl = new QVBoxLayout(tab);
    vl->setContentsMargins(4,4,4,4);
    vl->addWidget(makeTabSplit(listPanel, ed));

    connect(_faceList, &QListWidget::currentRowChanged, this, [this](int) { onFaceSelectionChanged(); });
    connect(_faceType, &QComboBox::currentTextChanged,  this, &ModelEditDialog::onFaceTypeChanged);
    connect(_faceTable, &QTableWidget::currentCellChanged,
            this, [this](int row, int col, int, int) { onFaceCellChanged(row, col); });
}

void ModelEditDialog::setupStatesTab(QWidget* tab) {
    auto* listPanel = makeListPanel(_stateList, this,
                                    SLOT(onStateAdd()), SLOT(onStateDelete()));

    _stateType = new QComboBox;
    _stateType->addItems({"NodeRange", "SingleNode"});

    _stateTable = new QTableWidget(0, 3);
    _stateTable->setHorizontalHeaderLabels({"Key", "Nodes", "Color"});
    _stateTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    _stateTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    _stateTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    _stateTable->verticalHeader()->hide();

    auto* seAdd = new QPushButton("+ Row");
    auto* seDel = new QPushButton("– Row");
    connect(seAdd, &QPushButton::clicked, this, &ModelEditDialog::onStateEntryAdd);
    connect(seDel, &QPushButton::clicked, this, &ModelEditDialog::onStateEntryDelete);
    auto* seBtns = new QHBoxLayout;
    seBtns->addWidget(seAdd); seBtns->addWidget(seDel); seBtns->addStretch();

    auto* ed  = new QWidget;
    auto* edVL = new QVBoxLayout(ed);
    auto* typeRow = new QHBoxLayout;
    typeRow->addWidget(new QLabel("Type:")); typeRow->addWidget(_stateType); typeRow->addStretch();
    edVL->addLayout(typeRow);
    edVL->addWidget(_stateTable, 1);
    edVL->addLayout(seBtns);

    auto* vl = new QVBoxLayout(tab);
    vl->setContentsMargins(4,4,4,4);
    vl->addWidget(makeTabSplit(listPanel, ed));

    connect(_stateList, &QListWidget::currentRowChanged, this, [this](int) { onStateSelectionChanged(); });
    connect(_stateType, &QComboBox::currentTextChanged,  this, &ModelEditDialog::onStateTypeChanged);
    connect(_stateTable, &QTableWidget::currentCellChanged,
            this, [this](int row, int col, int, int) { onStateCellChanged(row, col); });
}

// ── openForModel ──────────────────────────────────────────────────────────────

void ModelEditDialog::openForModel(const QString& modelName, const QtSequenceInfo& seqData) {
    _modelName = modelName;
    _titleLabel->setText(modelName);
    setWindowTitle("Model Editor — " + modelName);

    const QtModelInfo& mi = seqData.modelInfo(modelName);
    _nodeCount = mi.nodeCount > 0 ? mi.nodeCount : mi.bufferW * mi.bufferH;
    _subModels = mi.subModels;
    _faces     = mi.faces;
    _states    = mi.states;

    _preview->setNodePositions(mi.nodePositions);

    _curSm = _curFace = _curState = -1;
    refreshSmList();
    refreshFaceList();
    refreshStateList();

    show(); raise(); activateWindow();
}

// ── List refresh ──────────────────────────────────────────────────────────────

void ModelEditDialog::refreshSmList() {
    _smList->blockSignals(true);
    _smList->clear();
    for (const auto& sm : _subModels)
        _smList->addItem(sm.name.isEmpty() ? "(unnamed)" : sm.name);
    _smList->blockSignals(false);
    populateSmEditor(-1);
}

void ModelEditDialog::refreshFaceList() {
    _faceList->blockSignals(true);
    _faceList->clear();
    for (const auto& fi : _faces)
        _faceList->addItem(fi.name.isEmpty() ? "(unnamed)" : fi.name);
    _faceList->blockSignals(false);
    populateFaceEditor(-1);
}

void ModelEditDialog::refreshStateList() {
    _stateList->blockSignals(true);
    _stateList->clear();
    for (const auto& si : _states)
        _stateList->addItem(si.name.isEmpty() ? "(unnamed)" : si.name);
    _stateList->blockSignals(false);
    populateStateEditor(-1);
}

// ── Sub-model editor ──────────────────────────────────────────────────────────

void ModelEditDialog::populateSmEditor(int idx) {
    const bool valid = idx >= 0 && idx < _subModels.size();
    _smNameEdit->setEnabled(valid);
    _smLayout->setEnabled(valid);
    _smType->setEnabled(valid);
    _smBufferStyle->setEnabled(valid);
    _smRanges->setEnabled(valid);
    if (!valid) {
        _smNameEdit->clear();
        _smRanges->setUpdatesEnabled(false);
        _smRanges->setRowCount(0);
        _smRanges->setUpdatesEnabled(true);
        _preview->clearHighlight();
        return;
    }

    const QtSubModelInfo& sm = _subModels[idx];

    _smNameEdit->blockSignals(true); _smNameEdit->setText(sm.name); _smNameEdit->blockSignals(false);

    auto setCombo = [](QComboBox* cb, const QString& v) {
        cb->blockSignals(true); int i = cb->findText(v); cb->setCurrentIndex(i>=0?i:0); cb->blockSignals(false);
    };
    setCombo(_smLayout, sm.layout); setCombo(_smType, sm.type); setCombo(_smBufferStyle, sm.bufferStyle);

    _smRanges->setUpdatesEnabled(false);
    _smRanges->blockSignals(true);
    _smRanges->setRowCount(sm.ranges.size());
    for (int r = 0; r < sm.ranges.size(); ++r)
        _smRanges->setItem(r, 0, new QTableWidgetItem(sm.ranges[r]));
    _smRanges->blockSignals(false);
    _smRanges->setUpdatesEnabled(true);

    _preview->clearHighlight();
}

// ── Face editor ───────────────────────────────────────────────────────────────

void ModelEditDialog::populateFaceEditor(int idx) {
    const bool valid = idx >= 0 && idx < _faces.size();
    _faceType->setEnabled(valid);
    _faceTable->setEnabled(valid);
    _faceTable->setUpdatesEnabled(false);
    _faceTable->setRowCount(0);
    _faceTable->setUpdatesEnabled(true);
    if (!valid) { _preview->clearHighlight(); return; }

    const QtFaceInfo& fi = _faces[idx];
    _faceType->blockSignals(true);
    _faceType->setCurrentIndex(qMax(0, _faceType->findText(fi.type)));
    _faceType->blockSignals(false);

    rebuildFaceNodeCells(fi);
    _preview->clearHighlight();
}

void ModelEditDialog::rebuildFaceNodeCells(const QtFaceInfo& fi) {
    _faceTable->setUpdatesEnabled(false);
    _faceTable->blockSignals(true);
    _faceTable->setRowCount(0);

    if (fi.type == "Matrix") {
        // Image-path mode: show file-picker rows for matrix image keys.
        _faceTable->setColumnCount(3);
        _faceTable->setHorizontalHeaderLabels({"Feature", "Image Path", ""});
        _faceTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        _faceTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        // ResizeToContents for the browse-button column — avoids a Qt debug
        // assert when setColumnWidth() is called with a value below the DPI-scaled
        // minimum section size (e.g., 28px < ~30px at 125% DPI on Windows 11).
        _faceTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

        QStringList keys = kMatrixImageKeys;
        for (auto it = fi.attrs.constBegin(); it != fi.attrs.constEnd(); ++it) {
            const QString& k = it.key();
            if (k == "Name" || k == "Type" || k.endsWith(kColorSuffix)) continue;
            if (!keys.contains(k)) keys.append(k);
        }

        _faceTable->setRowCount(keys.size());
        for (int r = 0; r < keys.size(); ++r) {
            const QString& k = keys[r];
            auto* kItem = new QTableWidgetItem(k);
            kItem->setFlags(Qt::ItemIsEnabled);
            _faceTable->setItem(r, 0, kItem);
            _faceTable->setItem(r, 1, new QTableWidgetItem(fi.attrs.value(k)));

            auto* browseBtn = new QPushButton("…");
            browseBtn->setFixedSize(24, 20);
            const int row = r;
            connect(browseBtn, &QPushButton::clicked, this, [this, row]() {
                const QString path = QFileDialog::getOpenFileName(
                    this, "Select Image", {}, "Images (*.png *.jpg *.bmp *.gif)");
                if (!path.isEmpty())
                    if (auto* it = _faceTable->item(row, 1)) it->setText(path);
            });
            _faceTable->setCellWidget(r, 2, browseBtn);
        }
    } else {
        // SingleNode or NodeRange mode.
        _faceTable->setColumnCount(3);
        _faceTable->setHorizontalHeaderLabels({"Feature / Phoneme", "Nodes", "Color"});
        _faceTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        _faceTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        _faceTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

        QStringList keys = kPhonemeKeys;
        for (auto it = fi.attrs.constBegin(); it != fi.attrs.constEnd(); ++it) {
            const QString& k = it.key();
            if (k == "Name" || k == "Type" || k.endsWith(kColorSuffix)) continue;
            if (!keys.contains(k)) keys.append(k);
        }

        _faceTable->setRowCount(keys.size());
        for (int r = 0; r < keys.size(); ++r) {
            const QString& k = keys[r];
            auto* kItem = new QTableWidgetItem(k);
            kItem->setFlags(Qt::ItemIsEnabled);
            _faceTable->setItem(r, 0, kItem);

            const QString nodeVal = fi.attrs.value(k);
            if (fi.type == "SingleNode") {
                auto* spin = new QSpinBox;
                spin->setRange(0, _nodeCount);
                spin->setSpecialValueText("—");
                spin->setValue(nodeVal.isEmpty() ? 0 : nodeVal.toInt());
                connect(spin, QOverload<int>::of(&QSpinBox::valueChanged),
                        this, [this](int) { previewFromFaceRow(_faceTable->currentRow()); });
                _faceTable->setCellWidget(r, 1, spin);
            } else {
                _faceTable->setItem(r, 1, new QTableWidgetItem(nodeVal));
            }
            _faceTable->setCellWidget(r, 2, makeSwatchBtn(fi.attrs.value(k + kColorSuffix), _faceTable));
        }
    }
    _faceTable->blockSignals(false);
    _faceTable->setUpdatesEnabled(true);
}

// ── State editor ──────────────────────────────────────────────────────────────

void ModelEditDialog::populateStateEditor(int idx) {
    const bool valid = idx >= 0 && idx < _states.size();
    _stateType->setEnabled(valid);
    _stateTable->setEnabled(valid);
    _stateTable->setUpdatesEnabled(false);
    _stateTable->setRowCount(0);
    _stateTable->setUpdatesEnabled(true);
    if (!valid) { _preview->clearHighlight(); return; }

    const QtStateInfo& si = _states[idx];
    _stateType->blockSignals(true);
    _stateType->setCurrentIndex(qMax(0, _stateType->findText(si.type)));
    _stateType->blockSignals(false);

    rebuildStateNodeCells(si);
    _preview->clearHighlight();
}

void ModelEditDialog::rebuildStateNodeCells(const QtStateInfo& si) {
    _stateTable->setUpdatesEnabled(false);
    _stateTable->blockSignals(true);
    _stateTable->setRowCount(si.entries.size());

    for (int r = 0; r < si.entries.size(); ++r) {
        const QtStateEntry& e = si.entries[r];
        _stateTable->setItem(r, 0, new QTableWidgetItem(e.key));

        // Derive node display from key.
        QString nodeVal = e.key;
        if (nodeVal.startsWith('s') || nodeVal.startsWith('S'))
            nodeVal = nodeVal.mid(1).replace(QRegularExpression("^s|^S"), "");

        if (si.type == "SingleNode") {
            auto* spin = new QSpinBox;
            spin->setRange(1, qMax(1, _nodeCount));
            spin->setValue(qMax(1, nodeVal.section('-', 0, 0).toInt()));
            connect(spin, QOverload<int>::of(&QSpinBox::valueChanged),
                    this, [this](int) { previewFromStateRow(_stateTable->currentRow()); });
            _stateTable->setCellWidget(r, 1, spin);
        } else {
            _stateTable->setItem(r, 1, new QTableWidgetItem(nodeVal));
        }
        _stateTable->setCellWidget(r, 2, makeSwatchBtn(e.color, _stateTable));
    }
    _stateTable->blockSignals(false);
    _stateTable->setUpdatesEnabled(true);
}

// ── Preview highlight helpers ─────────────────────────────────────────────────

void ModelEditDialog::previewFromSmRow(int row) {
    if (row < 0 || row >= _smRanges->rowCount()) { _preview->clearHighlight(); return; }
    auto* it = _smRanges->item(row, 0);
    _preview->highlightNodes(parseRangeStr(it ? it->text() : QString()));
}

void ModelEditDialog::previewFromFaceRow(int row) {
    if (row < 0 || row >= _faceTable->rowCount()) { _preview->clearHighlight(); return; }
    if (_curFace < 0 || _curFace >= _faces.size()) return;
    const QString type = _faces[_curFace].type;

    QString nodeVal;
    if (type == "SingleNode") {
        if (auto* spin = qobject_cast<QSpinBox*>(_faceTable->cellWidget(row, 1)))
            nodeVal = QString::number(spin->value());
    } else if (type != "Matrix") {
        if (auto* it = _faceTable->item(row, 1)) nodeVal = it->text();
    }
    _preview->highlightNodes(parseRangeStr(nodeVal));
}

void ModelEditDialog::previewFromStateRow(int row) {
    if (row < 0 || row >= _stateTable->rowCount()) { _preview->clearHighlight(); return; }
    if (_curState < 0 || _curState >= _states.size()) return;

    const QString type = _states[_curState].type;
    QList<int> indices;

    auto* keyIt = _stateTable->item(row, 0);
    if (type == "SingleNode") {
        if (auto* spin = qobject_cast<QSpinBox*>(_stateTable->cellWidget(row, 1)))
            indices = parseRangeStr(QString::number(spin->value()));
    } else if (keyIt) {
        indices = parseStateKey(keyIt->text(), _nodeCount);
    }

    // Use the state's color for preview.
    QColor color = Qt::yellow;
    const QString hex = swatchColor(_stateTable->cellWidget(row, 2));
    if (!hex.isEmpty()) { QColor c(hex); if (c.isValid()) color = c; }

    _preview->highlightNodes(indices, color);
}

// ── Cell-changed slots (drive preview) ───────────────────────────────────────

void ModelEditDialog::onSmRangeCellChanged(int row, int) { previewFromSmRow(row); }
void ModelEditDialog::onFaceCellChanged(int row, int)    { previewFromFaceRow(row); }
void ModelEditDialog::onStateCellChanged(int row, int)   { previewFromStateRow(row); }

// ── Lasso → row population ────────────────────────────────────────────────────

void ModelEditDialog::onNodesLassoed(const QList<int>& indices) {
    if (indices.isEmpty()) return;

    if (_activeTab == 0) {
        // Sub-models: write range string into current range row.
        const int row = _smRanges->currentRow();
        if (row < 0) return;
        _smRanges->blockSignals(true);
        _smRanges->setItem(row, 0, new QTableWidgetItem(indicesToRangeStr(indices)));
        _smRanges->blockSignals(false);
        _preview->highlightNodes(indices);

    } else if (_activeTab == 1 && _curFace >= 0) {
        // Faces: write into Nodes cell of selected row.
        const int row = _faceTable->currentRow();
        if (row < 0) return;
        const QString type = _faceType->currentText();
        if (type == "SingleNode") {
            if (auto* spin = qobject_cast<QSpinBox*>(_faceTable->cellWidget(row, 1)))
                spin->setValue(indices.first() + 1);
        } else if (type == "NodeRange") {
            _faceTable->blockSignals(true);
            _faceTable->setItem(row, 1, new QTableWidgetItem(indicesToRangeStr(indices)));
            _faceTable->blockSignals(false);
        }
        _preview->highlightNodes(indices);

    } else if (_activeTab == 2 && _curState >= 0) {
        // States: write into Nodes cell of selected row.
        const int row = _stateTable->currentRow();
        if (row < 0) return;
        const QString type = _stateType->currentText();
        if (type == "SingleNode") {
            if (auto* spin = qobject_cast<QSpinBox*>(_stateTable->cellWidget(row, 1)))
                spin->setValue(indices.first() + 1);
        } else {
            _stateTable->blockSignals(true);
            _stateTable->setItem(row, 1, new QTableWidgetItem(indicesToRangeStr(indices)));
            _stateTable->blockSignals(false);
        }
        _preview->highlightNodes(indices);
    }
}

// ── Commit helpers ────────────────────────────────────────────────────────────

void ModelEditDialog::commitCurrentSubModel() {
    if (_curSm < 0 || _curSm >= _subModels.size()) return;
    QtSubModelInfo& sm = _subModels[_curSm];
    sm.name        = _smNameEdit->text().trimmed();
    sm.layout      = _smLayout->currentText();
    sm.type        = _smType->currentText();
    sm.bufferStyle = _smBufferStyle->currentText();
    sm.ranges.clear();
    for (int r = 0; r < _smRanges->rowCount(); ++r) {
        auto* it = _smRanges->item(r, 0);
        if (it && !it->text().trimmed().isEmpty()) sm.ranges.append(it->text().trimmed());
    }
}

void ModelEditDialog::commitCurrentFace() {
    if (_curFace < 0 || _curFace >= _faces.size()) return;
    QtFaceInfo& fi = _faces[_curFace];
    fi.type = _faceType->currentText();
    fi.attrs["Name"] = fi.name;
    fi.attrs["Type"] = fi.type;

    if (fi.type == "Matrix") {
        for (int r = 0; r < _faceTable->rowCount(); ++r) {
            auto* kIt = _faceTable->item(r, 0);
            auto* vIt = _faceTable->item(r, 1);
            if (kIt && vIt && !vIt->text().isEmpty())
                fi.attrs[kIt->text()] = vIt->text();
        }
    } else {
        for (int r = 0; r < _faceTable->rowCount(); ++r) {
            auto* kIt = _faceTable->item(r, 0);
            if (!kIt) continue;
            const QString key = kIt->text();

            QString nodeVal;
            if (fi.type == "SingleNode") {
                if (auto* spin = qobject_cast<QSpinBox*>(_faceTable->cellWidget(r, 1)))
                    nodeVal = spin->value() > 0 ? QString::number(spin->value()) : QString();
            } else {
                if (auto* vIt = _faceTable->item(r, 1)) nodeVal = vIt->text().trimmed();
            }
            if (nodeVal.isEmpty()) fi.attrs.remove(key); else fi.attrs[key] = nodeVal;

            const QString color = swatchColor(_faceTable->cellWidget(r, 2));
            if (color.isEmpty() || color == "#000000")
                fi.attrs.remove(key + kColorSuffix);
            else
                fi.attrs[key + kColorSuffix] = color;
        }
    }
}

void ModelEditDialog::commitCurrentState() {
    if (_curState < 0 || _curState >= _states.size()) return;
    QtStateInfo& si = _states[_curState];
    si.type = _stateType->currentText();
    si.entries.clear();
    for (int r = 0; r < _stateTable->rowCount(); ++r) {
        auto* kIt = _stateTable->item(r, 0);
        if (!kIt || kIt->text().trimmed().isEmpty()) continue;
        QtStateEntry e;
        e.key   = kIt->text().trimmed();
        e.color = swatchColor(_stateTable->cellWidget(r, 2));
        if (e.color.isEmpty()) e.color = "Black";
        si.entries.append(e);
    }
}

// ── Sub-model slots ───────────────────────────────────────────────────────────

void ModelEditDialog::onSmSelectionChanged() {
    commitCurrentSubModel();
    _curSm = _smList->currentRow();
    populateSmEditor(_curSm);
}

void ModelEditDialog::onSmAdd() {
    commitCurrentSubModel();
    QtSubModelInfo sm; sm.name = "SubModel" + QString::number(_subModels.size() + 1);
    _subModels.append(sm); refreshSmList();
    _smList->setCurrentRow(_subModels.size() - 1); onSmSelectionChanged();
}

void ModelEditDialog::onSmDelete() {
    if (_curSm < 0 || _curSm >= _subModels.size()) return;
    _subModels.removeAt(_curSm); _curSm = -1; refreshSmList();
}

void ModelEditDialog::onSmNameEdited(const QString& t) {
    if (_curSm < 0 || _curSm >= _subModels.size()) return;
    _subModels[_curSm].name = t.trimmed();
    if (auto* it = _smList->item(_curSm)) it->setText(t.trimmed().isEmpty() ? "(unnamed)" : t.trimmed());
}

void ModelEditDialog::onSmBufferStyleChanged(const QString& s) {
    if (_curSm >= 0 && _curSm < _subModels.size()) _subModels[_curSm].bufferStyle = s;
}

void ModelEditDialog::onSmRangeAdd() {
    if (_curSm < 0) return;
    const int r = _smRanges->rowCount();
    _smRanges->setRowCount(r + 1);
    _smRanges->setItem(r, 0, new QTableWidgetItem(""));
    _smRanges->editItem(_smRanges->item(r, 0));
}

void ModelEditDialog::onSmRangeDelete() {
    const int r = _smRanges->currentRow();
    if (r >= 0) _smRanges->removeRow(r);
}

// ── Face slots ────────────────────────────────────────────────────────────────

void ModelEditDialog::onFaceSelectionChanged() {
    commitCurrentFace();
    _curFace = _faceList->currentRow();
    populateFaceEditor(_curFace);
}

void ModelEditDialog::onFaceAdd() {
    commitCurrentFace();
    QtFaceInfo fi; fi.name = "Face" + QString::number(_faces.size() + 1);
    fi.type = "NodeRange"; fi.attrs["Name"] = fi.name; fi.attrs["Type"] = fi.type;
    _faces.append(fi); refreshFaceList();
    _faceList->setCurrentRow(_faces.size() - 1); onFaceSelectionChanged();
}

void ModelEditDialog::onFaceDelete() {
    if (_curFace < 0 || _curFace >= _faces.size()) return;
    _faces.removeAt(_curFace); _curFace = -1; refreshFaceList();
}

void ModelEditDialog::onFaceTypeChanged(const QString& t) {
    if (_curFace < 0 || _curFace >= _faces.size()) return;
    commitCurrentFace();
    _faces[_curFace].type = t;
    _faces[_curFace].attrs["Type"] = t;
    rebuildFaceNodeCells(_faces[_curFace]);
}

// ── State slots ───────────────────────────────────────────────────────────────

void ModelEditDialog::onStateSelectionChanged() {
    commitCurrentState();
    _curState = _stateList->currentRow();
    populateStateEditor(_curState);
}

void ModelEditDialog::onStateAdd() {
    commitCurrentState();
    QtStateInfo si; si.name = "State" + QString::number(_states.size() + 1); si.type = "NodeRange";
    _states.append(si); refreshStateList();
    _stateList->setCurrentRow(_states.size() - 1); onStateSelectionChanged();
}

void ModelEditDialog::onStateDelete() {
    if (_curState < 0 || _curState >= _states.size()) return;
    _states.removeAt(_curState); _curState = -1; refreshStateList();
}

void ModelEditDialog::onStateTypeChanged(const QString& t) {
    if (_curState < 0 || _curState >= _states.size()) return;
    commitCurrentState();
    _states[_curState].type = t;
    rebuildStateNodeCells(_states[_curState]);
}

void ModelEditDialog::onStateEntryAdd() {
    if (_curState < 0) return;
    const int r = _stateTable->rowCount();
    _stateTable->setRowCount(r + 1);
    const QString key = QString("s%1").arg(r + 1, 3, 10, QChar('0'));
    _stateTable->setItem(r, 0, new QTableWidgetItem(key));
    const QString type = _stateType->currentText();
    if (type == "SingleNode") {
        auto* spin = new QSpinBox; spin->setRange(1, qMax(1, _nodeCount)); spin->setValue(r + 1);
        _stateTable->setCellWidget(r, 1, spin);
    } else {
        _stateTable->setItem(r, 1, new QTableWidgetItem(""));
    }
    _stateTable->setCellWidget(r, 2, makeSwatchBtn("", _stateTable));
}

void ModelEditDialog::onStateEntryDelete() {
    const int r = _stateTable->currentRow();
    if (r >= 0) _stateTable->removeRow(r);
}

// ── Save ──────────────────────────────────────────────────────────────────────

void ModelEditDialog::onSave() {
    commitCurrentSubModel(); commitCurrentFace(); commitCurrentState();
    if (!saveToXml()) {
        QMessageBox::critical(this, "Save Failed",
            "Could not write to xlights_rgbeffects.xml.\n"
            "Check the show folder is set and the file is not read-only.");
        return;
    }
    const QString sf = QtXLightsApp::instance().showFolder();
    if (!sf.isEmpty()) QtXLightsApp::instance().setShowFolder(sf);
    accept();
}

bool ModelEditDialog::saveToXml() {
    const QString sf = QtXLightsApp::instance().showFolder();
    if (sf.isEmpty()) return false;
    const QString xmlPath = sf + "/xlights_rgbeffects.xml";
    pugi::xml_document doc;
    if (!doc.load_file(xmlPath.toStdString().c_str())) return false;
    pugi::xml_node root = doc.child("xrgb");
    if (!root) root = doc.child("xlights_rgbeffects");
    if (!root) return false;
    pugi::xml_node modelsList = root.child("models");
    if (!modelsList) return false;

    pugi::xml_node modelNode;
    for (auto m : modelsList.children("model"))
        if (QString::fromUtf8(m.attribute("name").as_string()) == _modelName) { modelNode = m; break; }
    if (!modelNode) return false;

    std::vector<pugi::xml_node> toRemove;
    for (auto ch : modelNode.children()) {
        std::string tag = ch.name();
        if (tag == "subModel" || tag == "faceInfo" || tag == "stateInfo") toRemove.push_back(ch);
    }
    for (auto& n : toRemove) modelNode.remove_child(n);

    for (const QtSubModelInfo& sm : _subModels) {
        auto smNode = modelNode.append_child("subModel");
        smNode.append_attribute("name")        = sm.name.toStdString().c_str();
        smNode.append_attribute("layout")      = sm.layout.toStdString().c_str();
        smNode.append_attribute("type")        = sm.type.toStdString().c_str();
        smNode.append_attribute("bufferstyle") = sm.bufferStyle.toStdString().c_str();
        if (sm.type == "ranges") {
            for (int i = 0; i < sm.ranges.size(); ++i)
                smNode.append_attribute(("line" + std::to_string(i)).c_str()) =
                    sm.ranges[i].toStdString().c_str();
        } else {
            smNode.append_attribute("subBuffer") =
                sm.ranges.isEmpty() ? "" : sm.ranges[0].toStdString().c_str();
        }
    }

    for (const QtFaceInfo& fi : _faces) {
        auto fNode = modelNode.append_child("faceInfo");
        fNode.append_attribute("Name") = fi.name.toStdString().c_str();
        fNode.append_attribute("Type") = fi.type.toStdString().c_str();
        for (auto it = fi.attrs.constBegin(); it != fi.attrs.constEnd(); ++it) {
            if (it.key() == "Name" || it.key() == "Type" || it.value().isEmpty()) continue;
            fNode.append_attribute(it.key().toStdString().c_str()) = it.value().toStdString().c_str();
        }
    }

    for (const QtStateInfo& si : _states) {
        auto sNode = modelNode.append_child("stateInfo");
        sNode.append_attribute("Name") = si.name.toStdString().c_str();
        sNode.append_attribute("Type") = si.type.toStdString().c_str();
        for (const QtStateEntry& e : si.entries) {
            if (e.key.isEmpty()) continue;
            sNode.append_attribute(e.key.toStdString().c_str()) = e.color.toStdString().c_str();
        }
    }

    return doc.save_file(xmlPath.toStdString().c_str());
}

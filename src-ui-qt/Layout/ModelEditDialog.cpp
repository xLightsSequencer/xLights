#include "ModelEditDialog.h"
#include "../App/QtXLightsApp.h"

#include <pugixml.hpp>
#include <spdlog/spdlog.h>

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include <QTabWidget>
#include <QVBoxLayout>

// ── Known face phoneme keys (display order) ───────────────────────────────────
static const QStringList kPhonemeKeys = {
    "FaceOutline",
    "Mouth_AI", "Mouth_E", "Mouth_etc", "Mouth_FV", "Mouth_L",
    "Mouth_MBP", "Mouth_O", "Mouth_rest", "Mouth_U", "Mouth_WQ",
    "Eyes_Open", "Eyes_Closed"
};
static const QString kColorSuffix = "Color";

// ── Color-swatch helpers ──────────────────────────────────────────────────────

// Parse a color string (name or #rrggbb) to QColor; returns black on failure.
static QColor parseColor(const QString& s) {
    if (s.isEmpty()) return QColor();
    QColor c(s);
    return c.isValid() ? c : QColor();
}

// Create a colored push-button that opens QColorDialog on click.
// The current color is stored in the Qt property "colorHex".
static QPushButton* makeSwatchBtn(const QString& colorStr, QWidget* parent) {
    auto* btn = new QPushButton(parent);
    btn->setFlat(true);
    btn->setFixedSize(40, 20);

    const QColor c = parseColor(colorStr);
    const QString hex = c.isValid() ? c.name() : "#000000";
    btn->setProperty("colorHex", hex);
    btn->setStyleSheet(QString("background-color:%1;border:1px solid #666;").arg(hex));

    QObject::connect(btn, &QPushButton::clicked, [btn, parent]() {
        const QColor cur = QColor(btn->property("colorHex").toString());
        const QColor chosen = QColorDialog::getColor(cur.isValid() ? cur : Qt::black, parent, "Color");
        if (chosen.isValid()) {
            btn->setProperty("colorHex", chosen.name());
            btn->setStyleSheet(QString("background-color:%1;border:1px solid #666;")
                               .arg(chosen.name()));
        }
    });
    return btn;
}

static QString swatchColor(QWidget* btn) {
    if (!btn) return {};
    return btn->property("colorHex").toString();
}

// ── Constructor ───────────────────────────────────────────────────────────────

ModelEditDialog::ModelEditDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Model Editor");
    setMinimumSize(860, 580);
    resize(1040, 660);

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

    auto* buttons = new QDialogButtonBox;
    auto* saveBtn  = buttons->addButton("Save",  QDialogButtonBox::AcceptRole);
    auto* closeBtn = buttons->addButton("Close", QDialogButtonBox::RejectRole);
    connect(saveBtn,  &QPushButton::clicked, this, &ModelEditDialog::onSave);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    root->addWidget(_titleLabel);
    root->addWidget(_tabs, 1);
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
    auto* w = new QWidget;
    auto* vl = new QVBoxLayout(w);
    vl->setContentsMargins(0,0,0,0);
    vl->addWidget(list, 1);
    vl->addLayout(btnRow);
    return w;
}

static QSplitter* makeTabSplit(QWidget* listPanel, QWidget* editor) {
    auto* s = new QSplitter(Qt::Horizontal);
    s->addWidget(listPanel);
    s->addWidget(editor);
    s->setSizes({220, 700});
    s->setStretchFactor(1, 1);
    return s;
}

void ModelEditDialog::setupSubModelsTab(QWidget* tab) {
    auto* listPanel = makeListPanel(_smList, this,
                                    SLOT(onSmAdd()), SLOT(onSmDelete()));

    _smNameEdit    = new QLineEdit;
    _smLayout      = new QComboBox;
    _smLayout->addItems({"horizontal", "vertical"});
    _smType        = new QComboBox;
    _smType->addItems({"ranges", "subbuffer"});
    _smBufferStyle = new QComboBox;
    _smBufferStyle->addItems({"Default", "Keep XY", "Stacked Strands"});

    auto* form = new QFormLayout;
    form->addRow("Name:",         _smNameEdit);
    form->addRow("Layout:",       _smLayout);
    form->addRow("Type:",         _smType);
    form->addRow("Buffer style:", _smBufferStyle);

    _smRanges = new QTableWidget(0, 1);
    _smRanges->setHorizontalHeaderLabels({"Node Ranges"});
    _smRanges->horizontalHeader()->setStretchLastSection(true);
    _smRanges->verticalHeader()->hide();

    auto* rAdd = new QPushButton("+ Row");
    auto* rDel = new QPushButton("– Row");
    connect(rAdd, &QPushButton::clicked, this, &ModelEditDialog::onSmRangeAdd);
    connect(rDel, &QPushButton::clicked, this, &ModelEditDialog::onSmRangeDelete);
    auto* rBtns = new QHBoxLayout;
    rBtns->addWidget(rAdd); rBtns->addWidget(rDel); rBtns->addStretch();

    auto* ed = new QWidget;
    auto* edVL = new QVBoxLayout(ed);
    edVL->addLayout(form);
    edVL->addWidget(new QLabel("Ranges (one per row):"));
    edVL->addWidget(_smRanges, 1);
    edVL->addLayout(rBtns);

    auto* vl = new QVBoxLayout(tab);
    vl->setContentsMargins(4,4,4,4);
    vl->addWidget(makeTabSplit(listPanel, ed));

    connect(_smList, &QListWidget::currentRowChanged, this, [this](int) { onSmSelectionChanged(); });
    connect(_smNameEdit,    &QLineEdit::textEdited,      this, &ModelEditDialog::onSmNameEdited);
    connect(_smBufferStyle, &QComboBox::currentTextChanged, this, &ModelEditDialog::onSmBufferStyleChanged);
}

void ModelEditDialog::setupFacesTab(QWidget* tab) {
    auto* listPanel = makeListPanel(_faceList, this,
                                    SLOT(onFaceAdd()), SLOT(onFaceDelete()));

    _faceType = new QComboBox;
    _faceType->addItems({"NodeRange", "SingleNode", "Matrix"});

    // 3 columns: Feature (read-only), Nodes, Color swatch
    _faceTable = new QTableWidget(0, 3);
    _faceTable->setHorizontalHeaderLabels({"Feature / Phoneme", "Nodes", "Color"});
    _faceTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    _faceTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    _faceTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    _faceTable->setColumnWidth(2, 50);
    _faceTable->verticalHeader()->hide();

    auto* ed = new QWidget;
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
}

void ModelEditDialog::setupStatesTab(QWidget* tab) {
    auto* listPanel = makeListPanel(_stateList, this,
                                    SLOT(onStateAdd()), SLOT(onStateDelete()));

    _stateType = new QComboBox;
    _stateType->addItems({"NodeRange", "SingleNode"});

    // 3 columns: Key (s001…), Nodes (derived / same), Color swatch
    _stateTable = new QTableWidget(0, 3);
    _stateTable->setHorizontalHeaderLabels({"Key (s001…)", "Nodes", "Color"});
    _stateTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    _stateTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    _stateTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    _stateTable->setColumnWidth(2, 50);
    _stateTable->verticalHeader()->hide();

    auto* seAdd = new QPushButton("+ Row");
    auto* seDel = new QPushButton("– Row");
    connect(seAdd, &QPushButton::clicked, this, &ModelEditDialog::onStateEntryAdd);
    connect(seDel, &QPushButton::clicked, this, &ModelEditDialog::onStateEntryDelete);
    auto* seBtns = new QHBoxLayout;
    seBtns->addWidget(seAdd); seBtns->addWidget(seDel); seBtns->addStretch();

    auto* ed = new QWidget;
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
}

// ── openForModel ──────────────────────────────────────────────────────────────

void ModelEditDialog::openForModel(const QString& modelName) {
    _modelName = modelName;
    _titleLabel->setText(modelName);
    setWindowTitle("Model Editor — " + modelName);

    const QtModelInfo& mi =
        QtXLightsApp::instance().currentSequence().modelInfo(modelName);
    _subModels = mi.subModels;
    _faces     = mi.faces;
    _states    = mi.states;

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

// ── Editor population ─────────────────────────────────────────────────────────

void ModelEditDialog::populateSmEditor(int idx) {
    const bool valid = idx >= 0 && idx < _subModels.size();
    _smNameEdit->setEnabled(valid);
    _smLayout->setEnabled(valid);
    _smType->setEnabled(valid);
    _smBufferStyle->setEnabled(valid);
    _smRanges->setEnabled(valid);

    if (!valid) { _smNameEdit->clear(); _smRanges->setRowCount(0); return; }

    const QtSubModelInfo& sm = _subModels[idx];

    _smNameEdit->blockSignals(true);
    _smNameEdit->setText(sm.name);
    _smNameEdit->blockSignals(false);

    auto setCombo = [](QComboBox* cb, const QString& v) {
        cb->blockSignals(true);
        int i = cb->findText(v);
        cb->setCurrentIndex(i >= 0 ? i : 0);
        cb->blockSignals(false);
    };
    setCombo(_smLayout,      sm.layout);
    setCombo(_smType,        sm.type);
    setCombo(_smBufferStyle, sm.bufferStyle);

    _smRanges->setRowCount(sm.ranges.size());
    for (int r = 0; r < sm.ranges.size(); ++r)
        _smRanges->setItem(r, 0, new QTableWidgetItem(sm.ranges[r]));
}

void ModelEditDialog::populateFaceEditor(int idx) {
    const bool valid = idx >= 0 && idx < _faces.size();
    _faceType->setEnabled(valid);
    _faceTable->setEnabled(valid);
    _faceTable->setRowCount(0);
    if (!valid) return;

    const QtFaceInfo& fi = _faces[idx];
    int tIdx = _faceType->findText(fi.type);
    _faceType->blockSignals(true);
    _faceType->setCurrentIndex(tIdx >= 0 ? tIdx : 0);
    _faceType->blockSignals(false);

    // Build rows: known phonemes first, then any unknown attrs that aren't Color variants.
    QStringList keys = kPhonemeKeys;
    for (auto it = fi.attrs.constBegin(); it != fi.attrs.constEnd(); ++it) {
        const QString& k = it.key();
        if (k == "Name" || k == "Type" || k.endsWith(kColorSuffix)) continue;
        if (!keys.contains(k)) keys.append(k);
    }

    _faceTable->setRowCount(keys.size());
    for (int r = 0; r < keys.size(); ++r) {
        const QString& k = keys[r];
        // Feature label (read-only)
        auto* kItem = new QTableWidgetItem(k);
        kItem->setFlags(Qt::ItemIsEnabled);
        _faceTable->setItem(r, 0, kItem);
        // Nodes column
        _faceTable->setItem(r, 1, new QTableWidgetItem(fi.attrs.value(k)));
        // Color swatch
        _faceTable->setCellWidget(r, 2,
            makeSwatchBtn(fi.attrs.value(k + kColorSuffix), _faceTable));
    }
}

void ModelEditDialog::populateStateEditor(int idx) {
    const bool valid = idx >= 0 && idx < _states.size();
    _stateType->setEnabled(valid);
    _stateTable->setEnabled(valid);
    _stateTable->setRowCount(0);
    if (!valid) return;

    const QtStateInfo& si = _states[idx];
    int tIdx = _stateType->findText(si.type);
    _stateType->blockSignals(true);
    _stateType->setCurrentIndex(tIdx >= 0 ? tIdx : 0);
    _stateType->blockSignals(false);

    _stateTable->setRowCount(si.entries.size());
    for (int r = 0; r < si.entries.size(); ++r) {
        const QtStateEntry& e = si.entries[r];
        _stateTable->setItem(r, 0, new QTableWidgetItem(e.key));
        // Nodes: for "s001-s010" show "1-10"; for "s003" show "3"
        QString nodes = e.key;
        if (nodes.startsWith('s')) nodes = nodes.mid(1).replace('-', '-');
        _stateTable->setItem(r, 1, new QTableWidgetItem(nodes));
        _stateTable->setCellWidget(r, 2, makeSwatchBtn(e.color, _stateTable));
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
        if (it && !it->text().trimmed().isEmpty())
            sm.ranges.append(it->text().trimmed());
    }
}

void ModelEditDialog::commitCurrentFace() {
    if (_curFace < 0 || _curFace >= _faces.size()) return;
    QtFaceInfo& fi = _faces[_curFace];
    fi.type = _faceType->currentText();
    fi.attrs["Name"] = fi.name;
    fi.attrs["Type"] = fi.type;
    for (int r = 0; r < _faceTable->rowCount(); ++r) {
        auto* kItem = _faceTable->item(r, 0);
        auto* vItem = _faceTable->item(r, 1);
        if (!kItem) continue;
        const QString key = kItem->text();
        const QString nodes = vItem ? vItem->text().trimmed() : QString();
        if (nodes.isEmpty())
            fi.attrs.remove(key);
        else
            fi.attrs[key] = nodes;
        // Color swatch
        const QString color = swatchColor(_faceTable->cellWidget(r, 2));
        if (color.isEmpty() || color == "#000000")
            fi.attrs.remove(key + kColorSuffix);
        else
            fi.attrs[key + kColorSuffix] = color;
    }
}

void ModelEditDialog::commitCurrentState() {
    if (_curState < 0 || _curState >= _states.size()) return;
    QtStateInfo& si = _states[_curState];
    si.type = _stateType->currentText();
    si.entries.clear();
    for (int r = 0; r < _stateTable->rowCount(); ++r) {
        auto* kItem = _stateTable->item(r, 0);
        if (!kItem || kItem->text().trimmed().isEmpty()) continue;
        QtStateEntry e;
        e.key   = kItem->text().trimmed();
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
    QtSubModelInfo sm;
    sm.name = "SubModel" + QString::number(_subModels.size() + 1);
    _subModels.append(sm);
    refreshSmList();
    _smList->setCurrentRow(_subModels.size() - 1);
    onSmSelectionChanged();
}

void ModelEditDialog::onSmDelete() {
    if (_curSm < 0 || _curSm >= _subModels.size()) return;
    _subModels.removeAt(_curSm);
    _curSm = -1;
    refreshSmList();
}

void ModelEditDialog::onSmNameEdited(const QString& text) {
    if (_curSm < 0 || _curSm >= _subModels.size()) return;
    _subModels[_curSm].name = text.trimmed();
    if (auto* it = _smList->item(_curSm))
        it->setText(text.trimmed().isEmpty() ? "(unnamed)" : text.trimmed());
}

void ModelEditDialog::onSmBufferStyleChanged(const QString& s) {
    if (_curSm >= 0 && _curSm < _subModels.size())
        _subModels[_curSm].bufferStyle = s;
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
    QtFaceInfo fi;
    fi.name = "Face" + QString::number(_faces.size() + 1);
    fi.type = "NodeRange";
    fi.attrs["Name"] = fi.name;
    fi.attrs["Type"] = fi.type;
    _faces.append(fi);
    refreshFaceList();
    _faceList->setCurrentRow(_faces.size() - 1);
    onFaceSelectionChanged();
}

void ModelEditDialog::onFaceDelete() {
    if (_curFace < 0 || _curFace >= _faces.size()) return;
    _faces.removeAt(_curFace);
    _curFace = -1;
    refreshFaceList();
}

void ModelEditDialog::onFaceTypeChanged(const QString& t) {
    if (_curFace >= 0 && _curFace < _faces.size()) {
        _faces[_curFace].type = t;
        _faces[_curFace].attrs["Type"] = t;
    }
}

// ── State slots ───────────────────────────────────────────────────────────────

void ModelEditDialog::onStateSelectionChanged() {
    commitCurrentState();
    _curState = _stateList->currentRow();
    populateStateEditor(_curState);
}

void ModelEditDialog::onStateAdd() {
    commitCurrentState();
    QtStateInfo si;
    si.name = "State" + QString::number(_states.size() + 1);
    si.type = "NodeRange";
    _states.append(si);
    refreshStateList();
    _stateList->setCurrentRow(_states.size() - 1);
    onStateSelectionChanged();
}

void ModelEditDialog::onStateDelete() {
    if (_curState < 0 || _curState >= _states.size()) return;
    _states.removeAt(_curState);
    _curState = -1;
    refreshStateList();
}

void ModelEditDialog::onStateTypeChanged(const QString& t) {
    if (_curState >= 0 && _curState < _states.size())
        _states[_curState].type = t;
}

void ModelEditDialog::onStateEntryAdd() {
    if (_curState < 0) return;
    const int r = _stateTable->rowCount();
    _stateTable->setRowCount(r + 1);
    _stateTable->setItem(r, 0, new QTableWidgetItem(
        QString("s%1").arg(r + 1, 3, 10, QChar('0'))));
    _stateTable->setItem(r, 1, new QTableWidgetItem(""));
    _stateTable->setCellWidget(r, 2, makeSwatchBtn("", _stateTable));
}

void ModelEditDialog::onStateEntryDelete() {
    const int r = _stateTable->currentRow();
    if (r >= 0) _stateTable->removeRow(r);
}

// ── Save ──────────────────────────────────────────────────────────────────────

void ModelEditDialog::onSave() {
    commitCurrentSubModel();
    commitCurrentFace();
    commitCurrentState();

    if (!saveToXml()) {
        QMessageBox::critical(this, "Save Failed",
            "Could not write to xlights_rgbeffects.xml.\n"
            "Check the show folder is set and the file is not read-only.");
        return;
    }

    const QString sf = QtXLightsApp::instance().showFolder();
    if (!sf.isEmpty())
        QtXLightsApp::instance().setShowFolder(sf);

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
    for (auto m : modelsList.children("model")) {
        if (QString::fromUtf8(m.attribute("name").as_string()) == _modelName) {
            modelNode = m; break;
        }
    }
    if (!modelNode) return false;

    // Remove existing sub-model, face, state children.
    std::vector<pugi::xml_node> toRemove;
    for (auto ch : modelNode.children()) {
        std::string tag = ch.name();
        if (tag == "subModel" || tag == "faceInfo" || tag == "stateInfo")
            toRemove.push_back(ch);
    }
    for (auto& n : toRemove) modelNode.remove_child(n);

    // Write sub-models using the correct attribute names.
    for (const QtSubModelInfo& sm : _subModels) {
        auto smNode = modelNode.append_child("subModel");
        smNode.append_attribute("name")        = sm.name.toStdString().c_str();
        smNode.append_attribute("layout")      = sm.layout.toStdString().c_str();
        smNode.append_attribute("type")        = sm.type.toStdString().c_str();
        smNode.append_attribute("bufferstyle") = sm.bufferStyle.toStdString().c_str();
        if (sm.type == "ranges") {
            for (int i = 0; i < sm.ranges.size(); ++i) {
                auto key = "line" + std::to_string(i);
                smNode.append_attribute(key.c_str()) =
                    sm.ranges[i].toStdString().c_str();
            }
        } else {
            smNode.append_attribute("subBuffer") =
                sm.ranges.isEmpty() ? "" : sm.ranges[0].toStdString().c_str();
        }
    }

    // Write faces.
    for (const QtFaceInfo& fi : _faces) {
        auto fNode = modelNode.append_child("faceInfo");
        fNode.append_attribute("Name") = fi.name.toStdString().c_str();
        fNode.append_attribute("Type") = fi.type.toStdString().c_str();
        for (auto it = fi.attrs.constBegin(); it != fi.attrs.constEnd(); ++it) {
            if (it.key() == "Name" || it.key() == "Type" || it.value().isEmpty())
                continue;
            fNode.append_attribute(it.key().toStdString().c_str()) =
                it.value().toStdString().c_str();
        }
    }

    // Write states.
    for (const QtStateInfo& si : _states) {
        auto sNode = modelNode.append_child("stateInfo");
        sNode.append_attribute("Name") = si.name.toStdString().c_str();
        sNode.append_attribute("Type") = si.type.toStdString().c_str();
        for (const QtStateEntry& e : si.entries) {
            if (e.key.isEmpty()) continue;
            sNode.append_attribute(e.key.toStdString().c_str()) =
                e.color.toStdString().c_str();
        }
    }

    return doc.save_file(xmlPath.toStdString().c_str());
}

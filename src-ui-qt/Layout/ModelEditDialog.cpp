#include "ModelEditDialog.h"
#include "../App/QtXLightsApp.h"

#include <pugixml.hpp>
#include <spdlog/spdlog.h>

#include <QCheckBox>
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

// ── Constructor ───────────────────────────────────────────────────────────────

ModelEditDialog::ModelEditDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Model Editor");
    setMinimumSize(820, 580);
    resize(1000, 660);

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

// ── Tab setup helpers ─────────────────────────────────────────────────────────

static QWidget* makeListPanel(QListWidget*& list, const QString& addTip,
                               QObject* recv, const char* addSlot, const char* delSlot) {
    list = new QListWidget;
    auto* addBtn = new QPushButton("+");
    auto* delBtn = new QPushButton("–");
    addBtn->setFixedWidth(28); delBtn->setFixedWidth(28);
    addBtn->setToolTip(addTip);
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

void ModelEditDialog::setupSubModelsTab(QWidget* tab) {
    auto* listPanel = makeListPanel(_smList, "Add sub-model",
                                    this, SLOT(onSmAdd()), SLOT(onSmDelete()));

    // Editor: form at top, ranges table below
    auto* form = new QFormLayout;
    _smNameEdit    = new QLineEdit;
    _smBufferStyle = new QComboBox;
    _smBufferStyle->addItems({"Default", "Keep XY", "Stacked Strands"});
    _smVertical    = new QCheckBox("Vertical layout");
    form->addRow("Name:",         _smNameEdit);
    form->addRow("Buffer style:", _smBufferStyle);
    form->addRow("",              _smVertical);

    _smRanges = new QTableWidget(0, 1);
    _smRanges->setHorizontalHeaderLabels({"Node Ranges"});
    _smRanges->horizontalHeader()->setStretchLastSection(true);
    _smRanges->verticalHeader()->hide();

    auto* rAddBtn = new QPushButton("+ Row");
    auto* rDelBtn = new QPushButton("– Row");
    connect(rAddBtn, &QPushButton::clicked, this, &ModelEditDialog::onSmRangeAdd);
    connect(rDelBtn, &QPushButton::clicked, this, &ModelEditDialog::onSmRangeDelete);
    auto* rBtns = new QHBoxLayout;
    rBtns->addWidget(rAddBtn); rBtns->addWidget(rDelBtn); rBtns->addStretch();

    auto* editorW = new QWidget;
    auto* edVL    = new QVBoxLayout(editorW);
    edVL->addLayout(form);
    edVL->addWidget(new QLabel("Ranges:"));
    edVL->addWidget(_smRanges, 1);
    edVL->addLayout(rBtns);

    auto* split = new QSplitter(Qt::Horizontal);
    split->addWidget(listPanel);
    split->addWidget(editorW);
    split->setSizes({220, 600});
    split->setStretchFactor(1, 1);

    auto* vl = new QVBoxLayout(tab);
    vl->setContentsMargins(4,4,4,4);
    vl->addWidget(split);

    connect(_smList, &QListWidget::currentRowChanged, this, [this](int) {
        onSmSelectionChanged();
    });
    connect(_smNameEdit,    &QLineEdit::textEdited,
            this, &ModelEditDialog::onSmNameEdited);
    connect(_smBufferStyle, &QComboBox::currentTextChanged,
            this, &ModelEditDialog::onSmBufferStyleChanged);
    connect(_smVertical,    &QCheckBox::toggled,
            this, &ModelEditDialog::onSmVerticalChanged);
}

void ModelEditDialog::setupFacesTab(QWidget* tab) {
    auto* listPanel = makeListPanel(_faceList, "Add face",
                                    this, SLOT(onFaceAdd()), SLOT(onFaceDelete()));

    _faceType = new QComboBox;
    _faceType->addItems({"NodeRange", "SingleNode", "Matrix"});

    _faceTable = new QTableWidget(0, 2);
    _faceTable->setHorizontalHeaderLabels({"Feature / Phoneme", "Nodes / Value"});
    _faceTable->horizontalHeader()->setStretchLastSection(true);
    _faceTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    _faceTable->verticalHeader()->hide();

    auto* editorW = new QWidget;
    auto* edVL    = new QVBoxLayout(editorW);
    auto* typeRow = new QHBoxLayout;
    typeRow->addWidget(new QLabel("Type:"));
    typeRow->addWidget(_faceType);
    typeRow->addStretch();
    edVL->addLayout(typeRow);
    edVL->addWidget(_faceTable, 1);

    auto* split = new QSplitter(Qt::Horizontal);
    split->addWidget(listPanel);
    split->addWidget(editorW);
    split->setSizes({220, 600});
    split->setStretchFactor(1, 1);

    auto* vl = new QVBoxLayout(tab);
    vl->setContentsMargins(4,4,4,4);
    vl->addWidget(split);

    connect(_faceList, &QListWidget::currentRowChanged,
            this, [this](int) { onFaceSelectionChanged(); });
    connect(_faceType, &QComboBox::currentTextChanged,
            this, &ModelEditDialog::onFaceTypeChanged);
}

void ModelEditDialog::setupStatesTab(QWidget* tab) {
    auto* listPanel = makeListPanel(_stateList, "Add state",
                                    this, SLOT(onStateAdd()), SLOT(onStateDelete()));

    _stateType = new QComboBox;
    _stateType->addItems({"NodeRange", "SingleNode"});

    _stateTable = new QTableWidget(0, 2);
    _stateTable->setHorizontalHeaderLabels({"Key (s001…)", "Color"});
    _stateTable->horizontalHeader()->setStretchLastSection(true);
    _stateTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    _stateTable->verticalHeader()->hide();

    auto* seAddBtn = new QPushButton("+ Row");
    auto* seDelBtn = new QPushButton("– Row");
    connect(seAddBtn, &QPushButton::clicked, this, &ModelEditDialog::onStateEntryAdd);
    connect(seDelBtn, &QPushButton::clicked, this, &ModelEditDialog::onStateEntryDelete);
    auto* seBtns = new QHBoxLayout;
    seBtns->addWidget(seAddBtn); seBtns->addWidget(seDelBtn); seBtns->addStretch();

    auto* editorW = new QWidget;
    auto* edVL    = new QVBoxLayout(editorW);
    auto* typeRow = new QHBoxLayout;
    typeRow->addWidget(new QLabel("Type:"));
    typeRow->addWidget(_stateType);
    typeRow->addStretch();
    edVL->addLayout(typeRow);
    edVL->addWidget(_stateTable, 1);
    edVL->addLayout(seBtns);

    auto* split = new QSplitter(Qt::Horizontal);
    split->addWidget(listPanel);
    split->addWidget(editorW);
    split->setSizes({220, 600});
    split->setStretchFactor(1, 1);

    auto* vl = new QVBoxLayout(tab);
    vl->setContentsMargins(4,4,4,4);
    vl->addWidget(split);

    connect(_stateList, &QListWidget::currentRowChanged,
            this, [this](int) { onStateSelectionChanged(); });
    connect(_stateType, &QComboBox::currentTextChanged,
            this, &ModelEditDialog::onStateTypeChanged);
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

    show();
    raise();
    activateWindow();
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
    _smBufferStyle->setEnabled(valid);
    _smVertical->setEnabled(valid);
    _smRanges->setEnabled(valid);

    if (!valid) {
        _smNameEdit->clear();
        _smRanges->setRowCount(0);
        return;
    }

    const QtSubModelInfo& sm = _subModels[idx];
    _smNameEdit->blockSignals(true);
    _smNameEdit->setText(sm.name);
    _smNameEdit->blockSignals(false);

    _smBufferStyle->blockSignals(true);
    int bsIdx = _smBufferStyle->findText(sm.bufferStyle);
    _smBufferStyle->setCurrentIndex(bsIdx >= 0 ? bsIdx : 0);
    _smBufferStyle->blockSignals(false);

    _smVertical->blockSignals(true);
    _smVertical->setChecked(sm.vertical);
    _smVertical->blockSignals(false);

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
    _faceType->blockSignals(true);
    int tIdx = _faceType->findText(fi.type);
    _faceType->setCurrentIndex(tIdx >= 0 ? tIdx : 0);
    _faceType->blockSignals(false);

    // Show known phoneme keys first, then any extra attrs.
    QStringList keys = kPhonemeKeys;
    for (auto it = fi.attrs.begin(); it != fi.attrs.end(); ++it) {
        const QString& k = it.key();
        if (k == "Name" || k == "Type") continue;
        if (!keys.contains(k)) keys.append(k);
    }

    _faceTable->setRowCount(keys.size());
    for (int r = 0; r < keys.size(); ++r) {
        const QString& k = keys[r];
        _faceTable->setItem(r, 0, new QTableWidgetItem(k));
        _faceTable->item(r, 0)->setFlags(Qt::ItemIsEnabled);
        _faceTable->setItem(r, 1, new QTableWidgetItem(fi.attrs.value(k)));
    }
}

void ModelEditDialog::populateStateEditor(int idx) {
    const bool valid = idx >= 0 && idx < _states.size();
    _stateType->setEnabled(valid);
    _stateTable->setEnabled(valid);
    _stateTable->setRowCount(0);
    if (!valid) return;

    const QtStateInfo& si = _states[idx];
    _stateType->blockSignals(true);
    int tIdx = _stateType->findText(si.type);
    _stateType->setCurrentIndex(tIdx >= 0 ? tIdx : 0);
    _stateType->blockSignals(false);

    const QStringList keys = si.entries.keys();
    _stateTable->setRowCount(keys.size());
    for (int r = 0; r < keys.size(); ++r) {
        _stateTable->setItem(r, 0, new QTableWidgetItem(keys[r]));
        _stateTable->setItem(r, 1, new QTableWidgetItem(si.entries[keys[r]]));
    }
}

// ── Commit helpers (editor → in-memory) ──────────────────────────────────────

void ModelEditDialog::commitCurrentSubModel() {
    if (_curSm < 0 || _curSm >= _subModels.size()) return;
    QtSubModelInfo& sm = _subModels[_curSm];
    sm.name        = _smNameEdit->text().trimmed();
    sm.bufferStyle = _smBufferStyle->currentText();
    sm.vertical    = _smVertical->isChecked();
    sm.ranges.clear();
    for (int r = 0; r < _smRanges->rowCount(); ++r) {
        auto* it = _smRanges->item(r, 0);
        if (it) sm.ranges.append(it->text().trimmed());
    }
}

void ModelEditDialog::commitCurrentFace() {
    if (_curFace < 0 || _curFace >= _faces.size()) return;
    QtFaceInfo& fi = _faces[_curFace];
    fi.type = _faceType->currentText();
    fi.attrs["Name"] = fi.name;
    fi.attrs["Type"] = fi.type;
    for (int r = 0; r < _faceTable->rowCount(); ++r) {
        auto* kIt = _faceTable->item(r, 0);
        auto* vIt = _faceTable->item(r, 1);
        if (kIt && vIt) {
            const QString v = vIt->text().trimmed();
            if (v.isEmpty())
                fi.attrs.remove(kIt->text());
            else
                fi.attrs[kIt->text()] = v;
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
        auto* vIt = _stateTable->item(r, 1);
        if (kIt && vIt && !kIt->text().trimmed().isEmpty())
            si.entries[kIt->text().trimmed()] = vIt->text().trimmed();
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
    sm.name = "NewSubModel" + QString::number(_subModels.size() + 1);
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

void ModelEditDialog::onSmVerticalChanged(bool v) {
    if (_curSm >= 0 && _curSm < _subModels.size())
        _subModels[_curSm].vertical = v;
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
            "Check that the show folder is set and the file is not read-only.");
        return;
    }

    // Reload the sequence so the in-memory QtSequenceInfo reflects the saved data.
    // This also refreshes the house preview and layout window via sequenceLoaded.
    const QString sf = QtXLightsApp::instance().showFolder();
    if (!sf.isEmpty())
        QtXLightsApp::instance().setShowFolder(sf);

    accept();
}

bool ModelEditDialog::saveToXml() {
    const QString sf = QtXLightsApp::instance().showFolder();
    if (sf.isEmpty()) return false;

    // Try both filename conventions.
    QString xmlPath = sf + "/xlights_rgbeffects.xml";
    if (!QFileInfo::exists(xmlPath))
        xmlPath = sf + "/xlights_rgbeffects.xml";

    pugi::xml_document doc;
    pugi::xml_parse_result res = doc.load_file(xmlPath.toStdString().c_str());
    if (!res) {
        spdlog::error("ModelEditDialog: cannot parse '{}'", xmlPath.toStdString());
        return false;
    }

    pugi::xml_node root = doc.child("xrgb");
    if (!root) root = doc.child("xlights_rgbeffects");
    if (!root) return false;

    pugi::xml_node modelsList = root.child("models");
    if (!modelsList) return false;

    // Find the model node by name.
    pugi::xml_node modelNode;
    for (auto m : modelsList.children("model")) {
        if (QString::fromUtf8(m.attribute("name").as_string()) == _modelName) {
            modelNode = m;
            break;
        }
    }
    if (!modelNode) {
        spdlog::error("ModelEditDialog: model '{}' not found in XML",
                      _modelName.toStdString());
        return false;
    }

    // Remove existing sub-model, face, state children.
    std::vector<pugi::xml_node> toRemove;
    for (auto ch : modelNode.children()) {
        const std::string tag = ch.name();
        if (tag == "subModel" || tag == "faceInfo" || tag == "stateInfo")
            toRemove.push_back(ch);
    }
    for (auto& n : toRemove) modelNode.remove_child(n);

    // Write sub-models.
    for (const QtSubModelInfo& sm : _subModels) {
        auto smNode = modelNode.append_child("subModel");
        smNode.append_attribute("name")        = sm.name.toStdString().c_str();
        smNode.append_attribute("vertical")    = sm.vertical ? "1" : "0";
        smNode.append_attribute("isRanges")    = sm.isRanges ? "1" : "0";
        smNode.append_attribute("bufferStyle") = sm.bufferStyle.toStdString().c_str();
        for (const QString& r : sm.ranges) {
            auto sb = smNode.append_child("subBuffer");
            sb.append_attribute("range") = r.toStdString().c_str();
        }
    }

    // Write faces.
    for (const QtFaceInfo& fi : _faces) {
        auto fNode = modelNode.append_child("faceInfo");
        // Write Name and Type first, then remaining attrs.
        fNode.append_attribute("Name") = fi.name.toStdString().c_str();
        fNode.append_attribute("Type") = fi.type.toStdString().c_str();
        for (auto it = fi.attrs.begin(); it != fi.attrs.end(); ++it) {
            if (it.key() == "Name" || it.key() == "Type") continue;
            fNode.append_attribute(it.key().toStdString().c_str()) =
                it.value().toStdString().c_str();
        }
    }

    // Write states.
    for (const QtStateInfo& si : _states) {
        auto sNode = modelNode.append_child("stateInfo");
        sNode.append_attribute("Name") = si.name.toStdString().c_str();
        sNode.append_attribute("Type") = si.type.toStdString().c_str();
        if (si.customColors)
            sNode.append_attribute("CustomColors") = "1";
        for (auto it = si.entries.begin(); it != si.entries.end(); ++it) {
            sNode.append_attribute(it.key().toStdString().c_str()) =
                it.value().toStdString().c_str();
        }
    }

    return doc.save_file(xmlPath.toStdString().c_str());
}

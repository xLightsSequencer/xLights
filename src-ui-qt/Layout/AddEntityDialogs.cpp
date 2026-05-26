#include "AddEntityDialogs.h"

#include "../../src-core/models/ModelManager.h"
#include "../../src-core/models/Model.h"
#include "../../src-core/models/ModelGroup.h"
#include "../../src-core/models/DisplayAsType.h"
#include "../../src-core/outputs/Output.h"
#include "../../src-core/outputs/SerialOutput.h"
#include "../../src-core/controllers/ControllerCaps.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSet>
#include <QSpinBox>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {

QString qstr(const std::string& s) { return QString::fromStdString(s); }

QStringList toQList(const std::list<std::string>& xs) {
    QStringList out;
    out.reserve(static_cast<int>(xs.size()));
    for (const auto& s : xs) out.append(qstr(s));
    return out;
}

// Compute a first-unused name like "Arches_001" given a set of existing names.
QString firstUnusedName(const QString& prefix, const QSet<QString>& existing) {
    for (int i = 1; i < 1000; ++i) {
        const QString name = QString("%1_%2").arg(prefix).arg(i, 3, 10, QChar('0'));
        if (!existing.contains(name)) return name;
    }
    return prefix;   // fallback — extremely unlikely
}

// Model types that the Add Model dialog offers.  Skip groups, sub-models,
// view objects, and "Unknown" — those have their own flows or no flow.
QStringList addableModelTypes() {
    return {
        "Arches", "CandyCanes", "ChannelBlock", "Circle", "Cube",
        "Custom", "DmxMovingHead", "DmxMovingHeadAdv", "DmxFloodArea",
        "DmxFloodlight", "DmxGeneral", "DmxServo", "DmxServo3d", "DmxSkull",
        "Icicles", "Matrix", "MultiPoint", "PolyLine", "SingleLine",
        "Sphere", "Spinner", "Star", "Tree", "WindowFrame", "Wreath",
    };
}

QStringList groupLayoutChoices() {
    return {"Default", "Minimal Grid", "Horizontal Stack",
            "Vertical Stack", "Overlay-Center", "Overlay-Scaled",
            "Per Model Default"};
}

QStringList ethernetProtocolDefaults() {
    return {OUTPUT_E131, OUTPUT_ZCPP, OUTPUT_ARTNET, OUTPUT_DDP,
            OUTPUT_OPC, OUTPUT_KINET, OUTPUT_TWINKLY, OUTPUT_PLAYER_ONLY};
}

} // namespace

// ── AddModelDialog ────────────────────────────────────────────────────────────

AddModelDialog::AddModelDialog(ModelManager* mm, QWidget* parent)
    : QDialog(parent), _mm(mm) {
    setWindowTitle("New Model");
    setModal(true);

    _type        = new QComboBox;
    _type->addItems(addableModelTypes());
    _type->setCurrentText("Arches");

    _name        = new QLineEdit;
    _layoutGroup = new QComboBox;
    _layoutGroup->setEditable(true);
    _layoutGroup->addItems(QStringList{"Default", "Unassigned"});
    if (_mm) {
        for (const auto& g : Model::GetLayoutGroups(*_mm)) {
            const QString s = qstr(g);
            if (_layoutGroup->findText(s) < 0) _layoutGroup->addItem(s);
        }
    }
    _layoutGroup->setCurrentText("Default");

    _startChan   = new QLineEdit("1");
    _startChan->setToolTip("DMX/sACN/etc. start channel; use \"1\" to start at the beginning");

    auto* form = new QFormLayout;
    form->addRow("Type",          _type);
    form->addRow("Name",          _name);
    form->addRow("Layout Group",  _layoutGroup);
    form->addRow("Start Channel", _startChan);

    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(bb);

    // Re-seed the default name whenever the type changes.  Users who've
    // typed their own name keep their entry — we only overwrite empty / the
    // previously-suggested name.
    connect(_type, &QComboBox::currentTextChanged, this, [this](const QString&) {
        seedDefaultName();
    });
    seedDefaultName();
    _name->selectAll();
}

void AddModelDialog::seedDefaultName() {
    QSet<QString> existing;
    if (_mm) {
        for (const auto& [n, _] : *_mm) existing.insert(qstr(n));
    }
    _name->setText(firstUnusedName(_type->currentText(), existing));
}

QString AddModelDialog::typeName()     const { return _type->currentText(); }
QString AddModelDialog::modelName()    const { return _name->text().trimmed(); }
QString AddModelDialog::layoutGroup()  const { return _layoutGroup->currentText().trimmed(); }
QString AddModelDialog::startChannel() const { return _startChan->text().trimmed(); }

// ── AddGroupDialog ────────────────────────────────────────────────────────────

AddGroupDialog::AddGroupDialog(ModelManager* mm, QWidget* parent)
    : QDialog(parent), _mm(mm) {
    setWindowTitle("New Group");
    setModal(true);
    resize(600, 400);

    _name        = new QLineEdit;
    _layout      = new QComboBox;
    _layout->addItems(groupLayoutChoices());

    _layoutGroup = new QComboBox;
    _layoutGroup->setEditable(true);
    _layoutGroup->addItems(QStringList{"Default", "Unassigned"});
    if (_mm) {
        for (const auto& g : Model::GetLayoutGroups(*_mm)) {
            const QString s = qstr(g);
            if (_layoutGroup->findText(s) < 0) _layoutGroup->addItem(s);
        }
    }
    _layoutGroup->setCurrentText("Default");

    _available = new QListWidget;
    _available->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _selected  = new QListWidget;
    _selected->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Populate available models (skip groups themselves — nested groups are
    // possible but rare and not exposed via this dialog).
    QSet<QString> existingGroups;
    if (_mm) {
        for (const auto& [name, m] : *_mm) {
            if (dynamic_cast<ModelGroup*>(m)) {
                existingGroups.insert(qstr(name));
                continue;
            }
            _available->addItem(qstr(name));
        }
    }
    _available->sortItems();
    _name->setText(firstUnusedName("Group", existingGroups));

    auto* form = new QFormLayout;
    form->addRow("Name",         _name);
    form->addRow("Layout",       _layout);
    form->addRow("Layout Group", _layoutGroup);

    auto* addBtn    = new QPushButton(QString::fromUtf8("→"));  // →
    auto* removeBtn = new QPushButton(QString::fromUtf8("←"));  // ←
    addBtn->setToolTip("Add selected models");
    removeBtn->setToolTip("Remove selected members");
    auto* midCol = new QVBoxLayout;
    midCol->addStretch(1);
    midCol->addWidget(addBtn);
    midCol->addWidget(removeBtn);
    midCol->addStretch(1);

    auto* lists = new QGridLayout;
    lists->addWidget(new QLabel("Available"), 0, 0);
    lists->addWidget(new QLabel("Members"),   0, 2);
    lists->addWidget(_available, 1, 0);
    lists->addLayout(midCol,     1, 1);
    lists->addWidget(_selected,  1, 2);
    lists->setColumnStretch(0, 1);
    lists->setColumnStretch(2, 1);

    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addLayout(lists, 1);
    root->addWidget(bb);

    connect(addBtn,    &QPushButton::clicked, this,
            [this]{ moveSelected(_available, _selected); });
    connect(removeBtn, &QPushButton::clicked, this,
            [this]{ moveSelected(_selected, _available); });
    // Double-click moves a single item to the other side.
    connect(_available, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem*){ moveSelected(_available, _selected); });
    connect(_selected,  &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem*){ moveSelected(_selected, _available); });
}

void AddGroupDialog::moveSelected(QListWidget* from, QListWidget* to) {
    for (auto* item : from->selectedItems()) {
        to->addItem(item->text());
        delete from->takeItem(from->row(item));
    }
    to->sortItems();
}

QString AddGroupDialog::groupName()   const { return _name->text().trimmed(); }
QString AddGroupDialog::layout()      const { return _layout->currentText(); }
QString AddGroupDialog::layoutGroup() const { return _layoutGroup->currentText().trimmed(); }

QStringList AddGroupDialog::members() const {
    QStringList out;
    for (int i = 0; i < _selected->count(); ++i)
        out.append(_selected->item(i)->text());
    return out;
}

// ── AddControllerDialog ───────────────────────────────────────────────────────

AddControllerDialog::AddControllerDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("New Controller");
    setModal(true);

    _type     = new QComboBox;
    _type->addItems({"Ethernet", "Serial", "Null"});

    _name     = new QLineEdit("Controller_001");

    _vendor   = new QComboBox;
    _model    = new QComboBox;
    _variant  = new QComboBox;

    _protocol = new QComboBox;
    _protocol->addItems(ethernetProtocolDefaults());

    _ip       = new QLineEdit;
    _ip->setPlaceholderText("e.g. 192.168.1.50");

    _port     = new QComboBox;
    _port->addItems(toQList(SerialOutput::GetPossibleSerialPorts()));

    _speed    = new QComboBox;
    _speed->addItems(toQList(SerialOutput::GetPossibleBaudRates()));

    auto* form = new QFormLayout;
    form->addRow("Type",     _type);
    form->addRow("Name",     _name);
    form->addRow("Vendor",   _vendor);
    form->addRow("Model",    _model);
    form->addRow("Variant",  _variant);
    form->addRow("Protocol", _protocol);
    form->addRow("IP",       _ip);
    form->addRow("Port",     _port);
    form->addRow("Speed",    _speed);

    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(bb);

    connect(_type,   &QComboBox::currentTextChanged, this,
            [this](const QString&){ onTypeChanged(); });
    connect(_vendor, &QComboBox::currentTextChanged, this,
            [this](const QString&){ rebuildModelChoices(); rebuildVariantChoices(); });
    connect(_model,  &QComboBox::currentTextChanged, this,
            [this](const QString&){ rebuildVariantChoices(); });

    onTypeChanged();
}

QString AddControllerDialog::currentTypeString() const {
    return _type->currentText();
}

void AddControllerDialog::onTypeChanged() {
    const bool isEth    = controllerType() == Type::Ethernet;
    const bool isSerial = controllerType() == Type::Serial;
    _protocol->setEnabled(isEth);
    _ip->setEnabled(isEth);
    _port->setEnabled(isSerial);
    _speed->setEnabled(isSerial);
    rebuildVendorChoices();
}

void AddControllerDialog::rebuildVendorChoices() {
    _vendor->blockSignals(true);
    _vendor->clear();
    _vendor->addItems(toQList(ControllerCaps::GetVendors(currentTypeString().toStdString())));
    _vendor->blockSignals(false);
    rebuildModelChoices();
}

void AddControllerDialog::rebuildModelChoices() {
    _model->blockSignals(true);
    _model->clear();
    _model->addItems(toQList(ControllerCaps::GetModels(
        currentTypeString().toStdString(), _vendor->currentText().toStdString())));
    _model->blockSignals(false);
    rebuildVariantChoices();
}

void AddControllerDialog::rebuildVariantChoices() {
    _variant->clear();
    _variant->addItems(toQList(ControllerCaps::GetVariants(
        currentTypeString().toStdString(),
        _vendor->currentText().toStdString(),
        _model->currentText().toStdString())));
}

AddControllerDialog::Type AddControllerDialog::controllerType() const {
    const QString t = _type->currentText();
    if (t == "Serial") return Type::Serial;
    if (t == "Null")   return Type::Null;
    return Type::Ethernet;
}

QString AddControllerDialog::controllerName() const { return _name->text().trimmed(); }
QString AddControllerDialog::vendor()         const { return _vendor->currentText(); }
QString AddControllerDialog::model()          const { return _model->currentText(); }
QString AddControllerDialog::variant()        const { return _variant->currentText(); }

QString AddControllerDialog::protocol() const {
    return controllerType() == Type::Ethernet ? _protocol->currentText() : QString();
}
QString AddControllerDialog::ip() const {
    return controllerType() == Type::Ethernet ? _ip->text().trimmed() : QString();
}
QString AddControllerDialog::port() const {
    return controllerType() == Type::Serial ? _port->currentText() : QString();
}
int AddControllerDialog::speed() const {
    return controllerType() == Type::Serial ? _speed->currentText().toInt() : 0;
}

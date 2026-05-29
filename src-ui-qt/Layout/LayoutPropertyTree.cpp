#include "LayoutPropertyTree.h"

#include "../../src-core/models/Model.h"
#include "../../src-core/models/ModelGroup.h"
#include "../../src-core/models/ModelManager.h"
#include "../../src-core/models/ModelScreenLocation.h"
#include "../../src-core/models/DisplayAsType.h"
#include "../../src-core/models/ControllerConnection.h"
#include "../../src-core/models/ArchesModel.h"
#include "../../src-core/models/CandyCaneModel.h"
#include "../../src-core/models/CubeModel.h"
#include "../../src-core/models/MatrixModel.h"
#include "../../src-core/models/ChannelBlockModel.h"
#include "../../src-core/models/CircleModel.h"
#include "../../src-core/models/CustomModel.h"
#include "../../src-core/models/IciclesModel.h"
#include "../../src-core/models/MultiPointModel.h"
#include "../../src-core/models/PolyLineModel.h"
#include "../../src-core/models/SingleLineModel.h"
#include "../../src-core/models/SphereModel.h"
#include "../../src-core/models/SpinnerModel.h"
#include "../../src-core/models/StarModel.h"
#include "../../src-core/models/TreeModel.h"
#include "../../src-core/models/WindowFrameModel.h"
#include "../../src-core/models/WreathModel.h"
#include "../../src-core/models/DMX/DmxModel.h"
#include "../../src-core/models/DMX/DmxServo.h"
#include "../../src-core/models/DMX/DmxSkull.h"
#include "../../src-core/models/DMX/DmxColorAbility.h"
#include "../../src-core/models/ImageModel.h"
#include "../../src-core/models/LabelModel.h"
#include "../../src-core/outputs/Controller.h"
#include "../../src-core/outputs/ControllerEthernet.h"
#include "../../src-core/outputs/ControllerSerial.h"
#include "../../src-core/outputs/Output.h"
#include "../../src-core/outputs/OutputManager.h"
#include "../../src-core/outputs/SerialOutput.h"
#include "../../src-core/controllers/ControllerCaps.h"
#include "../../src-core/utils/Color.h"

#include <QBrush>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFont>
#include <QHeaderView>
#include <QLineEdit>
#include <QPalette>
#include <QSpinBox>
#include <QStyledItemDelegate>

namespace {

QString qstr(const std::string& s) { return QString::fromStdString(s); }

// Render-type labels match wx ChoiceModelLayoutType.  The first two and
// the per-model variants are stored as special XML values ("grid",
// "minimalGrid", "horizontal", "vertical"); the rest store the display
// label verbatim.  See ModelGroupPanel::OnSaveButtonClick for wx mapping.
QStringList groupRenderTypeOptions() {
    return {
        "Grid as per preview", "Minimal Grid",
        "Horizontal Stack", "Vertical Stack",
        "Horizontal Stack - Scaled", "Vertical Stack - Scaled",
        "Horizontal Per Model", "Vertical Per Model",
        "Horizontal Per Model/Strand", "Vertical Per Model/Strand",
    };
}

QString groupRenderTypeDisplay(const QString& stored) {
    if (stored == "grid")        return "Grid as per preview";
    if (stored == "minimalGrid") return "Minimal Grid";
    if (stored == "horizontal")  return "Horizontal Per Model";
    if (stored == "vertical")    return "Vertical Per Model";
    return stored;
}

QString groupRenderTypeStored(const QString& display) {
    if (display == "Grid as per preview")  return "grid";
    if (display == "Minimal Grid")         return "minimalGrid";
    if (display == "Horizontal Per Model") return "horizontal";
    if (display == "Vertical Per Model")   return "vertical";
    return display;
}

QString hexColor(const xlColor& c) {
    return QString("#%1%2%3")
        .arg(c.red,   2, 16, QChar('0'))
        .arg(c.green, 2, 16, QChar('0'))
        .arg(c.blue,  2, 16, QChar('0')).toUpper();
}

QString pixelStyleLabel(Model::PIXEL_STYLE s) {
    switch (s) {
        case Model::PIXEL_STYLE::PIXEL_STYLE_SQUARE:         return "Square";
        case Model::PIXEL_STYLE::PIXEL_STYLE_SMOOTH:         return "Smooth";
        case Model::PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE:   return "Solid Circle";
        case Model::PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE: return "Blended Circle";
        default:                                             return "Unknown";
    }
}

QString directionLabel(int reverse) {
    return reverse == 0 ? "Forward" : (reverse == 1 ? "Reverse" : QString::number(reverse));
}

// Tag keys on QTreeWidgetItem data.  UserRole+1 is reserved for the dialog
// tab index used by editModelRequested (existing behaviour).
constexpr int kRoleKind        = Qt::UserRole + 10;   // LayoutPropertyTree::Kind enum
constexpr int kRoleFieldId     = Qt::UserRole + 11;   // QString fieldId
constexpr int kRoleEnumOptions = Qt::UserRole + 12;   // QStringList for Kind::Enum

QString activeStateLabel(Controller::ACTIVESTATE s) {
    switch (s) {
        case Controller::ACTIVESTATE::ACTIVE:           return "Active";
        case Controller::ACTIVESTATE::ACTIVEINXLIGHTSONLY:  return "xLights only";
        case Controller::ACTIVESTATE::INACTIVE:         return "Inactive";
        default:                                        return "Unknown";
    }
}

// Delegate that swaps in an editor based on the row's Kind tag.
// Read-only rows (Kind::None or no tag) return nullptr from createEditor —
// QTreeWidget then falls back to no-edit.
class LayoutPropertyDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem&,
                          const QModelIndex& index) const override {
        const auto kind = static_cast<LayoutPropertyTree::Kind>(
            index.data(kRoleKind).toInt());
        switch (kind) {
            case LayoutPropertyTree::Kind::String: {
                return new QLineEdit(parent);
            }
            case LayoutPropertyTree::Kind::Int: {
                auto* s = new QSpinBox(parent);
                s->setRange(0, 100000);
                return s;
            }
            case LayoutPropertyTree::Kind::IntPercent: {
                auto* s = new QSpinBox(parent);
                s->setRange(0, 100);
                s->setSuffix(" %");
                return s;
            }
            case LayoutPropertyTree::Kind::Double: {
                auto* s = new QDoubleSpinBox(parent);
                s->setRange(-100000.0, 100000.0);
                s->setDecimals(3);
                return s;
            }
            case LayoutPropertyTree::Kind::Bool: {
                auto* c = new QComboBox(parent);
                c->addItems({"no", "yes"});
                return c;
            }
            case LayoutPropertyTree::Kind::Enum: {
                auto* c = new QComboBox(parent);
                c->addItems(index.data(kRoleEnumOptions).toStringList());
                return c;
            }
            case LayoutPropertyTree::Kind::Color: {
                // Open the picker immediately, write the result back into the
                // EDIT role, and return nullptr so the view doesn't try to
                // host a child editor widget.
                const QColor initial(index.data(Qt::DisplayRole).toString());
                const QColor c = QColorDialog::getColor(initial.isValid() ? initial : Qt::black,
                                                        parent, "Choose color");
                if (c.isValid())
                    const_cast<QAbstractItemModel*>(index.model())
                        ->setData(index, c.name().toUpper(), Qt::EditRole);
                return nullptr;
            }
            default:
                return nullptr;
        }
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        const auto kind = static_cast<LayoutPropertyTree::Kind>(
            index.data(kRoleKind).toInt());
        const QString text = index.data(Qt::DisplayRole).toString();
        switch (kind) {
            case LayoutPropertyTree::Kind::String:
                static_cast<QLineEdit*>(editor)->setText(text);
                break;
            case LayoutPropertyTree::Kind::Int:
            case LayoutPropertyTree::Kind::IntPercent: {
                QString stripped = text;
                stripped.remove(" %");
                static_cast<QSpinBox*>(editor)->setValue(stripped.toInt());
                break;
            }
            case LayoutPropertyTree::Kind::Double:
                static_cast<QDoubleSpinBox*>(editor)->setValue(text.toDouble());
                break;
            case LayoutPropertyTree::Kind::Bool:
                static_cast<QComboBox*>(editor)->setCurrentIndex(text == "yes" ? 1 : 0);
                break;
            case LayoutPropertyTree::Kind::Enum: {
                auto* c = static_cast<QComboBox*>(editor);
                const int idx = c->findText(text);
                c->setCurrentIndex(idx >= 0 ? idx : 0);
                break;
            }
            default: break;
        }
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        const auto kind = static_cast<LayoutPropertyTree::Kind>(
            index.data(kRoleKind).toInt());
        QVariant out;
        switch (kind) {
            case LayoutPropertyTree::Kind::String:
                out = static_cast<QLineEdit*>(editor)->text();
                break;
            case LayoutPropertyTree::Kind::Int:
                out = static_cast<QSpinBox*>(editor)->value();
                break;
            case LayoutPropertyTree::Kind::IntPercent:
                out = QString::number(static_cast<QSpinBox*>(editor)->value()) + " %";
                break;
            case LayoutPropertyTree::Kind::Double:
                out = static_cast<QDoubleSpinBox*>(editor)->value();
                break;
            case LayoutPropertyTree::Kind::Bool:
                out = static_cast<QComboBox*>(editor)->currentIndex() == 1 ? "yes" : "no";
                break;
            case LayoutPropertyTree::Kind::Enum:
                out = static_cast<QComboBox*>(editor)->currentText();
                break;
            default: return;
        }
        model->setData(index, out, Qt::EditRole);
    }
};

} // namespace

LayoutPropertyTree::LayoutPropertyTree(QWidget* parent) : QTreeWidget(parent) {
    setColumnCount(2);
    setHeaderLabels({"Property", "Value"});
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);
    setRootIsDecorated(true);
    setUniformRowHeights(true);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    // Read-only rows are skipped by the delegate (returns nullptr from
    // createEditor); editable ones get an editor on double-click.
    setEditTriggers(QAbstractItemView::DoubleClicked |
                    QAbstractItemView::EditKeyPressed);
    setItemDelegateForColumn(1, new LayoutPropertyDelegate(this));
    setIndentation(14);

    // Catch successful edits — itemChanged fires after the delegate's
    // setModelData commits.  Route to the appropriate setter on the live
    // src-core object.
    connect(this, &QTreeWidget::itemChanged, this,
            [this](QTreeWidgetItem* item, int col) {
        // Ignore changes emitted while we're building the tree — only real
        // user edits should reach the commit dispatcher.
        if (_populating) return;
        if (col != 1 || !item) return;
        const QString fieldId = item->data(0, kRoleFieldId).toString();
        if (fieldId.isEmpty() || _currentEntity.isEmpty()) return;
        const QString val = item->text(1);
        switch (_currentKind) {
            case EntityKind::Model:
                if (commitModelField(fieldId, val))
                    emit modelChanged(_currentEntity);
                break;
            case EntityKind::Group:
                if (commitGroupField(fieldId, val))
                    emit groupChanged(_currentEntity);
                break;
            case EntityKind::Controller:
                if (commitControllerField(fieldId, val))
                    emit controllerChanged(_currentEntity);
                break;
            default: break;
        }
    });

    // Rows tagged with Qt::UserRole+1 are "editor" rows (Sub-Models / Faces /
    // States).  Double-click emits editModelRequested so LayoutWindow can open
    // ModelEditDialog on the right tab.
    connect(this, &QTreeWidget::itemDoubleClicked, this,
            [this](QTreeWidgetItem* item, int /*col*/) {
        if (!item) return;
        const QVariant v = item->data(0, Qt::UserRole + 1);
        if (!v.isValid()) return;

        // Walk up to find the model name from the populated tree.  The
        // identity category's first row holds the model name.  We re-resolve
        // by asking the parent widget — the tree itself doesn't store it.
        const int tab = v.toInt();
        QTreeWidgetItem* root = topLevelItem(0);
        if (!root || root->childCount() == 0) return;
        QTreeWidgetItem* nameRow = root->child(0);
        if (!nameRow) return;
        emit editModelRequested(nameRow->text(1), tab);
    });
}

void LayoutPropertyTree::setModelManager(ModelManager* mm) {
    _mm = mm;
    _om = mm ? mm->GetOutputManager() : nullptr;
}

void LayoutPropertyTree::clearAll() { clear(); }

void LayoutPropertyTree::showModel(const QString& name) {
    clear();
    _currentEntity.clear();
    _currentKind = EntityKind::None;
    if (!_mm || name.isEmpty()) return;

    Model* m = nullptr;
    try { m = _mm->GetModel(name.toStdString()); }
    catch (...) { m = nullptr; }
    if (!m) return;

    // Skip groups here — caller should use showGroup() for those.
    // Use GetDisplayAs() rather than dynamic_cast: it reads the model's
    // stored type enum and doesn't depend on RTTI behaving across the
    // separately-compiled Qt translation units.
    if (m->GetDisplayAs() == DisplayAsType::ModelGroup) return;

    _currentEntity = name;
    _currentKind = EntityKind::Model;

    _populating = true;
    populateModelIdentity(m);
    populateModelTypeProperties(m);
    populateModelSizingChannels(m);
    populateModelLayout(m);
    populateModelAppearance(m);
    populateModelStringProperties(m);
    populateModelControllerConnection(m);
    populateModelAuxiliary(m);
    _populating = false;
    expandAll();
    // Rebuilding can leave the viewport scrolled down from the previous
    // model, hiding the top categories.  Reset to the top.
    scrollToTop();
}

void LayoutPropertyTree::showGroup(const QString& name, const QtModelGroupInfo& fallback) {
    clear();
    _currentEntity.clear();
    _currentKind = EntityKind::None;
    if (name.isEmpty()) return;

    ModelGroup* g = nullptr;
    if (_mm) {
        try {
            Model* m = _mm->GetModel(name.toStdString());
            // GetDisplayAs() instead of dynamic_cast — see showModel.
            if (m && m->GetDisplayAs() == DisplayAsType::ModelGroup)
                g = static_cast<ModelGroup*>(m);
        } catch (...) { g = nullptr; }
    }

    _currentEntity = name;
    _currentKind = EntityKind::Group;

    _populating = true;
    if (g) {
        populateGroupIdentity(g);
        populateGroupBuffer(g);
        populateGroupBounds(g);
        populateGroupAppearance(g);
        populateGroupMembers(g);
    } else if (fallback.isValid()) {
        // ModelManager doesn't have the group — happens when the bridge
        // hasn't initialised yet, or when LoadGroups couldn't resolve the
        // members.  Fall back to the QtSequenceDoc snapshot so the user
        // still sees the group's properties (edits won't work in this
        // path — they require the live ModelGroup*).
        populateGroupFromInfo(fallback);
    } else {
        // No live group + no fallback snapshot — surface that as a row so
        // the empty tree doesn't look like a UI bug.
        auto* cat = addCategory("Model Group");
        addRow(cat, "Name", name);
        addRow(cat, "(not loaded)", "group is not in ModelManager");
    }
    _populating = false;
    expandAll();
    scrollToTop();
}

void LayoutPropertyTree::showController(const QString& name) {
    clear();
    _currentEntity.clear();
    _currentKind = EntityKind::None;
    if (!_om || name.isEmpty()) return;

    Controller* c = nullptr;
    try { c = _om->GetController(name.toStdString()); }
    catch (...) { c = nullptr; }
    if (!c) return;

    _currentEntity = name;
    _currentKind = EntityKind::Controller;

    _populating = true;
    populateControllerIdentity(c);
    populateControllerNetwork(c);
    populateControllerOutput(c);
    populateControllerCapabilities(c);
    populateControllerOutputs(c);
    _populating = false;
    expandAll();
    scrollToTop();
}

QTreeWidgetItem* LayoutPropertyTree::addCategory(const QString& label) {
    auto* cat = new QTreeWidgetItem(this);
    cat->setText(0, label);
    cat->setFirstColumnSpanned(true);
    QFont f = cat->font(0);
    f.setBold(true);
    cat->setFont(0, f);
    cat->setBackground(0, palette().alternateBase());
    cat->setExpanded(true);
    return cat;
}

QTreeWidgetItem* LayoutPropertyTree::addRow(QTreeWidgetItem* category,
                                            const QString& label,
                                            const QString& value) {
    auto* row = new QTreeWidgetItem(category);
    row->setText(0, label);
    row->setText(1, value);
    // Block itemChanged on read-only rows (would otherwise fire during
    // addEditableRow's own setItem calls if a downstream class adds an item).
    row->setFlags(row->flags() & ~Qt::ItemIsEditable);
    return row;
}

QTreeWidgetItem* LayoutPropertyTree::addEditableRow(QTreeWidgetItem* category,
                                                    const QString& label,
                                                    const QString& value,
                                                    Kind kind,
                                                    const QString& fieldId,
                                                    const QStringList& enumOptions) {
    // Suppress itemChanged while we initialise the row; only the user's
    // subsequent edit should reach the commit dispatcher.
    blockSignals(true);
    auto* row = new QTreeWidgetItem(category);
    row->setText(0, label);
    row->setText(1, value);
    row->setData(0, kRoleKind,    static_cast<int>(kind));
    row->setData(0, kRoleFieldId, fieldId);
    row->setData(1, kRoleKind,    static_cast<int>(kind));   // delegate reads from col 1
    if (!enumOptions.isEmpty())
        row->setData(1, kRoleEnumOptions, enumOptions);
    Qt::ItemFlags f = row->flags();
    f |= Qt::ItemIsEditable;
    row->setFlags(f);
    // Subtle visual hint that the value cell is editable.
    QFont font = row->font(1);
    font.setUnderline(true);
    row->setFont(1, font);
    blockSignals(false);
    return row;
}

// ── Commit (Model only — group/controller editing comes later) ───────────────

bool LayoutPropertyTree::commitModelField(const QString& fieldId, const QVariant& value) {
    if (!_mm) return false;
    Model* m = nullptr;
    try { m = _mm->GetModel(_currentEntity.toStdString()); }
    catch (...) { m = nullptr; }
    if (!m) return false;

    auto pctToInt = [](const QString& s) {
        QString stripped = s; stripped.remove(" %"); return stripped.toInt();
    };

    if (fieldId == "Description") {
        m->SetDescription(value.toString().toStdString());
        return true;
    }
    if (fieldId == "Active") {
        m->SetActive(value.toString() == "yes");
        return true;
    }
    if (fieldId == "TagColor") {
        const QColor c(value.toString());
        if (c.isValid()) {
            m->SetTagColour(xlColor(c.red(), c.green(), c.blue()));
            return true;
        }
        return false;
    }
    if (fieldId == "PixelSize") {
        m->SetPixelSize(value.toInt());
        return true;
    }
    if (fieldId == "Transparency") {
        m->SetTransparency(pctToInt(value.toString()));
        return true;
    }
    if (fieldId == "BlackTransparency") {
        m->SetBlackTransparency(pctToInt(value.toString()));
        return true;
    }
    if (fieldId == "LayoutGroup") {
        m->SetLayoutGroup(value.toString().toStdString());
        return true;
    }
    if (fieldId == "StringType") {
        m->SetStringType(value.toString().toStdString());
        return true;
    }
    if (fieldId == "RGBWHandling") {
        m->SetRGBWHandling(value.toString().toStdString());
        return true;
    }
    if (fieldId == "CustomColor") {
        // The Color delegate writes a #RRGGBB string, which SetCustomColor
        // accepts directly.
        const QColor c(value.toString());
        if (!c.isValid()) return false;
        m->SetCustomColor(value.toString().toStdString());
        return true;
    }

    // ── Controller connection (common to all models) ───────────────────
    if (fieldId == "CtrlPort")       { m->SetControllerPort(value.toInt()); return true; }
    if (fieldId == "CtrlProtocol")   { m->SetControllerProtocol(value.toString().toStdString()); return true; }
    if (fieldId == "DMXChannel")     { m->SetControllerDMXChannel(value.toInt()); return true; }
    if (fieldId == "StartNulls")     { m->SetControllerStartNulls(value.toInt()); return true; }
    if (fieldId == "EndNulls")       { m->SetControllerEndNulls(value.toInt()); return true; }
    if (fieldId == "CtrlBrightness") { m->SetControllerBrightness(pctToInt(value.toString())); return true; }
    if (fieldId == "CtrlGamma")      { m->SetControllerGamma(value.toFloat()); return true; }
    if (fieldId == "ColorOrder")     { m->SetControllerColorOrder(value.toString().toStdString()); return true; }
    if (fieldId == "GroupCount")     { m->SetControllerGroupCount(value.toInt()); return true; }
    if (fieldId == "CtrlZigZag")     { m->SetControllerZigZag(value.toInt()); return true; }
    if (fieldId == "CtrlDirection")  { m->SetControllerReverse(value.toString() == "Reverse" ? 1 : 0); return true; }

    // ── Smart Remote ───────────────────────────────────────────────────
    if (fieldId == "UseSmartRemote") {
        const bool on = value.toString() == "yes";
        // Enabling with no remote yet selected defaults to A (1); disabling
        // clears to 0.  SetSmartRemote drives the USE_SMART_REMOTE property.
        m->SetSmartRemote(on ? (m->GetSmartRemote() > 0 ? m->GetSmartRemote() : 1) : 0);
        return true;
    }
    if (fieldId == "SmartRemoteType") {
        m->SetSmartRemoteType(value.toString().toStdString());
        return true;
    }
    if (fieldId == "SmartRemote") {
        // The combo shows the GetSmartRemoteValues() letters; the model wants
        // the 1-based index (A=1).  Find the chosen value's position.
        auto* caps = m->GetControllerCaps();
        const int count = caps ? caps->GetSmartRemoteCount() : 15;
        const auto vals = m->GetSmartRemoteValues(count);
        for (int i = 0; i < (int)vals.size(); ++i) {
            if (qstr(vals[i]) == value.toString()) { m->SetSmartRemote(i + 1); return true; }
        }
        return false;
    }
    if (fieldId == "SRMaxCascade")    { m->SetSRMaxCascade(value.toInt()); return true; }
    if (fieldId == "SRCascadeOnPort") { m->SetSRCascadeOnPort(value.toString() == "yes"); return true; }

    // Screen-location edits (World / Scale / Rotation).  Scale setters take
    // the whole vec3, so read-modify-write the single axis.
    auto& loc = m->GetModelScreenLocation();
    if (fieldId == "WorldX") { loc.SetWorldPos_X(value.toFloat()); return true; }
    if (fieldId == "WorldY") { loc.SetWorldPos_Y(value.toFloat()); return true; }
    if (fieldId == "WorldZ") { loc.SetWorldPos_Z(value.toFloat()); return true; }
    if (fieldId == "ScaleX" || fieldId == "ScaleY" || fieldId == "ScaleZ") {
        glm::vec3 s = loc.GetScaleMatrix();
        const float v = value.toFloat();
        if (fieldId == "ScaleX") s.x = v;
        else if (fieldId == "ScaleY") s.y = v;
        else s.z = v;
        loc.SetScaleMatrix(s);
        return true;
    }
    if (fieldId == "RotateX") { loc.SetRotateX(value.toFloat()); return true; }
    if (fieldId == "RotateY") { loc.SetRotateY(value.toFloat()); return true; }
    if (fieldId == "RotateZ") { loc.SetRotateZ(value.toFloat()); return true; }

    // ── Per-type model parameter edits ─────────────────────────────────
    // Dispatch on GetDisplayAs() + static_cast.  Each case handles the
    // fieldIds emitted by the matching populateModelTypeProperties branch.
    const bool b = value.toString() == "yes";
    const int  iv = value.toInt();
    const float fv = value.toFloat();
    switch (m->GetDisplayAs()) {
        case DisplayAsType::Matrix:
        case DisplayAsType::Sphere: {
            auto* mat = static_cast<MatrixModel*>(m);
            if (fieldId == "Direction")       { mat->SetVertical(value.toString() == "Vertical"); return true; }
            if (fieldId == "NodesPerString")  { mat->SetNodesPerString(iv);  return true; }
            if (fieldId == "StrandsPerString"){ mat->SetStrandsPerString(iv); return true; }
            if (fieldId == "AlternateNodes")  { mat->SetAlternateNodes(b);   return true; }
            if (fieldId == "NoZigZag")        { mat->SetNoZigZag(b);         return true; }
            if (m->GetDisplayAs() == DisplayAsType::Sphere) {
                auto* sp = static_cast<SphereModel*>(m);
                if (fieldId == "StartLatitude") { sp->SetStartLatitude(iv); return true; }
                if (fieldId == "EndLatitude")   { sp->SetEndLatitude(iv);   return true; }
                if (fieldId == "SphereDegrees") { sp->SetDegrees(iv);       return true; }
            }
            break;
        }
        case DisplayAsType::Arches: {
            auto* a = static_cast<ArchesModel*>(m);
            if (fieldId == "NodesPerArch")  { a->SetNodesPerArch(iv);  return true; }
            if (fieldId == "LightsPerNode") { a->SetLightsPerNode(iv); return true; }
            if (fieldId == "Arc")           { a->SetArc(iv);           return true; }
            if (fieldId == "Hollow")        { a->SetHollow(iv);        return true; }
            if (fieldId == "Gap")           { a->SetGap(iv);           return true; }
            if (fieldId == "ZigZag")        { a->SetZigZag(b);         return true; }
            break;
        }
        case DisplayAsType::Tree: {
            auto* tr = static_cast<TreeModel*>(m);
            if (fieldId == "TreeType")        { tr->SetTreeType(iv);              return true; }
            if (fieldId == "TreeDegrees")     { tr->SetTreeDegrees((long)iv);     return true; }
            if (fieldId == "TreeRotation")    { tr->SetTreeRotation(fv);          return true; }
            if (fieldId == "SpiralRotations") { tr->SetTreeSpiralRotations(fv);   return true; }
            if (fieldId == "BottomTopRatio")  { tr->SetTreeBottomTopRatio(fv);    return true; }
            if (fieldId == "Perspective")     { tr->SetPerspective(fv);           return true; }
            if (fieldId == "FirstStrand")     { tr->SetFirstStrand(iv);           return true; }
            break;
        }
        case DisplayAsType::Star: {
            auto* st = static_cast<StarModel*>(m);
            if (fieldId == "StarRatio")    { st->SetStarRatio(fv);    return true; }
            if (fieldId == "InnerPercent") { st->SetInnerPercent(iv); return true; }
            break;
        }
        case DisplayAsType::Cube: {
            auto* cu = static_cast<CubeModel*>(m);
            if (fieldId == "CubeWidth")      { cu->SetCubeWidth(iv);       return true; }
            if (fieldId == "CubeHeight")     { cu->SetCubeHeight(iv);      return true; }
            if (fieldId == "CubeDepth")      { cu->SetCubeDepth(iv);       return true; }
            if (fieldId == "CubeStrings")    { cu->SetCubeStrings(iv);     return true; }
            if (fieldId == "StrandPerLayer") { cu->SetStrandPerLayer(b);   return true; }
            break;
        }
        case DisplayAsType::CandyCanes: {
            auto* cc = static_cast<CandyCaneModel*>(m);
            if (fieldId == "NumCanes")      { cc->SetNumCanes(iv);      return true; }
            if (fieldId == "LightsPerNode") { cc->SetLightsPerNode(iv); return true; }
            break;
        }
        case DisplayAsType::PolyLine: {
            auto* pl = static_cast<PolyLineModel*>(m);
            if (fieldId == "NumStrings")     { pl->SetNumStrings(iv);     return true; }
            if (fieldId == "LightsPerNode")  { pl->SetLightsPerNode(iv);  return true; }
            if (fieldId == "ModelHeight")    { pl->SetModelHeight(fv);    return true; }
            if (fieldId == "AlternateNodes") { pl->SetAlternateNodes(b);  return true; }
            break;
        }
        case DisplayAsType::WindowFrame: {
            auto* wf = static_cast<WindowFrameModel*>(m);
            if (fieldId == "Rotation") { wf->SetRotation(iv); return true; }
            break;
        }
        case DisplayAsType::Spinner: {
            auto* sp = static_cast<SpinnerModel*>(m);
            if (fieldId == "ArmsPerString") { sp->SetArmsPerString(iv); return true; }
            if (fieldId == "NodesPerArm")   { sp->SetNodesPerArm(iv);   return true; }
            if (fieldId == "Hollow")        { sp->SetHollow(iv);        return true; }
            if (fieldId == "Arc")           { sp->SetArc(iv);           return true; }
            if (fieldId == "StartAngle")    { sp->SetStartAngle(iv);    return true; }
            if (fieldId == "ZigZag")        { sp->SetZigZag(b);         return true; }
            if (fieldId == "Alternate")     { sp->SetAlternate(b);      return true; }
            break;
        }
        case DisplayAsType::Circle: {
            auto* ci = static_cast<CircleModel*>(m);
            if (fieldId == "NodesPerString") { ci->SetCircleNodesPerString(iv); return true; }
            if (fieldId == "CenterPercent")  { ci->SetCenterPercent(iv);        return true; }
            if (fieldId == "InsideOut")      { ci->SetInsideOut(b);             return true; }
            break;
        }
        case DisplayAsType::Icicles: {
            auto* ic = static_cast<IciclesModel*>(m);
            if (fieldId == "LightsPerString") { ic->SetLightsPerString(iv);          return true; }
            if (fieldId == "DropPattern")     { ic->SetDropPattern(value.toString().toStdString()); return true; }
            if (fieldId == "AlternateNodes")  { ic->SetAlternateNodes(b);            return true; }
            break;
        }
        case DisplayAsType::Custom: {
            auto* cm = static_cast<CustomModel*>(m);
            if (fieldId == "Background")    { cm->SetCustomBackground(value.toString().toStdString()); return true; }
            if (fieldId == "Lightness")     { cm->SetCustomLightness(iv);     return true; }
            if (fieldId == "BkgScale")      { cm->SetCustomBkgScale(iv);      return true; }
            if (fieldId == "BkgBrightness") { cm->SetCustomBkgBrightness(iv); return true; }
            break;
        }
        case DisplayAsType::SingleLine: {
            auto* sl = static_cast<SingleLineModel*>(m);
            if (fieldId == "NumLines")     { sl->SetNumLines(iv);      return true; }
            if (fieldId == "NodesPerLine") { sl->SetNodesPerLine(iv);  return true; }
            if (fieldId == "LightsPerNode"){ sl->SetLightsPerNode(iv); return true; }
            break;
        }
        case DisplayAsType::MultiPoint: {
            auto* mp = static_cast<MultiPointModel*>(m);
            if (fieldId == "ModelHeight") { mp->SetModelHeight(fv); return true; }
            break;
        }
        default: break;
    }

    // # Strings edits — dispatch per model type via GetDisplayAs() +
    // static_cast (dynamic_cast was unreliable in the Qt build — see
    // populateModelTypeProperties).
    if (fieldId == "NumStrings") {
        const int n = value.toInt();
        if (n <= 0) return false;
        switch (m->GetDisplayAs()) {
            case DisplayAsType::Matrix:
            case DisplayAsType::Sphere:
                static_cast<MatrixModel*>(m)->SetNumMatrixStrings(n); return true;
            case DisplayAsType::Arches:
                static_cast<ArchesModel*>(m)->SetNumArches(n); return true;
            case DisplayAsType::Spinner:
                static_cast<SpinnerModel*>(m)->SetNumSpinnerStrings(n); return true;
            case DisplayAsType::Circle:
                static_cast<CircleModel*>(m)->SetNumCircleStrings(n); return true;
            case DisplayAsType::Icicles:
                static_cast<IciclesModel*>(m)->SetNumIcicleStrings(n); return true;
            case DisplayAsType::ChannelBlock:
                static_cast<ChannelBlockModel*>(m)->SetNumChannels(n); return true;
            case DisplayAsType::Custom:
                static_cast<CustomModel*>(m)->SetNumStrings(n); return true;
            case DisplayAsType::SingleLine:
                static_cast<SingleLineModel*>(m)->SetNumLines(n); return true;
            case DisplayAsType::MultiPoint:
                static_cast<MultiPointModel*>(m)->SetNumStrings(n); return true;
            default:
                if (IsDmxDisplayType(m->GetDisplayAs())) {
                    static_cast<DmxModel*>(m)->SetDmxChannelCount(n); return true;
                }
                return false;
        }
    }

    return false;
}

bool LayoutPropertyTree::commitGroupField(const QString& fieldId, const QVariant& value) {
    if (!_mm) return false;
    ModelGroup* g = nullptr;
    try {
        Model* m = _mm->GetModel(_currentEntity.toStdString());
        if (m && m->GetDisplayAs() == DisplayAsType::ModelGroup)
            g = static_cast<ModelGroup*>(m);
    } catch (...) { return false; }
    if (!g) return false;

    if (fieldId == "Active")        { g->SetActive(value.toString() == "yes"); return true; }
    if (fieldId == "LayoutGroup")   { g->SetLayoutGroup(value.toString().toStdString()); return true; }
    if (fieldId == "Layout") {
        // The combo shows display labels; XML/setter wants the stored form
        // ("minimalGrid" etc.) for the four special values.  See the
        // groupRenderType{Display,Stored} pair next to populateGroupIdentity.
        g->SetLayout(groupRenderTypeStored(value.toString()).toStdString());
        return true;
    }
    if (fieldId == "DefaultCamera") { g->SetDefaultCamera(value.toString().toStdString()); return true; }
    if (fieldId == "TagColor") {
        const QColor c(value.toString());
        if (c.isValid()) {
            g->SetTagColour(xlColor(c.red(), c.green(), c.blue()));
            return true;
        }
        return false;
    }
    return false;
}

bool LayoutPropertyTree::commitControllerField(const QString& fieldId, const QVariant& value) {
    if (!_om) return false;
    Controller* c = nullptr;
    try { c = _om->GetController(_currentEntity.toStdString()); }
    catch (...) { return false; }
    if (!c) return false;

    if (fieldId == "Name") {
        const QString newName = value.toString().trimmed();
        if (newName.isEmpty() || newName == _currentEntity) return false;
        // Refuse the rename if another controller already owns the new name —
        // OutputManager keys by name and would otherwise silently collide.
        if (_om->GetController(newName.toStdString()) != nullptr) return false;
        c->SetName(newName.toStdString());
        _currentEntity = newName;
        return true;
    }
    if (fieldId == "Description")  { c->SetDescription(value.toString().toStdString()); return true; }
    if (fieldId == "Active")       { c->SetActive(value.toString().toStdString());      return true; }
    if (fieldId == "AutoLayout")   { c->SetAutoLayout(value.toString() == "yes");        return true; }
    if (fieldId == "AutoSize")     { c->SetAutoSize(value.toString() == "yes", nullptr); return true; }
    if (fieldId == "AutoUpload")   { c->SetAutoUpload(value.toString() == "yes");        return true; }
    if (fieldId == "Vendor")       { c->SetVendor(value.toString().toStdString());       return true; }
    if (fieldId == "Model")        { c->SetModel(value.toString().toStdString());        return true; }
    if (fieldId == "Variant")      { c->SetVariant(value.toString().toStdString());      return true; }
    if (fieldId == "SuppressDup")  { c->SetSuppressDuplicateFrames(value.toString() == "yes"); return true; }
    if (fieldId == "Monitor")      { c->SetMonitoring(value.toString() == "yes");        return true; }

    if (auto* e = dynamic_cast<ControllerEthernet*>(c)) {
        if (fieldId == "IP")       { e->SetIP(value.toString().toStdString());       return true; }
        if (fieldId == "FPPProxy") { e->SetFPPProxy(value.toString().toStdString()); return true; }
        if (fieldId == "Priority") { e->SetPriority(value.toInt());                  return true; }
        if (fieldId == "Protocol") { e->SetProtocol(value.toString().toStdString()); return true; }
    } else if (auto* s = dynamic_cast<ControllerSerial*>(c)) {
        if (fieldId == "Port")     { s->SetPort(value.toString().toStdString());     return true; }
        if (fieldId == "Speed")    { s->SetSpeed(value.toInt());                     return true; }
    }
    return false;
}

// ── Model ─────────────────────────────────────────────────────────────────────

void LayoutPropertyTree::populateModelIdentity(Model* m) {
    // Category is named "Model" (not the type) so it doesn't collide with the
    // per-type category that populateModelTypeProperties adds.
    auto* cat = addCategory("Model");
    addRow(cat, "Name", qstr(m->GetName()));
    addRow(cat, "Type", qstr(DisplayAsTypeToString(m->GetDisplayAs())));
    addEditableRow(cat, "Description", qstr(m->GetDescription()),
                   Kind::String, "Description");
}

void LayoutPropertyTree::populateModelSizingChannels(Model* m) {
    auto* cat = addCategory("Sizing & Channels");
    addRow(cat, "Start Channel",   qstr(m->GetModelStartChannel()));
    addRow(cat, "Nodes",           QString::number(m->GetNodeCount()));
    addRow(cat, "Channels/Node",   QString::number(m->GetChanCountPerNode()));
    addRow(cat, "Channels",        QString::number(m->GetNumChannels()));
    addRow(cat, "Active Channels", QString::number(m->GetActChanCount()));
    addRow(cat, "Strings",         QString::number(m->GetNumStrings()));
    addRow(cat, "Strands",         QString::number(m->GetNumStrands()));
    addRow(cat, "Buffer W",        QString::number(m->GetDefaultBufferWi()));
    addRow(cat, "Buffer H",        QString::number(m->GetDefaultBufferHt()));
    if (m->SupportsLowDefinitionRender())
        addRow(cat, "Low Def Factor", QString::number(m->GetLowDefFactor()));
    if (m->HasIndividualStartChannels())
        addRow(cat, "Indiv Start Chans", "yes");
}

void LayoutPropertyTree::populateModelLayout(Model* m) {
    auto* cat = addCategory("Layout");

    // Layout Group as an editable combo populated from the live layout-group
    // list (plus Default / Unassigned).  Pre-pend the current value if it's
    // not already in the list so the row keeps a valid selection.
    QStringList groups{"Default", "Unassigned"};
    if (_mm) {
        for (const auto& g : Model::GetLayoutGroups(*_mm)) {
            const QString s = qstr(g);
            if (!groups.contains(s)) groups << s;
        }
    }
    const QString curGroup = qstr(m->GetLayoutGroup());
    if (!curGroup.isEmpty() && !groups.contains(curGroup)) groups.prepend(curGroup);
    addEditableRow(cat, "Layout Group", curGroup, Kind::Enum, "LayoutGroup", groups);

    addEditableRow(cat, "Active", m->IsActive() ? "yes" : "no",
                   Kind::Bool, "Active");

    const auto& loc = m->GetModelScreenLocation();
    addEditableRow(cat, "World X", QString::number(loc.GetWorldPos_X(), 'f', 2),
                   Kind::Double, "WorldX");
    addEditableRow(cat, "World Y", QString::number(loc.GetWorldPos_Y(), 'f', 2),
                   Kind::Double, "WorldY");
    addEditableRow(cat, "World Z", QString::number(loc.GetWorldPos_Z(), 'f', 2),
                   Kind::Double, "WorldZ");
    const glm::vec3 s = loc.GetScaleMatrix();
    addEditableRow(cat, "Scale X", QString::number(s.x, 'f', 3), Kind::Double, "ScaleX");
    addEditableRow(cat, "Scale Y", QString::number(s.y, 'f', 3), Kind::Double, "ScaleY");
    addEditableRow(cat, "Scale Z", QString::number(s.z, 'f', 3), Kind::Double, "ScaleZ");
    // Rotation uses the rotatex/y/z trio (Get/SetRotateX/Y/Z) so display and
    // edit are consistent — GetRotationAngles() reads a different field.
    addEditableRow(cat, "Rotation X", QString::number(loc.GetRotateX(), 'f', 1),
                   Kind::Double, "RotateX");
    addEditableRow(cat, "Rotation Y", QString::number(loc.GetRotateY(), 'f', 1),
                   Kind::Double, "RotateY");
    addEditableRow(cat, "Rotation Z", QString::number(loc.GetRotateZ(), 'f', 1),
                   Kind::Double, "RotateZ");
}

void LayoutPropertyTree::populateModelAppearance(Model* m) {
    auto* cat = addCategory("Appearance");
    addEditableRow(cat, "Pixel Size", QString::number(m->GetPixelSize()),
                   Kind::Int, "PixelSize");
    addRow(cat, "Pixel Style", pixelStyleLabel(m->GetPixelStyle()));
    addEditableRow(cat, "Transparency",
                   QString::number(m->GetTransparency()) + " %",
                   Kind::IntPercent, "Transparency");
    addEditableRow(cat, "Black Transparency",
                   QString::number(m->GetBlackTransparency()) + " %",
                   Kind::IntPercent, "BlackTransparency");
    auto* row = addEditableRow(cat, "Tag Color", hexColor(m->GetTagColour()),
                               Kind::Color, "TagColor");
    const xlColor tc = m->GetTagColour();
    row->setBackground(1, QBrush(QColor(tc.red, tc.green, tc.blue)));
    const std::string smf = m->GetShadowModelFor();
    if (!smf.empty())
        addRow(cat, "Shadow Model For", qstr(smf));
}

void LayoutPropertyTree::populateModelStringProperties(Model* m) {
    auto* cat = addCategory("String Properties");

    static const QStringList kNodeTypes = {
        "RGB Nodes", "RBG Nodes", "GBR Nodes", "GRB Nodes",
        "BRG Nodes", "BGR Nodes", "Node Single Color", "3 Channel RGB",
        "4 Channel RGBW", "4 Channel WRGB", "Strobes", "Single Color",
        "Single Color Intensity", "Superstring", "WRGB Nodes", "WRBG Nodes",
        "WGBR Nodes", "WGRB Nodes", "WBRG Nodes", "WBGR Nodes", "RGBW Nodes",
        "RBGW Nodes", "GBRW Nodes", "GRBW Nodes", "BRGW Nodes", "BGRW Nodes",
        "RGBWW Nodes"
    };
    QStringList stringTypes = kNodeTypes;
    const QString curType = qstr(m->GetStringType());
    if (!curType.isEmpty() && !stringTypes.contains(curType)) stringTypes.prepend(curType);
    addEditableRow(cat, "String Type", curType, Kind::Enum, "StringType", stringTypes);

    auto* row = addEditableRow(cat, "Custom Color", hexColor(m->GetCustomColor()),
                               Kind::Color, "CustomColor");
    const xlColor cc = m->GetCustomColor();
    row->setBackground(1, QBrush(QColor(cc.red, cc.green, cc.blue)));

    addEditableRow(cat, "RGBW Handling", qstr(m->GetRGBWHandling()), Kind::Enum,
                   "RGBWHandling",
                   {"R=G=B -> W", "RGB Only", "White Only", "Advanced", "White On All"});
    addRow(cat, "RGB Order", qstr(m->GetRGBOrder()));   // no setter — read-only
}

// Add an editable value child under a "Set X" parent.  When the property
// isn't active (the model inherits the controller default) the child is
// dimmed, but it stays editable so the user can override.
QTreeWidgetItem* LayoutPropertyTree::addCtrlChild(QTreeWidgetItem* parent,
                                                  const QString& label,
                                                  const QString& value,
                                                  bool active,
                                                  Kind kind,
                                                  const QString& fieldId,
                                                  const QStringList& enumOptions) {
    auto* row = addEditableRow(parent, label, value, kind, fieldId, enumOptions);
    if (!active) {
        const QColor dim = palette().color(QPalette::Disabled, QPalette::Text);
        row->setForeground(0, dim);
        row->setForeground(1, dim);
    }
    parent->setExpanded(active);
    return row;
}

void LayoutPropertyTree::populateModelControllerConnection(Model* m) {
    auto* cat = addCategory("Controller Connection");

    const std::string ctrl = m->GetControllerName();
    addRow(cat, "Controller", ctrl.empty() ? "(use start channel)" : qstr(ctrl));
    addEditableRow(cat, "Port", QString::number(m->GetControllerPort(1)),
                   Kind::Int, "CtrlPort");
    // Protocol options from the model's available protocol list.
    {
        std::vector<std::string> cp;
        int idx = -1;
        m->GetControllerProtocols(cp, idx);
        QStringList protos;
        for (const auto& s : cp) protos << qstr(s);
        const QString cur = qstr(m->GetControllerProtocol());
        if (!cur.isEmpty() && !protos.contains(cur)) protos.prepend(cur);
        if (protos.isEmpty()) protos << cur;
        addEditableRow(cat, "Protocol", cur, Kind::Enum, "CtrlProtocol", protos);
    }

    auto* caps = m->GetControllerCaps();
    const bool pixel  = m->IsPixelProtocol();
    const bool serial = m->IsSerialProtocol();
    const bool pwm    = false; // IsPWMProtocol exists but most models don't expose it via Model

    // ── Smart Remote chain (pixel proto with smart-remote support) ──────
    if (pixel) {
        int smartRemoteCount = caps ? caps->GetSmartRemoteCount() : 15;
        if (smartRemoteCount != 0) {
            const bool useSR = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::USE_SMART_REMOTE);
            auto* parent = addEditableRow(cat, "Use Smart Remote", useSR ? "yes" : "no",
                                          Kind::Bool, "UseSmartRemote");
            if (useSR) {
                // Smart Remote Type — enum from the controller's available
                // types (when more than one).
                const auto srTypes = m->GetSmartRemoteTypes();
                if (srTypes.size() > 1) {
                    QStringList types;
                    for (const auto& tp : srTypes) types << qstr(tp);
                    addCtrlChild(parent, "Smart Remote Type", qstr(m->GetSmartRemoteType()),
                                 true, Kind::Enum, "SmartRemoteType", types);
                } else {
                    addRow(parent, "Smart Remote Type", qstr(m->GetSmartRemoteType()));
                }

                // Smart Remote A/B/C… — enum of the letter values; the model
                // stores it 1-based (A=1), so the option index maps to sr-1.
                QStringList srValues;
                for (const auto& v : m->GetSmartRemoteValues(smartRemoteCount))
                    srValues << qstr(v);
                const int sr = m->GetSmartRemote();
                const QString cur = (sr > 0 && sr - 1 < srValues.size())
                                      ? srValues[sr - 1]
                                      : (srValues.isEmpty() ? QString() : srValues.first());
                addCtrlChild(parent, "Smart Remote", cur, true,
                             Kind::Enum, "SmartRemote", srValues);

                if (m->GetNumPhysicalStrings() > 1) {
                    addCtrlChild(parent, "Max Cascade Remotes",
                                 QString::number(m->GetSRMaxCascade()), true,
                                 Kind::Int, "SRMaxCascade");
                    addCtrlChild(parent, "Cascade On Port",
                                 m->GetSRCascadeOnPort() ? "yes" : "no", true,
                                 Kind::Bool, "SRCascadeOnPort");
                }
                parent->setExpanded(true);
            }
        }
    }

    // ── Serial protocol: DMX channel + speed ────────────────────────────
    if (serial) {
        addEditableRow(cat, qstr(m->GetControllerProtocol()) + " Channel",
                       QString::number(m->GetControllerDMXChannel()),
                       Kind::Int, "DMXChannel");
        if (m->GetControllerProtocolSpeed() > 0)
            addRow(cat, "Speed", QString::number(m->GetControllerProtocolSpeed()));
    }

    // ── Pixel protocol: per-property "Set X / X" rows gated by caps ─────
    // The "Set X" parent toggles whether the model overrides the controller
    // default; the child holds the editable value.
    if (pixel) {
        if (!caps || caps->SupportsPixelPortNullPixels()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::START_NULLS_ACTIVE);
            auto* p = addRow(cat, "Set Start Null Pixels", act ? "yes" : "no");
            addCtrlChild(p, "Start Null Pixels",
                         QString::number(m->GetControllerStartNulls()), act, Kind::Int, "StartNulls");
        }
        if (!caps || caps->SupportsPixelPortEndNullPixels()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::END_NULLS_ACTIVE);
            auto* p = addRow(cat, "Set End Null Pixels", act ? "yes" : "no");
            addCtrlChild(p, "End Null Pixels",
                         QString::number(m->GetControllerEndNulls()), act, Kind::Int, "EndNulls");
        }
        if (!caps || caps->SupportsPixelPortBrightness()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::BRIGHTNESS_ACTIVE);
            auto* p = addRow(cat, "Set Brightness", act ? "yes" : "no");
            addCtrlChild(p, "Brightness",
                         QString::number(m->GetControllerBrightness()) + " %", act,
                         Kind::IntPercent, "CtrlBrightness");
        }
        if (!caps || caps->SupportsPixelPortGamma()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::GAMMA_ACTIVE);
            auto* p = addRow(cat, "Set Gamma", act ? "yes" : "no");
            addCtrlChild(p, "Gamma",
                         QString::number(m->GetControllerGamma(), 'f', 2), act,
                         Kind::Double, "CtrlGamma");
        }
        if (!caps || caps->SupportsPixelPortColourOrder()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::COLOR_ORDER_ACTIVE);
            auto* p = addRow(cat, "Set Color Order", act ? "yes" : "no");
            addCtrlChild(p, "Color Order", qstr(m->GetControllerColorOrder()), act,
                         Kind::Enum, "ColorOrder",
                         {"RGB", "RBG", "GRB", "GBR", "BRG", "BGR",
                          "RGBW", "RBGW", "GRBW", "GBRW", "BRGW", "BGRW", "WRGB"});
        }
        if (!caps || caps->SupportsPixelPortDirection()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::REVERSE_ACTIVE);
            auto* p = addRow(cat, "Set Pixel Direction", act ? "yes" : "no");
            addCtrlChild(p, "Direction", directionLabel(m->GetControllerReverse()), act,
                         Kind::Enum, "CtrlDirection", {"Forward", "Reverse"});
        }
        if (!caps || caps->SupportsPixelPortGrouping()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::GROUP_COUNT_ACTIVE);
            auto* p = addRow(cat, "Set Group Count", act ? "yes" : "no");
            addCtrlChild(p, "Group Count",
                         QString::number(m->GetControllerGroupCount()), act, Kind::Int, "GroupCount");
        }
        if (!caps || caps->SupportsPixelZigZag()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::ZIG_ZAG_ACTIVE);
            auto* p = addRow(cat, "Set Zig Zag", act ? "yes" : "no");
            addCtrlChild(p, "Zig Zag",
                         QString::number(m->GetControllerZigZag()), act, Kind::Int, "CtrlZigZag");
        }
        if (!caps || caps->SupportsTs()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::TS_ACTIVE);
            auto* p = addRow(cat, "Set Smart Ts", act ? "yes" : "no");
            // No SetSmartTs setter on the model — read-only child.
            addRow(p, "Smart Ts", QString::number(m->GetSmartTs()));
            p->setExpanded(act);
        }
    }
}

// ── Per-model-type properties ─────────────────────────────────────────────────

void LayoutPropertyTree::populateModelTypeProperties(Model* m) {
    const DisplayAsType t = m->GetDisplayAs();

    // Dispatch on the stored DisplayAsType enum + static_cast rather than
    // dynamic_cast — RTTI was producing null casts in the Qt build, which
    // silently dropped every per-type section.  GetDisplayAs() reads the
    // model's own type field and is always reliable.
    // Sphere is a MatrixModel subclass but has its own DisplayAsType.
    if (t == DisplayAsType::Sphere) {
        auto* sphere = static_cast<SphereModel*>(m);
        auto* cat = addCategory("Sphere");
        addEditableRow(cat, "Start Latitude", QString::number(sphere->GetStartLatitude()),
                       Kind::Int, "StartLatitude");
        addEditableRow(cat, "End Latitude",   QString::number(sphere->GetEndLatitude()),
                       Kind::Int, "EndLatitude");
        addEditableRow(cat, "Sphere Degrees", QString::number(sphere->GetSphereDegrees()),
                       Kind::Int, "SphereDegrees");
        // Sphere also inherits the Matrix knobs.
        addEditableRow(cat, "# Strings",
                       QString::number(sphere->GetNumPhysicalStrings()),
                       Kind::Int, "NumStrings");
        addEditableRow(cat, sphere->IsSingleNode() ? "Lights/String" : "Nodes/String",
                       QString::number(sphere->GetNodesPerString()),
                       Kind::Int, "NodesPerString");
        addEditableRow(cat, "Strands/String", QString::number(sphere->GetStrandsPerString()),
                       Kind::Int, "StrandsPerString");
        addEditableRow(cat, "Alternate Nodes", sphere->HasAlternateNodes() ? "yes" : "no",
                       Kind::Bool, "AlternateNodes");
        addEditableRow(cat, "Don't Zig Zag",   sphere->IsNoZigZag() ? "yes" : "no",
                       Kind::Bool, "NoZigZag");
        return;
    }

    if (t == DisplayAsType::Matrix) {
        auto* matrix = static_cast<MatrixModel*>(m);
        auto* cat = addCategory("Matrix");
        addEditableRow(cat, "Direction",
                       matrix->isVerticalMatrix() ? "Vertical" : "Horizontal",
                       Kind::Enum, "Direction", {"Horizontal", "Vertical"});
        addEditableRow(cat, "# Strings",
                       QString::number(matrix->GetNumPhysicalStrings()),
                       Kind::Int, "NumStrings");
        addEditableRow(cat, matrix->IsSingleNode() ? "Lights/String" : "Nodes/String",
                       QString::number(matrix->GetNodesPerString()),
                       Kind::Int, "NodesPerString");
        addEditableRow(cat, "Strands/String",
                       QString::number(matrix->GetStrandsPerString()),
                       Kind::Int, "StrandsPerString");
        addEditableRow(cat, "Alternate Nodes",
                       matrix->HasAlternateNodes() ? "yes" : "no",
                       Kind::Bool, "AlternateNodes");
        addEditableRow(cat, "Don't Zig Zag",
                       matrix->IsNoZigZag() ? "yes" : "no",
                       Kind::Bool, "NoZigZag");
        const QString start = QString("%1 %2")
            .arg(matrix->GetIsBtoT() ? "Bottom" : "Top")
            .arg(matrix->GetIsLtoR() ? "Left"   : "Right");
        addRow(cat, "Starting Location", start);
        return;
    }

    if (t == DisplayAsType::Arches) {
        auto* arches = static_cast<ArchesModel*>(m);
        auto* cat = addCategory("Arches");
        const bool layered = arches->GetLayerSizeCount() != 0;
        addRow(cat, "Layered Arches", layered ? "yes" : "no");
        if (!layered) {
            addEditableRow(cat, "# Arches",
                           QString::number(arches->GetNumArches()),
                           Kind::Int, "NumStrings");
            addEditableRow(cat, "Nodes Per Arch", QString::number(arches->GetNodesPerArch()),
                           Kind::Int, "NodesPerArch");
        } else {
            addEditableRow(cat, "Nodes",   QString::number(arches->GetNodesPerArch()),
                           Kind::Int, "NodesPerArch");
            addRow(cat, "Layer Count",  QString::number(arches->GetLayerSizeCount()));
            addEditableRow(cat, "Hollow %", QString::number(arches->GetHollow()),
                           Kind::Int, "Hollow");
            addEditableRow(cat, "Zig-Zag", arches->GetZigZag() ? "yes" : "no",
                           Kind::Bool, "ZigZag");
        }
        addEditableRow(cat, "Lights Per Node", QString::number(arches->GetLightsPerNode()),
                       Kind::Int, "LightsPerNode");
        addEditableRow(cat, "Arc Degrees", QString::number(arches->GetArc()),
                       Kind::Int, "Arc");
        if (!layered)
            addEditableRow(cat, "Gap Between Arches", QString::number(arches->GetGap()),
                           Kind::Int, "Gap");
        addRow(cat, "Starting Location",
               arches->GetIsLtoR() ? "Green Square" : "Blue Square");
        return;
    }

    if (t == DisplayAsType::Tree) {
        auto* tree = static_cast<TreeModel*>(m);
        auto* cat = addCategory("Tree");
        addEditableRow(cat, "Tree Type",        QString::number(tree->GetTreeType()),
                       Kind::Int, "TreeType");
        addEditableRow(cat, "Degrees",          QString::number(tree->GetTreeDegrees(),  'f', 1),
                       Kind::Double, "TreeDegrees");
        addEditableRow(cat, "Rotation",         QString::number(tree->GetTreeRotation(), 'f', 1),
                       Kind::Double, "TreeRotation");
        addEditableRow(cat, "Spiral Rotations", QString::number(tree->GetSpiralRotations(), 'f', 2),
                       Kind::Double, "SpiralRotations");
        addEditableRow(cat, "Bottom/Top Ratio", QString::number(tree->GetBottomTopRatio(),  'f', 2),
                       Kind::Double, "BottomTopRatio");
        addEditableRow(cat, "Perspective",      QString::number(tree->GetTreePerspective(), 'f', 2),
                       Kind::Double, "Perspective");
        addEditableRow(cat, "First Strand",     QString::number(tree->GetFirstStrand()),
                       Kind::Int, "FirstStrand");
        return;
    }

    if (t == DisplayAsType::Star) {
        auto* star = static_cast<StarModel*>(m);
        auto* cat = addCategory("Star");
        addEditableRow(cat, "Star Ratio",  QString::number(star->GetStarRatio(), 'f', 2),
                       Kind::Double, "StarRatio");
        addEditableRow(cat, "Inner %",     QString::number(star->GetInnerPercent()),
                       Kind::Int, "InnerPercent");
        addRow(cat, "Start Location", qstr(star->GetStartLocation()));
        addRow(cat, "Layers",         QString::number(star->GetLayerSizeCount()));
        return;
    }

    if (t == DisplayAsType::Cube) {
        auto* cube = static_cast<CubeModel*>(m);
        auto* cat = addCategory("Cube");
        addRow(cat, "Strands",        QString::number(cube->GetNumStrands()));
        addRow(cat, "Nodes/Strand",   QString::number(cube->NodesPerString()));
        addEditableRow(cat, "Width",  QString::number(cube->GetCubeWidth()),  Kind::Int, "CubeWidth");
        addEditableRow(cat, "Height", QString::number(cube->GetCubeHeight()), Kind::Int, "CubeHeight");
        addEditableRow(cat, "Depth",  QString::number(cube->GetCubeDepth()),  Kind::Int, "CubeDepth");
        addEditableRow(cat, "Cube Strings", QString::number(cube->GetCubeStrings()),
                       Kind::Int, "CubeStrings");
        addRow(cat, "Cube Style",     qstr(cube->GetCubeStyle()));
        addRow(cat, "Strand Style",   qstr(cube->GetStrandStyle()));
        addRow(cat, "Start",          qstr(cube->GetCubeStart()));
        addEditableRow(cat, "Strand Per Layer", cube->IsStrandPerLayer() ? "yes" : "no",
                       Kind::Bool, "StrandPerLayer");
        return;
    }

    if (t == DisplayAsType::CandyCanes) {
        auto* candy = static_cast<CandyCaneModel*>(m);
        auto* cat = addCategory("Candy Canes");
        addEditableRow(cat, "# Canes", QString::number(candy->GetNumStrings()),
                       Kind::Int, "NumCanes");
        addEditableRow(cat, "Lights Per Node", QString::number(candy->GetLightsPerNode()),
                       Kind::Int, "LightsPerNode");
        return;
    }

    if (t == DisplayAsType::PolyLine) {
        auto* poly = static_cast<PolyLineModel*>(m);
        auto* cat = addCategory("Poly Line");
        addEditableRow(cat, "# Strings", QString::number(poly->GetNumStrings()),
                       Kind::Int, "NumStrings");
        addRow(cat, "Drop Points",     QString::number(poly->GetDropPoints()));
        addRow(cat, "Segments",        QString::number(poly->GetNumSegments()));
        addEditableRow(cat, "Lights Per Node", QString::number(poly->GetLightsPerNode()),
                       Kind::Int, "LightsPerNode");
        addEditableRow(cat, "Model Height", QString::number(poly->GetModelHeight(), 'f', 2),
                       Kind::Double, "ModelHeight");
        addEditableRow(cat, "Alternate Nodes", poly->HasAlternateNodes() ? "yes" : "no",
                       Kind::Bool, "AlternateNodes");
        return;
    }

    if (t == DisplayAsType::WindowFrame) {
        auto* wf = static_cast<WindowFrameModel*>(m);
        auto* cat = addCategory("Window Frame");
        addEditableRow(cat, "Rotation", QString::number(wf->GetRotation()),
                       Kind::Int, "Rotation");
        return;
    }

    if (t == DisplayAsType::Spinner) {
        auto* spinner = static_cast<SpinnerModel*>(m);
        auto* cat = addCategory("Spinner");
        addEditableRow(cat, "# Strings",
                       QString::number(spinner->GetNumSpinnerStrings()),
                       Kind::Int, "NumStrings");
        addEditableRow(cat, "Arms/String", QString::number(spinner->GetArmsPerString()),
                       Kind::Int, "ArmsPerString");
        addEditableRow(cat, "Nodes/Arm",   QString::number(spinner->GetNodesPerArm()),
                       Kind::Int, "NodesPerArm");
        addEditableRow(cat, "Hollow %",    QString::number(spinner->GetHollowPercent()),
                       Kind::Int, "Hollow");
        addEditableRow(cat, "Arc Angle",   QString::number(spinner->GetArcAngle()),
                       Kind::Int, "Arc");
        addEditableRow(cat, "Start Angle", QString::number(spinner->GetStartAngle()),
                       Kind::Int, "StartAngle");
        addEditableRow(cat, "Zig-Zag",     spinner->HasZigZag() ? "yes" : "no",
                       Kind::Bool, "ZigZag");
        addEditableRow(cat, "Alternate Nodes", spinner->HasAlternateNodes() ? "yes" : "no",
                       Kind::Bool, "Alternate");
        return;
    }

    if (t == DisplayAsType::Circle) {
        auto* circle = static_cast<CircleModel*>(m);
        auto* cat = addCategory("Circle");
        addEditableRow(cat, "# Strings",
                       QString::number(circle->GetNumCircleStrings()),
                       Kind::Int, "NumStrings");
        addEditableRow(cat, circle->IsSingleNode() ? "Lights/String" : "Nodes/String",
                       QString::number(circle->GetNodesPerString()),
                       Kind::Int, "NodesPerString");
        addEditableRow(cat, "Center %", QString::number(circle->GetCenterPercent()),
                       Kind::Int, "CenterPercent");
        addEditableRow(cat, "Inside Out", circle->IsInsideOut() ? "yes" : "no",
                       Kind::Bool, "InsideOut");
        addRow(cat, "Layers",       QString::number(circle->GetLayerSizeCount()));
        return;
    }

    if (t == DisplayAsType::Icicles) {
        auto* icicles = static_cast<IciclesModel*>(m);
        auto* cat = addCategory("Icicles");
        addEditableRow(cat, "# Strings",
                       QString::number(icicles->GetNumIcicleStrings()),
                       Kind::Int, "NumStrings");
        addEditableRow(cat, "Lights/String", QString::number(icicles->GetLightsPerString()),
                       Kind::Int, "LightsPerString");
        addEditableRow(cat, "Drop Pattern", qstr(icicles->GetDropPattern()),
                       Kind::String, "DropPattern");
        addEditableRow(cat, "Alternate Nodes", icicles->HasAlternateNodes() ? "yes" : "no",
                       Kind::Bool, "AlternateNodes");
        return;
    }

    // Wreath intentionally skipped — deprecated model type.

    if (t == DisplayAsType::ChannelBlock) {
        auto* cb = static_cast<ChannelBlockModel*>(m);
        auto* cat = addCategory("Channel Block");
        addEditableRow(cat, "# Channels",
                       QString::number(cb->GetNumChannels()),
                       Kind::Int, "NumStrings");
        const auto& colors = cb->GetChannelColors();
        addRow(cat, "Color Slots", QString::number(colors.size()));
        for (int i = 0; i < (int)colors.size() && i < 24; ++i) {
            const QString hex = qstr(colors[i]);
            auto* row = addRow(cat, QString("  Channel %1").arg(i + 1), hex);
            QColor c(hex);
            if (c.isValid()) row->setBackground(1, c);
        }
        return;
    }

    if (t == DisplayAsType::Custom) {
        auto* custom = static_cast<CustomModel*>(m);
        auto* cat = addCategory("Custom");
        addRow(cat, "Width",   QString::number(custom->GetCustomWidth()));
        addRow(cat, "Height",  QString::number(custom->GetCustomHeight()));
        addRow(cat, "Depth",   QString::number(custom->GetCustomDepth()));
        addEditableRow(cat, "# Strings", QString::number(custom->GetNumStrings()),
                       Kind::Int, "NumStrings");
        addEditableRow(cat, "Background", qstr(custom->GetCustomBackground()),
                       Kind::String, "Background");
        addEditableRow(cat, "Lightness", QString::number(custom->GetCustomLightness()),
                       Kind::Int, "Lightness");
        addEditableRow(cat, "Bkg Scale", QString::number(custom->GetCustomBkgScale()),
                       Kind::Int, "BkgScale");
        addEditableRow(cat, "Bkg Brightness", QString::number(custom->GetCustomBkgBrightness()),
                       Kind::Int, "BkgBrightness");
        addRow(cat, "All Nodes Unique", custom->IsAllNodesUnique() ? "yes" : "no");
        return;
    }

    if (t == DisplayAsType::SingleLine) {
        auto* sl = static_cast<SingleLineModel*>(m);
        auto* cat = addCategory("Single Line");
        addEditableRow(cat, "# Lines",     QString::number(sl->GetNumLines()),
                       Kind::Int, "NumLines");
        addEditableRow(cat, "Nodes/Line",  QString::number(sl->GetNodesPerString()),
                       Kind::Int, "NodesPerLine");
        addEditableRow(cat, "Lights/Node", QString::number(sl->GetLightsPerNode()),
                       Kind::Int, "LightsPerNode");
        return;
    }

    if (t == DisplayAsType::MultiPoint) {
        auto* mp = static_cast<MultiPointModel*>(m);
        auto* cat = addCategory("Multi Point");
        addEditableRow(cat, "# Strings", QString::number(mp->GetNumStrings()),
                       Kind::Int, "NumStrings");
        addRow(cat, "# Points",       QString::number(mp->GetNumPoints()));
        addEditableRow(cat, "Model Height", QString::number(mp->GetModelHeight(), 'f', 2),
                       Kind::Double, "ModelHeight");
        return;
    }

    if (t == DisplayAsType::Image) {
        auto* img = static_cast<ImageModel*>(m);
        auto* cat = addCategory("Image");
        addRow(cat, "Image File",       qstr(img->GetImageFile()));
        addRow(cat, "White As Alpha",   img->IsWhiteAsAlpha() ? "yes" : "no");
        addRow(cat, "Off Brightness",   QString::number(img->GetOffBrightness()));
        return;
    }

    if (t == DisplayAsType::Label) {
        auto* lbl = static_cast<LabelModel*>(m);
        auto* cat = addCategory("Label");
        addRow(cat, "Text",       qstr(lbl->GetLabelText()));
        addRow(cat, "Font Size",  QString::number(lbl->GetLabelFontSize()));
        const xlColor c = lbl->GetLabelTextColor();
        auto* row = addRow(cat, "Text Color", hexColor(c));
        row->setBackground(1, QBrush(QColor(c.red, c.green, c.blue)));
        return;
    }

    // ── DMX models ────────────────────────────────────────────────────────
    // Subclasses with their own typed getters get their own section; every
    // DMX model also gets the shared DmxModel base section (channel count
    // + which DmxAbility components are configured).  Sub-section first so
    // it appears above the base.
    if (t == DisplayAsType::DmxServo) {
        auto* servo = static_cast<DmxServo*>(m);
        auto* cat = addCategory("DMX Servo");
        addRow(cat, "# Servos",     QString::number(servo->GetNumServos()));
        addRow(cat, "16-bit",       servo->Is16Bit() ? "yes" : "no");
        addRow(cat, "Brightness",   QString::number(servo->GetBrightness(), 'f', 2));
        addRow(cat, "Transparency", QString::number(servo->GetTransparency()));
    } else if (t == DisplayAsType::DmxSkull) {
        auto* skull = static_cast<DmxSkull*>(m);
        auto* cat = addCategory("DMX Skull");
        addRow(cat, "16-bit",     skull->Is16Bit()    ? "yes" : "no");
        addRow(cat, "Mesh Only",  skull->IsMeshOnly() ? "yes" : "no");
        addRow(cat, "Has Jaw",    skull->HasJaw()     ? "yes" : "no");
        addRow(cat, "Has Pan",    skull->HasPan()     ? "yes" : "no");
        addRow(cat, "Has Tilt",   skull->HasTilt()    ? "yes" : "no");
        addRow(cat, "Has Nod",    skull->HasNod()     ? "yes" : "no");
        addRow(cat, "Has Eye LR", skull->HasEyeLR()   ? "yes" : "no");
        addRow(cat, "Has Eye UD", skull->HasEyeUD()   ? "yes" : "no");
        addRow(cat, "Has Color",  skull->HasColor()   ? "yes" : "no");
        if (skull->HasJaw())    addRow(cat, "Jaw Orient",    QString::number(skull->GetJawOrient()));
        if (skull->HasPan())    addRow(cat, "Pan Orient",    QString::number(skull->GetPanOrient()));
        if (skull->HasTilt())   addRow(cat, "Tilt Orient",   QString::number(skull->GetTiltOrient()));
        if (skull->HasNod())    addRow(cat, "Nod Orient",    QString::number(skull->GetNodOrient()));
        if (skull->HasEyeUD())  addRow(cat, "Eye UD Orient", QString::number(skull->GetEyeUDOrient()));
        if (skull->HasEyeLR())  addRow(cat, "Eye LR Orient", QString::number(skull->GetEyeLROrient()));
    }

    if (IsDmxDisplayType(t)) {
        auto* dmx = static_cast<DmxModel*>(m);
        auto* cat = addCategory(qstr(DisplayAsTypeToString(t)));
        addEditableRow(cat, "DMX Channels",
                       QString::number(dmx->GetDmxChannelCount()),
                       Kind::Int, "NumStrings");
        if (dmx->HasColorAbility()) {
            const std::string type = dmx->GetColorAbility()
                                       ? dmx->GetColorAbility()->GetTypeName() : "";
            addRow(cat, "Color Ability", type.empty() ? "yes" : qstr(type));
        }
        if (dmx->HasPresetAbility())  addRow(cat, "Preset Ability",  "yes");
        if (dmx->HasShutterAbility()) addRow(cat, "Shutter Ability", "yes");
        if (dmx->HasDimmerAbility())  addRow(cat, "Dimmer Ability",  "yes");
        return;
    }

    // Fallback — reached only when no case above matched the model's
    // DisplayAsType.  Guarantee a visible category so the user never sees
    // a model with zero type-specific rows (and so we can tell a code-path
    // miss from a stale-binary problem).
    auto* cat = addCategory(qstr(DisplayAsTypeToString(t)) + " — Type Properties");
    addRow(cat, "Type",      qstr(DisplayAsTypeToString(t)));
    addRow(cat, "# Strings", QString::number(m->GetNumStrings()));
}

void LayoutPropertyTree::populateModelAuxiliary(Model* m) {
    auto* cat = addCategory("Sub-Models / Faces / States");
    // The three sub-model/faces/states rows are tagged with their dialog-tab
    // index in Qt::UserRole + 1 so the itemDoubleClicked handler can route to
    // the right ModelEditDialog tab.  Aliases stays a count-only row for now;
    // the dialog re-implementation is in phase 19d.
    auto tag = [](QTreeWidgetItem* r, int tab) {
        r->setData(0, Qt::UserRole + 1, tab);
        r->setText(1, r->text(1) + "   …");
        r->setToolTip(0, "Double-click to edit");
        r->setToolTip(1, "Double-click to edit");
    };
    tag(addRow(cat, "Sub-Models", QString::number(m->GetSubModels().size())), 0);
    tag(addRow(cat, "Faces",      QString::number(m->GetFaceInfo().size())),  1);
    tag(addRow(cat, "States",     QString::number(m->GetStateInfo().size())), 2);
    addRow(cat, "Aliases", QString::number(m->GetAliases().size()));
    // GetStrandNames/GetNodeNames return a single comma-joined string — we
    // show whether one was defined rather than the (often huge) raw value.
    addRow(cat, "Strand Names", m->GetStrandNames().empty() ? "(none)" : "defined");
    addRow(cat, "Node Names",   m->GetNodeNames().empty()   ? "(none)" : "defined");
}

// ── Model Group ───────────────────────────────────────────────────────────────

void LayoutPropertyTree::populateGroupIdentity(ModelGroup* g) {
    auto* cat = addCategory("Model Group");
    addRow(cat, "Name", qstr(g->GetName()));
    addRow(cat, "Type", "Model Group");
    addEditableRow(cat, "Render Type",
                   groupRenderTypeDisplay(qstr(g->GetLayout())),
                   Kind::Enum, "Layout", groupRenderTypeOptions());
    addRow(cat, "Members", QString::number(g->ModelNames().size()));
    addEditableRow(cat, "Layout Group", qstr(g->GetLayoutGroup()),
                   Kind::String, "LayoutGroup");
    addEditableRow(cat, "Active", g->IsActive() ? "yes" : "no",
                   Kind::Bool, "Active");
}

void LayoutPropertyTree::populateGroupBuffer(ModelGroup* g) {
    auto* cat = addCategory("Render Buffer");
    addRow(cat, "Buffer W",      QString::number(g->GetDefaultBufferWi()));
    addRow(cat, "Buffer H",      QString::number(g->GetDefaultBufferHt()));
    addRow(cat, "Channels",      QString::number(g->GetNumChannels()));
    addRow(cat, "First Channel", QString::number(g->GetFirstChannel()));
    addRow(cat, "Last Channel",  QString::number(g->GetLastChannel()));
    addRow(cat, "Grid Size",     QString::number(g->GetGridSize()));
    addRow(cat, "Models",        QString::number(g->GetModelCount()));
    addRow(cat, "X Centre Off",  QString::number(g->GetXCentreOffset()));
    addRow(cat, "Y Centre Off",  QString::number(g->GetYCentreOffset()));
}

void LayoutPropertyTree::populateGroupBounds(ModelGroup* g) {
    auto* cat = addCategory("Bounds");
    if (g->GetCentreDefined()) {
        addRow(cat, "Centre X", QString::number(g->GetCentreX(), 'f', 2));
        addRow(cat, "Centre Y", QString::number(g->GetCentreY(), 'f', 2));
        addRow(cat, "Min X",    QString::number(g->GetCentreMinx()));
        addRow(cat, "Min Y",    QString::number(g->GetCentreMiny()));
        addRow(cat, "Max X",    QString::number(g->GetCentreMaxx()));
        addRow(cat, "Max Y",    QString::number(g->GetCentreMaxy()));
    } else {
        addRow(cat, "Centre", "(not defined)");
    }
}

void LayoutPropertyTree::populateGroupAppearance(ModelGroup* g) {
    auto* cat = addCategory("Appearance");
    auto* row = addEditableRow(cat, "Tag Color", hexColor(g->GetTagColour()),
                               Kind::Color, "TagColor");
    const xlColor tc = g->GetTagColour();
    row->setBackground(1, QBrush(QColor(tc.red, tc.green, tc.blue)));
    addEditableRow(cat, "Default Camera", qstr(g->GetDefaultCamera()),
                   Kind::String, "DefaultCamera");
}

void LayoutPropertyTree::populateGroupMembers(ModelGroup* g) {
    auto* cat = addCategory("Members");
    const auto& names = g->ModelNames();
    if (names.empty()) {
        addRow(cat, "(none)", "");
        return;
    }
    int i = 1;
    for (const auto& n : names)
        addRow(cat, QString::number(i++), qstr(n));
}

void LayoutPropertyTree::populateGroupFromInfo(const QtModelGroupInfo& gi) {
    // Used only when ModelManager doesn't have the live ModelGroup*.
    // Rows here mirror what the XmlSerializer-backed live path shows, but
    // sourced from the QtSequenceDoc snapshot.  No editable rows — edits
    // need the live ModelGroup* to mutate.
    {
        auto* cat = addCategory("Model Group  (not loaded — read-only)");
        addRow(cat, "Name",         gi.name);
        addRow(cat, "Layout",       gi.layout);
        addRow(cat, "Members",      QString::number(gi.modelNames.size()));
    }
    {
        auto* cat = addCategory("Render Buffer");
        addRow(cat, "Buffer W",     QString::number(gi.bufferW));
        addRow(cat, "Buffer H",     QString::number(gi.bufferH));
    }
    {
        auto* cat = addCategory("Bounds");
        addRow(cat, "Min X",        QString::number(gi.minX, 'f', 2));
        addRow(cat, "Min Y",        QString::number(gi.minY, 'f', 2));
        addRow(cat, "Max X",        QString::number(gi.maxX, 'f', 2));
        addRow(cat, "Max Y",        QString::number(gi.maxY, 'f', 2));
    }
    {
        auto* cat = addCategory("Members");
        if (gi.modelNames.isEmpty()) {
            addRow(cat, "(none)", "");
        } else {
            int i = 1;
            for (const auto& n : gi.modelNames)
                addRow(cat, QString::number(i++), n);
        }
    }
}

// ── Controller ────────────────────────────────────────────────────────────────

void LayoutPropertyTree::populateControllerIdentity(Controller* c) {
    auto* cat = addCategory(qstr(c->GetType()));
    addEditableRow(cat, "Name", qstr(c->GetName()), Kind::String, "Name");
    addRow(cat, "Type", qstr(c->GetType()));
    addEditableRow(cat, "Description", qstr(c->GetDescription()),
                   Kind::String, "Description");

    // Vendor / Model / Variant come from the static ControllerCaps catalog.
    // Each list narrows from the previous selection, mirroring the wx tab's
    // cascading dropdowns.  When the user picks a new Vendor the controller
    // edit fires showController() again (see LayoutWindow::controllerChanged)
    // which rebuilds these rows with the new Model/Variant choices.
    auto toQList = [](const std::list<std::string>& xs) {
        QStringList out;
        out.reserve(static_cast<int>(xs.size()));
        for (const auto& s : xs) out.append(qstr(s));
        return out;
    };
    const std::string type = c->GetType();
    QStringList vendors  = toQList(ControllerCaps::GetVendors(type));
    if (vendors.isEmpty()) vendors << qstr(c->GetVendor());
    addEditableRow(cat, "Vendor", qstr(c->GetVendor()),
                   Kind::Enum, "Vendor", vendors);

    QStringList models = toQList(ControllerCaps::GetModels(type, c->GetVendor()));
    if (models.isEmpty()) models << qstr(c->GetModel());
    addEditableRow(cat, "Model", qstr(c->GetModel()),
                   Kind::Enum, "Model", models);

    QStringList variants = toQList(
        ControllerCaps::GetVariants(type, c->GetVendor(), c->GetModel()));
    if (variants.isEmpty()) variants << qstr(c->GetVariant());
    addEditableRow(cat, "Variant", qstr(c->GetVariant()),
                   Kind::Enum, "Variant", variants);

    addEditableRow(cat, "Active",  activeStateLabel(c->GetActive()), Kind::Enum, "Active",
                   {"Active", "Inactive", "xLights Only"});
    addEditableRow(cat, "Auto Layout", c->IsAutoLayout() ? "yes" : "no",
                   Kind::Bool, "AutoLayout");
    addEditableRow(cat, "Auto Size",   c->IsAutoSize()   ? "yes" : "no",
                   Kind::Bool, "AutoSize");
    addEditableRow(cat, "Auto Upload", c->IsAutoUpload() ? "yes" : "no",
                   Kind::Bool, "AutoUpload");
}

void LayoutPropertyTree::populateControllerNetwork(Controller* c) {
    auto* cat = addCategory("Connection");

    // Protocol dropdown.  Source matches the wx ControllerEthernet adapter:
    // when ControllerCaps is available we use caps->GetInputProtocols()
    // mapped to the OUTPUT_* constants; otherwise the full default set.
    // Serial controllers don't change protocol from this row — their
    // protocol is fixed by type — so they stay read-only.
    if (auto* e = dynamic_cast<ControllerEthernet*>(c)) {
        QStringList protocols;
        auto* caps = e->GetControllerCaps();
        if (caps) {
            for (const auto& it : caps->GetInputProtocols()) {
                if      (it == "e131")          protocols << OUTPUT_E131;
                else if (it == "zcpp")          protocols << OUTPUT_ZCPP;
                else if (it == "artnet")        protocols << OUTPUT_ARTNET;
                else if (it == "kinet")         protocols << OUTPUT_KINET;
                else if (it == "ddp")           protocols << OUTPUT_DDP;
                else if (it == "opc")           protocols << OUTPUT_OPC;
                else if (it == "twinkly")       protocols << OUTPUT_TWINKLY;
                else if (it == "player only")   protocols << OUTPUT_PLAYER_ONLY;
                else if (it == "xxx ethernet")  protocols << OUTPUT_xxxETHERNET;
            }
        }
        if (protocols.isEmpty()) {
            protocols << OUTPUT_E131 << OUTPUT_ZCPP << OUTPUT_ARTNET
                      << OUTPUT_DDP  << OUTPUT_OPC  << OUTPUT_KINET
                      << OUTPUT_TWINKLY << OUTPUT_PLAYER_ONLY;
        }
        const QString curProto = qstr(e->GetProtocol());
        if (!curProto.isEmpty() && !protocols.contains(curProto))
            protocols.prepend(curProto);
        addEditableRow(cat, "Protocol", curProto, Kind::Enum, "Protocol", protocols);

        addEditableRow(cat, "IP", qstr(e->GetIP()), Kind::String, "IP");
        addEditableRow(cat, "FPP Proxy", qstr(e->GetFPPProxy()),
                       Kind::String, "FPPProxy");
        addEditableRow(cat, "Priority", QString::number(e->GetPriority()),
                       Kind::Int, "Priority");
    } else if (auto* s = dynamic_cast<ControllerSerial*>(c)) {
        // Serial controller protocol is fixed by type — show as read-only.
        addRow(cat, "Protocol", qstr(c->GetProtocol()));
        auto toQList = [](const std::list<std::string>& xs) {
            QStringList out; out.reserve(static_cast<int>(xs.size()));
            for (const auto& v : xs) out.append(qstr(v));
            return out;
        };
        // Available ports come from SerialOutput's enumerator (scans
        // /dev/cu.* on macOS, COM ports on Windows, /dev/tty* on Linux).
        // Pre-pend the current value so the row keeps a valid selection on
        // headless systems / unplugged adapters.
        QStringList ports = toQList(SerialOutput::GetPossibleSerialPorts());
        const QString curPort = qstr(s->GetPort());
        if (!curPort.isEmpty() && !ports.contains(curPort))
            ports.prepend(curPort);
        if (ports.isEmpty()) ports << curPort;
        addEditableRow(cat, "Port", curPort, Kind::Enum, "Port", ports);

        QStringList baudRates = toQList(SerialOutput::GetPossibleBaudRates());
        const QString curSpeed = QString::number(s->GetSpeed());
        if (!curSpeed.isEmpty() && !baudRates.contains(curSpeed))
            baudRates.prepend(curSpeed);
        if (baudRates.isEmpty()) baudRates << curSpeed;
        addEditableRow(cat, "Speed", curSpeed, Kind::Enum, "Speed", baudRates);
    } else {
        addRow(cat, "Protocol", qstr(c->GetProtocol()));
    }
}

void LayoutPropertyTree::populateControllerOutput(Controller* c) {
    auto* cat = addCategory("Output Range");
    addRow(cat, "Start Channel", QString::number(c->GetStartChannel()));
    addRow(cat, "End Channel",   QString::number(c->GetEndChannel()));
    addRow(cat, "Channels",      QString::number(c->GetChannels()));
    addRow(cat, "Universe(s)",   qstr(c->GetUniverseString()));
}

void LayoutPropertyTree::populateControllerCapabilities(Controller* c) {
    auto* caps = c->GetControllerCaps();
    if (!caps) return;
    auto* cat = addCategory("Capabilities");
    addRow(cat, "Max Pixel Ports",      QString::number(caps->GetMaxPixelPort()));
    addRow(cat, "Max Serial Ports",     QString::number(caps->GetMaxSerialPort()));
    addRow(cat, "Pixel Port Channels",  QString::number(caps->GetMaxPixelPortChannels()));
    addRow(cat, "Serial Port Channels", QString::number(caps->GetMaxSerialPortChannels()));
    addRow(cat, "Smart Remote Count",   QString::number(caps->GetSmartRemoteCount()));
    addRow(cat, "Auto Layout",          caps->SupportsAutoLayout() ? "yes" : "no");
    addRow(cat, "Auto Upload",          caps->SupportsAutoUpload() ? "yes" : "no");
    addRow(cat, "Variant",              qstr(caps->GetVariantName()));
}

void LayoutPropertyTree::populateControllerOutputs(Controller* c) {
    const auto& outputs = c->GetOutputs();
    if (outputs.empty()) return;
    auto* cat = addCategory("Outputs");
    int i = 1;
    for (Output* o : outputs) {
        addRow(cat, QString("#%1 Universe").arg(i),
               qstr(o->GetUniverseString()));
        ++i;
    }
}

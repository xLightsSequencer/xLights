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
#include "../../src-core/models/PolyLineModel.h"
#include "../../src-core/models/StarModel.h"
#include "../../src-core/models/TreeModel.h"
#include "../../src-core/models/WindowFrameModel.h"
#include "../../src-core/outputs/Controller.h"
#include "../../src-core/outputs/ControllerEthernet.h"
#include "../../src-core/outputs/ControllerSerial.h"
#include "../../src-core/outputs/Output.h"
#include "../../src-core/outputs/OutputManager.h"
#include "../../src-core/controllers/ControllerCaps.h"
#include "../../src-core/utils/Color.h"

#include <QBrush>
#include <QFont>
#include <QHeaderView>
#include <QPalette>

namespace {

QString qstr(const std::string& s) { return QString::fromStdString(s); }

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

QString activeStateLabel(Controller::ACTIVESTATE s) {
    switch (s) {
        case Controller::ACTIVESTATE::ACTIVE:           return "Active";
        case Controller::ACTIVESTATE::ACTIVEINXLIGHTSONLY:  return "xLights only";
        case Controller::ACTIVESTATE::INACTIVE:         return "Inactive";
        default:                                        return "Unknown";
    }
}

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
    setEditTriggers(QAbstractItemView::NoEditTriggers);  // editable in phase 4
    setIndentation(14);
}

void LayoutPropertyTree::setModelManager(ModelManager* mm) {
    _mm = mm;
    _om = mm ? mm->GetOutputManager() : nullptr;
}

void LayoutPropertyTree::clearAll() { clear(); }

void LayoutPropertyTree::showModel(const QString& name) {
    clear();
    if (!_mm || name.isEmpty()) return;

    Model* m = nullptr;
    try { m = _mm->GetModel(name.toStdString()); }
    catch (...) { m = nullptr; }
    if (!m) return;

    // Skip groups here — caller should use showGroup() for those.
    if (dynamic_cast<ModelGroup*>(m)) return;

    populateModelIdentity(m);
    populateModelSizingChannels(m);
    populateModelLayout(m);
    populateModelAppearance(m);
    populateModelStringProperties(m);
    populateModelControllerConnection(m);
    populateModelAuxiliary(m);
    expandAll();
}

void LayoutPropertyTree::showGroup(const QString& name) {
    clear();
    if (!_mm || name.isEmpty()) return;

    ModelGroup* g = nullptr;
    try {
        Model* m = _mm->GetModel(name.toStdString());
        g = dynamic_cast<ModelGroup*>(m);
    } catch (...) { g = nullptr; }
    if (!g) return;

    populateGroupIdentity(g);
    populateGroupBuffer(g);
    populateGroupBounds(g);
    populateGroupAppearance(g);
    populateGroupMembers(g);
    expandAll();
}

void LayoutPropertyTree::showController(const QString& name) {
    clear();
    if (!_om || name.isEmpty()) return;

    Controller* c = nullptr;
    try { c = _om->GetController(name.toStdString()); }
    catch (...) { c = nullptr; }
    if (!c) return;

    populateControllerIdentity(c);
    populateControllerNetwork(c);
    populateControllerOutput(c);
    populateControllerCapabilities(c);
    populateControllerOutputs(c);
    expandAll();
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
    return row;
}

// ── Model ─────────────────────────────────────────────────────────────────────

void LayoutPropertyTree::populateModelIdentity(Model* m) {
    auto* cat = addCategory(qstr(DisplayAsTypeToString(m->GetDisplayAs())));
    addRow(cat, "Name",        qstr(m->GetName()));
    addRow(cat, "Type",        qstr(DisplayAsTypeToString(m->GetDisplayAs())));
    addRow(cat, "Description", qstr(m->GetDescription()));
}

void LayoutPropertyTree::populateModelSizingChannels(Model* m) {
    auto* cat = addCategory("Sizing & Channels");
    addRow(cat, "Start Channel", qstr(m->GetModelStartChannel()));
    addRow(cat, "Channels",      QString::number(m->GetNumChannels()));
    addRow(cat, "Strings",       QString::number(m->GetNumStrings()));
    addRow(cat, "Buffer W",      QString::number(m->GetDefaultBufferWi()));
    addRow(cat, "Buffer H",      QString::number(m->GetDefaultBufferHt()));
    if (m->SupportsLowDefinitionRender())
        addRow(cat, "Low Def Factor", QString::number(m->GetLowDefFactor()));
    if (m->HasIndividualStartChannels())
        addRow(cat, "Indiv Start Chans", "yes");
}

void LayoutPropertyTree::populateModelLayout(Model* m) {
    auto* cat = addCategory("Layout");
    addRow(cat, "Layout Group", qstr(m->GetLayoutGroup()));
    addRow(cat, "Active",       m->IsActive() ? "yes" : "no");
    const auto& loc = m->GetModelScreenLocation();
    addRow(cat, "World X", QString::number(loc.GetWorldPos_X(), 'f', 2));
    addRow(cat, "World Y", QString::number(loc.GetWorldPos_Y(), 'f', 2));
    addRow(cat, "World Z", QString::number(loc.GetWorldPos_Z(), 'f', 2));
    const glm::vec3 s = loc.GetScaleMatrix();
    addRow(cat, "Scale X", QString::number(s.x, 'f', 3));
    addRow(cat, "Scale Y", QString::number(s.y, 'f', 3));
    addRow(cat, "Scale Z", QString::number(s.z, 'f', 3));
    const glm::vec3 r = loc.GetRotationAngles();
    addRow(cat, "Rotation X", QString::number(r.x, 'f', 1));
    addRow(cat, "Rotation Y", QString::number(r.y, 'f', 1));
    addRow(cat, "Rotation Z", QString::number(r.z, 'f', 1));
}

void LayoutPropertyTree::populateModelAppearance(Model* m) {
    auto* cat = addCategory("Appearance");
    addRow(cat, "Pixel Size",         QString::number(m->GetPixelSize()));
    addRow(cat, "Pixel Style",        pixelStyleLabel(m->GetPixelStyle()));
    addRow(cat, "Transparency",       QString::number(m->GetTransparency()) + " %");
    addRow(cat, "Black Transparency", QString::number(m->GetBlackTransparency()) + " %");
    auto* row = addRow(cat, "Tag Color", hexColor(m->GetTagColour()));
    const xlColor tc = m->GetTagColour();
    row->setBackground(1, QBrush(QColor(tc.red, tc.green, tc.blue)));
    const std::string smf = m->GetShadowModelFor();
    if (!smf.empty())
        addRow(cat, "Shadow Model For", qstr(smf));
}

void LayoutPropertyTree::populateModelStringProperties(Model* m) {
    auto* cat = addCategory("String Properties");
    addRow(cat, "String Type",  qstr(m->GetStringType()));
    auto* row = addRow(cat, "Custom Color", hexColor(m->GetCustomColor()));
    const xlColor cc = m->GetCustomColor();
    row->setBackground(1, QBrush(QColor(cc.red, cc.green, cc.blue)));
    addRow(cat, "RGBW Handling", qstr(m->GetRGBWHandling()));
    addRow(cat, "RGB Order",     qstr(m->GetRGBOrder()));
}

// Add a value child under a "Set X" parent.  When the property isn't active
// (i.e. the model is inheriting the controller's default), the child is
// dimmed so the reader can tell at a glance which fields are overridden.
static void addCtrlChild(QTreeWidgetItem* parent,
                         const QString& label,
                         const QString& value,
                         bool active) {
    auto* row = new QTreeWidgetItem(parent);
    row->setText(0, label);
    row->setText(1, value);
    if (!active) {
        const QColor dim = parent->treeWidget()->palette().color(QPalette::Disabled, QPalette::Text);
        row->setForeground(0, dim);
        row->setForeground(1, dim);
    }
    parent->setExpanded(active);
}

void LayoutPropertyTree::populateModelControllerConnection(Model* m) {
    auto* cat = addCategory("Controller Connection");

    const std::string ctrl = m->GetControllerName();
    addRow(cat, "Controller", ctrl.empty() ? "(use start channel)" : qstr(ctrl));
    addRow(cat, "Port",       QString::number(m->GetControllerPort(1)));
    addRow(cat, "Protocol",   qstr(m->GetControllerProtocol()));

    auto* caps = m->GetControllerCaps();
    const bool pixel  = m->IsPixelProtocol();
    const bool serial = m->IsSerialProtocol();
    const bool pwm    = false; // IsPWMProtocol exists but most models don't expose it via Model

    // ── Smart Remote chain (pixel proto with smart-remote support) ──────
    if (pixel) {
        int smartRemoteCount = caps ? caps->GetSmartRemoteCount() : 15;
        if (smartRemoteCount != 0) {
            const bool useSR = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::USE_SMART_REMOTE);
            auto* parent = addRow(cat, "Use Smart Remote", useSR ? "yes" : "no");
            if (useSR) {
                addCtrlChild(parent, "Smart Remote Type",
                             qstr(m->GetSmartRemoteType()), true);

                const int sr = m->GetSmartRemote();
                const QString srLabel = sr > 0
                    ? QString(QChar('A' + sr - 1)) + QString(" (%1)").arg(sr)
                    : QString("(none)");
                addCtrlChild(parent, "Smart Remote", srLabel, true);

                if (m->GetNumPhysicalStrings() > 1) {
                    addCtrlChild(parent, "Max Cascade Remotes",
                                 QString::number(m->GetSRMaxCascade()), true);
                    addCtrlChild(parent, "Cascade On Port",
                                 m->GetSRCascadeOnPort() ? "yes" : "no", true);
                }
            }
        }
    }

    // ── Serial protocol: DMX channel + speed ────────────────────────────
    if (serial) {
        addRow(cat, qstr(m->GetControllerProtocol()) + " Channel",
               QString::number(m->GetControllerDMXChannel()));
        if (m->GetControllerProtocolSpeed() > 0)
            addRow(cat, "Speed", QString::number(m->GetControllerProtocolSpeed()));
    }

    // ── Pixel protocol: per-property "Set X / X" rows gated by caps ─────
    if (pixel) {
        if (!caps || caps->SupportsPixelPortNullPixels()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::START_NULLS_ACTIVE);
            auto* p = addRow(cat, "Set Start Null Pixels", act ? "yes" : "no");
            addCtrlChild(p, "Start Null Pixels",
                         QString::number(m->GetControllerStartNulls()), act);
        }
        if (!caps || caps->SupportsPixelPortEndNullPixels()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::END_NULLS_ACTIVE);
            auto* p = addRow(cat, "Set End Null Pixels", act ? "yes" : "no");
            addCtrlChild(p, "End Null Pixels",
                         QString::number(m->GetControllerEndNulls()), act);
        }
        if (!caps || caps->SupportsPixelPortBrightness()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::BRIGHTNESS_ACTIVE);
            auto* p = addRow(cat, "Set Brightness", act ? "yes" : "no");
            addCtrlChild(p, "Brightness",
                         QString::number(m->GetControllerBrightness()) + " %", act);
        }
        if (!caps || caps->SupportsPixelPortGamma()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::GAMMA_ACTIVE);
            auto* p = addRow(cat, "Set Gamma", act ? "yes" : "no");
            addCtrlChild(p, "Gamma",
                         QString::number(m->GetControllerGamma(), 'f', 2), act);
        }
        if (!caps || caps->SupportsPixelPortColourOrder()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::COLOR_ORDER_ACTIVE);
            auto* p = addRow(cat, "Set Color Order", act ? "yes" : "no");
            addCtrlChild(p, "Color Order", qstr(m->GetControllerColorOrder()), act);
        }
        if (!caps || caps->SupportsPixelPortDirection()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::REVERSE_ACTIVE);
            auto* p = addRow(cat, "Set Pixel Direction", act ? "yes" : "no");
            addCtrlChild(p, "Direction", directionLabel(m->GetControllerReverse()), act);
        }
        if (!caps || caps->SupportsPixelPortGrouping()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::GROUP_COUNT_ACTIVE);
            auto* p = addRow(cat, "Set Group Count", act ? "yes" : "no");
            addCtrlChild(p, "Group Count",
                         QString::number(m->GetControllerGroupCount()), act);
        }
        if (!caps || caps->SupportsPixelZigZag()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::ZIG_ZAG_ACTIVE);
            auto* p = addRow(cat, "Set Zig Zag", act ? "yes" : "no");
            addCtrlChild(p, "Zig Zag",
                         QString::number(m->GetControllerZigZag()), act);
        }
        if (!caps || caps->SupportsTs()) {
            const bool act = m->IsCtrlPropertySet(ControllerConnection::CTRL_PROPS::TS_ACTIVE);
            auto* p = addRow(cat, "Set Smart Ts", act ? "yes" : "no");
            addCtrlChild(p, "Smart Ts",
                         QString::number(m->GetSmartTs()), act);
        }
    }
}

void LayoutPropertyTree::populateModelAuxiliary(Model* m) {
    auto* cat = addCategory("Sub-Models / Faces / States");
    addRow(cat, "Faces",   QString::number(m->GetFaceInfo().size()));
    addRow(cat, "States",  QString::number(m->GetStateInfo().size()));
    addRow(cat, "Aliases", QString::number(m->GetAliases().size()));
}

// ── Model Group ───────────────────────────────────────────────────────────────

void LayoutPropertyTree::populateGroupIdentity(ModelGroup* g) {
    auto* cat = addCategory("Model Group");
    addRow(cat, "Name",   qstr(g->GetName()));
    addRow(cat, "Layout", qstr(g->GetLayout()));
    addRow(cat, "Members", QString::number(g->ModelNames().size()));
    addRow(cat, "Layout Group", qstr(g->GetLayoutGroup()));
    addRow(cat, "Active",  g->IsActive() ? "yes" : "no");
}

void LayoutPropertyTree::populateGroupBuffer(ModelGroup* g) {
    auto* cat = addCategory("Render Buffer");
    addRow(cat, "Buffer W",   QString::number(g->GetDefaultBufferWi()));
    addRow(cat, "Buffer H",   QString::number(g->GetDefaultBufferHt()));
    addRow(cat, "Channels",   QString::number(g->GetNumChannels()));
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
    auto* row = addRow(cat, "Tag Color", hexColor(g->GetTagColour()));
    const xlColor tc = g->GetTagColour();
    row->setBackground(1, QBrush(QColor(tc.red, tc.green, tc.blue)));
    addRow(cat, "Default Camera", qstr(g->GetDefaultCamera()));
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

// ── Controller ────────────────────────────────────────────────────────────────

void LayoutPropertyTree::populateControllerIdentity(Controller* c) {
    auto* cat = addCategory(qstr(c->GetType()));
    addRow(cat, "Name",        qstr(c->GetName()));
    addRow(cat, "Type",        qstr(c->GetType()));
    addRow(cat, "Description", qstr(c->GetDescription()));
    addRow(cat, "Vendor",      qstr(c->GetVendor()));
    addRow(cat, "Model",       qstr(c->GetModel()));
    if (!c->GetVariant().empty())
        addRow(cat, "Variant", qstr(c->GetVariant()));
    addRow(cat, "Active",      activeStateLabel(c->GetActive()));
    addRow(cat, "Auto Layout", c->IsAutoLayout() ? "yes" : "no");
    addRow(cat, "Auto Size",   c->IsAutoSize()   ? "yes" : "no");
    addRow(cat, "Auto Upload", c->IsAutoUpload() ? "yes" : "no");
}

void LayoutPropertyTree::populateControllerNetwork(Controller* c) {
    auto* cat = addCategory("Connection");
    addRow(cat, "Protocol", qstr(c->GetProtocol()));
    if (auto* e = dynamic_cast<ControllerEthernet*>(c)) {
        addRow(cat, "IP",         qstr(e->GetIP()));
        addRow(cat, "FPP Proxy",  qstr(e->GetFPPProxy()));
        addRow(cat, "Priority",   QString::number(e->GetPriority()));
    } else if (auto* s = dynamic_cast<ControllerSerial*>(c)) {
        addRow(cat, "Port",  qstr(s->GetPort()));
        addRow(cat, "Speed", QString::number(s->GetSpeed()));
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

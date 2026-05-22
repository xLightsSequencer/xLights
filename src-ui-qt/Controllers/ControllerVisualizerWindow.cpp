#include "ControllerVisualizerWindow.h"
#include "../App/QtXLightsApp.h"
#include "../Bridge/QtSequenceDoc.h"
#include "../Bridge/QtRenderBridge.h"

#include "../../src-core/models/ModelManager.h"
#include "../../src-core/models/Model.h"

#include <pugixml.hpp>
#include <spdlog/spdlog.h>

#include <QComboBox>
#include <QDrag>
#include <QDropEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QSplitter>
#include <QVBoxLayout>

static constexpr int kPortLabelW = 80;
static constexpr int kBoxH       = 46;
static constexpr int kBoxMinW    = 110;
static constexpr int kBoxPad     = 4;

// Stable per-name color for model boxes.
static QColor modelColor(const QString& name) {
    static const QColor kPalette[] = {
        {0x4e,0x79,0xa7}, {0xf2,0x8e,0x2b}, {0xe1,0x57,0x59},
        {0x76,0xb7,0xb2}, {0x59,0xa1,0x4f}, {0xed,0xc9,0x48},
        {0xb0,0x7a,0xa1}, {0xff,0x9d,0xa7}, {0x9c,0x75,0x5f},
        {0xba,0xb0,0xac}
    };
    uint h = qHash(name);
    return kPalette[h % 10];
}

// ── ModelBoxButton ────────────────────────────────────────────────────────────
// A small colored widget representing one model wired to a port.
// Starts a QDrag carrying the model name on mouse drag.

struct ModelPortInfo {
    int     count       = 0;    // pixels (pixel port) or channels (serial port)
    bool    isPixel     = true;
    QString colorOrder;
    char    smartRemote = 0;    // 0 = none, 'A'/'B'/'C'/…
    int     brightness  = 100;
    int     groupCount  = 1;
};

class ModelBoxButton : public QWidget {
    Q_OBJECT
public:
    ModelBoxButton(const QString& modelName, const ModelPortInfo& info,
                   int port, QWidget* parent = nullptr)
        : QWidget(parent), _name(modelName), _info(info), _port(port)
    {
        setFixedHeight(kBoxH);
        setMinimumWidth(kBoxMinW);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        setAcceptDrops(false);
        buildTooltip();
    }

    const QString& modelName() const { return _name; }
    int count() const { return _info.count; }
    bool isPixel() const { return _info.isPixel; }
    int port() const { return _port; }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QColor bg = modelColor(_name);
        p.setBrush(bg);
        p.setPen(bg.darker(130));
        p.drawRoundedRect(rect().adjusted(2,2,-2,-2), 4, 4);

        p.setPen(Qt::white);
        QFont f = p.font(); f.setPointSize(8); f.setBold(true); p.setFont(f);
        const QRect nameR = rect().adjusted(kBoxPad, 4, -kBoxPad, -kBoxH/2);
        p.drawText(nameR, Qt::AlignLeft | Qt::AlignVCenter,
                   p.fontMetrics().elidedText(_name, Qt::ElideRight, nameR.width()));

        f.setBold(false); f.setPointSize(7); p.setFont(f);
        p.setPen(QColor(230,230,230));
        const QRect chR = rect().adjusted(kBoxPad, kBoxH/2, -kBoxPad, -4);
        const QString unit = _info.isPixel ? "px" : "ch";
        p.drawText(chR, Qt::AlignLeft | Qt::AlignVCenter,
                   QString("%1 %2").arg(_info.count).arg(unit));
    }

    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton)
            _dragStart = e->pos();
    }

    void mouseMoveEvent(QMouseEvent* e) override {
        if (!(e->buttons() & Qt::LeftButton)) return;
        if ((e->pos() - _dragStart).manhattanLength() < 8) return;

        auto* drag = new QDrag(this);
        auto* mime = new QMimeData;
        mime->setData("application/x-xlights-model", _name.toUtf8());
        drag->setMimeData(mime);

        // Render a small pixmap for the drag ghost.
        QPixmap px(size());
        render(&px);
        drag->setPixmap(px);
        drag->setHotSpot(_dragStart);
        drag->exec(Qt::MoveAction);
    }

private:
    void buildTooltip() {
        QString tip = _name;
        const QString unit = _info.isPixel ? "px" : "ch";
        tip += QString("\n%1 %2  (Port %3)").arg(_info.count).arg(unit).arg(_port);
        if (!_info.colorOrder.isEmpty() && _info.colorOrder != "RGB")
            tip += "\nColor Order: " + _info.colorOrder;
        if (_info.smartRemote)
            tip += QString("\nSmart Receiver: %1").arg(QChar(_info.smartRemote));
        if (_info.brightness != 100)
            tip += QString("\nBrightness: %1%").arg(_info.brightness);
        if (_info.groupCount > 1)
            tip += QString("\nGrouping: %1").arg(_info.groupCount);
        setToolTip(tip);
    }

    QString      _name;
    ModelPortInfo _info;
    int          _port;
    QPoint       _dragStart;
};

// ── PortRowWidget ─────────────────────────────────────────────────────────────
// One horizontal row: port label + scrollable strip of model boxes.
// Accepts drops of "application/x-xlights-model".

class PortRowWidget : public QWidget {
    Q_OBJECT
public:
    PortRowWidget(const QString& portLabel, int portNum,
                  int maxChannels, QWidget* parent = nullptr)
        : QWidget(parent), _portLabel(portLabel),
          _portNum(portNum), _maxChannels(maxChannels)
    {
        setAcceptDrops(true);
        setFixedHeight(kBoxH + 16);

        auto* hl = new QHBoxLayout(this);
        hl->setContentsMargins(4,4,4,4);
        hl->setSpacing(4);

        auto* lbl = new QLabel(portLabel);
        lbl->setFixedWidth(kPortLabelW);
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lbl->setStyleSheet("color:#aaa;font-size:11px;");
        hl->addWidget(lbl);

        _strip = new QWidget;
        _stripLayout = new QHBoxLayout(_strip);
        _stripLayout->setContentsMargins(2,2,2,2);
        _stripLayout->setSpacing(3);
        _stripLayout->addStretch();

        auto* scroll = new QScrollArea;
        scroll->setWidget(_strip);
        scroll->setWidgetResizable(true);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setFixedHeight(kBoxH + 8);
        hl->addWidget(scroll, 1);

        _overflowLabel = new QLabel("⚠ overflow");
        _overflowLabel->setStyleSheet("color:#e55;font-size:10px;");
        _overflowLabel->hide();
        hl->addWidget(_overflowLabel);
    }

    int portNum() const { return _portNum; }

    void addModel(const QString& name, const ModelPortInfo& info) {
        auto* btn = new ModelBoxButton(name, info, _portNum, _strip);
        const int pos = qMax(0, _stripLayout->count() - 1);
        _stripLayout->insertWidget(pos, btn);
        // Overflow check uses channel count regardless of pixel/serial distinction.
        _totalChannels += info.isPixel ? info.count * 3 : info.count;
        updateOverflow();
    }

    // Remove a model box by name. Returns channel count removed.
    int removeModel(const QString& name) {
        for (int i = 0; i < _stripLayout->count(); ++i) {
            auto* btn = qobject_cast<ModelBoxButton*>(
                _stripLayout->itemAt(i)->widget());
            if (btn && btn->modelName() == name) {
                const int ch = btn->isPixel() ? btn->count() * 3 : btn->count();
                _stripLayout->removeItem(_stripLayout->itemAt(i));
                btn->deleteLater();
                _totalChannels -= ch;
                updateOverflow();
                return ch;
            }
        }
        return 0;
    }

    QStringList modelNames() const {
        QStringList names;
        for (int i = 0; i < _stripLayout->count(); ++i) {
            auto* btn = qobject_cast<ModelBoxButton*>(
                _stripLayout->itemAt(i)->widget());
            if (btn) names.append(btn->modelName());
        }
        return names;
    }

signals:
    void modelDropped(const QString& modelName, int fromPort, int toPort);

protected:
    void dragEnterEvent(QDragEnterEvent* e) override {
        if (e->mimeData()->hasFormat("application/x-xlights-model")) {
            e->acceptProposedAction();
            setStyleSheet("background:#1e3a5f;border-radius:4px;");
        }
    }

    void dragLeaveEvent(QDragLeaveEvent*) override {
        setStyleSheet("");
    }

    void dropEvent(QDropEvent* e) override {
        setStyleSheet("");
        if (!e->mimeData()->hasFormat("application/x-xlights-model")) return;
        const QString name = QString::fromUtf8(
            e->mimeData()->data("application/x-xlights-model"));

        // Find original port from source widget.
        int fromPort = 0;
        if (auto* srcBtn = qobject_cast<ModelBoxButton*>(e->source()))
            fromPort = srcBtn->port();

        e->acceptProposedAction();
        emit modelDropped(name, fromPort, _portNum);
    }

private:
    void updateOverflow() {
        const bool over = _maxChannels > 0 && _totalChannels > _maxChannels;
        _overflowLabel->setVisible(over);
        setStyleSheet(over ? "border:1px solid #e55;border-radius:4px;" : "");
    }

    QString      _portLabel;
    int          _portNum      = 0;
    int          _maxChannels  = 0;
    int          _totalChannels = 0;
    QWidget*     _strip        = nullptr;
    QHBoxLayout* _stripLayout  = nullptr;
    QLabel*      _overflowLabel = nullptr;
};

#include "ControllerVisualizerWindow.moc"

// ── ControllerVisualizerWindow ────────────────────────────────────────────────

ControllerVisualizerWindow::ControllerVisualizerWindow(QWidget* parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle("Controller Visualizer");
    setMinimumSize(900, 500);
    resize(1100, 650);

    // ── Toolbar ───────────────────────────────────────────────────────────
    _ctrlCombo = new QComboBox;
    _ctrlCombo->setMinimumWidth(220);
    auto* refreshBtn = new QPushButton("↺ Refresh");
    auto* toolbar = new QHBoxLayout;
    toolbar->addWidget(new QLabel("Controller:"));
    toolbar->addWidget(_ctrlCombo, 1);
    toolbar->addWidget(refreshBtn);
    toolbar->addStretch();

    // ── Port wiring view ──────────────────────────────────────────────────
    _portContainer = new QWidget;
    _portLayout    = new QVBoxLayout(_portContainer);
    _portLayout->setContentsMargins(4,4,4,4);
    _portLayout->setSpacing(4);
    _portLayout->addStretch();

    _portScroll = new QScrollArea;
    _portScroll->setWidget(_portContainer);
    _portScroll->setWidgetResizable(true);
    _portScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // ── Available models ──────────────────────────────────────────────────
    _availList = new QListWidget;
    _availList->setAcceptDrops(true);
    _availList->setDragDropMode(QAbstractItemView::DropOnly);
    _availList->setMinimumWidth(220);

    auto* availLabel = new QLabel("Available models");
    availLabel->setStyleSheet("font-weight:bold;");
    auto* rightPanel = new QWidget;
    auto* rightVL    = new QVBoxLayout(rightPanel);
    rightVL->setContentsMargins(0,0,0,0);
    rightVL->addWidget(availLabel);
    rightVL->addWidget(_availList, 1);

    // ── Layout ────────────────────────────────────────────────────────────
    _split = new QSplitter(Qt::Horizontal);
    _split->addWidget(_portScroll);
    _split->addWidget(rightPanel);
    _split->setSizes({800, 300});
    _split->setStretchFactor(0, 1);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(6,6,6,6);
    root->addLayout(toolbar);
    root->addWidget(_split, 1);

    // ── Connections ───────────────────────────────────────────────────────
    connect(_ctrlCombo, &QComboBox::currentTextChanged,
            this, &ControllerVisualizerWindow::onControllerChanged);
    connect(refreshBtn, &QPushButton::clicked,
            this, &ControllerVisualizerWindow::refresh);

    // Available list accepts drops: remove model from its current port.
    _availList->installEventFilter(this);
}

bool ControllerVisualizerWindow::eventFilter(QObject* obj, QEvent* ev) {
    if (obj == _availList && ev->type() == QEvent::Drop) {
        auto* de = static_cast<QDropEvent*>(ev);
        if (de->mimeData()->hasFormat("application/x-xlights-model")) {
            const QString name = QString::fromUtf8(
                de->mimeData()->data("application/x-xlights-model"));
            // Clear the wiring for this model.
            saveWiring(name, {}, 0);
            refresh();
            de->acceptProposedAction();
            return true;
        }
    }
    return QWidget::eventFilter(obj, ev);
}

void ControllerVisualizerWindow::setRenderBridge(QtRenderBridge* bridge) {
    _bridge = bridge;
}

void ControllerVisualizerWindow::refresh() {
    // Prefer the live sequence; fall back to show-file data when none is open.
    const QtSequenceInfo& live = QtXLightsApp::instance().currentSequence();
    if (!live.models.isEmpty() || !live.controllers.isEmpty()) {
        _data = live;
    } else {
        _data = QtSequenceInfo{};
        const QString sf = QtXLightsApp::instance().showFolder();
        if (!sf.isEmpty())
            QtSequenceDoc::loadModels(sf + "/xlights_rgbeffects.xml", _data);
    }

    const QString current = _ctrlCombo->currentText();
    _ctrlCombo->blockSignals(true);
    _ctrlCombo->clear();

    for (const QtControllerInfo& ci : _data.controllers)
        _ctrlCombo->addItem(ci.name);

    // Also add controllers referenced only by model assignments.
    QSet<QString> seen;
    for (const auto& ci : _data.controllers) seen.insert(ci.name);
    for (auto it = _data.models.constBegin(); it != _data.models.constEnd(); ++it) {
        if (!it->controllerName.isEmpty() && !seen.contains(it->controllerName)) {
            _ctrlCombo->addItem(it->controllerName);
            seen.insert(it->controllerName);
        }
    }

    _ctrlCombo->blockSignals(false);

    const int idx = _ctrlCombo->findText(current);
    _ctrlCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    buildPortView(_ctrlCombo->currentText());
}

void ControllerVisualizerWindow::openForController(const QString& name) {
    refresh();
    const int idx = _ctrlCombo->findText(name);
    if (idx >= 0) _ctrlCombo->setCurrentIndex(idx);
    show(); raise(); activateWindow();
}

void ControllerVisualizerWindow::onControllerChanged(const QString& name) {
    buildPortView(name);
}

void ControllerVisualizerWindow::clearPortView() {
    // Remove all PortRowWidgets from the layout (leave the trailing stretch).
    while (_portLayout->count() > 1) {
        auto* item = _portLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

void ControllerVisualizerWindow::buildPortView(const QString& ctrlName) {
    clearPortView();
    _availList->clear();

    if (ctrlName.isEmpty()) return;

    const QtSequenceInfo& seq = _data;

    // Find controller caps for port count and channel limits.
    int maxPixelPorts  = 0;
    int maxSerialPorts = 0;
    int maxPortCh      = 0;
    for (const QtControllerInfo& ci : seq.controllers) {
        if (ci.name == ctrlName) {
            maxPixelPorts  = ci.pixelPortCount;
            maxSerialPorts = ci.serialPortCount;
            maxPortCh      = ci.pixelPortChannels;
            break;
        }
    }

    // Group models by port for this controller.
    // Multi-string models span GetNumStrings() consecutive ports.
    // Pixel ports show pixel count; serial ports show channel count.
    ModelManager* mm = _bridge ? _bridge->modelManager() : nullptr;
    QMap<int, QList<QPair<QString,ModelPortInfo>>> portModels;
    for (auto it = seq.models.constBegin(); it != seq.models.constEnd(); ++it) {
        const QtModelInfo& m = *it;
        if (m.controllerName != ctrlName) continue;
        const int startPort = qMax(1, m.controllerPort);

        ModelPortInfo info;
        info.isPixel = true;   // assume pixel unless overridden below

        int numStrings = 1;
        if (mm) {
            Model* model = mm->GetModel(m.name.toStdString());
            if (model) {
                numStrings          = qMax(1, model->GetNumStrings());
                const int totalNodes = static_cast<int>(model->GetNodeCount());
                const int chanPerNode = model->GetChanCountPerNode();
                // Pixel count per port (nodes / strings); channels for serial models.
                if (chanPerNode == 1) {
                    info.isPixel = false;
                    info.count   = static_cast<int>(model->GetChanCount()) / numStrings;
                } else {
                    info.isPixel = true;
                    info.count   = totalNodes / numStrings;
                }
                info.colorOrder  = QString::fromStdString(model->GetControllerColorOrder());
                info.smartRemote = model->GetSmartRemoteLetter();
                info.brightness  = model->GetControllerBrightness();
                info.groupCount  = model->GetControllerGroupCount();
            }
        } else {
            info.count = m.nodeCount / numStrings;
        }

        for (int s = 0; s < numStrings; ++s)
            portModels[startPort + s].append({m.name, info});
    }

    // Determine how many pixel ports to show.
    const int maxAssigned = portModels.isEmpty() ? 0 : portModels.lastKey();
    const int showPorts   = qMax(maxAssigned, maxPixelPorts > 0 ? maxPixelPorts : 8);

    // Build port rows.
    for (int p = 1; p <= showPorts; ++p) {
        const QString portLabel2 = QString("Pixel %1").arg(p);
        auto* row = new PortRowWidget(portLabel2, p, maxPortCh, _portContainer);

        if (portModels.contains(p)) {
            auto& mlist = portModels[p];
            std::sort(mlist.begin(), mlist.end(), [&](const auto& a, const auto& b) {
                return seq.models.value(a.first).startChannel
                     < seq.models.value(b.first).startChannel;
            });
            for (const auto& [name, info] : mlist)
                row->addModel(name, info);
        }

        connect(row, &PortRowWidget::modelDropped,
                this, [this](const QString& modelName, int fromPort, int toPort) {
            Q_UNUSED(fromPort)
            saveWiring(modelName, _ctrlCombo->currentText(), toPort);
            refresh();
        });

        _portLayout->insertWidget(_portLayout->count() - 1, row);
    }

    // Serial ports.
    for (int p = 1; p <= maxSerialPorts; ++p) {
        const QString label = QString("Serial %1").arg(p);
        const int sport = showPorts + p;
        auto* row = new PortRowWidget(label, sport, 512, _portContainer);
        _portLayout->insertWidget(_portLayout->count() - 1, row);
    }

    // Available models: not assigned to this controller.
    for (auto it = seq.models.constBegin(); it != seq.models.constEnd(); ++it) {
        if (it->controllerName != ctrlName) {
            auto* item = new QListWidgetItem(
                QString("%1  (%2, %3 nodes)").arg(it->name).arg(it->type).arg(it->nodeCount));
            item->setData(Qt::UserRole, it->name);
            _availList->addItem(item);
        }
    }
}

void ControllerVisualizerWindow::saveWiring(const QString& modelName,
                                             const QString& ctrlName, int port)
{
    const QString sf = QtXLightsApp::instance().showFolder();
    if (sf.isEmpty()) return;

    const QString xmlPath = sf + "/xlights_rgbeffects.xml";
    pugi::xml_document doc;
    if (!doc.load_file(xmlPath.toStdString().c_str())) return;

    pugi::xml_node root = doc.child("xrgb");
    if (!root) root = doc.child("xlights_rgbeffects");
    if (!root) return;

    pugi::xml_node modelsList = root.child("models");
    if (!modelsList) return;

    for (auto m : modelsList.children("model")) {
        if (QString::fromUtf8(m.attribute("name").as_string()) != modelName) continue;

        if (ctrlName.isEmpty()) {
            m.remove_attribute("Controller");
            m.remove_child("ControllerConnection");
        } else {
            auto ctrlAttr = m.attribute("Controller");
            if (!ctrlAttr) ctrlAttr = m.append_attribute("Controller");
            ctrlAttr.set_value(ctrlName.toStdString().c_str());

            auto cc = m.child("ControllerConnection");
            if (!cc) cc = m.append_child("ControllerConnection");
            auto portAttr = cc.attribute("Port");
            if (!portAttr) portAttr = cc.append_attribute("Port");
            portAttr.set_value(port);
        }
        break;
    }

    doc.save_file(xmlPath.toStdString().c_str());

    // Reload so other windows stay in sync.
    QtXLightsApp::instance().setShowFolder(sf);
}

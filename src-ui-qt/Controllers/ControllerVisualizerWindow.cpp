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

static QList<QPair<QString, ModelPortInfo>> orderPortModels(
    const QList<QPair<QString, ModelPortInfo>>& mlist,
    const QtSequenceInfo& seq);

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

// ── StripDropArea ─────────────────────────────────────────────────────────────
// Inner widget that holds the model-box strip. Accepts drops directly so the
// drop area covers the visible part of the row (the row's own drop handler
// can't see drops here because the QScrollArea/viewport sits on top of it).

class StripDropArea : public QWidget {
    Q_OBJECT
public:
    explicit StripDropArea(QWidget* parent = nullptr) : QWidget(parent) {
        setAcceptDrops(true);
    }

signals:
    // insertIndex is the position in the port's ordered model list where the
    // dropped model should land (0 = first, -1 = append after end).
    void modelDropped(const QString& name, int fromPort, int insertIndex);

protected:
    void dragEnterEvent(QDragEnterEvent* e) override {
        if (e->mimeData()->hasFormat("application/x-xlights-model")) {
            e->acceptProposedAction();
            setStyleSheet("background:#1e3a5f;border-radius:4px;");
        }
    }
    void dragMoveEvent(QDragMoveEvent* e) override {
        if (e->mimeData()->hasFormat("application/x-xlights-model"))
            e->acceptProposedAction();
    }
    void dragLeaveEvent(QDragLeaveEvent*) override {
        setStyleSheet("");
    }
    void dropEvent(QDropEvent* e) override {
        setStyleSheet("");
        if (!e->mimeData()->hasFormat("application/x-xlights-model")) return;
        const QString name = QString::fromUtf8(
            e->mimeData()->data("application/x-xlights-model"));
        int fromPort = 0;
        if (auto* srcBtn = qobject_cast<ModelBoxButton*>(e->source()))
            fromPort = srcBtn->port();

        // Determine insertion index from the drop x position relative to the
        // existing model boxes (excluding the dragged one in a reorder).
        const int dropX = e->position().toPoint().x();
        QList<ModelBoxButton*> btns =
            findChildren<ModelBoxButton*>(QString(), Qt::FindDirectChildrenOnly);
        std::sort(btns.begin(), btns.end(), [](ModelBoxButton* a, ModelBoxButton* b) {
            return a->geometry().x() < b->geometry().x();
        });
        int insertIndex = 0;
        for (ModelBoxButton* btn : btns) {
            if (btn->modelName() == name) continue;
            if (dropX < btn->geometry().center().x()) break;
            ++insertIndex;
        }

        e->acceptProposedAction();
        emit modelDropped(name, fromPort, insertIndex);
    }
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

        _strip = new StripDropArea;
        _stripLayout = new QHBoxLayout(_strip);
        _stripLayout->setContentsMargins(2,2,2,2);
        _stripLayout->setSpacing(3);
        _stripLayout->addStretch();

        connect(_strip, &StripDropArea::modelDropped,
                this, [this](const QString& name, int fromPort, int insertIndex) {
                    emit modelDropped(name, fromPort, _portNum, insertIndex);
                });

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
    void modelDropped(const QString& modelName, int fromPort, int toPort, int insertIndex);

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
        // Drops on the row label area (outside the strip) always append.
        emit modelDropped(name, fromPort, _portNum, -1);
    }

private:
    void updateOverflow() {
        const bool over = _maxChannels > 0 && _totalChannels > _maxChannels;
        _overflowLabel->setVisible(over);
        setStyleSheet(over ? "border:1px solid #e55;border-radius:4px;" : "");
    }

    QString        _portLabel;
    int            _portNum      = 0;
    int            _maxChannels  = 0;
    int            _totalChannels = 0;
    StripDropArea* _strip        = nullptr;
    QHBoxLayout*   _stripLayout  = nullptr;
    QLabel*        _overflowLabel = nullptr;
};

// ── AvailableModelsList ───────────────────────────────────────────────────────
// QListWidget that initiates drags with our custom mime type and accepts drops
// of the same type (to clear a model's wiring).

class AvailableModelsList : public QListWidget {
    Q_OBJECT
public:
    explicit AvailableModelsList(QWidget* parent = nullptr) : QListWidget(parent) {
        setDragEnabled(true);
        setAcceptDrops(true);
        setDropIndicatorShown(false);
        setDragDropMode(QAbstractItemView::DragDrop);
        setDefaultDropAction(Qt::MoveAction);
        setSelectionMode(QAbstractItemView::SingleSelection);
    }

signals:
    void modelDroppedToAvailable(const QString& name);

protected:
    void startDrag(Qt::DropActions /*supportedActions*/) override {
        QListWidgetItem* it = currentItem();
        if (!it) return;
        const QString name = it->data(Qt::UserRole).toString();
        if (name.isEmpty()) return;

        auto* drag = new QDrag(this);
        auto* mime = new QMimeData;
        mime->setData("application/x-xlights-model", name.toUtf8());
        drag->setMimeData(mime);
        drag->exec(Qt::MoveAction);
    }

    void dragEnterEvent(QDragEnterEvent* e) override {
        if (e->mimeData()->hasFormat("application/x-xlights-model") &&
            e->source() != this) {
            e->acceptProposedAction();
        } else {
            QListWidget::dragEnterEvent(e);
        }
    }
    void dragMoveEvent(QDragMoveEvent* e) override {
        if (e->mimeData()->hasFormat("application/x-xlights-model") &&
            e->source() != this) {
            e->acceptProposedAction();
        } else {
            QListWidget::dragMoveEvent(e);
        }
    }
    void dropEvent(QDropEvent* e) override {
        if (e->mimeData()->hasFormat("application/x-xlights-model") &&
            e->source() != this) {
            const QString name = QString::fromUtf8(
                e->mimeData()->data("application/x-xlights-model"));
            e->acceptProposedAction();
            emit modelDroppedToAvailable(name);
            return;
        }
        QListWidget::dropEvent(e);
    }
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
    _availList = new AvailableModelsList;
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

    // Dropping a model box onto the available list clears its wiring.
    connect(_availList, &AvailableModelsList::modelDroppedToAvailable,
            this, [this](const QString& name) {
                const int fromPort = _data.models.value(name).controllerPort;
                QStringList sourceList;
                if (fromPort > 0) {
                    sourceList = portModelNames(fromPort);
                    sourceList.removeAll(name);
                }
                saveWiring(name, {}, 0, {}, sourceList);
                refresh();
            });
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
            auto ordered = orderPortModels(portModels[p], seq);
            for (const auto& [name, info] : ordered)
                row->addModel(name, info);
        }

        connect(row, &PortRowWidget::modelDropped,
                this, [this](const QString& modelName, int fromPort,
                             int toPort, int insertIndex) {
            applyDrop(modelName, fromPort, toPort, insertIndex);
        });

        _portLayout->insertWidget(_portLayout->count() - 1, row);
    }

    // Serial ports.
    for (int p = 1; p <= maxSerialPorts; ++p) {
        const QString label = QString("Serial %1").arg(p);
        const int sport = showPorts + p;
        auto* row = new PortRowWidget(label, sport, 512, _portContainer);
        connect(row, &PortRowWidget::modelDropped,
                this, [this](const QString& modelName, int fromPort,
                             int toPort, int insertIndex) {
            applyDrop(modelName, fromPort, toPort, insertIndex);
        });
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

// Resolve display order from each model's ModelChain attribute.
// A model with empty chain (or one that points outside this port's set) is a
// "root"; otherwise it follows another model. Walk root → next → next … via
// the inverse map. Falls back to start-channel order for roots and any models
// not reachable from a chain.
static QList<QPair<QString, ModelPortInfo>> orderPortModels(
    const QList<QPair<QString, ModelPortInfo>>& mlist,
    const QtSequenceInfo& seq)
{
    QSet<QString> nameSet;
    QMap<QString, QString> follows;   // prev → next (within this port's set)
    QSet<QString> hasPrev;
    for (const auto& [name, info] : mlist) nameSet.insert(name);

    for (const auto& [name, info] : mlist) {
        QString chain = seq.models.value(name).modelChain;
        if (chain.startsWith(">")) chain = chain.mid(1);
        if (!chain.isEmpty() && nameSet.contains(chain)) {
            follows.insert(chain, name);
            hasPrev.insert(name);
        }
    }

    QList<QPair<QString, ModelPortInfo>> roots;
    QMap<QString, ModelPortInfo> infoByName;
    for (const auto& [name, info] : mlist) {
        infoByName.insert(name, info);
        if (!hasPrev.contains(name)) roots.append({name, info});
    }
    std::sort(roots.begin(), roots.end(), [&](const auto& a, const auto& b) {
        return seq.models.value(a.first).startChannel
             < seq.models.value(b.first).startChannel;
    });

    QList<QPair<QString, ModelPortInfo>> ordered;
    QSet<QString> visited;
    for (const auto& [rname, rinfo] : roots) {
        QString cur = rname;
        while (!cur.isEmpty() && !visited.contains(cur)) {
            ordered.append({cur, infoByName.value(cur)});
            visited.insert(cur);
            cur = follows.value(cur);
        }
    }
    // Catch any models stranded by cycles.
    for (const auto& [name, info] : mlist) {
        if (!visited.contains(name)) {
            ordered.append({name, info});
            visited.insert(name);
        }
    }
    return ordered;
}

QStringList ControllerVisualizerWindow::portModelNames(int port) const {
    for (int i = 0; i < _portLayout->count(); ++i) {
        auto* row = qobject_cast<PortRowWidget*>(_portLayout->itemAt(i)->widget());
        if (row && row->portNum() == port)
            return row->modelNames();
    }
    return {};
}

void ControllerVisualizerWindow::applyDrop(const QString& modelName,
                                            int fromPort, int toPort, int insertIndex)
{
    const QString ctrlName = _ctrlCombo->currentText();

    QStringList targetList = portModelNames(toPort);
    targetList.removeAll(modelName);
    if (insertIndex < 0 || insertIndex > targetList.size())
        targetList.append(modelName);
    else
        targetList.insert(insertIndex, modelName);

    QStringList sourceList;
    if (fromPort != 0 && fromPort != toPort) {
        sourceList = portModelNames(fromPort);
        sourceList.removeAll(modelName);
    }

    saveWiring(modelName, ctrlName, toPort, targetList, sourceList);
    refresh();
}

void ControllerVisualizerWindow::saveWiring(const QString& modelName,
                                             const QString& ctrlName, int port,
                                             const QStringList& targetOrder,
                                             const QStringList& sourceOrder)
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

    auto findModel = [&](const QString& name) -> pugi::xml_node {
        for (auto m : modelsList.children("model"))
            if (QString::fromUtf8(m.attribute("name").as_string()) == name)
                return m;
        return {};
    };
    auto setAttr = [](pugi::xml_node node, const char* key, const std::string& value) {
        auto a = node.attribute(key);
        if (!a) a = node.append_attribute(key);
        a.set_value(value.c_str());
    };
    auto setChain = [&](pugi::xml_node m, const QString& chain) {
        if (chain.isEmpty())
            m.remove_attribute("ModelChain");
        else
            setAttr(m, "ModelChain", chain.toStdString());
    };

    // First, the moved model's controller assignment.
    if (auto m = findModel(modelName)) {
        if (ctrlName.isEmpty()) {
            m.remove_attribute("Controller");
            m.remove_child("ControllerConnection");
            m.remove_attribute("ModelChain");
        } else {
            setAttr(m, "Controller", ctrlName.toStdString());
            auto cc = m.child("ControllerConnection");
            if (!cc) cc = m.append_child("ControllerConnection");
            auto p = cc.attribute("Port");
            if (!p) p = cc.append_attribute("Port");
            p.set_value(port);
        }
    }

    // Write the target port's chain in order.
    for (int i = 0; i < targetOrder.size(); ++i) {
        auto m = findModel(targetOrder[i]);
        if (!m) continue;
        if (i == 0) setChain(m, "");
        else        setChain(m, ">" + targetOrder[i - 1]);
    }

    // Patch up the source port's chain (model has been removed from it).
    for (int i = 0; i < sourceOrder.size(); ++i) {
        auto m = findModel(sourceOrder[i]);
        if (!m) continue;
        if (i == 0) setChain(m, "");
        else        setChain(m, ">" + sourceOrder[i - 1]);
    }

    doc.save_file(xmlPath.toStdString().c_str());

    // Reload so other windows stay in sync.
    QtXLightsApp::instance().setShowFolder(sf);
}

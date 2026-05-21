#include "ControllerVisualizerWindow.h"
#include "../App/QtXLightsApp.h"
#include "../Bridge/QtSequenceDoc.h"

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

class ModelBoxButton : public QWidget {
    Q_OBJECT
public:
    ModelBoxButton(const QString& modelName, int channels,
                   int port, QWidget* parent = nullptr)
        : QWidget(parent), _name(modelName), _channels(channels), _port(port)
    {
        setFixedHeight(kBoxH);
        setMinimumWidth(kBoxMinW);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        setToolTip(QString("%1\n%2 channels\nPort %3").arg(modelName).arg(channels).arg(port));
        setAcceptDrops(false);
    }

    const QString& modelName() const { return _name; }
    int channels() const { return _channels; }
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
        p.drawText(chR, Qt::AlignLeft | Qt::AlignVCenter,
                   QString("%1 ch").arg(_channels));
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
    QString _name;
    int     _channels;
    int     _port;
    QPoint  _dragStart;
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

    void addModel(const QString& name, int channels) {
        // Insert before the trailing stretch.
        auto* btn = new ModelBoxButton(name, channels, _portNum, _strip);
        const int pos = qMax(0, _stripLayout->count() - 1);
        _stripLayout->insertWidget(pos, btn);
        _totalChannels += channels;
        updateOverflow();
    }

    // Remove a model box by name. Returns channel count removed.
    int removeModel(const QString& name) {
        for (int i = 0; i < _stripLayout->count(); ++i) {
            auto* btn = qobject_cast<ModelBoxButton*>(
                _stripLayout->itemAt(i)->widget());
            if (btn && btn->modelName() == name) {
                const int ch = btn->channels();
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

void ControllerVisualizerWindow::refresh() {
    const QString current = _ctrlCombo->currentText();

    _ctrlCombo->blockSignals(true);
    _ctrlCombo->clear();

    const QtSequenceInfo& seq = QtXLightsApp::instance().currentSequence();
    for (const QtControllerInfo& ci : seq.controllers)
        _ctrlCombo->addItem(ci.name);

    // Also add controllers found only as model.controllerName assignments.
    QSet<QString> seen;
    for (const auto& ci : seq.controllers) seen.insert(ci.name);
    for (auto it = seq.models.constBegin(); it != seq.models.constEnd(); ++it) {
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

    const QtSequenceInfo& seq = QtXLightsApp::instance().currentSequence();

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
    QMap<int, QList<QPair<QString,int>>> portModels; // port → [(name, channels)]
    for (auto it = seq.models.constBegin(); it != seq.models.constEnd(); ++it) {
        const QtModelInfo& m = *it;
        if (m.controllerName == ctrlName) {
            const int port = qMax(1, m.controllerPort);
            portModels[port].append({m.name, m.nodeCount});
        }
    }

    // Determine how many pixel ports to show.
    const int maxAssigned = portModels.isEmpty() ? 0 : portModels.lastKey();
    const int showPorts   = qMax(maxAssigned, maxPixelPorts > 0 ? maxPixelPorts : 8);

    // Build port rows.
    for (int p = 1; p <= showPorts; ++p) {
        const QString portLabel2 = QString("Pixel %1").arg(p);
        auto* row = new PortRowWidget(portLabel2, p, maxPortCh, _portContainer);

        if (portModels.contains(p)) {
            // Sort by start channel.
            auto& mlist = portModels[p];
            std::sort(mlist.begin(), mlist.end(), [&](const auto& a, const auto& b) {
                const auto& ma = seq.models.value(a.first);
                const auto& mb = seq.models.value(b.first);
                return ma.startChannel < mb.startChannel;
            });
            for (const auto& [name, ch] : mlist)
                row->addModel(name, ch);
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
                QString("%1  (%2, %3 ch)").arg(it->name).arg(it->type).arg(it->nodeCount));
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

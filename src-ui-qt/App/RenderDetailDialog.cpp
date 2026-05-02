#include "RenderDetailDialog.h"

#include <QLabel>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

RenderDetailDialog::RenderDetailDialog(QWidget* parent)
    : QDialog(parent, Qt::Window | Qt::WindowCloseButtonHint)
{
    setWindowTitle("Render Progress");
    setMinimumWidth(340);
    resize(380, 480);

    auto* vbox = new QVBoxLayout(this);
    vbox->setSpacing(6);

    _summary = new QLabel("Not rendering", this);
    _summary->setAlignment(Qt::AlignCenter);
    vbox->addWidget(_summary);

    _bar = new QProgressBar(this);
    _bar->setRange(0, 1);
    _bar->setValue(0);
    _bar->setTextVisible(true);
    _bar->setFormat("%v / %m models");
    vbox->addWidget(_bar);

    _list = new QListWidget(this);
    _list->setSelectionMode(QAbstractItemView::NoSelection);
    _list->setFocusPolicy(Qt::NoFocus);
    _list->setAlternatingRowColors(true);
    _list->setStyleSheet(
        "QListWidget { background: #1e1e1e; color: #ccc; font-family: Segoe UI; font-size: 9pt; }"
        "QListWidget::item:alternate { background: #222; }"
    );
    vbox->addWidget(_list, 1);

    auto* close = new QPushButton("Close", this);
    connect(close, &QPushButton::clicked, this, &QDialog::hide);
    vbox->addWidget(close, 0, Qt::AlignRight);
}

// ── Public API ────────────────────────────────────────────────────────────────

void RenderDetailDialog::beginRender(const QStringList& names) {
    _items.clear();
    _list->clear();
    _bar->setRange(0, names.size());
    _bar->setValue(0);
    _summary->setText(QString("Rendering %1 models…").arg(names.size()));

    for (const QString& n : names) {
        auto* item = new QListWidgetItem("  ○  " + n);
        item->setForeground(QColor(0x88, 0x88, 0x88));
        _list->addItem(item);
        _items.append(item);
    }
}

void RenderDetailDialog::setModelStatus(int index, Status s) {
    if (index < 0 || index >= _items.size()) return;
    QListWidgetItem* item = _items[index];

    // Strip old prefix (5 chars: "  ○  " etc.) then re-prefix
    const QString name = item->text().mid(5);

    switch (s) {
    case Pending:
        item->setText("  ○  " + name);
        item->setForeground(QColor(0x77, 0x77, 0x77));
        break;
    case Rendering:
        item->setText("  ▶  " + name);
        item->setForeground(QColor(0xff, 0xcc, 0x44));
        _list->scrollToItem(item);
        break;
    case Done:
        item->setText("  ●  " + name);
        item->setForeground(QColor(0x44, 0xcc, 0x44));
        break;
    case NoEffects:
        item->setText("  –  " + name);
        item->setForeground(QColor(0x55, 0x55, 0x55));
        break;
    }

    _bar->setValue(_bar->value() + (s == Done || s == NoEffects ? 1 : 0));
}

void RenderDetailDialog::endRender(int rendered, int total) {
    _bar->setValue(total);
    _summary->setText(
        QString("Done — %1 of %2 models had active effects").arg(rendered).arg(total));
}

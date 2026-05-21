#include "ControllerInfoWindow.h"
#include "../App/QtXLightsApp.h"
#include "../Bridge/QtSequenceDoc.h"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

ControllerInfoWindow::ControllerInfoWindow(QWidget* parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle("Controllers");
    setMinimumSize(600, 300);
    resize(800, 400);

    _table = new QTableWidget(this);
    _table->setColumnCount(6);
    _table->setHorizontalHeaderLabels(
        {"Name", "Type", "IP / Port", "Protocol", "Start Ch", "Channels"});
    _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->setAlternatingRowColors(true);
    _table->horizontalHeader()->setStretchLastSection(true);
    _table->verticalHeader()->setVisible(false);
    _table->setSortingEnabled(true);

    _vizBtn = new QPushButton("Open Visualizer…");
    _vizBtn->setEnabled(false);

    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(_vizBtn);

    connect(_table, &QTableWidget::currentCellChanged,
            this, [this](int row, int, int, int) {
        _vizBtn->setEnabled(row >= 0);
    });
    connect(_vizBtn, &QPushButton::clicked, this, [this]() {
        const int row = _table->currentRow();
        if (row < 0) return;
        auto* item = _table->item(row, 0);
        if (item) emit visualizerRequested(item->text());
    });

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(6, 6, 6, 6);
    lay->addWidget(_table);
    lay->addLayout(btnRow);

    refresh();
}

void ControllerInfoWindow::refresh() {
    const auto& controllers = QtXLightsApp::instance().currentSequence().controllers;

    _table->setSortingEnabled(false);
    _table->setRowCount(0);
    _table->setRowCount(controllers.size());

    for (int row = 0; row < controllers.size(); ++row) {
        const QtControllerInfo& c = controllers[row];

        auto* nameItem  = new QTableWidgetItem(c.name);
        auto* typeItem  = new QTableWidgetItem(c.type);
        auto* ipItem    = new QTableWidgetItem(c.ip);
        auto* protItem  = new QTableWidgetItem(c.protocol);
        auto* scItem    = new QTableWidgetItem(QString::number(c.startChannel));
        auto* chItem    = new QTableWidgetItem(QString::number(c.channelCount));

        scItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        chItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        _table->setItem(row, 0, nameItem);
        _table->setItem(row, 1, typeItem);
        _table->setItem(row, 2, ipItem);
        _table->setItem(row, 3, protItem);
        _table->setItem(row, 4, scItem);
        _table->setItem(row, 5, chItem);
    }

    _table->setSortingEnabled(true);
    _table->sortByColumn(0, Qt::AscendingOrder);
    _table->resizeColumnsToContents();
}

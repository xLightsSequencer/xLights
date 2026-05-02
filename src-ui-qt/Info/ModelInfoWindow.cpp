#include "ModelInfoWindow.h"
#include "../App/QtXLightsApp.h"
#include "../Bridge/QtSequenceDoc.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>

ModelInfoWindow::ModelInfoWindow(QWidget* parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle("Models");
    setMinimumSize(700, 400);
    resize(900, 500);

    _table = new QTableWidget(this);
    _table->setColumnCount(6);
    _table->setHorizontalHeaderLabels(
        {"Name", "Type", "Buffer W×H", "Nodes", "Start Ch", "Controller"});
    _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->setAlternatingRowColors(true);
    _table->horizontalHeader()->setStretchLastSection(true);
    _table->verticalHeader()->setVisible(false);
    _table->setSortingEnabled(true);

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(6, 6, 6, 6);
    lay->addWidget(_table);

    refresh();
}

void ModelInfoWindow::refresh() {
    const auto& seq = QtXLightsApp::instance().currentSequence();
    const auto& models = seq.models;

    _table->setSortingEnabled(false);
    _table->setRowCount(0);
    _table->setRowCount(models.size());

    int row = 0;
    for (auto it = models.begin(); it != models.end(); ++it, ++row) {
        const QtModelInfo& m = it.value();

        auto* nameItem = new QTableWidgetItem(m.name);
        auto* typeItem = new QTableWidgetItem(m.type);
        auto* bufItem  = new QTableWidgetItem(
            QString("%1 × %2").arg(m.bufferW).arg(m.bufferH));
        auto* nodeItem = new QTableWidgetItem(QString::number(m.nodeCount));
        auto* scItem   = new QTableWidgetItem(QString::number(m.startChannel));
        auto* ctrlItem = new QTableWidgetItem(QString());   // controller not yet in QtModelInfo

        // Right-align numeric columns
        nodeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        scItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        _table->setItem(row, 0, nameItem);
        _table->setItem(row, 1, typeItem);
        _table->setItem(row, 2, bufItem);
        _table->setItem(row, 3, nodeItem);
        _table->setItem(row, 4, scItem);
        _table->setItem(row, 5, ctrlItem);
    }

    _table->setSortingEnabled(true);
    _table->sortByColumn(0, Qt::AscendingOrder);
    _table->resizeColumnsToContents();
}

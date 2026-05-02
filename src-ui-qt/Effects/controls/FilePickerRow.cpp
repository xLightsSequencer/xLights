#include "FilePickerRow.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

FilePickerRow::FilePickerRow(const QString& label, const QString& filter,
                              QWidget* parent)
    : QWidget(parent), _filter(filter) {
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    auto* lbl = new QLabel(label, this);
    lbl->setFixedWidth(120);
    lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lay->addWidget(lbl);
    _edit = new QLineEdit(this);
    lay->addWidget(_edit, 1);
    auto* btn = new QPushButton("…", this);
    btn->setFixedWidth(28);
    lay->addWidget(btn);
    connect(_edit, &QLineEdit::textChanged, this, &FilePickerRow::valueChanged);
    connect(btn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "Select File", _edit->text(), _filter);
        if (!path.isEmpty()) _edit->setText(path);
    });
}

QString FilePickerRow::value() const { return _edit->text(); }
void FilePickerRow::setValue(const QString& v) { _edit->setText(v); }

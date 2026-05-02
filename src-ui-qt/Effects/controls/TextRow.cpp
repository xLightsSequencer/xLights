#include "TextRow.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

TextRow::TextRow(const QString& label, const QString& defaultVal, QWidget* parent)
    : QWidget(parent) {
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    auto* lbl = new QLabel(label, this);
    lbl->setFixedWidth(120);
    lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lay->addWidget(lbl);
    _edit = new QLineEdit(defaultVal, this);
    lay->addWidget(_edit, 1);
    connect(_edit, &QLineEdit::textChanged, this, &TextRow::valueChanged);
}

QString TextRow::value() const { return _edit->text(); }
void TextRow::setValue(const QString& v) { _edit->setText(v); }

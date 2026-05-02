#include "ComboRow.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

ComboRow::ComboRow(const QString& label, const QStringList& options,
                   const QString& defaultVal, QWidget* parent)
    : QWidget(parent) {
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    auto* lbl = new QLabel(label, this);
    lbl->setFixedWidth(120);
    lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lay->addWidget(lbl);
    _combo = new QComboBox(this);
    _combo->addItems(options);
    int idx = options.indexOf(defaultVal);
    if (idx >= 0) _combo->setCurrentIndex(idx);
    lay->addWidget(_combo, 1);
    connect(_combo, &QComboBox::currentTextChanged, this, &ComboRow::valueChanged);
}

QString ComboRow::value() const { return _combo->currentText(); }
void ComboRow::setValue(const QString& v) {
    int idx = _combo->findText(v);
    if (idx >= 0) _combo->setCurrentIndex(idx);
}

#include "CheckRow.h"
#include <QCheckBox>
#include <QHBoxLayout>

CheckRow::CheckRow(const QString& label, bool defaultVal, QWidget* parent)
    : QWidget(parent) {
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    _check = new QCheckBox(label, this);
    _check->setChecked(defaultVal);
    lay->addWidget(_check);
    lay->addStretch();
    connect(_check, &QCheckBox::toggled, this, &CheckRow::valueChanged);
}

bool CheckRow::value() const { return _check->isChecked(); }
void CheckRow::setValue(bool v) { _check->setChecked(v); }

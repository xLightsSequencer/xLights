#pragma once
#include <QWidget>
class QCheckBox;

class CheckRow : public QWidget {
    Q_OBJECT
public:
    CheckRow(const QString& label, bool defaultVal, QWidget* parent = nullptr);
    bool value() const;
    void setValue(bool v);
signals:
    void valueChanged(bool v);
private:
    QCheckBox* _check;
};

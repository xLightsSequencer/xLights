#pragma once
#include <QWidget>
class QLineEdit;

class FilePickerRow : public QWidget {
    Q_OBJECT
public:
    FilePickerRow(const QString& label, const QString& filter,
                  QWidget* parent = nullptr);
    QString value() const;
    void    setValue(const QString& v);
signals:
    void valueChanged(const QString& v);
private:
    QLineEdit* _edit;
    QString    _filter;
};

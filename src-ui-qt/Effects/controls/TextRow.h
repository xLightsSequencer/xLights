#pragma once
#include <QWidget>
class QLineEdit;

class TextRow : public QWidget {
    Q_OBJECT
public:
    TextRow(const QString& label, const QString& defaultVal, QWidget* parent = nullptr);
    QString value() const;
    void    setValue(const QString& v);
signals:
    void valueChanged(const QString& v);
private:
    QLineEdit* _edit;
};

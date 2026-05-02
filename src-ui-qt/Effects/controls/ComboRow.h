#pragma once
#include <QWidget>
#include <QStringList>
class QComboBox;
class QLabel;

class ComboRow : public QWidget {
    Q_OBJECT
public:
    ComboRow(const QString& label, const QStringList& options,
             const QString& defaultVal, QWidget* parent = nullptr);
    QString value() const;
    void    setValue(const QString& v);
signals:
    void valueChanged(const QString& v);
private:
    QComboBox* _combo;
};

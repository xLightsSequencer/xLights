#pragma once
#include <QToolBar>

class EffectToolBar : public QToolBar {
    Q_OBJECT
public:
    explicit EffectToolBar(QWidget* parent = nullptr);
    void populate(const QStringList& names);

signals:
    void effectSelected(const QString& name);

private:
    static QIcon makeIcon(const QString& name);
};

#pragma once
#include <QWidget>

class QTableWidget;

// Non-modal window listing all controllers loaded from xlights_networks.xml.
class ControllerInfoWindow : public QWidget {
    Q_OBJECT
public:
    explicit ControllerInfoWindow(QWidget* parent = nullptr);

    void refresh();

signals:
    void visualizerRequested(const QString& controllerName);

private:
    QTableWidget* _table;
};

#pragma once
#include <QWidget>

class QTableWidget;

// Non-modal window listing all models loaded from the show file.
// Stays open while the user works in the sequencer.
class ModelInfoWindow : public QWidget {
    Q_OBJECT
public:
    explicit ModelInfoWindow(QWidget* parent = nullptr);

    void refresh();   // reload from QtXLightsApp::instance().currentSequence()

private:
    QTableWidget* _table;
};

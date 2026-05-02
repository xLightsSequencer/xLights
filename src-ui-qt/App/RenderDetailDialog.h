#pragma once
#include <QDialog>
#include <QStringList>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QProgressBar;

// Non-modal popup showing per-model progress during a house render.
// Open by double-clicking the status-bar progress bar.
class RenderDetailDialog : public QDialog {
    Q_OBJECT
public:
    explicit RenderDetailDialog(QWidget* parent = nullptr);

    enum Status { Pending, Rendering, Done, NoEffects };

    // Initialise the model list before a render starts.
    void beginRender(const QStringList& modelNames);

    // Update one model's row during the render loop.
    void setModelStatus(int index, Status s);

    // Mark the render as finished and unlock the close button.
    void endRender(int rendered, int total);

private:
    QProgressBar*           _bar;
    QLabel*                 _summary;
    QListWidget*            _list;
    QList<QListWidgetItem*> _items;
};

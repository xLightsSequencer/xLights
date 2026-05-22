#pragma once
#include <QList>
#include <QPair>
#include <QString>
#include <QWidget>

class ModelEditDialog;
class ModelLayoutCanvas;
class QListWidget;
class QPushButton;
class QListWidgetItem;
class QSplitter;
class QTabWidget;
class QTableWidget;

// Phase 11 layout editor window.
// Non-modal, stays open alongside the sequencer.
// Left panel: tabbed list (Models / Groups / Controllers) + property table.
// Right panel: ModelLayoutCanvas for interactive 2D yard view.
class LayoutWindow : public QWidget {
    Q_OBJECT
public:
    explicit LayoutWindow(QWidget* parent = nullptr);

    // Reload all lists and canvas from the current sequence.
    void refresh();

signals:
    void visualizerRequested(const QString& controllerName);
    void uploadRequested(const QString& controllerName);

private slots:
    void onModelListClicked(QListWidgetItem* item);
    void onGroupListClicked(QListWidgetItem* item);
    void onControllerListClicked(QListWidgetItem* item);
    void onCanvasModelClicked(const QString& modelName);

private:
    void buildModelProps(const QString& name);
    void buildGroupProps(const QString& name);
    void buildControllerProps(const QString& name);
    void clearProps();
    void setProps(const QList<QPair<QString,QString>>& rows);

    // Left side
    QSplitter*         _leftSplit    = nullptr;
    QTabWidget*        _tabs         = nullptr;
    QListWidget*       _modelList    = nullptr;
    QListWidget*       _groupList    = nullptr;
    QListWidget*       _controllerList = nullptr;
    QTableWidget*      _propTable    = nullptr;

    // Right side
    ModelLayoutCanvas* _canvas       = nullptr;

    ModelEditDialog*   _editDialog   = nullptr;
};

#pragma once
#include <QDialog>
#include <QString>
#include <QStringList>

class ModelManager;
class OutputManager;
class QComboBox;
class QLineEdit;
class QListWidget;
class QSpinBox;

// Three small modal dialogs used by the Layout window's + buttons to create
// new models, groups, and controllers.  Each dialog only collects user input;
// the actual construction + registration with src-core happens in
// LayoutWindow's onAdd* slots (so the dialog stays free of src-core deps
// beyond ModelManager / OutputManager pointers used to enumerate choices).

// ── AddModelDialog ────────────────────────────────────────────────────────────

class AddModelDialog : public QDialog {
    Q_OBJECT
public:
    // mm is used to enumerate layout groups and pick a default start channel.
    // May be null — the dialog falls back to a minimal set of choices.
    explicit AddModelDialog(ModelManager* mm, QWidget* parent = nullptr);

    QString typeName() const;         // DisplayAsType string ("Arches", "Tree", …)
    QString modelName() const;
    QString layoutGroup() const;
    QString startChannel() const;     // "1", "&firstModel+1", etc.

private:
    QComboBox* _type        = nullptr;
    QLineEdit* _name        = nullptr;
    QComboBox* _layoutGroup = nullptr;
    QLineEdit* _startChan   = nullptr;

    void seedDefaultName();
    ModelManager* _mm = nullptr;
};

// ── AddGroupDialog ────────────────────────────────────────────────────────────

class AddGroupDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddGroupDialog(ModelManager* mm, QWidget* parent = nullptr);

    QString groupName()   const;
    QString layout()      const;
    QString layoutGroup() const;
    QStringList members() const;       // names of selected member models

private:
    QLineEdit*   _name        = nullptr;
    QComboBox*   _layout      = nullptr;
    QComboBox*   _layoutGroup = nullptr;
    QListWidget* _available   = nullptr;
    QListWidget* _selected    = nullptr;

    void moveSelected(QListWidget* from, QListWidget* to);
    ModelManager* _mm = nullptr;
};

// ── AddControllerDialog ───────────────────────────────────────────────────────

class AddControllerDialog : public QDialog {
    Q_OBJECT
public:
    enum class Type { Ethernet, Serial, Null };

    explicit AddControllerDialog(QWidget* parent = nullptr);

    Type    controllerType() const;
    QString controllerName() const;
    QString vendor()         const;
    QString model()          const;
    QString variant()        const;

    QString protocol()       const;    // Ethernet only — empty otherwise
    QString ip()             const;    // Ethernet only
    QString port()           const;    // Serial only
    int     speed()          const;    // Serial only, 0 when not applicable

private:
    QComboBox* _type        = nullptr;
    QLineEdit* _name        = nullptr;
    QComboBox* _vendor      = nullptr;
    QComboBox* _model       = nullptr;
    QComboBox* _variant     = nullptr;
    QComboBox* _protocol    = nullptr;
    QLineEdit* _ip          = nullptr;
    QComboBox* _port        = nullptr;
    QComboBox* _speed       = nullptr;

    void onTypeChanged();
    void rebuildVendorChoices();
    void rebuildModelChoices();
    void rebuildVariantChoices();
    QString currentTypeString() const;   // "Ethernet" / "Serial" / "Null"
};

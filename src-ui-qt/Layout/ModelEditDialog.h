#pragma once
#include "../Bridge/QtSequenceDoc.h"
#include <QDialog>

class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QTableWidget;
class QTabWidget;

// Non-modal dialog for editing sub-models, faces, and states for one model.
// Opened from LayoutWindow when the user double-clicks a model or presses Edit.
// Saves changes directly to xlights_rgbeffects.xml via pugixml.
class ModelEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit ModelEditDialog(QWidget* parent = nullptr);

    // Load data for the named model and show the dialog.
    void openForModel(const QString& modelName);

private slots:
    // Sub-models
    void onSmSelectionChanged();
    void onSmAdd();
    void onSmDelete();
    void onSmNameEdited(const QString&);
    void onSmBufferStyleChanged(const QString&);
    void onSmRangeAdd();
    void onSmRangeDelete();

    // Faces
    void onFaceSelectionChanged();
    void onFaceAdd();
    void onFaceDelete();
    void onFaceTypeChanged(const QString&);

    // States
    void onStateSelectionChanged();
    void onStateAdd();
    void onStateDelete();
    void onStateTypeChanged(const QString&);
    void onStateEntryAdd();
    void onStateEntryDelete();

    void onSave();

private:
    void setupSubModelsTab(QWidget*);
    void setupFacesTab(QWidget*);
    void setupStatesTab(QWidget*);

    // Flush the editor widgets back to the in-memory list before switching rows.
    void commitCurrentSubModel();
    void commitCurrentFace();
    void commitCurrentState();

    void refreshSmList();
    void refreshFaceList();
    void refreshStateList();

    void populateSmEditor(int idx);
    void populateFaceEditor(int idx);
    void populateStateEditor(int idx);

    bool saveToXml();

    QString _modelName;

    // In-memory copies of the three data sets being edited.
    QList<QtSubModelInfo> _subModels;
    QList<QtFaceInfo>     _faces;
    QList<QtStateInfo>    _states;

    int _curSm    = -1;
    int _curFace  = -1;
    int _curState = -1;

    // Sub-models tab widgets
    QListWidget*  _smList        = nullptr;
    QLineEdit*    _smNameEdit    = nullptr;
    QComboBox*    _smLayout      = nullptr;
    QComboBox*    _smType        = nullptr;
    QComboBox*    _smBufferStyle = nullptr;
    QTableWidget* _smRanges      = nullptr;

    // Faces tab widgets
    QListWidget*  _faceList      = nullptr;
    QComboBox*    _faceType      = nullptr;
    QTableWidget* _faceTable     = nullptr;

    // States tab widgets
    QListWidget*  _stateList     = nullptr;
    QComboBox*    _stateType     = nullptr;
    QTableWidget* _stateTable    = nullptr;

    QTabWidget* _tabs = nullptr;
    QLabel*     _titleLabel = nullptr;
};

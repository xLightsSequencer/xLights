#pragma once
#include "../Bridge/QtSequenceDoc.h"
#include <QDialog>

class ModelNodePreview;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QTableWidget;
class QTabWidget;

// Non-modal dialog for editing sub-models, faces, and states for one model.
// Right panel: ModelNodePreview shows highlighted nodes for the selected row
// and supports lasso selection to populate node fields.
class ModelEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit ModelEditDialog(QWidget* parent = nullptr);
    // seqData must be the effective sequence (live or show-file fallback).
    void openForModel(const QString& modelName, const QtSequenceInfo& seqData);

private slots:
    // Sub-models
    void onSmSelectionChanged();
    void onSmAdd();
    void onSmDelete();
    void onSmNameEdited(const QString&);
    void onSmBufferStyleChanged(const QString&);
    void onSmRangeAdd();
    void onSmRangeDelete();
    void onSmRangeCellChanged(int row, int col);

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

    // Preview lasso → write back to current row
    void onNodesLassoed(const QList<int>& indices);

private:
    void setupSubModelsTab(QWidget*);
    void setupFacesTab(QWidget*);
    void setupStatesTab(QWidget*);

    void commitCurrentSubModel();
    void commitCurrentFace();
    void commitCurrentState();

    void refreshSmList();
    void refreshFaceList();
    void refreshStateList();

    void populateSmEditor(int idx);
    void populateFaceEditor(int idx);
    void populateStateEditor(int idx);

    // Rebuild only the Nodes column cells after a type change.
    void rebuildFaceNodeCells(const QtFaceInfo& fi);
    void rebuildStateNodeCells(const QtStateInfo& si);

    // Highlight nodes in the preview from the currently selected row.
    void previewFromSmRow(int row);
    void previewFromFaceRow(int row);
    void previewFromStateRow(int row);

    // Node-range utilities (1-based strings ↔ 0-based index lists).
    QList<int> parseRangeStr(const QString& s) const;
    QString    indicesToRangeStr(const QList<int>& indices) const;
    QString    indicesToSingleNodeStr(const QList<int>& indices) const;

    bool saveToXml();

    QString _modelName;
    int     _nodeCount = 0;    // total nodes for the current model

    QList<QtSubModelInfo> _subModels;
    QList<QtFaceInfo>     _faces;
    QList<QtStateInfo>    _states;

    int _curSm    = -1;
    int _curFace  = -1;
    int _curState = -1;

    // Which tab is "active" for lasso routing (0=submodels,1=faces,2=states)
    int _activeTab = 0;

    // Sub-models tab
    QListWidget*  _smList        = nullptr;
    QLineEdit*    _smNameEdit    = nullptr;
    QComboBox*    _smLayout      = nullptr;
    QComboBox*    _smType        = nullptr;
    QComboBox*    _smBufferStyle = nullptr;
    QTableWidget* _smRanges      = nullptr;

    // Faces tab
    QListWidget*  _faceList       = nullptr;
    QComboBox*    _faceType       = nullptr;
    QTableWidget* _faceTable      = nullptr;
    QCheckBox*    _faceForceColor = nullptr;

    // States tab
    QListWidget*  _stateList       = nullptr;
    QComboBox*    _stateType       = nullptr;
    QTableWidget* _stateTable      = nullptr;
    QCheckBox*    _stateForceColor = nullptr;

    QTabWidget*       _tabs       = nullptr;
    QLabel*           _titleLabel = nullptr;
    ModelNodePreview* _preview    = nullptr;
};

#pragma once
#include <QColor>
#include <QDialog>
#include <QList>
#include <QMap>

#include "../Bridge/QtSequenceDoc.h"

class FseqWriter;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QtRenderBridge;
class QTimer;
class SequencerModel;

class ExportDialog : public QDialog {
    Q_OBJECT
public:
    ExportDialog(SequencerModel*    model,
                 const QtSequenceInfo& seq,
                 QtRenderBridge*    bridge,
                 QWidget*           parent = nullptr);

private slots:
    void onBrowse();
    void onExport();
    void onCancel();
    void onRenderTick();

private:
    void buildChannelMap();
    void setExporting(bool on);

    // Composite all active layers for modelName at frameIndex.
    // Returns empty list if no active blocks.
    QList<QColor> renderModelAt(const QString& modelName, int frameIndex);

    SequencerModel*  _model;
    QtSequenceInfo   _seq;
    QtRenderBridge*  _bridge;

    QLineEdit*    _pathEdit;
    QProgressBar* _progress;
    QLabel*       _statusLbl;
    QPushButton*  _exportBtn;
    QPushButton*  _cancelBtn;

    // Runtime export state
    QTimer*     _timer        = nullptr;
    FseqWriter* _writer       = nullptr;
    int         _currentFrame  = 0;
    int         _totalFrames   = 0;
    int         _totalChannels = 0;
    bool        _cancelled     = false;

    // Per-model channel layout
    struct ModelChannel {
        int start = 0;    // 0-based channel offset in frame buffer
        int count = 0;    // number of channels (nodeCount * 3)
    };
    QMap<QString, ModelChannel> _channelMap;   // model name → layout
    QList<QString>              _modelOrder;    // unique model names, ordered
};

#pragma once
#include <QDialog>
#include <QString>
#include <atomic>

#include "../../src-core/render/UICallbacks.h"

class QLabel;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QtRenderBridge;
class QThread;

// Qt implementation of UICallbacks — posts progress and messages back
// to the dialog via queued signals so the upload can run on a worker thread.
class QtUploadCallbacks : public QObject, public UICallbacks {
    Q_OBJECT
public:
    explicit QtUploadCallbacks(QObject* parent = nullptr) : QObject(parent) {}

    // UICallbacks interface — all pure virtuals must be provided
    void ShowMessage(const std::string& message,
                     const std::string& caption = "xLights") const override;
    bool PromptYesNo(const std::string& message,
                     const std::string& caption = "xLights") const override;
    std::string PromptForDirectory(const std::string& message,
                                   const std::string& defaultPath = "") const override;
    std::string PromptForFile(const std::string& message,
                              const std::string& wildcard = "",
                              const std::string& defaultPath = "") const override;
    long PromptForNumber(const std::string& message, const std::string& caption,
                         long defaultValue, long min, long max) const override;
    std::string PromptForText(const std::string& message, const std::string& caption,
                              const std::string& defaultValue = "") const override;
    ProgressToken BeginProgress(const std::string& message, int maximum = 100) override;
    void UpdateProgress(ProgressToken token, int value,
                        const std::string& newMessage = "") override;
    void EndProgress(ProgressToken token) override;

signals:
    void messageReady(const QString& caption, const QString& text);
    void progressBegun(int token, int maximum, const QString& message);
    void progressUpdated(int token, int value, const QString& message);
    void progressEnded(int token);

private:
    std::atomic<int> _nextToken{0};
};

// Non-modal dialog that uploads controller outputs + inputs.
// The upload runs on a QThread so the UI stays responsive.
class ControllerUploadDialog : public QDialog {
    Q_OBJECT
public:
    explicit ControllerUploadDialog(QWidget* parent = nullptr);
    ~ControllerUploadDialog() override;

    // Set the controller to upload and show the dialog.
    void openForController(const QString& controllerName,
                           QtRenderBridge* bridge);

private slots:
    void onUploadClicked();
    void onUploadFinished(bool success);
    void onMessage(const QString& caption, const QString& text);
    void onProgressBegun(int token, int maximum, const QString& message);
    void onProgressUpdated(int token, int value, const QString& message);
    void onProgressEnded(int token);

private:
    QLabel*         _ctrlLabel    = nullptr;
    QPlainTextEdit* _log          = nullptr;
    QProgressBar*   _progress     = nullptr;
    QPushButton*    _uploadBtn    = nullptr;
    QPushButton*    _closeBtn     = nullptr;

    QString         _controllerName;
    QtRenderBridge* _bridge       = nullptr;
    QThread*        _workerThread = nullptr;
};

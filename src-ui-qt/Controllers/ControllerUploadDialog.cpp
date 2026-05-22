#include "ControllerUploadDialog.h"
#include "../Bridge/QtRenderBridge.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QMetaObject>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QThread>
#include <QVBoxLayout>

// ── QtUploadCallbacks ─────────────────────────────────────────────────────────

void QtUploadCallbacks::ShowMessage(const std::string& message,
                                     const std::string& caption) const {
    emit const_cast<QtUploadCallbacks*>(this)->messageReady(
        QString::fromStdString(caption), QString::fromStdString(message));
}

bool QtUploadCallbacks::PromptYesNo(const std::string& /*message*/,
                                     const std::string& /*caption*/) const {
    return true;   // auto-accept during unattended upload
}

std::string QtUploadCallbacks::PromptForDirectory(const std::string& /*message*/,
                                                   const std::string& defaultPath) const {
    return defaultPath;
}

std::string QtUploadCallbacks::PromptForFile(const std::string& /*message*/,
                                              const std::string& /*wildcard*/,
                                              const std::string& defaultPath) const {
    return defaultPath;
}

long QtUploadCallbacks::PromptForNumber(const std::string& /*message*/,
                                         const std::string& /*caption*/,
                                         long defaultValue,
                                         long /*min*/, long /*max*/) const {
    return defaultValue;
}

std::string QtUploadCallbacks::PromptForText(const std::string& /*message*/,
                                              const std::string& /*caption*/,
                                              const std::string& defaultValue) const {
    return defaultValue;
}

UICallbacks::ProgressToken QtUploadCallbacks::BeginProgress(
    const std::string& message, int maximum)
{
    const int token = _nextToken.fetch_add(1);
    emit progressBegun(token, maximum, QString::fromStdString(message));
    return token;
}

void QtUploadCallbacks::UpdateProgress(ProgressToken token, int value,
                                        const std::string& newMessage) {
    emit progressUpdated(token, value, QString::fromStdString(newMessage));
}

void QtUploadCallbacks::EndProgress(ProgressToken token) {
    emit progressEnded(token);
}

// ── ControllerUploadDialog ────────────────────────────────────────────────────

ControllerUploadDialog::ControllerUploadDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Upload Controller");
    setMinimumSize(500, 360);
    resize(600, 420);

    _ctrlLabel = new QLabel("No controller selected");
    QFont f = _ctrlLabel->font(); f.setBold(true); _ctrlLabel->setFont(f);

    _progress = new QProgressBar;
    _progress->setRange(0, 100);
    _progress->setValue(0);
    _progress->setTextVisible(true);

    _log = new QPlainTextEdit;
    _log->setReadOnly(true);
    _log->setMaximumBlockCount(2000);

    _uploadBtn = new QPushButton("Upload");
    _closeBtn  = new QPushButton("Close");
    _uploadBtn->setDefault(true);

    auto* btnRow = new QDialogButtonBox;
    btnRow->addButton(_uploadBtn, QDialogButtonBox::ActionRole);
    btnRow->addButton(_closeBtn,  QDialogButtonBox::RejectRole);

    auto* root = new QVBoxLayout(this);
    root->addWidget(_ctrlLabel);
    root->addWidget(_progress);
    root->addWidget(_log, 1);
    root->addWidget(btnRow);

    connect(_uploadBtn, &QPushButton::clicked, this, &ControllerUploadDialog::onUploadClicked);
    connect(_closeBtn,  &QPushButton::clicked, this, &QDialog::reject);
}

ControllerUploadDialog::~ControllerUploadDialog() {
    if (_workerThread) {
        _workerThread->quit();
        _workerThread->wait(3000);
    }
}

void ControllerUploadDialog::openForController(const QString& controllerName,
                                                QtRenderBridge* bridge) {
    _controllerName = controllerName;
    _bridge         = bridge;
    _ctrlLabel->setText("Controller: " + controllerName);
    _log->clear();
    _progress->setValue(0);
    _uploadBtn->setEnabled(true);
    show(); raise(); activateWindow();
}

void ControllerUploadDialog::onUploadClicked() {
    if (!_bridge || _controllerName.isEmpty()) return;

    _uploadBtn->setEnabled(false);
    _log->clear();
    _progress->setValue(0);
    _log->appendPlainText("Starting upload to " + _controllerName + "…");

    // Run upload on a worker thread so the UI stays responsive.
    _workerThread = new QThread(this);

    // Callbacks live on the worker thread — signals cross to the dialog (main thread)
    // via QueuedConnection automatically because threads differ.
    auto* cb = new QtUploadCallbacks;
    cb->moveToThread(_workerThread);

    connect(cb, &QtUploadCallbacks::messageReady,
            this, &ControllerUploadDialog::onMessage);
    connect(cb, &QtUploadCallbacks::progressBegun,
            this, &ControllerUploadDialog::onProgressBegun);
    connect(cb, &QtUploadCallbacks::progressUpdated,
            this, &ControllerUploadDialog::onProgressUpdated);
    connect(cb, &QtUploadCallbacks::progressEnded,
            this, &ControllerUploadDialog::onProgressEnded);

    const QString name   = _controllerName;
    QtRenderBridge* brdg = _bridge;

    // Worker: runs upload, then emits finished back on main thread.
    connect(_workerThread, &QThread::started, this, [this, cb, name, brdg]() {
        const bool ok = brdg->upload(name, cb);
        QMetaObject::invokeMethod(this, "onUploadFinished",
                                  Qt::QueuedConnection,
                                  Q_ARG(bool, ok));
        cb->deleteLater();
    });

    connect(_workerThread, &QThread::finished, _workerThread, &QObject::deleteLater);
    _workerThread->start();
}

void ControllerUploadDialog::onUploadFinished(bool success) {
    _workerThread = nullptr;   // already scheduled for deleteLater
    _progress->setValue(_progress->maximum());
    _uploadBtn->setEnabled(true);
    if (success)
        _log->appendPlainText("\n✔ Upload completed successfully.");
    else
        _log->appendPlainText("\n✖ Upload failed — see messages above.");
}

void ControllerUploadDialog::onMessage(const QString& caption, const QString& text) {
    _log->appendPlainText("[" + caption + "] " + text);
}

void ControllerUploadDialog::onProgressBegun(int /*token*/, int maximum,
                                              const QString& message) {
    _progress->setRange(0, maximum > 0 ? maximum : 100);
    _progress->setValue(0);
    if (!message.isEmpty()) _log->appendPlainText(message);
}

void ControllerUploadDialog::onProgressUpdated(int /*token*/, int value,
                                                const QString& message) {
    _progress->setValue(value);
    if (!message.isEmpty()) _log->appendPlainText("  " + message);
}

void ControllerUploadDialog::onProgressEnded(int /*token*/) {
    _progress->setValue(_progress->maximum());
}

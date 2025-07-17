#include "Record_video.h"
RecordVideoThread::RecordVideoThread(const QString &ip, int duration, QProgressDialog *progress, QObject *parent)
    : QThread(parent), serverIP(ip), durationSec(duration), progressDialog(progress) {}

void RecordVideoThread::run() {
    QTcpSocket socket;
    socket.connectToHost(serverIP, 12345);
    if (!socket.waitForConnected(3000)) {
        emit error("Failed to connect to server.");
        return;
    }

    // Gửi lệnh JSON đến server
    QJsonObject obj;
    obj["command"] = "record_video";
    obj["value"] = QString::number(durationSec);
    QJsonDocument doc(obj);
    socket.write(doc.toJson(QJsonDocument::Compact) + "\n");
    socket.flush();

    if (!socket.waitForReadyRead(10000)) {
        emit error("Server timeout (no response).");
        return;
    }

    QByteArray header = socket.read(9); // Đọc "__VIDEO__"
    if (!header.startsWith("__VIDEO__")) {
        emit error("Server did not return a video file.");
        return;
    }

    // Đặt tên file theo thời gian hiện tại
    QString downloads = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString fileName = QString("video_%1.avi").arg(timestamp);
    QString savePath = downloads + "/" + fileName;

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit error("Cannot save video to disk.");
        return;
    }

    progressDialog->setLabelText("Receiving video...");
    progressDialog->setRange(0, 0); // unknown size

    while (socket.waitForReadyRead(2000)) {
        QByteArray data = socket.readAll();
        if (data.isEmpty()) break;
        file.write(data);
    }

    file.close();
    emit finished(savePath);
}

#include "Capture_photo.h"
CapturePhotoThread::CapturePhotoThread(const QString &ip, QProgressDialog *progress, QObject *parent)
    : QThread(parent), serverIP(ip), progressDialog(progress) {}

void CapturePhotoThread::run()
{
    QTcpSocket socket;
    socket.connectToHost(serverIP, 12345);
    if (!socket.waitForConnected(3000))
    {
        emit error("❌ Failed to connect to server.");
        return;
    }

    // Gửi yêu cầu chụp ảnh
    QJsonObject obj;
    obj["command"] = "capture_photo";
    QJsonDocument doc(obj);
    socket.write(doc.toJson(QJsonDocument::Compact) + "\n");
    socket.flush();

    if (!socket.waitForReadyRead(3000))
    {
        emit error("❌ No response from server.");
        return;
    }

    QByteArray header = socket.read(8); // "__PHOTO_"
    if (!header.startsWith("__PHOTO_"))
    {
        emit error("❌ Invalid photo response header.");
        return;
    }

    // Tạo tên file duy nhất
    QString downloads = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filename = QString("photo_%1.jpg").arg(timestamp);
    QString savePath = downloads + "/" + filename;

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        emit error("❌ Cannot save photo.");
        return;
    }

    QByteArray buffer;
    while (socket.waitForReadyRead(2000))
    {
        buffer = socket.read(4096);
        if (buffer.isEmpty())
            break;
        file.write(buffer);
    }

    file.close();
    emit finished(savePath);
}

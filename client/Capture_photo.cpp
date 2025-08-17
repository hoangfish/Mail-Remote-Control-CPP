#include "Capture_photo.h"
#include <QStandardPaths>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QFile>
#include <QDebug>
#include <QElapsedTimer>

CapturePhotoThread::CapturePhotoThread(const QString &ip, QProgressDialog *progress, QObject *parent)
    : QThread(parent), serverIP(ip), progressDialog(progress) {}

void CapturePhotoThread::run() {
    QTcpSocket socket;
    socket.connectToHost(serverIP, 12345);
    if (!socket.waitForConnected(5000)) {
        emit error("❌ Failed to connect to server.");
        qDebug() << "Connection failed to" << serverIP << ":12345";
        return;
    }
    qDebug() << "Connected to server";

    // Gửi yêu cầu chụp ảnh
    QJsonObject obj;
    obj["command"] = "capture_photo";
    QJsonDocument doc(obj);
    socket.write(doc.toJson(QJsonDocument::Compact) + "\n");
    socket.flush();
    qDebug() << "Sent capture_photo command";

    if (!socket.waitForReadyRead(10000)) {
        emit error("❌ No response from server.");
        qDebug() << "No server response after 10s";
        return;
    }

    // Đọc header nhận diện (chính xác 8 byte)
    QByteArray header;
    while (header.size() < 8) {
        if (!socket.waitForReadyRead(5000)) {
            emit error("❌ Timeout while reading header.");
            qDebug() << "Timeout reading header, received:" << header.size() << "bytes";
            return;
        }
        header.append(socket.read(8 - header.size()));
    }
    qDebug() << "Received header:" << header;
    if (!header.startsWith("__PHOTO_")) {
        emit error("❌ Invalid photo response header: " + QString(header));
        qDebug() << "Invalid header:" << header;
        return;
    }

    // Đọc kích thước ảnh (chính xác 4 byte)
    QByteArray sizeData;
    while (sizeData.size() < 4) {
        if (!socket.waitForReadyRead(5000)) {
            emit error("❌ Timeout while reading image size.");
            qDebug() << "Timeout reading size, received:" << sizeData.size() << "bytes";
            return;
        }
        sizeData.append(socket.read(4 - sizeData.size()));
    }
    uint32_t imgSize;
    memcpy(&imgSize, sizeData.constData(), sizeof(uint32_t));
    qDebug() << "Received image size:" << imgSize << "bytes";

    // Kiểm tra kích thước hợp lệ
    if (imgSize == 0 || imgSize > 10000000) {
        emit error("❌ Invalid image size: " + QString::number(imgSize));
        qDebug() << "Invalid image size:" << imgSize;
        return;
    }

    // Tạo tên file duy nhất
    QString downloads = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filename = QString("photo_%1.jpg").arg(timestamp);
    QString savePath = downloads + "/" + filename;
    qDebug() << "Saving to:" << savePath;

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit error("❌ Cannot save photo.");
        qDebug() << "Failed to open file:" << savePath;
        return;
    }

    // Đọc dữ liệu ảnh đúng kích thước
    QByteArray imageData;
    QElapsedTimer timer;
    timer.start();
    int retries = 5;
    while (imageData.size() < static_cast<int>(imgSize) && timer.elapsed() < 90000) {
        if (socket.state() != QAbstractSocket::ConnectedState) {
            emit error("❌ Socket disconnected while reading image data.");
            qDebug() << "Socket state:" << socket.state() << ", error:" << socket.errorString();
            file.close();
            return;
        }
        if (!socket.waitForReadyRead(15000)) {
            retries--;
            emit error("❌ Timeout while reading image data. Received: " + QString::number(imageData.size()) + "/" + QString::number(imgSize) + ", retries left: " + QString::number(retries));
            qDebug() << "Timeout, received:" << imageData.size() << "/" << imgSize << ", retries left:" << retries;
            if (retries == 0 || timer.elapsed() >= 90000) {
                file.close();
                qDebug() << "Aborted: Total timeout or no retries left";
                return;
            }
            continue;
        }
        QByteArray buffer = socket.read(qMin(static_cast<qint64>(8192), static_cast<qint64>(imgSize - imageData.size())));
        if (buffer.isEmpty()) {
            retries--;
            emit error("❌ Empty buffer while reading image data. Received: " + QString::number(imageData.size()) + "/" + QString::number(imgSize) + ", retries left: " + QString::number(retries));
            qDebug() << "Empty buffer, received:" << imageData.size() << "/" << imgSize << ", retries left:" << retries;
            if (retries == 0 || timer.elapsed() >= 90000) {
                file.close();
                qDebug() << "Aborted: Total timeout or no retries left";
                return;
            }
            continue;
        }
        imageData.append(buffer);
        retries = 5; // Reset retries on successful read
    }

    // Cắt dữ liệu nếu nhận thừa
    if (imageData.size() > static_cast<int>(imgSize)) {
        imageData = imageData.left(imgSize);
        qDebug() << "Truncated excess data:" << imageData.size() << "to" << imgSize;
    }

    file.write(imageData);
    file.close();
    qDebug() << "Saved photo to:" << savePath;
    emit finished(savePath);
}
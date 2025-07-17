#ifndef CAPTURE_PHOTO_H
#define CAPTURE_PHOTO_H
#include <QThread>
#include <QTcpSocket>
#include <QProgressDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDataStream>
#include <QDateTime>
#include <QDebug>
class CapturePhotoThread : public QThread {
    Q_OBJECT

public:
    CapturePhotoThread(const QString &ip, QProgressDialog *progress, QObject *parent = nullptr);

signals:
    void finished(const QString &savedPath);
    void error(const QString &errorMessage);

protected:
    void run() override;

private:
    QString serverIP;
    QProgressDialog *progressDialog;
};

#endif // CAPTURE_PHOTO_H

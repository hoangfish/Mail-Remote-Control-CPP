#ifndef DOWNLOAD_FILE_H
#define DOWNLOAD_FILE_H
#include <QThread>
#include <QProgressDialog>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QMetaObject>
class FileDownloaderThread : public QThread
{
    Q_OBJECT
public:
    QString serverIP, serverPath, savePath;
    QProgressDialog *progressDialog = nullptr;

    FileDownloaderThread(const QString &ip, const QString &remote, const QString &local, QProgressDialog *dialog, QObject *parent = nullptr);

protected:
    void run() override;

signals:
    void finished(const QString &path);
    void error(const QString &msg);
};

#endif // DOWNLOAD_FILE_H

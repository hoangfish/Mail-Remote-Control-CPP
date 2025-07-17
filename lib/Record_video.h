#ifndef RECORD_VIDEO_H
#define RECORD_VIDEO_H
#include <QThread>
#include <QTcpSocket>
#include <QProgressDialog>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDataStream>
#include <QDateTime>
class RecordVideoThread : public QThread
{
    Q_OBJECT

public:
    RecordVideoThread(const QString &ip, int duration, QProgressDialog *progress, QObject *parent = nullptr);

signals:
    void finished(const QString &savedPath);
    void error(const QString &errorMessage);

protected:
    void run() override;

private:
    QString serverIP;
    int durationSec;
    QProgressDialog *progressDialog;
};

#endif // RECORD_VIDEO_H

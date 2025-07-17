#include "Download_file.h"
FileDownloaderThread::FileDownloaderThread(const QString &ip, const QString &remote, const QString &local, QProgressDialog *dialog, QObject *parent)
    : QThread(parent), serverIP(ip), serverPath(remote), savePath(local), progressDialog(dialog) {}

void FileDownloaderThread::run()
{
    try
    {
        QTcpSocket socket;
        socket.connectToHost(serverIP, 12345);
        if (!socket.waitForConnected(3000))
        {
            emit error("Could not connect to server");
            return;
        }

        QJsonObject obj;
        obj["command"] = "download_file";
        obj["path"] = serverPath;
        QJsonDocument doc(obj);
        socket.write(doc.toJson(QJsonDocument::Compact) + "\n");
        socket.flush();

        QByteArray response;
        int total = 0;

        while (socket.waitForReadyRead(3000))
        {
            QByteArray chunk = socket.readAll();
            response += chunk;
            total += chunk.size();
            if (progressDialog)
            {
                int percent = qMin(100, total / 100000);
                QMetaObject::invokeMethod(progressDialog, "setValue", Qt::QueuedConnection, Q_ARG(int, percent));
            }
        }

        if (response.startsWith("__ERROR__"))
        {
            emit error(QString::fromUtf8(response.mid(9)));
        }
        else if (response.startsWith("__FILE__"))
        {
            QFile file(savePath);
            if (file.open(QIODevice::WriteOnly))
            {
                file.write(response.mid(9));
                file.close();
                emit finished(savePath);
            }
            else
            {
                emit error("Cannot write to file");
            }
        }
        else
        {
            emit error("Invalid response from server");
        }
    }
    catch (...)
    {
        emit error("Unknown error occurred");
    }
}

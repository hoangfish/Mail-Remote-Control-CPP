#ifndef CUSTOMMODE_UI_H
#define CUSTOMMODE_UI_H
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QSpinBox>
#include <QStandardPaths>
#include <QList> // Thêm để hỗ trợ QList trong makeButtonsFixed
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QThread>
#include <QFileDialog>
#include <QDir>
#include <QStandardPaths>

class CustomModeUI : public QWidget
{
    Q_OBJECT

public:
    explicit CustomModeUI(QWidget *parent = nullptr);
    ~CustomModeUI();

    // Các thành phần giao diện cần truy cập từ CustomMode.cpp
    QLineEdit *lineEditServerIP;

    // Sidebar buttons
    QPushButton *btnSystem;
    QPushButton *btnProcesses;
    QPushButton *btnApps;
    QPushButton *btnFiles;
    QPushButton *btnWebcam;
    QPushButton *btnKeylogger;

    // System Page
    QPushButton *btnRestart;
    QPushButton *btnShutdown;
    QTextEdit *textOutputSystem;

    // Process Page
    QPushButton *btnListProcesses;
    QPushButton *btnStartProcess;
    QPushButton *btnStopProcess;
    QTableWidget *tableProcesses;
    QTextEdit *textOutputProcess;

    // Apps Page
    QPushButton *btnListApps;
    QPushButton *btnStartAppOnly;
    QPushButton *btnStopAppOnly;
    QLineEdit *lineEditStartAppName;
    QLineEdit *lineEditStopAppName;
    QTextEdit *textEditAppLog;

    // Files Page
    QLineEdit *lineEditFilePath;
    QPushButton *btnGetFile;
    QTextEdit *textOutputFiles;

    // Webcam Page
    QPushButton *btnCapturePhoto;
    QPushButton *btnRecordVideo;
    QSpinBox *spinBoxRecordSec;
    QTextEdit *textOutputWebcam;

    // Keylogger Page
    QPushButton *btnKeyLogger;
    QTextEdit *textOutputKeylogger;

    // Stack widget để chuyển trang
    QStackedWidget *stackedWidget;

private slots:
    void handleBackButton(); // Đóng cửa sổ khi bấm Back

private:
    void setButtonStyle(QPushButton *button);
    void makeButtonsFixed(const QList<QPushButton *> &buttons, int width = 120, int height = 32);
    QTextEdit *createOutputLog(QWidget *parent = nullptr);

    // Tạo từng trang
    QWidget *createSystemPage();
    QWidget *createProcessesPage();
    QWidget *createAppsPage();
    QWidget *createFilesPage();
    QWidget *createWebcamPage();
    QWidget *createKeyloggerPage();
};

#endif // CUSTOMMODE_UI_H

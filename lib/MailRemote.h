#ifndef MAILREMOTE_H
#define MAILREMOTE_H

#include <QMainWindow>
#include <QTimer>
#include <QThread>
#include "MailRemote_ui.h"
#include "gmailapi.h"
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkInterface>
#include <QFile>
#include <QDir>
#include <QJsonArray>

// =========================
// Lớp Worker xử lý kiểm tra mail trong luồng riêng
// =========================
class MailCheckWorker : public QObject
{
    Q_OBJECT
public:
    explicit MailCheckWorker(GmailAPI *gmail, QTextEdit *textRequests, QTextEdit *textResults, QObject *parent = nullptr);
    ~MailCheckWorker();

public slots:
    void checkMail();
    void stop();

signals:
    void mailReceived(const QString &ip, const QString &command, const QString &response);
    void errorOccurred(const QString &error);

private:
    GmailAPI *gmail;
    QTextEdit *textRequests;
    QTextEdit *textResults;
    bool isRunning;
    void sendCommandToServer(const std::string &ip, const std::string &command, const std::string &messageId, const QString &subject);
};

// =========================
// Lớp chính điều khiển giao diện Gmail Remote
// =========================
class MailRemote : public QMainWindow
{
    Q_OBJECT

public:
    explicit MailRemote(QWidget *parent = nullptr);
    ~MailRemote();

private slots:
    void onBackClicked();       // Sự kiện: Nhấn nút "Back"
    void onChooseMailClicked(); // Sự kiện: Nhấn nút "Choose Mail Remote"
    void handleMailReceived(const QString &ip, const QString &command, const QString &response);
    void handleError(const QString &error);

private:
    MailRemoteUI *ui;         // Con trỏ đến giao diện đã tách riêng
    QTimer *mailCheckTimer;   // Timer để kiểm tra email định kỳ
    GmailAPI *gmail;          // Lưu đối tượng GmailAPI
    QThread *mailCheckThread; // Luồng riêng cho kiểm tra mail
    MailCheckWorker *worker;  // Worker xử lý kiểm tra mail

    // === Các hàm xử lý logic ===
    void setupConnections();    // Kết nối signal-slot UI
    void startGmailOAuthFlow(); // Bắt đầu quy trình đăng nhập Gmail
};

#endif // MAILREMOTE_H
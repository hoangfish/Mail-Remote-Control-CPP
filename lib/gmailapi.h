#ifndef GMAILAPI_H
#define GMAILAPI_H
#include <QTextEdit>
#include <QObject>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QEventLoop>
#include <QTimer>
#include <QDateTime>
#include <string>

struct Credentials
{
    std::string accessToken;
    std::string refreshToken;
    std::string userEmail;
};

class GmailAPI
{
public:
    GmailAPI();
    ~GmailAPI();

    bool authenticate(QTextEdit *logTextEdit);
    bool sendMail(const std::string &to, const std::string &subject, const std::string &body, QTextEdit *logEdit);
    bool sendMailWithAttachment(const std::string &to, const std::string &subject, const std::string &body,
                                const QByteArray &attachmentData, const std::string &attachmentName, QTextEdit *logEdit);
    bool sendReplyToUser(const std::string &body, QTextEdit *logEdit, const std::string &subject,
                         const QByteArray &attachmentData = QByteArray(), const std::string &attachmentName = "");
    std::string getUserEmail() const;
    std::string getAccessToken() const;

private:
    Credentials creds;
    bool saveCredentialToFile(const std::string &path);
    bool loadCredentialFromFile(const std::string &path);
    bool refreshAccessToken(QTextEdit *logEdit);
};

#endif // GMAILAPI_H
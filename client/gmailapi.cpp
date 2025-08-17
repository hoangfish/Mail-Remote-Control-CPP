#include "gmailapi.h"
#include "utils.h"

// ========== Hằng số ==========
const std::string TOKEN_FILE = "gmail_token.json";
const std::string GMAIL_CLIENT = "phphuc2417@clc.fitus.edu.vn";
const int AUTH_PORT = 50000;

// ========== Constructor ==========
GmailAPI::GmailAPI()
{
    loadCredentialFromFile(TOKEN_FILE);
}

// ========== Destructor ==========
GmailAPI::~GmailAPI()
{
    // Không cần logic đặc biệt, để rỗng
}

// ========== Hàm kiểm tra cổng có sẵn không ==========
static bool isPortAvailable(int port, QTextEdit *logEdit)
{
    QTcpServer server;
    bool result = server.listen(QHostAddress::LocalHost, port);
    if (!result)
    {
        if (logEdit)
            utils::log(logEdit, "🔍 Cổng " + QString::number(port) + ": Bị chiếm dụng. Lỗi: " + server.errorString());
    }
    else
    {
        if (logEdit)
            utils::log(logEdit, "🔍 Cổng " + QString::number(port) + ": Trống.");
        server.close();
        QThread::msleep(50);
    }
    return result;
}

// ========== Hàm chạy server cục bộ để nhận code ==========
static QString runLocalServer(QTextEdit *logEdit, QTcpServer &server, int port)
{
    QString receivedCode;

    if (!server.isListening())
    {
        if (logEdit)
            utils::log(logEdit, "❌ Server không đang lắng nghe trên port " + QString::number(port) + ". Lỗi: " + server.errorString());
        return "";
    }

    if (logEdit)
        utils::log(logEdit, "🌐 Server cục bộ đang chạy trên port: " + QString::number(port));

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(30000); // Timeout 30s

    QObject::connect(&server, &QTcpServer::newConnection, [&]()
                     {
                         QTcpSocket *socket = server.nextPendingConnection();
                         if (!socket)
                         {
                             if (logEdit)
                                 utils::log(logEdit, "❌ Không có socket kết nối.");
                             return;
                         }

                         if (!socket->waitForReadyRead(5000))
                         {
                             if (logEdit)
                                 utils::log(logEdit, "❌ Hết thời gian chờ dữ liệu từ socket: " + socket->errorString());
                             socket->close();
                             socket->deleteLater();
                             return;
                         }

                         QByteArray request = socket->readAll();
                         QString requestStr = QString::fromUtf8(request);

                         if (logEdit)
                             utils::log(logEdit, "📡 Request nhận được: " + requestStr);

                         if (requestStr.contains("GET /?code="))
                         {
                             int start = requestStr.indexOf("code=") + 5;
                             int end = requestStr.indexOf("&", start);
                             if (end == -1)
                                 end = requestStr.indexOf(" ", start);
                             receivedCode = requestStr.mid(start, end - start);
                         }
                         else
                         {
                             if (logEdit)
                                 utils::log(logEdit, "⚠️ Không tìm thấy mã xác thực trong request.");
                         }

                         // Phản hồi HTTP với JavaScript tự động đóng cửa sổ hoặc redirect sau 3 giây
                         QString response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                                            "<html><body style='font-family: Arial; text-align: center;'>"
                                            "<h1>Xác thực thành công!</h1>"
                                            "<p>Cửa sổ này sẽ tự đóng sau 3 giây.</p>"
                                            "<p>Nếu không, <a href='https://mail.google.com'>click vào đây để mở Gmail</a>.</p>"
                                            "<script>"
                                            "setTimeout(function() {"
                                            "  window.close();"
                                            "  if (!window.closed) {"
                                            "    window.location.href = 'https://mail.google.com';"
                                            "  }"
                                            "}, 3000);"
                                            "</script></body></html>";
                         socket->write(response.toUtf8());
                         socket->flush();
                         socket->waitForBytesWritten(1000);
                         socket->close();
                         socket->deleteLater();
                         server.close();
                         loop.quit(); // Thoát ngay sau khi nhận code
                     });

    QObject::connect(&timeoutTimer, &QTimer::timeout, [&]()
                     {
        if (logEdit) utils::log(logEdit, "⚠️ Hết thời gian chờ mã xác thực trên port " + QString::number(port));
        server.close();
        loop.quit(); });

    loop.exec();
    return receivedCode;
}

// ========== Hàm làm mới token ==========
bool GmailAPI::refreshAccessToken(QTextEdit *logEdit)
{
    try
    {
        if (creds.refreshToken.empty())
        {
            if (logEdit)
                utils::log(logEdit, "⚠️ Không có refresh_token để làm mới.");
            return false;
        }

        QFile file("credentials.json");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            if (logEdit)
                utils::log(logEdit, "❌ Không mở được file credentials.json!");
            return false;
        }

        QByteArray rawData = file.readAll();
        file.close();

        QJsonParseError parseErr;
        QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseErr);
        if (doc.isNull())
        {
            if (logEdit)
                utils::log(logEdit, "❌ Lỗi JSON credentials: " + parseErr.errorString());
            return false;
        }

        QJsonObject obj = doc.object()["installed"].toObject();
        QString clientId = obj["client_id"].toString();
        QString clientSecret = obj["client_secret"].toString();
        QString tokenUri = obj["token_uri"].toString();

        QNetworkAccessManager manager;
        QNetworkRequest request{QUrl(tokenUri)};
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        QByteArray postData;
        postData.append("refresh_token=" + QByteArray::fromStdString(creds.refreshToken));
        postData.append("&client_id=" + clientId.toUtf8());
        postData.append("&client_secret=" + clientSecret.toUtf8());
        postData.append("&grant_type=refresh_token");

        QEventLoop loop;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        timeoutTimer.start(5000); // Timeout 5s

        QNetworkReply *reply = manager.post(request, postData);
        QObject::connect(reply, &QNetworkReply::finished, [&]()
                         {
            if (reply->error() != QNetworkReply::NoError) {
                if (logEdit) utils::log(logEdit, "❌ Lỗi khi làm mới token: " + reply->errorString());
            }
            loop.quit(); });
        QObject::connect(&timeoutTimer, &QTimer::timeout, [&]()
                         {
            if (logEdit) utils::log(logEdit, "⚠️ Timeout khi làm mới token.");
            reply->abort();
            loop.quit(); });
        loop.exec();

        QByteArray response = reply->readAll();
        reply->deleteLater();

        QJsonDocument resDoc = QJsonDocument::fromJson(response);
        if (resDoc.isNull())
        {
            if (logEdit)
                utils::log(logEdit, "❌ Phản hồi làm mới token không phải JSON hợp lệ: " + QString::fromUtf8(response));
            return false;
        }

        QJsonObject resObj = resDoc.object();
        if (!resObj.contains("access_token"))
        {
            if (logEdit)
                utils::log(logEdit, "❌ Không lấy được access_token mới! Response: " + QString::fromUtf8(response));
            return false;
        }

        creds.accessToken = resObj["access_token"].toString().toStdString();
        if (!saveCredentialToFile(TOKEN_FILE))
        {
            if (logEdit)
                utils::log(logEdit, "⚠️ Không lưu được token mới vào file.");
        }

        if (logEdit)
            utils::log(logEdit, "✅ Đã làm mới access_token thành công.");
        return true;
    }
    catch (const std::exception &e)
    {
        if (logEdit)
            utils::log(logEdit, "❌ Lỗi khi làm mới token: " + QString(e.what()));
        return false;
    }
}

// ========== Hàm đăng nhập OAuth2 ==========
bool GmailAPI::authenticate(QTextEdit *logEdit)
{
    try
    {
        if (logEdit)
            utils::log(logEdit, "🔐 Bắt đầu xác thực Gmail OAuth2...");

        // Kiểm tra token hiện có
        if (!creds.accessToken.empty())
        {
            QNetworkAccessManager manager;
            QNetworkRequest profileRequest{QUrl("https://gmail.googleapis.com/gmail/v1/users/me/profile")};
            profileRequest.setRawHeader("Authorization", "Bearer " + QByteArray::fromStdString(creds.accessToken));

            QEventLoop loop;
            QTimer timeoutTimer;
            timeoutTimer.setSingleShot(true);
            timeoutTimer.start(5000); // Timeout 5s

            QNetworkReply *profileReply = manager.get(profileRequest);
            QObject::connect(profileReply, &QNetworkReply::finished, [&]()
                             {
                if (profileReply->error() != QNetworkReply::NoError) {
                    if (logEdit) utils::log(logEdit, "⚠️ Token hiện tại không hợp lệ: " + profileReply->errorString());
                }
                loop.quit(); });
            QObject::connect(&timeoutTimer, &QTimer::timeout, [&]()
                             {
                if (logEdit) utils::log(logEdit, "⚠️ Timeout khi kiểm tra token hiện tại.");
                profileReply->abort();
                loop.quit(); });
            loop.exec();

            if (profileReply->error() == QNetworkReply::NoError)
            {
                QJsonDocument profileDoc = QJsonDocument::fromJson(profileReply->readAll());
                if (profileDoc.object().contains("emailAddress"))
                {
                    creds.userEmail = profileDoc.object()["emailAddress"].toString().toStdString();
                    if (logEdit)
                        utils::log(logEdit, "✅ Token hiện tại còn hợp lệ cho user: " + QString::fromStdString(creds.userEmail));
                    profileReply->deleteLater();
                    return true;
                }
            }
            profileReply->deleteLater();

            // Thử làm mới token nếu có refresh_token
            if (!creds.refreshToken.empty())
            {
                if (refreshAccessToken(logEdit))
                {
                    QNetworkRequest newProfileRequest{QUrl("https://gmail.googleapis.com/gmail/v1/users/me/profile")};
                    newProfileRequest.setRawHeader("Authorization", "Bearer " + QByteArray::fromStdString(creds.accessToken));
                    QNetworkReply *newProfileReply = manager.get(newProfileRequest);
                    QObject::connect(newProfileReply, &QNetworkReply::finished, [&]()
                                     { loop.quit(); });
                    timeoutTimer.start(5000);
                    loop.exec();

                    if (newProfileReply->error() == QNetworkReply::NoError)
                    {
                        QJsonDocument profileDoc = QJsonDocument::fromJson(newProfileReply->readAll());
                        if (profileDoc.object().contains("emailAddress"))
                        {
                            creds.userEmail = profileDoc.object()["emailAddress"].toString().toStdString();
                            if (logEdit)
                                utils::log(logEdit, "✅ Token mới hợp lệ cho user: " + QString::fromStdString(creds.userEmail));
                            newProfileReply->deleteLater();
                            return true;
                        }
                    }
                    newProfileReply->deleteLater();
                }
            }
            if (logEdit)
                utils::log(logEdit, "⚠️ Token hiện tại không hợp lệ, yêu cầu xác thực mới.");
        }

        // Kiểm tra file credentials.json
        QFile file("credentials.json");
        if (!file.exists())
        {
            if (logEdit)
                utils::log(logEdit, "❌ File credentials.json không tồn tại!");
            return false;
        }
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            if (logEdit)
                utils::log(logEdit, "❌ Không mở được file credentials.json!");
            return false;
        }

        QByteArray rawData = file.readAll();
        file.close();

        QJsonParseError parseErr;
        QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseErr);
        if (doc.isNull())
        {
            if (logEdit)
                utils::log(logEdit, "❌ Lỗi JSON credentials: " + parseErr.errorString());
            return false;
        }

        QJsonObject obj = doc.object()["installed"].toObject();
        QString clientId = obj["client_id"].toString();
        QString clientSecret = obj["client_secret"].toString();
        QString authUri = obj["auth_uri"].toString();
        QString tokenUri = obj["token_uri"].toString();

        if (clientId.isEmpty() || clientSecret.isEmpty() || authUri.isEmpty() || tokenUri.isEmpty())
        {
            if (logEdit)
                utils::log(logEdit, "❌ File credentials.json thiếu các trường client_id, client_secret, auth_uri hoặc token_uri.");
            return false;
        }

        // Kiểm tra cổng
        int port = AUTH_PORT;
        if (logEdit)
            utils::log(logEdit, "🔍 Kiểm tra cổng " + QString::number(port));
        QTcpServer server;
        if (!isPortAvailable(port, logEdit))
        {
            if (logEdit)
                utils::log(logEdit, "❌ Cổng " + QString::number(port) + " không khả dụng. Vui lòng giải phóng cổng.");
            return false;
        }

        if (!server.listen(QHostAddress::LocalHost, port))
        {
            if (logEdit)
                utils::log(logEdit, "❌ Lỗi lắng nghe cổng " + QString::number(port) + ": " + server.errorString());
            return false;
        }
        if (logEdit)
            utils::log(logEdit, "✅ Cổng " + QString::number(port) + " đã được lắng nghe thành công.");

        QString redirectUri = QString("http://localhost:%1").arg(port);

        // Mở URL xác thực
        QString scope = "https://mail.google.com/";
        QString authUrl = QString("%1?response_type=code&client_id=%2&redirect_uri=%3&scope=%4&access_type=offline&prompt=consent")
                              .arg(authUri)
                              .arg(clientId)
                              .arg(redirectUri)
                              .arg(scope);

        if (logEdit)
            utils::log(logEdit, "🌐 Mở URL xác thực: " + authUrl);
        utils::openBrowser(authUrl.toStdString());

        // Nhận code từ server cục bộ
        QString code = runLocalServer(logEdit, server, port);
        if (code.isEmpty())
        {
            if (logEdit)
                utils::log(logEdit, "⚠️ Không nhận được mã xác thực.");
            return false;
        }
        if (logEdit)
            utils::log(logEdit, "✅ Nhận được mã xác thực: " + code);

        // Gửi POST để lấy token
        QNetworkAccessManager manager;
        QNetworkRequest request{QUrl(tokenUri)};
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        QByteArray postData;
        std::string encodedCode = utils::urlEncode(code.toStdString());
        postData.append("code=" + QByteArray::fromStdString(encodedCode));
        postData.append("&client_id=" + clientId.toUtf8());
        postData.append("&client_secret=" + clientSecret.toUtf8());
        postData.append("&redirect_uri=" + redirectUri.toUtf8());
        postData.append("&grant_type=authorization_code");

        if (logEdit)
            utils::log(logEdit, "📤 Gửi POST để lấy access_token...");
        QEventLoop loop;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        timeoutTimer.start(5000); // Timeout 5s

        QNetworkReply *reply = manager.post(request, postData);
        QObject::connect(reply, &QNetworkReply::finished, [&]()
                         {
            if (reply->error() != QNetworkReply::NoError) {
                if (logEdit) utils::log(logEdit, "❌ Lỗi khi gửi POST lấy token: " + reply->errorString());
            }
            loop.quit(); });
        QObject::connect(&timeoutTimer, &QTimer::timeout, [&]()
                         {
            if (logEdit) utils::log(logEdit, "⚠️ Timeout khi gửi POST lấy token.");
            reply->abort();
            loop.quit(); });
        loop.exec();

        QByteArray response = reply->readAll();
        reply->deleteLater();

        QJsonDocument resDoc = QJsonDocument::fromJson(response);
        if (resDoc.isNull())
        {
            if (logEdit)
                utils::log(logEdit, "❌ Phản hồi POST không phải JSON hợp lệ: " + QString::fromUtf8(response));
            return false;
        }

        QJsonObject resObj = resDoc.object();
        if (!resObj.contains("access_token"))
        {
            if (logEdit)
                utils::log(logEdit, "❌ Không lấy được access_token! Response: " + QString::fromUtf8(response));
            return false;
        }

        creds.accessToken = resObj["access_token"].toString().toStdString();
        if (resObj.contains("refresh_token"))
        {
            creds.refreshToken = resObj["refresh_token"].toString().toStdString();
        }
        if (logEdit)
            utils::log(logEdit, "✅ Đã lấy được access_token.");

        // Lấy email user thực tế
        QNetworkRequest profileRequest{QUrl("https://gmail.googleapis.com/gmail/v1/users/me/profile")};
        profileRequest.setRawHeader("Authorization", "Bearer " + QByteArray::fromStdString(creds.accessToken));

        if (logEdit)
            utils::log(logEdit, "📤 Gửi GET để lấy profile user...");
        QNetworkReply *profileReply = manager.get(profileRequest);
        timeoutTimer.start(5000);
        QObject::connect(profileReply, &QNetworkReply::finished, [&]()
                         {
            if (profileReply->error() != QNetworkReply::NoError) {
                if (logEdit) utils::log(logEdit, "❌ Lỗi khi lấy profile: " + profileReply->errorString());
            }
            loop.quit(); });
        QObject::connect(&timeoutTimer, &QTimer::timeout, [&]()
                         {
            if (logEdit) utils::log(logEdit, "⚠️ Timeout khi lấy profile user.");
            profileReply->abort();
            loop.quit(); });
        loop.exec();

        QByteArray profileResponse = profileReply->readAll();
        profileReply->deleteLater();

        QJsonDocument profileDoc = QJsonDocument::fromJson(profileResponse);
        if (profileDoc.isNull())
        {
            if (logEdit)
                utils::log(logEdit, "❌ Phản hồi profile không phải JSON hợp lệ: " + QString::fromUtf8(profileResponse));
            return false;
        }

        QJsonObject profileObj = profileDoc.object();
        if (!profileObj.contains("emailAddress"))
        {
            if (logEdit)
                utils::log(logEdit, "❌ Không lấy được email user! Response: " + QString::fromUtf8(profileResponse));
            return false;
        }

        creds.userEmail = profileObj["emailAddress"].toString().toStdString();
        if (!saveCredentialToFile(TOKEN_FILE))
        {
            if (logEdit)
                utils::log(logEdit, "⚠️ Không lưu được token vào file.");
        }

        if (logEdit)
            utils::log(logEdit, "✅ Đăng nhập Gmail thành công! User: " + QString::fromStdString(creds.userEmail));
        return true;
    }
    catch (const std::exception &e)
    {
        if (logEdit)
            utils::log(logEdit, "❌ Lỗi trong authenticate: " + QString(e.what()));
        return false;
    }
    catch (...)
    {
        if (logEdit)
            utils::log(logEdit, "❌ Lỗi không xác định trong authenticate.");
        return false;
    }
}

// ========== Gửi Mail ==========
bool GmailAPI::sendMail(const std::string &to, const std::string &subject, const std::string &body, QTextEdit *logEdit)
{
    try
    {
        if (logEdit)
            utils::log(logEdit, "📨 Đang gửi mail đến " + QString::fromStdString(to));

        // Mã hóa tiêu đề theo RFC 2047
        QString cleanSubject = QString::fromStdString(subject);
        QByteArray encodedSubject = cleanSubject.toUtf8().toBase64();
        QString mimeSubject = "=?UTF-8?B?" + QString(encodedSubject) + "?=";

        QString raw = "To: " + QString::fromStdString(to) + "\r\n" +
                      "Subject: " + mimeSubject + "\r\n" +
                      "Content-Type: text/plain; charset=UTF-8\r\n\r\n" +
                      QString::fromStdString(body);
        QByteArray encoded = raw.toUtf8().toBase64();

        QNetworkAccessManager manager;
        QNetworkRequest request{QUrl("https://gmail.googleapis.com/gmail/v1/users/me/messages/send")};
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", "Bearer " + QByteArray::fromStdString(creds.accessToken));

        QJsonObject payload;
        payload["raw"] = QString::fromUtf8(encoded);

        QEventLoop loop;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        timeoutTimer.start(5000);

        QNetworkReply *reply = manager.post(request, QJsonDocument(payload).toJson());
        QObject::connect(reply, &QNetworkReply::finished, [&]()
                         {
            if (reply->error() != QNetworkReply::NoError) {
                if (logEdit) utils::log(logEdit, "❌ Lỗi khi gửi mail: " + reply->errorString());
            }
            loop.quit(); });
        QObject::connect(&timeoutTimer, &QTimer::timeout, [&]()
                         {
            if (logEdit) utils::log(logEdit, "⚠️ Timeout khi gửi mail.");
            reply->abort();
            loop.quit(); });
        loop.exec();

        QByteArray response = reply->readAll();
        reply->deleteLater();

        QJsonDocument resDoc = QJsonDocument::fromJson(response);
        if (resDoc.isNull() || !resDoc.object().contains("id"))
        {
            if (logEdit)
                utils::log(logEdit, "❌ Gửi mail thất bại: " + QString::fromUtf8(response));
            return false;
        }

        if (logEdit)
            utils::log(logEdit, "✅ Gửi mail thành công.");
        return true;
    }
    catch (const std::exception &e)
    {
        if (logEdit)
            utils::log(logEdit, "❌ Lỗi khi gửi mail: " + QString(e.what()));
        return false;
    }
}

// ========== Gửi Mail với Đính kèm ==========
bool GmailAPI::sendMailWithAttachment(const std::string &to, const std::string &subject, const std::string &body,
                                      const QByteArray &attachmentData, const std::string &attachmentName, QTextEdit *logEdit)
{
    try
    {
        if (logEdit)
            utils::log(logEdit, "📨 Đang gửi mail với đính kèm đến " + QString::fromStdString(to));

        // Mã hóa tiêu đề theo RFC 2047
        QString cleanSubject = QString::fromStdString(subject);
        QByteArray encodedSubject = cleanSubject.toUtf8().toBase64();
        QString mimeSubject = "=?UTF-8?B?" + QString(encodedSubject) + "?=";

        // Mã hóa nội dung đính kèm thành base64
        QString attachmentBase64 = attachmentData.toBase64();

        // Tạo nội dung email với MIME
        QString boundary = "----=_Part_" + QString::number(QDateTime::currentMSecsSinceEpoch());
        QString raw = QString(
                          "To: %1\r\n"
                          "Subject: %2\r\n"
                          "Content-Type: multipart/mixed; boundary=%3\r\n"
                          "\r\n"
                          "--%3\r\n"
                          "Content-Type: text/plain; charset=UTF-8\r\n"
                          "\r\n"
                          "%4\r\n"
                          "--%3\r\n"
                          "Content-Type: application/octet-stream; name=\"%5\"\r\n"
                          "Content-Disposition: attachment; filename=\"%5\"\r\n"
                          "Content-Transfer-Encoding: base64\r\n"
                          "\r\n"
                          "%6\r\n"
                          "--%3--\r\n")
                          .arg(QString::fromStdString(to), mimeSubject, boundary, QString::fromStdString(body),
                               QString::fromStdString(attachmentName), attachmentBase64);

        QByteArray encoded = raw.toUtf8().toBase64();

        QNetworkAccessManager manager;
        QNetworkRequest request{QUrl("https://gmail.googleapis.com/gmail/v1/users/me/messages/send")};
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", "Bearer " + QByteArray::fromStdString(creds.accessToken));

        QJsonObject payload;
        payload["raw"] = QString::fromUtf8(encoded);

        QEventLoop loop;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        timeoutTimer.start(10000); // Timeout 10s cho file lớn

        QNetworkReply *reply = manager.post(request, QJsonDocument(payload).toJson());
        QObject::connect(reply, &QNetworkReply::finished, [&]()
                         {
            if (reply->error() != QNetworkReply::NoError) {
                if (logEdit) utils::log(logEdit, "❌ Lỗi khi gửi mail với đính kèm: " + reply->errorString());
            }
            loop.quit(); });
        QObject::connect(&timeoutTimer, &QTimer::timeout, [&]()
                         {
            if (logEdit) utils::log(logEdit, "⚠️ Timeout khi gửi mail với đính kèm.");
            reply->abort();
            loop.quit(); });
        loop.exec();

        QByteArray response = reply->readAll();
        reply->deleteLater();

        QJsonDocument resDoc = QJsonDocument::fromJson(response);
        if (resDoc.isNull() || !resDoc.object().contains("id"))
        {
            if (logEdit)
                utils::log(logEdit, "❌ Gửi mail với đính kèm thất bại: " + QString::fromUtf8(response));
            return false;
        }

        if (logEdit)
            utils::log(logEdit, "✅ Gửi mail với đính kèm thành công: " + QString::fromStdString(attachmentName));
        return true;
    }
    catch (const std::exception &e)
    {
        if (logEdit)
            utils::log(logEdit, "❌ Lỗi khi gửi mail với đính kèm: " + QString(e.what()));
        return false;
    }
}

// ========== Gửi phản hồi về user ==========
bool GmailAPI::sendReplyToUser(const std::string &body, QTextEdit *logEdit, const std::string &subject,
                               const QByteArray &attachmentData, const std::string &attachmentName)
{
    if (attachmentData.isEmpty() || attachmentName.empty())
    {
        return sendMail(creds.userEmail, subject, body, logEdit);
    }
    else
    {
        return sendMailWithAttachment(creds.userEmail, subject, body, attachmentData, attachmentName, logEdit);
    }
}

// ========== Truy xuất email ==========
std::string GmailAPI::getUserEmail() const
{
    return creds.userEmail;
}

// ========== Lưu token ==========
bool GmailAPI::saveCredentialToFile(const std::string &path)
{
    try
    {
        QFile file(QString::fromStdString(path));
        if (!file.open(QIODevice::WriteOnly))
        {
            return false;
        }

        QJsonObject obj;
        obj["access_token"] = QString::fromStdString(creds.accessToken);
        obj["refresh_token"] = QString::fromStdString(creds.refreshToken);
        obj["user_email"] = QString::fromStdString(creds.userEmail);

        file.write(QJsonDocument(obj).toJson());
        file.close();
        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

// ========== Tải token ==========
bool GmailAPI::loadCredentialFromFile(const std::string &path)
{
    try
    {
        QFile file(QString::fromStdString(path));
        if (!file.open(QIODevice::ReadOnly))
            return false;

        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject())
            return false;

        QJsonObject obj = doc.object();
        creds.accessToken = obj["access_token"].toString().toStdString();
        creds.refreshToken = obj["refresh_token"].toString().toStdString();
        creds.userEmail = obj["user_email"].toString().toStdString();

        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

// ========== Truy xuất access token ==========
std::string GmailAPI::getAccessToken() const
{
    return creds.accessToken;
}
#include "MailRemote.h"
#include "gmailapi.h"
#include "Mail_reader.h"
#include "utils.h"

MailCheckWorker::MailCheckWorker(GmailAPI* gmail, QTextEdit* textRequests, QTextEdit* textResults, QObject* parent)
    : QObject(parent), gmail(gmail), textRequests(textRequests), textResults(textResults), isRunning(true) {
}

MailCheckWorker::~MailCheckWorker() {
}

void MailCheckWorker::checkMail() {
    if (!isRunning) return;

    try {
        if (!gmail || gmail->getAccessToken().empty()) {
            emit errorOccurred("❌ GmailAPI không được khởi tạo hoặc token rỗng.");
            return;
        }

        MailReader reader(gmail->getAccessToken(), gmail->getUserEmail());
        std::vector<MailData> mails = reader.fetchLatestRequests(5, textRequests);

        if (mails.empty()) {
            return; // Không log "Không có email mới"
        }

        const MailData& req = mails[0];

        QRegularExpression ipRegex("\\d+\\.\\d+\\.\\d+\\.\\d+");
        QRegularExpressionMatch match = ipRegex.match(QString::fromStdString(req.subject));
        if (!match.hasMatch()) {
            emit errorOccurred("⚠️ Địa chỉ IP không hợp lệ: " + QString::fromStdString(req.subject));
            return;
        }

        QString ip = QString::fromStdString(req.subject);
        QString command = QString::fromStdString(req.body);

        sendCommandToServer(req.subject, req.body, req.threadId, QString::fromStdString(req.subject));

        QNetworkAccessManager modifyManager;
        QString url = QString("https://gmail.googleapis.com/gmail/v1/users/me/messages/%1/modify").arg(QString::fromStdString(req.threadId));
        QNetworkRequest request{QUrl(url)};
        request.setRawHeader("Authorization", ("Bearer " + QString::fromStdString(gmail->getAccessToken())).toUtf8());
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject payload;
        QJsonArray removeLabels;
        removeLabels.append("UNREAD");
        payload["removeLabelIds"] = removeLabels;

        QNetworkReply* reply = modifyManager.post(request, QJsonDocument(payload).toJson());
        QEventLoop loop;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        timeoutTimer.start(5000);

        QObject::connect(reply, &QNetworkReply::finished, [&]() {
            QByteArray response = reply->readAll();
            if (reply->error() != QNetworkReply::NoError) {
                emit errorOccurred("⚠️ Lỗi đánh dấu email: " + reply->errorString());
            } else {
                QJsonDocument resDoc = QJsonDocument::fromJson(response);
                if (!resDoc.object().contains("id")) {
                    emit errorOccurred("⚠️ Không thể đánh dấu email đã xử lý: " + QString::fromUtf8(response));
                } else {
                    utils::log(textRequests, "✅ Đã đánh dấu email là đã đọc.");
                }
            }
            reply->deleteLater();
            loop.quit();
        });
        QObject::connect(&timeoutTimer, &QTimer::timeout, [&]() {
            emit errorOccurred("⚠️ Timeout khi đánh dấu email.");
            reply->abort();
            loop.quit();
        });
        loop.exec();
    } catch (const std::exception& e) {
        emit errorOccurred("❌ Lỗi trong checkMail: " + QString(e.what()));
    }
}

void MailCheckWorker::stop() {
    isRunning = false;
}

void MailCheckWorker::sendCommandToServer(const std::string& ip, const std::string& command, const std::string& messageId, const QString& subject) {
    QTcpSocket* socket = nullptr;
    try {
        socket = new QTcpSocket();
        socket->connectToHost(QString::fromStdString(ip), 12345);

        if (!socket->waitForConnected(5000)) {
            QString errorMsg = "❌ Không thể kết nối đến server " + QString::fromStdString(ip) + ": " + socket->errorString();
            utils::log(textResults, errorMsg);
            gmail->sendReplyToUser(errorMsg.toStdString(), textResults, subject.toStdString());
            emit errorOccurred(errorMsg);
            return;
        }

        QJsonObject payload;
        std::string cmd, value;
        size_t spacePos = command.find(' ');
        if (spacePos != std::string::npos) {
            cmd = command.substr(0, spacePos);
            value = command.substr(spacePos + 1);
        } else {
            cmd = command;
        }
        // Loại bỏ \r\n trong cmd và value
        cmd.erase(std::remove(cmd.begin(), cmd.end(), '\r'), cmd.end());
        cmd.erase(std::remove(cmd.begin(), cmd.end(), '\n'), cmd.end());
        value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());
        value.erase(std::remove(value.begin(), value.end(), '\n'), value.end());

        payload["command"] = QString::fromStdString(cmd);
        if (!value.empty()) {
            if (cmd == "download_file") {
                payload["path"] = QString::fromStdString(value);
            } else {
                payload["value"] = QString::fromStdString(value);
            }
        }

        QJsonDocument doc(payload);
        QByteArray data = doc.toJson(QJsonDocument::Compact);

        socket->write(data);
        if (!socket->waitForBytesWritten(5000)) {
            QString errorMsg = "❌ Không thể gửi lệnh đến server: " + socket->errorString();
            utils::log(textResults, errorMsg);
            gmail->sendReplyToUser(errorMsg.toStdString(), textResults, subject.toStdString());
            emit errorOccurred(errorMsg);
            return;
        }

        // Nhận phản hồi từ server
        QByteArray response;
        QString fileExtension;
        bool isAttachment = false;

        // Chờ dữ liệu với timeout 30 giây
        QEventLoop loop;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        timeoutTimer.start(30000); // Timeout 30s để nhận file lớn

        QObject::connect(socket, &QTcpSocket::readyRead, [&]() {
            response.append(socket->readAll());
        });
        QObject::connect(socket, &QTcpSocket::disconnected, [&]() {
            loop.quit();
        });
        QObject::connect(&timeoutTimer, &QTimer::timeout, [&]() {
            loop.quit();
        });

        loop.exec();

        if (response.isEmpty()) {
            QString errorMsg = "❌ Không nhận được phản hồi từ server: " + socket->errorString();
            utils::log(textResults, errorMsg);
            gmail->sendReplyToUser(errorMsg.toStdString(), textResults, subject.toStdString());
            emit errorOccurred(errorMsg);
            socket->close();
            socket->deleteLater();
            return;
        }

        // Xử lý phản hồi
        QString serverReply;
        QByteArray attachmentData;

        // Kiểm tra header
        if (response.startsWith("__FILE__")) {
            isAttachment = true;
            fileExtension = ".bin"; // Mặc định, sẽ xác định sau
            attachmentData = response.mid(8); // Bỏ "__FILE__" (8 bytes)
            // Xác định phần mở rộng từ value (đường dẫn file)
            QString path = QString::fromStdString(value);
            QFileInfo fileInfo(path);
            fileExtension = fileInfo.suffix().isEmpty() ? ".bin" : "." + fileInfo.suffix();
            serverReply = "✅ File downloaded successfully.";
        } else if (response.startsWith("__PHOTO__")) {
            isAttachment = true;
            fileExtension = ".jpg";
            // Đọc kích thước ảnh (4 byte)
            if (response.size() < 12) { // "__PHOTO__" (9 bytes) + kích thước (4 bytes)
                QString errorMsg = "❌ Phản hồi ảnh không hợp lệ: quá ngắn.";
                utils::log(textResults, errorMsg);
                gmail->sendReplyToUser(errorMsg.toStdString(), textResults, subject.toStdString());
                emit errorOccurred(errorMsg);
                socket->close();
                socket->deleteLater();
                return;
            }
            uint32_t imgSize = *reinterpret_cast<const uint32_t*>(response.mid(9, 4).constData());
            attachmentData = response.mid(13, imgSize); // Bỏ "__PHOTO__" (9 bytes) + kích thước (4 bytes)
            serverReply = "✅ Photo captured successfully.";
        } else if (response.startsWith("__VIDEO__")) {
            isAttachment = true;
            fileExtension = ".avi";
            attachmentData = response.mid(9); // Bỏ "__VIDEO__" (9 bytes)
            serverReply = "✅ Video recorded successfully.";
        } else {
            // Phản hồi text thông thường
            serverReply = QString::fromUtf8(response).trimmed();
        }

        // Gửi phản hồi qua email
        if (isAttachment) {
            // Gửi email với đính kèm
            gmail->sendReplyToUser(serverReply.toStdString(), textResults, subject.toStdString(), attachmentData, ("attachment" + fileExtension).toStdString());
            utils::log(textResults, "✅ Đã gửi phản hồi với đính kèm (" + fileExtension + ") đến Gmail user với IP: " + QString::fromStdString(ip));
        } else {
            // Gửi email text thông thường
            gmail->sendReplyToUser(serverReply.toStdString(), textResults, subject.toStdString());
            utils::log(textResults, "✅ Đã gửi phản hồi lại Gmail user với IP: " + QString::fromStdString(ip));
        }

        utils::log(textResults, "✅ Đã thực hiện lệnh: " + QString::fromStdString(command));
        emit mailReceived(QString::fromStdString(ip), QString::fromStdString(command), serverReply);

        socket->close();
    } catch (const std::exception& e) {
        QString errorMsg = "❌ Lỗi trong sendCommandToServer: " + QString(e.what());
        utils::log(textResults, errorMsg);
        gmail->sendReplyToUser(errorMsg.toStdString(), textResults, subject.toStdString());
        emit errorOccurred(errorMsg);
    }
    if (socket) {
        socket->deleteLater();
    }
}

MailRemote::MailRemote(QWidget *parent)
    : QMainWindow(parent), gmail(nullptr), mailCheckThread(nullptr), worker(nullptr)
{
    try {
        // Áp dụng stylesheet cho QMainWindow
        this->setStyleSheet("QMainWindow { background-color: #2c3e50; }");
        this->setAutoFillBackground(true);

        ui = new MailRemoteUI(this);
        if (!ui) {
            throw std::runtime_error("Không thể khởi tạo MailRemoteUI");
        }
        ui->setupUI(); // Gọi setupUI từ MailRemote_ui.cpp
        setCentralWidget(ui);
        this->resize(900, 700);
        this->setWindowTitle(QString::fromUtf8("📡 Gmail Remote Control"));

        // Đặt textRequests và textResults thành read-only
        if (ui->textRequests) ui->textRequests->setReadOnly(true);
        if (ui->textResults) ui->textResults->setReadOnly(true);

        mailCheckTimer = new QTimer(this);
        setupConnections();
        //utils::log(ui->textRequests, "✅ Ứng dụng khởi tạo thành công.");
    } catch (const std::exception& e) {
        utils::log(ui->textRequests, "❌ Lỗi khởi tạo ứng dụng: " + QString(e.what()));
    }
}

MailRemote::~MailRemote()
{
    if (mailCheckTimer) {
        mailCheckTimer->stop();
        delete mailCheckTimer;
    }
    if (worker) {
        worker->stop();
    }
    if (mailCheckThread) {
        mailCheckThread->quit();
        mailCheckThread->wait();
        delete mailCheckThread;
    }
    delete gmail;
    delete ui;
    //utils::log(nullptr, "✅ Ứng dụng đã được hủy.");
}

void MailRemote::setupConnections()
{
    try {
        if (!ui || !ui->btnBack || !ui->btnChooseMail) {
            throw std::runtime_error("UI components không được khởi tạo.");
        }
        connect(ui->btnBack, &QPushButton::clicked, this, &MailRemote::onBackClicked);
        connect(ui->btnChooseMail, &QPushButton::clicked, this, &MailRemote::onChooseMailClicked);
        //utils::log(ui->textRequests, "✅ Đã thiết lập kết nối signal-slot.");
    } catch (const std::exception& e) {
        utils::log(ui->textRequests, "❌ Lỗi thiết lập kết nối: " + QString(e.what()));
    }
}

void MailRemote::onBackClicked()
{
    //utils::log(ui->textRequests, "🔙 Nút Back được nhấn. Đóng ứng dụng.");
    if (worker) {
        worker->stop();
    }
    if (mailCheckThread) {
        mailCheckThread->quit();
        mailCheckThread->wait();
    }
    this->close();
}

void MailRemote::onChooseMailClicked()
{
    try {
        if (!ui || !ui->textRequests || !ui->textResults) {
            throw std::runtime_error("UI components không được khởi tạo.");
        }
        ui->textRequests->clear();
        ui->textResults->clear();
        ui->textRequests->append("🔄 Đang đăng nhập Gmail...");
        startGmailOAuthFlow();
    } catch (const std::exception& e) {
        ui->textRequests->append("❌ Lỗi khi nhấn Choose Mail: " + QString(e.what()));
    }
}

void MailRemote::startGmailOAuthFlow()
{
    try {
        if (gmail) {
            delete gmail;
            gmail = nullptr;
        }
        gmail = new GmailAPI();

        utils::log(ui->textRequests, "🔐 Bắt đầu xác thực Gmail...");
        if (!gmail->authenticate(ui->textRequests)) {
            utils::log(ui->textRequests, "❌ Lỗi xác thực Gmail.");
            delete gmail;
            gmail = nullptr;
            return;
        }

        ui->textRequests->append("✅ Đăng nhập thành công: " + QString::fromStdString(gmail->getUserEmail()));
        ui->textRequests->append("✉️ Đã mở Gmail. Vui lòng gửi mail đến phphuc2417@clc.fitus.edu.vn ...");
        ui->textRequests->append("✅ Gmail user đã kết nối: " + QString::fromStdString(gmail->getUserEmail()) +
                                "\n✉️ Waiting for new message to phphuc2417@clc.fitus.edu.vn ...");

        // Mở trình duyệt Gmail
        if (!QDesktopServices::openUrl(QUrl("https://mail.google.com/mail/u/0/#inbox?compose=new"))) {
            utils::log(ui->textRequests, "⚠️ Không thể mở trình duyệt Gmail.");
        } else {
            utils::log(ui->textRequests, "🌐 Đã mở trình duyệt để soạn email.");
        }

        // Khởi động worker trong luồng riêng
        mailCheckThread = new QThread(this);
        worker = new MailCheckWorker(gmail, ui->textRequests, ui->textResults);
        worker->moveToThread(mailCheckThread);

        connect(mailCheckThread, &QThread::started, worker, &MailCheckWorker::checkMail);
        connect(worker, &MailCheckWorker::mailReceived, this, &MailRemote::handleMailReceived);
        connect(worker, &MailCheckWorker::errorOccurred, this, &MailRemote::handleError);
        connect(mailCheckTimer, &QTimer::timeout, worker, &MailCheckWorker::checkMail);

        mailCheckThread->start();
        mailCheckTimer->start(10000); // 10s
        //utils::log(ui->textRequests, "✅ Timer kiểm tra email đã khởi động.");
    } catch (const std::exception& e) {
        utils::log(ui->textRequests, "❌ Lỗi trong startGmailOAuthFlow: " + QString(e.what()));
        delete gmail;
        gmail = nullptr;
    }
}

void MailRemote::handleMailReceived(const QString& ip, const QString& command, const QString& response) {
    utils::log(ui->textRequests, "📥 Đã nhận yêu cầu:");
    utils::log(ui->textRequests, "🌐 IP Server: " + ip);
    utils::log(ui->textRequests, "📦 Command: " + command);
    // Không log response vào textResults, vì đã gửi qua email
}

void MailRemote::handleError(const QString& error) {
    utils::log(ui->textRequests, error);
}
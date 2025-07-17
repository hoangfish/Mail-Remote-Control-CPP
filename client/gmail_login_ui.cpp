#include "gmail_login_ui.h"
#include "MainWindow_ui.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QMessageBox>
#include <QScreen>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonArray>
#include <QTcpServer>
#include <QTcpSocket>
#include <QRegularExpression>

GmailLoginUI::GmailLoginUI(QWidget *parent) : QDialog(parent) {
    this->setWindowTitle("Gmail Remote Login");
    this->setFixedSize(380, 320);
    this->setStyleSheet("background-color: #f8f9fa;");
    setModal(true);

    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    move((screenGeometry.width() - width()) / 2,
         (screenGeometry.height() - height()) / 2);
    setupUI();
}

void GmailLoginUI::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    QLabel *logoLabel = new QLabel(this);
    QPixmap logo("images/google_logo.png");
    if (!logo.isNull()) {
        logo = logo.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        logoLabel->setPixmap(logo);
        logoLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(logoLabel);
    }

    QLabel *title = new QLabel("Remote Gmail Control", this);
    title->setStyleSheet("font: bold 15px Helvetica; color: #202124;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QLabel *desc = new QLabel("Choose your Gmail account to activate remote control", this);
    desc->setStyleSheet("font: 10px Helvetica; color: #5f6368;");
    desc->setAlignment(Qt::AlignCenter);
    desc->setWordWrap(true);
    layout->addWidget(desc);

    QPushButton *loginButton = new QPushButton("🔒 Choose Gmail Client", this);
    loginButton->setCursor(Qt::PointingHandCursor);
    loginButton->setStyleSheet(
        "font: bold 11px Helvetica; color: white;"
        "background-color: #4285F4; padding: 8px 15px;"
        "border: none; border-radius: 5px;"
        "margin-top: 20px;"
    );
    connect(loginButton, &QPushButton::clicked, this, &GmailLoginUI::handleLogin);
    layout->addWidget(loginButton);

    QLabel *footer = new QLabel("© MMT Team | v1.0", this);
    footer->setStyleSheet("font: 8px Helvetica; color: #9aa0a6;");
    footer->setAlignment(Qt::AlignCenter);
    layout->addWidget(footer);
}

void GmailLoginUI::handleLogin() {
    QFile file("credentials.json");
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Unable to open credentials.json.");
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject creds = doc.object().value("installed").toObject();

    QString clientId = creds.value("client_id").toString();
    QString redirectUri = creds.value("redirect_uris").toArray().first().toString();

    if (clientId.isEmpty() || redirectUri.isEmpty()) {
        QMessageBox::critical(this, "Error", "Missing information in credentials.json.");
        return;
    }
    QUrl url("https://accounts.google.com/o/oauth2/v2/auth");
    QUrlQuery query;
    query.addQueryItem("client_id", clientId);
    query.addQueryItem("redirect_uri", redirectUri);
    query.addQueryItem("response_type", "code");
    query.addQueryItem("scope", "https://www.googleapis.com/auth/gmail.readonly");
    query.addQueryItem("access_type", "offline");
    url.setQuery(query);

    QDesktopServices::openUrl(url);
    waitForAuthCode();
}

void GmailLoginUI::waitForAuthCode() {
    redirectServer = new QTcpServer(this);
    if (!redirectServer->listen(QHostAddress::Any, 80)) {
        QMessageBox::critical(this, "Error", "Unable to listen on localhost:80.\nPlease run as administrator or change redirect_uri to a different port.");
        return;
    }

    connect(redirectServer, &QTcpServer::newConnection, this, [=]() {
        QTcpSocket *socket = redirectServer->nextPendingConnection();
        if (!socket) return;

        socket->waitForReadyRead(1000);
        QString request = socket->readAll();

        QString code;
        QRegularExpression re("GET /\\?code=([^& ]+)");
        QRegularExpressionMatch match = re.match(request);
        if (match.hasMatch()) {
            code = match.captured(1);
        }

        QByteArray response = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n"
                              "<html><body><h2>Login successful!</h2><p>You may now return to the application.</p></body></html>";

        socket->write(response);
        socket->disconnectFromHost();
        redirectServer->close();

        if (!code.isEmpty()) {
            exchangeCodeForToken(code);
        } else {
            QMessageBox::critical(this, "Error", "Authorization code not found in redirect.");
        }
    });
}

void GmailLoginUI::exchangeCodeForToken(const QString &code) {
    QFile file("credentials.json");
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Unable to open credentials.json.");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject creds = doc.object().value("installed").toObject();

    QString clientId = creds.value("client_id").toString();
    QString clientSecret = creds.value("client_secret").toString();
    QString redirectUri = creds.value("redirect_uris").toArray().first().toString();

    QUrl tokenUrl("https://oauth2.googleapis.com/token");
    QNetworkRequest request(tokenUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("code", code);
    params.addQueryItem("client_id", clientId);
    params.addQueryItem("client_secret", clientSecret);
    params.addQueryItem("redirect_uri", redirectUri);
    params.addQueryItem("grant_type", "authorization_code");

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QEventLoop loop;
    QNetworkReply *reply = manager->post(request, params.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray replyData = reply->readAll();
    QJsonDocument tokenDoc = QJsonDocument::fromJson(replyData);
    QJsonObject tokenObj = tokenDoc.object();
    if (tokenObj.contains("access_token")) {
        accept();
    }
    reply->deleteLater();
}

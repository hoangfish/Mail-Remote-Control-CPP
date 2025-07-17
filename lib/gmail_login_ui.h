#ifndef GMAIL_LOGIN_UI_H
#define GMAIL_LOGIN_UI_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>

class GmailLoginUI : public QDialog {
    Q_OBJECT
public:
    explicit GmailLoginUI(QWidget *parent = nullptr);

private slots:
    void handleLogin();

private:
    void setupUI();
    void waitForAuthCode();
    void exchangeCodeForToken(const QString &code);

    QTcpServer *redirectServer = nullptr;
};

#endif // GMAIL_LOGIN_UI_H

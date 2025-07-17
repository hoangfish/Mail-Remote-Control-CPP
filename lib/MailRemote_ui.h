#ifndef MAILREMOTE_UI_H
#define MAILREMOTE_UI_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFont>
#include <QPushButton>

class MailRemoteUI : public QWidget
{
    Q_OBJECT

public:
    explicit MailRemoteUI(QWidget *parent = nullptr);
    void setupUI(); // Chuyển setupUI sang public để MailRemote.cpp truy cập

    // Public UI components để các file như MailRemote.cpp truy cập
    QPushButton *btnBack;
    QPushButton *btnChooseMail;
    QLabel *labelRequests;
    QTextEdit *textRequests;
    QLabel *labelResults;
    QTextEdit *textResults;
};

#endif // MAILREMOTE_UI_H
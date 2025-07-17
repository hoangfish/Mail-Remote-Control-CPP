#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QFont>
#include <QStatusBar>
#include <QMenuBar>

class MailRemote;
class CustomMode;
class AboutUs;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openGmailRemote();
    void openCustomMode();
    void openAboutUs();

private:
    QWidget *centralWidget;
    QLabel *titleLabel;
    QPushButton *btn_gmail;
    QPushButton *btn_custom;
    QPushButton *btn_about;

    MailRemote *gmailWindow;
    CustomMode *customWindow;
    AboutUs *aboutWindow;
};

#endif // MAINWINDOW_H
